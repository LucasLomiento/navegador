// Site do Norte: a bússola que segue o ponteiro, a réplica interativa da
// janela (espaços, abas, lateral que some, barra na cor do site) e os
// detalhes dos cartões.

document.documentElement.classList.add('js')
const semMovimento = matchMedia('(prefers-reduced-motion: reduce)').matches
const $ = (seletor, raiz = document) => raiz.querySelector(seletor)
const $$ = (seletor, raiz = document) => [...raiz.querySelectorAll(seletor)]
const icone = (nome) => `<svg class="i"><use href="#i-${nome}"/></svg>`

// --- Topo e entrada ao rolar --------------------------------------------------------

const topo = $('#topo')
const marcarTopo = () => topo.classList.toggle('rolou', scrollY > 8)
addEventListener('scroll', marcarTopo, { passive: true })
marcarTopo()

// Irmãos que surgem juntos entram um pouco depois do outro.
for (const pai of new Set($$('.surgir').map((el) => el.parentElement))) {
  $$(':scope > .surgir', pai).forEach((el, i) => el.style.setProperty('--atraso', `${Math.min(i, 6) * 70}ms`))
}
const observador = new IntersectionObserver(
  (entradas) => {
    for (const e of entradas) {
      if (e.isIntersecting) {
        e.target.classList.add('visivel')
        observador.unobserve(e.target)
      }
    }
  },
  { rootMargin: '0px 0px -8% 0px' },
)
$$('.surgir').forEach((el) => observador.observe(el))

// --- Bússola ---------------------------------------------------------------------------

const marcas = $('.aro-marcas')
if (marcas) {
  let linhas = ''
  for (let g = 0; g < 360; g += 15) {
    const forte = g % 90 === 0
    if (forte) continue // o lugar das letras N, L, S, O
    const r1 = g % 45 === 0 ? 80 : 84
    const a = (g * Math.PI) / 180
    const [x1, y1] = [100 + Math.sin(a) * r1, 100 - Math.cos(a) * r1]
    const [x2, y2] = [100 + Math.sin(a) * 90, 100 - Math.cos(a) * 90]
    linhas += `<line x1="${x1.toFixed(1)}" y1="${y1.toFixed(1)}" x2="${x2.toFixed(1)}" y2="${y2.toFixed(1)}"${g % 45 === 0 ? ' class="forte"' : ''}/>`
  }
  marcas.innerHTML = linhas
}

const agulha = $('#bussola-agulha')
if (agulha) {
  // A agulha aponta pro ponteiro; parado, ela balança devagar em volta do nordeste.
  let angulo = 45
  let alvo = 45
  let ultimoMovimento = -1e9
  addEventListener(
    'pointermove',
    (e) => {
      const r = agulha.getBoundingClientRect()
      const dx = e.clientX - (r.left + r.width / 2)
      const dy = e.clientY - (r.top + r.height / 2)
      if (Math.hypot(dx, dy) < 30) return
      alvo = (Math.atan2(dy, dx) * 180) / Math.PI + 90
      ultimoMovimento = performance.now()
    },
    { passive: true },
  )
  const girar = (agora) => {
    if (agora - ultimoMovimento > 2500) alvo = 45 + Math.sin(agora / 1400) * 10
    let d = ((alvo - angulo + 540) % 360) - 180 // o caminho mais curto
    angulo += d * 0.09
    agulha.style.setProperty('--ang', `${angulo.toFixed(2)}deg`)
    requestAnimationFrame(girar)
  }
  if (!semMovimento) requestAnimationFrame(girar)
}

// --- Réplica da janela ------------------------------------------------------------------

