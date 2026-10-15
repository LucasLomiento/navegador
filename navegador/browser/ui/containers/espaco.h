// Espaço da janela (id do espaço, ver espacos_dados.h), pros containers.
// Separado do containers_ui pra o browser_navigator poder usar sem puxar a
// interface.

#ifndef NAVEGADOR_BROWSER_UI_CONTAINERS_ESPACO_H_
#define NAVEGADOR_BROWSER_UI_CONTAINERS_ESPACO_H_

#include <optional>
#include <string>

#include "base/memory/raw_ptr.h"

class BrowserWindowInterface;

namespace navegador::containers {

// Espaço das abas novas da janela: o pendente (ver abaixo), se houver; senão
// o do grupo focado.
std::optional<std::string> EspacoFocado(BrowserWindowInterface* browser);

// Enquanto vive, as abas novas da janela são do `espaco` (a janela criando a
// primeira aba de um espaço que ainda não tem grupo nela).
class EspacoPendente {
 public:
  EspacoPendente(BrowserWindowInterface* browser, std::string espaco);
  EspacoPendente(const EspacoPendente&) = delete;
  EspacoPendente& operator=(const EspacoPendente&) = delete;
  ~EspacoPendente();

 private:
  raw_ptr<BrowserWindowInterface> browser_;
};

}  // namespace navegador::containers

#endif  // NAVEGADOR_BROWSER_UI_CONTAINERS_ESPACO_H_
