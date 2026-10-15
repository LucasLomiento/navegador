#include "navegador/browser/ui/configuracoes.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/no_destructor.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "cc/paint/paint_flags.h"
#include "chrome/app/vector_icons/vector_icons.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "components/constrained_window/constrained_window_views.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "components/vector_icons/vector_icons.h"
#include "navegador/browser/containers/containers.h"
#include "navegador/browser/containers/espacos_dados.h"
#include "navegador/browser/ui/atalhos.h"
#include "navegador/browser/ui/containers/containers_ui.h"
#include "navegador/browser/ui/containers/gerenciador.h"
#include "navegador/browser/ui/espacos.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/models/image_model.h"
#include "ui/base/models/simple_combobox_model.h"
#include "ui/base/mojom/dialog_button.mojom.h"
#include "ui/base/mojom/ui_base_types.mojom-shared.h"
#include "ui/color/color_id.h"
#include "ui/events/event.h"
#include "ui/events/keycodes/keyboard_codes.h"
#include "ui/gfx/canvas.h"
#include "ui/views/animation/ink_drop.h"
#include "ui/views/background.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/button/image_button_factory.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/combobox/combobox.h"
#include "ui/views/controls/highlight_path_generator.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/scroll_view.h"
#include "ui/views/controls/tabbed_pane/tabbed_pane.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/window/dialog_delegate.h"

namespace navegador::configuracoes {

namespace {

constexpr const char* kPaleta[] = {"azul",    "roxo",     "verde",
                                   "laranja", "rosa",     "turquesa",
                                   "amarelo", "vermelho", "cinza"};

gfx::NativeWindow JanelaDe(BrowserWindowInterface* browser) {
  BrowserView* view = BrowserView::GetBrowserViewForBrowser(browser);
  return view && view->GetWidget() ? view->GetWidget()->GetNativeWindow()
                                   : gfx::NativeWindow();
}

// No Wayland o diálogo é uma subsuperfície da janela: só aparece quando a
// janela manda um quadro novo. Aberto por atalho (sem menu fechando por
// cima), nada redesenha a janela e o diálogo ficava invisível com o foco.
void MostrarSobre(views::Widget* dialogo, BrowserWindowInterface* browser) {
  dialogo->Show();
  BrowserView* view = BrowserView::GetBrowserViewForBrowser(browser);
  if (view && view->GetWidget() && view->GetWidget()->GetRootView()) {
    view->GetWidget()->GetRootView()->SchedulePaint();
  }
}

// "+ Algo" como linha de texto, como o "+ Nova guia" da lateral.
std::unique_ptr<views::LabelButton> BotaoAdicionar(
    std::u16string texto,
    views::Button::PressedCallback ao_clicar) {
  auto botao =
      std::make_unique<views::LabelButton>(std::move(ao_clicar), texto);
  botao->SetImageModel(
      views::Button::STATE_NORMAL,
      ui::ImageModel::FromVectorIcon(kAddIcon, ui::kColorIconSecondary, 16));
  botao->SetImageLabelSpacing(20);
  botao->SetBorder(views::CreateEmptyBorder(gfx::Insets::VH(4, 6)));
  botao->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  views::InkDrop::Get(botao.get())
      ->SetMode(views::InkDropHost::InkDropMode::ON);
  views::InkDrop::Get(botao.get())->SetBaseColor(ui::kColorSysOnSurface);
  views::InstallRoundRectHighlightPathGenerator(botao.get(), gfx::Insets(), 6);
  return botao;
}

std::unique_ptr<views::Label> Legenda(std::u16string texto) {
  auto legenda = std::make_unique<views::Label>(texto);
  legenda->SetMultiLine(true);
  legenda->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  legenda->SetEnabledColor(ui::kColorLabelForegroundSecondary);
  legenda->SizeToFit(460);
  return legenda;
}

// Texto solto, editável no lugar (o anel de foco aparece ao clicar).
void Discreto(views::Textfield* campo) {
  campo->SetBorder(views::CreateEmptyBorder(gfx::Insets::VH(4, 6)));
  campo->SetBackgroundEnabled(false);
  campo->RemoveHoverEffect();
}

// Bolinha de cor; `marcada` desenha o anel em volta.
class BolinhaDeCor : public views::Button {
  METADATA_HEADER(BolinhaDeCor, views::Button)