const ESPACOS = [
  {
    nome: 'Pessoal',
    cor: '#3aa0ff',
    fixados: [
      { letra: 'M', fundo: '#2b6cf5' },
      { letra: 'S', fundo: '#1db954' },
      { letra: 'N', fundo: '#e50914' },
    ],
    abas: [
      { titulo: 'Florianópolis – Wikipédia', dominio: 'pt.wikipedia.org', pagina: 'artigo', favicon: { letra: 'W', fundo: '#fff', cor: '#111' }, topo: '#ffffff', barrados: 0 },
      { titulo: 'OpenStreetMap', dominio: 'openstreetmap.org', pagina: 'mapa', favicon: { letra: 'O', fundo: '#7ebc6f' }, topo: '#f2efe9', barrados: 2 },
      { titulo: 'Hacker News', dominio: 'news.ycombinator.com', pagina: 'lista', favicon: { letra: 'Y', fundo: '#ff6600' }, topo: '#ff6600', barrados: 0 },
      { titulo: 'Nova guia', pagina: 'nova', topo: '#141417' },
    ],
    ativa: 0,
  },
  {
    nome: 'Trabalho',
    cor: '#ff9f0a',
    container: '#ff9f0a',
    fixados: [
      { letra: 'C', fundo: '#4f46e5' },
      { letra: 'D', fundo: '#0ea5e9' },
    ],
    abas: [
      { titulo: 'Lateral nova · Pull request #42', dominio: 'github.com', pagina: 'codigo', favicon: { github: true, fundo: '#0d1117' }, topo: '#010409', barrados: 3 },
      { titulo: 'CSS | MDN', dominio: 'developer.mozilla.org', pagina: 'artigo-escuro', favicon: { letra: 'M', fundo: '#1b1b1b' }, topo: '#1b1b1b', barrados: 1 },
      { titulo: 'Ícones – Lucide', dominio: 'lucide.dev', pagina: 'grade', favicon: { letra: 'L', fundo: '#f56565' }, topo: '#0f0f10', barrados: 0 },
    ],
    ativa: 0,
  },
  {
    nome: 'Estudos',
    cor: '#4cd137',
    fixados: [{ letra: 'A', fundo: '#f59e0b' }],
    abas: [
      { titulo: 'Vocabulário do mar', dominio: 'cartoes.app', pagina: 'cartas', favicon: { letra: 'V', fundo: '#4cd137' }, topo: '#fbfaf7', barrados: 0 },
      { titulo: 'Teorema de Pitágoras – Wikipédia', dominio: 'pt.wikipedia.org', pagina: 'artigo-pitagoras', favicon: { letra: 'W', fundo: '#fff', cor: '#111' }, topo: '#ffffff', barrados: 0 },
      { titulo: 'Nova guia', pagina: 'nova', topo: '#141417' },
    ],
    ativa: 0,
  },
]

const hojeHora = () => new Date().toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit' })
const hojeData = () => {
  const t = new Date().toLocaleDateString('pt-BR', { weekday: 'long', day: 'numeric', month: 'long' })
  return t.charAt(0).toUpperCase() + t.slice(1)
}

function linhas(larguras, link = []) {
  return larguras.map((l, i) => `<i style="width:${l}%"${link.includes(i) ? ' class="link"' : ''}></i>`).join('')
}

