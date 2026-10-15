#include "navegador/browser/ui/atualizador.h"

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/functional/bind.h"
#include "base/json/json_reader.h"
#include "base/memory/raw_ptr.h"
#include "base/no_destructor.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/process/process.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/time/time.h"
#include "base/values.h"
#include "base/version.h"
#include "cc/paint/paint_flags.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/lifetime/application_lifetime.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/navigator/browser_navigator.h"
#include "chrome/browser/ui/navigator/browser_navigator_params.h"
#include "components/prefs/pref_service.h"
#include "navegador/browser/containers/espacos_dados.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/resource_request.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "services/network/public/cpp/simple_url_loader.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/page_transition_types.h"
#include "ui/base/window_open_disposition.h"
#include "ui/color/color_id.h"
#include "ui/color/color_provider.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/font_list.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "url/gurl.h"

namespace navegador::atualizador {

namespace {

constexpr net::NetworkTrafficAnnotationTag kAnotacao =
    net::DefineNetworkTrafficAnnotation("navegador_atualizador", R"(
        semantics {
          sender: "Atualizador do Navegador"
          description:
            "Procura a última versão do navegador nas Releases do GitHub."
          trigger: "Ao abrir o navegador, ou em 'Procurar agora'."
          data: "Nenhum dado do usuário."
          destination: OTHER
        }
        policy {
          cookies_allowed: NO
          setting: "Configurações do Navegador → Geral → Atualizações."
          policy_exception_justification: "Navegador próprio."
        })");

enum class Situacao { kNada, kDisponivel, kInstalando, kInstalada, kErro };

struct Versao {
  base::Version chromium;
  int revisao = 0;
};

// "155.0.8059.79-9" ou "v155.0.8059.79-9".
std::optional<Versao> LerVersao(std::string texto) {
  texto = std::string(base::TrimWhitespaceASCII(texto, base::TRIM_ALL));
  if (texto.starts_with("v")) {
    texto.erase(0, 1);
  }
  const size_t traco = texto.rfind('-');
  if (traco == std::string::npos) {
    return std::nullopt;
  }
  Versao versao;
  versao.chromium = base::Version(texto.substr(0, traco));
  if (!versao.chromium.IsValid() ||
      !base::StringToInt(texto.substr(traco + 1), &versao.revisao)) {
    return std::nullopt;
  }
  return versao;
}

bool MaisNova(const Versao& a, const Versao& b) {
  const int ordem = a.chromium.CompareTo(b.chromium);
  return ordem > 0 || (ordem == 0 && a.revisao > b.revisao);
}

class Aviso;

// Estado do processo inteiro (uma procura, avisos em todas as janelas).
struct Estado {
  std::string instalada;
  bool instalada_lida = false;
  bool procurou = false;
  Situacao situacao = Situacao::kNada;
  std::string nova;
  std::string url_pacote;
  std::string url_pagina;
  std::u16string erro;
  std::unique_ptr<network::SimpleURLLoader> pedido;
  std::vector<base::OnceCallback<void(const std::u16string&)>> esperando;
  std::set<Aviso*> avisos;
};

Estado& Atual() {
  static base::NoDestructor<Estado> estado;
  return *estado;
}

void AvisarTodos();

std::string LerArquivoDeVersao() {
  base::FilePath pasta;
  std::string texto;
  if (!base::PathService::Get(base::DIR_EXE, &pasta) ||
      !base::ReadFileToString(pasta.Append("VERSAO"), &texto)) {
    return std::string();
  }
  return std::string(base::TrimWhitespaceASCII(texto, base::TRIM_ALL));
}

void Responder(const std::u16string& frase) {
  for (auto& resposta : Atual().esperando) {
    std::move(resposta).Run(frase);
  }
  Atual().esperando.clear();
}

void Chegou(std::optional<std::string> corpo) {
  Estado& estado = Atual();
  estado.pedido.reset();
  std::optional<base::Value> json =
      corpo ? base::JSONReader::Read(*corpo, base::JSON_PARSE_RFC)
            : std::nullopt;
  if (!json || !json->is_dict()) {
    Responder(u"Não deu pra consultar o GitHub agora.");
    return;
  }
  const base::DictValue& release = json->GetDict();
  const std::string* tag = release.FindString("tag_name");
  std::optional<Versao> nova = tag ? LerVersao(*tag) : std::nullopt;
  std::optional<Versao> instalada = LerVersao(estado.instalada);
  if (!nova || !instalada) {
    Responder(u"Não deu pra comparar as versões.");
    return;
  }
  if (!MaisNova(*nova, *instalada)) {
    Responder(u"Você está na versão mais nova (" +
              base::UTF8ToUTF16(estado.instalada) + u").");
    return;
  }
  std::string url;
  if (const base::ListValue* arquivos = release.FindList("assets")) {
    for (const base::Value& arquivo : *arquivos) {
      const base::DictValue* d = arquivo.GetIfDict();
      const std::string* nome = d ? d->FindString("name") : nullptr;
      const std::string* endereco =
          d ? d->FindString("browser_download_url") : nullptr;
      if (nome && endereco && nome->ends_with(".pkg.tar.zst")) {
        url = *endereco;
      }
    }
  }
  if (url.empty()) {
    Responder(u"A versão nova ainda não tem o pacote.");
    return;
  }
  estado.situacao = Situacao::kDisponivel;
  estado.nova = tag->starts_with("v") ? tag->substr(1) : *tag;
  estado.url_pacote = url;
  if (const std::string* pagina = release.FindString("html_url")) {
    estado.url_pagina = *pagina;
  }
  Responder(u"Versão nova: " + base::UTF8ToUTF16(estado.nova) +
            u". O aviso está no pé da lateral.");
  AvisarTodos();
}

void Procurar() {
  Estado& estado = Atual();
  if (estado.pedido || estado.instalada.empty()) {
    if (estado.instalada.empty()) {
      Responder(u"Esta cópia não veio do pacote: sem atualização automática.");
    }
    return;
  }
  auto pedido = std::make_unique<network::ResourceRequest>();
  pedido->url = GURL(std::string("https://api.github.com/repos/") +
                     kRepositorio + "/releases/latest");
  pedido->credentials_mode = network::mojom::CredentialsMode::kOmit;
  pedido->headers.SetHeader("Accept", "application/vnd.github+json");
  estado.pedido =
      network::SimpleURLLoader::Create(std::move(pedido), kAnotacao);
  estado.pedido->DownloadToString(
      g_browser_process->shared_url_loader_factory().get(),
      base::BindOnce(&Chegou), 1024 * 1024);
}

// Lê a versão instalada (arquivo) e, se pedido, procura.
void Comecar(bool procurar) {
  Estado& estado = Atual();
  if (estado.instalada_lida) {
    if (procurar) {
      Procurar();
    }
    return;
  }
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::BEST_EFFORT},
      base::BindOnce(&LerArquivoDeVersao),
      base::BindOnce(
          [](bool procurar, std::string versao) {
            Atual().instalada = versao;
            Atual().instalada_lida = true;
            if (procurar) {
              Procurar();
            }
          },
          procurar));
}

