#!/bin/bash
# Máquina virtual com Windows 11 só pra rodar o Dia e comparar o visual.
# Usa o dockurr/windows (QEMU num container: baixa a ISO da Microsoft e
# instala sozinho). Tela pelo navegador em http://127.0.0.1:8006.
#
#   tools/vm-windows.sh subir    -> cria/liga (a primeira vez instala o Windows, ~20-40 min)
#   RAM=4G NUCLEOS=2 tools/vm-windows.sh subir  -> mais leve (ex.: com compilação rodando)
#   tools/vm-windows.sh recriar  -> recria o container (pra trocar RAM/núcleos; o disco fica)
#   tools/vm-windows.sh parar    -> desliga (o disco fica em ~/.local/share/navegador/vm)
#   tools/vm-windows.sh status
#
# O Dia entra provisionado pra todos os usuários pelo oem/install.bat, a
# partir do MSIX oficial (o mesmo da referência). Pasta compartilhada:
# vm/compartilhado aqui = \\host.lan\Data no Windows (capturas de tela).
#
# Não rodar junto com a compilação do Chromium: são 8 GB de RAM.
set -euo pipefail

# Sessão aberta antes de entrar no grupo docker: entra nele com newgrp (não
# precisa relogar; o sg não existe nesta máquina).
if ! docker info >/dev/null 2>&1 && getent group docker | grep -qw "$USER"; then
  exec newgrp docker <<< "$(printf '%q ' "$0" "$@")"
fi

NOME=navegador-windows
RAM="${RAM:-8G}"
NUCLEOS="${NUCLEOS:-4}"
BASE="$HOME/.local/share/navegador/vm"
REF="$HOME/.local/share/navegador/referencia/download"

preparar_oem() {
  mkdir -p "$BASE/disco" "$BASE/oem" "$BASE/compartilhado"
  # btrfs: sem copy-on-write no disco virtual (o dockur avisa que a
  # instalação do Windows pode falhar com CoW).
  chattr +C "$BASE/disco" 2>/dev/null || true
  cp -u "$REF/Dia.x64.msix" "$BASE/oem/Dia.x64.msix"
  if [ ! -f "$BASE/oem/VCLibs.appx" ]; then
    curl -fsSL -o "$BASE/oem/VCLibs.appx" \
      https://releases.diabrowser.com/windows/dependencies/x64/Microsoft.VCLibs.x64.14.00.Desktop.14.0.33728.0.appx
  fi
  cat > "$BASE/oem/install.bat" <<'BAT'
@echo off
rem Roda no fim da instalação do Windows (como SYSTEM): provisiona o Dia pra todos os usuários.
powershell -NoProfile -ExecutionPolicy Bypass -Command "Add-AppxProvisionedPackage -Online -PackagePath C:\OEM\Dia.x64.msix -DependencyPackagePath C:\OEM\VCLibs.appx -SkipLicense" > C:\OEM\install-dia.log 2>&1
BAT
}

case "${1:-status}" in
  subir)
    if docker container inspect "$NOME" >/dev/null 2>&1; then
      docker start "$NOME"
    else
      preparar_oem
      docker run -d --name "$NOME" \
        -e VERSION=11 -e RAM_SIZE="$RAM" -e CPU_CORES="$NUCLEOS" -e DISK_SIZE=64G \
        -e USERNAME=dia -e PASSWORD=dia \
        -p 127.0.0.1:8006:8006 -p 127.0.0.1:3389:3389/tcp -p 127.0.0.1:3389:3389/udp \
        --device=/dev/kvm --device=/dev/net/tun --cap-add NET_ADMIN \
        -v "$BASE/disco:/storage" -v "$BASE/oem:/oem" -v "$BASE/compartilhado:/data" \
        --stop-timeout 120 \
        dockurr/windows
    fi
    echo "Tela: http://127.0.0.1:8006"
    ;;
  parar)
    docker stop "$NOME"
    ;;
  recriar)
    docker rm -f "$NOME" >/dev/null 2>&1 || true
    exec "$0" subir
    ;;
  status)
    docker ps -a --filter "name=$NOME" --format '{{.Names}}: {{.Status}}'
    ;;
  *)
    echo "uso: $0 subir|parar|recriar|status" >&2
    exit 1
    ;;
esac
