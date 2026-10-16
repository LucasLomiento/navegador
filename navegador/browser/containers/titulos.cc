#include "navegador/browser/containers/titulos.h"

#include <memory>
#include <utility>

#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/supports_user_data.h"
#include "content/public/browser/web_contents.h"

namespace navegador::titulos {

namespace {

const char kChave[] = "navegador_titulo_da_aba";

struct TituloDaAba : public base::SupportsUserData::Data {
  explicit TituloDaAba(std::u16string titulo) : titulo(std::move(titulo)) {}
  std::u16string titulo;
};

}  // namespace

const std::u16string* Titulo(const content::WebContents* contents) {
  if (!contents) {
    return nullptr;
  }
  auto* dado = static_cast<const TituloDaAba*>(contents->GetUserData(kChave));
  return dado ? &dado->titulo : nullptr;
}

void Guardar(content::WebContents* contents, const std::u16string& titulo) {
  if (!contents) {
    return;
  }
  std::u16string limpo(base::TrimWhitespace(titulo, base::TRIM_ALL));
  if (limpo.empty()) {
    contents->RemoveUserData(kChave);
  } else {
    contents->SetUserData(kChave,
                          std::make_unique<TituloDaAba>(std::move(limpo)));
  }
}

std::optional<std::string> ParaSessao(const content::WebContents* contents) {
  if (const std::u16string* titulo = Titulo(contents)) {
    return base::UTF16ToUTF8(*titulo);
  }
  return std::nullopt;
}

void RestaurarDaSessao(content::WebContents* contents,
                       const std::map<std::string, std::string>& extra_data) {
  auto it = extra_data.find(kSessionKey);
  if (it != extra_data.end() && !it->second.empty()) {
    Guardar(contents, base::UTF8ToUTF16(it->second));
  }
}

void AddExtraData(content::WebContents* contents,
                  std::map<std::string, std::string>& extra_data) {
  if (std::optional<std::string> titulo = ParaSessao(contents)) {
    extra_data[kSessionKey] = *titulo;
  }
}

}  // namespace navegador::titulos
