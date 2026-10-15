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

// Vidro (o "acrílico" do Dia): a moldura, que é o fundo da barra lateral e
// da margem em volta do cartão, sai translúcida e a janela não se declara
// opaca, pro desfoque do compositor (Hyprland) aparecer atrás. O cartão
// (barra de navegação e página) continua opaco.
inline constexpr bool kVidro = true;
inline constexpr unsigned char kVidroAlfaClaro = 0xC8;
inline constexpr unsigned char kVidroAlfaEscuro = 0xC0;

void AddColorMixer(ui::ColorProvider* provider,
                   const ui::ColorProviderKey& key);

}  // namespace navegador::cores

#endif  // NAVEGADOR_BROWSER_UI_CORES_H_
