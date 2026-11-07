# Evidence-based job interview brief

## What the project demonstrates today

**C++ systems development:** typed CAN/CAN FD frames, bounded input processing, configuration parsing, deterministic stateful rule execution, bit-exact signal extraction, reliable exit codes, portable CMake builds and low-dependency CLI tools.

**Code quality:** compiler warning gates, Clang Static Analyzer production checks, sensitivity checks on intentionally defective C++ functions, ASan/UBSan instrumentation, optional clang-tidy/cppcheck, CTest coverage and parser libFuzzer harnesses.

**Automotive domain knowledge:** classic/FD DLC mapping, standard/extended IDs, BRS/RTR semantics, payload length checks, signal bit ordering, message periodicity, rolling counters and diagnostic trace formats. This is an *offline test system*, not an OEM ECU integration.

**Evaluation engineering:** deterministic seeded faults, defined negative control, machine-readable oracle, source/data contract tests, traceability matrix and reproducible toolchain smoke tests. No unseen data evaluation results are claimed.

## Demonstration walkthrough

1. Explain risk of malformed DLC and unsafe bit reads; show `src/parser.cpp`, `src/signals.cpp` and `tests/test_signals.cpp`.
2. Build twice (`g++` and `clang++`) with strict warnings; explain portability and why unsigned/signed conversion warnings matter.
3. Show nominal synthetic trace with no alerts; inject and detect checksum or counter mutation, with JSON evidence and exit 3 when requested.
4. Run Clang SA against production translation units; run seeded mutant sensitivity check without executing unsafe functions.
5. Explain fuzz harness, ASan/UBSan and the difference between runtime vulnerability tests and protocol anomaly detection.
6. Describe limitations: no dSPACE/CANoe adapter or real attack dataset; propose a credible HIL/vehicle evaluation plan.

## CV-ready wording

> **Automotive CAN-Bus — C++17 Static Analysis & Defect Detection (Engineering R&D).** Developed a protocol-aware offline CAN/CAN FD trace analysis framework with 11-/29-bit ID validation, DLC checks, Intel/Motorola signal decoding, configurable timing/counter/checksum anomaly rules, seeded fault-injection campaigns, and JSON diagnostics. Integrated Clang Static Analyzer, strict GCC/Clang builds, ASan/UBSan, libFuzzer, CMake/CTest, and automated CI checks; verified behavior on deterministic synthetic traces.

**Do not claim:** automotive certification, on-vehicle deployment, production-grade IDS recall, verified HIL integration, a complete DBC interpreter, or that this newly built portfolio project was developed during the earlier L&T employment.
