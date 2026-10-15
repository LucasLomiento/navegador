# Instalar o navegador

O pacote é montado a partir da compilação de release (`out/release`, ver
[docs/ARQUITETURA.md](docs/ARQUITETURA.md)). Com ela pronta:

```sh
cd ~/Work/github/pessoal/navegador/pacote
makepkg -f                                  # monta o navegador-*.pkg.tar.zst
sudo pacman -U navegador-*.pkg.tar.zst      # instala (pede senha)
```

Depois, `navegador` no terminal ou "Navegador" no menu de apps. Na primeira
vez aparece a tela de boas-vindas, onde dá pra deixar ele como navegador
padrão.

## Onde fica cada coisa

- Programa: `/usr/lib/navegador`; comando: `/usr/bin/navegador`.
- Perfil: `~/.config/navegador` (separado do Chromium e do Zen).
- Flags extras (uma por linha): `~/.config/navegador-flags.conf`.
- Políticas (se um dia precisar): `/etc/navegador/policies/managed/*.json`.
- Idioma: português por padrão; `NAVEGADOR_IDIOMA=en-US navegador` troca.

## Menu e perfis

- O menu do navegador fica no logo, no canto de cima da lateral (como no
  Dia); `Alt+F` também abre. Clique direito no logo: recolher/abrir a lateral
  (ou `Ctrl+Shift+L`).
- Perfis: menu do logo → o primeiro submenu (com o nome do perfil) → outros
  perfis, "Adicionar novo perfil" (escolhe nome e cor; a cor tinge a
  lateral), visitante e "Gerenciar perfis". Cada perfil abre na própria
  janela.

## Atalhos próprios

- `Ctrl+Alt+N`: novo espaço.
- `Ctrl+Alt+→` / `Ctrl+Alt+←`: próximo / anterior espaço (a rodinha do mouse
  em cima da faixa de espaços também troca).
- Clique direito na aba: "Abrir em container"; num link: "Abrir link em
  container"; no "+ Nova aba": "Nova aba em container". Em todos, "Gerenciar
  containers…".
- Clique direito na faixa de espaços (embaixo da lateral): editar o espaço,
  escolher o container padrão dele, novo espaço.
- A cor da lateral: menu do logo → Mais ferramentas → Personalizar (ou
  Configurações → Aparência).

## Netflix

O DRM (Widevine) é baixado sozinho na primeira vez que precisa (alguns
segundos depois de abrir). No Linux o Netflix limita a resolução em qualquer
navegador.