 public:
  BolinhaDeCor(std::string cor, bool marcada, PressedCallback ao_clicar)
      : views::Button(std::move(ao_clicar)),
        cor_(std::move(cor)),
        marcada_(marcada) {
    SetPreferredSize(gfx::Size(28, 28));
    SetTooltipText(base::UTF8ToUTF16(cor_));
  }

  void SetMarcada(bool marcada) {
    marcada_ = marcada;
    SchedulePaint();
  }

  void PaintButtonContents(gfx::Canvas* canvas) override {
    const gfx::PointF centro(width() / 2.0f, height() / 2.0f);
    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    if (marcada_ || IsMouseHovered() || HasFocus()) {
      flags.setStyle(cc::PaintFlags::kStroke_Style);
      flags.setStrokeWidth(1.5f);
      flags.setColor(SkColorSetA(containers::Cor(cor_), marcada_ ? 0xFF : 0x80));
      canvas->DrawCircle(centro, 11, flags);
      flags.setStyle(cc::PaintFlags::kFill_Style);
    }
    flags.setColor(containers::Cor(cor_));
    canvas->DrawCircle(centro, 7, flags);
  }

  void OnMouseEntered(const ui::MouseEvent& event) override {
    views::Button::OnMouseEntered(event);
    SchedulePaint();
  }
  void OnMouseExited(const ui::MouseEvent& event) override {
    views::Button::OnMouseExited(event);
    SchedulePaint();
  }

