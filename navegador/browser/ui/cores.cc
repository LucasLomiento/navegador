#include "navegador/browser/ui/cores.h"

#include <map>

#include "base/no_destructor.h"

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
    .aba_ativa = SkColorSetARGB(0x1F, 0xFF, 0xFF, 0xFF),
    .aba_hover = SkColorSetARGB(0x0B, 0xFF, 0xFF, 0xFF),
    .aba_selecionada = SkColorSetARGB(0x1F, 0xFF, 0xFF, 0xFF),
    .divisor = SkColorSetARGB(0x15, 0xFF, 0xFF, 0xFF),
    .endereco = SkColorSetRGB(0x1F, 0x1F, 0x1F),
    .endereco_hover = SkColorSetRGB(0x2B, 0x2B, 0x2B),
    .destaque = SkColorSetRGB(0x60, 0xCD, 0xFF),
};

// Cor da página na barra de navegação, com texto e ícones no contraste.
class Barra : public ui::ColorProviderKey::InitializerSupplier {
 public:
  explicit Barra(SkColor cor) : cor_(SkColorSetA(cor, SK_AlphaOPAQUE)) {}

  // Só a janela do navegador chama isto (pela chave); o trabalho de verdade
  // é o Pintar, chamado no fim do mixer do navegador, depois da paleta.
  void AddColorMixers(ui::ColorProvider* provider,
                      const ui::ColorProviderKey& key) const override {}

  void Pintar(ui::ColorMixer& mixer) const {
    const bool escura = color_utils::IsDark(cor_);
    const SkColor texto =
        escura ? SK_ColorWHITE : SkColorSetARGB(0xE4, 0x00, 0x00, 0x00);
    mixer[kColorToolbar] = {cor_};
    mixer[kColorWebContentsBackground] = {cor_};
    mixer[kColorLocationBarBackground] = {cor_};
    mixer[kColorLocationBarBackgroundHovered] = {
        color_utils::BlendTowardMaxContrast(cor_, 0x16)};
    mixer[kColorToolbarText] = {texto};
    mixer[kColorOmniboxText] = {texto};
    mixer[kColorOmniboxTextDimmed] = {SkColorSetA(texto, 0x99)};
    mixer[kColorToolbarButtonIcon] = {texto};
    mixer[kColorToolbarButtonIconHovered] = {texto};
    mixer[kColorToolbarButtonIconPressed] = {texto};
    mixer[kColorToolbarButtonIconInactive] = {SkColorSetA(texto, 0x5C)};
    mixer[kColorToolbarInkDrop] = {texto};
    mixer[kColorToolbarContentAreaSeparator] = {SkColorSetA(texto, 0x1A)};
  }

 private:
  SkColor cor_;
};

std::map<SkColor, Barra*>& Barras() {
  static base::NoDestructor<std::map<SkColor, Barra*>> barras;
  return *barras;
}

const Barra* BarraDe(const ui::ColorProviderKey::InitializerSupplier* s) {
  if (!s) {
    return nullptr;
  }
  for (const auto& [cor, barra] : Barras()) {
    if (barra == s) {
      return barra;
    }
  }
  return nullptr;
}

}  // namespace

ui::ColorProviderKey::InitializerSupplier* FornecedorDaBarra(SkColor cor) {
  cor = SkColorSetA(cor, SK_AlphaOPAQUE);
  Barra*& barra = Barras()[cor];
  if (!barra) {
    barra = new Barra(cor);  // Vive até o fim do processo (ver cores.h).
  }
  return barra;
}

void AddColorMixer(ui::ColorProvider* provider,
                   const ui::ColorProviderKey& key) {
  // Janelas de app (PWA) seguem a cor do app; a barra na cor da página é
  // nossa e entra no fim.
  const Barra* barra = BarraDe(key.app_controller.get());
  if (key.app_controller && !barra) {
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
  // Inativas sem fundo nenhum (só o hover), como no Dia.
  mixer[kColorTabBackgroundInactiveFrameActive] = {SK_ColorTRANSPARENT};
  mixer[kColorTabBackgroundInactiveFrameInactive] = {SK_ColorTRANSPARENT};
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

  // "+ Nova aba" e os botões do topo da lateral: sem fundo, texto discreto
  // (o realce do hover vem do ink drop).
  mixer[kColorNewTabButtonCRBackgroundFrameActive] = {SK_ColorTRANSPARENT};
  mixer[kColorNewTabButtonCRBackgroundFrameInactive] = {SK_ColorTRANSPARENT};
  mixer[kColorTabSearchButtonCRForegroundFrameActive] = {p.texto_inativo};
  mixer[kColorTabSearchButtonCRForegroundFrameInactive] = {p.texto_inativo};

  // Destaque (barra de carregamento, foco, botões principais).
  mixer[ui::kColorAccent] = {p.destaque};

  if (barra) {
    barra->Pintar(mixer);
  }
}

}  // namespace navegador::cores
