// Cores do Dia (tema do WinUI que vem no pacote dele; docs/dia-medidas.md,
// seção 13) por cima das do Chromium.
//
// Compilado dentro do alvo //chrome/browser/ui/color:mixers. O gancho fica
// no fim de AddChromeColorMixers, depois até do tema do GTK, pra que o
// visual seja sempre o do navegador.

#ifndef NAVEGADOR_BROWSER_UI_CORES_H_
#define NAVEGADOR_BROWSER_UI_CORES_H_

namespace ui {
class ColorProvider;
struct ColorProviderKey;
}  // namespace ui

namespace navegador::cores {

void AddColorMixer(ui::ColorProvider* provider,
                   const ui::ColorProviderKey& key);

}  // namespace navegador::cores

#endif  // NAVEGADOR_BROWSER_UI_CORES_H_