void Instalar() {
  Estado& estado = Atual();
  if (estado.situacao != Situacao::kDisponivel &&
      estado.situacao != Situacao::kErro) {
    return;
  }
  estado.situacao = Situacao::kInstalando;
  AvisarTodos();
  // pacman instala direto do endereço; o pkexec pede a senha numa janela
  // do sistema (o agente do polkit).
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(
          [](std::string url) {
            base::CommandLine comando(base::FilePath("/usr/bin/pkexec"));
            comando.AppendArg("/usr/bin/pacman");
            comando.AppendArg("-U");
            comando.AppendArg("--noconfirm");
            comando.AppendArg(url);
            base::Process processo =
                base::LaunchProcess(comando, base::LaunchOptions());
            int codigo = -1;
            if (!processo.IsValid() ||
                !processo.WaitForExitWithTimeout(base::Minutes(15), &codigo)) {
              return -1;
            }
            return codigo;
          },
          estado.url_pacote),
      base::BindOnce([](int codigo) {
        Estado& estado = Atual();
        if (codigo == 0) {
          estado.situacao = Situacao::kInstalada;
        } else {
          estado.situacao = Situacao::kErro;
          // 126/127: senha cancelada ou sem a janela de senha do sistema.
          estado.erro = codigo == 126 || codigo == 127
                            ? u"Cancelado (ou o sistema não pediu a senha)."
                            : u"O pacman não conseguiu instalar (código " +
                                  base::NumberToString16(codigo) + u").";
        }
        AvisarTodos();
      }));
}

// --- Aviso no pé da lateral -------------------------------------------------

class Aviso : public views::View {
  METADATA_HEADER(Aviso, views::View)

