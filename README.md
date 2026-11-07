# Automotive CAN — offline C++17 frame validation prototype

This repository now contains a small, **synthetic-data-only** classic CAN frame validator intended to demonstrate C++ input validation, regression fixtures, and a Clang Static Analyzer workflow. It is **not** a production automotive safety mechanism, a bus sniffer, an anomaly-detection classifier, or an integration with Vector CANoe/dSPACE.

## Reproducible C++ validator

```bash
g++ -std=c++17 -Wall -Wextra -Werror cpp/can_validate.cpp -o can_validate
./can_validate examples/standard_frames.csv
python3 -m unittest discover -s tests -v
clang++ --analyze -std=c++17 -Wall -Wextra cpp/can_validate.cpp
```

Input: four CSV columns, with no header: `timestamp_ms,can_id_hex,dlc,payload_bytes_hex_spaced`. For example:

```csv
100,0x123,2,AA BB
101,0x3F0,1,7F
102,0x7FF,0,
```

The validator checks that timestamps are nonnegative decimal integers within a documented prototype range, identifiers are at most 11 bits, classic CAN DLC is 0–8, payload tokens are bytes, and payload byte count equals DLC. A nonzero exit code indicates rejected rows; diagnostics include source line numbers.

**Automated checks:** `tests/test_can_validate.py` contains deterministic valid/invalid fixtures. `.github/workflows/can-validation.yml` runs tests and Clang's `--analyze` stage in GitHub Actions. CI execution status must be checked separately; a workflow file alone is not evidence of a passing build.

## Legacy Python experiments (separate project, not CAN validation)

The pre-existing `Anomaly_Detection/` directory contains experimental Python data-processing, Prophet forecasting and Isolation Forest snippets involving public-transport demand and a UCI car-evaluation dataset. These files are **not a CAN-message processing implementation**. They have not been repurposed or represented as automotive test coverage. Some scripts are incomplete/inconsistent and should be treated as legacy exploratory code.

## Scope and evidence for job applications

Supported by the current C++ additions:
- Standard 11-bit classic-CAN frame parsing and validation from synthetic CSV traces
- Seeded regression fixtures and input-defect checks
- Clang Static Analyzer integration and C++17 compilation configuration

Not demonstrated here: automotive DBC parsing, extended CAN identifiers, CAN FD, live ECU communication, CANoe/dSPACE integration, real-vehicle anomalies, detection accuracy or safety certification.

**History note:** The C++ validator and tests were added later as a portfolio engineering exercise. The separate professional CAN-bus and HIL experience in the CV should be supported through your L&T employment, not attributed to this repository's historical code.