 private:
  std::string cor_;
  bool marcada_;
};

BEGIN_METADATA(BolinhaDeCor)
END_METADATA

// Itens do seletor de container: "Sem container" e os do perfil.
std::unique_ptr<ui::SimpleComboboxModel> ModeloDeContainers(
    const std::vector<containers::Container>& lista) {
  std::vector<ui::SimpleComboboxModel::Item> itens;
  itens.emplace_back(u"Sem container");
  for (const containers::Container& container : lista) {
    itens.emplace_back(base::UTF8ToUTF16(container.name), std::u16string(),
                       containers::Bolinha(container.color));
  }
  return std::make_unique<ui::SimpleComboboxModel>(std::move(itens));
}

size_t IndiceDoContainer(const std::vector<containers::Container>& lista,
                         const std::string& id) {
  for (size_t i = 0; i < lista.size(); ++i) {
    if (lista[i].id == id) {
      return i + 1;
    }
  }
  return 0;
}

// --- Aba Espaços ------------------------------------------------------------

class PainelDeEspacos : public views::View,
                        public views::TextfieldController {
  METADATA_HEADER(PainelDeEspacos, views::View)

 public:
  PainelDeEspacos(BrowserWindowInterface* browser)
      : browser_(browser), perfil_(browser->GetProfile()) {
    SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets::VH(16, 16),
        10));
    AddChildView(Legenda(
        u"Cada espaço tem as próprias abas e um container (cookies e logins "
        u"separados). Troque de espaço pelos atalhos, pelas bolinhas no pé "
        u"da lateral, rolando de lado ou com os botões laterais do mouse em "
        u"cima da lateral."));
    lista_ = AddChildView(std::make_unique<views::View>());
    lista_->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets(), 2));
    AddChildView(BotaoAdicionar(
        u"Novo espaço",
        base::BindRepeating(&PainelDeEspacos::Novo, base::Unretained(this))));

    prefs_.Init(perfil_->GetPrefs());
    prefs_.Add(espacos::kListaPref,
               base::BindRepeating(&PainelDeEspacos::QuandoMudar,
                                   base::Unretained(this)));
    prefs_.Add(containers::kContainersPref,
               base::BindRepeating(&PainelDeEspacos::Reconstruir,
                                   base::Unretained(this)));
    Reconstruir();
  }

  // views::TextfieldController: renomeia (ou troca o emoji) enquanto digita.
  void ContentsChanged(views::Textfield* sender,
                       const std::u16string& new_contents) override {
    for (const Linha& linha : linhas_) {
      if (linha.nome != sender && linha.icone != sender) {
        continue;
      }
      if (std::optional<espacos::Espaco> espaco =
              espacos::Get(perfil_, linha.id)) {
        if (linha.nome == sender) {
          espaco->nome = base::UTF16ToUTF8(new_contents);
        } else {
          espaco->icone = base::UTF16ToUTF8(new_contents);
        }
        editando_ = true;
        espacos::Atualizar(perfil_, *espaco);
        editando_ = false;
      }
      return;
    }
  }

 private:
  struct Linha {
    std::string id;
    raw_ptr<views::Textfield> icone;
    raw_ptr<views::Textfield> nome;
  };

  // Mudou por fora (outra janela, a faixa): redesenha, menos quando é a
  // própria digitação daqui (perderia o foco do campo).
  void QuandoMudar() {
    if (!editando_) {
      Reconstruir();
    }
  }

  void Reconstruir() {
    linhas_.clear();
    lista_->RemoveAllChildViews();
    containers_ = containers::GetAll(perfil_);
    const std::vector<espacos::Espaco> todos = espacos::Todos(perfil_);
    for (size_t i = 0; i < todos.size(); ++i) {
      const espacos::Espaco& espaco = todos[i];
      auto* linha = lista_->AddChildView(std::make_unique<views::View>());
      auto* layout =
          linha->SetLayoutManager(std::make_unique<views::BoxLayout>(
              views::BoxLayout::Orientation::kHorizontal, gfx::Insets(), 6));
      layout->set_cross_axis_alignment(
          views::BoxLayout::CrossAxisAlignment::kCenter);

      linha->AddChildView(std::make_unique<BolinhaDeCor>(
          espaco.cor, /*marcada=*/false,
          base::BindRepeating(&PainelDeEspacos::TrocarCor,
                              base::Unretained(this), espaco.id)));

      auto* icone = linha->AddChildView(std::make_unique<views::Textfield>());
      icone->SetText(base::UTF8ToUTF16(espaco.icone));
      icone->SetPlaceholderText(u"＋");
      icone->SetDefaultWidthInChars(2);
      icone->SetAccessibleName(u"Emoji do espaço");
      icone->set_controller(this);
      Discreto(icone);

      auto* nome = linha->AddChildView(std::make_unique<views::Textfield>());
      nome->SetText(base::UTF8ToUTF16(espaco.nome));
      nome->SetDefaultWidthInChars(14);
      nome->SetAccessibleName(u"Nome do espaço");
      nome->set_controller(this);
      Discreto(nome);
      layout->SetFlexForView(nome, 1);

      auto* container = linha->AddChildView(
          std::make_unique<views::Combobox>(ModeloDeContainers(containers_)));
      container->SetAccessibleName(u"Container do espaço");
      container->SetSelectedIndex(
          IndiceDoContainer(containers_, espaco.container));
      container->SetCallback(base::BindRepeating(
          &PainelDeEspacos::TrocarContainer, base::Unretained(this),
          espaco.id, base::Unretained(container)));

      auto* subir =
          linha->AddChildView(views::CreateVectorImageButtonWithNativeTheme(
              base::BindRepeating(&PainelDeEspacos::Mover,
                                  base::Unretained(this), espaco.id, i, -1),
              vector_icons::kKeyboardArrowUpIcon, 16));
      subir->SetTooltipText(u"Subir");
      subir->SetEnabled(i > 0);
      auto* descer =
          linha->AddChildView(views::CreateVectorImageButtonWithNativeTheme(
              base::BindRepeating(&PainelDeEspacos::Mover,
                                  base::Unretained(this), espaco.id, i, 1),
              vector_icons::kKeyboardArrowDownIcon, 16));
      descer->SetTooltipText(u"Descer");
      descer->SetEnabled(i + 1 < todos.size());
      auto* excluir =
          linha->AddChildView(views::CreateVectorImageButtonWithNativeTheme(
              base::BindRepeating(&PainelDeEspacos::Excluir,
                                  base::Unretained(this), espaco.id),
              kDeleteIcon, 16));
      excluir->SetTooltipText(u"Excluir espaço (fecha as abas dele)");
      excluir->SetEnabled(todos.size() > 1);

      linhas_.push_back({espaco.id, icone, nome});
    }
    PreferredSizeChanged();
  }

  void Novo() {
    EditarEspaco(browser_, std::string(),
                 base::BindOnce(
                     [](base::WeakPtr<PainelDeEspacos> painel,
                        BrowserWindowInterface* browser,
                        const std::string& espaco) {
                       espacos::IrPara(browser, espaco);
                     },
                     weak_factory_.GetWeakPtr(), browser_));
  }

  void TrocarCor(std::string id) {
    std::optional<espacos::Espaco> espaco = espacos::Get(perfil_, id);
    if (!espaco) {
      return;
    }
    size_t i = 0;
    while (i < std::size(kPaleta) && espaco->cor != kPaleta[i]) {
      ++i;
    }
    espaco->cor = kPaleta[(i + 1) % std::size(kPaleta)];
    espacos::Atualizar(perfil_, *espaco);
  }

  void TrocarContainer(std::string id, views::Combobox* combobox) {
    std::optional<espacos::Espaco> espaco = espacos::Get(perfil_, id);
    const std::optional<size_t> escolhido = combobox->GetSelectedIndex();
    if (!espaco || !escolhido) {
      return;
    }
    espaco->container =
        *escolhido == 0 ? std::string() : containers_[*escolhido - 1].id;
    editando_ = true;
    espacos::Atualizar(perfil_, *espaco);
    editando_ = false;
  }

  void Mover(std::string id, size_t posicao, int sentido) {
    if (sentido < 0 && posicao == 0) {
      return;
    }
    espacos::Mover(perfil_, id, posicao + sentido);
  }

  // Como no Zen, pergunta antes: as abas e os fixados do espaço vão junto.
  void Excluir(std::string id) {
    std::optional<espacos::Espaco> espaco = espacos::Get(perfil_, id);
    if (!espaco) {
      return;
    }
    auto dialogo = std::make_unique<views::DialogDelegate>();
    dialogo->SetTitle(u"Excluir o espaço \u201C" +
                      base::UTF8ToUTF16(espaco->nome) + u"\u201D?");
    dialogo->SetModalType(ui::mojom::ModalType::kWindow);
    dialogo->SetShowCloseButton(true);
    dialogo->SetButtonLabel(ui::mojom::DialogButton::kOk, u"Excluir");
    dialogo->SetButtonLabel(ui::mojom::DialogButton::kCancel, u"Cancelar");
    dialogo->set_fixed_width(380);
    auto aviso = std::make_unique<views::Label>(
        u"As abas dele fecham em todas as janelas, e os favoritos, sites "
        u"fixados e pastas dele são apagados. O container continua.");
    aviso->SetMultiLine(true);
    aviso->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    aviso->SetBorder(views::CreateEmptyBorder(gfx::Insets::TLBR(4, 20, 8, 20)));
    dialogo->SetContentsView(std::move(aviso));
    dialogo->SetAcceptCallback(base::BindOnce(
        [](Profile* perfil, std::string id) { espacos::Excluir(perfil, id); },
        perfil_.get(), id));
    MostrarSobre(constrained_window::CreateBrowserModalDialogViews(
                     std::move(dialogo), JanelaDe(browser_)),
                 browser_);
  }

  raw_ptr<BrowserWindowInterface> browser_;
  raw_ptr<Profile> perfil_;
  PrefChangeRegistrar prefs_;
  raw_ptr<views::View> lista_ = nullptr;
  std::vector<Linha> linhas_;
  std::vector<containers::Container> containers_;
  bool editando_ = false;
  base::WeakPtrFactory<PainelDeEspacos> weak_factory_{this};
};

