#include "navegador/browser/ui/containers/containers_ui.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/paint/paint_flags.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/tabs/tab_enums.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/tabs/public/tab_interface.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "navegador/browser/containers/containers.h"
#include "ui/base/page_transition_types.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/geometry/rect_f.h"
#include "ui/gfx/image/canvas_image_source.h"
#include "ui/menus/simple_menu_model.h"
#include "ui/views/view.h"
#include "url/gurl.h"

namespace navegador::containers {

namespace {

// Paleta dos containers do Firefox (os nomes são os do pref).
struct CorNomeada {
  const char* nome;
  SkColor cor;
};
constexpr CorNomeada kCores[] = {
    {"azul", SkColorSetRGB(0x37, 0xAD, 0xFF)},
    {"turquesa", SkColorSetRGB(0x00, 0xC7, 0x9A)},
    {"verde", SkColorSetRGB(0x51, 0xCD, 0x00)},
    {"amarelo", SkColorSetRGB(0xFF, 0xCB, 0x00)},
    {"laranja", SkColorSetRGB(0xFF, 0x9F, 0x00)},
    {"vermelho", SkColorSetRGB(0xFF, 0x61, 0x3D)},
    {"rosa", SkColorSetRGB(0xFF, 0x4B, 0xDA)},
    {"roxo", SkColorSetRGB(0xAF, 0x51, 0xF5)},
    {"cinza", SkColorSetRGB(0x8F, 0x8F, 0x9D)},
};

class BolinhaSource : public gfx::CanvasImageSource {
 public:
  explicit BolinhaSource(SkColor cor)
      : gfx::CanvasImageSource(gfx::Size(16, 16)), cor_(cor) {}

  void Draw(gfx::Canvas* canvas) override {
    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    flags.setColor(cor_);
    canvas->DrawCircle(gfx::PointF(8, 8), 5, flags);
  }

