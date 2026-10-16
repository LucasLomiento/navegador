#!/bin/bash
# Compila e empacota o navegador pro Windows (compilação cruzada):
#
#   tools/windows.sh gerar      -> gn gen out/win (tools/args-win.gn)
#   tools/windows.sh compilar   -> chrome e o instalador (mini_installer)
#   tools/windows.sh pacote     -> pacote/navegador-<versão>-windows-x64.exe
#
# Precisa das ferramentas da Microsoft empacotadas
# (tools/vm-ferramentas-windows.sh): o .zip em
# ~/.local/share/navegador/vm-ferramentas/compartilhado, que o gclient usa
# pelas variáveis abaixo.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${NAVEGADOR_SRC:-$HOME/Work/navegador-chromium/src}"
OUT="$SRC/out/win"
FERRAMENTAS="$HOME/.local/share/navegador/vm-ferramentas/compartilhado"
export PATH="$HOME/.local/share/navegador/depot_tools:$PATH"
export DEPOT_TOOLS_WIN_TOOLCHAIN_BASE_URL="$FERRAMENTAS"
ZIP=$(ls "$FERRAMENTAS"/*.zip 2>/dev/null | head -1)
if [ -n "$ZIP" ]; then
  # O hash que o Chromium espera (build/vs_toolchain.py) aponta pro nosso.
  ESPERADO=$(sed -n "s/^TOOLCHAIN_HASH = '\(.*\)'/\1/p" "$SRC/build/vs_toolchain.py")
  export "GYP_MSVS_HASH_$ESPERADO=$(basename "$ZIP" .zip)"
fi

VERSAO_PACOTE() {
  local chromium pkgrel
  chromium=$(cat "$RAIZ/CHROMIUM_VERSION")
  pkgrel=$(sed -n 's/^pkgrel=//p' "$RAIZ/pacote/PKGBUILD")
  echo "$chromium-$pkgrel"
}

case "${1:-}" in
  ferramentas)
    # Põe as ferramentas da Microsoft no lugar (o hook win_toolchain).
    cd "$SRC/.." && nice -n 19 gclient runhooks
    ;;
  gerar)
    mkdir -p "$OUT"
    cp "$RAIZ/tools/args-win.gn" "$OUT/args.gn"
    cd "$SRC" && gn gen "$OUT"
    ;;
  compilar)
    # O instalador leva, além do Chromium, a versão, as regras do bloqueador
    # e as fontes (chrome/installer/mini_installer/chrome.release).
    VERSAO_PACOTE > "$OUT/VERSAO"
    rm -rf "$OUT/bloqueador" "$OUT/fontes"
    mkdir -p "$OUT/bloqueador" "$OUT/fontes"
    cp "$RAIZ/pacote/bloqueador/"* "$OUT/bloqueador/"
    cp "$RAIZ/navegador/app/fontes/"*.ttf "$OUT/fontes/"
    JOBS="${JOBS:-10}" MEMORIA="${MEMORIA:-16G}" \
      "$RAIZ/tools/compilar.sh" out/win chrome mini_installer
    ;;
  pacote)
    destino="$RAIZ/pacote/navegador-$(VERSAO_PACOTE)-windows-x64.exe"
    cp "$OUT/mini_installer.exe" "$destino"
    echo "$destino"
    ;;
  *)
    echo "uso: $0 ferramentas|gerar|compilar|pacote" >&2
    exit 1
    ;;
esac
