# Arquitetura

O navegador é o Chromium com uma camada nossa por cima, no esquema do Brave: o código do Chromium fica intacto o máximo possível, e o que é nosso mora em `navegador/` (ligado dentro da árvore como `src/navegador`) e em `patches/` (o mínimo de mudanças em arquivos do Chromium).

## Base

- **Versão**: `CHROMIUM_VERSION` (tag estável do Chromium). Atualizar = trocar a versão, `tools/fonte.sh`, reaplicar os patches, compilar.
- **Compilação**: GN + autoninja do depot_tools, toolchain e sysroot do próprio Chromium (baixados pelo `gclient runhooks`). Args de desenvolvimento em `tools/args-dev.gn` (component build, sem símbolos). O pacote final usa uma compilação estática otimizada (args próprios no PKGBUILD).
- **DRM (Netflix)**: `enable_widevine = true`, `proprietary_codecs = true`, `ffmpeg_branding = "Chrome"`. O módulo Widevine não vem junto: o component updater baixa do Google na primeira vez (é o que o Chromium do Arch faz; ele fica em `~/.config/<produto>/WidevineCdm`). No Linux, o Netflix limita a resolução em qualquer navegador.
- **Sem chaves de API do Google**: sem sincronização com a conta Google e sem a barra de "chaves ausentes" (removida por patch).

## Interface (o design do Dia)

O Chrome 155 já tem abas verticais nativas (Views, C++): `chrome/browser/ui/views/frame/vertical_tab_strip_region_view.*`, `chrome/browser/ui/views/tabs/vertical/` (contêineres de cima e de baixo, barra de rolagem, abrir ao passar o mouse) e até um fundo desfocado (`vertical_tab_strip_background_blur_backdrop.*`). O plano é partir disso, não refazer do zero:

1. **Abas verticais ligadas por padrão e sem opção de voltar pra cima.**
2. **Visual do Dia por cima do nativo**: cores num mixer de cores nosso (`ColorProvider`), medidas e raios nas constantes de layout, desenho próprio onde o Chrome não deixa configurar.
3. **Partes que o Chrome não tem** (troca de espaços, seletor de container, cabeçalho do menu lateral do Dia) como Views nossas em `navegador/`, encaixadas por ganchos pequenos.

As medidas vêm das telas do Dia decodificadas (`docs/dia-medidas.md`) e, quando a máquina virtual com Windows estiver de pé, de capturas do Dia rodando, comparadas lado a lado com as nossas.

## Perfis

Perfis do Chromium como estão (cada um com cookies, histórico, extensões e janela próprios). Muda só a aparência do seletor, no estilo do Dia, no menu lateral.

## Containers

O Chromium não tem. A ideia: **cada container é uma `StoragePartition` própria dentro do mesmo perfil** (cookies, localStorage, IndexedDB, cache e service workers separados; histórico, favoritos e senhas continuam do perfil, como nos containers do Firefox). A aba de um container nasce com um `SiteInstance` de partição fixa (`SiteInstance::CreateForFixedStoragePartition`, o mesmo mecanismo das Isolated Web Apps e do `<webview>`).

Pontos a confirmar no código antes de construir:
- se a navegação para outro site, digitada na barra (troca de BrowsingInstance), mantém a partição;
- abrir link em nova aba (Ctrl+clique, `noopener`) tem que herdar o container: gancho na criação da aba;
- restaurar sessão: guardar o container de cada aba e recriar com a mesma partição.

Os containers aparecem como cor e ícone na aba, com "Abrir em container" no menu de contexto e uma tela pra gerenciar.

## Espaços (do Dia)

Os espaços do Dia agrupam abas no menu lateral (troca embaixo, como no Dia). Cada espaço pode ter um container padrão: abrir aba nova num espaço já abre naquele container (o jeito do Zen).

## Pacote

PKGBUILD próprio (`pacote/`), instalando em `/usr/lib/navegador`, com `.desktop`, ícone e o comando `navegador`. Perfil do usuário em `~/.config/navegador`.
