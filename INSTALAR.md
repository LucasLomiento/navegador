# Instalar o navegador

O pacote é montado a partir da compilação já feita: o `out/release`
(binário único) se existir, senão o `out/dev`. Os dois têm a mesma
otimização (`is_debug = false`, sem DCHECK, mesmos codecs e Widevine); o
`out/dev` só vem dividido em bibliotecas e abre a janela em ~0,8 s. Ver
[docs/ARQUITETURA.md](docs/ARQUITETURA.md).

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
- Flags extras (uma por linha): `~/.config/navegador-flags.conf`. O launcher
  já liga o teclado japonês no Wayland (`--enable-wayland-ime`), as senhas
  no chaveiro do GNOME, o gesto de voltar no touchpad e as extensões do
  Omarchy (Alt+Shift+L copia o endereço, Alt+Shift+D baixa o vídeo). Um
  `--enable-features` no arquivo substitui o do launcher: repetir
  `TouchpadOverscrollHistoryNavigation` nele.
- Políticas (se um dia precisar): `/etc/navegador/policies/managed/*.json`.
- Idioma: português por padrão; `NAVEGADOR_IDIOMA=en-US navegador` troca.

## Menu e perfis

- O menu do navegador fica no logo, no canto de cima da lateral (como no
  Dia); `Alt+F` também abre. Clique direito no logo: recolher/abrir a lateral
  (ou `Ctrl+Shift+L`).
- Lateral recolhida some inteira; encostar o mouse na borda esquerda faz ela
  aparecer por cima da página (como a lateral flutuante do Dia). Clicar no
  logo nela fixa a lateral aberta de novo.
- Perfis: menu do logo → o submenu com o nome do perfil → outros perfis,
  "Adicionar novo perfil" (escolhe nome e cor; a cor tinge a lateral),
  visitante e "Gerenciar perfis". Cada perfil abre na própria janela; ao
  abrir o navegador, volta o último perfil usado.
- Containers: menu do logo → Containers → nova aba num container ou
  "Gerenciar containers…" (criar, renomear, trocar a cor, excluir).

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

O DRM (Widevine) é baixado sozinho cerca de um minuto depois da primeira
abertura, e o Chromium no Linux só carrega ele na abertura seguinte: na
primeira vez, abrir o navegador, esperar um minuto, fechar e abrir de novo
(testado no pacote em 15/10: depois de reabrir, o Widevine responde). No
Linux o Netflix limita a resolução em qualquer navegador.