BEGIN_METADATA(PainelDeEspacos)
END_METADATA

// --- Aba Atalhos ------------------------------------------------------------

// Botão com o atalho da ação; clicar e apertar as teclas troca o atalho.
class BotaoDeAtalho : public views::LabelButton {
  METADATA_HEADER(BotaoDeAtalho, views::LabelButton)

 public:
  BotaoDeAtalho(Profile* perfil,
                std::string acao,
                base::RepeatingClosure mudou)
      : views::LabelButton(
            base::BindRepeating(&BotaoDeAtalho::Capturar,
                                base::Unretained(this))),
        perfil_(perfil),
        acao_(std::move(acao)),
        mudou_(std::move(mudou)) {
    SetFocusBehavior(FocusBehavior::ALWAYS);
    SetMinSize(gfx::Size(130, 28));
    SetHorizontalAlignment(gfx::ALIGN_CENTER);
    // Cara de tecla: um chip arredondado.
    SetBackground(views::CreateRoundedRectBackground(
        ui::kColorSysSurfaceVariant, 6));
    views::InkDrop::Get(this)->SetMode(views::InkDropHost::InkDropMode::ON);
    views::InkDrop::Get(this)->SetBaseColor(ui::kColorSysOnSurface);
    views::InstallRoundRectHighlightPathGenerator(this, gfx::Insets(), 6);
    Mostrar();
  }

