# Changes

## November 2026 — GEM-CAN evaluation

- Added a C++17 importer for the authors' published GEM-CAN CSV schema.
- Built a normal-traffic anomaly baseline and independent-session performance reporting.
- Added a version-pinned dataset downloader with archive and per-file integrity checks.
- Added real-recording evaluation workflow, parser tests, and a dedicated methodology document.
- Retained the existing CAN/CAN FD protocol engine and source-analysis test suite.

## Earlier engineering work

- Implemented protocol-aware message parsing, signal extraction, message rules and JSON findings.
- Added CMake/CTest, Clang Static Analyzer, ASan/UBSan, libFuzzer and fault-regression checks.
