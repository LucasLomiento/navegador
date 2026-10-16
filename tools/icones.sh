#!/bin/bash
# Gera os ícones do navegador (navegador/app/icone.svg, só a agulha, sem
# fundo) nos tamanhos que o Chromium usa e põe no lugar dos do Chromium. Roda
# pelo tools/fonte.sh. O icone-fundo.svg (quadrado grafite) fica pro sistema
# que exigir fundo; o Linux não exige.
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
# As de 100% e 200% (páginas internas: chrome://theme/current-channel-logo).
for escala in 1 2; do
  for dir in "$SRC/chrome/app/theme/default_${escala}00_percent/chromium" \
             "$SRC/chrome/app/theme/default_${escala}00_percent/chromium/linux"; do
    for tam in 16 32; do
      if [ -f "$dir/product_logo_$tam.png" ]; then
        rsvg-convert -w $((tam * escala)) -h $((tam * escala)) "$SVG" -o "$TMP"
        por "$dir/product_logo_$tam.png"
      fi
    done
  done
done
# Favicon da nova guia: a agulha em cinza (no Dia, a silhueta do logo).
MONO="$RAIZ/navegador/app/icone-mono.svg"
rsvg-convert -w 16 -h 16 "$MONO" -o "$TMP"
por "$SRC/chrome/app/theme/default_100_percent/common/favicon_ntp.png"
rsvg-convert -w 32 -h 32 "$MONO" -o "$TMP"
por "$SRC/chrome/app/theme/default_200_percent/common/favicon_ntp.png"
# Logo das páginas internas no escuro (Configurações, Histórico...).
cp "$SVG" "$TMP"
por "$SRC/ui/webui/resources/images/chrome_logo_dark.svg"
rsvg-convert -w 32 -h 32 "$SVG" | magick png:- "xpm:$TMP"
por "$TEMA/linux/product_logo_32.xpm"
cp "$SVG" "$TMP"
por "$TEMA/product_logo.svg"
# Ícones do Lucide no lugar dos do Chromium (navegador/icones/LUGARES; os
# .icon saem de tools/lucide.py).
while read -r nome destino; do
  case "$nome" in ''|\#*) continue ;; esac
  cp "$RAIZ/navegador/icones/$nome.icon" "$TMP"
  por "$SRC/$destino"
done < "$RAIZ/navegador/icones/LUGARES"
echo "icones: ok"
