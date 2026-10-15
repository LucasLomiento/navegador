#include "navegador/browser/ui/inicio.h"

#include <string>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/grit/generated_resources.h"
#include "components/prefs/pref_change_registrar.h"
#include "content/public/browser/web_ui.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui_message_handler.h"
#include "navegador/browser/containers/espacos_dados.h"
#include "ui/webui/webui_util.h"

namespace navegador::inicio {

namespace {

base::DictValue Estado(Profile* perfil) {
  const PrefService* prefs = perfil->GetPrefs();
  base::DictValue estado;
  estado.Set("hora", prefs->GetBoolean(espacos::kInicioHoraPref));
  estado.Set("data", prefs->GetBoolean(espacos::kInicioDataPref));
  estado.Set("atalhos", static_cast<int>(LerAtalhos(prefs)));
  return estado;
}

class Mensagens : public content::WebUIMessageHandler {
 public:
  explicit Mensagens(Profile* perfil) : perfil_(perfil) {}
  ~Mensagens() override = default;

  // content::WebUIMessageHandler:
  void RegisterMessages() override {
    web_ui()->RegisterMessageCallback(
        "navegadorInicioPronto",
        base::BindRepeating(&Mensagens::Pronto, base::Unretained(this)));
    web_ui()->RegisterMessageCallback(
        "navegadorInicioDefinir",
        base::BindRepeating(&Mensagens::Definir, base::Unretained(this)));
  }

  void OnJavascriptAllowed() override {
    registrar_.Init(perfil_->GetPrefs());
    for (const char* pref :
         {espacos::kInicioHoraPref, espacos::kInicioDataPref,
          ntp_prefs::kNtpShortcutsVisible, ntp_prefs::kNtpCustomLinksVisible}) {
      registrar_.Add(pref, base::BindRepeating(&Mensagens::Avisar,
                                               base::Unretained(this)));
    }
  }

  void OnJavascriptDisallowed() override { registrar_.Reset(); }

 private:
  void Pronto(const base::ListValue& args) {
    AllowJavascript();
    Avisar();
  }

  void Definir(const base::ListValue& args) {
    AllowJavascript();
    if (args.size() != 2 || !args[0].is_string()) {
      return;
    }
    PrefService* prefs = perfil_->GetPrefs();
    const std::string& chave = args[0].GetString();
    if (chave == "hora" && args[1].is_bool()) {
      prefs->SetBoolean(espacos::kInicioHoraPref, args[1].GetBool());
    } else if (chave == "data" && args[1].is_bool()) {
      prefs->SetBoolean(espacos::kInicioDataPref, args[1].GetBool());
    } else if (chave == "atalhos" && args[1].is_int() && args[1].GetInt() >= 0 &&
               args[1].GetInt() <= static_cast<int>(Atalhos::kNenhum)) {
      DefinirAtalhos(prefs, static_cast<Atalhos>(args[1].GetInt()));
    }
  }

  void Avisar() { FireWebUIListener("navegador-inicio", Estado(perfil_)); }

  raw_ptr<Profile> perfil_;
  PrefChangeRegistrar registrar_;
};

}  // namespace

void PrepararFonte(content::WebUIDataSource* fonte, Profile* perfil) {
  // A nova aba do Chromium pra outros buscadores deixava a edição de atalhos
  // sem texto (só mostrava os mais visitados); aqui ela edita "meus atalhos".
  static constexpr webui::LocalizedString kTextos[] = {
      {"addLinkTitle", IDS_NTP_CUSTOM_LINKS_ADD_SHORTCUT_TITLE},
      {"editLinkTitle", IDS_NTP_CUSTOM_LINKS_EDIT_SHORTCUT},
      {"invalidUrl", IDS_NTP_CUSTOM_LINKS_INVALID_URL},
      {"linkAddedMsg", IDS_NTP_CONFIRM_MSG_SHORTCUT_ADDED},
      {"linkCancel", IDS_NTP_CUSTOM_LINKS_CANCEL},
      {"linkCantCreate", IDS_NTP_CUSTOM_LINKS_CANT_CREATE},
      {"linkCantEdit", IDS_NTP_CUSTOM_LINKS_CANT_EDIT},
      {"linkDone", IDS_NTP_CUSTOM_LINKS_DONE},
      {"linkEditedMsg", IDS_NTP_CONFIRM_MSG_SHORTCUT_EDITED},
      {"shortcutMoreActions", IDS_NTP_CUSTOM_LINKS_MORE_ACTIONS},
      {"nameField", IDS_NTP_CUSTOM_LINKS_NAME},
      {"restoreDefaultLinks", IDS_NTP_CONFIRM_MSG_RESTORE_DEFAULTS},
      {"shortcutAlreadyExists", IDS_NTP_CUSTOM_LINKS_ALREADY_EXISTS},
      {"urlField", IDS_NTP_CUSTOM_LINKS_URL},
  };
  fonte->AddLocalizedStrings(kTextos);

  const PrefService* prefs = perfil->GetPrefs();
  fonte->AddBoolean("navegadorHora",
                    prefs->GetBoolean(espacos::kInicioHoraPref));
  fonte->AddBoolean("navegadorData",
                    prefs->GetBoolean(espacos::kInicioDataPref));
  fonte->AddInteger("navegadorAtalhos", static_cast<int>(LerAtalhos(prefs)));
}

std::unique_ptr<content::WebUIMessageHandler> CriarMensagens(Profile* perfil) {
  return std::make_unique<Mensagens>(perfil);
}

}  // namespace navegador::inicio
