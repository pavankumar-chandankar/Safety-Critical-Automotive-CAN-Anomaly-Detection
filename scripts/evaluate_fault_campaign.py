#!/usr/bin/env python3
"""Synthetic oracle consistency checks for named seeded faults, NOT real-world ROC/accuracy."""
import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FAULT_EXPECTATIONS = {
    "none": (None, None),
    "counter-jump": ("COUNTER_JUMP", None),
    "checksum": ("CHECKSUM_MISMATCH", None),
    "signal-range": ("SIGNAL_RANGE", None),
    "unknown-id": ("UNKNOWN_ID", None),
    "wrong-bus": ("WRONG_BUS", None),
    "timing-burst": ("PERIOD_TOO_SHORT", None),
    "fd-length": ("UNEXPECTED_FD", None),
    "malformed": (None, "PAYLOAD_HEX"),
}


def run(binary: Path, seed: int, cycles: int):
    results = []
    with tempfile.TemporaryDirectory() as temp:
        for scenario, (expected_finding, expected_parse) in FAULT_EXPECTATIONS.items():
            trace = Path(temp) / f"{scenario}.csv"
            subprocess.run([sys.executable, str(ROOT / "scripts/generate_fixtures.py"),
                            "--seed", str(seed), "--cycles", str(cycles),
                            "--fault", scenario, "--output", str(trace)], check=True)
            completed = subprocess.run([str(binary), "analyze", "--trace", str(trace),
                                        "--rules", str(ROOT / "config/rules.csv"),
                                        "--signals", str(ROOT / "config/signals.csv")],
                                       capture_output=True, text=True)
            if completed.returncode not in (0, 2):
                raise RuntimeError(f"{scenario}: CLI exited {completed.returncode}: {completed.stderr}")
            report = json.loads(completed.stdout)
            observed = set(report["by_code"])
            parse_observed = {x["code"] for x in report["parse_errors"]}
            passed = (expected_finding is None or expected_finding in observed) and (
                expected_parse is None or expected_parse in parse_observed)
            if scenario == "none":
                passed = passed and report["findings_total"] == 0 and report["parse_errors_total"] == 0
            results.append({"scenario": scenario, "expected_finding": expected_finding,
                            "expected_parse_error": expected_parse, "observed_codes": sorted(observed),
                            "observed_parse_errors": sorted(parse_observed), "oracle_pass": passed})
    return {"fixture_type": "synthetic", "seed": seed, "cycles": cycles,
            "scenarios": len(results), "oracle_passed": sum(x["oracle_pass"] for x in results),
            "results": results,
            "caution": "Only deterministic seeded-fixture validation. No real vehicle or adversarial attack detection claims."}


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--binary", default=str(ROOT / "build/cansec"))
    p.add_argument("--seed", type=int, default=42)
    p.add_argument("--cycles", type=int, default=40)
    p.add_argument("--output", default="")
    args = p.parse_args()
    result = run(Path(args.binary).resolve(), args.seed, args.cycles)
    serialized = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.output:
        output = Path(args.output)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(serialized, encoding="utf-8")
    print(serialized)
    return 0 if result["oracle_passed"] == result["scenarios"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
