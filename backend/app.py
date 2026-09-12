import os
import sys
from pathlib import Path
from flask import Flask, jsonify, request, send_from_directory, abort

# Ensure base directory is in sys.path
BASE_DIR = Path(__file__).resolve().parent.parent
if str(BASE_DIR) not in sys.path:
    sys.path.insert(0, str(BASE_DIR))

from backend.compiler_runner import CompilerRunner

FRONTEND_DIR = BASE_DIR / "frontend"

app = Flask(__name__, static_folder=str(FRONTEND_DIR), static_url_path="")
runner = CompilerRunner(base_dir=BASE_DIR)

# Helper for safe filenames
def is_safe_filename(filename: str) -> bool:
    if not filename or ".." in filename or "/" in filename or "\\" in filename:
        return False
    return filename.endswith(".cl")

@app.route("/")
def serve_index():
    return send_from_directory(str(FRONTEND_DIR), "index.html")

@app.route("/<path:path>")
def serve_static(path):
    if (FRONTEND_DIR / path).exists():
        return send_from_directory(str(FRONTEND_DIR), path)
    return send_from_directory(str(FRONTEND_DIR), "index.html")

@app.route("/api/health", methods=["GET"])
def api_health():
    exe_exists = runner.executable_path.exists()
    return jsonify({
        "status": "healthy",
        "compiler_executable": str(runner.executable_path),
        "compiler_ready": exe_exists
    })

@app.route("/api/examples", methods=["GET"])
def api_examples():
    examples = runner.get_example_list()
    return jsonify({"success": True, "examples": examples})

@app.route("/api/examples/<filename>", methods=["GET"])
def api_get_example(filename: str):
    if not is_safe_filename(filename):
        return jsonify({"success": False, "error": "Invalid filename format."}), 400

    code = runner.get_example_code(filename)
    if code is None:
        return jsonify({"success": False, "error": f"Example '{filename}' not found."}), 404

    return jsonify({
        "success": True,
        "filename": filename,
        "source": code
    })

@app.route("/api/compile", methods=["POST"])
def api_compile():
    data = request.get_json(silent=True) or {}
    source = data.get("source", "")
    filename = data.get("filename", "program.cl")

    if not source.strip():
        return jsonify({"success": False, "error": "Source code cannot be empty."}), 400

    if not is_safe_filename(filename):
        filename = "program.cl"

    res = runner.run_full_pipeline(source, filename_hint=filename)
    return jsonify(res)

@app.route("/api/phase", methods=["POST"])
def api_phase():
    data = request.get_json(silent=True) or {}
    source = data.get("source", "")
    filename = data.get("filename", "program.cl")
    phase = data.get("phase", "all").lower().strip()

    valid_phases = ["lex", "tokens", "parse", "tree", "ast", "symbols", "semantic", "tac", "optimize", "stack", "run", "all"]
    if phase not in valid_phases:
        return jsonify({"success": False, "error": f"Invalid phase '{phase}'. Valid phases: {valid_phases}"}), 400

    if not is_safe_filename(filename):
        filename = "program.cl"

    # If source is provided, run on source
    if source.strip():
        res = runner.run_phase_on_source(source, phase=phase, filename_hint=filename)
    else:
        # Otherwise run on example file
        res = runner.run_phase_on_file(filename, phase=phase)

    return jsonify(res)

@app.route("/api/full-compile", methods=["POST"])
def api_full_compile():
    data = request.get_json(silent=True) or {}
    source = data.get("source", "")
    filename = data.get("filename", "program.cl")

    if not source.strip():
        return jsonify({"success": False, "error": "Source code is empty."}), 400

    if not is_safe_filename(filename):
        filename = "program.cl"

    res = runner.run_full_pipeline(source, filename_hint=filename)
    return jsonify(res)

@app.route("/api/run", methods=["POST"])
def api_run():
    data = request.get_json(silent=True) or {}
    source = data.get("source", "")
    filename = data.get("filename", "program.cl")

    if not source.strip():
        return jsonify({"success": False, "error": "Source code is empty."}), 400

    if not is_safe_filename(filename):
        filename = "program.cl"

    res = runner.run_phase_on_source(source, phase="run", filename_hint=filename)
    return jsonify(res)

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    print(f"Starting CompiLearn Web Server on http://127.0.0.1:{port}")
    app.run(host="0.0.0.0", port=port, debug=False)