const PAGINAS = {
  nova: () => `<div class="p-nova">
      <div class="hora" data-relogio>${hojeHora()}</div>
      <div class="data" data-data>${hojeData()}</div>
      <div class="atalhos-n">
        ${[['W', '#fff', '#111', 'Wikipédia'], ['Y', '#ff6600', '#fff', 'Hacker News'], ['L', '#f56565', '#fff', 'Lucide'], ['M', '#1b1b1b', '#fff', 'MDN'],
           ['O', '#7ebc6f', '#fff', 'Mapas'], ['G', '#0d1117', '#fff', 'GitHub'], ['V', '#4cd137', '#fff', 'Cartões'], ['+', 'rgba(255,255,255,.06)', '#a1a1aa', 'Adicionar']]
          .map(([l, f, c, n]) => `<span class="atalho-n"><i style="--fc:${c};background:${f}">${l}</i>${n}</span>`).join('')}
      </div>
    </div>`,
  artigo: () => `<div class="p-artigo">
      <div class="wiki-topo"><span class="wiki-logo"></span><span class="wiki-nome">WIKIPÉDIA</span><span class="wiki-busca"></span></div>
      <h4>Florianópolis</h4>
      <div class="wiki-corpo">
        <div class="linhas">${linhas([96, 92, 98, 88, 94, 60, 0, 97, 90, 95, 93, 70, 0, 91, 96, 85], [1, 4, 8, 11, 14])}</div>
        <div class="caixa"><div class="foto" style="--foto:linear-gradient(180deg,#8ec5ff 0%,#cfe8ff 38%,#2f8fb8 39%,#1d6f95 62%,#4f8f4a 63%,#3f7a3c 100%)"></div><i></i><i style="width:70%"></i><i></i><i style="width:55%"></i></div>
      </div>
    </div>`,
  'artigo-pitagoras': () => `<div class="p-artigo">
      <div class="wiki-topo"><span class="wiki-logo"></span><span class="wiki-nome">WIKIPÉDIA</span><span class="wiki-busca"></span></div>
      <h4>Teorema de Pitágoras</h4>
      <div class="wiki-corpo">
        <div class="linhas">${linhas([94, 97, 90, 62, 0, 96, 92, 88, 95, 74, 0, 93, 97, 80], [2, 6, 9, 12])}</div>
        <div class="caixa"><svg viewBox="0 0 200 150" style="width:100%;height:150px;background:#fff"><polygon points="40,120 160,120 40,40" fill="none" stroke="#202122" stroke-width="2"/><rect x="40" y="108" width="12" height="12" fill="none" stroke="#202122" stroke-width="1.5"/><text x="95" y="138" font-size="13" font-family="Georgia" fill="#202122">b</text><text x="24" y="85" font-size="13" font-family="Georgia" fill="#202122">a</text><text x="104" y="76" font-size="13" font-family="Georgia" fill="#202122">c</text><text x="112" y="40" font-size="14" font-family="Georgia" font-style="italic" fill="#3366cc">a²+b²=c²</text></svg><i></i><i style="width:60%"></i></div>
      </div>
    </div>`,
  'artigo-escuro': () => `<div class="p-artigo escuro">
      <div class="wiki-topo"><span class="wiki-logo">M</span><span class="wiki-nome">mdn_</span><span class="wiki-busca"></span></div>
      <h4>CSS</h4>
      <div class="wiki-corpo">
        <div class="linhas">${linhas([95, 90, 97, 70, 0, 93, 96, 88, 91, 64, 0, 97, 85, 92], [1, 6, 9, 13])}</div>
        <div class="caixa"><i></i><i style="width:80%"></i><i style="width:65%"></i><i></i><i style="width:72%"></i><i style="width:50%"></i></div>
      </div>
    </div>`,
  mapa: () => `<div class="p-mapa">
      <svg viewBox="0 0 900 560" preserveAspectRatio="xMidYMid slice">
        <rect width="900" height="560" fill="#f2efe9"/>
        <path d="M0 0 H380 C340 90 300 120 330 210 C360 300 290 360 250 430 C220 480 230 520 260 560 H0 Z" fill="#aad3df"/>
        <path d="M640 560 C660 470 720 430 800 420 C850 414 880 380 900 360 V560 Z" fill="#aad3df"/>
        <path d="M470 80 C520 60 600 90 610 140 C620 190 560 220 510 200 C460 180 440 110 470 80 Z" fill="#c8facc"/>
        <path d="M560 300 C600 280 680 300 690 350 C700 400 630 420 590 400 C550 380 530 320 560 300 Z" fill="#c8facc"/>
        <path d="M330 210 C420 230 520 250 900 230" stroke="#fff" stroke-width="14" fill="none"/>
        <path d="M330 210 C420 230 520 250 900 230" stroke="#f9b29c" stroke-width="8" fill="none"/>
        <path d="M420 0 C440 120 470 260 480 560" stroke="#fff" stroke-width="10" fill="none"/>
        <path d="M420 0 C440 120 470 260 480 560" stroke="#fcd6a4" stroke-width="5" fill="none"/>
        <path d="M600 0 C610 100 640 200 700 560" stroke="#fff" stroke-width="7" fill="none"/>
        <path d="M250 430 C400 400 560 470 900 480" stroke="#fff" stroke-width="7" fill="none"/>
        <g fill="#d9d0c9">${Array.from({ length: 26 }, (_, i) => `<rect x="${520 + (i % 6) * 34}" y="${150 + Math.floor(i / 6) * 30}" width="24" height="18" rx="2"/>`).join('')}</g>
      </svg>
      <div class="busca-mapa">Florianópolis, Santa Catarina</div>
      <div class="pino"></div>
    </div>`,
  lista: () => `<div class="p-lista">
      <div class="hn-topo"><b>Y</b><strong>Hacker News</strong><span>new | past | comments | ask | show</span></div>
      <ol>${[78, 64, 88, 55, 71, 82, 60, 76, 69, 85, 58, 73].map((l) => `<li><i style="width:${l}%"></i><u></u></li>`).join('')}</ol>
    </div>`,
  codigo: () => `<div class="p-codigo">
      <div class="gh-topo"><svg><use href="#i-github"/></svg><span>LucasLomiento / <b>norte</b></span></div>
      <div class="pr-titulo">Lateral que some com Ctrl+S <em>#42</em></div>
      <span class="pr-estado"><svg class="i"><use href="#i-git-pull-request"/></svg>Aberto</span>
      <div class="arquivo">
        <div class="arquivo-nome">navegador/browser/ui/espacos.cc</div>
        <div class="ln" data-n="118"><span class="c">// Some com Ctrl+S; volta ao encostar na borda.</span></div>
        <div class="ln menos" data-n="119">- <span class="k">void</span> <span class="f">Esconder</span>() { lateral_-&gt;<span class="f">SetVisible</span>(<span class="n">false</span>); }</div>
        <div class="ln mais" data-n="119">+ <span class="k">void</span> <span class="f">Esconder</span>(<span class="k">bool</span> animar) {</div>
        <div class="ln mais" data-n="120">+   lateral_-&gt;<span class="f">Recolher</span>(animar ? <span class="n">kFolga</span> : <span class="n">0</span>);</div>
        <div class="ln mais" data-n="121">+   borda_.<span class="f">Ligar</span>(<span class="s">"espiar"</span>);</div>
        <div class="ln mais" data-n="122">+ }</div>
        <div class="ln" data-n="123"></div>
        <div class="ln" data-n="124"><span class="k">bool</span> <span class="f">Visivel</span>() <span class="k">const</span> { <span class="k">return</span> !escondida_; }</div>
      </div>
    </div>`,
  grade: () => `<div class="p-grade">
      <div class="lu-topo"><i></i>Lucide</div>
      <div class="lu-busca"><svg class="i"><use href="#i-search"/></svg>Buscar ícones…</div>
      <div class="icones">${['layers', 'briefcase', 'shield-check', 'clock', 'paintbrush', 'globe', 'pin', 'download', 'keyboard', 'settings', 'map', 'zap', 'code', 'cookie', 'bell-ring', 'users', 'monitor', 'package', 'terminal', 'search', 'list', 'file-text', 'copy', 'check', 'gauge', 'memory-stick', 'sparkles', 'eye-off', 'hammer', 'import']
        .map((n) => `<span>${icone(n)}</span>`).join('')}</div>
    </div>`,
  cartas: () => `<div class="p-cartas">
      <h4>Vocabulário do mar</h4>
      <p>5 cartões · 3 revisados hoje</p>
      <div class="cartas">${[['compass', 'bússola', 1], ['harbor', 'porto', 1], ['sail', 'vela', 1], ['wind', 'vento', 0], ['anchor', 'âncora', 0]]
        .map(([en, pt, feita]) => `<div class="carta${feita ? ' feita' : ''}"><b>${en}</b><span>${pt}</span></div>`).join('')}</div>
    </div>`,
}

