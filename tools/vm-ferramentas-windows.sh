#!/bin/bash
# Ferramentas da Microsoft pra compilar o navegador pro Windows a partir do
# Linux (docs/win_cross.md do Chromium): uma máquina virtual com Windows 11
# (dockurr/windows) instala sozinha o Visual Studio 2026 Community (C++,
# ATL/MFC), o Windows SDK que o Chromium pede (build/vs_toolchain.py,
# SDK_VERSION) e o Python, e roda o package_from_installed.py do depot_tools.
# O .zip sai em ~/.local/share/navegador/vm-ferramentas/compartilhado.
#
#   tools/vm-ferramentas-windows.sh subir    -> cria (instala tudo, ~1-2 h) ou liga
#   tools/vm-ferramentas-windows.sh status   -> container + progresso.txt
#   tools/vm-ferramentas-windows.sh apagar   -> remove a VM e o disco (o .zip fica)
#
# Separada da VM do Dia (tools/vm-windows.sh): esta só serve pra gerar o
# pacote e pode ser apagada depois. Licença do Visual Studio Community: uso
# individual e de código aberto.
set -euo pipefail

if ! docker info >/dev/null 2>&1 && getent group docker | grep -qw "$USER"; then
  exec newgrp docker <<< "$(printf '%q ' "$0" "$@")"
fi

NOME=navegador-ferramentas
RAM="${RAM:-6G}"
NUCLEOS="${NUCLEOS:-4}"
BASE="$HOME/.local/share/navegador/vm-ferramentas"
DEPOT="${DEPOT:-$HOME/.local/share/navegador/depot_tools}"
# Windows SDK da versão que o Chromium pede (docs/windows_build_instructions.md);
# links da página oficial de downloads do SDK.
SDK_VERSAO="${SDK_VERSAO:-10.0.28000.0}"
SDK_URL="${SDK_URL:-https://go.microsoft.com/fwlink/?linkid=2370315}"  # 10.0.28000.2270

preparar_oem() {
  mkdir -p "$BASE/disco" "$BASE/oem" "$BASE/compartilhado"
  chattr +C "$BASE/disco" 2>/dev/null || true
  cp "$DEPOT/win_toolchain/package_from_installed.py" \
     "$DEPOT/win_toolchain/get_toolchain_if_necessary.py" "$BASE/oem/"
  # Roda no fim da instalação do Windows, como SYSTEM.
  cat > "$BASE/oem/install.bat" <<'BAT'
@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File C:\OEM\ferramentas.ps1 > C:\OEM\ferramentas-saida.log 2>&1
BAT
  cat > "$BASE/oem/ferramentas.ps1" <<PS1
\$ErrorActionPreference = 'Stop'
\$ProgressPreference = 'SilentlyContinue'
# A pasta compartilhada do dockur é de convidado: o Windows 11 recusa
# convidado e exige assinatura no SMB por padrão.
reg add HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f | Out-Null
reg add HKLM\\SYSTEM\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters /v RequireSecuritySignature /t REG_DWORD /d 0 /f | Out-Null
reg add HKLM\\SOFTWARE\\Policies\\Microsoft\\Windows\\LanmanWorkstation /v AllowInsecureGuestAuth /t REG_DWORD /d 1 /f | Out-Null
function Passo(\$texto) {
  \$linha = "\$(Get-Date -Format s) \$texto"
  Add-Content -Path C:\\OEM\\progresso.txt -Value \$linha
  try { Copy-Item C:\\OEM\\progresso.txt \\\\host.lan\\Data\\progresso.txt -Force } catch {}
}
try {
  Passo 'Baixando o Visual Studio 2026 Community'
  Invoke-WebRequest https://aka.ms/vs/stable/vs_community.exe -OutFile C:\\OEM\\vs_community.exe
  Passo 'Instalando o Visual Studio (C++ e ATL/MFC)'
  \$p = Start-Process C:\\OEM\\vs_community.exe -Wait -PassThru -ArgumentList '--quiet --wait --norestart --nocache --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.VC.ATLMFC --includeRecommended'
  Passo "Visual Studio: código \$(\$p.ExitCode)"
  Passo 'Baixando o Windows SDK $SDK_VERSAO'
  Invoke-WebRequest '$SDK_URL' -OutFile C:\\OEM\\winsdksetup.exe
  \$p = Start-Process C:\\OEM\\winsdksetup.exe -Wait -PassThru -ArgumentList '/features + /q /norestart /ceip off'
  Passo "SDK: código \$(\$p.ExitCode)"
  Passo 'Instalando o Python'
  Invoke-WebRequest https://www.python.org/ftp/python/3.12.10/python-3.12.10-amd64.exe -OutFile C:\\OEM\\python.exe
  Start-Process C:\\OEM\\python.exe -Wait -ArgumentList '/quiet InstallAllUsers=1 PrependPath=0 Include_test=0 TargetDir=C:\\Python312'
  Passo 'Empacotando (package_from_installed.py)'
  New-Item -ItemType Directory -Force C:\\OEM\\pacote | Out-Null
  Set-Location C:\\OEM\\pacote
  & C:\\Python312\\python.exe C:\\OEM\\package_from_installed.py 2026 -w $SDK_VERSAO --noarm *> C:\\OEM\\pacote.log
  Copy-Item C:\\OEM\\pacote.log \\\\host.lan\\Data\\pacote.log -Force
  \$zip = Get-ChildItem C:\\OEM\\pacote\\*.zip | Select-Object -First 1
  Passo "Copiando \$(\$zip.Name) (\$([math]::Round(\$zip.Length / 1GB, 2)) GB)"
  Copy-Item \$zip.FullName \\\\host.lan\\Data\\ -Force
  Passo "PRONTO \$(\$zip.Name)"
} catch {
  Passo "ERRO: \$(\$_.Exception.Message)"
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
        -e VERSION=11 -e RAM_SIZE="$RAM" -e CPU_CORES="$NUCLEOS" -e DISK_SIZE=110G \
        -e USERNAME=dev -e PASSWORD=dev \
        -p 127.0.0.1:8007:8006 \
        --device=/dev/kvm --device=/dev/net/tun --cap-add NET_ADMIN \
        -v "$BASE/disco:/storage" -v "$BASE/oem:/oem" -v "$BASE/compartilhado:/data" \
        --stop-timeout 120 \
        dockurr/windows
    fi
    echo "Tela: http://127.0.0.1:8007 — progresso em $BASE/compartilhado/progresso.txt"
    ;;
  status)
    docker ps -a --filter "name=$NOME" --format '{{.Names}}: {{.Status}}'
    cat "$BASE/compartilhado/progresso.txt" 2>/dev/null || true
    ;;
  apagar)
    docker rm -f "$NOME" >/dev/null 2>&1 || true
    rm -rf "$BASE/disco"
    ;;
  *)
    echo "uso: $0 subir|status|apagar" >&2
    exit 1
    ;;
esac
