# Threat model and security boundaries

## Assets and deployment

The assets are integrity of source-code checks, fidelity of offline trace parsing, reproducibility of defect findings, and confidentiality of any imported diagnostic traces. The tool runs on a developer workstation or CI worker, not in an ECU or gateway; it has no CAN socket and cannot transmit frames.

## Trust boundaries

- Untrusted trace input: timestamps and frames may be malformed, intentionally hostile or arbitrarily long. Enforce bounds before interpreting payload or using bitfields.
- Untrusted rule/signal catalogs: malformed lengths, overlapping fields and duplicate keys should fail startup. Rules must be reviewed and version-controlled.
- CI/toolchain: threat of compromised compiler, dependencies, runner images or reporting artifacts. Avoid uncontrolled dependency fetching and shell interpolation from trace text.
- Output reports: findings contain ID, timestamps and line numbers but omit payload values by default. These can still constitute operational metadata; restrict sharing where necessary.

## Threats and mitigations

| Threat | Demonstrated mitigation | Residual risk |
|---|---|---|
| Malformed IDs/DLC/payload | strict parser, limits and negative regression tests | unsupported log dialects rejected |
| Out-of-bounds signal access | validated bit footprints, sanitizer tests | undocumented non-DBC encodings |
| Timestamp manipulation | monotonicity and period diagnostics | clock resets and capture jitter can appear malicious |
| Replayed/displaced messages | counter and period anomaly checks | sufficiently crafted replay could preserve expected counters/timing |
| Rule misconfiguration | loader rejects invalid and duplicate entries | semantically incorrect yet valid rules remain possible |
| Data exfiltration | no network client or raw-payload JSON output | users can still mishandle input files |
| Static source bugs | compiler warnings, Clang SA, CTest, ASan/UBSan, fuzz | no formal proof of source correctness |

## Misuse boundary

This is a defensive **offline** validation tool. The code does not implement UDS service calls, exploit payload generation, ECU unlocking, live frame injection, or vehicle control. Synthetic fault injection acts on in-memory/offline fixtures. The repository should not be used as evidence of real intrusion detection effectiveness without independently labeled testing.
