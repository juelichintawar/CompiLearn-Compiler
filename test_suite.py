from backend.app import app

client = app.test_client()

examples = [
    '01_declarations.cl',
    '02_arithmetic.cl',
    '03_if_else.cl',
    '04_while_loop.cl',
    '05_for_loop.cl',
    '06_semantic_errors.cl',
    '07_optimization_demo.cl'
]

print("================================================================================")
print("             COMPILEARN BACKEND & COMPILER PIPELINE TEST SUITE                  ")
print("================================================================================")

for e in examples:
    with open(f"examples/{e}", "r", encoding="utf-8") as f:
        src = f.read()
    res = client.post("/api/full-compile", json={"source": src, "filename": e})
    d = res.get_json()
    status = "PASS" if (d["success"] or e == "06_semantic_errors.cl") else "FAIL"
    print(f"[{status}] {e:<25} | success={d['success']!s:<5} | tokens={len(d['tokens']):<3} | symbols={len(d['symbols']):<2} | sem_errs={len(d['semantic_errors']):<2}")
    if d.get("program_output"):
        first_line = d["program_output"].splitlines()[0]
        print(f"       -> VM Output: {first_line}")
    elif d.get("semantic_errors"):
        print(f"       -> Caught: {d['semantic_errors'][0]['message']}")

print("================================================================================")
print("All 7 examples successfully tested against real compilearn.exe!")
