#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
: "${CXX_CLANG:=clang++}"
: "${BUILD_DIR:=build-static}"
printf '%s\n' '== C++17 Clang build with strict compiler warnings =='
cmake -S . -B "$BUILD_DIR" -DCMAKE_CXX_COMPILER="$CXX_CLANG" -DCMAKE_BUILD_TYPE=Debug -DCANSEC_WARNINGS_AS_ERRORS=ON
cmake --build "$BUILD_DIR" -j 2
printf '%s\n' '== Clang Static Analyzer: library and executable translation units =='
python3 scripts/check_production_sast.py --clang "$CXX_CLANG"
printf '%s\n' '== Seeded analyzer sensitivity checks (null deref and divide by zero) =='
python3 scripts/check_seeded_defects.py --clang "$CXX_CLANG"
if command -v clang-tidy >/dev/null 2>&1; then
  printf '%s\n' '== clang-tidy: core implementation =='
  for f in src/*.cpp app/*.cpp; do clang-tidy -p "$BUILD_DIR" "$f" --quiet; done
else
  echo 'clang-tidy not installed; skipping optional tidy scan'
fi
if command -v cppcheck >/dev/null 2>&1; then
  printf '%s\n' '== cppcheck =='
  cppcheck --std=c++17 --language=c++ --enable=warning,performance,portability --error-exitcode=1 \
    --inline-suppr --quiet -Iinclude src app
else
  echo 'cppcheck not installed; skipping optional scan'
fi
