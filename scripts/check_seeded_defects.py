#!/usr/bin/env python3
"""Clang Static Analyzer sensitivity self-test using quarantined buggy examples."""
import argparse
import json
import shutil
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXPECT = {
    "null_deref.cpp": ["null pointer", "dereference of null", "null dereference"],
    "divide_by_zero.cpp": ["division by zero", "divide by zero"],
}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--clang", default=shutil.which("clang++") or "clang++")
    parser.add_argument("--output", default="")
    args = parser.parse_args()
    report = []
    for name, indicators in EXPECT.items():
        path = ROOT / "defects/seeded" / name
        command = [args.clang, "--analyze", "-std=c++17", "-Xanalyzer", "-analyzer-output=text", str(path)]
        completed = subprocess.run(command, text=True, capture_output=True)
        output = (completed.stdout + completed.stderr).lower()
        detected = any(token in output for token in indicators)
        report.append({"mutant": name, "diagnostic_found": detected, "return_code": completed.returncode,
                       "diagnostic_excerpt": (completed.stderr or completed.stdout)[:1200]})
    passed = sum(item["diagnostic_found"] for item in report)
    result = {"checks": len(report), "caught": passed, "analyzer": args.clang, "results": report}
    if args.output:
        path = Path(args.output)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"checks": len(report), "caught": passed,
                      "missed": [x["mutant"] for x in report if not x["diagnostic_found"]]}, indent=2))
    return 0 if passed == len(report) else 1


if __name__ == "__main__":
    raise SystemExit(main())
