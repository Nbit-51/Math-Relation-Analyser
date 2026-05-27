/**
 * app.js — UI layer (presentation only; all analysis runs in C++ / WASM)
 */

const SECTIONS = ['intro', 'properties', 'types', 'mappings', 'analyser', 'hasse'];

const EXAMPLES = {
  equiv:      { A: '1, 2, 3', R: '(1,1); (2,2); (3,3); (1,2); (2,1); (2,3); (3,2); (1,3); (3,1)' },
  partial:    { A: '1, 2, 3', R: '(1,1); (2,2); (3,3); (1,2); (1,3); (2,3)' },
  total:      { A: '1, 2, 3', R: '(1,1); (2,2); (3,3); (1,2); (2,3); (1,3)' },
  divisibility:{ A: '1, 2, 3, 4, 6, 12', R: '(1,1); (2,2); (3,3); (4,4); (6,6); (12,12); (1,2); (1,3); (1,4); (1,6); (1,12); (2,4); (2,6); (2,12); (3,6); (3,12); (4,12); (6,12)' },
  sym:        { A: '1, 2, 3', R: '(1,2); (2,1); (2,3); (3,2)' },
  empty:      { A: '1, 2, 3', R: '' },
  identity:   { A: '1, 2, 3', R: '(1,1); (2,2); (3,3)' },
  asymmetric: { A: '1, 2, 3', R: '(1,2); (2,3); (1,3)' },
  bijection:  { A: '1, 2, 3', R: '(1,2); (2,3); (3,1)' },
};

function $(id) {
  return document.getElementById(id);
}

function showSection(id) {
  SECTIONS.forEach(sec => {
    const el = $('sec-' + sec);
    el.classList.toggle('active', sec === id);
    el.hidden = sec !== id;
  });

  document.querySelectorAll('.nav-btn').forEach(btn => {
    const active = btn.dataset.section === id;
    btn.classList.toggle('active', active);
    btn.setAttribute('aria-selected', String(active));
    btn.tabIndex = active ? 0 : -1;
  });

  window.scrollTo({ top: 0, behavior: prefersReducedMotion() ? 'auto' : 'smooth' });
}

function prefersReducedMotion() {
  return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
}

function toggleAcc(id) {
  const body    = $('bd-' + id);
  const chevron = $('ch-' + id);
  const header  = $('hdr-' + id);
  const isOpen  = body.classList.toggle('open');
  chevron.classList.toggle('open', isOpen);
  if (header) header.setAttribute('aria-expanded', String(isOpen));
}

function loadExample(key) {
  const ex = EXAMPLES[key];
  if (!ex) return;
  setActiveInputs(ex.A, ex.R);
  getActiveAnalyser() === 'hasse' ? analyseHasse() : analyse();
}

function clearAll() {
  const active = getActiveAnalyser();
  const setEl = active === 'hasse' ? $('hasseSetA') : $('setA');
  const relEl = active === 'hasse' ? $('hasseRelR') : $('relR');
  setEl.value = '';
  relEl.value = '';
  if (active === 'hasse') $('hasse-result-panel').hidden = true;
  else $('result-panel').hidden = true;
  hideErr();
  hideHasseErr();
  setEl.focus();
}

function showErr(msg) {
  const el = $('err');
  el.textContent = msg;
  el.hidden = false;
}

function hideErr() {
  $('err').hidden = true;
}

function showHasseErr(msg) {
  const el = $('hasse-err');
  el.textContent = msg;
  el.hidden = false;
}

function hideHasseErr() {
  const el = $('hasse-err');
  if (el) el.hidden = true;
}

function showMatrixErr(msg) {
  const el = $('matrix-err');
  el.textContent = msg;
  el.hidden = false;
}

function hideMatrixErr() {
  const el = $('matrix-err');
  if (el) el.hidden = true;
}

function getActiveAnalyser() {
  return $('sec-hasse')?.classList.contains('active') ? 'hasse' : 'relation';
}

function setActiveInputs(setRaw, relRaw) {
  if (getActiveAnalyser() === 'hasse') {
    $('hasseSetA').value = setRaw;
    $('hasseRelR').value = relRaw;
  } else {
    $('setA').value = setRaw;
    $('relR').value = relRaw;
  }
}

