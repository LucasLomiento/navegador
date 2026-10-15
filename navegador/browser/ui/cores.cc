#include "navegador/browser/ui/cores.h"

#include "chrome/browser/ui/color/chrome_color_id.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/color_utils.h"
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
  SkColor barra;          // barra de navegação (no Dia, da cor da página)
  SkColor pagina;         // fundo da página antes de carregar
  SkColor texto;          // TextFillColorPrimary
  SkColor texto_inativo;  // abas que não estão selecionadas
  SkColor aba_ativa;      // linha da aba ativa
  SkColor aba_hover;      // SubtleFillColorSecondary
  SkColor aba_selecionada;  // várias abas selecionadas
  SkColor divisor;        // DividerStrokeColorDefault
  SkColor endereco;       // barra de endereço parada (no Dia, texto solto)
  SkColor endereco_hover; // com o mouse em cima vira caixa
  SkColor destaque;       // AccentFillColorDefault
};

constexpr Paleta kClaro = {
    .fundo = SkColorSetRGB(0xF3, 0xF3, 0xF3),
    .barra = SK_ColorWHITE,
    .pagina = SK_ColorWHITE,
    .texto = SkColorSetARGB(0xE4, 0x00, 0x00, 0x00),
    .texto_inativo = SkColorSetARGB(0xB8, 0x00, 0x00, 0x00),
    .aba_ativa = SkColorSetARGB(0xCC, 0xFF, 0xFF, 0xFF),
    .aba_hover = SkColorSetARGB(0x0F, 0x00, 0x00, 0x00),
    .aba_selecionada = SkColorSetARGB(0x12, 0x00, 0x00, 0x00),
    .divisor = SkColorSetARGB(0x0F, 0x00, 0x00, 0x00),
    .endereco = SK_ColorWHITE,
    .endereco_hover = SkColorSetRGB(0xF1, 0xF1, 0xF1),
    .destaque = SkColorSetRGB(0x00, 0x5F, 0xB8),
};

constexpr Paleta kEscuro = {
    .fundo = SkColorSetRGB(0x20, 0x20, 0x20),
    .barra = SkColorSetRGB(0x1F, 0x1F, 0x1F),
    .pagina = SkColorSetRGB(0x1F, 0x1F, 0x1F),
    .texto = SK_ColorWHITE,
    .texto_inativo = SkColorSetARGB(0xC5, 0xFF, 0xFF, 0xFF),
    .aba_ativa = SkColorSetARGB(0xE6, 0x1C, 0x1C, 0x1C),
    .aba_hover = SkColorSetARGB(0x0B, 0xFF, 0xFF, 0xFF),
    .aba_selecionada = SkColorSetARGB(0x1F, 0xFF, 0xFF, 0xFF),
    .divisor = SkColorSetARGB(0x15, 0xFF, 0xFF, 0xFF),
    .endereco = SkColorSetRGB(0x1F, 0x1F, 0x1F),
    .endereco_hover = SkColorSetRGB(0x2B, 0x2B, 0x2B),
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
  // A cor escolhida no "Personalizar" do navegador (key.user_color) tinge o
  // fundo; sem escolha, fica neutro. Com o vidro, translúcida.
  const bool escuro = key.color_mode == ui::ColorProviderKey::ColorMode::kDark;
  SkColor fundo = p.fundo;
  if (key.user_color &&
      key.user_color_source == ui::ColorProviderKey::UserColorSource::kAccent) {
    fundo = color_utils::AlphaBlend(*key.user_color, p.fundo,
                                    escuro ? 0.22f : 0.18f);
  }
  const SkColor moldura =
      kVidro ? SkColorSetA(fundo, escuro ? kVidroAlfaEscuro : kVidroAlfaClaro)
             : fundo;
  mixer[ui::kColorFrameActive] = {moldura};
  mixer[ui::kColorFrameInactive] = {moldura};

  // Cartão: barra de navegação e página.
  mixer[kColorToolbar] = {p.barra};
  mixer[kColorWebContentsBackground] = {p.pagina};
  mixer[kColorToolbarButtonIcon] = {p.texto};
  mixer[kColorToolbarSeparator] = {SK_ColorTRANSPARENT};
  // A linha fina entre a barra e a página.
  mixer[kColorToolbarContentAreaSeparator] = {p.divisor};
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
