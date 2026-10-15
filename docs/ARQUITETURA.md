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
3. **Partes que o Chrome não tem** (espaços, fixados e pastas, seletor de container, configurações, cabeçalho do menu lateral do Dia) como Views nossas em `navegador/`, encaixadas por ganchos pequenos.

As medidas vêm das telas do Dia decodificadas (`docs/dia-medidas.md`) e, quando a máquina virtual com Windows estiver de pé, de capturas do Dia rodando, comparadas lado a lado com as nossas.

## O que o Chromium mudou de padrão no navegador

- Abas sempre na lateral (190 de largura), sem opção de voltar pra cima.
- Barra de cima na cor do site: `navegador/browser/ui/cor_da_pagina.*` amostra 64x2 pixels do topo da página (como o `ColorSampleControllerV2` do Dia) e a janela recalcula as cores com um fornecedor de cor na chave (`BrowserWidget::GetColorProviderKey`).
- Lateral e moldura: cor escolhida no "Personalizar" tinge a moldura; vidro com a moldura translúcida pintada uma vez na janela inteira (`OpaqueBrowserFrameView::OnPaint`) e a lateral transparente por cima.
- Barra de endereço parada mostra "domínio / título" (`ShouldTrimDisplayUrlAfterHostName` ligado + `GetPageTitleForDisplay`; o título entra só no texto de exibição, nunca no URL de edição). Com o mouse em cima vira caixa com o URL sem o esquema (`LocationBarView::MostrarEnderecoInteiro`); o ícone do site (no fim da caixa, onde o Dia põe o "...") e as ações da página só acendem no hover ou no foco. Caixa focada e sugestões no tom da barra (`cores.cc`, `Barra::Pintar`).
- Menu do navegador no logo da lateral (`navegador/browser/ui/menu_do_logo.*`), como o AppMenuButton do Dia; Alt+F também. A barra fica sem avatar e sem "⋮" (os dois continuam com 1 px numa camada transparente, porque bolhas se ancoram neles; escondidos, o Chromium dá CHECK). Recolher a lateral: clique direito no logo ou Ctrl+Shift+L.
- Cartão (barra + página) com margem 6, cantos 8 (no fundo do top container e no da barra) e sombra suave pintada na moldura.
- Grupo de abas num bloco de fundo próprio no escuro (`TabGroupView`, como o Dia), menos dentro de um espaço.
- Nova aba: página simples (degradê e logo), sempre, com qualquer busca.
- Sem IA: flags desligadas na criação da FeatureList (`navegador/browser/sem_ia`: Modo IA, Compose, Lens, Gemini/Glic, histórico com IA, organizar guias, preenchimento e permissões com IA, tarefas contextuais...) e sem a página "IA no..." das Configurações.
- Sem login na conta Google (`signin.allowed` falso) e sem convite no botão de perfil.
- Sem barra de grupos salvos na nova aba; sem o botão "tudo" (grupos) no cabeçalho da lateral.
- Texto da lateral em 13; logo vetorial do navegador (`kProductIcon`, apagado como o do Dia) no cabeçalho, e a seta de "abas recentes" (a busca de abas do Chromium) à direita.
- Marca: `tools/marca.py` troca "Chromium" por "Navegador" nos textos de marca (o `chromium_strings.grd` com as partes `.grdp`, o dos componentes e mensagens escolhidas do `generated_resources`), recalculando os ids das traduções; `tools/icones.sh` gera os logos (inclusive os das páginas internas).

## Testar

`tools/testar.sh` abre o navegador compilado com perfil em /tmp numa janela flutuante, tira captura (`foto`), abre endereço (`url`, pelo DevTools na porta 9333), lista abas e manda teclas (`teclas`, só se a janela ativa for a de teste). `tools/cdp.mjs` roda JavaScript numa aba. Exemplo de teste de container: `--navegador-container-padrao=trabalho` + cookies comparados entre abas.

## Perfis

