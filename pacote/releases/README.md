# Navegador

Navegador baseado no Chromium, com as abas numa lateral que se esconde, **espaços** (cada um com as suas abas, sites fixados, pastas e favoritos), **containers** (cookies e logins separados dentro do mesmo perfil, como os do Firefox) e importação do Zen. Sem recursos de IA.

Pacotes para **Arch Linux (x86_64)**.

## Instalar (atualiza junto com o sistema)

Acrescente ao fim do `/etc/pacman.conf`:

```ini
[navegador]
SigLevel = Optional TrustAll
Server = https://github.com/LucasLomiento/navegador-releases/releases/latest/download
```

E instale:

```sh
sudo pacman -Sy navegador
```

Depois disso ele atualiza junto com o sistema (`sudo pacman -Syu`). O próprio navegador também procura versão nova quando abre e mostra um aviso no pé da lateral, com o botão "Atualizar" (Configurações → Geral → Atualizações).

> Os pacotes não são assinados (`SigLevel = Optional TrustAll`): o pacman confia no que vem deste repositório.

## Instalar só o arquivo

Baixe o `navegador-*-x86_64.pkg.tar.zst` da [última versão](https://github.com/LucasLomiento/navegador-releases/releases/latest) e:

```sh
sudo pacman -U navegador-*-x86_64.pkg.tar.zst
```

## Primeiros passos

- **Configurações do navegador:** `Ctrl+,` (ou clique no nome do espaço, no alto da lateral).
- **Mostrar/ocultar a lateral:** `Ctrl+S`. Escondida, ela aparece ao encostar o ponteiro na borda esquerda.
- **Trocar de espaço:** `Ctrl+Alt+←/→`, as bolinhas no pé da lateral, ou os botões laterais do mouse em cima dela.
- **Vindo do Zen:** Configurações → Geral → Importar do Zen (containers, espaços, sites fixados, cookies de cada container, senhas, favoritos e histórico).
- **Netflix e afins (Widevine):** o componente é baixado sozinho no primeiro uso.

## Licenças

Feito sobre o [Chromium](https://www.chromium.org/) (licença BSD) e os componentes de terceiros dele; a lista completa está em `chrome://credits`. Não é um produto do Google.
