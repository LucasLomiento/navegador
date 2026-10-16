// Sem "grupos de guias salvos": os espaços usam grupos de abas do Chromium
// por baixo (um por janela), e o serviço de grupos salvos guardava cada um,
// aberto ou fechado, pra sempre. O menu "Grupos de guias" enchia de "Pessoal",
// "Trabalho"... repetidos. Os espaços já guardam o que importa (as abas
// voltam com a sessão), então aqui o serviço não guarda nada: cada grupo novo
// sai da lista na hora, e os que ficaram de antes são apagados.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_GRUPOS_SALVOS_H_
#define NAVEGADOR_BROWSER_UI_GRUPOS_SALVOS_H_

class Profile;

namespace navegador::grupos_salvos {

// Uma vez por perfil (as janelas chamam ao abrir).
void Desligar(Profile* perfil);

}  // namespace navegador::grupos_salvos

#endif  // NAVEGADOR_BROWSER_UI_GRUPOS_SALVOS_H_
