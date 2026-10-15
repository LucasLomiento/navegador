// Atalhos configuráveis em texto ("Ctrl+Alt+Right"), como ficam guardados na
// pref navegador.atalhos (ver navegador/browser/containers/espacos_dados.h).

#ifndef NAVEGADOR_BROWSER_UI_ATALHOS_H_
#define NAVEGADOR_BROWSER_UI_ATALHOS_H_

#include <optional>
#include <string>
#include <string_view>

#include "ui/base/accelerators/accelerator.h"

class Profile;

namespace navegador::atalhos {

// "Ctrl+Alt+Right" -> Accelerator. Nada se o texto for vazio ou inválido.
std::optional<ui::Accelerator> Ler(std::string_view texto);
// Accelerator -> "Ctrl+Alt+Right". Vazio se a tecla não tiver nome conhecido.
std::string Escrever(const ui::Accelerator& atalho);
// Pra mostrar na tela ("Ctrl+Alt+→"); vazio = "Sem atalho".
std::u16string Mostrar(std::string_view texto);

// É atalho de alguma ação do navegador (Configurações → Atalhos)? Esses valem
// antes da página, como os reservados do Chromium (Ctrl+W): o Ctrl+S mostra
// a lateral mesmo num site que trata o Ctrl+S (gancho no BrowserView).
bool EDoNavegador(Profile* profile, const ui::Accelerator& atalho);

}  // namespace navegador::atalhos

#endif  // NAVEGADOR_BROWSER_UI_ATALHOS_H_
