# Build and test all configurations for Windows
# Run this script from the repository root directory

$ErrorActionPreference = "Stop"  # Exit on first error

$Presets = @(
    "windows-msvc-debug",
    "windows-msvc-release",
    "windows-clang-debug",
    "windows-clang-release"
)

foreach ($preset in $Presets) {
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "=== Building $preset ===" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    # Build everything (all test executables, examples, etc.)
    cmake --build build/$preset -j $env:NUMBER_OF_PROCESSORS
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "=== Running all tests for $preset ===" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    # Run all test executables
    ctest --test-dir build/$preset --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    
    Write-Host "✓ $preset complete" -ForegroundColor Green
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Green
Write-Host "All builds and tests passed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
