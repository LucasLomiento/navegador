#include "navegador/browser/ui/containers/espaco.h"

#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/tab_groups/tab_group_id.h"

namespace navegador::containers {

std::optional<std::string> EspacoFocado(BrowserWindowInterface* browser) {
  if (!browser || !browser->GetTabStripModel()) {
    return std::nullopt;
  }
  std::optional<tab_groups::TabGroupId> grupo =
      browser->GetTabStripModel()->GetFocusedGroup();
  if (!grupo) {
    return std::nullopt;
  }
  return grupo->ToString();
}

}  // namespace navegador::containers