Perfis do Chromium como estão (cada um com cookies, histórico, extensões e janela próprios). Como no Dia, ficam no menu do logo (submenu com o nome do perfil): outros perfis, adicionar (nome e cor; a cor do perfil tinge a lateral), visitante e gerenciar.

## Containers

O Chromium não tem. A ideia: **cada container é uma `StoragePartition` própria dentro do mesmo perfil** (cookies, localStorage, IndexedDB, cache e service workers separados; histórico, favoritos e senhas continuam do perfil, como nos containers do Firefox). A aba de um container nasce com um `SiteInstance` de partição fixa (`SiteInstance::CreateForFixedStoragePartition`, o mesmo mecanismo das Isolated Web Apps e do `<webview>`).

Conferido no código do 155:
- **Navegar pra outro site mantém a partição**: `RenderFrameHostManager` (content/browser/renderer_host/render_frame_host_manager.cc, ~4076) copia a `StoragePartitionConfig` pra BrowsingInstance nova quando a atual é de partição fixa. Então o container é da aba inteira, pra sempre.
- **O container de uma aba se descobre pela própria aba**: a `StoragePartitionConfig` do `SiteInstance` dela (domínio `navegador-container`, nome = id do container). Não precisa de tabela paralela.

Ganchos (uma linha cada no Chromium, o resto em `navegador/containers/`):
- **Aba nova**: `CreateTargetContents` (chrome/browser/ui/navigator/browser_navigator.cc). Sem `opener`, pergunta pro nosso código: herda o container de `params.source_contents` (Ctrl+clique, "abrir em nova aba") ou usa o container padrão do espaço atual. Janela nova ainda não tem grupo focado: aí vale o espaço `Ativo` (em que a janela vai entrar), senão a primeira aba nascia fora do container.
- **Pré-carregamento**: a pré-renderização e a pré-busca saem fora da partição fixa (a página pré-renderizada nasce noutra BrowsingInstance; a pré-busca usa sempre a rede da partição padrão). Digitar um endereço já visitado numa aba de container abria a página com os cookies de fora. Desligados pra abas de container: `BrowserWebContentsDelegate::IsPrerender2Supported`, `SearchPrefetchService::MaybePrefetchURL` e o interceptador da busca pré-buscada, e no `content/` o `PrefetchDocumentManager::PrefetchUrl` (página numa partição que não é a padrão não pré-busca) e o `PrefetchURLLoaderInterceptor` (navegação fora da partição padrão não usa pré-busca).
- **Restaurar sessão**: `CreateRestoredTab` (chrome/browser/ui/browser_tabrestore.cc) lê `extra_data["navegador_container"]` e põe o `site_instance` no `CreateParams`, como o glic e o send-tab-to-self já fazem com o `extra_data`.
- **Salvar sessão**: um tab helper nosso grava o container com `SessionService::AddTabExtraData` (padrão do `SendTabToSelfActivationTracker::PersistGUID`). Atenção: `BuildCommandsForTab` (chrome/browser/sessions/session_service_base.cc) **não** regrava `extra_data` quando a sessão é reconstruída; precisa de gancho ali senão o container some depois de um tempo.
- **Reabrir aba fechada** (Ctrl+Shift+T): o `TabRestoreService` já guarda `extra_data`; falta só incluir o nosso na coleta.
- **sessionStorage**: na restauração, o namespace criado é o da partição padrão; o container ganha um novo vazio (perde o sessionStorage ao reiniciar, como no Firefox).
- **Página de nova aba**: WebUI (`chrome://`) numa partição de container é arriscado; a aba nova de container abre a nossa página simples (ou `about:blank` com a barra focada).

Os containers aparecem como cor e ícone na aba, com "Abrir em container" no menu de contexto e uma tela pra gerenciar.

## Espaços (como o Zen, com a cara do Dia)

