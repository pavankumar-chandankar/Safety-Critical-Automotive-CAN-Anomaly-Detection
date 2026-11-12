# Limitations

The C++17 toolchain makes it possible to reproduce a basic CAN intrusion benchmark on the public GEM-CAN recordings. The dataset is real, but one dataset is not sufficient to establish that the detector would behave reliably on another vehicle or network configuration.

- The original normal-driving capture and attack session were collected under different operating conditions. Apparent novelty may reflect that difference rather than malicious behavior.
- The baseline uses heuristic feature weights, with the threshold calibrated on nominal traffic only. It is neither a supervised classifier nor a formally verified intrusion detector.
- The dataset is heavily imbalanced toward DoS. Per-class recall and normal false-positive rates are essential context.
- The protocol rule packs are examples and do not establish OEM message semantics, proprietary checksum algorithms, or full DBC coverage.
- Static analyzers, fuzzing and sanitizers help find software defects. Passing them cannot certify a safety-critical embedded system.
- No physical ECU communication, vehicle network injection, CANoe/dSPACE HIL bench, or production deployment is included.

The real-data benchmark's CSV loader, calibration, metrics and provenance checks are described in [gem_can_evaluation.md](gem_can_evaluation.md).
