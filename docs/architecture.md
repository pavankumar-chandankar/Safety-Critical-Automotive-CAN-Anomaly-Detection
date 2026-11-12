# Architecture and design review

## Context and system boundaries

`CAN Sentinel` is a deterministic offline software analysis pipeline. Input assets are untrusted normalized trace CSV or selected Linux can-utils candump ASCII lines. It does not read a vehicle bus or control actuators. Policy files are likewise untrusted input and must pass independent validation. The executable emits a trace summary and diagnostic catalog without persisting raw payloads in the output. The `cansec` executable implements configurable protocol checks. A separate `cansec_gem` tool builds a lightweight traffic profile from the real GEM-CAN normal-driving recording and evaluates that profile on a different labeled attack session.

## Logical components

1. **Frame value type** (`frame.hpp`): a bounded `std::array<uint8_t,64>`, typed identifiers, microsecond timestamps, explicit frame format (`std`/`ext`), bus index, CAN FD/BRS/RTR and DLC. A fixed capacity makes bounds evident in review.
2. **Normalized trace parser** (`parser.cpp`): exact nine-column schema, bounded records, strict flags and hex payload width, DLC-to-length mapping, field-level parse diagnostics. Parse errors are collected and the remainder can be analyzed for forensic completeness.
3. **candump adapter** (`candump.cpp`): strict ASCII subset converter for training and recorded logs. Must not silently guess frame format, ESI support or payload length.
4. **Signal interpretation** (`signals.cpp`): Intel (LSB-first) and Motorola (DBC sawtooth) bit walks, sign extension and physical conversion; invalid footprints fail closed.
5. **Policy loader** (`rules.cpp`): configurable ID+format, bus, period limits, FD permission, payload ceilings, rolling counter and demo checksum byte. Reject invalid controls and duplicate keys.
6. **Analysis state machine** (`engine.cpp`): keyed by `(bus, format, identifier)` for timing/counter state, with global timestamp monotonicity check, capped detail retention and unlimited total aggregated counts within bounded total frames.
7. **Evidence/reporting** (`report.cpp`): stable JSON version 1 and readable text, no raw capture data or potentially private vehicle information by default.
8. **Fault injection** (`replay.cpp`, generator): deterministic, explicit contamination of synthetic frame sequences; never target a real ECU or live bus.
9. **Real-recording benchmark** (`gem.cpp`, `gem_benchmark.cpp`): validated GEM-CAN CSV import, chronological baseline fitting, calibration, held-out benign assessment, separate attack-session scoring and per-class metrics. The original raw recordings remain external to this repository.
10. **Verification**: unit, CLI, seeded oracles, static analysis, sanitizer builds, fuzzer entry points, release build and performance smoke.

## Design invariants

- A frame entering the detector came from validation or from a typed in-memory generator; parsers fail explicitly instead of accepting ambiguous layouts.
- A CAN FD DLC of 9 means 12 bytes, not nine; classic CAN uses <=8 actual bytes.
- Payload reads always require byte index `< length` and signal-bit walk must remain within `length * 8`.
- Rule and signal identifiers distinguish standard and extended frames even when numerical IDs coincide.
- The detector may record multiple findings per frame, with observable codes for triage and configurable cap on retained details.
- Repeated `Detector::inspect` calls clear state and produce identical results for identical input.
- Global time order is checked independently of per-message periodicity; capture ordering is not automatically sorted.
- There is no automatic signal/warning suppression based on assumed vehicle state; policies must be version-controlled and validated.

## Failure mode strategy

| Failure | Behavior |
|---|---|
| Malformed CSV / unsupported CAN semantics | `parse_errors` with file line; CLI exits 2 |
| Invalid rules or signal definitions | fail startup, diagnostic on stderr, exit 2 |
| Unknown identifier | `UNKNOWN_ID` finding (default on) |
| Signal missing from short payload | `SIGNAL_UNDECODABLE` finding |
| Detailed finding limit reached | aggregate counters continue; returned findings detail capped |
| Rule breach with `--fail-on-finding` | machine-readable result + exit 3 |
| Clean, valid trace | result + exit 0 |

## Scaling discussion

The current parser holds at most one million frames in memory by default. It is suitable for bounded offline regression and exploratory trace analysis, not an unbounded continuous stream. Production adaptation should use an incremental ring buffer, bounded queues, backpressure, per-bus clock synchronization, payload redaction, and a stress-test plan that covers multi-hour traces and bus reset behavior. Latency benchmark numbers vary per machine and must be reported together with hardware and compiler configuration.
