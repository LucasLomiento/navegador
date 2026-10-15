#!/bin/bash
# Abre o navegador compilado com um perfil de teste, numa janela flutuante
# de 1440x900 no centro da tela, e tira uma captura da janela.
#
#   tools/testar.sh abrir [url...]   -> abre (perfil em /tmp, sobra entre execuções)
#   tools/testar.sh foto [arquivo]   -> captura da janela de teste (padrão: /tmp/navegador-teste.png)
#   tools/testar.sh fechar           -> fecha só o processo que este script abriu
#   tools/testar.sh limpar           -> fecha e apaga o perfil de teste
#   tools/testar.sh url <endereço>   -> abre o endereço numa aba nova (DevTools na 9333)
#   tools/testar.sh abas             -> lista as abas
#   tools/testar.sh teclas <wtype args...>  -> foca a janela de teste e manda
#        teclas com o wtype; não manda nada se a janela ativa não for a de teste
#        (ex.: tools/testar.sh teclas -M ctrl -k t -m ctrl)
#
# A janela tem app_id "navegador-teste" (CHROME_DESKTOP), pra não pegar as
# regras do navegador de verdade. Variáveis: OUT (padrão out/dev),
# PERFIL (padrão /tmp/navegador-teste-perfil), ESCALA (padrão 1).
set -euo pipefail
SRC="${NAVEGADOR_SRC:-$HOME/Work/navegador-chromium/src}"
OUT="${OUT:-out/dev}"
PERFIL="${PERFIL:-/tmp/navegador-teste-perfil}"
PIDFILE=/tmp/navegador-teste.pid
CLASSE=navegador-teste

janela() {
  hyprctl clients -j | python3 -c '
import json, sys
for c in json.load(sys.stdin):
    if c.get("class") == sys.argv[1]:
        x, y = c["at"]; w, h = c["size"]
        print(f"{x},{y} {w}x{h}")
        break
' "$CLASSE"
}

case "${1:-abrir}" in
  abrir)
    shift || true
    # Flutuante e centralizada (regra vale até o próximo reload do Hyprland).
    hyprctl eval "hl.window_rule({ match = { class = \"$CLASSE\" }, float = true, center = true, size = { 1440, 900 } })" >/dev/null
    mkdir -p "$PERFIL"
    CHROME_DESKTOP="$CLASSE.desktop" LANGUAGE=pt_BR setsid "$SRC/$OUT/chrome" \
      --user-data-dir="$PERFIL" --ozone-platform=wayland --lang=pt-BR \
      --no-first-run --no-default-browser-check \
      --remote-debugging-address=127.0.0.1 --remote-debugging-port=9333 \
      --force-device-scale-factor="${ESCALA:-1}" \
      "$@" >/tmp/navegador-teste.log 2>&1 &
    echo $! > "$PIDFILE"
    for _ in $(seq 60); do
      [ -n "$(janela)" ] && break
      sleep 0.5
    done
    if [ -z "$(janela)" ]; then
      echo "janela não apareceu; log em /tmp/navegador-teste.log" >&2
      tail -20 /tmp/navegador-teste.log >&2
      exit 1
    fi
    echo "aberto (pid $(cat "$PIDFILE")): $(janela)"
    ;;
  foto)
    geo="$(janela)"
    [ -n "$geo" ] || { echo "sem janela de teste aberta" >&2; exit 1; }
    grim -g "$geo" "${2:-/tmp/navegador-teste.png}"
    echo "${2:-/tmp/navegador-teste.png}"
    ;;
  fechar)
    if [ -f "$PIDFILE" ]; then
      kill "$(cat "$PIDFILE")" 2>/dev/null || true
      rm -f "$PIDFILE"
    fi
    ;;
  teclas)
    shift
    endereco="$(hyprctl clients -j | python3 -c '
import json, sys
for c in json.load(sys.stdin):
    if c.get("class") == sys.argv[1]:
        print(c["address"]); break' "$CLASSE")"
    [ -n "$endereco" ] || { echo "sem janela de teste" >&2; exit 1; }
    hyprctl dispatch "hl.dsp.focus({ window = \"address:$endereco\" })" >/dev/null
    sleep 0.3
    ativa="$(hyprctl activewindow -j | python3 -c 'import json,sys; print(json.load(sys.stdin).get("class",""))')"
    if [ "$ativa" != "$CLASSE" ]; then
      echo "janela ativa é \"$ativa\", não a de teste: não mandei nada" >&2
      exit 1
    fi
    wtype "$@"
    ;;
  url)
    curl -fsS -X PUT "http://127.0.0.1:9333/json/new?$2" >/dev/null && echo "aberto: $2"
    ;;
  abas)
    curl -fsS http://127.0.0.1:9333/json/list | python3 -c '
import json, sys
for t in json.load(sys.stdin):
    if t["type"] == "page":
        print(t["id"], t["url"][:80], "|", t["title"][:50])'
    ;;
  limpar)
    "$0" fechar
    sleep 1
    rm -rf "$PERFIL"
    ;;
  *)
    echo "uso: $0 abrir [url...] | foto [arquivo] | fechar | limpar" >&2
    exit 1
    ;;
esac
