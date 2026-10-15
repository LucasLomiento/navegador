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
// a borda da janela e cantos arredondados (medido no Dia rodando: margem ~6,
// cantos ~8; docs/dia-medidas.md, seção 15).
inline constexpr int kCartaoMargem = 6;
inline constexpr int kCartaoRaio = 8;

// Folga entre as abas da lateral: no Dia, uma aba (destaque de 30) a cada 35.
inline constexpr int kEspacoEntreAbas = 5;

// Lateral recolhida: só a borda onde se encosta o mouse pra ela aparecer
// por cima (a lateral flutuante do Dia).
inline constexpr int kLateralRecolhida = 8;

std::optional<int> LayoutConstantOverride(LayoutConstant constant);
std::optional<gfx::Insets> LayoutInsetsOverride(LayoutInset inset);

}  // namespace navegador::visual

#endif  // NAVEGADOR_BROWSER_UI_VISUAL_H_
