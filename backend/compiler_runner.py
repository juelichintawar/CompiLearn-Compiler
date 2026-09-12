import os
import re
import sys
import tempfile
import subprocess
from pathlib import Path
from typing import Dict, Any, List, Optional

ANSI_REGEX = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')

def clean_ansi(text: str) -> str:
    """Strip ANSI terminal escape sequences."""
    return ANSI_REGEX.sub('', text)

class CompilerRunner:
    def __init__(self, base_dir: Optional[Path] = None):
        if base_dir is None:
            self.base_dir = Path(__file__).resolve().parent.parent
        else:
            self.base_dir = Path(base_dir).resolve()

        self.examples_dir = self.base_dir / "examples"
        self.executable_path = self._locate_compiler_binary()

        # Build custom environment with MSYS2 paths if on Windows
        self.env = os.environ.copy()
        if sys.platform == "win32":
            msys_paths = ["C:\\msys64\\ucrt64\\bin", "C:\\msys64\\usr\\bin"]
            current_path = self.env.get("PATH", "")
            for p in msys_paths:
                if os.path.isdir(p) and p not in current_path:
                    current_path = p + ";" + current_path
            self.env["PATH"] = current_path

    def _locate_compiler_binary(self) -> Path:
        exe_names = ["compilearn.exe", "compilearn"]
        for name in exe_names:
            candidate = self.base_dir / name
            if candidate.exists() and os.access(str(candidate), os.X_OK):
                return candidate

        # Check compiler subfolder if applicable
        for name in exe_names:
            candidate = self.base_dir / "compiler" / name
            if candidate.exists() and os.access(str(candidate), os.X_OK):
                return candidate

        # Default fallback
        return self.base_dir / ("compilearn.exe" if sys.platform == "win32" else "compilearn")

    def execute_raw(self, args: List[str], timeout: float = 6.0) -> Dict[str, Any]:
        """Execute compilearn with given CLI args."""
        if not self.executable_path.exists():
            return {
                "success": False,
                "stdout": "",
                "stderr": f"Compiler executable not found at '{self.executable_path}'. Please run 'make' first.",
                "exit_code": -1
            }

        cmd = [str(self.executable_path)] + args
        try:
            res = subprocess.run(
                cmd,
                cwd=str(self.base_dir),
                env=self.env,
                capture_output=True,
                text=True,
                timeout=timeout
            )
            return {
                "success": (res.returncode == 0),
                "stdout": clean_ansi(res.stdout),
                "stderr": clean_ansi(res.stderr),
                "exit_code": res.returncode
            }
        except subprocess.TimeoutExpired:
            return {
                "success": False,
                "stdout": "",
                "stderr": f"Execution timed out after {timeout} seconds (possible infinite loop in program).",
                "exit_code": -2
            }
        except Exception as e:
            return {
                "success": False,
                "stdout": "",
                "stderr": f"Failed to execute compiler: {str(e)}",
                "exit_code": -3
            }

    def run_phase_on_source(self, source_code: str, phase: str, filename_hint: str = "temp.cl") -> Dict[str, Any]:
        """Write source code to a temporary .cl file and execute the requested phase."""
        temp_dir = self.base_dir / "temp_build"
        temp_dir.mkdir(exist_ok=True)

        temp_file = temp_dir / filename_hint
        try:
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write(source_code)

            result = self.execute_raw(["--file", str(temp_file), "--phase", phase])
            result["phase"] = phase
            result["filename"] = filename_hint
            return result
        finally:
            if temp_file.exists():
                try:
                    temp_file.unlink()
                except OSError:
                    pass

    def run_phase_on_file(self, filename: str, phase: str) -> Dict[str, Any]:
        """Execute phase on an existing example file."""
        safe_name = Path(filename).name
        file_path = self.examples_dir / safe_name
        if not file_path.exists():
            return {
                "success": False,
                "stdout": "",
                "stderr": f"Source file '{safe_name}' does not exist in examples.",
                "exit_code": 1,
                "phase": phase
            }

        result = self.execute_raw(["--file", str(file_path), "--phase", phase])
        result["phase"] = phase
        result["filename"] = safe_name
        return result

    def get_example_list(self) -> List[Dict[str, str]]:
        """Return list of available example programs."""
        examples = []
        if not self.examples_dir.exists():
            return examples

        for file in sorted(self.examples_dir.glob("*.cl")):
            # Extract first comment line as description
            desc = ""
            try:
                with open(file, "r", encoding="utf-8") as f:
                    for line in f:
                        line = line.strip()
                        if line.startswith("//"):
                            desc = line.lstrip("/ ").strip()
                            break
            except Exception:
                pass

            examples.append({
                "filename": file.name,
                "title": file.stem.replace("_", " ").title(),
                "description": desc or "CompiLearn example program"
            })
        return examples

    def get_example_code(self, filename: str) -> Optional[str]:
        """Read source code of an example file."""
        safe_name = Path(filename).name
        file_path = self.examples_dir / safe_name
        if file_path.exists() and file_path.is_file():
            with open(file_path, "r", encoding="utf-8") as f:
                return f.read()
        return None

    def run_full_pipeline(self, source_code: str, filename_hint: str = "program.cl") -> Dict[str, Any]:
        """
        Runs full pipeline against source code, extracting structured outputs for:
        - Lexical Tokens
        - Parse Tree
        - AST
        - Symbol Table
        - Semantic Analysis
        - TAC
        - Optimization
        - Stack Code
        - Stack VM Execution Output
        """
        temp_dir = self.base_dir / "temp_build"
        temp_dir.mkdir(exist_ok=True)
        temp_file = temp_dir / filename_hint

        try:
            with open(temp_file, "w", encoding="utf-8") as f:
                f.write(source_code)

            # 1. Run all phases in individual steps to capture dedicated outputs cleanly
            res_tokens = self.execute_raw(["--file", str(temp_file), "--phase", "tokens"])
            res_pt = self.execute_raw(["--file", str(temp_file), "--phase", "tree"])
            res_ast = self.execute_raw(["--file", str(temp_file), "--phase", "ast"])
            res_sym = self.execute_raw(["--file", str(temp_file), "--phase", "symbols"])
            res_sem = self.execute_raw(["--file", str(temp_file), "--phase", "semantic"])
            res_tac = self.execute_raw(["--file", str(temp_file), "--phase", "tac"])
            res_opt = self.execute_raw(["--file", str(temp_file), "--phase", "optimize"])
            res_stack = self.execute_raw(["--file", str(temp_file), "--phase", "stack"])
            res_all = self.execute_raw(["--file", str(temp_file), "--phase", "all"])

            # 2. Extract structured data
            tokens_list = self._parse_tokens_table(res_tokens.get("stdout", ""))
            symbol_list = self._parse_symbol_table(res_sym.get("stdout", ""))
            sem_errors = self._parse_semantic_errors(res_sem.get("stdout", ""))
            vm_output = self._extract_program_output(res_all.get("stdout", ""))

            # 3. Determine status of each stage
            has_lex_err = "ERRORS DETECTED" in res_tokens.get("stdout", "") or res_tokens.get("exit_code") != 0
            has_parse_err = "PARSING FAILED" in res_pt.get("stdout", "") or res_pt.get("exit_code") != 0
            has_sem_err = len(sem_errors) > 0 or "[FAIL]" in res_sem.get("stdout", "")

            pipeline_status = {
                "lexical": {
                    "name": "Lexical Analysis",
                    "status": "error" if has_lex_err else "success",
                    "summary": f"{len(tokens_list)} Tokens" if not has_lex_err else "Lexical Errors",
                    "output": res_tokens.get("stdout", "")
                },
                "syntax": {
                    "name": "Syntax Analysis",
                    "status": "error" if has_parse_err else "success",
                    "summary": "Parsing Failed" if has_parse_err else "Grammar Validated",
                    "output": res_pt.get("stdout", "")
                },
                "ast": {
                    "name": "Abstract Syntax Tree",
                    "status": "error" if has_parse_err else "success",
                    "summary": "AST Built" if not has_parse_err else "Failed",
                    "output": res_ast.get("stdout", "")
                },
                "symbols": {
                    "name": "Symbol Table",
                    "status": "error" if (has_parse_err or len(symbol_list) == 0 and not has_sem_err) else "success",
                    "summary": f"{len(symbol_list)} Scoped Symbols" if not has_parse_err else "Unavailable",
                    "output": res_sym.get("stdout", "")
                },
                "semantic": {
                    "name": "Semantic Analysis",
                    "status": "error" if has_sem_err else "success",
                    "summary": f"{len(sem_errors)} Error(s)" if has_sem_err else "Type Checking Passed",
                    "output": res_sem.get("stdout", "")
                },
                "tac": {
                    "name": "Three Address Code",
                    "status": "error" if has_parse_err else "success",
                    "summary": "IR Quadruples Generated",
                    "output": res_tac.get("stdout", "")
                },
                "optimization": {
                    "name": "Code Optimization",
                    "status": "error" if has_parse_err else "success",
                    "summary": self._extract_opt_summary(res_opt.get("stdout", "")),
                    "output": res_opt.get("stdout", "")
                },
                "stack": {
                    "name": "Stack Code Generation",
                    "status": "error" if has_parse_err else "success",
                    "summary": "Bytecode Generated",
                    "output": res_stack.get("stdout", "")
                },
                "execution": {
                    "name": "Stack VM Execution",
                    "status": "error" if (has_parse_err or has_sem_err or res_all.get("exit_code") != 0) else "success",
                    "summary": "Execution Complete" if not (has_parse_err or has_sem_err) else "Execution Aborted",
                    "output": vm_output
                }
            }

            overall_success = not (has_lex_err or has_parse_err or has_sem_err)

            return {
                "success": overall_success,
                "filename": filename_hint,
                "pipeline": pipeline_status,
                "tokens": tokens_list,
                "symbols": symbol_list,
                "semantic_errors": sem_errors,
                "program_output": vm_output,
                "raw_all": res_all.get("stdout", ""),
                "full_output": {
                    "tokens": res_tokens.get("stdout", ""),
                    "parse_tree": res_pt.get("stdout", ""),
                    "ast": res_ast.get("stdout", ""),
                    "symbols": res_sym.get("stdout", ""),
                    "semantic": res_sem.get("stdout", ""),
                    "tac": res_tac.get("stdout", ""),
                    "optimized_tac": res_opt.get("stdout", ""),
                    "stack_code": res_stack.get("stdout", ""),
                    "vm_execution": vm_output or res_all.get("stdout", "")
                }
            }
        finally:
            if temp_file.exists():
                try:
                    temp_file.unlink()
                except OSError:
                    pass

    def _parse_tokens_table(self, stdout: str) -> List[Dict[str, Any]]:
        """Parse tabular token output into list of dicts."""
        tokens = []
        for line in stdout.splitlines():
            line = line.strip()
            if line.startswith("|") and not ("Line" in line or "+---" in line):
                parts = [p.strip() for p in line.split("|")[1:-1]]
                if len(parts) >= 4:
                    try:
                        tokens.append({
                            "line": int(parts[0]),
                            "column": int(parts[1]),
                            "type": parts[2],
                            "lexeme": parts[3]
                        })
                    except ValueError:
                        continue
        return tokens

    def _parse_symbol_table(self, stdout: str) -> List[Dict[str, Any]]:
        """Parse tabular symbol table output into list of dicts."""
        symbols = []
        for line in stdout.splitlines():
            line = line.strip()
            if line.startswith("|") and not ("Variable Name" in line or "+---" in line or "No symbols defined" in line):
                parts = [p.strip() for p in line.split("|")[1:-1]]
                if len(parts) >= 5:
                    try:
                        symbols.append({
                            "name": parts[0],
                            "type": parts[1],
                            "scope": parts[2],
                            "address": parts[3],
                            "line": int(parts[4])
                        })
                    except ValueError:
                        continue
        return symbols

    def _parse_semantic_errors(self, stdout: str) -> List[Dict[str, Any]]:
        """Extract semantic errors from semantic analysis output."""
        errors = []
        pattern = re.compile(r'\[\d+\]\s+Line\s+(\d+)\s*:\s*(.+)')
        for line in stdout.splitlines():
            m = pattern.search(line.strip())
            if m:
                errors.append({
                    "line": int(m.group(1)),
                    "message": m.group(2).strip()
                })
        return errors

    def _extract_opt_summary(self, stdout: str) -> str:
        """Extract optimization reduction percentage."""
        m = re.search(r'Instruction Reduction\s*:\s*(\d+\s*\(.*?reduction\))', stdout)
        if m:
            return f"Reduced by {m.group(1)}"
        return "Completed"

    def _extract_program_output(self, stdout: str) -> str:
        """Extract lines that are output by the program during VM execution."""
        lines = []
        capture = False
        seen_header_divider = 0

        for line in stdout.splitlines():
            clean = line.strip()
            if "STACK MACHINE EXECUTION (VM)" in clean:
                capture = True
                seen_header_divider = 0
                continue
            if capture:
                if clean.startswith("===="):
                    seen_header_divider += 1
                    if seen_header_divider > 1:
                        # Reached the closing divider
                        break
                    continue
                if "[PROGRAM OUTPUT]" in clean:
                    lines.append(clean.replace("[PROGRAM OUTPUT]", "").strip())
                elif "[VM Complete]" in clean or "[VM Warning]" in clean:
                    lines.append(f"\n{clean}")

        return "\n".join(lines).strip()
