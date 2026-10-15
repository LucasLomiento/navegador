// Detalhes das abas da lateral no estilo do Dia.

#ifndef NAVEGADOR_BROWSER_UI_ABAS_ABAS_H_
#define NAVEGADOR_BROWSER_UI_ABAS_ABAS_H_

namespace gfx {
class Canvas;
}  // namespace gfx

namespace views {
class View;
}  // namespace views

namespace navegador::abas {

// TabView::OnPaint, antes do fundo da aba: as abas fixadas são "ladrilhos"
// (os favoritos do Dia), com um fundo um pouco mais claro que a lateral.
void PintarLadrilho(gfx::Canvas* canvas, const views::View& aba, bool fixada);

}  // namespace navegador::abas

#endif  // NAVEGADOR_BROWSER_UI_ABAS_ABAS_H_
