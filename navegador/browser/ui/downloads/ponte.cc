#include "navegador/browser/ui/downloads/ponte.h"

#include <utility>

#include "base/no_destructor.h"

namespace navegador::downloads {

namespace {

AncoraDaJanela& Ancoras() {
  static base::NoDestructor<AncoraDaJanela> ancora;
  return *ancora;
}

AvisoDeProgresso& Avisos() {
  static base::NoDestructor<AvisoDeProgresso> aviso;
  return *aviso;
}

}  // namespace

void DefinirAncora(AncoraDaJanela ancora) {
  Ancoras() = std::move(ancora);
}

views::View* Ancora(BrowserWindowInterface* browser) {
  return Ancoras() ? Ancoras().Run(browser) : nullptr;
}

bool TemBotaoNaLateral() {
  return !Ancoras().is_null();
}

void DefinirAvisoDeProgresso(AvisoDeProgresso aviso) {
  Avisos() = std::move(aviso);
}

void AvisarProgresso(BrowserWindowInterface* browser,
                     const Progresso& progresso) {
  if (Avisos()) {
    Avisos().Run(browser, progresso);
  }
}

}  // namespace navegador::downloads
