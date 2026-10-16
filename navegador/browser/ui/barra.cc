#include "navegador/browser/ui/barra.h"

#include <map>
#include <memory>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/no_destructor.h"
#include "base/scoped_observation.h"
#include "base/task/sequenced_task_runner.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface_iterator.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/browser/ui/views/toolbar/toolbar_view.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_contents.h"
#include "navegador/browser/containers/espacos_dados.h"
#include "ui/views/focus/focus_manager.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"

namespace navegador::barra {

namespace {

// Refaz o layout da janela: o BrowserView::IsToolbarVisible() consulta
// Escondida(), e o layout esconde ou mostra a barra a partir dele.
void Refazer(BrowserView* view) {
  view->InvalidateLayout();
  if (views::Widget* widget = view->GetWidget()) {
    widget->GetRootView()->SchedulePaint();
  }
}

// Se o foco está na barra que vai sumir, passa pra página (senão o views
// passa pro próximo da fila, que pode ser a lateral).
void TirarFocoDaBarra(BrowserView* view) {
  views::FocusManager* foco = view->GetFocusManager();
  if (!foco || !view->toolbar() ||
      !view->toolbar()->Contains(foco->GetFocusedView())) {
    return;
  }
  if (content::WebContents* pagina = view->GetActiveWebContents()) {
    pagina->Focus();
  }
}

// A barra à vista pelo Ctrl+L numa janela: acompanha o foco e esconde de novo
// quando ele vai pra fora da barra.
class Espia : public views::FocusChangeListener, public views::WidgetObserver {
 public:
  Espia(BrowserView* view, views::Widget* widget)
      : view_(view), widget_(widget) {
    observacao_.Observe(widget);
    widget->GetFocusManager()->AddFocusChangeListener(this);
  }
  Espia(const Espia&) = delete;
  Espia& operator=(const Espia&) = delete;
  ~Espia() override {
    if (views::FocusManager* foco = widget_->GetFocusManager()) {
      foco->RemoveFocusChangeListener(this);
    }
  }

  bool espiando() const { return espiando_; }
  void set_espiando(bool espiando) {
    espiando_ = espiando;
    fraco_.InvalidateWeakPtrs();
  }

  // views::FocusChangeListener:
  void OnDidChangeFocus(views::View* antes, views::View* agora) override {
    // Sem foco = a janela perdeu a vez (outra janela, um menu): a barra fica,
    // o foco volta pra ela quando a janela voltar.
    if (!espiando_ || !agora || !view_->toolbar() ||
        view_->toolbar()->Contains(agora)) {
      return;
    }
    // Fora do aviso de mudança de foco: o layout esconde a barra e mexe na
    // visibilidade de views (que mexe no foco).
    base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE,
        base::BindOnce(&Espia::Esconder, fraco_.GetWeakPtr()));
  }

  // views::WidgetObserver:
  void OnWidgetDestroying(views::Widget* widget) override;

 private:
  void Esconder() {
    espiando_ = false;
    Refazer(view_);
  }

  const raw_ptr<BrowserView> view_;
  const raw_ptr<views::Widget> widget_;
  bool espiando_ = false;
  base::ScopedObservation<views::Widget, views::WidgetObserver> observacao_{
      this};
  base::WeakPtrFactory<Espia> fraco_{this};
};

std::map<const BrowserView*, std::unique_ptr<Espia>>& Espias() {
  static base::NoDestructor<std::map<const BrowserView*, std::unique_ptr<Espia>>>
      espias;
  return *espias;
}

void Espia::OnWidgetDestroying(views::Widget* widget) {
  // Apaga este objeto.
  Espias().erase(view_);
}

bool Ligada(const BrowserView* view) {
  const BrowserWindowInterface* browser = view ? view->browser() : nullptr;
  return browser && browser->GetType() == BrowserWindowInterface::TYPE_NORMAL &&
         browser->GetProfile()->GetPrefs()->GetBoolean(
             espacos::kBarraEscondidaPref);
}

}  // namespace

bool Escondida(const BrowserView* view) {
  if (!Ligada(view)) {
    return false;
  }
  auto it = Espias().find(view);
  return it == Espias().end() || !it->second->espiando();
}

void Definir(Profile* perfil, bool escondida) {
  perfil->GetPrefs()->SetBoolean(espacos::kBarraEscondidaPref, escondida);
  for (BrowserWindowInterface* browser : GetAllBrowserWindowInterfaces()) {
    if (browser->GetProfile() != perfil) {
      continue;
    }
    BrowserView* view = BrowserView::GetBrowserViewForBrowser(browser);
    if (!view) {
      continue;
    }
    if (auto it = Espias().find(view); it != Espias().end()) {
      it->second->set_espiando(false);
    }
    if (escondida) {
      TirarFocoDaBarra(view);
    }
    Refazer(view);
  }
}

void Alternar(BrowserWindowInterface* browser) {
  Profile* perfil = browser->GetProfile();
  Definir(perfil,
          !perfil->GetPrefs()->GetBoolean(espacos::kBarraEscondidaPref));
}

void Espiar(BrowserView* view) {
  views::Widget* widget = view->GetWidget();
  if (!Escondida(view) || !widget || !widget->GetFocusManager()) {
    return;
  }
  std::unique_ptr<Espia>& espia = Espias()[view];
  if (!espia) {
    espia = std::make_unique<Espia>(view, widget);
  }
  espia->set_espiando(true);
  // Já: o endereço só aceita foco com a barra à vista (o layout do
  // BrowserView chama o UpdateFocusBehavior).
  view->InvalidateLayout();
  widget->LayoutRootViewIfNecessary();
}

}  // namespace navegador::barra
