# Math — Relation Analyser

Interactive tool for analysing **binary relations** on finite sets. All computation runs in a **C++ core** compiled to **WebAssembly** for the browser.

## Quick start

```powershell
# One-time setup (installs Emscripten + builds WASM)
# If scripts are blocked, use setup.bat instead:
.\setup.bat
# Or: powershell -ExecutionPolicy Bypass -File .\setup.ps1

# Run local server (required — WASM does not work from file://)
npm start
```

Open **http://localhost:3000** → tab **05 · Analyser**.

## Architecture

| Layer | Role |
|-------|------|
| `relation_engine.hpp` | Header-only C++ analysis core |
| `bridge.cpp` | Emscripten export → `wasm/relation_engine.wasm` |
| `js/engine.js` | Loads WASM module only (no JS analysis fallback) |
| `js/app.js` | UI / rendering only |

## Build commands

```powershell
.\build-wasm.ps1    # WASM + native CLI
.\build-wasm.bat    # Same (wrapper)
g++ -std=c++17 -O2 main.cpp -o math-analyser.exe   # CLI only
```

Native CLI:

```powershell
.\math-analyser.exe "1,2,3" "(1,1); (1,2); (2,3)"
```

## Input format (ISO-style ordered pairs)

| Field | Format | Example |
|-------|--------|---------|
| Set A | Comma-separated | `1, 2, 3` |
| Relation R | `(a,b); (c,d)` or newline-separated | `(1,1); (1,2); (2,3)` |

Max **12** elements. Duplicate set elements are rejected.

## Standards

- **HTML5** semantic landmarks, `lang="en-GB"`, skip link
- **WCAG 2.2** — focus visible, ARIA tabs/accordions, live regions, keyboard navigation
- **ISO 80000-2** mathematical notation (∈, ⊆, ∀, ∅, ×)
- **`prefers-color-scheme`**, **`prefers-reduced-motion`**, **`prefers-contrast`**

## Project layout

```
relation_engine.hpp   C++ core
bridge.cpp            WASM bridge
main.cpp              CLI entry
wasm/                 Compiled relation_engine.js + .wasm
js/engine.js          WASM loader
js/app.js             UI
index.html            Application
css/style.css         Styles
setup.ps1             Full install + build
```

## Requirements

- **Emscripten** (via `setup.ps1`) for WASM build
- **g++** (C++17) for native CLI
- **Node.js** for `npm start` (static server)

---

Math Relation Analyser · C++ WebAssembly Engine
