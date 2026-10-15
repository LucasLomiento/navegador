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
    "GlicSelectionPrompt,"  // sugestão do Gemini ao selecionar texto
    "HistoryEmbeddings,"   // busca no histórico com IA
    "TabOrganization,"     // "organizar guias" com IA
    "AutofillAi,"          // preenchimento automático com IA
    "AutofillAiWithDataSchema,"
    "PermissionsAIv1,"     // pedidos de permissão avaliados por IA
    "PermissionsAIv3,"
    "PermissionsAIP92,"
    "ContextualTasks,"     // tarefas com IA no painel lateral
    "ContextualCueing,"    // sugestões de IA pela página
    "ContextualCueingV2,"
    "GoogleSearchAiModeWorkspace,"
    "AiModeOmniboxEntryPoint,"
    "Indigo,"
    "SkillsEnabled";

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
