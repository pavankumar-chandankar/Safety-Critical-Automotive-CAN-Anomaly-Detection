# GEM-CAN v3: initial C++17 baseline results

**Dataset:** [GEM-CAN, Tavasoli et al., Zenodo v3](https://doi.org/10.5281/zenodo.19161139)  
**Data paper:** [Data in Brief 66 (2026), 112805](https://doi.org/10.1016/j.dib.2026.112805)  
**Verified execution:** [GitHub Actions run 43241363927](https://github.com/pavankumar-chandankar/Safety-Critical-Automotive-CAN-Anomaly-Detection/actions/runs/43241363927)  
**Code revision evaluated:** `4accb2f944d2fa0d662f10afc21e7ae7a702687e`

The workflow downloaded the authors' public recorded CAN files from Zenodo, checked the source archive's published MD5, compiled the C++17 code, ran tests, and wrote a confusion-matrix report. This is a **measured real-data experiment**, not a replay of the small generated parser fixtures.

## Data integrity

| Recording | Rows parsed | SHA-256 |
| --- | ---: | --- |
| Normal driving | 100,000 | `e5ef6e81883702edbe4e772dea3894de3ccdd112d2928df3f3d362aa5ba4f1aa` |
| Attack scenario | 43,444 | `b0ac27048130f6acc121d2ac49d565200331a923ab4f4ab94eb3c7792309f92d` |

Source archive MD5: `03f7a86ed05da9ab282f2886bf9b3ca3`.

The parsed attack file contains 43,444 rows, including 1,039 labeled normal frames. The data paper quotes approximately 43,443 rows / 1,038 benign frames; the counts above are from the verified version-pinned CSV actually processed.

## Experiment

The normal-driving capture was chronologically split into 60,000 fitting frames, 20,000 calibration frames, and 20,000 held-out normal frames. No attack-session labels or frames were used in fitting or threshold calibration. The profile scored CAN ID novelty, DLC changes, previously unseen payload bytes and shortened interarrival intervals. The calibrated score threshold was **0.75** for a target 1% nominal false-positive rate.

## Measured results

| Partition | TP | FP | TN | FN | FPR | Precision | Recall | F1 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Held-out normal driving | 0 | 152 | 19,848 | 0 | 0.76% | N/A | N/A | N/A |
| Independent attack recording | 41,081 | 425 | 614 | 1,324 | 40.90% | 98.98% | 96.88% | 97.92% |

### Attack-specific recall

| Attack | Labeled attack frames | Detected | Recall |
| --- | ---: | ---: | ---: |
| DoS flooding | 41,080 | 41,080 | 100% |
| Brake tampering | 663 | 1 | 0.15% |
| Steering tampering | 662 | 0 | 0% |

The aggregate F1 is high because DoS dominates this corpus and has an obvious new arbitration identifier. The same detector nearly completely **misses semantic tampering using legitimate IDs**. Its 40.9% false-positive rate on benign frames embedded in the attack recording makes it unsuitable as an operational IDS without considerable further work.

## Interpretation and follow-up

The mismatch between the normal-driving baseline and stationary-vehicle attack recording is a serious domain-shift risk. A classifier that learns which IDs appear when the vehicle moves may not generalize to legitimate traffic on a stationary vehicle. Frame-level F1 alone is inadequate: evaluate detection delay, false positives per hour, category-specific recall, class imbalance, and generalization across driving conditions.

Possible next research questions include context-conditioned normal profiles, per-ID multibyte payload transitions, sequence-based detectors, and additional independent benign sessions. New methods should be evaluated using additional held-out sessions or clearly labeled exploratory comparisons; optimizing against these same attack labels would contaminate the test.

## Reproduction

```bash
python3 scripts/download_gem_can.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python3 scripts/run_gem_benchmark.py
```

The GitHub Actions run stores `metrics.json`, `report.md`, and `provenance.json` as workflow artifacts. No raw third-party vehicle logs are redistributed in this repository.
