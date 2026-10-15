#include "navegador/browser/ui/visual.h"

namespace navegador::visual {

std::optional<int> LayoutConstantOverride(LayoutConstant constant) {
  switch (constant) {
    // Abas da lateral: destaque de 30 com cantos 8 (medido no Dia rodando).
    case LayoutConstant::kVerticalTabHeight:
      return 30;
    case LayoutConstant::kVerticalTabPinnedHeight:
      return 41;
    case LayoutConstant::kVerticalTabCornerRadius:
      return 8;
    // "+ Nova aba": linha do tamanho de uma aba, ícone 16.
    case LayoutConstant::kVerticalTabStripNewTabButtonSize:
      return 30;
    case LayoutConstant::kVerticalTabStripButtonIconSize:
      return 16;
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
