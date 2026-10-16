#include "navegador/browser/ui/fontes.h"

#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>

#include <string>

#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/path_service.h"
#include "base/task/thread_pool.h"
#include "base/win/registry.h"
#endif

namespace navegador::fontes {

#if BUILDFLAG(IS_WIN)
namespace {

void Instalar() {
  base::FilePath modulo;
  base::FilePath locais;
  if (!base::PathService::Get(base::DIR_MODULE, &modulo) ||
      !base::PathService::Get(base::DIR_LOCAL_APP_DATA, &locais)) {
    return;
  }
  const base::FilePath destino =
      locais.Append(FILE_PATH_LITERAL("Microsoft\\Windows\\Fonts"));
  if (!base::CreateDirectory(destino)) {
    return;
  }
  base::win::RegKey registro(
      HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
      KEY_QUERY_VALUE | KEY_SET_VALUE);
  if (!registro.Valid()) {
    return;
  }
  base::FileEnumerator arquivos(modulo.Append(FILE_PATH_LITERAL("fontes")),
                                /*recursive=*/false,
                                base::FileEnumerator::FILES,
                                FILE_PATH_LITERAL("*.ttf"));
  for (base::FilePath fonte = arquivos.Next(); !fonte.empty();
       fonte = arquivos.Next()) {
    const base::FilePath alvo = destino.Append(fonte.BaseName());
    if (!base::PathExists(alvo) && !base::CopyFile(fonte, alvo)) {
      continue;
    }
    const std::wstring nome =
        fonte.BaseName().RemoveExtension().value() + L" (TrueType)";
    if (!registro.HasValue(nome.c_str())) {
      registro.WriteValue(nome.c_str(), alvo.value().c_str());
    }
  }
}

}  // namespace
#endif

void InstalarSeFaltar() {
#if BUILDFLAG(IS_WIN)
  base::ThreadPool::PostTask(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::BEST_EFFORT},
      base::BindOnce(&Instalar));
#endif
}

}  // namespace navegador::fontes
