#include "navegador/browser/ui/espacos.h"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "cc/paint/paint_flags.h"
#include "chrome/app/vector_icons/vector_icons.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/color/chrome_color_id.h"
#include "chrome/browser/ui/tabs/tab_enums.h"
#include "chrome/browser/ui/tabs/tab_group_model.h"
#include "chrome/browser/ui/tabs/tab_group_theme.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "chrome/browser/ui/tabs/vertical_tab_strip_state_controller.h"
#include "chrome/browser/ui/views/tabs/groups/tab_group_editor_bubble_view.h"
#include "components/tab_groups/tab_group_color.h"
#include "components/tab_groups/tab_group_id.h"
#include "components/tab_groups/tab_group_visual_data.h"
#include "components/tabs/public/tab_group.h"
#include "navegador/browser/containers/containers.h"
#include "navegador/browser/ui/containers/containers_ui.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/models/image_model.h"
#include "ui/color/color_provider.h"
#include "ui/events/event.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_list.h"
#include "ui/menus/simple_menu_model.h"
#include "ui/views/background.h"
#include "ui/views/context_menu_controller.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/image_button_factory.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/widget.h"

namespace navegador::espacos {

namespace {

// Medidas do seletor de espaços do Dia (docs/dia-medidas.md, seção 3).
constexpr int kAlturaFaixa = 38;
constexpr int kLarguraMaximaTitulo = 160;
constexpr int kEspacoEntreBolinhas = 3;
constexpr int kBolinha = 10;  // área clicável de cada bolinha
constexpr float kRaioBolinha = 3.0f;
constexpr float kRaioBolinhaAtual = 4.0f;

// Cores dos espaços novos, em rodízio.
constexpr tab_groups::TabGroupColorId kCoresNovas[] = {
    tab_groups::TabGroupColorId::kBlue,   tab_groups::TabGroupColorId::kPurple,
    tab_groups::TabGroupColorId::kGreen,  tab_groups::TabGroupColorId::kOrange,
    tab_groups::TabGroupColorId::kPink,   tab_groups::TabGroupColorId::kCyan,
    tab_groups::TabGroupColorId::kYellow, tab_groups::TabGroupColorId::kRed,
};

// Uma bolinha: um espaço. Clicar foca o espaço.
class Bolinha : public views::View {
  METADATA_HEADER(Bolinha, views::View)

 public:
  Bolinha(std::optional<tab_groups::TabGroupColorId> cor,
          bool atual,
          std::u16string nome,
          base::RepeatingClosure ao_clicar)
      : cor_(cor), atual_(atual), ao_clicar_(std::move(ao_clicar)) {
    SetTooltipText(nome);
    SetPreferredSize(gfx::Size(kBolinha, kBolinha));
  }

  void OnPaint(gfx::Canvas* canvas) override {
    const ui::ColorProvider* cores = GetColorProvider();
    if (!cores) {
      return;
    }
    SkColor cor =
        cor_ ? cores->GetColor(GetTabGroupTabStripColorId(*cor_, true))
             : cores->GetColor(kColorTabForegroundActiveFrameActive);
    if (!atual_) {
      cor = SkColorSetA(cor, 0x66);
    }
    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    flags.setColor(cor);
    canvas->DrawCircle(gfx::PointF(width() / 2.0f, height() / 2.0f),
                       atual_ ? kRaioBolinhaAtual : kRaioBolinha, flags);
  }

  bool OnMousePressed(const ui::MouseEvent& event) override {
    return event.IsOnlyLeftMouseButton();
  }

  void OnMouseReleased(const ui::MouseEvent& event) override {
    if (HitTestPoint(event.location())) {
      ao_clicar_.Run();
    }
  }

