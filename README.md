# Math Relation Analyser

> Interactive binary relation analyser — properties, closures, Hasse diagrams & function analysis. C++ engine compiled to WebAssembly.

🔗 **Live Demo:** [relationanalysers.netlify.app](https://relationanalysers.netlify.app)

---

## What is it?

A browser-based tool for analysing binary relations on finite sets, built for the **Discrete Mathematical Structures (DMS)** course at VTU (Subject Code: BCS405A, Scheme 2022).

Enter any finite set *A* and a binary relation *R ⊆ A × A* as ordered pairs — the tool performs a full mathematical analysis instantly, powered by a C++ engine compiled to **WebAssembly**. No server, no installation, runs entirely in your browser.

---

## Features

| Feature | Description |
|---|---|
| **Property Checker** | Verifies all 7 properties — reflexive, irreflexive, symmetric, antisymmetric, asymmetric, transitive, total/connex |
| **Relation Classifier** | Identifies equivalence relations, partial orders, total orders, strict partial orders, preorders, tolerance relations, and more |
| **Closure Computation** | Computes reflexive, symmetric, and transitive closures (Warshall's algorithm, O(n³)) |
| **Relation Operations** | Inverse (R⁻¹), complement (R̄), composition (R ∘ R) |
| **Boolean Matrix** | Generates the n×n adjacency matrix M_R |
| **Hasse Diagram** | Covering pairs, levels, minimal/maximal elements, least/greatest elements, incomparable pairs |
| **Function Analysis** | Tests if R is a function; determines injective, surjective, or bijective |
| **Theory Sections** | In-app reference covering all DMS relation topics with formal definitions and examples |

---

## Tech Stack

- **C++** — Core computation engine (property checks, Warshall's algorithm, Hasse construction)
- **WebAssembly** — C++ compiled via [Emscripten](https://emscripten.org/) for browser execution
- **HTML / CSS / JavaScript** — Front-end interface and WASM bindings
- **Netlify** — Deployment

---

## Project Structure

```
├── src/
│   ├── engine/              # C++ source files
│   │   ├── property_checker.cpp
│   │   ├── classifier.cpp
│   │   ├── closure.cpp
│   │   ├── hasse.cpp
│   │   └── function_analyser.cpp
│   ├── js/                  # JavaScript front-end & WASM bindings
│   └── css/                 # Styles
├── public/
│   ├── engine.wasm          # Compiled WebAssembly binary
│   └── engine.js            # Emscripten-generated JS glue
├── index.html
├── .gitignore
└── README.md
```

> **Note:** Adjust the structure above to match your actual repo layout.

---

## Building from Source

### Prerequisites

- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
- A modern browser with WebAssembly support

### Compile the C++ engine

```bash
# Activate Emscripten environment
source /path/to/emsdk/emsdk_env.sh

# Compile to WebAssembly
emcc src/engine/*.cpp -o public/engine.js \
  -s EXPORTED_FUNCTIONS='["_analyseRelation"]' \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='RelationEngine' \
  -O2
```

### Run locally

```bash
# Any static file server works — e.g. with Python
python3 -m http.server 8080
# Then open http://localhost:8080
```

> WebAssembly requires a server context — opening `index.html` directly via `file://` won't work.

---

## Usage

1. Enter your set *A* as comma-separated values: `1, 2, 3` or `a, b, c`
2. Enter relation *R* as ordered pairs: `(1,2); (2,3); (1,3)`
3. Click **Analyse** — results appear instantly
4. Navigate the six sections for theory, examples, and the Hasse diagram tool

### Example inputs

| Set A | Relation R | Type |
|---|---|---|
| `1,2,3` | `(1,1),(2,2),(3,3),(1,2),(2,1)` | Equivalence Relation |
| `1,2,3,4,6,12` | divisibility pairs | Partial Order |
| `1,2,3` | `(1,2),(2,3),(3,1)` | Bijection |
| `1,2,3` | `(empty)` | Empty / Strict Partial Order |

---

## Academic Context

- **Subject:** Discrete Mathematical Structures (DMS) — BCS405A
- **University:** Visveswaraya Technological University (VTU), Belagavi
- **Scheme:** 2022 | Semester IV
- **Department:** Computer Science & Engineering

---

## License

This project is open for academic use. If you build on it, a credit/star is appreciated!

---

## Topics

`discrete-mathematics` `webassembly` `cpp` `vtu` `binary-relations` `hasse-diagram` `warshall-algorithm` `graph-theory` `education` `emscripten`