  bool OnKeyPressed(const ui::KeyEvent& event) override {
    if (!capturando_) {
      return views::LabelButton::OnKeyPressed(event);
    }
    const ui::KeyboardCode tecla = event.key_code();
    if (tecla == ui::VKEY_CONTROL || tecla == ui::VKEY_MENU ||
        tecla == ui::VKEY_SHIFT || tecla == ui::VKEY_LWIN ||
        tecla == ui::VKEY_RWIN || tecla == ui::VKEY_COMMAND) {
      return true;  // espera a tecla de verdade
    }
    const int modificadores =
        event.flags() & (ui::EF_CONTROL_DOWN | ui::EF_ALT_DOWN |
                         ui::EF_SHIFT_DOWN | ui::EF_COMMAND_DOWN);
    capturando_ = false;
    if (tecla == ui::VKEY_ESCAPE && !modificadores) {
      Mostrar();
      return true;
    }
    if ((tecla == ui::VKEY_BACK || tecla == ui::VKEY_DELETE) &&
        !modificadores) {
      espacos::DefinirAtalho(perfil_, acao_, "");
      mudou_.Run();
      return true;
    }
    const bool tecla_de_funcao = tecla >= ui::VKEY_F1 && tecla <= ui::VKEY_F12;
    const std::string texto =
        atalhos::Escrever(ui::Accelerator(tecla, modificadores));
    if (texto.empty() || (!modificadores && !tecla_de_funcao)) {
      // Tecla sozinha atrapalharia a digitação nas páginas.
      Mostrar();
      return true;
    }
    // O mesmo atalho em outra ação: sai de lá.
    for (const espacos::Atalho& outro : espacos::Atalhos()) {
      if (outro.acao != acao_ &&
          espacos::AtalhoDe(perfil_, outro.acao) == texto) {
        espacos::DefinirAtalho(perfil_, outro.acao, "");
      }
    }
    espacos::DefinirAtalho(perfil_, acao_, texto);
    mudou_.Run();
    return true;
  }