 public:
  Aviso() {
    auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
        views::BoxLayout::Orientation::kVertical, gfx::Insets::VH(10, 12), 6));
    layout->set_cross_axis_alignment(
        views::BoxLayout::CrossAxisAlignment::kStretch);
    SetProperty(views::kMarginsKey, gfx::Insets::TLBR(0, 8, 8, 8));
    titulo_ = AddChildView(std::make_unique<views::Label>());
    titulo_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    titulo_->SetFontList(gfx::FontList().DeriveWithWeight(
        gfx::Font::Weight::SEMIBOLD));
    detalhe_ = AddChildView(std::make_unique<views::Label>());
    detalhe_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    detalhe_->SetMultiLine(true);
    detalhe_->SetFontList(gfx::FontList().DeriveWithSizeDelta(-1));
    botao_ = AddChildView(std::make_unique<views::MdTextButton>(
        base::BindRepeating(&Aviso::Apertou, base::Unretained(this)),
        std::u16string()));
    botao_->SetStyle(ui::ButtonStyle::kProminent);
    botao_->SetProperty(views::kCrossAxisAlignmentKey,
                        views::LayoutAlignment::kStart);
    Atual().avisos.insert(this);
    Mostrar();
  }

  ~Aviso() override { Atual().avisos.erase(this); }

  void Mostrar() {
    const Estado& estado = Atual();
    SetVisible(estado.situacao != Situacao::kNada);
    switch (estado.situacao) {
      case Situacao::kNada:
        break;
      case Situacao::kDisponivel:
        titulo_->SetText(u"Atualização disponível");
        detalhe_->SetText(u"Versão " + base::UTF8ToUTF16(estado.nova));
        botao_->SetText(u"Atualizar");
        botao_->SetEnabled(true);
        break;
      case Situacao::kInstalando:
        titulo_->SetText(u"Atualizando…");
        detalhe_->SetText(u"Digite a senha na janela do sistema.");
        botao_->SetText(u"Atualizar");
        botao_->SetEnabled(false);
        break;
      case Situacao::kInstalada:
        titulo_->SetText(u"Atualizado");
        detalhe_->SetText(u"Reinicie pra usar a versão " +
                          base::UTF8ToUTF16(estado.nova) + u".");
        botao_->SetText(u"Reiniciar");
        botao_->SetEnabled(true);
        break;
      case Situacao::kErro:
        titulo_->SetText(u"Não atualizou");
        detalhe_->SetText(estado.erro);
        botao_->SetText(u"Tentar de novo");
        botao_->SetEnabled(true);
        break;
    }
    detalhe_->SizeToFit(150);
    PreferredSizeChanged();
  }

  void OnPaint(gfx::Canvas* canvas) override {
    cc::PaintFlags flags;
    flags.setAntiAlias(true);
    flags.setColor(SkColorSetA(GetColorProvider()->GetColor(ui::kColorAccent),
                               0x26));
    canvas->DrawRoundRect(gfx::RectF(GetLocalBounds()), 8, flags);
  }

 private:
  void Apertou() {
    if (Atual().situacao == Situacao::kInstalada) {
      chrome::AttemptRestart();
      return;
    }
    Instalar();
  }

  raw_ptr<views::Label> titulo_ = nullptr;
  raw_ptr<views::Label> detalhe_ = nullptr;
  raw_ptr<views::MdTextButton> botao_ = nullptr;
};

BEGIN_METADATA(Aviso)
END_METADATA

void AvisarTodos() {
  for (Aviso* aviso : Atual().avisos) {
    aviso->Mostrar();
  }
}

}  // namespace

std::unique_ptr<views::View> CriarAviso(BrowserWindowInterface* browser) {
  Estado& estado = Atual();
  if (!estado.procurou) {
    estado.procurou = true;
    const bool procurar = browser->GetProfile()->GetPrefs()->GetBoolean(
        espacos::kProcurarAtualizacoesPref);
    // Depois de a janela abrir (não atrasa a abertura).
    base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
        FROM_HERE, base::BindOnce(&Comecar, procurar), base::Seconds(5));
  }
  return std::make_unique<Aviso>();
}

void ProcurarAgora(Profile* perfil,
                   base::OnceCallback<void(const std::u16string&)> resposta) {
  Atual().esperando.push_back(std::move(resposta));
  Comecar(/*procurar=*/true);
}

void LerVersaoInstalada(
    base::OnceCallback<void(const std::string&)> resposta) {
  if (Atual().instalada_lida) {
    std::move(resposta).Run(Atual().instalada);
    return;
  }
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock(), base::TaskPriority::USER_VISIBLE},
      base::BindOnce(&LerArquivoDeVersao),
      base::BindOnce(
          [](base::OnceCallback<void(const std::string&)> resposta,
             std::string versao) {
            Atual().instalada = versao;
            Atual().instalada_lida = true;
            std::move(resposta).Run(versao);
          },
          std::move(resposta)));
}

}  // namespace navegador::atualizador
