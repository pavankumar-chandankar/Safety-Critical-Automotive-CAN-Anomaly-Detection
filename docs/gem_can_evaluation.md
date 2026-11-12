# GEM-CAN: recorded-traffic benchmark protocol

## Dataset

The main benchmark uses **GEM-CAN**, published by Tavasoli *et al.*, DOI [10.5281/zenodo.19161139](https://doi.org/10.5281/zenodo.19161139), and documented in *Data in Brief* (2026), DOI [10.1016/j.dib.2026.112805](https://doi.org/10.1016/j.dib.2026.112805). The research group captured actual CAN frames from a GEM e6 autonomous electric vehicle, using PCAN-USB hardware, during normal operation and controlled attack experiments.

The downloadable archive contains two labeled CSV files: normal driving (~100,000 CAN frames) and an attack recording (43,443 frames). Attack classes are DoS, brake tampering, and steering tampering. The original attack capture includes 42,405 injected frames and 1,038 normal frames, according to the paper. These are the **dataset authors' published counts**, not measured outcomes of this repository.

**Rights:** Zenodo v3 lists Creative Commons Attribution Non Commercial 3.0. The archive is **not** part of this Git repository. Downloading for research requires compliance with the dataset license, source attribution and restrictions on commercial reuse.

## Import contract

`scripts/download_gem_can.py` downloads the exact Zenodo v3 archive (or imports a local archive) and verifies its published MD5 before extraction. The script copies only the two named labeled CSVs and records SHA-256 hashes in local provenance metadata. It refuses encrypted entries and excessive sizes. The C++ importer validates mandatory columns, numeric ranges, Classic CAN DLC 0–8, payload bytes in hex, and the binary label. An optional `Attack_Type` column is expected in the attack session.

Data with a missing label, malformed byte or unexpected row shape are **rejected with line numbers**. Records are sorted by recorded timestamp *within their own session*, never joined into a fictional continuous driving trace. The importer accepts `label` or `Label`, `arbitration_id` or `Arbitration_ID`, and quoted timestamp thousands separators.

## Experimental design

1. Chronological **first 60%** of the normal-driving session: learn observed IDs, DLCs, byte-value vocabularies and per-ID low-quantile interarrival times.
2. Chronological **next 20%**: calibrate an anomaly threshold targeting 1% frame-level false-alarm rate. No attack labels are used in calibration.
3. Chronological **final 20%**: report false-alarm rate on held-out normal traffic.
4. **Separate attack capture**: evaluate the frozen profile and threshold without fitting or calibration on this session. Report frame-level precision, recall, F1, confusion matrices and recall by annotated attack class.

This avoids direct attack-session leakage. It does **not** solve domain shift between moving normal driving and stationary attack capture. A strong DoS score could dominate overall metrics and obscure weak performance on tampering. The threshold is only approximately calibrated; its false-positive rate can differ on the held-out normal segment and mixed attack capture.

## Baseline

A non-ML temporal and payload anomaly baseline is implemented in C++17. It scores:

- identifiers absent from the normal profile;
- DLCs not observed for a known identifier;
- payload bytes not observed in a sufficiently frequent normal identifier;
- inter-arrival times much shorter than a low-quantile normal cadence.

Weights are simple interpretable heuristics, not tuned on attacks. Alert decisions use a threshold chosen from normal calibration only. Source line, timestamp, ID, label, score and an indicative reason are written per attack-frame row. The labels are read **only for scoring**, never for profile fitting.

The classic CAN decoder, signal rules, fuzz harnesses and Clang Static Analyzer operate separately from this statistical baseline; the project does not claim these tools can prove that an injected frame is malicious.

## Run

```bash
python3 scripts/download_gem_can.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python3 scripts/run_gem_benchmark.py
cat artifacts/gem_can/report.md
```

The optional GitHub Actions **GEM-CAN public-recording evaluation** workflow runs the same commands and uploads only metrics and provenance, not raw third-party recordings or per-frame predictions.

## Limitations

- This is a **single-vehicle**, single-study benchmark; a frame classifier trained on one capture is not a deployable automotive IDS.
- The normal and attack recordings were collected under different operating conditions. Results are affected by trace characteristics and attack imbalance.
- Signal decoding uses separately configured illustrative signal definitions, not a verified OEM DBC for this vehicle.
- The dataset does not provide a vehicle safety case, functional-safety certification, or direct CAN actuator access.
- The C++ parser's unit tests use the published schema and documented example rows; they are software-contract tests, not replacements for real-data evaluation.
