// Espaço focado da janela, como texto (id do grupo de abas), pros
// containers. Separado do containers_ui pra o browser_navigator poder usar
// sem puxar a interface.

#ifndef NAVEGADOR_BROWSER_UI_CONTAINERS_ESPACO_H_
#define NAVEGADOR_BROWSER_UI_CONTAINERS_ESPACO_H_

#include <optional>
#include <string>

class BrowserWindowInterface;

namespace navegador::containers {

std::optional<std::string> EspacoFocado(BrowserWindowInterface* browser);

}  // namespace navegador::containers

#endif  // NAVEGADOR_BROWSER_UI_CONTAINERS_ESPACO_H_
