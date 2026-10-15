#include "navegador/browser/ui/abas/abas.h"

#include "cc/paint/paint_flags.h"
#include "chrome/browser/ui/color/chrome_color_id.h"
#include "chrome/browser/ui/layout_constants.h"
#include "ui/color/color_id.h"
#include "ui/color/color_provider.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/views/view.h"

namespace navegador::abas {

void PintarLadrilho(gfx::Canvas* canvas, const views::View& aba, bool fixada) {
  const ui::ColorProvider* cores = aba.GetColorProvider();
  if (!fixada || !cores) {
    return;
  }
  // Claro: branco translúcido (mais claro que a lateral). Escuro: um véu de
  // branco bem leve.
  const bool escuro =
      color_utils::IsDark(cores->GetColor(ui::kColorFrameActive));
  const SkColor fundo = escuro ? SkColorSetARGB(0x10, 0xFF, 0xFF, 0xFF)
                               : SkColorSetARGB(0x99, 0xFF, 0xFF, 0xFF);
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setColor(fundo);
  canvas->DrawRoundRect(
      gfx::RectF(aba.GetLocalBounds()),
      GetLayoutConstant(LayoutConstant::kVerticalTabCornerRadius), flags);
}

}  // namespace navegador::abas
