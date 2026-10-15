#!/usr/bin/env python3
"""Troca "Chromium" por "Navegador" nos textos da interface, sem perder as
traduções.

O grit identifica cada tradução (.xtb) por um hash do texto em inglês da
mensagem (.grd). Trocar o texto muda o hash e a tradução some (cai pro
inglês). Então, pra cada mensagem de chrome/app/chromium_strings.grd:

  1. calcula o id antigo (texto original) e o novo (texto com Navegador);
  2. em cada chromium_strings_*.xtb, renomeia a tradução do id antigo pro
     novo e troca "Chromium" no texto traduzido.

Idempotente: se o .grd já não tem "Chromium" nas mensagens, não faz nada.
Roda pelo tools/fonte.sh, depois dos patches (os arquivos gerados aqui não
entram em patches/: são muitos e mudam a cada versão).

  python3 -I tools/marca.py [--src ~/Work/navegador-chromium/src]
"""

import argparse
import glob
import os
import re
import sys

ANTIGO = "Chromium"
NOVO = "Navegador"
# Nomes que não são o produto: ficam como estão.
PRESERVAR = ("Chromium OS", "ChromiumOS", "Chromium Authors", "chromium.org",
             "Chromium Embedded")


def trocar(texto):
    marcas = {}
    for i, p in enumerate(PRESERVAR):
        chave = f"\0{i}\0"
        marcas[chave] = p
        texto = texto.replace(p, chave)
    texto = texto.replace(ANTIGO, NOVO)
    for chave, p in marcas.items():
        texto = texto.replace(chave, p)
    return texto


def ids_em_ordem(src, grd):
    sys.path.insert(0, os.path.join(src, "tools", "grit"))
    from grit import grd_reader  # noqa: E402
    from grit.node import message  # noqa: E402

    # Toda variável dos <if expr> vira False: só queremos os ids, de todas
    # as mensagens (o percurso abaixo passa por todos os ramos).
    texto = open(grd, encoding="utf-8").read()
    nomes = set()
    for expr in re.findall(r'expr="([^"]*)"', texto):
        nomes.update(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", expr))
    nomes -= {"and", "or", "not", "True", "False", "in", "pp_if",
              "pp_ifdef", "is_linux", "is_win", "is_macosx", "is_android",
              "is_ios", "is_fuchsia", "is_bsd", "is_posix", "is_chromeos",
              "lang"}
    defines = {n: False for n in nomes}
    raiz = grd_reader.Parse(grd, debug=False, defines=defines)
    # Em ordem, não por nome: o mesmo IDS_ aparece em ramos diferentes
    # (Mac e o resto, por exemplo) com textos diferentes.
    return [node.GetCliques()[0].GetMessage().GetId()
            for node in raiz.Preorder()
            if isinstance(node, message.MessageNode)]


# Mensagens fora do chromium_strings.grd que dizem "Chrome" querendo dizer o
# navegador. Só essas mudam (o arquivo tem "Chrome Web Store" e afins).
ESPECIFICAS = {
    "chrome/app/generated_resources.grd": (
        "chrome/app/resources/generated_resources_*.xtb",
        {"IDS_SHOW_CUSTOMIZE_CHROME_SIDE_PANEL"},
        "Chrome",
    ),
}


def trocar_arquivo(src, grd_rel, xtb_glob, trocar_mensagem, antigo):
    """Troca o texto das mensagens escolhidas e leva as traduções junto."""
    grd = os.path.join(src, grd_rel)
    original = open(grd, encoding="utf-8").read()

    def nas_mensagens(m):
        nome = re.search(r'name="([^"]+)"', m.group(1))
        if not trocar_mensagem(nome.group(1) if nome else ""):
            return m.group(0)
        return m.group(1) + m.group(2).replace(antigo, NOVO) + m.group(3)

    novo = re.sub(r"(<message\b[^>]*>)(.*?)(</message>)", nas_mensagens,
                  original, flags=re.S)
    if novo == original:
        return 0, 0

    ids_antigos = ids_em_ordem(src, grd)
    open(grd, "w", encoding="utf-8").write(novo)
    ids_novos = ids_em_ordem(src, grd)
    assert len(ids_antigos) == len(ids_novos)
    mapa = {a: n for a, n in zip(ids_antigos, ids_novos) if a != n}

    padrao = re.compile(r'<translation id="(\d+)">(.*?)</translation>', re.S)
    total = 0
    for xtb in sorted(glob.glob(os.path.join(src, xtb_glob))):
        conteudo = open(xtb, encoding="utf-8").read()

        def renomear(m):
            nonlocal total
            if m.group(1) not in mapa:
                return m.group(0)
            total += 1
            texto = m.group(2).replace(antigo, NOVO)
            return f'<translation id="{mapa[m.group(1)]}">{texto}</translation>'

        novo_xtb = padrao.sub(renomear, conteudo)
        if novo_xtb != conteudo:
            open(xtb, "w", encoding="utf-8").write(novo_xtb)
    return len(mapa), total


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default=os.path.expanduser(
        "~/Work/navegador-chromium/src"))
    args = ap.parse_args()

    grd = os.path.join(args.src, "chrome/app/chromium_strings.grd")
    original = open(grd, encoding="utf-8").read()

    # Só o conteúdo das <message> muda (não comentários nem atributos).
    def nas_mensagens(m):
        return m.group(1) + trocar(m.group(2)) + m.group(3)

    novo = re.sub(r"(<message\b[^>]*>)(.*?)(</message>)", nas_mensagens,
                  original, flags=re.S)
    if novo == original:
        print("marca: nada a trocar (já aplicado)")
        especificas(args.src)
        return

    ids_antigos = ids_em_ordem(args.src, grd)
    open(grd, "w", encoding="utf-8").write(novo)
    ids_novos = ids_em_ordem(args.src, grd)

    assert len(ids_antigos) == len(ids_novos)
    mapa = {antigo: novo_id
            for antigo, novo_id in zip(ids_antigos, ids_novos)
            if novo_id != antigo}

    padrao = re.compile(r'<translation id="(\d+)">(.*?)</translation>', re.S)
    total = 0
    for xtb in sorted(glob.glob(os.path.join(
            args.src, "chrome/app/resources/chromium_strings_*.xtb"))):
        conteudo = open(xtb, encoding="utf-8").read()

        def renomear(m):
            nonlocal total
            antigo = m.group(1)
            if antigo not in mapa:
                return m.group(0)
            total += 1
            return f'<translation id="{mapa[antigo]}">{trocar(m.group(2))}</translation>'

        open(xtb, "w", encoding="utf-8").write(padrao.sub(renomear, conteudo))

    print(f"marca: {len(mapa)} mensagens com id novo, {total} traduções movidas")
    especificas(args.src)


def especificas(src):
    for grd_rel, (xtb_glob, nomes, antigo) in ESPECIFICAS.items():
        n, t = trocar_arquivo(src, grd_rel, xtb_glob, lambda nome: nome in nomes,
                              antigo)
        if n:
            print(f"marca: {grd_rel}: {n} mensagens, {t} traduções")


if __name__ == "__main__":
    main()
