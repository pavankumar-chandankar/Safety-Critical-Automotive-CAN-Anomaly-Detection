#!/usr/bin/env python3
"""Treat Clang Static Analyzer production warnings as a deterministic CI failure."""
import argparse
import json
import re
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--clang", default=shutil.which("clang++") or "clang++")
    parser.add_argument("--output", default="")
    args = parser.parse_args()
    files = sorted((ROOT / "src").glob("*.cpp")) + sorted((ROOT / "app").glob("*.cpp"))
    failures = []
    for file in files:
        proc = subprocess.run([args.clang, "--analyze", "-std=c++17", "-Wall", "-Wextra", "-Wpedantic",
                               "-I", str(ROOT / "include"), str(file)],
                              capture_output=True, text=True)
        output = proc.stdout + proc.stderr
        warning_lines = [line for line in output.splitlines()
                         if re.search(r"\b(?:warning|error):", line, re.IGNORECASE)]
        if proc.returncode != 0 or warning_lines:
            failures.append({"file": str(file.relative_to(ROOT)), "status": proc.returncode,
                             "warnings": warning_lines, "output_excerpt": output[:3000]})
    result = {"tool": "Clang Static Analyzer", "files_checked": len(files),
              "passed": len(failures) == 0, "failures": failures}
    if args.output:
        path = Path(args.output)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
