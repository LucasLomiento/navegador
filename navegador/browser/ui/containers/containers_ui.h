// Interface dos containers: a cor na aba, o submenu "Abrir em container" e
// abrir uma aba num container.

#ifndef NAVEGADOR_BROWSER_UI_CONTAINERS_CONTAINERS_UI_H_
#define NAVEGADOR_BROWSER_UI_CONTAINERS_CONTAINERS_UI_H_

#include <memory>
#include <string_view>

#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/models/image_model.h"

class GURL;
class TabStripModel;

namespace content {
class WebContents;
}  // namespace content

namespace gfx {
class Canvas;
}  // namespace gfx

namespace tabs {
class TabInterface;
}  // namespace tabs

namespace ui {
class SimpleMenuModel;
}  // namespace ui

namespace views {
class View;
}  // namespace views

namespace navegador::containers {

// Cor de verdade de um nome da paleta (azul, laranja, ...). As mesmas no
// tema claro e no escuro: são vivas o bastante pros dois.
SkColor Cor(std::string_view nome);

// Bolinha da cor do container, pros menus.
ui::ImageModel Bolinha(std::string_view nome);

// TabView::OnPaint: a barrinha colorida na esquerda da aba de container.
void PintarIndicador(gfx::Canvas* canvas,
                     const views::View& aba,
                     const tabs::TabInterface* tab);

// Abre `url` numa aba nova no container `id` (vazio = sem container) logo
// depois de `index`.
void AbrirEmContainer(TabStripModel* tab_strip,
                      const GURL& url,
                      std::string_view id,
                      int index,
                      bool ativar);

// Menu de contexto da aba: "Abrir em container" com os containers do
// perfil (e "Sem container", se a aba está em um). Reabre a aba no
// escolhido e fecha a original.
std::unique_ptr<ui::SimpleMenuModel> CriarSubmenuDaAba(TabStripModel* tab_strip,
                                                       int index);

// Menu de contexto de um link: "Abrir link em container". Abre o link numa
// aba nova (em segundo plano, como "abrir link em nova aba") ao lado da aba
// de origem.
std::unique_ptr<ui::SimpleMenuModel> CriarSubmenuDoLink(
    TabStripModel* tab_strip,
    content::WebContents* origem,
    const GURL& link);

// Menu do botão "Nova aba": "Nova aba em container".
std::unique_ptr<ui::SimpleMenuModel> CriarSubmenuNovaAba(
    TabStripModel* tab_strip);

}  // namespace navegador::containers

#endif  // NAVEGADOR_BROWSER_UI_CONTAINERS_CONTAINERS_UI_H_
