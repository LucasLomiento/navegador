#include "navegador/browser/ui/cor_da_pagina.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <map>
#include <memory>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/no_destructor.h"
#include "base/task/bind_post_task.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "components/prefs/pref_service.h"
#include "components/viz/common/frame_sinks/copy_output_result.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "navegador/browser/containers/espacos_dados.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/views/widget/widget.h"

namespace navegador::cor_da_pagina {

namespace {

std::map<const BrowserView*, std::optional<SkColor>>& Cores() {
  static base::NoDestructor<std::map<const BrowserView*, std::optional<SkColor>>>
      cores;
  return *cores;
}

// Cor dominante da faixa: o "balde" de cor (5 bits por canal) com mais
// pixels, se ele tiver pelo menos 40% da faixa; senão, a média de tudo.
std::optional<SkColor> CorDominante(const SkBitmap& bitmap) {
  std::map<uint32_t, int> baldes;
  std::map<uint32_t, std::array<uint64_t, 3>> somas;
  uint64_t r = 0, g = 0, b = 0;
  int total = 0;
  for (int y = 0; y < bitmap.height(); ++y) {
    for (int x = 0; x < bitmap.width(); ++x) {
      const SkColor c = bitmap.getColor(x, y);
      if (SkColorGetA(c) < 0x80) {
        continue;
      }
      const uint32_t balde = ((SkColorGetR(c) >> 3) << 10) |
                             ((SkColorGetG(c) >> 3) << 5) | (SkColorGetB(c) >> 3);
      ++baldes[balde];
      auto& soma = somas[balde];
      soma[0] += SkColorGetR(c);
      soma[1] += SkColorGetG(c);
      soma[2] += SkColorGetB(c);
      r += SkColorGetR(c);
      g += SkColorGetG(c);
      b += SkColorGetB(c);
      ++total;
    }
  }
  if (total == 0) {
    return std::nullopt;
  }
  const auto maior = std::max_element(
      baldes.begin(), baldes.end(),
      [](const auto& a, const auto& b) { return a.second < b.second; });
  if (maior->second * 10 >= total * 4) {
    const auto& soma = somas[maior->first];
    const int n = maior->second;
    return SkColorSetRGB(soma[0] / n, soma[1] / n, soma[2] / n);
  }
  return SkColorSetRGB(r / total, g / total, b / total);
}

// Diferença visível entre duas cores (evita recalcular o tema por ruído).
bool Diferentes(std::optional<SkColor> a, std::optional<SkColor> b) {
  if (a.has_value() != b.has_value()) {
    return true;
  }
  if (!a) {
    return false;
  }
  const auto canal = [](SkColor c, int deslocamento) {
    return static_cast<int>((c >> deslocamento) & 0xFF);
  };
  int d = 0;
  for (int deslocamento : {16, 8, 0}) {
    d += std::abs(canal(*a, deslocamento) - canal(*b, deslocamento));
  }
  return d > 6;
}

// Invisível, fora do layout: só existe pra viver junto com a janela.
// Como o Dia (ColorSampleControllerV2 no código dele): amostra os pixels do
// topo da página aberta e usa a cor dominante na barra de navegação.
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
  void AddedToWidget() override {
    Amostrar();
    // A página muda sem avisar (rolar, cabeçalho fixo, vídeo): amostra de
    // novo de tempos em tempos. 64x2 pixels, barato.
    relogio_.Start(FROM_HERE, base::Milliseconds(1500),
                   base::BindRepeating(&Observador::Amostrar,
                                       base::Unretained(this)));
  }
  void RemovedFromWidget() override { relogio_.Stop(); }

  // TabStripModelObserver:
  void OnTabStripModelChanged(
      TabStripModel* tab_strip_model,
      const TabStripModelChange& change,
      const TabStripSelectionChange& selection) override {
    if (selection.active_tab_changed()) {
      Observe(model_->GetActiveWebContents());
      ++geracao_;
      Amostrar();
    }
  }

  // content::WebContentsObserver:
  void DidFirstVisuallyNonEmptyPaint() override { Amostrar(); }
  void DidStopLoading() override { Amostrar(); }
  void DidChangeThemeColor() override { Amostrar(); }
  void OnBackgroundColorChanged() override { Amostrar(); }

 private:
  // Copia uma faixa de 6 de altura no topo da página, reduzida pra 64x2.
  void Amostrar() {
    content::WebContents* contents = web_contents();
    content::RenderWidgetHostView* view =
        contents ? contents->GetRenderWidgetHostView() : nullptr;
    if (!view || !view->IsSurfaceAvailableForCopy()) {
      Definir(Reserva());
      return;
    }
    const gfx::Size tamanho = view->GetVisibleViewportSize();
    if (tamanho.IsEmpty()) {
      return;
    }
    view->CopyFromSurface(
        gfx::Rect(0, 0, tamanho.width(), std::min(6, tamanho.height())),
        gfx::Size(64, 2), base::Seconds(1),
        base::BindPostTaskToCurrentDefault(
            base::BindOnce(&Observador::AoAmostrar,
                           weak_factory_.GetWeakPtr(), geracao_)));
  }

  void AoAmostrar(int geracao, const content::CopyFromSurfaceResult& resultado) {
    if (geracao != geracao_) {
      return;  // Trocou de aba no meio.
    }
    std::optional<SkColor> cor;
    if (resultado.has_value() && !resultado->bitmap.drawsNothing()) {
      cor = CorDominante(resultado->bitmap);
    }
    Definir(cor ? cor : Reserva());
  }

  // Sem pixels (aba ainda não pintou): theme-color do site, ou o fundo.
  std::optional<SkColor> Reserva() {
    content::WebContents* contents = web_contents();
    if (!contents) {
      return std::nullopt;
    }
    std::optional<SkColor> cor = contents->GetThemeColor();
    if (!cor) {
      cor = contents->GetBackgroundColor();
    }
    if (cor && SkColorGetA(*cor) < 0x80) {
      cor.reset();  // Transparente não é cor de página.
    }
    return cor;
  }

  void Definir(std::optional<SkColor> cor) {
    auto& cores = Cores();
    auto it = cores.find(browser_view_);
    if (it != cores.end() && !Diferentes(it->second, cor)) {
      return;
    }
    cores[browser_view_] = cor;
    if (views::Widget* widget = GetWidget()) {
      widget->ThemeChanged();
    }
  }

  raw_ptr<BrowserView> browser_view_;
  raw_ptr<TabStripModel> model_;
  int geracao_ = 0;
  base::RepeatingTimer relogio_;
  base::WeakPtrFactory<Observador> weak_factory_{this};
};

BEGIN_METADATA(Observador)
END_METADATA

}  // namespace

std::optional<SkColor> CorDaBarra(const BrowserView* browser_view) {
  // Desligado nas Configurações do Navegador (Aparência): barra neutra.
  if (browser_view && browser_view->browser() &&
      !browser_view->browser()->GetProfile()->GetPrefs()->GetBoolean(
          navegador::espacos::kBarraNaCorDoSitePref)) {
    return std::nullopt;
  }
  auto& cores = Cores();
  auto it = cores.find(browser_view);
  return it == cores.end() ? std::nullopt : it->second;
}

void Acompanhar(views::View* dono, BrowserView* browser_view) {
  dono->AddChildView(std::make_unique<Observador>(browser_view));
}

}  // namespace navegador::cor_da_pagina
