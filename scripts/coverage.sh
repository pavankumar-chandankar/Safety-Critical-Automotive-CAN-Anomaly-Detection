#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DCANSEC_ENABLE_COVERAGE=ON
cmake --build build-coverage -j 2
ctest --test-dir build-coverage --output-on-failure
if command -v gcovr >/dev/null 2>&1; then
  mkdir -p artifacts
  gcovr --root . --filter 'src/' --exclude 'tests/' --xml-pretty -o artifacts/coverage.xml build-coverage
  gcovr --root . --filter 'src/' --exclude 'tests/' --html-details -o artifacts/coverage.html build-coverage
  gcovr --root . --filter 'src/' build-coverage
else
  echo 'gcovr not installed; raw .gcda/.gcno data available in build-coverage'
fi