  void OnBlur() override {
    views::LabelButton::OnBlur();
    if (capturando_) {
      capturando_ = false;
      Mostrar();
    }
  }

 private:
  void Capturar() {
    capturando_ = true;
    SetText(u"Aperte as teclas…");
    RequestFocus();
  }

  void Mostrar() {
    SetText(atalhos::Mostrar(espacos::AtalhoDe(perfil_, acao_)));
  }

  raw_ptr<Profile> perfil_;
  std::string acao_;
  base::RepeatingClosure mudou_;
  bool capturando_ = false;
};

BEGIN_METADATA(BotaoDeAtalho)
END_METADATA

class PainelDeAtalhos : public views::View {
  METADATA_HEADER(PainelDeAtalhos, views::View)

 public:
  explicit PainelDeAtalhos(Profile* perfil) : perfil_(perfil) {
    SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets::VH(16, 16),
        10));
    AddChildView(Legenda(
        u"Clique no atalho e aperte as teclas novas. Esc cancela; Backspace "
        u"tira o atalho. Valem em todas as janelas."));
    lista_ = AddChildView(std::make_unique<views::View>());
    lista_->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets(), 2));
    auto* restaurar = AddChildView(std::make_unique<views::MdTextButton>(
        base::BindRepeating(&PainelDeAtalhos::Restaurar,
                            base::Unretained(this)),
        u"Restaurar os padrões"));
    restaurar->SetProperty(views::kCrossAxisAlignmentKey,
                           views::LayoutAlignment::kStart);
    Reconstruir();
  }

 private:
  void Reconstruir() {
    lista_->RemoveAllChildViews();
    for (const espacos::Atalho& atalho : espacos::Atalhos()) {
      auto* linha = lista_->AddChildView(std::make_unique<views::View>());
      auto* layout =
          linha->SetLayoutManager(std::make_unique<views::BoxLayout>(
              views::BoxLayout::Orientation::kHorizontal,
              gfx::Insets::VH(0, 6), 8));
      layout->set_cross_axis_alignment(
          views::BoxLayout::CrossAxisAlignment::kCenter);
      auto* descricao =
          linha->AddChildView(std::make_unique<views::Label>(atalho.descricao));
      descricao->SetHorizontalAlignment(gfx::ALIGN_LEFT);
      layout->SetFlexForView(descricao, 1);
      linha->AddChildView(std::make_unique<BotaoDeAtalho>(
          perfil_, atalho.acao,
          base::BindRepeating(&PainelDeAtalhos::ReconstruirDepois,
                              weak_factory_.GetWeakPtr())));
    }
    PreferredSizeChanged();
  }

  // O botão que avisou ainda está na pilha: reconstrói no próximo giro.
  void ReconstruirDepois() {
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&PainelDeAtalhos::Reconstruir,
                                  weak_factory_.GetWeakPtr()));
  }

  void Restaurar() {
    espacos::RestaurarAtalhos(perfil_);
    Reconstruir();
  }

  raw_ptr<Profile> perfil_;
  raw_ptr<views::View> lista_ = nullptr;
  base::WeakPtrFactory<PainelDeAtalhos> weak_factory_{this};
};

BEGIN_METADATA(PainelDeAtalhos)
END_METADATA

// --- Janela -----------------------------------------------------------------

std::map<BrowserWindowInterface*, base::WeakPtr<views::Widget>>& Abertas() {
  static base::NoDestructor<
      std::map<BrowserWindowInterface*, base::WeakPtr<views::Widget>>>
      abertas;
  return *abertas;
}

