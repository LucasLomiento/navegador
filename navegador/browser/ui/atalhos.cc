#include "navegador/browser/ui/atalhos.h"

#include <vector>

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/events/event_constants.h"
#include "ui/events/keycodes/keyboard_codes.h"

namespace navegador::atalhos {

namespace {

struct Tecla {
  const char* nome;
  ui::KeyboardCode codigo;
};

// Teclas fora de A-Z, 0-9 e F1-F12.
constexpr Tecla kTeclas[] = {
    {"Left", ui::VKEY_LEFT},       {"Right", ui::VKEY_RIGHT},
    {"Up", ui::VKEY_UP},           {"Down", ui::VKEY_DOWN},
    {"Comma", ui::VKEY_OEM_COMMA}, {"Period", ui::VKEY_OEM_PERIOD},
    {"Slash", ui::VKEY_OEM_2},     {"Minus", ui::VKEY_OEM_MINUS},
    {"Equal", ui::VKEY_OEM_PLUS},  {"Space", ui::VKEY_SPACE},
    {"Tab", ui::VKEY_TAB},         {"Home", ui::VKEY_HOME},
    {"End", ui::VKEY_END},         {"PageUp", ui::VKEY_PRIOR},
    {"PageDown", ui::VKEY_NEXT},   {"Backspace", ui::VKEY_BACK},
    {"Delete", ui::VKEY_DELETE},   {"Insert", ui::VKEY_INSERT},
    {"Escape", ui::VKEY_ESCAPE},   {"Enter", ui::VKEY_RETURN},
    {"Semicolon", ui::VKEY_OEM_1}, {"Quote", ui::VKEY_OEM_7},
    {"BracketLeft", ui::VKEY_OEM_4}, {"BracketRight", ui::VKEY_OEM_6},
};

std::optional<ui::KeyboardCode> CodigoDe(std::string_view nome) {
  if (nome.size() == 1) {
    const char c = base::ToUpperASCII(nome[0]);
    if (c >= 'A' && c <= 'Z') {
      return static_cast<ui::KeyboardCode>(ui::VKEY_A + (c - 'A'));
    }
    if (c >= '0' && c <= '9') {
      return static_cast<ui::KeyboardCode>(ui::VKEY_0 + (c - '0'));
    }
  }
  if (nome.size() >= 2 && (nome[0] == 'F' || nome[0] == 'f')) {
    int n = 0;
    for (char c : nome.substr(1)) {
      if (c < '0' || c > '9') {
        n = 0;
        break;
      }
      n = n * 10 + (c - '0');
    }
    if (n >= 1 && n <= 12) {
      return static_cast<ui::KeyboardCode>(ui::VKEY_F1 + (n - 1));
    }
  }
  for (const Tecla& tecla : kTeclas) {
    if (base::EqualsCaseInsensitiveASCII(nome, tecla.nome)) {
      return tecla.codigo;
    }
  }
  return std::nullopt;
}

std::string NomeDe(ui::KeyboardCode codigo) {
  if (codigo >= ui::VKEY_A && codigo <= ui::VKEY_Z) {
    return std::string(1, static_cast<char>('A' + (codigo - ui::VKEY_A)));
  }
  if (codigo >= ui::VKEY_0 && codigo <= ui::VKEY_9) {
    return std::string(1, static_cast<char>('0' + (codigo - ui::VKEY_0)));
  }
  if (codigo >= ui::VKEY_F1 && codigo <= ui::VKEY_F12) {
    return "F" + base::NumberToString(codigo - ui::VKEY_F1 + 1);
  }
  for (const Tecla& tecla : kTeclas) {
    if (tecla.codigo == codigo) {
      return tecla.nome;
    }
  }
  return std::string();
}

}  // namespace

std::optional<ui::Accelerator> Ler(std::string_view texto) {
  if (texto.empty()) {
    return std::nullopt;
  }
  int modificadores = ui::EF_NONE;
  std::optional<ui::KeyboardCode> codigo;
  for (std::string_view parte : base::SplitStringPiece(
           texto, "+", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY)) {
    if (base::EqualsCaseInsensitiveASCII(parte, "Ctrl")) {
      modificadores |= ui::EF_CONTROL_DOWN;
    } else if (base::EqualsCaseInsensitiveASCII(parte, "Alt")) {
      modificadores |= ui::EF_ALT_DOWN;
    } else if (base::EqualsCaseInsensitiveASCII(parte, "Shift")) {
      modificadores |= ui::EF_SHIFT_DOWN;
    } else if (base::EqualsCaseInsensitiveASCII(parte, "Super")) {
      modificadores |= ui::EF_COMMAND_DOWN;
    } else if (!codigo) {
      codigo = CodigoDe(parte);
      if (!codigo) {
        return std::nullopt;
      }
    } else {
      return std::nullopt;
    }
  }
  if (!codigo) {
    return std::nullopt;
  }
  return ui::Accelerator(*codigo, modificadores);
}

std::string Escrever(const ui::Accelerator& atalho) {
  const std::string tecla = NomeDe(atalho.key_code());
  if (tecla.empty()) {
    return std::string();
  }
  std::vector<std::string> partes;
  if (atalho.IsCtrlDown()) {
    partes.push_back("Ctrl");
  }
  if (atalho.IsAltDown()) {
    partes.push_back("Alt");
  }
  if (atalho.IsShiftDown()) {
    partes.push_back("Shift");
  }
  if (atalho.IsCmdDown()) {
    partes.push_back("Super");
  }
  partes.push_back(tecla);
  return base::JoinString(partes, "+");
}

std::u16string Mostrar(std::string_view texto) {
  std::optional<ui::Accelerator> atalho = Ler(texto);
  if (!atalho) {
    return u"Sem atalho";
  }
  return atalho->GetShortcutText();
}

}  // namespace navegador::atalhos
