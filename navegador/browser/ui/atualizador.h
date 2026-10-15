// Atualizador: ao abrir o navegador (o primeiro processo depois de fechar
// todas as janelas), procura a última versão nas Releases do GitHub
// (LucasLomiento/navegador-releases) e, se for mais nova que a instalada,
// mostra um aviso no pé da lateral. "Atualizar" instala o pacote novo com o
// pacman (pela janela de senha do sistema, pkexec) e oferece reiniciar.
//
// A versão instalada vem do arquivo VERSAO ao lado do executável, que o
// pacote grava ("155.0.8059.79-9"); sem ele (compilação de desenvolvimento)
// não procura nada.
//
// Compilado dentro de //chrome/browser/ui:ui.

#ifndef NAVEGADOR_BROWSER_UI_ATUALIZADOR_H_
#define NAVEGADOR_BROWSER_UI_ATUALIZADOR_H_

#include <memory>
#include <string>

#include "base/functional/callback.h"

class BrowserWindowInterface;
class Profile;

namespace views {
class View;
}  // namespace views

namespace navegador::atualizador {

// Releases públicas (só os pacotes; o código fica no repositório privado).
inline constexpr char kRepositorio[] = "LucasLomiento/navegador-releases";

// Aviso no pé da lateral (escondido enquanto não há versão nova). A primeira
// janela do processo também dispara a procura, se estiver ligada.
std::unique_ptr<views::View> CriarAviso(BrowserWindowInterface* browser);

// "Procurar agora" das Configurações: `resposta` recebe uma frase.
void ProcurarAgora(Profile* perfil,
                   base::OnceCallback<void(const std::u16string&)> resposta);

// Versão instalada, pra mostrar ("" fora do pacote).
void LerVersaoInstalada(base::OnceCallback<void(const std::string&)> resposta);

}  // namespace navegador::atualizador

#endif  // NAVEGADOR_BROWSER_UI_ATUALIZADOR_H_