function setAnalysing(busy) {
  const btn = $('btn-analyse');
  btn.disabled = busy || !RelationEngine.isReady();
  btn.setAttribute('aria-busy', String(busy));
  btn.textContent = busy ? 'Analysing…' : 'Analyse relation';
  $('analyser-form').setAttribute('aria-busy', String(busy));
}

function setHasseAnalysing(busy) {
  const btn = $('btn-hasse-analyse');
  btn.disabled = busy || !RelationEngine.isReady();
  btn.setAttribute('aria-busy', String(busy));
  btn.textContent = busy ? 'Drawing…' : 'Analyse Hasse diagram';
  $('hasse-form').setAttribute('aria-busy', String(busy));
}

function setMatrixAnalysing(busy) {
  const btn = $('btn-matrix-analyse');
  btn.disabled = busy || !RelationEngine.isReady();
  btn.setAttribute('aria-busy', String(busy));
  btn.textContent = busy ? 'Generating…' : 'Generate matrix';
  $('matrix-form').setAttribute('aria-busy', String(busy));
}

function setEngineStatus(state, message) {
  const loader = $('engine-loader');
  const badge  = $('engine-badge');
  loader.dataset.state = state;
  loader.querySelector('.loader-text').textContent = message;

  if (state === 'ready') {
    loader.hidden = true;
    const banner = $('engine-error-banner');
    if (banner) banner.hidden = true;
    badge.textContent = 'C++ / WebAssembly engine';
    badge.className = 'engine-badge engine-ready';
    $('btn-analyse').disabled = false;
    const hasseBtn = $('btn-hasse-analyse');
    if (hasseBtn) hasseBtn.disabled = false;
    const matrixBtn = $('btn-matrix-analyse');
    if (matrixBtn) matrixBtn.disabled = false;
  } else if (state === 'error') {
    loader.hidden = true;
    badge.textContent = 'Engine unavailable';
    badge.className = 'engine-badge engine-error';
    $('btn-analyse').disabled = true;
    const hasseBtn = $('btn-hasse-analyse');
    if (hasseBtn) hasseBtn.disabled = true;
    const matrixBtn = $('btn-matrix-analyse');
    if (matrixBtn) matrixBtn.disabled = true;
    const banner = $('engine-error-banner');
    if (banner) {
      banner.hidden = false;
      banner.textContent = message;
    }
  } else {
    const banner = $('engine-error-banner');
    if (banner) banner.hidden = true;
    loader.hidden = false;
    badge.textContent = 'Initialising engine…';
    badge.className = 'engine-badge engine-loading';
  }
}

async function analyse() {
  if (!RelationEngine.isReady()) {
    showErr('Analysis engine is not ready. Run build-wasm.bat and serve over HTTP.');
    return;
  }

  hideErr();
  setAnalysing(true);

  try {
    const rawA = $('setA').value;
    const rawR = $('relR').value;
    const result = await RelationEngine.analyse(rawA, rawR);
    renderResult(result);
    announce(`Analysis complete. ${result.classifications.length} classification(s) found.`);
  } catch (err) {
    showErr(err.message || 'Analysis failed.');
    $('result-panel').hidden = true;
  } finally {
    setAnalysing(false);
  }
}

function announce(msg) {
  const live = $('sr-announcer');
  live.textContent = '';
  requestAnimationFrame(() => { live.textContent = msg; });
}

function formatPairs(pairs, max = 20) {
  if (!pairs.length) return '∅';
  const shown = pairs.slice(0, max).map(([a, b]) => `(${a},${b})`).join(', ');
  return pairs.length > max ? `${shown} … (+${pairs.length - max} more)` : shown;
}

function renderResult(r) {
  renderMatrix(r);
  renderProps(r);
  renderMapping(r);
  renderOperations(r);
  renderHasseSummary(r);
  renderBadges(r);
  renderInferences(r);

  const panel = $('result-panel');
  panel.hidden = false;
  panel.scrollIntoView({ behavior: prefersReducedMotion() ? 'auto' : 'smooth', block: 'nearest' });
}

function renderMatrix(r) {
  $('res-matrix').innerHTML = buildMatrixHtml(r, 'matrix-caption');
}

