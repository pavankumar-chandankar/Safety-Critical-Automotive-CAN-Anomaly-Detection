# Design decisions

## Keep the detector offline

The project reads captured traces rather than transmitting frames or connecting to a vehicle bus. This keeps testing repeatable and avoids mixing validation research with vehicle modification.

## Separate message rules from learned profiles

The original CAN protocol checks validate frame structure and configured signal semantics. The GEM-CAN evaluator instead learns identifiers, DLC patterns, bytes and message timing from the dataset's normal-driving capture. It does not silently assume that example signal definitions correspond to the GEM vehicle.

## Use independent sessions for evaluation

The normal-driving recording is the sole source of fitting and threshold calibration. The attack capture is never mixed into a random row-level split. Separate recordings reduce direct leakage, although operating-condition shifts remain a problem to evaluate explicitly.

## Keep source-code defect analysis distinct from intrusion detection

Clang Static Analyzer and sanitizers find defects in C++ software. They cannot label a CAN frame as malicious. Conversely, an anomaly score on a public dataset does not prove that the C++ code is free of memory or logic defects. CI evaluates the two concerns independently.