const janela = $('#janela')
if (janela) {
  const lateral = $('#lateral')
  const borda = $('#janela-borda')
  const nome = $('#lat-nome')
  const seletor = $('#lat-seletor')
  const fixados = $('#lat-fixados')
  const abas = $('#lat-abas')
  const bolinhas = $('#lat-bolinhas')
  const endereco = $('#endereco')
  const escudoNum = $('#escudo-num')
  const pagina = $('#pagina')
  let atual = 0

  const favicon = (aba) => {
    if (aba.pagina === 'nova') return `<span class="favicon agulha"><svg><use href="#agulha-mono"/></svg></span>`
    const f = aba.favicon || {}
    if (f.github) return `<span class="favicon" style="--ff:${f.fundo};--fc:#fff"><svg style="width:12px;height:12px"><use href="#i-github"/></svg></span>`
    return `<span class="favicon" style="--ff:${f.fundo};--fc:${f.cor || '#fff'}">${f.letra}</span>`
  }

  function desenharPagina() {
    const e = ESPACOS[atual]
    const aba = e.abas[e.ativa]
    janela.style.setProperty('--topo-pag', aba.topo)
    endereco.innerHTML = aba.pagina === 'nova' ? '<span>Pesquise ou digite o endereço</span>' : `<b>${aba.dominio}</b> / ${aba.titulo}`
    escudoNum.textContent = aba.barrados ?? 0
    pagina.innerHTML = PAGINAS[aba.pagina]()
  }

  function desenhar() {
    const e = ESPACOS[atual]
    janela.style.setProperty('--cor', e.cor)
    nome.textContent = e.nome
    fixados.innerHTML = e.fixados.map((f) => `<span class="fixado"><b style="--ff:${f.fundo}">${f.letra}</b></span>`).join('')
    abas.innerHTML = e.abas
      .map(
        (a, i) => `<li class="aba${i === e.ativa ? ' ativa' : ''}${e.container ? ' container' : ''}" style="--cc:${e.container || 'transparent'}" data-i="${i}">
          ${favicon(a)}<span class="aba-titulo">${a.titulo}</span>
          <button class="aba-fechar" type="button" aria-label="Fechar ${a.titulo}" data-fechar="${i}">${icone('x')}</button></li>`,
      )
      .join('')
    bolinhas.innerHTML = ESPACOS.map(
      (s, i) => `<button type="button" style="--c:${s.cor}" class="${i === atual ? 'atual' : ''}" aria-label="Espaço ${s.nome}" data-espaco="${i}"></button>`,
    ).join('')
    seletor.innerHTML = ESPACOS.map(
      (s, i) => `<button type="button" class="${i === atual ? 'atual' : ''}" data-espaco="${i}" style="--c:${s.cor}"><i></i>${s.nome}</button>`,
    ).join('')
    desenharPagina()
  }

  const trocarEspaco = (i) => {
    atual = (i + ESPACOS.length) % ESPACOS.length
    seletor.hidden = true
    desenhar()
  }
  const alternarLateral = () => {
    janela.classList.toggle('sem-lateral')
    janela.classList.remove('espiando')
  }

  abas.addEventListener('click', (ev) => {
    const e = ESPACOS[atual]
    const fechar = ev.target.closest('[data-fechar]')
    if (fechar) {
      const i = Number(fechar.dataset.fechar)
      e.abas.splice(i, 1)
      if (!e.abas.length) e.abas.push({ titulo: 'Nova guia', pagina: 'nova', topo: '#141417' })
      if (e.ativa >= i) e.ativa = Math.max(0, e.ativa - 1)
      desenhar()
      return
    }
    const aba = ev.target.closest('.aba')
    if (aba) {
      e.ativa = Number(aba.dataset.i)
      desenhar()
    }
  })
  $('#lat-nova').addEventListener('click', () => {
    const e = ESPACOS[atual]
    e.abas.push({ titulo: 'Nova guia', pagina: 'nova', topo: '#141417' })
    e.ativa = e.abas.length - 1
    desenhar()
  })
  bolinhas.addEventListener('click', (ev) => {
    const b = ev.target.closest('[data-espaco]')
    if (b) trocarEspaco(Number(b.dataset.espaco))
  })
  $('#lat-espaco').addEventListener('click', (ev) => {
    ev.stopPropagation()
    seletor.hidden = !seletor.hidden
  })
  seletor.addEventListener('click', (ev) => {
    const b = ev.target.closest('[data-espaco]')
    if (b) trocarEspaco(Number(b.dataset.espaco))
  })
  document.addEventListener('click', (ev) => {
    if (!seletor.hidden && !ev.target.closest('.lat-topo')) seletor.hidden = true
  })

  // Os botões laterais do mouse, com o ponteiro na lateral, trocam de espaço
  // (e não voltam a página do site).
  lateral.addEventListener('mouseup', (ev) => {
    if (ev.button === 3 || ev.button === 4) {
      ev.preventDefault()
      trocarEspaco(atual + (ev.button === 4 ? 1 : -1))
    }
  })
  lateral.addEventListener('mousedown', (ev) => {
    if (ev.button === 3 || ev.button === 4) ev.preventDefault()
  })

  // Lateral escondida: encostar na borda mostra por cima; sair dela esconde.
  let esconderDepois
  borda.addEventListener('pointerenter', () => {
    clearTimeout(esconderDepois)
    janela.classList.add('espiando')
  })
  lateral.addEventListener('pointerenter', () => clearTimeout(esconderDepois))
  lateral.addEventListener('pointerleave', () => {
    esconderDepois = setTimeout(() => janela.classList.remove('espiando'), 150)
  })

  // Os atalhos valem com o ponteiro em cima da janela (ou com ela em foco).
  let ponteiroDentro = false
  janela.addEventListener('pointerenter', () => (ponteiroDentro = true))
  janela.addEventListener('pointerleave', () => (ponteiroDentro = false))
  document.addEventListener('keydown', (ev) => {
    if (!ponteiroDentro && document.activeElement !== janela && !janela.contains(document.activeElement)) return
    const ctrl = ev.ctrlKey || ev.metaKey
    if (ctrl && !ev.altKey && ev.key.toLowerCase() === 's') {
      ev.preventDefault()
      alternarLateral()
    } else if (ctrl && ev.altKey && (ev.key === 'ArrowRight' || ev.key === 'ArrowLeft')) {
      ev.preventDefault()
      trocarEspaco(atual + (ev.key === 'ArrowRight' ? 1 : -1))
    }
  })
  $$('.dica[data-acao]').forEach((b) =>
    b.addEventListener('click', () => (b.dataset.acao === 'lateral' ? alternarLateral() : trocarEspaco(atual + 1))),
  )

  desenhar()

  // Escala (a janela tem 1120×660; em tela menor encolhe inteira) e a
  // inclinação que some conforme a página rola.
  const escala = $('#demo-escala')
  const palco = $('#demo-palco')
  new ResizeObserver(([e]) => {
    escala.style.setProperty('--k', Math.min(1, e.contentRect.width / 1120).toFixed(4))
  }).observe(escala)
  if (!semMovimento) {
    let pedido = 0
    const inclinar = () => {
      pedido = 0
      const r = escala.getBoundingClientRect()
      const t = Math.max(0, Math.min(1, (r.top - innerHeight * 0.18) / (innerHeight * 0.55)))
      palco.style.setProperty('--t', t.toFixed(3))
    }
    addEventListener('scroll', () => (pedido ||= requestAnimationFrame(inclinar)), { passive: true })
    addEventListener('resize', inclinar)
    inclinar()
  }
}