O espaço é **dado do perfil** (`navegador/browser/containers/espacos_dados.*`): pref `navegador.espacos.lista` (`[{id, nome, icone, cor, container}]`), `navegador.espacos.grupos` (token do grupo de abas → espaço; o token se mantém entre sessões) e `navegador.espacos.ativo` (último usado). Não existe "Início": toda aba é de um espaço.

Na janela, cada espaço é um **grupo de abas focado** (`features::kTabGroupsFocusing`, que o Chrome 155 tem desligado; o navegador liga). A janela sempre tem um grupo focado e a lateral mostra só as abas dele. O controle da janela é a `FaixaDeEspacos` (`navegador/browser/ui/espacos.*`, dentro de `//chrome/browser/ui:ui`):

- Grupo sem espaço (sessão antiga, aba vinda de outra janela) vira espaço pelo nome, ou entra no espaço atual se não tem nome; aba solta entra no espaço atual.
- Aba nova nasce no espaço e no container dele (`EspacoPendente` + `CreateTargetContents`); link herda o container da aba de origem.
- Ativar uma aba de outro grupo foca o grupo dela (patch em `TabStripModel::ActivateTab`, 0008-espacos.patch): a janela vai pro espaço da aba, como no Zen. A faixa lembra a última aba de cada espaço.
- A última aba do espaço fechou: a janela fica no espaço, com aba nova.
- **Cor da janela**: o Chromium tinge a moldura com a cor do grupo focado (`SetUserColorOverride`), mas só na troca de foco com a lateral montada; na abertura da janela ficava neutra. A faixa guarda a cor do espaço (`espacos::CorDaJanela`) e o `BrowserWidget::GetColorProviderKey` usa ela desde o primeiro quadro; trocar de espaço troca o tema e repinta a moldura (que pinta o vidro atrás da lateral).
- **Troca**: botões laterais do mouse (voltar/avançar) com o ponteiro na lateral, rolagem horizontal ou Ctrl+rodinha na lateral (200 ms entre trocas, como o Zen), deslizar de lado no touchpad, bolinhas no pé, atalhos e o nome no cabeçalho.
- **Cabeçalho**: o nome do espaço entre o logo e a seta (`SetTituloDoEspaco` no `VerticalTabStripTopContainer`); clicar abre a lista como o seletor de perfis do Dia (✓ no atual, bolinha da cor nos outros, "Novo espaço…", "Editar espaços…"). O cabeçalho do grupo focado some da lista (`TabGroupViewLayout`).
- **Menu da aba**: "Mover para o espaço ›" no lugar dos itens de grupo do Chromium.

## Configurações e atalhos

