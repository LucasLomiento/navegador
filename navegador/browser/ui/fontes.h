// Fontes do desenho (Geist na interface, Bricolage Grotesque nos títulos;
// navegador/app/fontes).
//
// No Linux o pacote instala em /usr/share/fonts/navegador e não há nada a
// fazer. No Windows o instalador do Chromium só copia arquivos: as fontes
// vão na pasta da versão (fontes\) e, na partida, se ainda não estiverem
// instaladas pro usuário, vão pra %LOCALAPPDATA%\Microsoft\Windows\Fonts e
// pro registro do usuário (sem pedir administrador). A fonte da interface é
// lida na partida, então vale a partir da abertura seguinte.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_FONTES_H_
#define NAVEGADOR_BROWSER_UI_FONTES_H_

namespace navegador::fontes {

void InstalarSeFaltar();

}  // namespace navegador::fontes

#endif  // NAVEGADOR_BROWSER_UI_FONTES_H_
