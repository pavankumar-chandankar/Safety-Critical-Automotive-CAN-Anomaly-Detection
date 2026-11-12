#!/usr/bin/env python3
"""Run the C++ baseline on the downloaded GEM-CAN normal and attack recordings."""
from __future__ import annotations
import argparse
import json
import hashlib
from pathlib import Path
import subprocess


def run(binary: Path, data: Path, output: Path, target_fpr: float):
    normal = data / "GEM_Normal_Driving_Raw_Labeled.csv"
    attack = data / "GEM_Attack_Scenario_Raw_Labeled.csv"
    missing = [p for p in (normal, attack) if not p.is_file()]
    if missing:
        raise FileNotFoundError(f"Missing actual GEM-CAN data: {missing}. Run scripts/download_gem_can.py first.")
    provenance_file = data / "provenance.json"
    if not provenance_file.is_file():
        raise RuntimeError("Verified public dataset provenance is missing; run scripts/download_gem_can.py")
    provenance = json.loads(provenance_file.read_text(encoding="utf-8"))
    if provenance.get("original_archive_md5") != "03f7a86ed05da9ab282f2886bf9b3ca3":
        raise RuntimeError("Unexpected dataset archive version")
    for path in (normal, attack):
        source_digest = provenance.get("files", {}).get(path.name, {}).get("sha256")
        if hashlib.sha256(path.read_bytes()).hexdigest() != source_digest:
            raise RuntimeError(f"Dataset SHA-256 mismatch: {path.name}")
    output.mkdir(parents=True, exist_ok=True)
    cmd = [str(binary.resolve()), "--normal", str(normal), "--attack", str(attack),
           "--report", str(output / "metrics.json"),
           "--predictions", str(output / "predictions.csv"),
           "--target-fpr", str(target_fpr)]
    subprocess.run(cmd, check=True)
    results = json.loads((output / "metrics.json").read_text(encoding="utf-8"))
    if not 80_000 <= results["train_frames"] + results["calibration_frames"] + results["normal_holdout_frames"] <= 120_000:
        raise RuntimeError("Real-data normal capture row count outside published range")
    if not 42_000 <= results["attack_session_frames"] <= 46_000:
        raise RuntimeError("Real-data attack session row count outside published range")
    lines = ["# GEM-CAN evaluation", "", "Dataset: GEM-CAN v3 (Zenodo 19161139).",
             "", "The model was fitted only to the chronological normal-driving training segment.",
             "Threshold was calibrated on a later segment of that recording. The attack session was evaluated separately.",
             "", f"Known IDs: {results['known_ids']}", f"Threshold: {results['threshold']}",
             "", "| Partition | TP | FP | TN | FN | Precision | Recall | F1 | FPR |",
             "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |"]
    for name in ("normal_holdout", "attack_session"):
        v = results[name]
        lines.append(f"| {name} | {v['tp']} | {v['fp']} | {v['tn']} | {v['fn']} | {v['precision']:.3f} | {v['recall']:.3f} | {v['f1']:.3f} | {v['fpr']:.3f} |")
    lines.extend(["", "## Attack categories", "", "| Category | Frames | Recall |", "| --- | ---: | ---: |"])
    for name, v in results["by_attack_type"].items():
        lines.append(f"| {name.replace('|', ' ')} | {v['tp'] + v['fp'] + v['tn'] + v['fn']} | {v['recall']:.3f} |")
    lines += ["", "## Interpretation", "", "These numbers evaluate frame-level anomaly scores in this specific corpus,",
              "not generalization across vehicles, drivers, ECU software or attack campaigns.",
              "The dataset includes a large DoS majority and relatively few brake/steering tampering frames.",
              "Report per-category recall and benign false alerts before interpreting aggregate F1.", ""]
    (output / "report.md").write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote {output / 'metrics.json'}, {output / 'predictions.csv'}, {output / 'report.md'}")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--binary", type=Path, default=Path("build/cansec_gem"))
    ap.add_argument("--data", type=Path, default=Path("data/gem_can"))
    ap.add_argument("--output", type=Path, default=Path("artifacts/gem_can"))
    ap.add_argument("--target-fpr", type=float, default=0.01)
    args = ap.parse_args()
    run(args.binary, args.data, args.output, args.target_fpr)


if __name__ == "__main__":
    main()
