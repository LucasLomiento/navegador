#!/bin/bash
# Lançador do Norte. Flags extras do usuário (uma por linha, # comenta) em
# ~/.config/norte-flags.conf, como o chromium do Arch faz (o
# navegador-flags.conf, do nome antigo, ainda vale).
XDG_CONFIG_HOME="${XDG_CONFIG_HOME:-$HOME/.config}"
FLAGS=()
for arquivo in "$XDG_CONFIG_HOME/norte-flags.conf" \
               "$XDG_CONFIG_HOME/navegador-flags.conf"; do
  [[ -f "$arquivo" ]] || continue
  while read -r linha; do
    [[ -z "$linha" || "$linha" = \#* ]] && continue
    FLAGS+=($linha)
  done < "$arquivo"
  break
done
# Interface em português (os textos próprios do Norte são em português).
# Pra trocar: NORTE_IDIOMA=en-US no ambiente.
IDIOMA="${NORTE_IDIOMA:-${NAVEGADOR_IDIOMA:-pt-BR}}"
export LANGUAGE="${IDIOMA/-/_}"

# Integração com o sistema (as mesmas do chromium-flags.conf do Omarchy):
# - método de entrada no Wayland (fcitx5: japonês com Mozc);
# - senhas no chaveiro do GNOME (sem isto, no Hyprland, iriam em texto puro);
# - voltar/avançar com dois dedos no touchpad.
SISTEMA=(
  --ozone-platform-hint=auto
  --enable-wayland-ime --wayland-text-input-version=3
  --password-store=gnome-libsecret
  --enable-features=TouchpadOverscrollHistoryNavigation
)
# Extensões que o Omarchy põe no Chromium (Alt+Shift+L copia o endereço,
# Alt+Shift+D baixa o vídeo, WhatsApp no tema do sistema), se existirem.
OMARCHY=/usr/share/omarchy/default/chromium/extensions
EXTENSOES=()
for e in copy-url yt-dlp whatsapp-slim; do
  [[ -f "$OMARCHY/$e/manifest.json" ]] && EXTENSOES+=("$OMARCHY/$e")
done
if ((${#EXTENSOES[@]})); then
  SISTEMA+=("--load-extension=$(IFS=,; echo "${EXTENSOES[*]}")")
fi

exec /usr/lib/norte/norte "${SISTEMA[@]}" --lang="$IDIOMA" \
  "${FLAGS[@]}" "$@"
