# CAN Sentinel — Automotive CAN/CAN FD Static Analysis & Defect Detection

**C++17 automotive software-quality and offline CAN-bus trace analysis research platform.** A reproducible reference implementation integrating protocol-aware parsing, DBC-inspired signal interpretation, deterministic fault injection, Clang Static Analyzer, sanitizer testing, seed-driven regression campaigns, and security-oriented verification.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) ![CMake](https://img.shields.io/badge/build-CMake%20%2B%20CTest-informational) ![Scope](https://img.shields.io/badge/data-synthetic%20validation-orange)

**Development status:** implemented and locally tested. This is a software engineering demonstration, **not** a production ECU security component, an ISO 26262 / ISO/SAE 21434 certified system, OEM DBC parser, or proof of real-world attack detection. Illustrative signal definitions, rule thresholds, and checksums are NOT authentic OEM behavior. No vehicle was connected and no commands are transmitted on a CAN bus.

## Engineering objectives

1. Validate malformed, inconsistent, truncated and semantically impossible CAN/CAN FD trace records before any detector logic sees them.
2. Decode signed/unsigned, Motorola/Intel bitfields without undefined shifts or unbounded payload access.
3. Detect *policy violations* that could represent communication defects: unknown IDs, message-rate anomalies, wrong buses, rolling-counter skips, simple checksum mismatches, abnormal payload lengths, out-of-range decoded signals, unexpected FD frames.
4. Make source-code quality checks and intentionally seeded defect reproduction **first-class** via compiler warnings, Clang Static Analyzer, clang-tidy, cppcheck, ASan/UBSan, CI artifacts and libFuzzer harnesses.
5. Produce stable machine-readable findings (JSON) with evidence line numbers, fault taxonomies and failure exit codes for continuous integration.
6. Document enough methodology to withstand technical review, while keeping *measured synthetic success* distinct from actual vehicle efficacy.

## System at a glance

```text
Linux candump log ----[strict C++ converter]----+
                                               |
Normalized CSV (timestamp / CAN / CAN FD) ------+---[validate contract]----[typed frame]
                                                                        |
                    +---------------------------+----------------------------+
                    |                           |                            |
            [message rule pack]          [signal dictionary]             [frame history]
                    |                           |                            |
                    +----------[protocol + semantic checks]-----------------+
                                               |
                                     [severity-coded findings]
                                               |
                                 [JSON / text + CI exit status]

C++ implementation --- Clang SA / clang-tidy / cppcheck --- source diagnostics
         |--- ASan / UBSan --- memory and undefined-behavior checks
         |--- libFuzzer --- CSV and candump fuzz harnesses
         +--- CTest / 100 native test cases / CLI integration / fault campaign
```

## Quick start (Linux/macOS with C++17 compiler, CMake, Python 3)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 2
ctest --test-dir build --output-on-failure

./build/cansec analyze \
  --trace fixtures/nominal.csv \
  --rules config/rules.csv \
  --signals config/signals.csv \
  --output report.json

python3 scripts/evaluate_fault_campaign.py --binary ./build/cansec --output artifacts/fault_campaign.json
```

Expected for the committed **synthetic** `nominal.csv`: 200 frames, 0 findings, 0 parse errors. The synthetic oracle validates nine named scenarios, including the unmodified nominal case. Detection performance on real vehicles is **unknown**.

### Introduce a seeded CAN trace defect

```bash
python3 scripts/generate_fixtures.py --fault counter-jump --seed 42 --output artifacts/bad_counter.csv
./build/cansec analyze --trace artifacts/bad_counter.csv --rules config/rules.csv \
  --signals config/signals.csv --fail-on-finding --format text
# Returns 3 when a semantic finding is reported; malformed input/configuration returns 2.
```

### Convert a supported subset of `candump` trace logs

```bash
./build/cansec convert --input fixtures/candump_sample.log --output artifacts/converted.csv
./build/cansec analyze --trace artifacts/converted.csv --rules config/rules.csv
```

The converter accepts `(seconds.microseconds) can0 123#AABB` and CAN FD `##0`/`##1` flags. It is intentionally **strict and partial**; CAN FD ESI flags, nonstandard log headers, virtual channels beyond `can15`, truncated payloads and extended candump syntaxes are not silently accepted. See [Data contract](docs/data_contract.md).

## Repository design

| Directory | Role |
|---|---|
| `include/cansec/`, `src/` | C++17 core library (frames, CSV/candump parsers, Intel/Motorola signals, policy validation, reporting, fault injection) |
| `app/main.cpp` | Pure C++ command-line program (`analyze`, `convert`) |
| `config/` | Example policy pack and DBC-inspired signal dictionary; **illustrative only** |
| `tests/` | Native C++ unit tests and executable-level Python regression tests |
| `fixtures/` | Seeded synthetic nominal/fault traces and candump sample |
| `defects/seeded/` | Isolated intentionally buggy C++ source for static-analyzer sensitivity checks |
| `defects/fuzz/` | libFuzzer entry points for both parsers |
| `benchmarks/` | Synthetic throughput microbenchmark with no external dataset |
| `scripts/` | Sanitizers, coverage, static-analysis, fixture generation, fault-oracle evaluation |
| `.github/workflows/` | GCC/Clang builds, Clang SA, quality tools, sanitizer tests, coverage and scheduled fuzz smoke |
| `docs/` | System requirements, architecture, threat model, signal semantics, verification evidence, engineering decisions |

## Static-analysis workflow

```bash
bash scripts/static_analysis.sh
bash scripts/sanitize.sh
bash scripts/coverage.sh     # optionally use gcovr for reports
python3 scripts/check_seeded_defects.py
```

The source-analysis job **hard-gates** Clang Static Analyzer findings and independently verifies analyzer sensitivity on isolated null-dereference and divide-by-zero examples. `clang-tidy` and `cppcheck` run when installed; see the CI workflow for gating policies. Intentionally buggy samples are **never compiled into application targets**.

Optional Clang libFuzzer (typically Linux with LLVM/Clang runtime):

```bash
cmake -S . -B build-fuzz -DCMAKE_CXX_COMPILER=clang++ \
  -DCANSEC_BUILD_FUZZERS=ON -DCANSEC_BUILD_TESTS=OFF
cmake --build build-fuzz
./build-fuzz/fuzz_trace -seed=42 -runs=2000
./build-fuzz/fuzz_candump -seed=42 -runs=2000
```

## Key design constraints and limitations

- **Offline only:** no live SocketCAN adapter, no frames transmitted, no J1939 transport reassembly, no ISO-TP, no diagnosis/service scanning, no OEM DBC file redistribution.
- **Signal catalog:** small, explicitly documented DBC-inspired CSV subset, **not** full Vector DBC grammar (e.g. multiplexing, value tables, cyclic attributes and containers unsupported).
- **Checksum:** illustrative SUM8 (sum of other bytes modulo 256), not a proven OEM checksum/CRC; per-ID semantics are configured and require manufacturer-specific verification.
- **Timing:** heuristic policy limits, not statistically trained detectors; high load, bus scheduling, multi-bus clocks and capture jitter can cause false positives.
- **Research validity:** synthetic fixture self-tests establish code behavior, not recall/precision on unseen real attacks. No HIL/CANoe/dSPACE, functional safety certification or physical vehicle tests are claimed here.
- **Legacy history:** earlier unrelated Prophet/Python snippets remain recoverable under Git history / `legacy/` for transparency; they are not part of this C++ architecture.

## Documentation

Read [System architecture](docs/architecture.md), [Requirements & traceability](docs/requirements_traceability.md), [Data contract](docs/data_contract.md), [Protocol & signal semantics](docs/signal_decoding.md), [Static-analysis methodology](docs/static_analysis.md), [Fault injection](docs/fault_injection.md), [Threat model](docs/threat_model.md), [Verification](docs/verification.md), [Research protocol](docs/research_protocol.md), [Limitations](docs/limitations.md), and [Engineering decisions](docs/decisions.md).

## License & attribution

The existing repository `LICENSE` is retained; check its terms before any downstream redistribution. This is original educational/research engineering work added to the user's repository. References to CAN/CAN FD specifications, source analyzers, Linux can-utils and software quality standards are informative, **not endorsements or conformance attestations**.
