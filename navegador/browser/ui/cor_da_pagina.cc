#include "navegador/browser/ui/cor_da_pagina.h"

#include <map>
#include <memory>

#include "base/memory/raw_ptr.h"
#include "base/no_destructor.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/widget.h"

namespace navegador::cor_da_pagina {

namespace {

std::map<const BrowserView*, std::optional<SkColor>>& Cores() {
  static base::NoDestructor<std::map<const BrowserView*, std::optional<SkColor>>>
      cores;
  return *cores;
}

// Invisível, fora do layout: só existe pra viver junto com a janela.
class Observador : public views::View,
                   public TabStripModelObserver,
                   public content::WebContentsObserver {
  METADATA_HEADER(Observador, views::View)

 public:
  explicit Observador(BrowserView* browser_view)
      : browser_view_(browser_view),
        model_(browser_view->browser()->GetTabStripModel()) {
    SetVisible(false);
    SetProperty(views::kViewIgnoredByLayoutKey, true);
    model_->AddObserver(this);
    Observe(model_->GetActiveWebContents());
  }

  ~Observador() override { Cores().erase(browser_view_); }

  // views::View:
  void AddedToWidget() override { Atualizar(); }

  // TabStripModelObserver:
  void OnTabStripModelChanged(
      TabStripModel* tab_strip_model,
      const TabStripModelChange& change,
      const TabStripSelectionChange& selection) override {
    if (selection.active_tab_changed()) {
      Observe(model_->GetActiveWebContents());
      Atualizar();
    }
  }

  // content::WebContentsObserver:
  void DidChangeThemeColor() override { Atualizar(); }
  void OnBackgroundColorChanged() override { Atualizar(); }
  void PrimaryPageChanged(content::Page& page) override { Atualizar(); }

 private:
  void Atualizar() {
    std::optional<SkColor> cor;
    if (content::WebContents* contents = web_contents()) {
      cor = contents->GetThemeColor();
      if (!cor) {
        cor = contents->GetBackgroundColor();
      }
      // Transparente não é cor de página.
      if (cor && SkColorGetA(*cor) < 0x80) {
        cor.reset();
      }
    }
    auto& cores = Cores();
    auto it = cores.find(browser_view_);
    if (it != cores.end() && it->second == cor) {
      return;
    }
    cores[browser_view_] = cor;
    if (views::Widget* widget = GetWidget()) {
      widget->ThemeChanged();
    }
  }

  raw_ptr<BrowserView> browser_view_;
  raw_ptr<TabStripModel> model_;
};

BEGIN_METADATA(Observador)
END_METADATA

}  // namespace

std::optional<SkColor> CorDaBarra(const BrowserView* browser_view) {
  auto& cores = Cores();
  auto it = cores.find(browser_view);
  return it == cores.end() ? std::nullopt : it->second;
}

void Acompanhar(views::View* dono, BrowserView* browser_view) {
  dono->AddChildView(std::make_unique<Observador>(browser_view));
}

}  // namespace navegador::cor_da_pagina
