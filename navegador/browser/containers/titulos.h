// Nome que a pessoa deu a uma aba (clique duplo no nome, na lateral). Fica
// na aba (dado do WebContents) e na sessão, e vale no lugar do título da
// página: o TabUIHelper::GetTitle devolve ele.
//
// Núcleo sem UI, como containers.h: usado pelo tab_ui_helper, pela sessão e
// pelo tabrestore.

#ifndef NAVEGADOR_BROWSER_CONTAINERS_TITULOS_H_
#define NAVEGADOR_BROWSER_CONTAINERS_TITULOS_H_

#include <map>
#include <optional>
#include <string>

namespace content {
class WebContents;
}  // namespace content

namespace navegador::titulos {

// Chave no extra_data da aba na sessão.
inline constexpr char kSessionKey[] = "navegador_titulo";

// O nome dado à aba, ou nulo (vale o título da página).
const std::u16string* Titulo(const content::WebContents* contents);

// Guarda o nome (sem espaços nas pontas); vazio tira o nome.
void Guardar(content::WebContents* contents, const std::u16string& titulo);

// Pra gravar na sessão: o nome em UTF-8, se houver.
std::optional<std::string> ParaSessao(const content::WebContents* contents);

// Aba restaurada (sessão ou "reabrir aba fechada").
void RestaurarDaSessao(content::WebContents* contents,
                       const std::map<std::string, std::string>& extra_data);

// Coleta pro extra_data da aba fechada ("reabrir aba fechada").
void AddExtraData(content::WebContents* contents,
                  std::map<std::string, std::string>& extra_data);

}  // namespace navegador::titulos

#endif  // NAVEGADOR_BROWSER_CONTAINERS_TITULOS_H_