function buildMatrixHtml(r, captionId) {
  const { elements, matrix } = r;
  const caption = `Boolean adjacency matrix for relation R on set {${elements.join(', ')}}`;

  let html = `<div class="matrix-label">Boolean matrix <var>M</var>[<var>i</var>][<var>j</var>]</div>`;
  html += `<div class="matrix-wrap"><table class="matrix-table" aria-describedby="${captionId}"><caption id="${captionId}" class="visually-hidden">${escapeHtml(caption)}</caption><thead><tr><th scope="col"><span class="visually-hidden">Row and column headers</span></th>`;
  for (const b of elements) html += `<th scope="col">${escapeHtml(b)}</th>`;
  html += '</tr></thead><tbody>';

  for (let i = 0; i < elements.length; i++) {
    html += `<tr><th scope="row">${escapeHtml(elements[i])}</th>`;
    for (let j = 0; j < elements.length; j++) {
      const v = matrix[i][j];
      const cls = i === j ? `cell-diag ${v ? 'cell-1' : 'cell-0'}` : (v ? 'cell-1' : 'cell-0');
      html += `<td class="${cls}" aria-label="(${escapeHtml(elements[i])}, ${escapeHtml(elements[j])}) ${v ? 'in R' : 'not in R'}">${v}</td>`;
    }
    html += '</tr>';
  }
  html += '</tbody></table></div>';
  return html;
}

async function analyseMatrix() {
  if (!RelationEngine.isReady()) {
    showMatrixErr('Analysis engine is not ready. Run build-wasm.bat and serve over HTTP.');
    return;
  }

  hideMatrixErr();
  setMatrixAnalysing(true);

  try {
    const result = await RelationEngine.analyse($('matrixSetA').value, $('matrixRelR').value);
    renderMatrixTool(result);
    announce(`Relation matrix generated: ${result.n} by ${result.n}.`);
  } catch (err) {
    showMatrixErr(err.message || 'Matrix generation failed.');
    $('matrix-result-panel').hidden = true;
  } finally {
    setMatrixAnalysing(false);
  }
}

function renderMatrixTool(r) {
  $('matrix-output').innerHTML = buildMatrixHtml(r, 'matrix-tool-caption');
  $('matrix-details').innerHTML = `
    <div class="mapping-summary">Rows and columns follow A = {${escapeHtml(r.elements.join(', '))}}. Entry M[i][j] is 1 exactly when the C++ engine found (row, column) in R.</div>
    <dl class="prop-list">
      <div class="prop-row"><dt class="name">Order</dt><dd class="val">${r.n} × ${r.n}</dd></div>
      <div class="prop-row"><dt class="name">Pairs in R</dt><dd class="val val-yes">${r.pairs.length}</dd></div>
      <div class="prop-row"><dt class="name">Ones in matrix</dt><dd class="val val-yes">${r.pairs.length}</dd></div>
      <div class="prop-row"><dt class="name">Zeros in matrix</dt><dd class="val">${(r.n * r.n) - r.pairs.length}</dd></div>
    </dl>`;
  $('matrix-result-panel').hidden = false;
}

function clearMatrixTool() {
  $('matrixSetA').value = '';
  $('matrixRelR').value = '';
  $('matrix-result-panel').hidden = true;
  hideMatrixErr();
  $('matrixSetA').focus();
}

function renderProps(r) {
  const { props, vacuous } = r;
  const vac = vacuous ? 'vacuous' : null;

  const rows = [
    ['Reflexive',      props.refl,    vac, props.reflViol[0]    || null],
    ['Irreflexive',    props.irrefl,  null, props.irreflViol[0]  || null],
    ['Symmetric',      props.sym,     vac, props.symViol[0]     || null],
    ['Antisymmetric',  props.antisym, vac, props.antisymViol[0] || null],
    ['Asymmetric',     props.asym,    null, null],
    ['Transitive',     props.trans,   vac, props.transViol[0]   || null],
    ['Total (connex)', props.total,   null, null],
  ];

  let html = `<div class="panel-subtitle">Relation properties</div><dl class="prop-list">`;
  for (const [name, val, vacNote, viol] of rows) {
    const isVac = vacNote && val;
    const cls = isVac ? 'val-vac' : (val ? 'val-yes' : 'val-no');
    const labelText = isVac ? 'vacuous' : (val ? 'yes' : 'no');

    html += `<div class="prop-row">
      <dt class="name">${name}${viol && !val ? `<span class="viol">${escapeHtml(viol)}</span>` : ''}</dt>
      <dd class="val ${cls}"><span class="visually-hidden">${name}: </span>${labelText}</dd>
    </div>`;
  }
  html += '</dl>';
  $('res-props').innerHTML = html;
}

