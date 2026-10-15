#!/bin/bash
# Publica o pacote mais novo de pacote/ nas Releases públicas
# (LucasLomiento/navegador-releases): só os pacotes e o repositório do
# pacman; o código fica neste repositório, privado.
#
#   tools/publicar.sh "notas da versão (markdown)"
#
# Cada release leva o pacote e o banco do pacman (navegador.db), então o
# Server ".../releases/latest/download" do pacman.conf sempre aponta pra
# versão mais nova. O atualizador do navegador lê a mesma release pela API.
set -euo pipefail
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
REPO=LucasLomiento/navegador-releases
NOTAS="${1:-}"

PKG=$(ls -t "$RAIZ"/pacote/navegador-*-x86_64.pkg.tar.zst | head -1)
VERSAO=$(basename "$PKG" | sed -E 's/^navegador-(.*)-x86_64\.pkg\.tar\.zst$/\1/')
TAG="v$VERSAO"
echo "Pacote: $(basename "$PKG") -> $REPO $TAG"

if gh release view "$TAG" --repo "$REPO" >/dev/null 2>&1; then
  echo "A release $TAG já existe." >&2
  exit 1
fi

# O repositório (público) e o README dele.
TRAB=$(mktemp -d)
trap 'rm -rf "$TRAB"' EXIT
if ! gh repo view "$REPO" >/dev/null 2>&1; then
  gh repo create "$REPO" --public \
    --description "Navegador (Chromium com espaços, containers e lateral): pacotes pro Arch Linux"
fi
gh repo clone "$REPO" "$TRAB/repo" -- -q 2>/dev/null || {
  mkdir -p "$TRAB/repo"; git -C "$TRAB/repo" init -q -b main
  git -C "$TRAB/repo" remote add origin "https://github.com/$REPO.git"
}
cp "$RAIZ/pacote/releases/README.md" "$TRAB/repo/README.md"
if [[ -n "$(git -C "$TRAB/repo" status --porcelain)" ]]; then
  git -C "$TRAB/repo" add README.md
  git -C "$TRAB/repo" commit -q -m "README: como instalar e atualizar"
  git -C "$TRAB/repo" push -q -u origin HEAD:main
fi

# Repositório do pacman só com esta versão.
mkdir "$TRAB/pub"
cp "$PKG" "$TRAB/pub/"
(cd "$TRAB/pub" && repo-add -q navegador.db.tar.gz "$(basename "$PKG")" >/dev/null &&
  rm -f navegador.db navegador.files &&
  cp navegador.db.tar.gz navegador.db && cp navegador.files.tar.gz navegador.files)

[[ -z "$NOTAS" ]] && NOTAS="Navegador $VERSAO."
NOTAS="$NOTAS

**Instalar ou atualizar:** veja o [README](https://github.com/$REPO#readme) (repositório do pacman ou o arquivo .pkg.tar.zst abaixo)."
gh release create "$TAG" --repo "$REPO" --title "Navegador $VERSAO" \
  --notes "$NOTAS" --latest \
  "$TRAB/pub/$(basename "$PKG")" "$TRAB/pub/navegador.db" \
  "$TRAB/pub/navegador.files" "$TRAB/pub/navegador.db.tar.gz" \
  "$TRAB/pub/navegador.files.tar.gz"
echo "Publicado: https://github.com/$REPO/releases/tag/$TAG"
