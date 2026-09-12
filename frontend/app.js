/**
 * CompiLearn Web IDE - Frontend Client Application
 */

(function () {
  'use strict';

  // --- State Management ---
  const state = {
    currentFileName: '01_declarations.cl',
    originalSource: '',
    isModified: false,
    examples: [],
    tokens: [],
    symbols: [],
    pipelineResults: null
  };

  // --- DOM Elements ---
  const el = {
    // Top Bar
    compilerStatusBadge: document.getElementById('compilerStatusBadge'),
    compilerStatusText: document.getElementById('compilerStatusText'),
    btnCompileAll: document.getElementById('btnCompileAll'),
    btnRunVM: document.getElementById('btnRunVM'),
    phaseSelect: document.getElementById('phaseSelect'),
    btnRunPhase: document.getElementById('btnRunPhase'),
    btnResetCode: document.getElementById('btnResetCode'),
    btnClearEditor: document.getElementById('btnClearEditor'),
    themeToggleBtn: document.getElementById('themeToggleBtn'),
    themeIcon: document.getElementById('themeIcon'),

    // Sidebar
    examplesList: document.getElementById('examplesList'),
    btnNewFile: document.getElementById('btnNewFile'),

    // Editor
    currentFileName: document.getElementById('currentFileName'),
    fileModifiedDot: document.getElementById('fileModifiedDot'),
    codeEditor: document.getElementById('codeEditor'),
    lineNumbers: document.getElementById('lineNumbers'),
    cursorPos: document.getElementById('cursorPos'),
    charCount: document.getElementById('charCount'),

    // Pipeline Ribbon
    pipelineSteps: document.querySelectorAll('.pipeline-step'),

    // Output Tabs
    outputTabsBar: document.getElementById('outputTabsBar'),
    tabPanes: document.querySelectorAll('.tab-pane'),

    // Tab Contents
    overviewGrid: document.getElementById('overviewGrid'),
    tokensTableBody: document.getElementById('tokensTableBody'),
    tokenFilterInput: document.getElementById('tokenFilterInput'),
    tokenCountBadge: document.getElementById('tokenCountBadge'),
    parseTreeOutput: document.getElementById('parseTreeOutput'),
    astOutput: document.getElementById('astOutput'),
    symbolsTableBody: document.getElementById('symbolsTableBody'),
    symbolCountBadge: document.getElementById('symbolCountBadge'),
    semanticStatusPill: document.getElementById('semanticStatusPill'),
    semanticReportContainer: document.getElementById('semanticReportContainer'),
    tacOutput: document.getElementById('tacOutput'),
    optTacOutput: document.getElementById('optTacOutput'),
    stackCodeOutput: document.getElementById('stackCodeOutput'),
    vmOutputConsole: document.getElementById('vmOutputConsole'),
    vmStatusBadge: document.getElementById('vmStatusBadge'),
    rawCliConsole: document.getElementById('rawCliConsole'),

    // Toast Container
    toastContainer: document.getElementById('toastContainer')
  };

  // --- Initializer ---
  function init() {
    initTheme();
    bindEvents();
    checkCompilerHealth();
    loadExamplesList();
  }

  // --- Theme Management ---
  function initTheme() {
    const savedTheme = localStorage.getItem('compilearn_theme') || 'dark';
    document.documentElement.setAttribute('data-theme', savedTheme);
    el.themeIcon.textContent = (savedTheme === 'dark') ? '☀️' : '🌙';
  }

  function toggleTheme() {
    const currentTheme = document.documentElement.getAttribute('data-theme');
    const newTheme = (currentTheme === 'dark') ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', newTheme);
    localStorage.setItem('compilearn_theme', newTheme);
    el.themeIcon.textContent = (newTheme === 'dark') ? '☀️' : '🌙';
  }

  // --- Toast Notifications ---
  function showToast(message, type = 'info', duration = 3500) {
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    el.toastContainer.appendChild(toast);
    setTimeout(() => {
      toast.style.opacity = '0';
      setTimeout(() => toast.remove(), 300);
    }, duration);
  }

  // --- Event Bindings ---
  function bindEvents() {
    // Theme Toggle
    el.themeToggleBtn.addEventListener('click', toggleTheme);

    // Code Editor Events
    el.codeEditor.addEventListener('input', onEditorInput);
    el.codeEditor.addEventListener('scroll', onEditorScroll);
    el.codeEditor.addEventListener('keyup', updateCursorPosition);
    el.codeEditor.addEventListener('click', updateCursorPosition);
    el.codeEditor.addEventListener('keydown', onEditorKeyDown);

    // Header Action Buttons
    el.btnCompileAll.addEventListener('click', () => runFullCompilation());
    el.btnRunVM.addEventListener('click', () => runVMExecution());
    el.btnRunPhase.addEventListener('click', () => runSelectedPhase());
    el.btnResetCode.addEventListener('click', resetEditorCode);
    el.btnClearEditor.addEventListener('click', clearEditorCode);
    el.btnNewFile.addEventListener('click', createNewFile);

    // Token Filter
    el.tokenFilterInput.addEventListener('input', onFilterTokens);

    // Tab Switching
    el.outputTabsBar.addEventListener('click', (e) => {
      const tabBtn = e.target.closest('.output-tab');
      if (tabBtn) {
        switchTab(tabBtn.getAttribute('data-tab'));
      }
    });

    // Pipeline Ribbon Step Clicking
    el.pipelineSteps.forEach(step => {
      step.addEventListener('click', () => {
        const targetTab = step.getAttribute('data-tab');
        if (targetTab) {
          switchTab(targetTab);
        }
      });
    });

    // Copy Buttons Setup
    setupCopyButton('copyTokensBtn', () => copyTableToClipboard(el.tokensTableBody));
    setupCopyButton('copyParseTreeBtn', () => el.parseTreeOutput.textContent);
    setupCopyButton('copyAstBtn', () => el.astOutput.textContent);
    setupCopyButton('copySymbolsBtn', () => copyTableToClipboard(el.symbolsTableBody));
    setupCopyButton('copyTacBtn', () => el.tacOutput.textContent);
    setupCopyButton('copyOptBtn', () => el.optTacOutput.textContent);
    setupCopyButton('copyStackBtn', () => el.stackCodeOutput.textContent);
    setupCopyButton('copyVmOutputBtn', () => el.vmOutputConsole.textContent);
    setupCopyButton('copyRawBtn', () => el.rawCliConsole.textContent);
  }

  function setupCopyButton(btnId, getContentFn) {
    const btn = document.getElementById(btnId);
    if (!btn) return;
    btn.addEventListener('click', async () => {
      try {
        const text = getContentFn();
        await navigator.clipboard.writeText(text);
        showToast('Copied to clipboard!', 'success', 2000);
      } catch (err) {
        showToast('Failed to copy to clipboard', 'error');
      }
    });
  }

  function copyTableToClipboard(tbody) {
    const rows = Array.from(tbody.querySelectorAll('tr'));
    return rows.map(r => Array.from(r.querySelectorAll('td, th')).map(c => c.textContent.trim()).join('\t')).join('\n');
  }

  // --- Editor Functionality ---
  function onEditorInput() {
    updateLineNumbers();
    updateCharCount();
    checkIfModified();
  }

  function onEditorScroll() {
    el.lineNumbers.scrollTop = el.codeEditor.scrollTop;
  }

  function onEditorKeyDown(e) {
    // Tab key inserts 4 spaces
    if (e.key === 'Tab') {
      e.preventDefault();
      const start = el.codeEditor.selectionStart;
      const end = el.codeEditor.selectionEnd;
      el.codeEditor.value = el.codeEditor.value.substring(0, start) + '    ' + el.codeEditor.value.substring(end);
      el.codeEditor.selectionStart = el.codeEditor.selectionEnd = start + 4;
      onEditorInput();
      return;
    }

    // Ctrl+Enter or F5 compiles all
    if ((e.ctrlKey && e.key === 'Enter') || e.key === 'F5') {
      e.preventDefault();
      runFullCompilation();
    }
  }

  function updateLineNumbers() {
    const lineCount = (el.codeEditor.value.match(/\n/g) || []).length + 1;
    let nums = '';
    for (let i = 1; i <= lineCount; i++) {
      nums += i + '\n';
    }
    el.lineNumbers.textContent = nums;
  }

  function updateCursorPosition() {
    const val = el.codeEditor.value;
    const sel = el.codeEditor.selectionStart;
    const lines = val.substring(0, sel).split('\n');
    const lineNum = lines.length;
    const colNum = lines[lines.length - 1].length + 1;
    el.cursorPos.textContent = `Line ${lineNum}, Col ${colNum}`;
  }

  function updateCharCount() {
    el.charCount.textContent = `${el.codeEditor.value.length} chars`;
  }

  function checkIfModified() {
    state.isModified = (el.codeEditor.value !== state.originalSource);
    el.fileModifiedDot.style.display = state.isModified ? 'inline' : 'none';
  }

  function resetEditorCode() {
    el.codeEditor.value = state.originalSource;
    onEditorInput();
    showToast('Code restored to original example.', 'info');
  }

  function clearEditorCode() {
    el.codeEditor.value = '';
    onEditorInput();
  }

  function createNewFile() {
    const name = prompt('Enter new file name:', 'my_program.cl');
    if (name) {
      const safeName = name.trim().endsWith('.cl') ? name.trim() : `${name.trim()}.cl`;
      state.currentFileName = safeName;
      state.originalSource = '// New CompiLearn Program\nint x = 10;\nprint(x);\n';
      el.currentFileName.textContent = safeName;
      el.codeEditor.value = state.originalSource;
      onEditorInput();
      resetPipelineRibbon();
      showToast(`Created ${safeName}`, 'success');
    }
  }

  // --- Compiler Health Check ---
  async function checkCompilerHealth() {
    try {
      const res = await fetch('/api/health');
      const data = await res.json();
      if (data.compiler_ready) {
        el.compilerStatusBadge.className = 'compiler-status-badge';
        el.compilerStatusText.textContent = 'Compiler Ready (MSYS2 UCRT64)';
      } else {
        el.compilerStatusBadge.className = 'compiler-status-badge badge-warning';
        el.compilerStatusText.textContent = 'compilearn.exe Missing - run make';
      }
    } catch (err) {
      el.compilerStatusBadge.className = 'compiler-status-badge badge-error';
      el.compilerStatusText.textContent = 'Backend Offline';
    }
  }

  // --- Examples Management ---
  async function loadExamplesList() {
    try {
      const res = await fetch('/api/examples');
      const data = await res.json();
      if (data.success && data.examples) {
        state.examples = data.examples;
        renderExamplesSidebar(data.examples);

        // Auto-load first example
        if (data.examples.length > 0) {
          loadExampleFile(data.examples[0].filename);
        }
      }
    } catch (err) {
      el.examplesList.innerHTML = '<div class="empty-state">Failed to load examples</div>';
    }
  }

  function renderExamplesSidebar(examples) {
    el.examplesList.innerHTML = '';
    examples.forEach(ex => {
      const item = document.createElement('div');
      item.className = 'example-item';
      if (ex.filename === state.currentFileName) item.classList.add('active');
      item.dataset.filename = ex.filename;

      item.innerHTML = `
        <div class="example-item-title">${ex.filename}</div>
        <div class="example-item-desc">${ex.description || ex.title}</div>
      `;

      item.addEventListener('click', () => loadExampleFile(ex.filename));
      el.examplesList.appendChild(item);
    });
  }

  async function loadExampleFile(filename) {
    try {
      const res = await fetch(`/api/examples/${encodeURIComponent(filename)}`);
      const data = await res.json();
      if (data.success) {
        state.currentFileName = data.filename;
        state.originalSource = data.source;
        el.currentFileName.textContent = data.filename;
        el.codeEditor.value = data.source;
        onEditorInput();

        // Highlight active sidebar item
        document.querySelectorAll('.example-item').forEach(item => {
          item.classList.toggle('active', item.dataset.filename === filename);
        });

        resetPipelineRibbon();
        showToast(`Loaded ${filename}`, 'info', 2000);
      }
    } catch (err) {
      showToast(`Error loading example '${filename}'`, 'error');
    }
  }

  // --- Tab Switching ---
  function switchTab(tabId) {
    document.querySelectorAll('.output-tab').forEach(tab => {
      tab.classList.toggle('active', tab.getAttribute('data-tab') === tabId);
    });
    el.tabPanes.forEach(pane => {
      pane.classList.toggle('active', pane.id === tabId);
    });
  }

  // --- Reset Pipeline Ribbon ---
  function resetPipelineRibbon() {
    const defaultStatuses = {
      'status-source': 'Loaded',
      'status-lexical': 'Idle',
      'status-syntax': 'Idle',
      'status-ast': 'Idle',
      'status-symbols': 'Idle',
      'status-semantic': 'Idle',
      'status-tac': 'Idle',
      'status-optimize': 'Idle',
      'status-stack': 'Idle',
      'status-execution': 'Idle'
    };

    el.pipelineSteps.forEach(step => {
      step.className = 'pipeline-step';
      const statusEl = step.querySelector('.step-status');
      if (statusEl && defaultStatuses[statusEl.id]) {
        statusEl.textContent = defaultStatuses[statusEl.id];
      }
    });

    document.getElementById('step-source').classList.add('active');
  }

  // --- Run Full Compilation Pipeline ---
  async function runFullCompilation() {
    const source = el.codeEditor.value;
    if (!source.trim()) {
      showToast('Editor is empty. Write or select code first.', 'warning');
      return;
    }

    setButtonLoading(el.btnCompileAll, true, 'Compiling...');
    showToast('Running full compiler pipeline...', 'info', 1500);

    try {
      const res = await fetch('/api/full-compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          source: source,
          filename: state.currentFileName
        })
      });

      const data = await res.json();
      state.pipelineResults = data;

      // Update Pipeline Ribbon Badges
      updatePipelineRibbon(data.pipeline);

      // Populate Tab Panes
      populateTabPanes(data);

      if (data.success) {
        showToast('Full Pipeline Compilation Successful!', 'success');
      } else {
        showToast('Compilation encountered errors. Check Semantics/Syntax.', 'error', 4500);
      }
    } catch (err) {
      showToast(`Compilation failed: ${err.message}`, 'error');
    } finally {
      setButtonLoading(el.btnCompileAll, false, '▶ Compile All (Pipeline)');
    }
  }

  // --- Run VM Execution Directly ---
  async function runVMExecution() {
    const source = el.codeEditor.value;
    if (!source.trim()) {
      showToast('Editor is empty.', 'warning');
      return;
    }

    setButtonLoading(el.btnRunVM, true, 'Running VM...');
    try {
      const res = await fetch('/api/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          source: source,
          filename: state.currentFileName
        })
      });
      const data = await res.json();
      el.vmOutputConsole.textContent = data.stdout || data.stderr || '(No output produced)';
      el.vmStatusBadge.textContent = data.success ? 'Halted (Success)' : 'Error';
      el.vmStatusBadge.className = data.success ? 'badge badge-success' : 'badge badge-danger';
      switchTab('tab-output');
      showToast('VM Execution finished.', 'info', 2000);
    } catch (err) {
      showToast(`VM execution failed: ${err.message}`, 'error');
    } finally {
      setButtonLoading(el.btnRunVM, false, '⚡ Run VM');
    }
  }

  // --- Run Individual Phase ---
  async function runSelectedPhase() {
    const phase = el.phaseSelect.value;
    if (phase === 'all') {
      runFullCompilation();
      return;
    }

    const source = el.codeEditor.value;
    if (!source.trim()) {
      showToast('Editor is empty.', 'warning');
      return;
    }

    setButtonLoading(el.btnRunPhase, true, 'Running...');

    try {
      const res = await fetch('/api/phase', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          source: source,
          filename: state.currentFileName,
          phase: phase
        })
      });

      const data = await res.json();
      const output = data.stdout || data.stderr || '(No output produced)';

      // Switch to relevant tab and show output
      const phaseTabMap = {
        'lex': 'tab-tokens',
        'tokens': 'tab-tokens',
        'parse': 'tab-parsetree',
        'tree': 'tab-parsetree',
        'ast': 'tab-ast',
        'symbols': 'tab-symbols',
        'semantic': 'tab-semantic',
        'tac': 'tab-tac',
        'optimize': 'tab-optimize',
        'stack': 'tab-stack',
        'run': 'tab-output'
      };

      const targetTab = phaseTabMap[phase] || 'tab-raw';
      switchTab(targetTab);

      // Populate raw or specific view
      if (phase === 'tokens') {
        el.rawCliConsole.textContent = output;
      } else if (phase === 'tree') {
        el.parseTreeOutput.textContent = output;
      } else if (phase === 'ast') {
        el.astOutput.textContent = output;
      } else if (phase === 'tac') {
        el.tacOutput.textContent = output;
      } else if (phase === 'optimize') {
        el.optTacOutput.textContent = output;
      } else if (phase === 'stack') {
        el.stackCodeOutput.textContent = output;
      } else if (phase === 'run') {
        el.vmOutputConsole.textContent = output;
      }

      el.rawCliConsole.textContent = output;
      showToast(`Phase '${phase}' completed.`, data.success ? 'success' : 'error');
    } catch (err) {
      showToast(`Phase execution failed: ${err.message}`, 'error');
    } finally {
      setButtonLoading(el.btnRunPhase, false, 'Run Phase');
    }
  }

  // --- Pipeline Ribbon Updates ---
  function updatePipelineRibbon(p) {
    if (!p) return;

    setStepStatus('step-source', 'Loaded', 'success');
    setStepStatus('step-lexical', p.lexical.summary, p.lexical.status);
    setStepStatus('step-syntax', p.syntax.summary, p.syntax.status);
    setStepStatus('step-ast', p.ast.summary, p.ast.status);
    setStepStatus('step-symbols', p.symbols.summary, p.symbols.status);
    setStepStatus('step-semantic', p.semantic.summary, p.semantic.status);
    setStepStatus('step-tac', p.tac.summary, p.tac.status);
    setStepStatus('step-optimize', p.optimization.summary, p.optimization.status);
    setStepStatus('step-stack', p.stack.summary, p.stack.status);
    setStepStatus('step-execution', p.execution.summary, p.execution.status);
  }

  function setStepStatus(stepId, summaryText, statusClass) {
    const step = document.getElementById(stepId);
    if (!step) return;
    step.className = `pipeline-step ${statusClass}`;
    const textEl = step.querySelector('.step-status');
    if (textEl) textEl.textContent = summaryText;
  }

  // --- Populate Tab Panes with Data ---
  function populateTabPanes(data) {
    const full = data.full_output || {};

    // 1. Overview Grid Cards
    renderOverviewGrid(data);

    // 2. Tokens Table
    renderTokensTable(data.tokens || []);

    // 3. Parse Tree
    el.parseTreeOutput.textContent = full.parse_tree || '(Not available)';

    // 4. AST
    el.astOutput.textContent = full.ast || '(Not available)';

    // 5. Symbols Table
    renderSymbolsTable(data.symbols || []);

    // 6. Semantic Analysis Report
    renderSemanticReport(data.semantic_errors || [], full.semantic || '');

    // 7. TAC
    el.tacOutput.textContent = full.tac || '(Not available)';

    // 8. Optimized TAC
    el.optTacOutput.textContent = full.optimized_tac || '(Not available)';

    // 9. Stack Code
    el.stackCodeOutput.textContent = full.stack_code || '(Not available)';

    // 10. Stack VM Output Console
    el.vmOutputConsole.textContent = data.program_output || '(No printed output recorded)';
    el.vmStatusBadge.textContent = (data.pipeline && data.pipeline.execution.status === 'success') ? 'Success' : 'Error';
    el.vmStatusBadge.className = (data.pipeline && data.pipeline.execution.status === 'success') ? 'badge badge-success' : 'badge badge-error';

    // 11. Raw CLI Output
    el.rawCliConsole.textContent = data.raw_all || '(No raw output)';
  }

  // --- Render Overview Grid ---
  function renderOverviewGrid(data) {
    const p = data.pipeline || {};
    const stages = [
      { key: 'lexical', title: 'Lexical Analysis', metric: `${(data.tokens || []).length} Tokens`, desc: 'Scanned via Flex lexer' },
      { key: 'syntax', title: 'Syntax Analysis', metric: p.syntax ? p.syntax.status.toUpperCase() : 'N/A', desc: 'Bison LALR(1) parser' },
      { key: 'symbols', title: 'Symbol Table', metric: `${(data.symbols || []).length} Variables`, desc: 'Scoped environment' },
      { key: 'semantic', title: 'Semantic Analysis', metric: (data.semantic_errors || []).length === 0 ? 'Passed' : `${data.semantic_errors.length} Errors`, desc: 'Type & Scope validation' },
      { key: 'optimization', title: 'Optimization', metric: p.optimization ? p.optimization.summary : 'Complete', desc: 'Constant folding & DCE' },
      { key: 'execution', title: 'Stack VM', metric: p.execution ? p.execution.status.toUpperCase() : 'Ready', desc: 'Bytecode Execution' }
    ];

    let cardsHtml = `
      <div class="overview-welcome">
        <h3>Compilation Complete: ${data.filename || state.currentFileName}</h3>
        <p>The compiler executed all phases using the native <strong>compilearn.exe</strong> binary. Click any card or top ribbon tab to inspect details.</p>
      </div>
    `;

    stages.forEach(s => {
      const status = p[s.key] ? p[s.key].status : 'idle';
      cardsHtml += `
        <div class="stage-card" onclick="document.querySelector('[data-tab=tab-${s.key === 'lexical' ? 'tokens' : s.key === 'syntax' ? 'parsetree' : s.key}]')?.click()">
          <div class="stage-card-header">
            <span class="stage-card-title">${s.title}</span>
            <span class="stage-card-badge ${status}">${status.toUpperCase()}</span>
          </div>
          <div class="stage-card-metric">${s.metric}</div>
          <div class="stage-card-desc">${s.desc}</div>
        </div>
      `;
    });

    el.overviewGrid.innerHTML = cardsHtml;
  }

  // --- Render Tokens Table ---
  function renderTokensTable(tokens) {
    state.tokens = tokens;
    el.tokenCountBadge.textContent = `${tokens.length} tokens`;

    if (tokens.length === 0) {
      el.tokensTableBody.innerHTML = '<tr><td colspan="4" class="empty-state">No tokens scanned.</td></tr>';
      return;
    }

    let rowsHtml = '';
    tokens.forEach(t => {
      rowsHtml += `
        <tr>
          <td>${t.line}</td>
          <td>${t.column}</td>
          <td><span class="token-type-badge">${escapeHtml(t.type)}</span></td>
          <td><strong>${escapeHtml(t.lexeme)}</strong></td>
        </tr>
      `;
    });
    el.tokensTableBody.innerHTML = rowsHtml;
  }

  function onFilterTokens() {
    const q = el.tokenFilterInput.value.toLowerCase();
    const filtered = state.tokens.filter(t =>
      t.type.toLowerCase().includes(q) || t.lexeme.toLowerCase().includes(q)
    );
    renderTokensTable(filtered);
  }

  // --- Render Symbols Table ---
  function renderSymbolsTable(symbols) {
    state.symbols = symbols;
    el.symbolCountBadge.textContent = `${symbols.length} symbols`;

    if (symbols.length === 0) {
      el.symbolsTableBody.innerHTML = '<tr><td colspan="5" class="empty-state">No symbols recorded in table.</td></tr>';
      return;
    }

    let rowsHtml = '';
    symbols.forEach(s => {
      rowsHtml += `
        <tr>
          <td><strong>${escapeHtml(s.name)}</strong></td>
          <td><span class="badge">${escapeHtml(s.type)}</span></td>
          <td>${escapeHtml(s.scope)}</td>
          <td><code>${escapeHtml(s.address)}</code></td>
          <td>${s.line}</td>
        </tr>
      `;
    });
    el.symbolsTableBody.innerHTML = rowsHtml;
  }

  // --- Render Semantic Report ---
  function renderSemanticReport(errors, fullText) {
    if (errors.length === 0 && !fullText.includes('[FAIL]')) {
      el.semanticStatusPill.textContent = 'Passed (0 Errors)';
      el.semanticStatusPill.className = 'status-pill pass';
      el.semanticReportContainer.innerHTML = `
        <div class="empty-state" style="color: var(--accent-success); font-weight: 600;">
          ✓ All semantic checks passed! No redeclarations, undeclared variables, or type mismatches.
        </div>
      `;
      return;
    }

    el.semanticStatusPill.textContent = `Failed (${errors.length} Errors)`;
    el.semanticStatusPill.className = 'status-pill fail';

    let errsHtml = '';
    errors.forEach((err, idx) => {
      errsHtml += `
        <div class="error-card">
          <span class="error-card-badge">Error #${idx + 1}</span>
          <div class="error-card-text">
            <strong>Line ${err.line}:</strong> ${escapeHtml(err.message)}
          </div>
        </div>
      `;
    });

    if (errors.length === 0 && fullText) {
      errsHtml = `<pre class="code-pre">${escapeHtml(fullText)}</pre>`;
    }

    el.semanticReportContainer.innerHTML = errsHtml;
  }

  // --- Helpers ---
  function setButtonLoading(btn, isLoading, text) {
    btn.disabled = isLoading;
    btn.innerHTML = text;
  }

  function escapeHtml(str) {
    if (!str) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;');
  }

  // --- Launch Application ---
  document.addEventListener('DOMContentLoaded', init);
})();
