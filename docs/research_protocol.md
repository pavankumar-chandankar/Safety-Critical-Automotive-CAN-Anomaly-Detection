# Experimental protocol

The study dataset for this repository is GEM-CAN, DOI [10.5281/zenodo.19161139](https://doi.org/10.5281/zenodo.19161139). The normal and attack files are **different physical recordings**. Neither capture is created by this codebase.

## Goal

Estimate whether a simple, interpretable C++17 profile built solely from nominal operation identifies message anomalies in a separate, labeled CAN intrusion session. We report *both* false alarms and per-attack-category recall to avoid hiding weak classes behind a large DoS majority.

## Sampling and thresholds

The first 60% of timestamp-ordered baseline frames fit per-ID features. The next 20% calibrate the cutoff for a user-specified nominal false-positive budget (default 1%). The final 20% check whether false alarms remain low on unseen nominal frames. The profile and threshold are frozen before the attack-session evaluation.

No attack labels or attack-session frames enter fitting or threshold selection. Results are frame-level and cannot independently establish detection latency, robustness to environmental changes, or generalization to another vehicle. The physical recordings can differ in drivetrain state and active ECU mix, which may create misleading alerts.

## Validation

The separate protocol parser and signal checks have unit tests and Clang static-analysis gates. Unit tests may use short constructed rows reflecting the published input schema, but those are contract tests. Dataset performance is produced only by `scripts/run_gem_benchmark.py` using an archive verified against the authors' release digest.
