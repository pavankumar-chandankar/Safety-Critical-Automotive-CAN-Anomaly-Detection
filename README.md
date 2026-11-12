# CAN Sentinel

C++17 tools for inspecting CAN traffic, checking message integrity, and evaluating lightweight intrusion detection on recorded vehicle networks.

The project has two parts. The first is a protocol and software-quality toolkit: strict CAN/CAN FD parsing, configurable message rules, signal decoding, CMake-based tests, Clang static analysis, sanitizers and parser fuzzing. The second is an **evaluation pipeline for a public real-vehicle dataset**, using a normal-driving capture to establish a baseline and a separate attack capture to assess false alerts and attack detection.

[Real-data methodology](docs/gem_can_evaluation.md) · [C++ source](src/) · [Static analysis](docs/static_analysis.md) · [GitHub Actions](.github/workflows/ci.yml)

## Recorded traffic: GEM-CAN

The primary dataset is **GEM-CAN**, released by Mahsa Tavasoli and colleagues in 2026. CAN messages were captured on an autonomous GEM e6 electric vehicle using PCAN equipment under normal operation and controlled attack scenarios. The public release includes approximately **100,000 normal-driving frames** and **43,443 frames from an attack session**, with labeled DoS, brake-tampering and steering-tampering traffic.

Source: [GEM-CAN v3 on Zenodo](https://doi.org/10.5281/zenodo.19161139) and the [data paper](https://doi.org/10.1016/j.dib.2026.112805). The dataset is the work of its authors, not recordings collected by this repository's maintainer. **The published archive is CC BY-NC 3.0**; raw data are not repackaged in this source tree.

### Reproduce the C++ benchmark

Requirements: CMake 3.16+, a C++17 compiler, Python 3. No Python ML framework is required to fit the baseline.

```bash
git clone https://github.com/pavankumar-chandankar/Safety-Critical-Automotive-CAN-Anomaly-Detection.git
cd Safety-Critical-Automotive-CAN-Anomaly-Detection

# Download a version-pinned archive from the authors' Zenodo record and verify its checksum.
python3 scripts/download_gem_can.py

# Compile the CAN library, real-data evaluator and regression suite.
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure

# Fit on the independent normal-driving file and evaluate the attack session.
python3 scripts/run_gem_benchmark.py
cat artifacts/gem_can/report.md
```

If you downloaded the ZIP manually, use `python3 scripts/download_gem_can.py --archive /path/to/GEM_CAN_Dataset_R2.zip`. The import checks its published MD5 and records SHA-256 fingerprints of the extracted CSVs.

The evaluator writes `metrics.json`, `report.md`, and a per-frame `predictions.csv` to `artifacts/gem_can/`. The [GitHub Actions benchmark](.github/workflows/gem-can.yml) runs the same test against the original Zenodo archive and uploads measured metrics and dataset provenance.

### Recorded-data baseline results

The [first verified end-to-end run](https://github.com/pavankumar-chandankar/Safety-Critical-Automotive-CAN-Anomaly-Detection/actions/runs/43241363927) used the authors' version-pinned archive and produced the following frame-level measurements:

| Evaluation | Measurement |
| --- | ---: |
| Normal-driving frames used for fitting | 60,000 |
| Normal calibration / held-out normal frames | 20,000 / 20,000 |
| Normal holdout false-positive rate | 0.76% |
| Attack-session frames | 43,444 |
| Overall precision / recall / F1 (attack session) | 98.98% / 96.88% / 97.92% |
| DoS recall (41,080 frames) | 100% |
| Brake tampering recall (663 frames) | 0.15% |
| Steering tampering recall (662 frames) | 0% |
| False-positive rate among benign attack-session frames | 40.90% |

**Interpretation:** This is a deliberately transparent baseline, **not a strong general-purpose intrusion detector**. Almost all true positives come from DoS flooding. The legitimate-ID brake and steering tampering remain largely undetected, while normal traffic in the separate attack recording shifts substantially from the normal-driving baseline. The weak per-class results are just as important as the high aggregate F1. The [measured-results note](docs/gem_can_results.md) gives the confusion counts, provenance and limitations.

### Detection method

The benchmark does not use attack labels for fitting. It sorts each recording by timestamp and divides the normal recording chronologically into **60% profile fitting, 20% threshold calibration, and 20% holdout**. Only after that does it score the independent attack session.

The profile uses per-identifier timing, observed DLC values, payload-byte distributions and previously seen CAN IDs. Calibration selects an anomaly threshold against nominal traffic, then reports a confusion matrix and per-attack-category recall. This is a transparent C++ heuristic baseline, not a model trained to recognize every vehicle attack. The evaluation also reports false positives on held-out normal driving and benign frames inside the attack capture.

**Reading the results:** DoS dominates the attack dataset by frame count. An apparently strong overall score could still hide poor brake/steering detection. Moreover, the baseline and attack recordings have different operating conditions. Both issues are discussed in the [evaluation protocol](docs/gem_can_evaluation.md).

## CAN protocol and defect checks

`cansec` is a separate C++17 command-line tool for offline message and source-quality checks:

```bash
./build/cansec analyze --trace fixtures/nominal.csv --rules config/rules.csv \
  --signals config/signals.csv --output artifacts/checks.json

./build/cansec convert --input fixtures/candump_sample.log \
  --output artifacts/converted.csv
```

The rule engine supports Classical CAN and CAN FD, standard and extended identifiers, basic Intel/Motorola signal fields, configurable message periods, counters, payload checks and error reporting. Rule packs in `config/` are **illustrative**, not confidential or verified OEM definitions; the GEM-CAN benchmark deliberately learns its own profile instead of applying those rules as vehicle specifications.

The small committed traces in `fixtures/` are deterministic regression inputs for the parser and rules, **not the basis for the reported vehicle benchmark**. Real-recording evaluation uses the dataset above.

## Static analysis and source quality

The software-quality workflow is implemented around Clang and CMake:

```bash
python3 scripts/check_production_sast.py
python3 scripts/check_seeded_defects.py
bash scripts/sanitize.sh
```

It includes strict GCC/Clang diagnostics, Clang Static Analyzer, AddressSanitizer and UndefinedBehaviorSanitizer, plus libFuzzer entry points for parsers. Isolated intentionally defective C++ examples verify that the analysis setup can detect known classes of source-code defects; they are not compiled into the production library. The CI pipeline also builds a release artifact and runs CTest.

The source tree is organized as `include/cansec/` and `src/` (library), `app/` (tools), `tests/` (regression tests), `scripts/` (evaluation and analysis), `docs/` (design), and `.github/workflows/` (automation). A separate `legacy/` folder preserves early exploratory scripts without mixing them into the active build.

## Scope

CAN Sentinel is an **offline engineering and research tool**. It reads files and does not transmit CAN frames. The baseline is built for the published GEM-CAN recording format; it has not been validated across production vehicle fleets and has no ISO 26262 / ISO/SAE 21434 certification. It is not a replacement for an OEM DBC, an ECU safety case, or a HIL validation campaign.

The original dataset and data paper should be cited in any report using the benchmark. Details of the data license, preprocessing, split design, metrics and threats to validity are in [docs/gem_can_evaluation.md](docs/gem_can_evaluation.md).