function renderMapping(r) {
  const m = r.mapping;
  if (!m) { $('res-mapping').innerHTML = ''; return; }

  const rows = [
    ['Function (total)', m.isFunction],
    ['Partial function', m.isPartialFn && !m.isFunction],
    ['Injective (one-to-one)', m.injective],
    ['Surjective (onto)', m.surjective],
    ['Bijective', m.bijective],
  ];

  let html = `<div class="panel-subtitle">Mapping properties</div><dl class="prop-list">`;
  for (const [name, val] of rows) {
    html += `<div class="prop-row">
      <dt class="name">${name}</dt>
      <dd class="val ${val ? 'val-yes' : 'val-no'}"><span class="visually-hidden">${name}: </span>${val ? 'yes' : 'no'}</dd>
    </div>`;
  }
  html += '</dl>';
  if (m.summary) html += `<p class="mapping-summary">${escapeHtml(m.summary)}</p>`;
  $('res-mapping').innerHTML = html;
}

function renderOperations(r) {
  const op = r.operations;
  if (!op) { $('res-operations').innerHTML = ''; return; }

  const rows = [
    ['Inverse', 'R⁻¹', op.inverse],
    ['Complement', 'R̄ = (A×A)\\R', op.complement],
    ['Composition', 'R ∘ R', op.composeRR],
    ['Reflexive closure', 'r(R)', op.reflexiveClosure],
    ['Symmetric closure', 's(R)', op.symmetricClosure],
    ['Transitive closure', 't(R)', op.transitiveClosure],
  ];

  let html = `<div class="panel-subtitle">Relation operations</div>`;
  html += `<div class="table-wrap"><table class="ops-table"><thead><tr>
    <th scope="col">Operation</th><th scope="col">Notation</th>
    <th scope="col">|pairs|</th><th scope="col">Result</th></tr></thead><tbody>`;

  for (const [name, notation, pairs] of rows) {
    html += `<tr>
      <th scope="row">${name}</th>
      <td><code>${notation}</code></td>
      <td>${pairs.length}</td>
      <td class="ops-pairs">${escapeHtml(formatPairs(pairs))}</td>
    </tr>`;
  }

  html += `</tbody></table></div>`;
  html += `<p class="ops-note">R² ⊆ R: <strong class="${op.rrSubsetR ? 'val-yes' : 'val-no'}">${op.rrSubsetR ? 'yes' : 'no'}</strong> — boolean matrix test for transitivity.</p>`;
  $('res-operations').innerHTML = html;
}

function renderHasseSummary(r) {
  const h = r.hasse;
  if (!h) { $('res-hasse').innerHTML = ''; return; }

  let html = `<div class="panel-subtitle">Hasse analysis</div>`;
  if (!h.isPartialOrder) {
    html += `<p class="mapping-summary">No Hasse diagram: ${escapeHtml(h.explanation)}</p>`;
  } else {
    html += `<dl class="prop-list">
      <div class="prop-row"><dt class="name">Cover relations</dt><dd class="val val-yes">${h.covers.length}</dd></div>
      <div class="prop-row"><dt class="name">Levels</dt><dd class="val val-yes">${h.levels.length}</dd></div>
      <div class="prop-row"><dt class="name">Least element</dt><dd class="val ${h.least ? 'val-yes' : 'val-no'}">${escapeHtml(h.least || 'none')}</dd></div>
      <div class="prop-row"><dt class="name">Greatest element</dt><dd class="val ${h.greatest ? 'val-yes' : 'val-no'}">${escapeHtml(h.greatest || 'none')}</dd></div>
    </dl>
    <p class="mapping-summary">${escapeHtml(formatCovers(h.covers))}</p>`;
  }
  $('res-hasse').innerHTML = html;
}

async function analyseHasse() {
  if (!RelationEngine.isReady()) {
    showHasseErr('Analysis engine is not ready. Run build-wasm.bat and serve over HTTP.');
    return;
  }

  hideHasseErr();
  setHasseAnalysing(true);

  try {
    const result = await RelationEngine.analyse($('hasseSetA').value, $('hasseRelR').value);
    renderHasseResult(result);
    announce(result.hasse.isPartialOrder
      ? `Hasse analysis complete. ${result.hasse.covers.length} cover relation(s) found.`
      : 'Relation analysed, but it is not a partial order.');
  } catch (err) {
    showHasseErr(err.message || 'Hasse analysis failed.');
    $('hasse-result-panel').hidden = true;
  } finally {
    setHasseAnalysing(false);
  }
}

