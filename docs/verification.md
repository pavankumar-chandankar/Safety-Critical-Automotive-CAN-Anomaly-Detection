# Verification plan and evidence

## Local commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j 2
ctest --test-dir build --output-on-failure
bash scripts/static_analysis.sh
bash scripts/sanitize.sh
cmake -S . -B build-fuzz -DCMAKE_CXX_COMPILER=clang++ -DCANSEC_BUILD_FUZZERS=ON -DCANSEC_BUILD_TESTS=OFF
cmake --build build-fuzz
./build-fuzz/fuzz_trace -runs=2000 -seed=42
./build-fuzz/fuzz_candump -runs=2000 -seed=42
```

Tests cover the exact input contract, DLC edge mapping, mixed endian fields, signed extreme values, overlapping field rejection, rule sanity, counter wraparound, trigger codes, retention limits, JSON string escaping, strict candump conversion and deterministic fault mutation.

The **GEM-CAN benchmark** is evaluated separately from the deterministic protocol regression suite. To run it, use `scripts/download_gem_can.py` followed by `scripts/run_gem_benchmark.py`. That workflow loads the source authors' real vehicle recordings, writes frame-level metrics and checks each source CSV against recorded SHA-256 hashes.

## Pass criteria

1. All C++ registrations and CLI tests pass on GCC and Clang.
2. The nominal synthetic capture has zero findings; seeded defects cause their required diagnostic code.
3. Clang SA emits no warnings on production source and catches the two required quarantined defect examples.
4. ASan/UBSan runs complete without an error. Zero sanitizer errors is **not** proof of absence of all memory bugs.
5. Bounded libFuzzer smoke completes without a crash; long-duration fuzzing remains future work.
6. Release installation yields an executable binary without downloading third-party C++ libraries.

## Regression and baselines

Source changes should add regression tests to cover new parsing, decoding, and rule branches. Record new false positives as a change request to the rules and oracle with trace provenance. Prefer keeping the nominal fixture as a stable negative control rather than retrofitting expected results after a code failure. CI failures are visible from GitHub Actions; treat `workflow added` and `workflow passed` as different statements.

## Cautions

Fuzzing in CI uses short budgets and starts with empty corpus. Measured coverage is therefore limited. The synthetic benchmark preloads frames and measures only the in-memory detector stage; it intentionally excludes disk input, file parsing, log serialization and toolchain startup. It is **not** a realtime CAN gateway benchmark.
