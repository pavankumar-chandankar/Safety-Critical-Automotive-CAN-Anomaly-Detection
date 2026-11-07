#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
if ! command -v clang-format >/dev/null 2>&1; then
  echo 'clang-format is not installed; install it to enforce formatting.' >&2
  exit 2
fi
find include src app tests benchmarks defects/fuzz -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0 | \
  xargs -0 clang-format --dry-run --Werror
