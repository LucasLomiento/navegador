#include "navegador/browser/containers/containers.h"

#include <utility>

#include "base/rand_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "components/pref_registry/pref_registry_syncable.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "content/public/browser/security_principal.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "url/gurl.h"

namespace navegador::containers {

namespace {

constexpr char kId[] = "id";
constexpr char kName[] = "name";
constexpr char kColor[] = "color";
constexpr char kIcon[] = "icon";

base::DictValue ToDict(const Container& container) {
  base::DictValue dict;
  dict.Set(kId, container.id);
  dict.Set(kName, container.name);
  dict.Set(kColor, container.color);
  dict.Set(kIcon, container.icon);
  return dict;
}

std::optional<Container> FromValue(const base::Value& value) {
  const base::DictValue* dict = value.GetIfDict();
  if (!dict) {
    return std::nullopt;
  }
  const std::string* id = dict->FindString(kId);
  const std::string* name = dict->FindString(kName);
  if (!id || id->empty() || !name) {
    return std::nullopt;
  }
  const std::string* color = dict->FindString(kColor);
  const std::string* icon = dict->FindString(kIcon);
  return Container{*id, *name, color ? *color : "azul", icon ? *icon : ""};
}

// Os quatro do Firefox, com ids fixos: assim a lista padrão é igual em
// todo perfil novo e não precisa ser gravada até alguém mexer.
base::ListValue DefaultContainers() {
  base::ListValue list;
  list.Append(ToDict({"pessoal", "Pessoal", "azul", "pessoa"}));
  list.Append(ToDict({"trabalho", "Trabalho", "laranja", "maleta"}));
  list.Append(ToDict({"banco", "Banco", "verde", "cifrao"}));
  list.Append(ToDict({"compras", "Compras", "rosa", "carrinho"}));
  return list;
}

std::string NewId() {
  uint8_t bytes[6];
  base::RandBytes(bytes);
  return "c" + base::HexEncodeLower(bytes);
}

}  // namespace

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
  registry->RegisterListPref(kContainersPref, DefaultContainers());
}

std::vector<Container> GetAll(Profile* profile) {
  std::vector<Container> result;
  for (const base::Value& value :
       profile->GetPrefs()->GetList(kContainersPref)) {
    if (std::optional<Container> container = FromValue(value)) {
      result.push_back(std::move(*container));
    }
  }
  return result;
}

std::optional<Container> Get(Profile* profile, std::string_view id) {
  for (Container& container : GetAll(profile)) {
    if (container.id == id) {
      return std::move(container);
    }
  }
  return std::nullopt;
}

Container Add(Profile* profile,
              std::string_view name,
              std::string_view color,
              std::string_view icon) {
  Container container{NewId(), std::string(name), std::string(color),
                      std::string(icon)};
  ScopedListPrefUpdate update(profile->GetPrefs(), kContainersPref);
  update->Append(ToDict(container));
  return container;
}

void Update(Profile* profile, const Container& container) {
  ScopedListPrefUpdate update(profile->GetPrefs(), kContainersPref);
  for (base::Value& value : *update) {
    base::DictValue* dict = value.GetIfDict();
    const std::string* id = dict ? dict->FindString(kId) : nullptr;
    if (id && *id == container.id) {
      *dict = ToDict(container);
      return;
    }
  }
}

void Remove(Profile* profile, std::string_view id) {
  ScopedListPrefUpdate update(profile->GetPrefs(), kContainersPref);
  update->EraseIf([&](const base::Value& value) {
    const base::DictValue* dict = value.GetIfDict();
    const std::string* value_id = dict ? dict->FindString(kId) : nullptr;
    return value_id && *value_id == id;
  });
}

content::StoragePartitionConfig PartitionConfig(Profile* profile,
                                                std::string_view id) {
  return content::StoragePartitionConfig::Create(profile, kPartitionDomain, id,
                                                 /*in_memory=*/false);
}

std::optional<std::string> IdForSiteInstance(content::SiteInstance* instance) {
  if (!instance) {
    return std::nullopt;
  }
  const content::StoragePartitionConfig& config =
      instance->GetSecurityPrincipal().GetStoragePartitionConfig();
  if (config.partition_domain() != kPartitionDomain ||
      config.partition_name().empty()) {
    return std::nullopt;
  }
  return config.partition_name();
}

std::optional<std::string> IdForWebContents(content::WebContents* contents) {
  return contents ? IdForSiteInstance(contents->GetSiteInstance())
                  : std::nullopt;
}

scoped_refptr<content::SiteInstance> CreateSiteInstance(Profile* profile,
                                                        const GURL& url,
                                                        std::string_view id) {
  return content::SiteInstance::CreateForFixedStoragePartition(
      profile, url, PartitionConfig(profile, id));
}

scoped_refptr<content::SiteInstance> SiteInstanceForNewTab(
    Profile* profile,
    content::WebContents* source_contents,
    const GURL& url) {
  // Só herda dentro do mesmo perfil: uma aba levada pra janela de outro
  // perfil não carrega o container junto.
  if (!source_contents || source_contents->GetBrowserContext() != profile) {
    return nullptr;
  }
  std::optional<std::string> id = IdForWebContents(source_contents);
  if (!id || !Get(profile, *id)) {
    return nullptr;
  }
  return CreateSiteInstance(profile, url, *id);
}

scoped_refptr<content::SiteInstance> SiteInstanceForRestoredTab(
    Profile* profile,
    const std::map<std::string, std::string>& extra_data) {
  auto it = extra_data.find(kSessionExtraDataKey);
  if (it == extra_data.end() || !Get(profile, it->second)) {
    return nullptr;
  }
  return CreateSiteInstance(profile, GURL(), it->second);
}

void AddExtraData(content::WebContents* contents,
                  std::map<std::string, std::string>& extra_data) {
  if (std::optional<std::string> id = IdForWebContents(contents)) {
    extra_data[kSessionExtraDataKey] = *id;
  }
}

}  // namespace navegador::containers
