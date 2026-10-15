#!/bin/bash
# Listas do bloqueador de anúncios e rastreadores: baixa a EasyList, a
# EasyPrivacy e a EasyList Portuguese, converte as regras de rede pro formato
# do subresource filter do Chromium e separa as de esconder elementos
# (cosmetico.txt, que o navegador injeta como CSS).
#
#   tools/listas.sh [saida]   (padrão: pacote/bloqueador)
#
# Gera "Filtering Rules" (as regras de rede), cosmetico.txt, LICENSE (de onde
# vieram) e VERSAO (a data, que o navegador usa pra saber se precisa
# reindexar). Precisa do
# ruleset_converter compilado:
#   tools/compilar.sh out/release components/subresource_filter/tools:ruleset_converter
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${SRC:-$HOME/Work/navegador-chromium/src}"
CONVERSOR="$SRC/out/release/ruleset_converter"
SAIDA="${1:-$RAIZ/pacote/bloqueador}"

LISTAS=(
  "https://easylist.to/easylist/easylist.txt"
  "https://easylist.to/easylist/easyprivacy.txt"
  "https://easylist-downloads.adblockplus.org/easylistportuguese.txt"
  # Consertos do uBlock Origin pra sites que quebram com as listas acima
  # (exceções de rede e scriptlets "set", que o navegador aplica).
  "https://ublockorigin.github.io/uAssets/filters/unbreak.txt"
)

[ -x "$CONVERSOR" ] || { echo "Falta o $CONVERSOR (ver o cabeçalho)." >&2; exit 1; }

BAIXADAS=$(mktemp -d)
trap 'rm -rf "$BAIXADAS"' EXIT
ARQUIVOS=()
for url in "${LISTAS[@]}"; do
  destino="$BAIXADAS/$(basename "$url")"
  curl -fsSL --retry 3 -o "$destino" "$url"
  # Lista de filtros de verdade começa com "[Adblock Plus" ou "! Title:".
  head -c 300 "$destino" | grep -q "^\[Adblock\|^! Title:" || {
    echo "Não parece uma lista de filtros: $url" >&2; exit 1; }
  ARQUIVOS+=("$destino")
done
# A EasyPrivacy quase não tem regras cosméticas; as outras, sim.
ARQUIVOS_COSMETICOS=("${ARQUIVOS[0]}" "${ARQUIVOS[2]}" "${ARQUIVOS[3]}")

mkdir -p "$SAIDA"
IFS=,
"$CONVERSOR" --input_format=filter-list --output_format=unindexed-ruleset \
  --input_files="${ARQUIVOS[*]}" --output_file="$SAIDA/Filtering Rules"
unset IFS

# Regras de esconder elementos (cosméticas), que o filtro do Chromium não
# aplica: o navegador injeta como CSS (bloqueador.cc). Formato, uma por
# linha: "G<tab>seletor" (todo site), "S<tab>dominios<tab>seletor" (só
# nesses sites; "~dominio" exclui) e "E<tab>dominios<tab>seletor" (exceção:
# não esconder nesses sites).
python3 -I "$RAIZ/tools/cosmetico.py" "${ARQUIVOS_COSMETICOS[@]}" > "$SAIDA/cosmetico.txt"

date -u +%Y.%m.%d.%H%M > "$SAIDA/VERSAO"
{
  echo "Regras do bloqueador do Navegador, convertidas de:"
  echo
  for url in "${LISTAS[@]}"; do echo "  $url"; done
  echo
  echo "EasyList, EasyPrivacy e EasyList Portuguese: (c) dos autores da"
  echo "EasyList (https://easylist.to), sob a GNU GPL v3 ou a Creative"
  echo "Commons Atribuição-CompartilhaIgual 3.0 (à escolha)."
  echo "https://easylist.to/pages/licence.html"
  echo
  echo "uBlock filters – Unbreak: (c) dos autores do uBlock Origin"
  echo "(https://github.com/uBlockOrigin/uAssets), sob a GNU GPL v3."
} > "$SAIDA/LICENSE"

echo "Regras em $SAIDA ($(du -h "$SAIDA/Filtering Rules" | cut -f1) de rede, $(wc -l < "$SAIDA/cosmetico.txt") cosméticas, versão $(cat "$SAIDA/VERSAO"))."
