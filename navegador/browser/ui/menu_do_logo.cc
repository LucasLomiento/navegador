#include "navegador/browser/ui/menu_do_logo.h"

#include <map>
#include <memory>

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/memory/raw_ptr.h"
#include "base/no_destructor.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/tabs/vertical_tab_strip_state_controller.h"
#include "chrome/browser/ui/toolbar/app_menu_model.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/toolbar/app_menu.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "ui/base/class_property.h"
#include "ui/base/mojom/menu_source_type.mojom.h"
#include "ui/events/event.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/widget/widget.h"

namespace navegador::menu_do_logo {
namespace {

class MenuDoLogo;

// Logo de cada janela, pro Alt+F achar.
std::map<BrowserWindowInterface*, MenuDoLogo*>& Logos() {
  static base::NoDestructor<std::map<BrowserWindowInterface*, MenuDoLogo*>>
      logos;
  return *logos;
}

// Mora no botão do logo (propriedade dona) e morre com ele.
class MenuDoLogo {
 public:
  MenuDoLogo(views::Button* logo, BrowserWindowInterface* browser)
      : logo_(logo), browser_(browser) {
    Logos()[browser_] = this;
  }
  MenuDoLogo(const MenuDoLogo&) = delete;
  MenuDoLogo& operator=(const MenuDoLogo&) = delete;
  ~MenuDoLogo() {
    // O menu antes do modelo (o AppMenu guarda ponteiro pro modelo).
    menu_.reset();
    modelo_.reset();
    auto it = Logos().find(browser_);
    if (it != Logos().end() && it->second == this) {
      Logos().erase(it);
    }
  }

  bool Visivel() const { return logo_->IsDrawn() && logo_->GetWidget(); }

  void Abrir(bool pelo_teclado) {
    if (menu_ && menu_->IsShowing()) {
      menu_->CloseMenu();
      return;
    }
    BrowserView* browser_view =
        BrowserView::GetBrowserViewForBrowser(browser_);
    if (!browser_view || !browser_view->toolbar()) {
      return;
    }
    ToolbarView* toolbar = browser_view->toolbar();

    menu_.reset();
    modelo_ = std::make_unique<AppMenuModel>(
        toolbar, browser_, toolbar->app_menu_icon_controller());
    modelo_->Init();

    int tipos = 0;
    if (pelo_teclado) {
      tipos |= views::MenuRunner::SHOULD_SHOW_MNEMONICS |
               views::MenuRunner::INVOKED_FROM_KEYBOARD;
    }
    // O AppMenu chama o callback ao fechar, sem checar se é nulo.
    menu_ = std::make_unique<AppMenu>(browser_, modelo_.get(), tipos,
                                      base::DoNothing());
    menu_->RunMenu(logo_->GetWidget(), logo_->GetAnchorBoundsInScreen(),
                   pelo_teclado ? ui::mojom::MenuSourceType::kKeyboard
                                : ui::mojom::MenuSourceType::kMouse,
                   views::MenuAnchorPosition::kTopLeft);
  }

 private:
  raw_ptr<views::Button> logo_;
  raw_ptr<BrowserWindowInterface> browser_;
  std::unique_ptr<AppMenuModel> modelo_;
  std::unique_ptr<AppMenu> menu_;
};

}  // namespace
}  // namespace navegador::menu_do_logo

DEFINE_UI_CLASS_PROPERTY_TYPE(navegador::menu_do_logo::MenuDoLogo*)

namespace navegador::menu_do_logo {
namespace {

DEFINE_OWNED_UI_CLASS_PROPERTY_KEY(MenuDoLogo, kMenuDoLogoKey)

}  // namespace

void Ligar(views::Button* logo, BrowserWindowInterface* browser) {
  MenuDoLogo* menu = logo->SetProperty(
      kMenuDoLogoKey, std::make_unique<MenuDoLogo>(logo, browser));
  logo->SetCallback(base::BindRepeating(
      [](MenuDoLogo* menu, BrowserWindowInterface* browser,
         const ui::Event& event) {
        // Recolhida, o logo continua abrindo a lateral.
        auto* estado = tabs::VerticalTabStripStateController::From(browser);
        if (estado && estado->IsCollapsed()) {
          chrome::ExecuteCommand(browser, IDC_TOGGLE_VERTICAL_TABS_COLLAPSE);
          return;
        }
        menu->Abrir(event.IsKeyEvent());
      },
      base::Unretained(menu), base::Unretained(browser)));
}

bool Abrir(BrowserWindowInterface* browser, bool pelo_teclado) {
  auto it = Logos().find(browser);
  if (it == Logos().end() || !it->second->Visivel()) {
    return false;
  }
  it->second->Abrir(pelo_teclado);
  return true;
}

}  // namespace navegador::menu_do_logo