`navegador/browser/ui/configuracoes.*`: janela "Configurações do Navegador" (Ctrl+, ou menu do logo, ou menu do nome do espaço: "Atalhos do teclado…" e "Configurações do Navegador…"), diálogo do Chromium (`constrained_window`, 820×600) no desenho da janela de configurações do Dia (Preferences/*.xaml): menu de 200 à esquerda (ícone + nome, o escolhido com fundo e a pílula de destaque), e páginas com título de 20 e seções (título + cartão com contorno; linhas com título de 14, descrição de 12 e o controle à direita). Páginas:
  - **Geral**: começar com uma nova aba ou as abas de onde parei (`SessionStartupPref`), botões da janela (`navegador.janela.botoes`, desligados), atalhos pras configurações e extensões do Chromium.
  - **Aparência**: modo claro/escuro/sistema (`ThemeService::SetBrowserColorScheme`), espessura da moldura (`navegador.janela.moldura`, 0–12, padrão 3) e a barra na cor do site (`navegador.barra.cor_do_site`, lido no `cor_da_pagina::CorDaBarra`).
  - **Lateral**: mostrar ao encostar na borda (`navegador.lateral.mostrar_na_borda`), esconder depois de (`navegador.lateral.esconder_depois_ms`, padrão 150 como o Zen), opacidade e desfoque da lateral por cima da página (`navegador.lateral.flutuante.opacidade` 90 / `.desfoque` 20) e o quadro de favoritos vazio.
  - **Espaços**: lista (bolinha de 20, nome, container, seta) que abre o detalhe no lugar (nome, emoji, container, cor, ordem, excluir), como os perfis no Dia. A cor sai do **seletor de cor** (`navegador/browser/ui/seletor_de_cor.*`): 18 cores prontas e uma livre (quadrado de saturação/brilho, barra de matiz, código). Cor livre fica como "#RRGGBB" no espaço; `containers::Cor` lê nome ou código, e o grupo de abas do Chromium (que só tem a paleta dele) recebe a mais perto pelo matiz (`containers::NomeMaisProximo`).
  - **Containers** (o painel de `containers/gerenciador`) e **Atalhos** (botão que captura a tecla; Esc cancela, Backspace tira; atalho repetido sai da outra ação).

Atalhos em `navegador.atalhos` (só os trocados; padrões em `espacos::Atalhos()`, Ctrl+S pra lateral), lidos por `navegador/browser/ui/atalhos.*` e registrados no FocusManager da janela em prioridade alta. Valem antes da página, como os reservados do Chromium: gancho no `BrowserView::PreHandleKeyboardEvent` (`atalhos::EDoNavegador`), senão um site que trata o Ctrl+S (ou o "Salvar página" do Chromium) ficava com a tecla.

- **Botões da janela**: `OpaqueBrowserFrameView::ShouldShowCaptionButtons()` lê a pref (e `BrowserFrameViewLinux::CaptionButtonsOn*Edge`, pra o layout não reservar o canto); trocar a opção refaz o layout das janelas abertas.
- **Moldura**: a margem do cartão (barra + página) vem da pref em `CalculateProposedLayout`; com a lateral escondida, a borda dela (`kLateralRecolhida`, 3) entra na conta pra a esquerda ficar igual aos outros lados. Sem os botões da janela, `IncreasePaddingToMinimum` não põe a "alça" de 36 antes do Voltar (a barra ficava desalinhada).
- **Lateral escondida (como o modo compacto do Zen)**: `VerticalTabStripRegionView`. Aparece na hora em que o ponteiro encosta na borda (sem a heurística de velocidade do Chromium, ~200 ms). Duas folgas pra esconder (`mouse_exit_timer_`, também esperando um menu aberto): `esconder_depois_ms` (150) quando o ponteiro sai dela pra página, e `esconder_fora_ms` (500) quando sai da janela ou a janela perde o foco (no Hyprland o foco vai com o ponteiro; o Chromium recolhia na hora). A saída da janela é vista na janela raiz do aura (`SaidaDaJanela`, só o evento da plataforma, não as saídas sintetizadas de passar pra janela da página, que antes contavam como saída e seguravam 1 s). Saindo pela borda esquerda da janela (ponteiro jogado pro canto da tela, além do vão entre as janelas), ela abre e segura a folga de fora, como o Zen. Animação de entrada 250 ms desacelerando, saída 150 ms. Abrindo ou escondendo, os itens ficam na largura cheia e deslizam (o `Layout` usa o layout da largura final com deslocamento; a camada corta), em vez de se amassarem na largura do meio. Por cima da página: opacidade da pref no fundo (`browser_view_tabbed_layout_impl.cc`) e `layer()->SetBackgroundBlur` com o desfoque da pref. Os fixados somem pela largura (lateral estreita), não pelo estado recolhido: aparecem também por cima da página.
- **Nova aba**: `new_tab_page_third_party` com a hora e a data grandes no centro e os mais visitados (`cr-most-visited`), no lugar do logo sozinho.
- **Sem linha entre a barra e a página**: em abas verticais fora da tela cheia, `CalculateSeparatorInfo` não liga o separador de cima do `MultiContentsView` (nem o do contêiner de cima); barra e página são um cartão só, como no Dia.

O menu da aba (`//chrome/browser/ui/tabs`) e o do logo (`toolbar:impl`) não enxergam o `ui:ui`: chamam por **pontes** em `containers_ui` (`DefinirMoverAba`, `DefinirFixarAba`, `DefinirAbrirConfiguracoes`), que o código da janela registra.

## Fixados, pastas e favoritos (do Zen)

`navegador/browser/containers/fixos.*` (dados) e `navegador/browser/ui/fixos.*` (lateral). Pref `navegador.espacos.fixos`: `{ "<espaço>": [site | pasta], "favoritos:<espaço>": [site] }`, com site `{id, url, titulo}` e pasta `{id, nome, aberta, itens}`. São do espaço, não da janela: toda janela no espaço mostra a mesma lista, e ela sobrevive a fechar o navegador.

- **Área no alto da lateral** (`AreaDeFixos`, filha do `TabStripView`, posta pelo `TabStripViewLayout` pelo id `visual::kIdAreaDeFixos`): favoritos em ladrilhos 3 por linha (54×41, cantos 8, como no Dia; sem favoritos, o quadro tracejado com alfinete, que some pelo clique direito "Ocultar este quadro" ou em Configurações → Geral), sites soltos e pastas (ícone, nome em cinza, seta; sites recuados 12), e uma linha antes das abas.
- **Aba ligada**: clicar num site abre (ou ativa) uma aba ligada a ele (user data na `WebContents`). A aba ligada some da lista (altura zero no `TabGroupViewLayout`) e aparece no lugar do site, destacada quando ativa; o "x" fecha a aba e o site fica. A ligação vai pra sessão (`extra_data` "navegador_fixo", em `session_service_base.cc` e `browser_tabrestore.cc`), então volta ligada ao reabrir. Aba ligada a site que sumiu, ou movida pra outro espaço, vira aba comum.
- **Menu da aba**: "Fixar no espaço ›" (fora das pastas, numa pasta, "Nova pasta…"; "Desafixar" se já é fixada) e "Adicionar aos favoritos" (gancho no `CommandTogglePinned` do `TabStripModel`; aba fixada do Chromium, de antes, segue o caminho antigo).
- **Menus da área**: site (renomear, usar o endereço atual, fechar a aba, mover, mover pra pasta, desafixar), pasta (renomear, abrir todos, fechar as abas, mover, excluir: as abas abertas viram comuns) e ladrilho (usar o endereço atual, fechar, mover, tirar dos favoritos).

## Interface dos containers

`navegador/browser/ui/containers/`:
- Barrinha de 3 px na esquerda da aba, na cor do container (gancho em `TabView::OnPaint`).
- "Abrir em container" no menu da aba (reabre a aba ao lado e fecha a original), "Abrir link em container" no menu da página, "Nova aba em container" no menu do botão "Nova aba". Os submenus têm delegate próprio; o item que abre cada submenu usa um id que o Chromium já aceita (id desconhecido derruba o navegador no `IsContextMenuCommandEnabled`).
- Janela "Containers" (gerenciar): criar, renomear, trocar a cor, excluir. Excluir apaga os dados da partição (`StoragePartition::ClearData`, tudo) e tira da lista; abas abertas nele ficam sem login.

## Pacote

PKGBUILD próprio (`pacote/`), instalando em `/usr/lib/navegador`, com `.desktop`, ícone e o comando `navegador`. Perfil do usuário em `~/.config/navegador`. Empacota o `out/release` (binário único, `tools/args-release.gn`) se estiver compilado, senão o `out/dev` (mesma otimização, em bibliotecas: o PKGBUILD copia os `.so` pro lado do programa, que tem RPATH `$ORIGIN`; sem `chrome-sandbox`, o Chromium usa o sandbox de namespaces do kernel). O launcher (`pacote/navegador.sh`) liga a integração com o sistema: IME no Wayland, chaveiro do GNOME, gesto de voltar e as extensões do Omarchy.
