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

exec /usr/lib/navegador/navegador "${SISTEMA[@]}" --lang="$IDIOMA" \
  "${FLAGS[@]}" "$@"
