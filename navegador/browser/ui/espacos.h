// Espaços do Dia: a faixa no pé da barra lateral, com o nome do espaço
// atual, uma bolinha por espaço e o "+" pra criar um novo.
//
// Cada espaço é um grupo de abas "focado" (features::kTabGroupsFocusing, que
// o navegador liga por padrão): com um grupo focado, a lateral mostra só as
// abas dele. "Início" é sem foco, com todas as abas.
//
// Compilado dentro de //chrome/browser/ui:ui (precisa do BrowserWindowInterface,
// do chrome::NewTab e dos grupos; a barra lateral, que chama isto, mora lá).

#ifndef NAVEGADOR_BROWSER_UI_ESPACOS_H_
#define NAVEGADOR_BROWSER_UI_ESPACOS_H_

class BrowserWindowInterface;

namespace views {
class View;
}  // namespace views

namespace navegador::espacos {

// Põe no fim de `lateral` (a VerticalTabStripRegionView, de FlexLayout
// vertical) um espaçador que empurra pra baixo e a faixa de espaços.
void AdicionarFaixa(views::View* lateral, BrowserWindowInterface* browser);

}  // namespace navegador::espacos

#endif  // NAVEGADOR_BROWSER_UI_ESPACOS_H_
