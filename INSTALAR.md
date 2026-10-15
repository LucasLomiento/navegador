# Instalar o navegador

O pacote é montado a partir da compilação já feita: o `out/release`
(binário único, abre a janela em ~0,4 s) se existir, senão o `out/dev`
(mesma otimização, só dividido em bibliotecas, ~0,8 s). Ver
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

## Espaços, fixados e configurações

- **Configurações do Navegador**: `Ctrl+,` ou menu do logo → "Configurações
  do Navegador…". Abas: Espaços (nome, emoji, cor, container de cada espaço,
  ordem, excluir, novo), Containers e Atalhos (clicar no atalho e apertar as
  teclas novas; Esc cancela, Backspace tira). As configurações do Chromium
  continuam no menu do logo → Configurações.
- **Espaços** (como os workspaces do Zen): cada janela está num espaço e mostra
  só as abas dele; aba nova nasce no container do espaço. Trocar: nome do
  espaço no alto da lateral (como o seletor de perfis do Dia), botões
  laterais do mouse (voltar/avançar) com o ponteiro em cima da lateral,
  rolagem de lado ou `Ctrl`+rodinha na lateral, as bolinhas do pé ou os
  atalhos. Menu da aba → "Mover para o espaço".
- **Favoritos, sites fixados e pastas** são do espaço (iguais em toda janela
  nele, e ficam ao fechar o navegador). Menu da aba → "Adicionar aos
  favoritos" (ladrilho no alto) ou "Fixar no espaço" (fora das pastas, numa
  pasta ou numa pasta nova). Clicar abre o site; o "x" fecha só a aba.
  Clique direito num site ou pasta: renomear, usar o endereço atual, mover,
  desafixar, excluir a pasta.

## Atalhos próprios (padrão; mudam nas Configurações)

- `Ctrl+Shift+S`: mostrar/ocultar a lateral.
- `Ctrl+Alt+→` / `Ctrl+Alt+←`: próximo / anterior espaço.
- `Ctrl+Alt+1` … `Ctrl+Alt+5`: ir pro espaço 1 a 5.
- `Ctrl+Alt+N`: novo espaço.
- `Ctrl+,`: Configurações do Navegador.
- Clique direito na aba: "Abrir em container"; num link: "Abrir link em
  container"; no "+ Nova aba": "Nova aba em container".
- A cor da lateral: menu do logo → Mais ferramentas → Personalizar (ou
  Configurações → Aparência); dentro de um espaço, a lateral pega a cor dele.

## Netflix

O DRM (Widevine) é baixado sozinho cerca de um minuto depois da primeira
abertura, e o Chromium no Linux só carrega ele na abertura seguinte: na
primeira vez, abrir o navegador, esperar um minuto, fechar e abrir de novo
(testado no pacote em 15/10: depois de reabrir, o Widevine responde). No
Linux o Netflix limita a resolução em qualquer navegador.
