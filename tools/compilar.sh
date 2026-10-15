#!/bin/bash
# Compila o navegador no menor peso possível pro resto do PC:
# prioridade ociosa de CPU e de disco, e poucos processos ao mesmo tempo
# (30 GB de RAM: cada compilação do Blink chega a 2 GB).
#   tools/compilar.sh            -> out/dev, alvo chrome
#   JOBS=12 tools/compilar.sh    -> mais rápido, mais pesado
#   tools/compilar.sh out/dev chrome chrome_sandbox
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${NAVEGADOR_SRC:-$HOME/Work/navegador-chromium/src}"
OUT="${1:-out/dev}"; shift || true
ALVOS=("${@:-chrome}")
JOBS="${JOBS:-8}"
export PATH="$HOME/.local/share/navegador/depot_tools:$PATH"
cd "$SRC"
if [ ! -f "$OUT/args.gn" ]; then
  mkdir -p "$OUT"
  cp "$RAIZ/tools/args-dev.gn" "$OUT/args.gn"
  gn gen "$OUT"
fi
exec chrt --idle 0 nice -n 19 ionice -c3 autoninja -j "$JOBS" -C "$OUT" "${ALVOS[@]}"
