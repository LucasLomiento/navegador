// Medidas do visual do Dia aplicadas no Chromium.
//
// Os números vêm de docs/dia-medidas.md (telas do Dia decodificadas). O
// Chromium pergunta as medidas dele pra GetLayoutConstant/GetLayoutInsets
// (chrome/browser/ui/layout_constants.cc); o gancho lá consulta este arquivo
// primeiro, e o que não está aqui fica como o Chromium faz.
//
// Compilado dentro do alvo //chrome/browser/ui:layout_constants (e não num
// alvo próprio) porque precisa do enum dele, e ele precisa da gente.

#ifndef NAVEGADOR_BROWSER_UI_VISUAL_H_
#define NAVEGADOR_BROWSER_UI_VISUAL_H_

#include <optional>

#include "chrome/browser/ui/layout_constants.h"
#include "ui/gfx/geometry/insets.h"

namespace navegador::visual {

// Cartão do conteúdo: a barra de navegação e a página juntas, com margem até
// a borda da janela e cantos arredondados (a "superfície" do Dia, cantos 7).
inline constexpr int kCartaoMargem = 8;
inline constexpr int kCartaoRaio = 7;

std::optional<int> LayoutConstantOverride(LayoutConstant constant);
std::optional<gfx::Insets> LayoutInsetsOverride(LayoutInset inset);

}  // namespace navegador::visual

#endif  // NAVEGADOR_BROWSER_UI_VISUAL_H_
