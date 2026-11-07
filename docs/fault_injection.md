# Deterministic seeded-fault campaign

This campaign tests **software behavior on controlled synthetic inputs**, not anomaly recall on an external dataset. The oracle contains declared expected diagnostic codes, stable test data and a negative control.

| Scenario | Mutated field or behavior | Oracle |
|---|---|---|
| `none` | Nominal message schedule | Zero findings, zero parse errors |
| `counter-jump` | Counter differs from expected modulo-16 increment | `COUNTER_JUMP` |
| `checksum` | Illustrative SUM8 checksum byte toggled | `CHECKSUM_MISMATCH` |
| `signal-range` | Synthetic speed raw field placed outside allowed engineering range | `SIGNAL_RANGE` |
| `unknown-id` | Message ID replaced with unconfigured ID | `UNKNOWN_ID` |
| `wrong-bus` | Bus index changed | `WRONG_BUS` |
| `timing-burst` | Event timestamp duplicated | `PERIOD_TOO_SHORT` |
| `fd-length` | FD frame provided to a classic-only policy | `UNEXPECTED_FD` |
| `malformed` | Invalid hexadecimal payload | `PAYLOAD_HEX` parser error |

Run:

```bash
python3 scripts/evaluate_fault_campaign.py --binary ./build/cansec --seed 42 --cycles 40 --output artifacts/fault_campaign.json
```

**Interpretation:** `oracle_passed` counts whether each designed perturbation caused its expected code, not true positive rate in the scientific sense. Some perturbations also disrupt scheduling or other downstream counters, creating *additional legitimate secondary findings*; only the stated code is mandatory. Do not report the campaign result as `100% attack detection accuracy` in a CV or paper.

## Future scientific evaluation plan

To estimate real effectiveness, obtain permission to use appropriately documented CAN traces with benign operational variance, then pre-register attack/fault categories, separation across vehicles and drives, noise conditions, base rates, thresholds and calibration. Report precision/recall, false alerts per hour, per-ID error rates, latency percentiles, robustness to missing frames and capture clock drift; include confidence intervals and failure analysis. Use a protocol-valid baseline and peer-reviewed benchmark where licensing permits. Separately test static analyzer findings on known source-code defect corpora rather than trace labels.
