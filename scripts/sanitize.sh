#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug -DCANSEC_ENABLE_SANITIZERS=ON
cmake --build build-sanitized -j 2
ASAN_OPTIONS="detect_leaks=1:halt_on_error=1" UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1" \
  ctest --test-dir build-sanitized --output-on-failure
