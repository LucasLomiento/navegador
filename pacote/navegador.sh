#!/bin/bash
# Launcher do navegador. Flags extras do usuário (uma por linha, # comenta)
# em ~/.config/navegador-flags.conf, como o chromium do Arch faz.
XDG_CONFIG_HOME="${XDG_CONFIG_HOME:-$HOME/.config}"
FLAGS=()
if [[ -f "$XDG_CONFIG_HOME/navegador-flags.conf" ]]; then
  while read -r linha; do
    [[ -z "$linha" || "$linha" = \#* ]] && continue
    FLAGS+=($linha)
  done < "$XDG_CONFIG_HOME/navegador-flags.conf"
fi
# Interface em português (os textos próprios do navegador são em português).
# Pra trocar: NAVEGADOR_IDIOMA=en-US no ambiente.
IDIOMA="${NAVEGADOR_IDIOMA:-pt-BR}"
export LANGUAGE="${IDIOMA/-/_}"
exec /usr/lib/navegador/navegador --ozone-platform-hint=auto --lang="$IDIOMA" \
  "${FLAGS[@]}" "$@"
