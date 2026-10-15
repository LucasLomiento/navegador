// Importar do Zen (Firefox): containers, espaços (workspaces), sites fixados
// e pastas, cookies de cada container (os logins dos sites), senhas,
// favoritos, histórico e buscadores. Lê de uma cópia do perfil do Zen, então
// funciona com o Zen aberto.
//
// Containers e cookies: os cookies do Zen ficam num sqlite só, com o
// container no originAttributes (userContextId); vão pra partição do
// container daqui (containers::PartitionConfig). Senhas, favoritos e
// histórico: o importador de Firefox do próprio Chromium, apontado pra
// cópia. Fica de fora o que os sites guardam fora dos cookies (localStorage,
// IndexedDB): os poucos sites que guardam o login ali pedem login de novo.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_IMPORTAR_ZEN_H_
#define NAVEGADOR_BROWSER_UI_IMPORTAR_ZEN_H_

#include <string>

#include "base/functional/callback.h"

class Profile;

namespace navegador::importar_zen {

struct Resumo {
  bool ok = false;
  std::u16string erro;
  int containers = 0;
  int espacos = 0;
  int fixados = 0;
  int cookies = 0;
};

// Procura o perfil do Zen (~/.config/zen, ~/.zen ou o do Flatpak) numa
// tarefa de fundo e responde se achou.
void ProcurarZen(base::OnceCallback<void(bool achou)> resposta);

// `andamento` recebe frases curtas ("Copiando o perfil do Zen…");
// `pronto`, o resumo no fim.
void Importar(Profile* perfil,
              base::RepeatingCallback<void(const std::u16string&)> andamento,
              base::OnceCallback<void(const Resumo&)> pronto);

}  // namespace navegador::importar_zen

#endif  // NAVEGADOR_BROWSER_UI_IMPORTAR_ZEN_H_
