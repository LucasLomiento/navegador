// Botão de downloads da lateral, no pé dela (à esquerda das bolinhas dos
// espaços), sempre à vista. No Chromium o botão aparece do nada no canto da
// barra quando um download começa; aqui a bolha de downloads abre ancorada
// neste botão, com o anel de progresso em volta dele.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_DOWNLOADS_BOTAO_H_
#define NAVEGADOR_BROWSER_UI_DOWNLOADS_BOTAO_H_

#include <memory>

class BrowserWindowInterface;

namespace views {
class View;
}  // namespace views

namespace navegador::downloads {

std::unique_ptr<views::View> CriarBotao(BrowserWindowInterface* browser);

}  // namespace navegador::downloads

#endif  // NAVEGADOR_BROWSER_UI_DOWNLOADS_BOTAO_H_