// Editor de espaço (novo ou existente).
class EditorDeEspaco : public views::DialogDelegate {
 public:
  EditorDeEspaco(Profile* perfil,
                 std::string espaco,
                 base::OnceCallback<void(const std::string&)> pronto)
      : perfil_(perfil),
        id_(std::move(espaco)),
        pronto_(std::move(pronto)) {
    std::optional<espacos::Espaco> atual =
        id_.empty() ? std::nullopt : espacos::Get(perfil_, id_);
    cor_ = atual ? atual->cor : espacos::ProximaCor(perfil_);
    SetTitle(atual ? u"Editar espaço" : u"Novo espaço");
    SetModalType(ui::mojom::ModalType::kWindow);
    SetShowCloseButton(true);
    SetButtonLabel(ui::mojom::DialogButton::kOk,
                   atual ? u"Salvar" : u"Criar");
    SetButtonLabel(ui::mojom::DialogButton::kCancel, u"Cancelar");
    SetAcceptCallback(base::BindOnce(&EditorDeEspaco::Confirmar,
                                     base::Unretained(this)));

    auto conteudo = std::make_unique<views::View>();
    conteudo->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical,
        gfx::Insets::TLBR(4, 20, 8, 20), 12));

    auto* linha_nome =
        conteudo->AddChildView(std::make_unique<views::View>());
    auto* layout_nome =
        linha_nome->SetLayoutManager(std::make_unique<views::BoxLayout>(
            views::BoxLayout::Orientation::kHorizontal, gfx::Insets(), 8));
    layout_nome->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kCenter);
    icone_ = linha_nome->AddChildView(std::make_unique<views::Textfield>());
    icone_->SetPlaceholderText(u"＋");
    icone_->SetDefaultWidthInChars(2);
    icone_->SetAccessibleName(u"Emoji do espaço");
    nome_ = linha_nome->AddChildView(std::make_unique<views::Textfield>());
    nome_->SetPlaceholderText(u"Nome do espaço (ex.: Trabalho)");
    nome_->SetDefaultWidthInChars(24);
    nome_->SetAccessibleName(u"Nome do espaço");
    layout_nome->SetFlexForView(nome_, 1);

    auto* linha_cor = conteudo->AddChildView(std::make_unique<views::View>());
    linha_cor->SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kHorizontal, gfx::Insets(), 2));
    for (const char* cor : kPaleta) {
      cores_.push_back(linha_cor->AddChildView(std::make_unique<BolinhaDeCor>(
          cor, cor == cor_,
          base::BindRepeating(&EditorDeEspaco::EscolherCor,
                              base::Unretained(this), std::string(cor)))));
    }

    containers_ = containers::GetAll(perfil_);
    auto* linha_container =
        conteudo->AddChildView(std::make_unique<views::View>());
    auto* layout_container =
        linha_container->SetLayoutManager(std::make_unique<views::BoxLayout>(
            views::BoxLayout::Orientation::kHorizontal, gfx::Insets(), 8));
    layout_container->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kCenter);
    linha_container->AddChildView(
        std::make_unique<views::Label>(u"Container"));
    container_ = linha_container->AddChildView(
        std::make_unique<views::Combobox>(ModeloDeContainers(containers_)));
    container_->SetAccessibleName(u"Container do espaço");
    conteudo->AddChildView(Legenda(
        u"As abas novas do espaço abrem no container: os sites ficam "
        u"logados só nele."));

    if (atual) {
      icone_->SetText(base::UTF8ToUTF16(atual->icone));
      nome_->SetText(base::UTF8ToUTF16(atual->nome));
      container_->SetSelectedIndex(
          IndiceDoContainer(containers_, atual->container));
    } else {
      container_->SetSelectedIndex(0);
    }
    SetInitiallyFocusedView(nome_);
    SetContentsView(std::move(conteudo));
  }

 private:
  void EscolherCor(std::string cor) {
    cor_ = std::move(cor);
    for (size_t i = 0; i < cores_.size(); ++i) {
      cores_[i]->SetMarcada(cor_ == kPaleta[i]);
    }
  }

  void Confirmar() {
    std::string nome = base::UTF16ToUTF8(nome_->GetText());
    if (nome.empty()) {
      nome = "Espaço";
    }
    const std::optional<size_t> escolhido = container_->GetSelectedIndex();
    const std::string container = escolhido && *escolhido > 0
                                      ? containers_[*escolhido - 1].id
                                      : std::string();
    const std::string icone = base::UTF16ToUTF8(icone_->GetText());
    std::string id = id_;
    if (std::optional<espacos::Espaco> atual =
            id.empty() ? std::nullopt : espacos::Get(perfil_, id)) {
      atual->nome = nome;
      atual->icone = icone;
      atual->cor = cor_;
      atual->container = container;
      espacos::Atualizar(perfil_, *atual);
    } else {
      id = espacos::Criar(perfil_, nome, icone, cor_, container).id;
    }
    if (pronto_) {
      std::move(pronto_).Run(id);
    }
  }

  raw_ptr<Profile> perfil_;
  std::string id_;
  base::OnceCallback<void(const std::string&)> pronto_;
  std::string cor_;
  std::vector<containers::Container> containers_;
  raw_ptr<views::Textfield> icone_ = nullptr;
  raw_ptr<views::Textfield> nome_ = nullptr;
  raw_ptr<views::Combobox> container_ = nullptr;
  std::vector<raw_ptr<BolinhaDeCor>> cores_;
};

}  // namespace

