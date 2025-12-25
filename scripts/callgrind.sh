# These scripts + CMakePreset.json are just for practice manual build
#!/usr/bin/env bash
set -euo pipefail

cmake --preset rel
cmake --build --preset rel

# Run callgrind and show results in less
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./rel/test_unique_fd 
callgrind_annotate callgrind.out | less