function formatCovers(covers) {
  if (!covers.length) return 'Covering pairs: none';
  return `Covering pairs: ${covers.map(([a, b]) => `${a} ≺ ${b}`).join(', ')}`;
}

function renderHasseResult(r) {
  renderHasseDiagram(r);
  renderHasseDetails(r);
  const panel = $('hasse-result-panel');
  panel.hidden = false;
  panel.scrollIntoView({ behavior: prefersReducedMotion() ? 'auto' : 'smooth', block: 'nearest' });
}

function renderHasseDiagram(r) {
  const h = r.hasse;
  const panel = $('hasse-diagram');
  if (!h.isPartialOrder) {
    panel.innerHTML = `<div class="hasse-empty">
      <strong>Not a partial order</strong>
      <span>${escapeHtml(h.explanation)}</span>
    </div>`;
    return;
  }

  const width = 620;
  const height = Math.max(260, h.levels.length * 96);
  const marginX = 70;
  const marginY = 42;
  const nodeMap = new Map();

  h.levels.forEach((level, levelIndex) => {
    const y = height - marginY - levelIndex * ((height - marginY * 2) / Math.max(h.levels.length - 1, 1));
    level.forEach((element, index) => {
      const step = (width - marginX * 2) / Math.max(level.length - 1, 1);
      const x = level.length === 1 ? width / 2 : marginX + step * index;
      nodeMap.set(element, { x, y });
    });
  });

  let svg = `<svg class="hasse-svg" viewBox="0 0 ${width} ${height}" role="img" aria-labelledby="hasse-svg-title hasse-svg-desc">
    <title id="hasse-svg-title">Hasse diagram</title>
    <desc id="hasse-svg-desc">${escapeHtml(formatCovers(h.covers))}</desc>`;

  for (const [a, b] of h.covers) {
    const from = nodeMap.get(a);
    const to = nodeMap.get(b);
    if (!from || !to) continue;
    svg += `<line class="hasse-edge" x1="${from.x}" y1="${from.y}" x2="${to.x}" y2="${to.y}" />`;
  }

  for (const level of h.levels) {
    for (const element of level) {
      const pt = nodeMap.get(element);
      svg += `<g class="hasse-node">
        <circle cx="${pt.x}" cy="${pt.y}" r="20"></circle>
        <text x="${pt.x}" y="${pt.y}" text-anchor="middle" dominant-baseline="middle">${escapeHtml(element)}</text>
      </g>`;
    }
  }

  svg += '</svg>';
  panel.innerHTML = svg;
}

function renderHasseDetails(r) {
  const h = r.hasse;
  const status = h.isPartialOrder
    ? '<span class="result-badge badge-po">Partial order</span><span class="result-badge badge-sp">Hasse diagram valid</span>'
    : '<span class="result-badge badge-none">No Hasse diagram</span>';

  const levelText = h.levels.map((level, index) => `Level ${index}: {${level.join(', ')}}`).join('<br>');
  const incomparable = h.incomparable.length
    ? h.incomparable.map(([a, b]) => `(${a}, ${b})`).join(', ')
    : 'none';

  $('hasse-details').innerHTML = `
    <div class="badge-group">${status}</div>
    <dl class="prop-list">
      <div class="prop-row"><dt class="name">Cover relations</dt><dd class="val ${h.isPartialOrder ? 'val-yes' : 'val-no'}">${h.covers.length}</dd></div>
      <div class="prop-row"><dt class="name">Minimal elements</dt><dd class="val">${escapeHtml(h.minimal.join(', ') || 'none')}</dd></div>
      <div class="prop-row"><dt class="name">Maximal elements</dt><dd class="val">${escapeHtml(h.maximal.join(', ') || 'none')}</dd></div>
      <div class="prop-row"><dt class="name">Least element</dt><dd class="val ${h.least ? 'val-yes' : 'val-no'}">${escapeHtml(h.least || 'none')}</dd></div>
      <div class="prop-row"><dt class="name">Greatest element</dt><dd class="val ${h.greatest ? 'val-yes' : 'val-no'}">${escapeHtml(h.greatest || 'none')}</dd></div>
    </dl>
    <div class="mapping-summary">${escapeHtml(formatCovers(h.covers))}</div>
    <div class="mapping-summary">${levelText || 'No levels available.'}</div>
    <div class="mapping-summary">Incomparable pairs: ${escapeHtml(incomparable)}</div>
    <div class="mapping-summary">${escapeHtml(h.explanation)}</div>`;
}

