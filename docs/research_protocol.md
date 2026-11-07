# R&D evaluation protocol (future real-data work)

## Research question

Under which operating conditions can deterministic protocol-aware invariants detect CAN communication anomalies, and how does their false-positive behavior compare with statistical or learned baselines on unseen vehicles or recording sessions? A second, distinct question is how static C++ analysis, sanitizers and fuzzers affect defect discovery during implementation.

## Proposed hypotheses (not yet tested)

H1: explicit per-ID timing and counter invariants detect selected injected schedule/counter faults when the underlying message contracts are known and stable. H2: fixed thresholds produce false positives under benign clock drift, capture dropout or shifted vehicle duty cycles. H3: a layered source-quality pipeline detects distinct bug classes compared with compiler warnings alone.

## Proposed data design

Do not collect or publish raw vehicle CAN telemetry without authorization. Required metadata: capture hardware, DLC/FD settings, timestamp clock, bus load, model/year if permitted, environmental conditions, drives/vehicles and trace ownership. Separate data by physical vehicle and drive to prevent near-duplicate leakage. Maintain a locked holdout and a formal label taxonomy for baseline, physical faults, injection artifacts and unknown events.

## Baselines and metrics

Baseline A: no filtering (everything accepted). B: simple ID whitelist. C: per-ID period rules. D: period+counter+checksum. If meaningful labeled data becomes available, add a statistical model and compare resource requirements. Metrics: recall and precision by fault family, false alerts per hour and per 10k messages, median/95th/99th processing latency, memory limits, robustness to timestamp jitter, per-vehicle generalization and calibration drift. Provide uncertainty intervals and confidence bounds for sufficiently independent recording units.

## Reproducibility and governance

Freeze split manifest, rule pack SHA-256, code commit SHA, compiler versions, seeds, generated report hashes and toolchain diagnostics. Document policy provenance and distinguish user-defined synthetic checksum from real OEM format. Never position this work as satisfying an automotive standard without independent evidence and a formal certification process.
