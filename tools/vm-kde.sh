#!/bin/bash
# Máquina virtual com o KDE Plasma (CachyOS live, sem instalar) pra testar o
# Norte fora do Hyprland. Usa o qemux/qemu (QEMU num container). Tela pelo
# navegador em http://127.0.0.1:8006 ou por VNC em 127.0.0.1:5900.
#
#   tools/vm-kde.sh subir    -> baixa o ISO (a primeira vez, ~3 GB) e liga
#   tools/vm-kde.sh parar    -> desliga e apaga o container (live: nada se perde
#                              além do que foi feito dentro da sessão)
#   tools/vm-kde.sh status
#
# Pasta compartilhada: vm-kde/compartilhado aqui = /mnt/host na VM, depois de
#   sudo mkdir -p /mnt/host && sudo mount -t 9p -o trans=virtio shared /mnt/host
# O subir copia pra lá o pacote mais novo de pacote/.
#
# Não rodar junto com a compilação do Chromium: são 6 GB de RAM.
set -euo pipefail

if ! docker info >/dev/null 2>&1 && getent group docker | grep -qw "$USER"; then
  exec newgrp docker <<< "$(printf '%q ' "$0" "$@")"
fi

NOME=norte-kde
RAM="${RAM:-6G}"
NUCLEOS="${NUCLEOS:-4}"
BASE="$HOME/.local/share/navegador/vm-kde"
REPO="$(cd "$(dirname "$0")/.." && pwd)"

baixar_iso() {
  [ -f "$BASE/cachyos.iso" ] && return
  mkdir -p "$BASE"
  # O mesmo caminho que o qemux usa pro BOOT=cachy: o ISO desktop (KDE) mais novo.
  local url
  url=$(curl -fsSL https://cachyos.org/download/ | tr '&' '\n' |
    grep "ISO/desktop" | grep -v 'iso.sha\|iso.sig' | cut -d';' -f2 | head -1)
  curl -fL --retry 3 -o "$BASE/cachyos.iso.part" "$url"
  curl -fsSL "$url.sha256" | awk '{print $1 "  cachyos.iso.part"}' |
    (cd "$BASE" && sha256sum -c -)
  mv "$BASE/cachyos.iso.part" "$BASE/cachyos.iso"
}

case "${1:-status}" in
  subir)
    baixar_iso
    mkdir -p "$BASE/compartilhado" "$BASE/disco"
    chattr +C "$BASE/disco" 2>/dev/null || true
    pacote=$(ls -t "$REPO"/pacote/*.pkg.tar.zst 2>/dev/null | head -1)
    if [ -n "$pacote" ]; then
      rm -f "$BASE"/compartilhado/*.pkg.tar.zst
      cp "$pacote" "$BASE/compartilhado/"
    fi
    docker rm -f "$NOME" >/dev/null 2>&1 || true
    docker run -d --name "$NOME" \
      -e RAM_SIZE="$RAM" -e CPU_CORES="$NUCLEOS" -e DISK_SIZE=16G \
      -p 127.0.0.1:8006:8006 -p 127.0.0.1:5900:5900 \
      --device=/dev/kvm --device=/dev/net/tun --cap-add NET_ADMIN \
      -v "$BASE/cachyos.iso:/boot.iso" -v "$BASE/disco:/storage" \
      -v "$BASE/compartilhado:/shared" \
      --stop-timeout 30 \
      qemux/qemu
    echo "Tela: http://127.0.0.1:8006 (VNC: 127.0.0.1:5900)"
    ;;
  parar)
    docker rm -f "$NOME"
    ;;
  status)
    docker ps -a --filter "name=$NOME" --format '{{.Names}}: {{.Status}}'
    ;;
  *)
    echo "uso: $0 subir|parar|status" >&2
    exit 1
    ;;
esac
