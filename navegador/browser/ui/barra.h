// Barra de endereço escondida (atalho "alternar_barra", Ctrl+Shift+S): a
// página ocupa o cartão inteiro. O Ctrl+L e a nova aba mostram a barra
// enquanto o foco estiver nela; ao sair (Enter, clique na página), ela some de
// novo.

#ifndef NAVEGADOR_BROWSER_UI_BARRA_H_
#define NAVEGADOR_BROWSER_UI_BARRA_H_

class BrowserView;
class BrowserWindowInterface;
class Profile;

namespace navegador::barra {

// A barra desta janela está escondida agora (pref ligada e sem estar à vista
// pelo Ctrl+L)? Só vale pras janelas normais: popups e apps mantêm a barra.
bool Escondida(const BrowserView* view);

// Liga ou desliga a barra escondida no perfil e refaz as janelas abertas.
void Definir(Profile* perfil, bool escondida);
void Alternar(BrowserWindowInterface* browser);

// Mostra a barra escondida até o foco sair dela (gancho no
// BrowserView::SetFocusToLocationBar, antes de focar o endereço).
void Espiar(BrowserView* view);

}  // namespace navegador::barra

#endif  // NAVEGADOR_BROWSER_UI_BARRA_H_
