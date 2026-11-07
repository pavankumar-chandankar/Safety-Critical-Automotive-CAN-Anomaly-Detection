#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCANSEC_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python3 scripts/check_production_sast.py
python3 scripts/check_seeded_defects.py
python3 scripts/evaluate_fault_campaign.py --binary ./build/cansec --output artifacts/fault_campaign.json
