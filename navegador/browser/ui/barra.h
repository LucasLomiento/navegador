// Barra de endereço escondida (atalho "alternar_barra", Ctrl+Shift+S): a
// página ocupa o cartão inteiro. O ponteiro no alto da página (ou saindo da
// janela por cima) mostra a barra até voltar pra página, como a lateral
// escondida; o Ctrl+L e a nova aba mostram enquanto o foco estiver nela.

#ifndef NAVEGADOR_BROWSER_UI_BARRA_H_
#define NAVEGADOR_BROWSER_UI_BARRA_H_

#include <optional>

class BrowserView;
class BrowserWindowInterface;
class Profile;

namespace navegador::barra {

// A barra desta janela está escondida agora (pref ligada e sem estar à vista
// pelo Ctrl+L)? Só vale pras janelas normais: popups e apps mantêm a barra.
bool Escondida(const BrowserView* view);

// Barra escondida à vista agora (ou descendo/subindo): quanto já desceu, de 0
// a 1. O layout põe a barra por cima da página, que não muda de tamanho (é
// recortada no alto), como a lateral flutuante. Nada = layout normal.
std::optional<double> Sobreposta(const BrowserView* view);

// Liga ou desliga a barra escondida no perfil e refaz as janelas abertas.
void Definir(Profile* perfil, bool escondida);
void Alternar(BrowserWindowInterface* browser);

// Mostra a barra escondida até o foco sair dela (gancho no
// BrowserView::SetFocusToLocationBar, antes de focar o endereço).
void Espiar(BrowserView* view);

}  // namespace navegador::barra

#endif  // NAVEGADOR_BROWSER_UI_BARRA_H_
