#include "navegador/browser/ui/cores.h"

#include "chrome/browser/ui/color/chrome_color_id.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/color/color_id.h"
#include "ui/color/color_mixer.h"
#include "ui/color/color_provider.h"
#include "ui/color/color_provider_key.h"
#include "ui/color/color_recipe.h"

namespace navegador::cores {

namespace {

// Paleta do Dia: claro e escuro lado a lado. ARGB, alfa primeiro.
struct Paleta {
  SkColor fundo;          // barra lateral e moldura (SolidBackgroundFillColorBase)
  SkColor barra;          // barra de navegação (SolidBackgroundFillColorTertiary)
  SkColor pagina;         // fundo da página antes de carregar
  SkColor texto;          // TextFillColorPrimary
  SkColor texto_inativo;  // abas que não estão selecionadas
  SkColor aba_ativa;      // linha da aba ativa
  SkColor aba_hover;      // SubtleFillColorSecondary
  SkColor aba_selecionada;  // várias abas selecionadas
  SkColor divisor;        // DividerStrokeColorDefault
  SkColor endereco;       // fundo da barra de endereço
  SkColor endereco_hover;
  SkColor destaque;       // AccentFillColorDefault
};

constexpr Paleta kClaro = {
    .fundo = SkColorSetRGB(0xF3, 0xF3, 0xF3),
    .barra = SkColorSetRGB(0xF9, 0xF9, 0xF9),
    .pagina = SK_ColorWHITE,
    .texto = SkColorSetARGB(0xE4, 0x00, 0x00, 0x00),
    .texto_inativo = SkColorSetARGB(0xB8, 0x00, 0x00, 0x00),
    .aba_ativa = SK_ColorWHITE,
    .aba_hover = SkColorSetARGB(0x09, 0x00, 0x00, 0x00),
    .aba_selecionada = SkColorSetARGB(0x12, 0x00, 0x00, 0x00),
    .divisor = SkColorSetARGB(0x0F, 0x00, 0x00, 0x00),
    .endereco = SkColorSetARGB(0x09, 0x00, 0x00, 0x00),
    .endereco_hover = SkColorSetARGB(0x0F, 0x00, 0x00, 0x00),
    .destaque = SkColorSetRGB(0x00, 0x5F, 0xB8),
};

constexpr Paleta kEscuro = {
    .fundo = SkColorSetRGB(0x20, 0x20, 0x20),
    .barra = SkColorSetRGB(0x28, 0x28, 0x28),
    .pagina = SkColorSetRGB(0x1F, 0x1F, 0x1F),
    .texto = SK_ColorWHITE,
    .texto_inativo = SkColorSetARGB(0xC5, 0xFF, 0xFF, 0xFF),
    .aba_ativa = SkColorSetARGB(0x15, 0xFF, 0xFF, 0xFF),
    .aba_hover = SkColorSetARGB(0x0B, 0xFF, 0xFF, 0xFF),
    .aba_selecionada = SkColorSetARGB(0x1F, 0xFF, 0xFF, 0xFF),
    .divisor = SkColorSetARGB(0x15, 0xFF, 0xFF, 0xFF),
    .endereco = SkColorSetARGB(0x0F, 0xFF, 0xFF, 0xFF),
    .endereco_hover = SkColorSetARGB(0x15, 0xFF, 0xFF, 0xFF),
    .destaque = SkColorSetRGB(0x60, 0xCD, 0xFF),
};

}  // namespace

void AddColorMixer(ui::ColorProvider* provider,
                   const ui::ColorProviderKey& key) {
  // Janelas de app (PWA) seguem a cor do app.
  if (key.app_controller) {
    return;
  }
  const Paleta& p = key.color_mode == ui::ColorProviderKey::ColorMode::kDark
                        ? kEscuro
                        : kClaro;
  ui::ColorMixer& mixer = provider->AddMixer();

  // Moldura = fundo da barra lateral. Igual com a janela focada ou não.
  // Com o vidro, translúcida.
  const bool escuro = key.color_mode == ui::ColorProviderKey::ColorMode::kDark;
  const SkColor moldura =
      kVidro ? SkColorSetA(p.fundo, escuro ? kVidroAlfaEscuro : kVidroAlfaClaro)
             : p.fundo;
  mixer[ui::kColorFrameActive] = {moldura};
  mixer[ui::kColorFrameInactive] = {moldura};

  // Cartão: barra de navegação e página.
  mixer[kColorToolbar] = {p.barra};
  mixer[kColorWebContentsBackground] = {p.pagina};
  mixer[kColorToolbarButtonIcon] = {p.texto};
  mixer[kColorToolbarSeparator] = {SK_ColorTRANSPARENT};
  mixer[kColorToolbarContentAreaSeparator] = {SK_ColorTRANSPARENT};
  mixer[kColorToolbarTopSeparatorFrameActive] = {SK_ColorTRANSPARENT};
  mixer[kColorToolbarTopSeparatorFrameInactive] = {SK_ColorTRANSPARENT};

  // Barra de endereço.
  mixer[kColorLocationBarBackground] = {p.endereco};
  mixer[kColorLocationBarBackgroundHovered] = {p.endereco_hover};
  mixer[kColorOmniboxText] = {p.texto};

  // Abas da lateral.
  mixer[kColorTabBackgroundActiveFrameActive] = {p.aba_ativa};
  mixer[kColorTabBackgroundActiveFrameInactive] = {p.aba_ativa};
  mixer[kColorTabBackgroundInactiveHoverFrameActive] = {p.aba_hover};
  mixer[kColorTabBackgroundInactiveHoverFrameInactive] = {p.aba_hover};
  mixer[kColorTabBackgroundSelectedFrameActive] = {p.aba_selecionada};
  mixer[kColorTabBackgroundSelectedFrameInactive] = {p.aba_selecionada};
  mixer[kColorTabForegroundActiveFrameActive] = {p.texto};
  mixer[kColorTabForegroundActiveFrameInactive] = {p.texto};
  mixer[kColorTabForegroundInactiveFrameActive] = {p.texto_inativo};
  mixer[kColorTabForegroundInactiveFrameInactive] = {p.texto_inativo};
  mixer[kColorTabDividerFrameActive] = {p.divisor};
  mixer[kColorTabDividerFrameInactive] = {p.divisor};
  mixer[kColorVerticalTabStripShadow] = {SK_ColorTRANSPARENT};

  // Destaque (barra de carregamento, foco, botões principais).
  mixer[ui::kColorAccent] = {p.destaque};
}

}  // namespace navegador::cores
