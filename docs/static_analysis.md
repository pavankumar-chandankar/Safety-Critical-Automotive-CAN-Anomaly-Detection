# Static-analysis strategy and defect taxonomy

## Purpose

Static analysis searches C++ code paths for potential defects without executing actual CAN frames. It complements (and cannot replace) runtime sanitizers, unit tests and realistic validation. Separately, **trace anomaly detection** looks for communication-policy violations in input data; do not conflate these two different meanings of "defect detection".

## Configured layers

| Layer | Commands/config | Examples of targets | Verification status |
|---|---|---|---|
| Compiler warnings | GCC/Clang `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wformat=2 -Wnull-dereference -Werror` | signedness, shadowing, conversion, formatting | hard gate in CMake |
| Clang Static Analyzer | `clang++ --analyze ...` (production sources) | path-sensitive null dereference, div-zero, use of uninitialized memory | local run; CI checks |
| Clang analyzer seeded sensitivity | `scripts/check_seeded_defects.py` | deliberately incorrect null-deref, div-zero | must catch both |
| `clang-tidy` | `.clang-tidy`, compilation database | bugprone, cert, analyzer, performance | optional report-only initial baseline |
| `cppcheck` | `--enable=warning,performance,portability` | various dataflow/style risks | CI quality gate when tool installed |
| ASan | `-fsanitize=address` | overflows, use-after-free, invalid memory | sanitized C++/CLI runs |
| UBSan | `-fsanitize=undefined` | invalid arithmetic, shifts, undefined operations | sanitized C++/CLI runs |
| libFuzzer | `defects/fuzz/fuzz_{trace,candump}.cpp` | malformed parser inputs | optional bounded CI smoke |
| Code coverage | GCC/Clang coverage flags with `gcovr` | branch/line gaps | XML artifact |

## Seeded defect laboratory

The functions in `defects/seeded/` intentionally contain dangerous behavior and are excluded from application targets. This is a controlled **source-only** verification that the analyzer can flag representative error classes. The checker requires findings for a known null dereference and a divide-by-zero. Out-of-bounds and double-delete examples are included as review exercises; do not assume stable detection across compiler versions.

`python3 scripts/check_seeded_defects.py` returns 1 if required diagnostics are not found. Caution: detecting two seeded examples does not establish full bug-detection recall or formally prove the rest of the code is defect-free.

## Static-analysis finding triage

For each warning, record a unique finding ID, checker, source location, reproduction command, root-cause hypothesis, developer disposition, regression test and reviewed rationale for any suppression. The CI artifact retains raw clang-tidy diagnostics. Avoid broad blanket suppressions. Changes to rule packs require unit tests because a static analyzer cannot verify actual CAN protocol semantics.

## Supply chain / execution boundary

CMake does not automatically fetch external C++ dependencies. The Python campaign uses the standard library. Compilers, tools and GitHub runners remain external dependencies. Pin toolchain versions where practical, validate releases, avoid running untrusted trace data in privileged environments, and never execute the seeded defective functions.

References: [Clang Static Analyzer](https://clang.llvm.org/docs/ClangStaticAnalyzer.html) and [scan-build usage](https://clang.llvm.org/docs/analyzer/user-docs/CommandLineUsage.html).
