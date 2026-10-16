// Gera o site do Norte (GitHub Pages): junta o modelo de site/src com as
// releases de LucasLomiento/norte-releases (novidades, versão e links de
// download) e escreve em site/dist.
//
//   node site/build.mjs            (GITHUB_TOKEN opcional; sem ele, 60 pedidos/h)
//
// O Actions roda isto a cada push que mexe no site e depois de cada versão
// publicada (tools/publicar.sh dispara o .github/workflows/site.yml), então a
// página acompanha as versões sozinha.

import { cpSync, mkdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs'
import { dirname, join } from 'node:path'
import { fileURLToPath } from 'node:url'

const REPO = 'LucasLomiento/norte-releases'
const RAIZ = dirname(fileURLToPath(import.meta.url))
const SAIDA = join(RAIZ, 'dist')
/** Quantas versões aparecem abertas; o resto fica em "versões anteriores". */
const ABERTAS = 3

const esc = (texto) =>
  texto.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;')

/** Negrito, código e link (o que as notas das releases usam). */
function inline(texto) {
  return esc(texto)
    .replace(/`([^`]+)`/g, '<code>$1</code>')
    .replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>')
    .replace(/\[([^\]]+)\]\((https?:[^)\s]+)\)/g, '<a href="$2">$1</a>')
}

/** Notas de uma release em HTML: parágrafos e listas, sem o "Instalar ou atualizar" (igual em todas). */
export function notas(corpo) {
  const html = []
  let lista = false
  let paragrafo = []
  const fecharParagrafo = () => {
    if (paragrafo.length) html.push(`<p>${inline(paragrafo.join(' '))}</p>`)
    paragrafo = []
  }
  const fecharLista = () => {
    if (lista) html.push('</ul>')
    lista = false
  }
  for (const linha of corpo.replace(/\r/g, '').split('\n')) {
    const item = linha.match(/^\s*[-*]\s+(.*)$/)
    if (/^\*\*Instalar ou atualizar/.test(linha.trim())) {
      fecharParagrafo()
      fecharLista()
      continue
    }
    if (item) {
      fecharParagrafo()
      if (!lista) html.push('<ul>')
      lista = true
      html.push(`<li>${inline(item[1])}</li>`)
    } else if (!linha.trim()) {
      fecharParagrafo()
      fecharLista()
    } else {
      fecharLista()
      paragrafo.push(linha.trim())
    }
  }
  fecharParagrafo()
  fecharLista()
  return html.join('\n')
}

const fusoBR = 'America/Sao_Paulo'
const dia = (iso) =>
  new Intl.DateTimeFormat('pt-BR', { day: 'numeric', month: 'long', year: 'numeric', timeZone: fusoBR }).format(new Date(iso))
const diaCurto = (iso) =>
  new Intl.DateTimeFormat('pt-BR', { day: 'numeric', month: 'short', timeZone: fusoBR })
    .format(new Date(iso))
    .replace('.', '')
const hora = (iso) =>
  new Intl.DateTimeFormat('pt-BR', { hour: '2-digit', minute: '2-digit', timeZone: fusoBR })
    .format(new Date(iso))
    .replace(':', 'h')
const megas = (bytes) => `${(bytes / 1024 / 1024).toFixed(0)} MB`

async function releases() {
  const cabecalhos = { Accept: 'application/vnd.github+json', 'User-Agent': 'norte-site' }
  if (process.env.GITHUB_TOKEN) cabecalhos.Authorization = `Bearer ${process.env.GITHUB_TOKEN}`
  const resposta = await fetch(`https://api.github.com/repos/${REPO}/releases?per_page=100`, { headers: cabecalhos })
  if (!resposta.ok) throw new Error(`GitHub respondeu ${resposta.status} ao listar as releases`)
  const todas = await resposta.json()
  return todas
    .filter((r) => !r.draft && !r.prerelease)
    .sort((a, b) => new Date(b.published_at) - new Date(a.published_at))
}

/** "v155.0.8059.79-16" -> { chromium: "155.0.8059.79", pacote: "16" } */
function versao(tag) {
  const m = tag.replace(/^v/, '').match(/^(.*)-(\d+)$/)
  return m ? { chromium: m[1], pacote: m[2] } : { chromium: tag.replace(/^v/, ''), pacote: '' }
}

function novidades(lista) {
  const blocos = lista.map((r, i) => {
    const v = versao(r.tag_name)
    const primeira = i === lista.length - 1
    return `<article class="versao${i === 0 ? ' atual' : ''}" id="${esc(r.tag_name)}">
  <div class="versao-marca" aria-hidden="true"><span></span></div>
  <header>
    <a class="versao-num" href="#${esc(r.tag_name)}"><span class="versao-pkg">${esc(v.pacote)}</span></a>
    <div>
      <h3>${i === 0 ? 'Versão atual' : primeira ? 'Primeira versão pública' : `Versão ${esc(v.pacote)}`}</h3>
      <p class="versao-data"><time datetime="${esc(r.published_at)}">${esc(dia(r.published_at))}, ${esc(hora(r.published_at))}</time> · <code>${esc(r.tag_name)}</code></p>
    </div>
  </header>
  <div class="versao-notas">${notas(r.body || '')}</div>
</article>`
  })
  const abertas = blocos.slice(0, ABERTAS).join('\n')
  const resto = blocos.slice(ABERTAS)
  if (!resto.length) return abertas
  return `${abertas}
<details class="anteriores">
  <summary>Versões anteriores <span>${resto.length}</span></summary>
  ${resto.join('\n')}
</details>`
}

const lista = await releases()
if (!lista.length) throw new Error('Nenhuma release publicada')
const atual = lista[0]
const pacote = atual.assets.find((a) => /\.pkg\.tar\.zst$/.test(a.name))
if (!pacote) throw new Error(`A release ${atual.tag_name} não tem o .pkg.tar.zst`)
const v = versao(atual.tag_name)

const valores = {
  VERSAO: esc(atual.tag_name.replace(/^v/, '')),
  CHROMIUM: esc(v.chromium),
  CHROMIUM_MAIOR: esc(v.chromium.split('.')[0]),
  PACOTE: esc(v.pacote),
  DATA: esc(dia(atual.published_at)),
  DATA_CURTA: esc(diaCurto(atual.published_at)),
  ARQUIVO: esc(pacote.name),
  TAMANHO: esc(megas(pacote.size)),
  LINK_PACOTE: esc(pacote.browser_download_url),
  LINK_VERSAO: esc(atual.html_url),
  NUM_VERSOES: String(lista.length),
  NOVIDADES: novidades(lista),
  ICONES: readFileSync(join(RAIZ, 'src/icones.svg'), 'utf8'),
  ANO: String(new Date().getFullYear()),
}

let html = readFileSync(join(RAIZ, 'src/index.html'), 'utf8')
html = html.replace(/\{\{([A-Z_]+)\}\}/g, (_, chave) => {
  if (!(chave in valores)) throw new Error(`{{${chave}}} sem valor`)
  return valores[chave]
})

rmSync(SAIDA, { recursive: true, force: true })
mkdirSync(SAIDA, { recursive: true })
writeFileSync(join(SAIDA, 'index.html'), html)
for (const arquivo of ['style.css', 'main.js']) cpSync(join(RAIZ, 'src', arquivo), join(SAIDA, arquivo))
cpSync(join(RAIZ, 'assets'), join(SAIDA, 'assets'), { recursive: true })
writeFileSync(join(SAIDA, '.nojekyll'), '')
console.log(`site/dist: ${atual.tag_name}, ${lista.length} versões, ${pacote.name} (${megas(pacote.size)})`)
