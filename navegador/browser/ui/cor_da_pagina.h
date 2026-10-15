// A barra de navegação na cor do site aberto, como o Dia: amostra os pixels
// do topo da página ativa (cor dominante de uma faixa fina) e manda a janela
// recalcular as cores quando muda. Sem pixels, usa o theme-color ou o fundo.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_COR_DA_PAGINA_H_
#define NAVEGADOR_BROWSER_UI_COR_DA_PAGINA_H_

#include <optional>

#include "third_party/skia/include/core/SkColor.h"

class BrowserView;

namespace views {
class View;
}  // namespace views

namespace navegador::cor_da_pagina {

// Cor da página ativa da janela, se tiver (BrowserWidget::GetColorProviderKey).
std::optional<SkColor> CorDaBarra(const BrowserView* browser_view);

// Põe dentro de `dono` (uma View que vive tanto quanto a janela) o
// observador da janela `browser_view`.
void Acompanhar(views::View* dono, BrowserView* browser_view);

}  // namespace navegador::cor_da_pagina

#endif  // NAVEGADOR_BROWSER_UI_COR_DA_PAGINA_H_