void Mostrar(BrowserWindowInterface* browser, Aba aba) {
  auto& abertas = Abertas();
  if (auto it = abertas.find(browser);
      it != abertas.end() && it->second) {
    if (auto* abas = static_cast<views::TabbedPane*>(
            it->second->GetContentsView()->children().front())) {
      abas->SelectTabAt(static_cast<size_t>(aba));
    }
    it->second->Activate();
    return;
  }

  auto dialogo = std::make_unique<views::DialogDelegate>();
  dialogo->SetTitle(u"Configurações do Navegador");
  dialogo->SetModalType(ui::mojom::ModalType::kWindow);
  dialogo->SetShowCloseButton(true);
  dialogo->SetButtons(static_cast<int>(ui::mojom::DialogButton::kNone));
  dialogo->set_fixed_width(600);

  auto conteudo = std::make_unique<views::View>();
  conteudo->SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kVertical));
  auto* abas = conteudo->AddChildView(std::make_unique<views::TabbedPane>());
  auto rolavel = [](std::unique_ptr<views::View> painel) {
    auto rolagem = std::make_unique<views::ScrollView>();
    rolagem->SetContents(std::move(painel));
    rolagem->ClipHeightTo(320, 540);
    rolagem->SetBackgroundColor(std::nullopt);
    return rolagem;
  };
  abas->AddTab(u"Espaços", rolavel(std::make_unique<PainelDeEspacos>(browser)));
  abas->AddTab(u"Containers",
               rolavel(containers::CriarPainel(browser->GetProfile())));
  abas->AddTab(u"Atalhos", rolavel(std::make_unique<PainelDeAtalhos>(
                               browser->GetProfile())));
  abas->SelectTabAt(static_cast<size_t>(aba));
  dialogo->SetContentsView(std::move(conteudo));

  // Como os diálogos do Chromium: centralizado no alto da janela.
  views::Widget* widget = constrained_window::CreateBrowserModalDialogViews(
      std::move(dialogo), JanelaDe(browser));
  abertas[browser] = widget->GetWeakPtr();
  MostrarSobre(widget, browser);
}

void EditarEspaco(BrowserWindowInterface* browser,
                  const std::string& espaco,
                  base::OnceCallback<void(const std::string&)> pronto) {
  views::Widget* widget = constrained_window::CreateBrowserModalDialogViews(
      std::make_unique<EditorDeEspaco>(browser->GetProfile(), espaco,
                                       std::move(pronto)),
      JanelaDe(browser));
  MostrarSobre(widget, browser);
}

}  // namespace navegador::configuracoes
