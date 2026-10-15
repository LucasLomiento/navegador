// Roda JavaScript numa aba do navegador de teste pelo DevTools (porta 9333).
//   node tools/cdp.mjs <id da aba> '<expressão>'
//   node tools/cdp.mjs navegar <id da aba> <url>
const porta = 9333;
const [a, b, c] = process.argv.slice(2);
const navegar = a === 'navegar';
const id = navegar ? b : a;
const abas = await (await fetch(`http://127.0.0.1:${porta}/json/list`)).json();
const aba = abas.find((t) => t.id === id);
if (!aba) {
  console.error('aba não encontrada');
  process.exit(1);
}
const ws = new WebSocket(aba.webSocketDebuggerUrl);
await new Promise((r) => ws.addEventListener('open', r));
const pedido = navegar
  ? {id: 1, method: 'Page.navigate', params: {url: c}}
  : {id: 1, method: 'Runtime.evaluate',
     params: {expression: b, returnByValue: true, awaitPromise: true}};
ws.send(JSON.stringify(pedido));
ws.addEventListener('message', (m) => {
  const r = JSON.parse(m.data);
  if (r.id !== 1) return;
  console.log(JSON.stringify(r.result?.result?.value ?? r.result ?? r.error));
  ws.close();
});
