# Requirements-to-evidence traceability

| ID | Requirement | Main implementation | Automated evidence |
|---|---|---|---|
| FR-01 | Strict nine-field CSV decoding | `src/parser.cpp` | `test_parser.cpp`, `test_cli.py` |
| FR-02 | Classic CAN DLC and CAN FD 0..15 conversion | `src/frame.cpp` | `test_frame.cpp`, `test_parser.cpp` |
| FR-03 | 11-bit/29-bit IDs and distinct format identities | `src/parser.cpp`, `src/candump.cpp` | `test_parser.cpp`, `test_candump.cpp` |
| FR-04 | Intel/Motorola signal decoding and limits | `src/signals.cpp` | `test_signals.cpp`, `test_engine.cpp` |
| FR-05 | Rule-pack schema, counter and checksum semantics | `src/rules.cpp` | `test_rules.cpp` |
| FR-06 | Timing, IDs, payload, bus, FD findings | `src/engine.cpp` | `test_engine.cpp`, fault campaign |
| FR-07 | Machine-readable diagnostics, line references, exit codes | `src/report.cpp`, `app/main.cpp` | `test_report.cpp`, `test_cli.py` |
| FR-08 | Replay/fault injection and seeded oracle | `src/replay.cpp`, `scripts/generate_fixtures.py` | `test_replay.cpp`, `evaluate_fault_campaign.py` |
| FR-09 | Linux candump strict subset adapter | `src/candump.cpp` | `test_candump.cpp`, `test_cli.py` |
| FR-10 | Read version-pinned public GEM-CAN captures | `scripts/download_gem_can.py`, `src/gem.cpp` | Published ZIP checksum and `test_gem.cpp` |
| FR-11 | Train on independent nominal session, score real attack recording | `src/gem.cpp`, `app/gem_benchmark.cpp` | `test_gem.cpp`, `gem-can.yml` |
| FR-12 | Traceable per-frame decisions and category-specific confusion counts | `src/gem.cpp`, `scripts/run_gem_benchmark.py` | JSON metrics and prediction export |
| NFR-01 | C++17 warning-as-error build with two compilers | `CMakeLists.txt` | GitHub matrix CI |
| NFR-02 | Source-level static defect checks | `scripts/static_analysis.sh`, `defects/seeded` | `check_seeded_defects.py`, GitHub static-analysis job |
| NFR-03 | Runtime UB and memory error checks | `scripts/sanitize.sh` | ASan/UBSan CI |
| NFR-04 | Parser fuzz robustness | `defects/fuzz` | bounded scheduled libFuzzer run |
| NFR-05 | Test and coverage artifacts | `tests`, `scripts/coverage.sh` | CTest + coverage XML job |
| NFR-06 | Deterministic performance probe | `benchmarks/benchmark.cpp` | CI benchmark smoke |
| NFR-07 | Offline/no vehicle side effects | no bus I/O implementation | architecture + threat review |

## Evidence classification

- **Verified locally:** source build (GCC and Clang), C++ unit tests, CLI contracts, synthetic oracle, source analyzer, sanitizer and bounded fuzzer smoke.
- **CI-defined:** workflows checked into repository; successful remote execution requires observing GitHub Actions results for current commit.
- **Real-recording evaluation:** the GEM-CAN workflow evaluates source-author recorded vehicle traffic after checksum-verified download. The resulting metrics must come from an actual run; a checked-in workflow is not itself a performance result.
- **Not achieved:** certified functional safety, real HIL testing, CANoe/dSPACE integration, cross-vehicle generalization, full OEM signal extraction or live system throughput guarantees.

The test matrix is a traceability starting point, not an ISO 26262 validation plan or formal assurance case.
