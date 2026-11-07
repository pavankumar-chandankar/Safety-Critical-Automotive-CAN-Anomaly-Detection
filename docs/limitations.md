# Limitations and honest project claims

## Supported

- C++17 typed-frame processing, classic CAN and CAN FD DLC semantics, standard/extended IDs, bounded CSV parser, subset of candump ASCII conversion.
- DBC-inspired Intel/Motorola signal extraction and physical range checks (not a complete DBC parser).
- Rule-based offline communication anomaly *checks* (not validated intrusion detection).
- Testable rolling counters and illustrative SUM8 check, timing bounds, burst/missing-heartbeat heuristics, structured diagnostics.
- Clang Static Analyzer, compiler diagnostics, seeded source defects, sanitizer builds, fuzz harnesses, coverage support, CI configuration, deterministic synthetic oracle.

## Not supported

- Real CAN bus ingestion/transmission (SocketCAN, PCAN, VN/VX), CANoe interaction, dSPACE HIL, real-time ECU software, live automotive cybersecurity monitoring.
- Complete DBC import/export, UDS or ISO-TP, J1939 multi-packet transport, authenticated CAN, SecOC, AUTOSAR compliant stack.
- Proprietary OEM checksums, trained ML models, per-vehicle tuning, false-positive rate on real operational captures, real-attack precision/recall.
- Formal verification, certification, ISO 26262 compliance, ASIL designation, ISO/SAE 21434 compliance claim, MISRA conformance claim.

## How to represent it in a CV

Good: "Built a C++17 offline CAN/CAN FD validation and defect-detection framework with protocol-aware parsing, Intel/Motorola signal decoding, configurable anomaly rules, deterministic fault injection, and Clang Static Analyzer / ASan / UBSan / libFuzzer / CTest CI workflows. Verified seeded scenarios and regression tests."

Do not claim: "Deployed safety-critical CAN IDS on production vehicles", "certified to ISO 26262", "100% accurate automotive intrusion detection", "developed OEM DBC parser", or "validated in dSPACE/CANoe", unless those were actually established through separate evidence.

Note: This project was built as a later portfolio engineering exercise. It should not be described as being completed at L&T unless it genuinely occurred there.
