// Janela "Containers": criar, renomear, trocar a cor e excluir.

#ifndef NAVEGADOR_BROWSER_UI_CONTAINERS_GERENCIADOR_H_
#define NAVEGADOR_BROWSER_UI_CONTAINERS_GERENCIADOR_H_

namespace content {
class WebContents;
}  // namespace content

namespace navegador::containers {

// Abre a janela por cima da janela do navegador que tem `aba`.
void MostrarGerenciador(content::WebContents* aba);

}  // namespace navegador::containers

#endif  // NAVEGADOR_BROWSER_UI_CONTAINERS_GERENCIADOR_H_