// --- Cartões ----------------------------------------------------------------------------

// Relógios (o da réplica e o do cartão "Nova aba").
const atualizarRelogios = () => {
  $$('[data-relogio]').forEach((el) => (el.textContent = hojeHora()))
  $$('[data-data]').forEach((el) => (el.textContent = hojeData()))
}
atualizarRelogios()
setInterval(atualizarRelogios, 5000)

// Espaços: o destaque passa de um espaço pro outro.
const ilusEspacos = $('.ilus-espacos')
if (ilusEspacos) {
  const pilulas = $$('.ie-pilulas span', ilusEspacos)
  const pontos = $$('.ie-bolinhas b', ilusEspacos)
  const cores = ['#3aa0ff', '#ff9f0a', '#4cd137']
  const nomeIe = $('.ie-nome span', ilusEspacos)
  let i = 0
  const passo = () => {
    pilulas.forEach((p, j) => p.classList.toggle('atual', j === i))
    pontos.forEach((p, j) => p.classList.toggle('atual', j === i))
    ilusEspacos.style.setProperty('--ie', cores[i])
    nomeIe.textContent = pilulas[i].textContent
    i = (i + 1) % pilulas.length
  }
  passo()
  if (!semMovimento) setInterval(passo, 2200)
}

// Contador do bloqueador: conta quando aparece.
$$('.contador-num').forEach((el) => {
  const alvo = Number(el.dataset.alvo)
  const formato = new Intl.NumberFormat('pt-BR')
  const contar = () => {
    if (semMovimento) return (el.textContent = formato.format(alvo))
    const inicio = performance.now()
    const quadro = (agora) => {
      const p = Math.min(1, (agora - inicio) / 1600)
      el.textContent = formato.format(Math.round(alvo * (1 - Math.pow(1 - p, 3))))
      if (p < 1) requestAnimationFrame(quadro)
    }
    requestAnimationFrame(quadro)
  }
  new IntersectionObserver((entradas, obs) => {
    if (entradas[0].isIntersecting) {
      contar()
      obs.disconnect()
    }
  }).observe(el)
})

