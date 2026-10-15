#include "navegador/browser/ui/containers/gerenciador.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/functional/callback_helpers.h"
#include "base/memory/raw_ptr.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/paint/paint_flags.h"
#include "chrome/app/vector_icons/vector_icons.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_contents.h"
#include "navegador/browser/containers/containers.h"
#include "navegador/browser/ui/containers/containers_ui.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/mojom/dialog_button.mojom.h"
#include "ui/base/mojom/ui_base_types.mojom-shared.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/image_button_factory.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/dialog_delegate.h"

namespace navegador::containers {

namespace {

// Ordem do rodízio de cores (clicar na bolinha passa pra próxima).
constexpr const char* kPaleta[] = {"azul",    "turquesa", "verde",
                                   "amarelo", "laranja",  "vermelho",
                                   "rosa",    "roxo",     "cinza"};

const char* ProximaCor(const std::string& atual) {
  for (size_t i = 0; i < std::size(kPaleta); ++i) {
    if (atual == kPaleta[i]) {
      return kPaleta[(i + 1) % std::size(kPaleta)];
    }
  }
  return kPaleta[0];
}

// Bolinha da cor do container; clicar troca a cor.
class BotaoCor : public views::Button {
  METADATA_HEADER(BotaoCor, views::Button)

 public:
  BotaoCor(std::string cor, PressedCallback ao_clicar)
      : views::Button(std::move(ao_clicar)), cor_(std::move(cor)) {
    SetPreferredSize(gfx::Size(24, 24));
    SetTooltipText(u"Trocar a cor");
  }

  void PaintButtonContents(gfx::Canvas* canvas) override {
    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    flags.setColor(Cor(cor_));
    canvas->DrawCircle(gfx::PointF(width() / 2.0f, height() / 2.0f), 7, flags);
  }

 private:
  std::string cor_;
};

BEGIN_METADATA(BotaoCor)
END_METADATA

class Gerenciador : public views::DialogDelegate,
                    public views::TextfieldController {
 public:
  explicit Gerenciador(Profile* profile) : profile_(profile) {
    SetTitle(u"Containers");
    SetModalType(ui::mojom::ModalType::kWindow);
    SetShowCloseButton(true);
    SetButtons(static_cast<int>(ui::mojom::DialogButton::kOk));
    SetButtonLabel(ui::mojom::DialogButton::kOk, u"Fechar");

    auto conteudo = std::make_unique<views::View>();
    conteudo->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets::VH(12, 16),
        12));
    lista_ = conteudo->AddChildView(std::make_unique<views::View>());
    lista_->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets(), 6));
    conteudo->AddChildView(std::make_unique<views::MdTextButton>(
        base::BindRepeating(&Gerenciador::Novo, base::Unretained(this)),
        u"Novo container"));
    SetContentsView(std::move(conteudo));
    Reconstruir();
  }

  // views::TextfieldController: renomeia enquanto digita.
  void ContentsChanged(views::Textfield* sender,
                       const std::u16string& new_contents) override {
    for (const Linha& linha : linhas_) {
      if (linha.nome == sender) {
        if (std::optional<Container> c = Get(profile_, linha.id)) {
          c->name = base::UTF16ToUTF8(new_contents);
          Update(profile_, *c);
        }
        return;
      }
    }
  }

 private:
  struct Linha {
    std::string id;
    raw_ptr<views::Textfield> nome;
  };

  void Reconstruir(const std::string& focar = std::string()) {
    linhas_.clear();
    lista_->RemoveAllChildViews();
    for (const Container& c : GetAll(profile_)) {
      auto* linha = lista_->AddChildView(std::make_unique<views::View>());
      auto* layout =
          linha->SetLayoutManager(std::make_unique<views::BoxLayout>(
              views::BoxLayout::Orientation::kHorizontal, gfx::Insets(), 8));
      layout->set_cross_axis_alignment(
          views::BoxLayout::CrossAxisAlignment::kCenter);

      linha->AddChildView(std::make_unique<BotaoCor>(
          c.color, base::BindRepeating(&Gerenciador::TrocarCor,
                                       base::Unretained(this), c.id)));
      auto* nome = linha->AddChildView(std::make_unique<views::Textfield>());
      nome->SetText(base::UTF8ToUTF16(c.name));
      nome->SetDefaultWidthInChars(18);
      nome->SetAccessibleName(u"Nome do container");
      nome->set_controller(this);
      layout->SetFlexForView(nome, 1);
      auto* excluir =
          linha->AddChildView(views::CreateVectorImageButtonWithNativeTheme(
              base::BindRepeating(&Gerenciador::Excluir,
                                  base::Unretained(this), c.id),
              kDeleteIcon, 16));
      excluir->SetTooltipText(u"Excluir container");
      linhas_.push_back({c.id, nome});
      if (c.id == focar) {
        nome->RequestFocus();
        nome->SelectAll(false);
      }
    }
    if (GetWidget()) {
      GetWidget()->SetSize(GetWidget()->non_client_view()->GetPreferredSize());
    }
  }

  void Novo() {
    const std::vector<Container> todos = GetAll(profile_);
    const std::string cor = todos.empty() ? kPaleta[0]
                                          : ProximaCor(todos.back().color);
    Reconstruir(Add(profile_, "Novo container", cor, "").id);
  }

  void TrocarCor(std::string id) {
    if (std::optional<Container> c = Get(profile_, id)) {
      c->color = ProximaCor(c->color);
      Update(profile_, *c);
      Reconstruir();
    }
  }

  // Apaga os dados do container (cookies, armazenamento, cache) e tira da
  // lista. Abas abertas nele continuam abertas, sem login.
  void Excluir(std::string id) {
    ApagarDados(profile_, id, base::DoNothing());
    Remove(profile_, id);
    Reconstruir();
  }

  raw_ptr<Profile> profile_;
  raw_ptr<views::View> lista_ = nullptr;
  std::vector<Linha> linhas_;
};

}  // namespace

void MostrarGerenciador(content::WebContents* aba) {
  if (!aba) {
    return;
  }
  Profile* profile = Profile::FromBrowserContext(aba->GetBrowserContext());
  views::Widget* widget = views::DialogDelegate::CreateDialogWidget(
      std::make_unique<Gerenciador>(profile), /*context=*/nullptr,
      /*parent=*/aba->GetTopLevelNativeWindow());
  widget->Show();
}

}  // namespace navegador::containers
