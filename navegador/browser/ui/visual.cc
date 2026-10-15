#include "navegador/browser/ui/visual.h"

namespace navegador::visual {

std::optional<int> LayoutConstantOverride(LayoutConstant constant) {
  switch (constant) {
    // Abas da lateral: linha de 32 (a linha de grupo do Dia tem mínimo 32),
    // cantos 6 (os do botão "Nova aba" e do seletor de espaços).
    case LayoutConstant::kVerticalTabHeight:
    case LayoutConstant::kVerticalTabPinnedHeight:
      return 32;
    case LayoutConstant::kVerticalTabCornerRadius:
      return 6;
    // Recuo lateral da lista do Dia: 8 de cada lado.
    case LayoutConstant::kVerticalTabStripHorizontalPadding:
      return 8;
    case LayoutConstant::kVerticalTabStripUncollapsedVerticalPadding:
      return 8;

    // Barra de navegação do Dia: botões 32×32 com ícone 16, barra de
    // endereço com 32 de altura.
    case LayoutConstant::kToolbarButtonHeight:
      return 32;
    case LayoutConstant::kToolbarButtonIconSize:
      return 16;
    case LayoutConstant::kLocationBarHeight:
      return 32;

    // Cantos do cartão de conteúdo.
    case LayoutConstant::kToolbarCornerRadius:
    case LayoutConstant::kMainBackgroundRegionCornerRadius:
      return kCartaoRaio;

    default:
      return std::nullopt;
  }
}

std::optional<gfx::Insets> LayoutInsetsOverride(LayoutInset inset) {
  switch (inset) {
    // Botão de 32 com ícone de 16.
    case TOOLBAR_BUTTON:
      return gfx::Insets(8);
    // Barra do Dia: 4 em cima, embaixo e nas laterais -> 40 de altura.
    case TOOLBAR_INTERIOR_MARGIN:
      return gfx::Insets(4);
    default:
      return std::nullopt;
  }
}

}  // namespace navegador::visual
