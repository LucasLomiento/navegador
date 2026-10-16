#include "navegador/browser/ui/grupos_salvos.h"

#include <memory>

#include "base/functional/bind.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/supports_user_data.h"
#include "base/task/single_thread_task_runner.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/tab_group_sync/tab_group_sync_service_factory.h"
#include "components/saved_tab_groups/public/saved_tab_group.h"
#include "components/saved_tab_groups/public/tab_group_sync_service.h"

namespace navegador::grupos_salvos {

namespace {

const char kChave[] = "navegador_sem_grupos_salvos";

class SemGruposSalvos : public base::SupportsUserData::Data,
                        public tab_groups::TabGroupSyncService::Observer {
 public:
  explicit SemGruposSalvos(tab_groups::TabGroupSyncService* servico)
      : servico_(servico) {
    servico_->AddObserver(this);
  }

  ~SemGruposSalvos() override {
    if (servico_) {
      servico_->RemoveObserver(this);
    }
  }

  // tab_groups::TabGroupSyncService::Observer:
  void OnInitialized() override { Agendar(); }

  void OnWillBeDestroyed() override {
    servico_->RemoveObserver(this);
    servico_ = nullptr;
  }

  void OnTabGroupAdded(const tab_groups::SavedTabGroup& grupo,
                       tab_groups::TriggerSource fonte) override {
    Agendar();
  }

 private:
  // Fora da notificação do próprio serviço (mexer nele lá dentro reentra).
  void Agendar() {
    if (agendado_) {
      return;
    }
    agendado_ = true;
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostTask(
        FROM_HERE, base::BindOnce(&SemGruposSalvos::Limpar,
                                  weak_factory_.GetWeakPtr()));
  }

  void Limpar() {
    agendado_ = false;
    if (!servico_) {
      return;
    }
    for (const tab_groups::SavedTabGroup& grupo : servico_->GetAllGroups()) {
      if (grupo.is_shared_tab_group()) {
        continue;  // Compartilhados não são dos espaços.
      }
      if (grupo.local_group_id()) {
        // Aberto: continua na janela, só sai da lista de salvos.
        servico_->UnsaveGroup(*grupo.local_group_id());
      } else {
        servico_->RemoveGroup(grupo.saved_guid());
      }
    }
  }

  raw_ptr<tab_groups::TabGroupSyncService> servico_;
  bool agendado_ = false;
  base::WeakPtrFactory<SemGruposSalvos> weak_factory_{this};
};

}  // namespace

void Desligar(Profile* perfil) {
  if (!perfil || perfil->GetUserData(kChave)) {
    return;
  }
  tab_groups::TabGroupSyncService* servico =
      tab_groups::TabGroupSyncServiceFactory::GetForProfile(perfil);
  if (!servico) {
    return;
  }
  auto observador = std::make_unique<SemGruposSalvos>(servico);
  // Já pronto (janela aberta depois da partida): limpa agora também.
  observador->OnInitialized();
  perfil->SetUserData(kChave, std::move(observador));
}

}  // namespace navegador::grupos_salvos
