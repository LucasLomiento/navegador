// Espaços: conjuntos de abas com nome, ícone, cor e container próprios, como
// os workspaces do Zen e os perfis-na-mesma-janela do Dia. São do perfil (a
// lista vale pra todas as janelas); cada janela está num espaço por vez e
// mostra só as abas dele.
//
// Na janela, cada espaço é um grupo de abas focado (o modo "focar grupo" do
// Chromium já esconde as abas dos outros grupos). Este arquivo guarda só os
// dados: a lista de espaços, qual grupo de qual janela é de qual espaço, o
// último espaço usado e os atalhos configuráveis. A interface fica em
// navegador/browser/ui/espacos.*.

#ifndef NAVEGADOR_BROWSER_CONTAINERS_ESPACOS_DADOS_H_
#define NAVEGADOR_BROWSER_CONTAINERS_ESPACOS_DADOS_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

class Profile;

namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

namespace navegador::espacos {

// [{ "id", "nome", "icone", "cor", "container" }], na ordem da faixa.
inline constexpr char kListaPref[] = "navegador.espacos.lista";
// { "<token do grupo de abas>": "<id do espaço>" }. O token do grupo se
// mantém quando a sessão é restaurada.
inline constexpr char kGruposPref[] = "navegador.espacos.grupos";
// Último espaço usado: janela nova (ou aba solta) vai pra ele.
inline constexpr char kAtivoPref[] = "navegador.espacos.ativo";
// { "<ação>": "Ctrl+Alt+Right" }: só os atalhos trocados pela pessoa
// ("" = sem atalho); o resto usa o padrão de Atalhos().
inline constexpr char kAtalhosPref[] = "navegador.atalhos";
// Quadro tracejado no alto da lateral quando o espaço não tem favoritos.
inline constexpr char kQuadroDeFavoritosPref[] =
    "navegador.lateral.quadro_de_favoritos";
// Botões da janela (fechar, minimizar, maximizar) no canto da barra. Num
// gerenciador de janelas por atalho (Hyprland) sobram: desligado por padrão.
inline constexpr char kBotoesDaJanelaPref[] = "navegador.janela.botoes";

struct Espaco {
  std::string id;
  std::string nome;
  // Emoji (ou vazio: a faixa mostra a primeira letra do nome).
  std::string icone;
  // Nome da cor da paleta dos containers (azul, roxo, verde...).
  std::string cor;
  // Id do container das abas novas do espaço; vazio = perfil normal.
  std::string container;
};

void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

// Nunca vazia: sem nada salvo, vem um espaço "Pessoal" sem container.
std::vector<Espaco> Todos(Profile* profile);
std::optional<Espaco> Get(Profile* profile, std::string_view id);
Espaco Criar(Profile* profile,
             std::string_view nome,
             std::string_view icone,
             std::string_view cor,
             std::string_view container);
void Atualizar(Profile* profile, const Espaco& espaco);
// Não apaga o último espaço (a lista nunca fica vazia).
void Remover(Profile* profile, std::string_view id);
// Muda de posição na faixa (índice na lista nova).
void Mover(Profile* profile, std::string_view id, size_t posicao);

// Cor da próxima criação, em rodízio.
std::string ProximaCor(Profile* profile);

// Ligação grupo de abas (TabGroupId::ToString()) -> espaço.
std::optional<std::string> EspacoDoGrupo(Profile* profile,
                                         std::string_view grupo);
void LigarGrupo(Profile* profile,
                std::string_view grupo,
                std::string_view espaco);
void DesligarGrupo(Profile* profile, std::string_view grupo);

std::string Ativo(Profile* profile);
void DefinirAtivo(Profile* profile, std::string_view espaco);

// --- Atalhos ---------------------------------------------------------------

struct Atalho {
  // Chave na pref (ex.: "proximo_espaco").
  std::string acao;
  // Texto pra tela de configurações.
  std::u16string descricao;
  // Padrão, no formato "Ctrl+Alt+Right" ("" = sem atalho).
  std::string padrao;
};

// Ações com atalho configurável, na ordem da tela de configurações.
const std::vector<Atalho>& Atalhos();
// Atalho em uso pra ação (o da pessoa ou o padrão).
std::string AtalhoDe(Profile* profile, std::string_view acao);
// "" tira o atalho; igual ao padrão volta a seguir o padrão.
void DefinirAtalho(Profile* profile,
                   std::string_view acao,
                   std::string_view atalho);
void RestaurarAtalhos(Profile* profile);

// --- Geral -----------------------------------------------------------------

bool MostrarQuadroDeFavoritos(Profile* profile);
bool MostrarBotoesDaJanela(Profile* profile);

}  // namespace navegador::espacos

#endif  // NAVEGADOR_BROWSER_CONTAINERS_ESPACOS_DADOS_H_
