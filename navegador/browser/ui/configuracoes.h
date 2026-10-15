// Configurações do Navegador: a janela das coisas que são nossas (espaços,
// containers e atalhos), aberta pelo menu do logo ou pelo atalho (Ctrl+,
// por padrão). As do Chromium continuam em chrome://settings.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_CONFIGURACOES_H_
#define NAVEGADOR_BROWSER_UI_CONFIGURACOES_H_

#include <string>

#include "base/functional/callback_forward.h"

class BrowserWindowInterface;

namespace navegador::configuracoes {

enum class Aba { kEspacos = 0, kContainers = 1, kAtalhos = 2 };

// Abre (ou traz pra frente) a janela de configurações na aba pedida.
void Mostrar(BrowserWindowInterface* browser, Aba aba = Aba::kEspacos);

// Editor de um espaço: nome, emoji, cor e container. Com `espaco` vazio,
// cria um novo. `pronto` recebe o id quando a pessoa confirma.
void EditarEspaco(BrowserWindowInterface* browser,
                  const std::string& espaco,
                  base::OnceCallback<void(const std::string&)> pronto);

}  // namespace navegador::configuracoes

#endif  // NAVEGADOR_BROWSER_UI_CONFIGURACOES_H_