// Luz que segue o ponteiro nos cartões.
$$('.cartao').forEach((c) =>
  c.addEventListener('pointermove', (e) => {
    const r = c.getBoundingClientRect()
    c.style.setProperty('--mx', `${e.clientX - r.left}px`)
    c.style.setProperty('--my', `${e.clientY - r.top}px`)
  }),
)

// --- Galeria ---------------------------------------------------------------------------

const img = $('#galeria-img')
if (img) {
  const legenda = $('#galeria-legenda')
  const botoes = $$('.galeria-lista [data-foto]')
  // Pré-carrega as outras fotos depois que a página assenta.
  addEventListener('load', () =>
    setTimeout(() => botoes.forEach((b) => (new Image().src = `assets/fotos/${b.dataset.foto}.webp`)), 1200),
  )
  botoes.forEach((b) =>
    b.addEventListener('click', () => {
      botoes.forEach((x) => x.setAttribute('aria-selected', String(x === b)))
      const foto = b.dataset.foto
      img.classList.add('trocando')
      const nova = new Image()
      nova.srcset = `assets/fotos/${foto}.webp 1440w, assets/fotos/${foto}@2x.webp 2880w`
      nova.sizes = img.sizes
      nova.src = `assets/fotos/${foto}.webp`
      const mostrar = () => {
        img.srcset = nova.srcset
        img.src = nova.src
        img.alt = `${b.textContent.trim()}: ${b.dataset.legenda}`
        legenda.textContent = b.dataset.legenda
        img.classList.remove('trocando')
      }
      nova.decode().then(mostrar, mostrar)
    }),
  )
}

// --- Copiar ----------------------------------------------------------------------------

$$('.copiar').forEach((b) =>
  b.addEventListener('click', async () => {
    const texto = $(`#${b.dataset.copiar}`).innerText.replace(/^\$ /gm, '')
    try {
      await navigator.clipboard.writeText(texto)
    } catch {
      const area = Object.assign(document.createElement('textarea'), { value: texto })
      document.body.append(area)
      area.select()
      document.execCommand('copy')
      area.remove()
    }
    const rotulo = $('span', b)
    b.classList.add('feito')
    rotulo.textContent = 'Copiado'
    setTimeout(() => {
      b.classList.remove('feito')
      rotulo.textContent = 'Copiar'
    }, 1800)
  }),
)
