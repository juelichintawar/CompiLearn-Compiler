# CompiLearn: Educational Compiler Construction Platform & Web IDE

**CompiLearn** is a comprehensive educational compiler construction and interactive visualization platform built in C using **Flex**, **Bison**, and **GCC**, paired with a **Python Flask REST API** and a **Modern Web IDE**.

Designed for college courses, compiler instructors, and students, CompiLearn visualizes every phase of the compiler pipeline—from source tokenization to intermediate representation, multi-pass optimization, and bytecode execution on an integrated Stack Virtual Machine.

---

## 1. Project Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                     CompiLearn Web IDE (Frontend)                      │
│   - Professional IDE theme (Dark/Light) with live typography            │
│   - Left Sidebar: Example programs (.cl), New File, Reset               │
│   - Center: Code editor with line numbers and caret metadata            │
│   - Visual Compiler Pipeline Ribbon (Stages with ✓ / ✗ status)          │
│   - Output Panel: Tabs for Tokens, CST, AST, Symbols, Semantics,       │
│     Three Address Code (TAC), Optimized TAC, Bytecode & VM Console      │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ HTTP / JSON REST API
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     Flask Backend (backend/app.py)                      │
│   - REST Endpoints (/api/examples, /api/phase, /api/full-compile, etc.) │
│   - Path traversal security checks (strictly .cl files)                 │
│   - Safe temporary build sandbox                                        │
│   - compiler_runner.py (Invokes compilearn.exe, parses stdout/stderr)   │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ Process Execution
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│             Existing C Compiler (compilearn.exe / MSYS2 UCRT64)         │
│   Lexer.l ──► Parser.y ──► CST & AST ──► Symbol Table ──► Semantic      │
│   ──► Three Address Code ──► Optimizer ──► Stack Code ──► Stack VM      │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Directory Structure

```
C:\CompiLearn\
├── lexer.l             # Flex lexical scanner (tokenization & column tracking)
├── parser.y            # Bison LALR(1) parser (CST & AST generation, 0 conflicts)
├── ast.c / ast.h       # Concrete Parse Tree & AST nodes, ASCII tree printers
├── symboltable.c / .h  # Hierarchical scoped symbol table (offsets, levels)
├── semantic.c / .h     # Semantic analyzer (redeclarations, type checking)
├── tac.c / tac.h       # Three Address Code quadruples generator (t1, t2, L1...)
├── optimizer.c / .h    # Multi-pass IR optimizer (constant folding, DCE)
├── stackcode.c / .h    # Stack machine bytecode generator & execution Stack VM
├── menu.c / menu.h     # 12-option interactive CLI menu driver
├── main.c              # CLI entry point (interactive & batch mode flags)
├── Makefile            # GNU Makefile (flex, bison, gcc, run-web, test-api)
├── test_suite.py       # Automated integration test suite
│
├── examples/           # Test suite & sample programs (.cl)
│   ├── 01_declarations.cl
│   ├── 02_arithmetic.cl
│   ├── 03_if_else.cl
│   ├── 04_while_loop.cl
│   ├── 05_for_loop.cl
│   ├── 06_semantic_errors.cl
│   └── 07_optimization_demo.cl
│
├── backend/            # Python Flask Backend
│   ├── app.py          # REST API & static file server
│   ├── compiler_runner.py # compilearn.exe invocation & output parsing
│   └── requirements.txt # Python dependencies (Flask>=3.0.0)
│
├── frontend/           # Modern Web IDE Frontend
│   ├── index.html      # IDE layout & visual pipeline ribbon
│   ├── style.css       # Dark/light responsive theme & table styles
│   └── app.js          # Interactive controller & REST API client
│
└── README.md           # Documentation
```

---

## 3. Requirements & Prerequisites

