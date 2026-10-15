#!/bin/bash
# Gera os ícones do navegador (navegador/app/icone.svg) nos tamanhos que o
# Chromium usa e põe no lugar dos do Chromium. Roda pelo tools/fonte.sh.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${NAVEGADOR_SRC:-$HOME/Work/navegador-chromium/src}"
SVG="$RAIZ/navegador/app/icone.svg"
TEMA="$SRC/chrome/app/theme/chromium"
TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT

# Só regrava se mudou: mexer na data do arquivo recompila os recursos.
por() {
  cmp -s "$TMP" "$1" || cp "$TMP" "$1"
}

for tam in 16 24 48 64 128 256; do
  for dir in "$TEMA" "$TEMA/linux"; do
    if [ -f "$dir/product_logo_$tam.png" ]; then
      rsvg-convert -w "$tam" -h "$tam" "$SVG" -o "$TMP"
      por "$dir/product_logo_$tam.png"
    fi
  done
done
rsvg-convert -w 32 -h 32 "$SVG" | magick png:- "xpm:$TMP"
por "$TEMA/linux/product_logo_32.xpm"
cp "$SVG" "$TMP"
por "$TEMA/product_logo.svg"
echo "icones: ok"
