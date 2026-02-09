#!/bin/bash
# Build and test all configurations for Linux

set -e  # Exit on first error

PRESETS=(
  "linux-clang-debug"
  "linux-clang-release"
  "linux-gcc-debug"
  "linux-gcc-release"
)

for preset in "${PRESETS[@]}"; do
  echo "========================================"
  echo "=== Building $preset ==="
  echo "========================================"
  
  # Build everything (all test executables, examples, etc.)
  cmake --build build/$preset -j $(nproc)
  
  echo ""
  echo "========================================"
  echo "=== Running all tests for $preset ==="
  echo "========================================"
  
  # Run all test executables
  ctest --test-dir build/$preset --output-on-failure
  
  echo "✓ $preset complete"
  echo ""
done

echo "========================================"
echo "All builds and tests passed!"
echo "========================================"