### Windows (MSYS2 UCRT64)
1. Install **MSYS2** from [https://www.msys2.org](https://www.msys2.org).
2. Open the **MSYS2 UCRT64** terminal and install the development packages:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc flex bison make python python-pip
   ```
3. Install Python dependencies:
   ```bash
   python -m pip install -r backend/requirements.txt
   ```

### Linux (Ubuntu / Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential flex bison python3 python3-pip
python3 -m pip install -r backend/requirements.txt
```

---

## 4. How to Build the C Compiler

From **MSYS2 UCRT64** terminal or Windows PowerShell (with `C:\msys64\ucrt64\bin;C:\msys64\usr\bin` in PATH):

```bash
cd /c/CompiLearn
make clean
make
```

This compiles:
1. `bison -d parser.y` (produces `parser.tab.c` and `parser.tab.h`)
2. `flex lexer.l` (produces `lex.yy.c`)
3. `gcc -Wall -Wextra -std=c11 -O2` compiles all C modules
4. Links into `compilearn.exe` with **0 errors and 0 warnings**.

---

## 5. How to Run the CLI Compiler

### Interactive Menu Mode
```bash
./compilearn.exe
```
or preload a file:
```bash
./compilearn.exe examples/01_declarations.cl
```

Interactive Menu:
```
================================
          COMPILEARN            
================================
Current File: examples/01_declarations.cl
--------------------------------
1.  Lexical Analysis
2.  View Tokens
3.  Parse Source Code
4.  Display Parse Tree
5.  Display AST
6.  Display Symbol Table
7.  Semantic Analysis
8.  Generate TAC
9.  Optimize Code
10. Generate Stack Code
11. Compile Complete Program
12. Exit
--------------------------------
C.  Change Source File
================================
Enter choice (1-12 or C):
```

### Non-Interactive Command-Line Modes
```bash
# Named arguments format:
./compilearn.exe --file examples/01_declarations.cl --phase tokens
./compilearn.exe --file examples/01_declarations.cl --phase ast
./compilearn.exe --file examples/01_declarations.cl --phase tac
./compilearn.exe --file examples/01_declarations.cl --phase all

# Or direct flags format:
./compilearn.exe --tokens examples/01_declarations.cl
./compilearn.exe --pt     examples/01_declarations.cl
./compilearn.exe --ast    examples/01_declarations.cl
./compilearn.exe --sym    examples/01_declarations.cl
./compilearn.exe --sem    examples/06_semantic_errors.cl
./compilearn.exe --tac    examples/02_arithmetic.cl
./compilearn.exe --opt    examples/07_optimization_demo.cl
./compilearn.exe --run    examples/04_while_loop.cl
./compilearn.exe --all    examples/01_declarations.cl
```

---

## 6. How to Start the Web IDE

### Step 1: Start the Backend Server
```bash
# From CompiLearn directory:
python backend/app.py
```
Or using make:
```bash
make run-web
```

Output:
```
Starting CompiLearn Web Server on http://127.0.0.1:5000
 * Running on http://127.0.0.1:5000
```

### Step 2: Open the Frontend in Your Browser
Navigate to:
```
http://127.0.0.1:5000
```

---

## 7. How Frontend Communicates with `compilearn.exe`

The web application communicates exclusively with the genuine C/Flex/Bison binary:
1. **User Action**: The user writes or selects code in the web editor and clicks **Compile All** or an individual phase button.
2. **REST Request**: The frontend makes an asynchronous `POST /api/full-compile` (or `/api/phase`) request carrying the `.cl` source code.
3. **Safe Sandbox Execution**: `backend/compiler_runner.py` saves the code to a temporary `.cl` file in `temp_build/` and executes `compilearn.exe --file temp.cl --phase <phase>`.
4. **Output Parsing**: The runner captures stdout/stderr, strips ANSI codes, extracts structured token tables, symbol entries, semantic error line numbers, and VM program output.
5. **UI Rendering**: The frontend renders the visual pipeline ribbon (green for success, red for errors), populates the tabbed panels, and displays the real compiler execution output.

---

## 8. Backend REST API Reference

| Endpoint | Method | Payload | Description |
|---|---|---|---|
| `/api/health` | GET | None | Checks if `compilearn.exe` is compiled and available |
| `/api/examples` | GET | None | Returns list of example `.cl` programs |
| `/api/examples/<name>` | GET | None | Returns source code of requested example |
| `/api/compile` | POST | `{"source": "...", "filename": "..."}` | Runs full pipeline and returns summary |
| `/api/phase` | POST | `{"source": "...", "phase": "..."}` | Runs a specific compiler phase (`tokens`, `ast`, `tac`, etc.) |
| `/api/full-compile` | POST | `{"source": "...", "filename": "..."}` | Runs all phases and returns structured tab outputs |
| `/api/run` | POST | `{"source": "...", "filename": "..."}` | Compiles and executes code on Stack VM |

---

## 9. Examples Suite

| Example | Educational Objective | Key Output Verified |
|---|---|---|
| `01_declarations.cl` | Tests all basic data types (`int`, `float`, `string`, `bool`), assignments | 52 tokens, 5 symbols, prints all variable types |
| `02_arithmetic.cl` | Operator precedence, modulo, unary minus, parentheses | Generates exact TAC: `t1 = c*d`, `t2 = b+t1`, `result = t2` |
| `03_if_else.cl` | Nested conditional branches, relational & logical operators | Branch evaluation on Stack VM (`Distinction!`) |
| `04_while_loop.cl` | While loop iteration, backward jumps, factorial of 5 | VM executes 119 steps, prints `120` |
| `05_for_loop.cl` | For loop initialization, condition, update, sum 1..10 | VM executes 219 steps, prints `55` |
| `06_semantic_errors.cl` | Demonstrates line-numbered semantic diagnostics | Catches redeclaration, undeclared var, type mismatch, scope leaks |
| `07_optimization_demo.cl` | Constant folding, constant/copy propagation, dead code elimination | Reduces instructions by 42.9% |

---

## 10. Automated Testing

Run the full automated test suite verifying all 7 examples against the real `compilearn.exe`:
```bash
python test_suite.py
```
Or using make:
```bash
make test-api
```

---

## 11. Troubleshooting

- **`compilearn.exe not found`**:
  Run `make` inside `C:\CompiLearn` using MSYS2 UCRT64 to generate the binary.
- **`flex` or `bison` not found in PATH**:
  Ensure `C:\msys64\ucrt64\bin;C:\msys64\usr\bin` is added to your environment `PATH` variable.
- **Port 5000 in use**:
  Run `PORT=8080 python backend/app.py` to change the listening port.
