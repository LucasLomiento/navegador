// Ponte entre o controle de downloads do Chromium (DownloadToolbarUIController)
// e o botão de downloads da lateral (navegador/browser/ui/downloads.*): o
// controle não enxerga o ui:ui, então o botão se registra aqui.

#ifndef NAVEGADOR_BROWSER_UI_DOWNLOADS_PONTE_H_
#define NAVEGADOR_BROWSER_UI_DOWNLOADS_PONTE_H_

#include "base/functional/callback.h"

class BrowserWindowInterface;

namespace views {
class View;
}  // namespace views

namespace navegador::downloads {

// O botão da lateral pra ancorar a bolha de downloads (nulo se a janela não
// tem o botão à vista: aí a bolha não abre, já que a barra não tem o botão).
using AncoraDaJanela =
    base::RepeatingCallback<views::View*(BrowserWindowInterface*)>;
void DefinirAncora(AncoraDaJanela ancora);
views::View* Ancora(BrowserWindowInterface* browser);
// O botão da lateral existe (então a barra não mostra o dela).
bool TemBotaoNaLateral();

// Progresso dos downloads da janela, pro anel em volta do botão.
struct Progresso {
  int quantos = 0;           // downloads em andamento
  int porcentagem = 0;       // de todos juntos
  bool tamanho_conhecido = true;
  bool concluido_novo = false;  // terminou e ninguém viu ainda
};
using AvisoDeProgresso =
    base::RepeatingCallback<void(BrowserWindowInterface*, const Progresso&)>;
void DefinirAvisoDeProgresso(AvisoDeProgresso aviso);
void AvisarProgresso(BrowserWindowInterface* browser,
                     const Progresso& progresso);

}  // namespace navegador::downloads

#endif  // NAVEGADOR_BROWSER_UI_DOWNLOADS_PONTE_H_
