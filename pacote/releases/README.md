# Norte

Navegador baseado no Chromium, com as abas numa lateral que se esconde, **espaços** (cada um com as suas abas, sites fixados, pastas e favoritos), **containers** (cookies e logins separados dentro do mesmo perfil, como os do Firefox), **bloqueador de anúncios e rastreadores** e importação do Zen. Escuro, minimalista e sem recursos de IA. Código: [LucasLomiento/norte](https://github.com/LucasLomiento/norte).

Site: [lucaslomiento.github.io/norte](https://lucaslomiento.github.io/norte/).

Versões para **Arch Linux e derivados** (CachyOS, EndeavourOS, Manjaro), em x86_64.

> Até a versão 20 o Norte se chamava **Navegador**. Quem já tinha instalado não precisa fazer nada: a atualização troca o nome sozinha e leva o perfil junto (logins, espaços, containers e extensões, de `~/.config/navegador` pra `~/.config/norte`). A seção `[navegador]` antiga do `pacman.conf` continua funcionando.

## Instalar (atualiza junto com o sistema)

Acrescente ao fim do `/etc/pacman.conf`:

```ini
[norte]
SigLevel = Optional TrustAll
Server = https://github.com/LucasLomiento/norte-releases/releases/latest/download
```

E instale:

```sh
sudo pacman -Sy norte
```

Depois disso ele atualiza junto com o sistema (`sudo pacman -Syu`). O próprio Norte também procura versão nova quando abre e mostra um aviso no pé da lateral, com o botão "Atualizar" (Configurações → Geral → Atualizações).

> Os pacotes não são assinados (`SigLevel = Optional TrustAll`): o pacman confia no que vem deste repositório.

## Instalar só o arquivo

Baixe o `.pkg.tar.zst` da [última versão](https://github.com/LucasLomiento/norte-releases/releases/latest) e:

```sh
sudo pacman -U ./*.pkg.tar.zst
```

## Primeiros passos

- **Configurações do Norte:** `Ctrl+,` (ou clique no nome do espaço, no alto da lateral).
- **Mostrar/ocultar a lateral:** `Ctrl+S`. Escondida, ela aparece ao encostar o ponteiro na borda esquerda.
- **Trocar de espaço:** `Ctrl+Alt+←/→`, as bolinhas no pé da lateral, ou os botões laterais do mouse em cima dela.
- **Vindo do Zen:** Configurações → Geral → Importar do Zen (containers, espaços, sites fixados, cookies de cada container, senhas, favoritos e histórico).
- **Netflix e afins (Widevine):** o componente é baixado sozinho no primeiro uso.
- **1Password:** a extensão já conversa com o app do 1Password; o pacote instala a ligação e põe o Norte na lista de navegadores em que o app confia.

## Licenças

Feito sobre o [Chromium](https://www.chromium.org/) (licença BSD) e os componentes de terceiros dele; a lista completa está em `chrome://credits`. Não é um produto do Google.