function renderBadges(r) {
  let html = `<div class="panel-subtitle">Classification</div><div class="badge-group" role="list">`;
  html += r.classifications
    .map(c => `<span class="result-badge ${c.cls}" role="listitem">${escapeHtml(c.label)}</span>`)
    .join('');
  html += '</div>';
  $('res-badges').innerHTML = html;
}

function renderInferences(r) {
  let html = '';
  if (r.inferences.length === 0) {
    html = `<p class="inference-block inference-ok">No additional inferences — R satisfies only the properties listed above.</p>`;
  } else {
    html = '<div class="inference-list">';
    for (const inf of r.inferences) {
      html += `<article class="inference-block">
        <h4 class="inf-title">${escapeHtml(inf.title)}</h4>
        <p>${escapeHtml(inf.body)}</p>
      </article>`;
    }
    html += '</div>';
  }
  $('res-inferences').innerHTML = html;
}

function escapeHtml(str) {
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

function bindTabs() {
  document.querySelectorAll('.nav-btn').forEach((btn, index) => {
    btn.addEventListener('click', () => showSection(btn.dataset.section));
    btn.addEventListener('keydown', (e) => {
      let next = index;
      if (e.key === 'ArrowRight') next = (index + 1) % SECTIONS.length;
      else if (e.key === 'ArrowLeft') next = (index - 1 + SECTIONS.length) % SECTIONS.length;
      else if (e.key === 'Home') next = 0;
      else if (e.key === 'End') next = SECTIONS.length - 1;
      else return;
      e.preventDefault();
      const target = document.querySelectorAll('.nav-btn')[next];
      target.focus();
      showSection(target.dataset.section);
    });
  });
}

function bindAccordions() {
  document.querySelectorAll('[data-accordion]').forEach(header => {
    const id = header.dataset.accordion;
    header.addEventListener('click', () => toggleAcc(id));
    header.addEventListener('keydown', (e) => {
      if (e.key === 'Enter' || e.key === ' ') {
        e.preventDefault();
        toggleAcc(id);
      }
    });
  });
}

function bindExamples() {
  document.querySelectorAll('[data-example]').forEach(btn => {
    btn.addEventListener('click', () => loadExample(btn.dataset.example));
  });
}

function bindHasseExamples() {
  document.querySelectorAll('[data-hasse-example]').forEach(btn => {
    btn.addEventListener('click', () => loadExample(btn.dataset.hasseExample));
  });
}

function bindMatrixExamples() {
  document.querySelectorAll('[data-matrix-example]').forEach(btn => {
    btn.addEventListener('click', () => {
      const ex = EXAMPLES[btn.dataset.matrixExample];
      if (!ex) return;
      $('matrixSetA').value = ex.A;
      $('matrixRelR').value = ex.R;
      analyseMatrix();
    });
  });
}

document.addEventListener('keydown', (e) => {
  if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
    e.preventDefault();
    analyse();
  }
});

document.addEventListener('DOMContentLoaded', async () => {
  bindTabs();
  bindAccordions();
  bindExamples();
  bindHasseExamples();
  bindMatrixExamples();

  $('analyser-form').addEventListener('submit', (e) => {
    e.preventDefault();
    analyse();
  });

  $('btn-clear').addEventListener('click', clearAll);
  $('matrix-form').addEventListener('submit', (e) => {
    e.preventDefault();
    analyseMatrix();
  });
  $('btn-matrix-clear').addEventListener('click', clearMatrixTool);
  $('hasse-form').addEventListener('submit', (e) => {
    e.preventDefault();
    analyseHasse();
  });
  $('btn-hasse-clear').addEventListener('click', clearAll);

  setEngineStatus('loading', 'Loading C++ analysis engine (WebAssembly)…');

  try {
    await RelationEngine.init();
    setEngineStatus('ready', '');
    if ($('setA').value.trim()) analyse();
  } catch (err) {
    setEngineStatus('error', `Engine failed to load: ${err.message}. Run .\\setup.ps1 or .\\build-wasm.bat, then serve via npm start.`);
    showErr(err.message);
  }
});
