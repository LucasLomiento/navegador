// Menu do navegador no logo da barra lateral, como no Dia: lá o logo no
// canto de cima da lateral é o botão do menu do app (AppMenuButton no XAML
// do Dia), e a barra de navegação não tem "⋮" nem avatar. Os perfis ficam no
// submenu "Perfil" do mesmo menu.
//
// O botão do logo é o de recolher a lateral do Chromium (que já mostra o
// logo); recolher continua no clique direito e no Ctrl+Shift+L.
//
// Compilado dentro de //chrome/browser/ui:ui (precisa do AppMenu e da
// BrowserView, que moram lá).

#ifndef NAVEGADOR_BROWSER_UI_MENU_DO_LOGO_H_
#define NAVEGADOR_BROWSER_UI_MENU_DO_LOGO_H_

class BrowserWindowInterface;

namespace views {
class Button;
}  // namespace views

namespace navegador::menu_do_logo {

// Faz `logo` abrir o menu do navegador, embaixo dele.
void Ligar(views::Button* logo, BrowserWindowInterface* browser);

// Abre o menu no logo da janela (Alt+F, Alt+E). Falso se a janela não tem o
// logo à vista (lateral escondida, tela cheia); aí quem chamou abre do jeito
// do Chromium.
bool Abrir(BrowserWindowInterface* browser, bool pelo_teclado);

}  // namespace navegador::menu_do_logo

#endif  // NAVEGADOR_BROWSER_UI_MENU_DO_LOGO_H_
