# Architecture decision records

## ADR-001 — C++17 and zero runtime third-party dependencies

**Decision:** C++17 core and standard library containers, CMake as build system. **Reason:** portable offline validation with transparent memory and numeric semantics, easy compilation with several toolchains. **Tradeoff:** no full DBC library, no heavyweight JSON schema dependency, no CAN driver adapters.

## ADR-002 — explicit normalized capture contract

**Decision:** strict CSV with exactly nine fields; subset candump converter. **Reason:** avoiding silent misinterpretation of ambiguous capture variants is more important than permissive parsing in a defect test tool. **Tradeoff:** legitimate vendor formats need declared adapters.

## ADR-003 — synthetic oracle not real-world accuracy

**Decision:** seeded faults are designed fixtures with explicit code expectations; no precision/recall claim. **Reason:** no authorized labeled real-vehicle dataset or universal OEM message definitions are available.

## ADR-004 — Clang SA, seeded examples, fuzz, sanitizers as separate instruments

**Decision:** source-level analysis, runtime sanitizers and offline fault detection have separate test targets and artifacts. **Reason:** a trace heuristic never proves that C++ memory code is safe, and static analysis never proves an ECU attack was detected.

## ADR-005 — rule packs as versioned configuration

**Decision:** machine-readable CSV policies with validators and non-overlapping signal footprints. **Reason:** testability and auditability; separate implementation defects from semantic modeling mistakes.

## ADR-006 — fail closed for unsupported protocols

**Decision:** reject CAN FD ESI flags in candump converter, nonrepresentable DLC lengths, invalid RTR/FD combinations, unsupported ID widths and ambiguous bitfields. **Reason:** never silently transform unknown source input into apparently valid evidence.

## ADR-007 — sensitive data minimization

**Decision:** default diagnostics include frame ID, line, time, code and explanation, not raw signal payload bytes. **Reason:** CAN traces may reveal operational patterns; tests use synthetic fixtures. **Limitation:** metadata itself can still be sensitive.
