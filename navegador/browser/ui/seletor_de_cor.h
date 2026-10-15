// Seletor de cor dos espaços: a paleta (bolinhas) e uma cor livre (quadrado
// de saturação e brilho, barra de matiz e o código "#RRGGBB"), como o
// seletor de cor de tema do Arc/Dia. A cor sai como texto: nome da paleta
// ("laranja") ou "#RRGGBB" (containers::Cor lê os dois).
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_SELETOR_DE_COR_H_
#define NAVEGADOR_BROWSER_UI_SELETOR_DE_COR_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"

namespace views {
class View;
}  // namespace views

namespace navegador::seletor_de_cor {

using Mudou = base::RepeatingCallback<void(const std::string& cor)>;

// `cor`: a atual. `mudou` roda a cada mudança (também durante o arrasto).
std::unique_ptr<views::View> Criar(const std::string& cor, Mudou mudou);

}  // namespace navegador::seletor_de_cor

#endif  // NAVEGADOR_BROWSER_UI_SELETOR_DE_COR_H_
