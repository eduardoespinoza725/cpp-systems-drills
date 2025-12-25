# These scripts + CMakePreset.json are just for practice manual build

#!/usr/bin/env bash
set -euo pipefail

cmake --preset debug
cmake --build --preset debug

ctest --preset debug
