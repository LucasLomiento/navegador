// Nova aba (chrome://new-tab-page-third-party, a página calma no estilo do
// Dia): o que aparece nela. Hora e data são prefs nossas; os atalhos usam as
// prefs do próprio Chromium (ntp_prefs), as mesmas da nova aba do Chrome:
// mais visitados, meus atalhos (lista editável, com o "+") ou nenhum.
//
// A página muda as opções pelo botão "Personalizar" (chrome.send) e recebe na
// hora as mudanças feitas nas Configurações do Navegador (Aparência).
//
// O .cc é compilado dentro de
// //chrome/browser/ui/webui/new_tab_page_third_party:impl; as funções de
// atalho aqui são inline pra servir também às Configurações (ui:ui).

#ifndef NAVEGADOR_BROWSER_UI_INICIO_H_
#define NAVEGADOR_BROWSER_UI_INICIO_H_

#include <memory>

#include "chrome/browser/new_tab_page/prefs/ntp_pref_names.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_ui_message_handler.h"

class Profile;

namespace content {
class WebUIDataSource;
}  // namespace content

namespace navegador::inicio {

enum class Atalhos { kMaisVisitados = 0, kMeus = 1, kNenhum = 2 };

inline Atalhos LerAtalhos(const PrefService* prefs) {
  if (!prefs->GetBoolean(ntp_prefs::kNtpShortcutsVisible)) {
    return Atalhos::kNenhum;
  }
  return prefs->GetBoolean(ntp_prefs::kNtpCustomLinksVisible)
             ? Atalhos::kMeus
             : Atalhos::kMaisVisitados;
}

inline void DefinirAtalhos(PrefService* prefs, Atalhos atalhos) {
  prefs->SetBoolean(ntp_prefs::kNtpShortcutsVisible,
                    atalhos != Atalhos::kNenhum);
  if (atalhos != Atalhos::kNenhum) {
    prefs->SetBoolean(ntp_prefs::kNtpCustomLinksVisible,
                      atalhos == Atalhos::kMeus);
  }
}

// Strings da edição de atalhos e as opções na hora de abrir a página.
void PrepararFonte(content::WebUIDataSource* fonte, Profile* perfil);

// Mensagens da página: "navegadorInicioPronto" e
// "navegadorInicioDefinir" [chave, valor]; avisa mudanças com o evento
// "navegador-inicio" {hora, data, atalhos}.
std::unique_ptr<content::WebUIMessageHandler> CriarMensagens(Profile* perfil);

}  // namespace navegador::inicio

#endif  // NAVEGADOR_BROWSER_UI_INICIO_H_