 private:
  std::optional<tab_groups::TabGroupColorId> cor_;
  bool atual_;
  base::RepeatingClosure ao_clicar_;
};

BEGIN_METADATA(Bolinha)
END_METADATA

class FaixaDeEspacos : public views::View,
                       public TabStripModelObserver,
                       public views::ContextMenuController,
                       public ui::SimpleMenuModel::Delegate {
  METADATA_HEADER(FaixaDeEspacos, views::View)

 public:
  explicit FaixaDeEspacos(BrowserWindowInterface* browser)
      : browser_(browser), model_(browser->GetTabStripModel()) {
    auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kHorizontal, gfx::Insets::VH(0, 10),
        8));
    layout->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kCenter);

    titulo_ = AddChildView(std::make_unique<views::Label>());
    titulo_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    titulo_->SetElideBehavior(gfx::ELIDE_TAIL);
    titulo_->SetMaximumWidthSingleLine(kLarguraMaximaTitulo);
    titulo_->SetFontList(
        gfx::FontList().DeriveWithSizeDelta(13 - gfx::FontList().GetFontSize())
            .DeriveWithWeight(gfx::Font::Weight::MEDIUM));
    layout->SetFlexForView(titulo_, 1);

    bolinhas_ = AddChildView(std::make_unique<views::View>());
    bolinhas_->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kHorizontal, gfx::Insets(),
        kEspacoEntreBolinhas));

    novo_ = AddChildView(views::CreateVectorImageButtonWithNativeTheme(
        base::BindRepeating(&FaixaDeEspacos::NovoEspaco,
                            base::Unretained(this)),
        kAddIcon, 14));
    novo_->SetTooltipText(u"Novo espaço");

    model_->AddObserver(this);
    set_context_menu_controller(this);

    if (auto* estado = tabs::VerticalTabStripStateController::From(browser)) {
      SetVisible(estado->GetCollapseState() ==
                 tabs::VerticalTabStripCollapseState::kExpanded);
      recolher_ = estado->RegisterOnCollapseChanged(base::BindRepeating(
          [](FaixaDeEspacos* faixa,
             tabs::VerticalTabStripCollapseState state) {
            // Recolhida (56 de largura) não cabe; volta ao expandir.
            faixa->SetVisible(state ==
                              tabs::VerticalTabStripCollapseState::kExpanded);
          },
          base::Unretained(this)));
    }

    Atualizar();
  }

  ~FaixaDeEspacos() override { model_->RemoveObserver(this); }

  // views::View:
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override {
    return gfx::Size(views::View::CalculatePreferredSize(available_size).width(),
                     kAlturaFaixa);
  }

  void OnThemeChanged() override {
    views::View::OnThemeChanged();
    titulo_->SetEnabledColor(
        GetColorProvider()->GetColor(kColorTabForegroundActiveFrameActive));
  }

  // A rodinha do mouse passa de um espaço pro outro (o "deslizar" do Dia).
  bool OnMouseWheel(const ui::MouseWheelEvent& event) override {
    const int delta = event.y_offset() != 0 ? event.y_offset() : event.x_offset();
    if (delta == 0) {
      return false;
    }
    Girar(delta < 0);
    return true;
  }

  // views::ContextMenuController: clique direito na faixa.
  void ShowContextMenuForViewImpl(
      views::View* source,
      const gfx::Point& point,
      ui::mojom::MenuSourceType source_type) override {
    menu_ = std::make_unique<ui::SimpleMenuModel>(this);
    if (model_->GetFocusedGroup()) {
      menu_->AddItem(kEditar, u"Editar espaço…");
      containers_ = containers::GetAll(browser_->GetProfile());
      submenu_containers_ = std::make_unique<ui::SimpleMenuModel>(this);
      submenu_containers_->AddCheckItem(kSemContainer, u"Nenhum");
      for (size_t i = 0; i < containers_.size(); ++i) {
        submenu_containers_->AddCheckItem(
            kPrimeiroContainer + i, base::UTF8ToUTF16(containers_[i].name));
        submenu_containers_->SetIcon(
            submenu_containers_->GetItemCount() - 1,
            containers::Bolinha(containers_[i].color));
      }
      menu_->AddSubMenu(kContainerDoEspaco, u"Container do espaço",
                        submenu_containers_.get());
      menu_->AddSeparator(ui::NORMAL_SEPARATOR);
    }
    menu_->AddItem(kNovo, u"Novo espaço");
    menu_runner_ = std::make_unique<views::MenuRunner>(
        menu_.get(), views::MenuRunner::CONTEXT_MENU);
    menu_runner_->RunMenuAt(GetWidget(), nullptr, gfx::Rect(point, gfx::Size()),
                            views::MenuAnchorPosition::kTopLeft, source_type);
  }

  // ui::SimpleMenuModel::Delegate:
  bool IsCommandIdChecked(int command_id) const override {
    const auto atual = model_->GetFocusedGroup();
    if (!atual) {
      return false;
    }
    const std::optional<std::string> container =
        containers::ContainerDoEspaco(browser_->GetProfile(), atual->ToString());
    if (command_id == kSemContainer) {
      return !container;
    }
    const size_t i = command_id - kPrimeiroContainer;
    return container && i < containers_.size() && containers_[i].id == *container;
  }

  void ExecuteCommand(int command_id, int event_flags) override {
    const auto atual = model_->GetFocusedGroup();
    if (command_id == kNovo) {
      NovoEspaco();
      return;
    }
    if (!atual) {
      return;
    }
    if (command_id == kEditar) {
      editor_ = TabGroupEditorBubbleView::Show(browser_, *atual, this,
                                               std::nullopt,
                                               /*stop_context_menu_propagation=*/
                                               false);
      return;
    }
    if (command_id == kSemContainer) {
      containers::DefinirContainerDoEspaco(browser_->GetProfile(),
                                           atual->ToString(), "");
      return;
    }
    const size_t i = command_id - kPrimeiroContainer;
    if (i < containers_.size()) {
      containers::DefinirContainerDoEspaco(
          browser_->GetProfile(), atual->ToString(), containers_[i].id);
    }
  }

  // TabStripModelObserver:
  void OnTabGroupFocusChanged(
      std::optional<tab_groups::TabGroupId> new_focused_group_id,
      std::optional<tab_groups::TabGroupId> old_focused_group_id) override {
    AtualizarDepois();
  }
  void OnTabGroupChanged(const TabGroupChange& change) override {
    AtualizarDepois();
  }
  void OnTabGroupAdded(const tab_groups::TabGroupId& group_id) override {
    AtualizarDepois();
  }
  void OnTabGroupWillBeRemoved(const tab_groups::TabGroupId& group_id) override {
    AtualizarDepois();
  }

 private:
  std::vector<tab_groups::TabGroupId> Espacos() const {
    return model_->group_model() ? model_->group_model()->ListTabGroups()
                                 : std::vector<tab_groups::TabGroupId>();
  }

  // Os avisos do modelo chegam no meio das mudanças (o grupo removido ainda
  // existe, por exemplo): redesenha depois.
  void AtualizarDepois() {
    if (atualizacao_pendente_) {
      return;
    }
    atualizacao_pendente_ = true;
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&FaixaDeEspacos::Atualizar,
                                  weak_factory_.GetWeakPtr()));
  }

  void Atualizar() {
    atualizacao_pendente_ = false;
    const std::optional<tab_groups::TabGroupId> atual =
        model_->GetFocusedGroup();
    bolinhas_->RemoveAllChildViews();

    bolinhas_->AddChildView(std::make_unique<Bolinha>(
        std::nullopt, !atual, u"Início",
        base::BindRepeating(&FaixaDeEspacos::Focar, base::Unretained(this),
                            std::nullopt)));
    std::u16string titulo = u"Início";
    for (const tab_groups::TabGroupId& id : Espacos()) {
      const TabGroup* grupo = model_->group_model()->GetTabGroup(id);
      if (!grupo) {
        continue;
      }
      std::u16string nome = grupo->visual_data()->title();
      if (nome.empty()) {
        nome = u"Espaço sem nome";
      }
      if (atual == id) {
        titulo = nome;
      }
      bolinhas_->AddChildView(std::make_unique<Bolinha>(
          grupo->visual_data()->color(), atual == id, nome,
          base::BindRepeating(&FaixaDeEspacos::Focar, base::Unretained(this),
                              std::optional<tab_groups::TabGroupId>(id))));
    }
    titulo_->SetText(titulo);
    InvalidateLayout();
    SchedulePaint();
  }

  void Focar(std::optional<tab_groups::TabGroupId> espaco) {
    if (espaco && !model_->group_model()->ContainsTabGroup(*espaco)) {
      return;
    }
    model_->SetFocusedGroup(espaco);
  }

  // Ordem: Início, depois os espaços na ordem das abas. Dá a volta.
  void Girar(bool pra_frente) {
    std::vector<std::optional<tab_groups::TabGroupId>> ordem = {std::nullopt};
    for (const auto& id : Espacos()) {
      ordem.push_back(id);
    }
    const auto atual = model_->GetFocusedGroup();
    size_t i = 0;
    while (i < ordem.size() && ordem[i] != atual) {
      ++i;
    }
    const size_t n = ordem.size();
    Focar(ordem[(i + (pra_frente ? 1 : n - 1)) % n]);
  }

  void NovoEspaco() {
    // A aba nova não pode cair dentro do espaço focado.
    model_->SetFocusedGroup(std::nullopt);
    chrome::NewTab(browser_, NewTabTypes::kNewTabCommand);
    const int index = model_->active_index();
    if (index == TabStripModel::kNoTab) {
      return;
    }
    const std::vector<tab_groups::TabGroupId> antes = Espacos();
    const tab_groups::TabGroupId espaco = model_->AddToNewGroup({index});
    const size_t n = antes.size();
    model_->ChangeTabGroupVisuals(
        espaco,
        tab_groups::TabGroupVisualData(
            u"Espaço " + base::NumberToString16(n + 1),
            kCoresNovas[n % std::size(kCoresNovas)]),
        /*is_customized=*/true);
    model_->SetFocusedGroup(espaco);
  }

  static constexpr int kEditar = 1;
  static constexpr int kNovo = 2;
  static constexpr int kContainerDoEspaco = 3;
  static constexpr int kSemContainer = 100;
  static constexpr int kPrimeiroContainer = 101;

  raw_ptr<BrowserWindowInterface> browser_;
  raw_ptr<TabStripModel> model_;
  std::unique_ptr<ui::SimpleMenuModel> menu_;
  std::unique_ptr<ui::SimpleMenuModel> submenu_containers_;
  std::unique_ptr<views::MenuRunner> menu_runner_;
  std::unique_ptr<views::Widget> editor_;
  std::vector<containers::Container> containers_;
  raw_ptr<views::Label> titulo_ = nullptr;
  raw_ptr<views::View> bolinhas_ = nullptr;
  raw_ptr<views::ImageButton> novo_ = nullptr;
  bool atualizacao_pendente_ = false;
  base::CallbackListSubscription recolher_;
  base::WeakPtrFactory<FaixaDeEspacos> weak_factory_{this};
};

BEGIN_METADATA(FaixaDeEspacos)
END_METADATA

}  // namespace

void AdicionarFaixa(views::View* lateral, BrowserWindowInterface* browser) {
  // Espaçador: ocupa o que sobra embaixo das abas e empurra a faixa pro pé.
  auto* espacador = lateral->AddChildView(std::make_unique<views::View>());
  espacador->SetProperty(
      views::kFlexBehaviorKey,
      views::FlexSpecification(views::MinimumFlexSizeRule::kScaleToZero,
                               views::MaximumFlexSizeRule::kUnbounded));
  lateral->AddChildView(std::make_unique<FaixaDeEspacos>(browser));
}

}  // namespace navegador::espacos