 private:
  SkColor cor_;
};

std::optional<Container> ContainerDaAba(content::WebContents* contents) {
  if (!contents) {
    return std::nullopt;
  }
  std::optional<std::string> id = IdForWebContents(contents);
  if (!id) {
    return std::nullopt;
  }
  return Get(Profile::FromBrowserContext(contents->GetBrowserContext()), *id);
}

// "Abrir em container" no menu de contexto da aba.
class SubmenuDaAba : public ui::SimpleMenuModel,
                     public ui::SimpleMenuModel::Delegate {
 public:
  SubmenuDaAba(TabStripModel* tab_strip, int index)
      : ui::SimpleMenuModel(this),
        tab_strip_(tab_strip),
        alvo_(tab_strip->GetWebContentsAt(index)) {
    std::optional<Container> atual = ContainerDaAba(alvo_);
    containers_ = GetAll(tab_strip->profile());
    for (size_t i = 0; i < containers_.size(); ++i) {
      if (atual && atual->id == containers_[i].id) {
        continue;
      }
      AddItemWithIcon(kPrimeiro + i, base::UTF8ToUTF16(containers_[i].name),
                      Bolinha(containers_[i].color));
    }
    if (atual) {
      AddSeparator(ui::NORMAL_SEPARATOR);
      AddItem(kSemContainer, u"Sem container");
    }
  }

  // ui::SimpleMenuModel::Delegate:
  void ExecuteCommand(int command_id, int event_flags) override {
    const int index = tab_strip_->GetIndexOfWebContents(alvo_);
    if (index == TabStripModel::kNoTab) {
      return;
    }
    std::string id;
    if (command_id != kSemContainer) {
      const size_t i = command_id - kPrimeiro;
      if (i >= containers_.size()) {
        return;
      }
      id = containers_[i].id;
    }
    const GURL url = alvo_->GetLastCommittedURL().is_empty()
                         ? alvo_->GetVisibleURL()
                         : alvo_->GetLastCommittedURL();
    AbrirEmContainer(tab_strip_, url, id, index, /*ativar=*/true);
    tab_strip_->CloseWebContentsAt(index, TabCloseTypes::CLOSE_USER_GESTURE);
  }

 private:
  static constexpr int kPrimeiro = 1000;
  static constexpr int kSemContainer = 999;

  raw_ptr<TabStripModel> tab_strip_;
  raw_ptr<content::WebContents> alvo_;
  std::vector<Container> containers_;
};

// "Abrir link em container" no menu de contexto da página.
class SubmenuDoLink : public ui::SimpleMenuModel,
                      public ui::SimpleMenuModel::Delegate {
 public:
  SubmenuDoLink(TabStripModel* tab_strip,
                content::WebContents* origem,
                const GURL& link)
      : ui::SimpleMenuModel(this),
        tab_strip_(tab_strip),
        origem_(origem),
        link_(link),
        containers_(GetAll(tab_strip->profile())) {
    for (size_t i = 0; i < containers_.size(); ++i) {
      AddItemWithIcon(kPrimeiro + i, base::UTF8ToUTF16(containers_[i].name),
                      Bolinha(containers_[i].color));
    }
  }

  // ui::SimpleMenuModel::Delegate:
  void ExecuteCommand(int command_id, int event_flags) override {
    const size_t i = command_id - kPrimeiro;
    if (i >= containers_.size()) {
      return;
    }
    int index = tab_strip_->GetIndexOfWebContents(origem_);
    if (index == TabStripModel::kNoTab) {
      index = tab_strip_->count() - 1;
    }
    AbrirEmContainer(tab_strip_, link_, containers_[i].id, index,
                     /*ativar=*/false);
  }

 private:
  static constexpr int kPrimeiro = 1000;

  raw_ptr<TabStripModel> tab_strip_;
  raw_ptr<content::WebContents> origem_;
  GURL link_;
  std::vector<Container> containers_;
};

// "Nova aba em container" no menu do botão "Nova aba".
class SubmenuNovaAba : public ui::SimpleMenuModel,
                       public ui::SimpleMenuModel::Delegate {
 public:
  explicit SubmenuNovaAba(TabStripModel* tab_strip)
      : ui::SimpleMenuModel(this),
        tab_strip_(tab_strip),
        containers_(GetAll(tab_strip->profile())) {
    for (size_t i = 0; i < containers_.size(); ++i) {
      AddItemWithIcon(kPrimeiro + i, base::UTF8ToUTF16(containers_[i].name),
                      Bolinha(containers_[i].color));
    }
  }

  // ui::SimpleMenuModel::Delegate:
  void ExecuteCommand(int command_id, int event_flags) override {
    const size_t i = command_id - kPrimeiro;
    if (i >= containers_.size()) {
      return;
    }
    AbrirEmContainer(tab_strip_, GURL("chrome://newtab/"), containers_[i].id,
                     tab_strip_->count() - 1, /*ativar=*/true);
  }

 private:
  static constexpr int kPrimeiro = 1000;

  raw_ptr<TabStripModel> tab_strip_;
  std::vector<Container> containers_;
};

}  // namespace

SkColor Cor(std::string_view nome) {
  for (const CorNomeada& c : kCores) {
    if (nome == c.nome) {
      return c.cor;
    }
  }
  return kCores[0].cor;
}

ui::ImageModel Bolinha(std::string_view nome) {
  return ui::ImageModel::FromImageSkia(
      gfx::CanvasImageSource::MakeImageSkia<BolinhaSource>(Cor(nome)));
}

void PintarIndicador(gfx::Canvas* canvas,
                     const views::View& aba,
                     const tabs::TabInterface* tab) {
  if (!tab) {
    return;
  }
  std::optional<Container> container = ContainerDaAba(tab->GetContents());
  if (!container) {
    return;
  }
  // Barrinha de 3 na borda esquerda, com metade da altura da aba.
  const gfx::Rect bounds = aba.GetLocalBounds();
  const float altura = bounds.height() / 2.0f;
  const gfx::RectF barra(2, (bounds.height() - altura) / 2.0f, 3, altura);
  cc::PaintFlags flags;
  flags.setAntiAlias(true);
  flags.setColor(Cor(container->color));
  canvas->DrawRoundRect(barra, 1.5f, flags);
}

void AbrirEmContainer(TabStripModel* tab_strip,
                      const GURL& url,
                      std::string_view id,
                      int index,
                      bool ativar) {
  Profile* profile = tab_strip->profile();
  scoped_refptr<content::SiteInstance> site_instance =
      id.empty() ? content::SiteInstance::CreateForURL(profile, url)
                 : CreateSiteInstance(profile, url, id);
  content::WebContents::CreateParams params(profile, site_instance);
  params.initially_hidden = !ativar;
  const int inserida = tab_strip->InsertWebContentsAt(
      index + 1, content::WebContents::Create(params),
      ativar ? AddTabTypes::ADD_ACTIVE : AddTabTypes::ADD_NONE);
  content::NavigationController::LoadURLParams load(url);
  load.transition_type = ui::PAGE_TRANSITION_AUTO_TOPLEVEL;
  tab_strip->GetWebContentsAt(inserida)->GetController().LoadURLWithParams(
      load);
}

std::unique_ptr<ui::SimpleMenuModel> CriarSubmenuDaAba(TabStripModel* tab_strip,
                                                       int index) {
  return std::make_unique<SubmenuDaAba>(tab_strip, index);
}

std::unique_ptr<ui::SimpleMenuModel> CriarSubmenuDoLink(
    TabStripModel* tab_strip,
    content::WebContents* origem,
    const GURL& link) {
  return std::make_unique<SubmenuDoLink>(tab_strip, origem, link);
}

std::unique_ptr<ui::SimpleMenuModel> CriarSubmenuNovaAba(
    TabStripModel* tab_strip) {
  return std::make_unique<SubmenuNovaAba>(tab_strip);
}

}  // namespace navegador::containers
