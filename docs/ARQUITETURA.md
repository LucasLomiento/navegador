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

## O que o Chromium mudou de padrão no navegador

- Abas sempre na lateral (190 de largura), sem opção de voltar pra cima.
- Barra de cima na cor do site: `navegador/browser/ui/cor_da_pagina.*` amostra 64x2 pixels do topo da página (como o `ColorSampleControllerV2` do Dia) e a janela recalcula as cores com um fornecedor de cor na chave (`BrowserWidget::GetColorProviderKey`).
- Lateral e moldura: cor escolhida no "Personalizar" tinge a moldura; vidro com a moldura translúcida pintada uma vez na janela inteira (`OpaqueBrowserFrameView::OnPaint`) e a lateral transparente por cima.
- Barra de endereço parada mostra só o domínio (`ShouldTrimDisplayUrlAfterHostName` ligado); caixa só aparece com o mouse em cima.
- Nova aba: página simples (degradê e logo), sempre, com qualquer busca.
- Sem IA: flags `AimEnabled`, `Aim3pEntrypoint`, `NtpComposebox`, `Compose`, `LensOverlay`, `Glic`, `GlicRollout`, `HistoryEmbeddings` desligadas na criação da FeatureList (`navegador/browser/sem_ia`).
- Sem login na conta Google (`signin.allowed` falso) e sem convite no botão de perfil.
- Sem barra de grupos salvos na nova aba; sem o botão "tudo" (grupos) no cabeçalho da lateral.
- Texto da lateral em 13; logo vetorial do navegador (`kProductIcon`) no botão de recolher.

## Testar

`tools/testar.sh` abre o navegador compilado com perfil em /tmp numa janela flutuante, tira captura (`foto`), abre endereço (`url`, pelo DevTools na porta 9333), lista abas e manda teclas (`teclas`, só se a janela ativa for a de teste). `tools/cdp.mjs` roda JavaScript numa aba. Exemplo de teste de container: `--navegador-container-padrao=trabalho` + cookies comparados entre abas.

## Perfis

Perfis do Chromium como estão (cada um com cookies, histórico, extensões e janela próprios). Muda só a aparência do seletor, no estilo do Dia, no menu lateral.

## Containers

O Chromium não tem. A ideia: **cada container é uma `StoragePartition` própria dentro do mesmo perfil** (cookies, localStorage, IndexedDB, cache e service workers separados; histórico, favoritos e senhas continuam do perfil, como nos containers do Firefox). A aba de um container nasce com um `SiteInstance` de partição fixa (`SiteInstance::CreateForFixedStoragePartition`, o mesmo mecanismo das Isolated Web Apps e do `<webview>`).

Conferido no código do 155:
- **Navegar pra outro site mantém a partição**: `RenderFrameHostManager` (content/browser/renderer_host/render_frame_host_manager.cc, ~4076) copia a `StoragePartitionConfig` pra BrowsingInstance nova quando a atual é de partição fixa. Então o container é da aba inteira, pra sempre.
- **O container de uma aba se descobre pela própria aba**: a `StoragePartitionConfig` do `SiteInstance` dela (domínio `navegador-container`, nome = id do container). Não precisa de tabela paralela.

Ganchos (uma linha cada no Chromium, o resto em `navegador/containers/`):
- **Aba nova**: `CreateTargetContents` (chrome/browser/ui/navigator/browser_navigator.cc). Sem `opener`, pergunta pro nosso código: herda o container de `params.source_contents` (Ctrl+clique, "abrir em nova aba") ou usa o container padrão do espaço atual.
- **Restaurar sessão**: `CreateRestoredTab` (chrome/browser/ui/browser_tabrestore.cc) lê `extra_data["navegador_container"]` e põe o `site_instance` no `CreateParams`, como o glic e o send-tab-to-self já fazem com o `extra_data`.
- **Salvar sessão**: um tab helper nosso grava o container com `SessionService::AddTabExtraData` (padrão do `SendTabToSelfActivationTracker::PersistGUID`). Atenção: `BuildCommandsForTab` (chrome/browser/sessions/session_service_base.cc) **não** regrava `extra_data` quando a sessão é reconstruída; precisa de gancho ali senão o container some depois de um tempo.
- **Reabrir aba fechada** (Ctrl+Shift+T): o `TabRestoreService` já guarda `extra_data`; falta só incluir o nosso na coleta.
- **sessionStorage**: na restauração, o namespace criado é o da partição padrão; o container ganha um novo vazio (perde o sessionStorage ao reiniciar, como no Firefox).
- **Página de nova aba**: WebUI (`chrome://`) numa partição de container é arriscado; a aba nova de container abre a nossa página simples (ou `about:blank` com a barra focada).

Os containers aparecem como cor e ícone na aba, com "Abrir em container" no menu de contexto e uma tela pra gerenciar.

## Espaços (do Dia)

Cada espaço é um **grupo de abas focado** (`features::kTabGroupsFocusing`, que o Chrome 155 tem mas vem desligado; o navegador liga). Com um grupo focado, a lateral mostra só as abas dele e o `SetFocusedGroup` já ativa a primeira aba do grupo. "Início" é sem foco (todas as abas).

- **Faixa no pé da lateral** (`navegador/browser/ui/espacos.*`, compilado dentro de `//chrome/browser/ui:ui`): nome do espaço atual, uma bolinha por espaço na cor do grupo, "+" pra criar. A rodinha do mouse passa de um pro outro. Some com a lateral recolhida. Medidas do seletor do Dia (38 de altura, título até 160, bolinhas a 3).
- **Clique direito na faixa**: "Editar espaço…" (o balão de grupo do próprio Chrome: nome e cor), "Container do espaço", "Novo espaço".
- **Container padrão do espaço**: pref `navegador.espacos_container` (`{ id do grupo: id do container }`; o id do grupo se mantém entre sessões). Ctrl+T e "Nova aba" num espaço com container nascem nele; link aberto de uma aba herda o container da aba.

## Interface dos containers

`navegador/browser/ui/containers/`:
- Barrinha de 3 px na esquerda da aba, na cor do container (gancho em `TabView::OnPaint`).
- "Abrir em container" no menu da aba (reabre a aba ao lado e fecha a original), "Abrir link em container" no menu da página, "Nova aba em container" no menu do botão "Nova aba". Os submenus têm delegate próprio; o item que abre cada submenu usa um id que o Chromium já aceita (id desconhecido derruba o navegador no `IsContextMenuCommandEnabled`).
- Janela "Containers" (gerenciar): criar, renomear, trocar a cor, excluir. Excluir apaga os dados da partição (`StoragePartition::ClearData`, tudo) e tira da lista; abas abertas nele ficam sem login.

## Pacote

PKGBUILD próprio (`pacote/`), instalando em `/usr/lib/navegador`, com `.desktop`, ícone e o comando `navegador`. Perfil do usuário em `~/.config/navegador`.
