#include "navegador/browser/sem_ia/sem_ia.h"

#include <string>

#include "base/base_switches.h"
#include "base/command_line.h"

namespace navegador {

namespace {

// Nomes das flags (o texto do BASE_FEATURE, sem o "k").
constexpr char kSemIA[] =
    "AimEnabled,"          // "Modo IA" na barra de endereço
    "Aim3pEntrypoint,"     // entrada do Modo IA com outra busca
    "NtpComposebox,"       // caixa de IA na nova aba
    "Compose,"             // "Me ajude a escrever"
    "LensOverlay,"         // Google Lens com IA por cima da página
    "Glic,"                // Gemini no Chrome
    "GlicRollout,"
    "HistoryEmbeddings";   // busca no histórico com IA

}  // namespace

void DesligarIA(base::CommandLine* command_line) {
  std::string desligadas =
      command_line->GetSwitchValueASCII(switches::kDisableFeatures);
  if (!desligadas.empty()) {
    desligadas += ",";
  }
  desligadas += kSemIA;
  command_line->AppendSwitchASCII(switches::kDisableFeatures, desligadas);
}

}  // namespace navegador
