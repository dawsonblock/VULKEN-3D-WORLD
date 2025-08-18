#!/usr/bin/env bash
set -euo pipefail
BUILD_DIR=${1:-build}
shift || true
cmake -S "$(dirname "$0")/.." -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo "$@"
cmake --build "$BUILD_DIR" -j
ctest --test-dir "$BUILD_DIR" --output-on-failure
