#!/bin/bash
# Prepara a árvore do Chromium com as nossas mudanças:
#   1. liga navegador/ dentro dela como src/navegador
#   2. aplica patches/*.patch em ordem (pula os que já estão aplicados)
#   3. troca "Chromium" por "Navegador" nos textos (tools/marca.py)
#   4. põe os ícones do navegador (tools/icones.sh)
#
#   tools/fonte.sh            -> prepara
#   tools/fonte.sh --gerar    -> regrava patches/ a partir do que mudou no src
#                                (um patch por área, ver patches/SERIES)
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${NAVEGADOR_SRC:-$HOME/Work/navegador-chromium/src}"

if [ ! -d "$SRC/.git" ]; then
  echo "Sem árvore do Chromium em $SRC (rode ~/Work/navegador-chromium/sync.sh)" >&2
  exit 1
fi

ln -sfn "$RAIZ/navegador" "$SRC/navegador"

if [ "${1:-}" = "--gerar" ]; then
  # Cada linha de patches/SERIES: <nome>.patch <caminhos...>
  # O que mudou no src e não está em nenhuma linha vai pra 9999-resto.patch.
  cd "$SRC"
  usados=()
  while read -r nome caminhos; do
    [ -z "$nome" ] || [[ "$nome" = \#* ]] && continue
    # shellcheck disable=SC2086
    git diff --binary -- $caminhos > "$RAIZ/patches/$nome"
    [ -s "$RAIZ/patches/$nome" ] || rm -f "$RAIZ/patches/$nome"
    # shellcheck disable=SC2206
    usados+=($caminhos)
  done < "$RAIZ/patches/SERIES"
  excl=()
  for c in "${usados[@]}"; do excl+=(":(exclude)$c"); done
  # Os textos com o nome do produto são gerados pelo tools/marca.py.
  git diff --binary -- . ':(exclude)navegador' \
    ':(exclude)chrome/app/chromium_strings.grd' \
    ':(exclude)chrome/app/resources/chromium_strings_*.xtb' \
    ':(exclude)chrome/app/generated_resources.grd' \
    ':(exclude)chrome/app/resources/generated_resources_*.xtb' \
    ':(exclude)chrome/app/theme/chromium/*product_logo*' \
    ':(exclude)chrome/app/theme/chromium/linux/*product_logo*' \
    "${excl[@]}" > "$RAIZ/patches/9999-resto.patch"
  [ -s "$RAIZ/patches/9999-resto.patch" ] || rm -f "$RAIZ/patches/9999-resto.patch"
  ls -1 "$RAIZ/patches"
  exit 0
fi

cd "$SRC"
for p in "$RAIZ"/patches/*.patch; do
  [ -e "$p" ] || continue
  if git apply --check "$p" 2>/dev/null; then
    git apply "$p" && echo "aplicado: $(basename "$p")"
  elif git apply --check --reverse "$p" 2>/dev/null; then
    echo "já estava: $(basename "$p")"
  else
    echo "CONFLITO: $(basename "$p")" >&2
    exit 1
  fi
done

python3 -I "$RAIZ/tools/marca.py" --src "$SRC"
"$RAIZ/tools/icones.sh"
