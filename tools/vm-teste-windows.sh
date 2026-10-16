#!/bin/bash
# Máquina virtual com Windows 11 pra testar o navegador do Windows sem sair
# do Linux. Ao entrar no Windows, um laço (pasta de inicialização de todos os
# usuários) roda os .ps1 que aparecem em compartilhado/comandos e grava a
# saída em compartilhado/saida/<nome>.txt; "foto" tira uma foto da tela.
#
#   tools/vm-teste-windows.sh subir            -> cria (instala o Windows, ~30 min) ou liga
#   tools/vm-teste-windows.sh rodar arquivo.ps1 [espera]  -> roda e mostra a saída
#   tools/vm-teste-windows.sh foto saida.png   -> foto da tela do Windows
#   tools/vm-teste-windows.sh parar | apagar | status
#
# Os comandos rodam como o usuário, na sessão dele (janelas aparecem).
set -euo pipefail

if ! docker info >/dev/null 2>&1 && getent group docker | grep -qw "$USER"; then
  exec newgrp docker <<< "$(printf '%q ' "$0" "$@")"
fi

NOME=navegador-teste
RAM="${RAM:-6G}"
NUCLEOS="${NUCLEOS:-4}"
BASE="$HOME/.local/share/navegador/vm-teste"
COMP="$BASE/compartilhado"

preparar_oem() {
  mkdir -p "$BASE/disco" "$BASE/oem" "$COMP/comandos" "$COMP/saida"
  chattr +C "$BASE/disco" 2>/dev/null || true
  # No fim da instalação (como SYSTEM): libera a pasta compartilhada de
  # convidado e põe o laço na inicialização de todos os usuários.
  cat > "$BASE/oem/install.bat" <<'BAT'
@echo off
reg add HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f
reg add HKLM\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters /v RequireSecuritySignature /t REG_DWORD /d 0 /f
reg add HKLM\SOFTWARE\Policies\Microsoft\Windows\LanmanWorkstation /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f
copy /y C:\OEM\comandos.ps1 C:\ProgramData\comandos.ps1
copy /y C:\OEM\comandos.cmd "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp\comandos.cmd"
BAT
  cat > "$BASE/oem/comandos.cmd" <<'CMD'
@start "" /min powershell -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File C:\ProgramData\comandos.ps1
CMD
  cat > "$BASE/oem/comandos.ps1" <<'PS1'
$base = '\\host.lan\Data'
Set-Content -Path "$base\saida\pronto.txt" -Value (Get-Date -Format s) -ErrorAction SilentlyContinue
while ($true) {
  try {
    Get-ChildItem "$base\comandos\*.ps1" -ErrorAction Stop | Sort-Object Name | ForEach-Object {
      $saida = "$base\saida\$($_.BaseName).txt"
      $script = Get-Content $_.FullName -Raw
      Remove-Item $_.FullName -Force
      try {
        & ([scriptblock]::Create($script)) *>&1 | Out-File -FilePath $saida -Encoding utf8
      } catch {
        $_ | Out-File -FilePath $saida -Encoding utf8 -Append
      }
      Add-Content -Path $saida -Value 'FIM'
    }
  } catch {}
  Start-Sleep -Seconds 2
}
PS1
}

case "${1:-status}" in
  subir)
    if docker container inspect "$NOME" >/dev/null 2>&1; then
      docker start "$NOME"
    else
      preparar_oem
      docker run -d --name "$NOME" \
        -e VERSION=11 -e RAM_SIZE="$RAM" -e CPU_CORES="$NUCLEOS" -e DISK_SIZE=64G \
        -e USERNAME=teste -e PASSWORD=teste \
        -e WIDTH=1600 -e HEIGHT=1000 \
        -p 127.0.0.1:8008:8006 \
        --device=/dev/kvm --device=/dev/net/tun --cap-add NET_ADMIN \
        -v "$BASE/disco:/storage" -v "$BASE/oem:/oem" -v "$COMP:/data" \
        --stop-timeout 120 \
        dockurr/windows
    fi
    echo "Tela: http://127.0.0.1:8008 — pronto quando existir $COMP/saida/pronto.txt"
    ;;
  rodar)
    arquivo="$2"
    espera="${3:-120}"
    nome="$(date +%s%N)-$(basename "$arquivo" .ps1)"
    cp "$arquivo" "$COMP/comandos/$nome.ps1"
    for _ in $(seq "$espera"); do
      if [ -f "$COMP/saida/$nome.txt" ] && tail -1 "$COMP/saida/$nome.txt" | grep -q FIM; then
        cat "$COMP/saida/$nome.txt"
        exit 0
      fi
      sleep 1
    done
    echo "(sem resposta em ${espera}s)" >&2
    exit 1
    ;;
  foto)
    destino="${2:-$COMP/saida/tela.png}"
    tmp=$(mktemp --suffix=.ps1)
    cat > "$tmp" <<'PS1'
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
$tela = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$imagem = New-Object System.Drawing.Bitmap $tela.Width, $tela.Height
$g = [System.Drawing.Graphics]::FromImage($imagem)
$g.CopyFromScreen($tela.Location, [System.Drawing.Point]::Empty, $tela.Size)
$imagem.Save('\\host.lan\Data\saida\tela.png', [System.Drawing.Imaging.ImageFormat]::Png)
"ok $($tela.Width)x$($tela.Height)"
PS1
    "$0" rodar "$tmp" 60 >/dev/null
    rm -f "$tmp"
    [ "$destino" = "$COMP/saida/tela.png" ] || cp "$COMP/saida/tela.png" "$destino"
    echo "$destino"
    ;;
  parar)
    docker stop "$NOME"
    ;;
  apagar)
    docker rm -f "$NOME" >/dev/null 2>&1 || true
    rm -rf "$BASE/disco"
    ;;
  status)
    docker ps -a --filter "name=$NOME" --format '{{.Names}}: {{.Status}}'
    ls "$COMP/saida" 2>/dev/null || true
    ;;
  *)
    echo "uso: $0 subir|rodar|foto|parar|apagar|status" >&2
    exit 1
    ;;
esac
