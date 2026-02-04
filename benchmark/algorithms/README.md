# Algorithm Benchmarks

This directory contains benchmarks for graph algorithms implemented in the graph-v3 library.

## Overview

Benchmarks use Google Benchmark to provide:
- Statistical performance analysis
- Regression tracking over time
- Comparison between different container types
- Scalability analysis across graph sizes

## Running Benchmarks

### Quick Start

```bash
# Build benchmarks
cmake --build build --target benchmark_shortest_path

# Run all algorithm benchmarks
./build/benchmark/algorithms/benchmark_shortest_path

# Run with JSON output for regression tracking
./build/benchmark/algorithms/benchmark_shortest_path --benchmark_format=json > results.json

# Run specific benchmarks
./build/benchmark/algorithms/benchmark_shortest_path --benchmark_filter=Dijkstra

# Control iterations
./build/benchmark/algorithms/benchmark_shortest_path --benchmark_min_time=5.0
```

### Benchmark Output

```
Run on (16 X 3400 MHz CPU s)
CPU Caches:
  L1 Data 32K (x8)
  L1 Instruction 32K (x8)
  L2 Unified 256K (x8)
  L3 Unified 16384K (x2)
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
Dijkstra_vov_small/10           1234 ns         1234 ns       567890
Dijkstra_vov_medium/100        12345 ns        12345 ns        56789
Dijkstra_vov_large/1000       123456 ns       123456 ns         5678
```

## Benchmark Structure

Each algorithm benchmark file follows this pattern:

```cpp
// benchmark_shortest_path.cpp
#include <benchmark/benchmark.h>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include "tests/common/graph_fixtures.hpp"

// Benchmark for specific graph size and container type
static void BM_Dijkstra_vov(benchmark::State& state) {
    size_t n = state.range(0);
    auto g = create_graph<vov_weighted>(n);
    
    std::vector<int> distance(n);
    std::vector<uint32_t> predecessor(n);
    
    for (auto _ : state) {
        dijkstra(g, 0, distance, predecessor, 
                 [](const auto& e) { return edge_value(e); });
        benchmark::DoNotOptimize(distance);
    }
    
    state.SetComplexityN(n);
}
BENCHMARK(BM_Dijkstra_vov)->Range(8, 8<<10)->Complexity();

BENCHMARK_MAIN();
```

## Regression Tracking

### Saving Baselines

After implementing a new algorithm, save a baseline:

```bash
./build/benchmark/algorithms/benchmark_shortest_path \
    --benchmark_format=json \
    --benchmark_out=benchmark/baselines/dijkstra_baseline_$(date +%Y%m%d).json
```

### Comparing Results

Use the comparison script:

```bash
python3 scripts/compare_benchmarks.py \
    benchmark/baselines/dijkstra_baseline_20260203.json \
    current_results.json
```

Example output:

```
Benchmark                      Before      After     Ratio
------------------------------------------------------------
Dijkstra_vov_small/10         1234 ns    1200 ns    0.972x (faster)
Dijkstra_vov_medium/100      12345 ns   12500 ns    1.013x (slower)
Dijkstra_vov_large/1000     123456 ns  120000 ns    0.972x (faster)
```

## Benchmark Files

### Current Benchmarks

- `benchmark_shortest_path.cpp` - Dijkstra, Bellman-Ford, etc.
- `benchmark_search.cpp` - BFS, DFS, etc.
- `benchmark_spanning_tree.cpp` - Prim, Kruskal, etc.
- `benchmark_connectivity.cpp` - Connected components, etc.

### Adding New Benchmarks

1. Create `benchmark_<category>.cpp`
2. Include necessary fixtures from `tests/common/graph_fixtures.hpp`
3. Benchmark with multiple graph sizes using `->Range()`
4. Test with multiple container types
5. Add complexity analysis with `->Complexity()`
6. Update `CMakeLists.txt` to build the benchmark
7. Document expected complexity in `docs/algorithms/README.md`

## Best Practices

### Graph Size Selection

- **Small:** 8-64 vertices (cache-friendly, high iteration count)
- **Medium:** 128-1024 vertices (typical use case)
- **Large:** 2048-8192 vertices (scalability testing)
- Use powers of 2 for consistent comparison

### Container Type Selection

Benchmark representative container types:
- `vov` - baseline (vector<vector>)
- `dov` - deque vertices
- `vol` - list edges
- `mov` - sparse vertices
- At least one from each category

### Iteration Control

- Let Google Benchmark auto-tune iterations for small graphs
- Use `--benchmark_min_time=N` for stable results on fast operations
- Use `--benchmark_repetitions=10` for statistical confidence

### Memory Benchmarks

```cpp
static void BM_Dijkstra_Memory(benchmark::State& state) {
    size_t n = state.range(0);
    auto g = create_graph<vov_weighted>(n);
    
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> distance(n);
        std::vector<uint32_t> predecessor(n);
        state.ResumeTiming();
        
        dijkstra(g, 0, distance, predecessor);
        
        benchmark::DoNotOptimize(distance);
    }
    
    state.SetItemsProcessed(state.iterations() * n);
    state.SetBytesProcessed(state.iterations() * n * (sizeof(int) + sizeof(uint32_t)));
}
```

## Performance Goals

### Time Complexity Targets

| Algorithm | Expected | Container | Notes |
|-----------|----------|-----------|-------|
| Dijkstra | O((V+E) log V) | Any | Priority queue overhead |
| BFS | O(V+E) | Any | Optimal |
| DFS | O(V+E) | Any | Optimal |
| Bellman-Ford | O(VE) | Any | Handles negative weights |
| Prim's MST | O((V+E) log V) | Any | Similar to Dijkstra |

### Regression Thresholds

- **Acceptable:** ≤5% slower than baseline
- **Warning:** 5-10% slower (investigate)
- **Failure:** >10% slower (requires fix)
- **Improvement:** >5% faster (document why)

## CI Integration

### Automated Benchmarking

The project can be configured to run benchmarks on each commit:

```yaml
# .github/workflows/benchmark.yml
name: Benchmark
on: [push, pull_request]

jobs:
  benchmark:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: cmake --build build --target benchmark_shortest_path
      - name: Run
        run: |
          ./build/benchmark/algorithms/benchmark_shortest_path \
            --benchmark_format=json \
            --benchmark_out=results.json
      - name: Compare
        run: python3 scripts/compare_benchmarks.py baseline.json results.json
```

## Troubleshooting

### High Variance

If results show high variance:
1. Close other applications
2. Disable CPU frequency scaling: `sudo cpupower frequency-set --governor performance`
3. Increase iterations: `--benchmark_repetitions=20`
4. Run on dedicated benchmark machine

### Unexpected Performance

If benchmarks don't match expected complexity:
1. Check compiler optimizations are enabled (`-O3`)
2. Verify fixture data is generated correctly
3. Profile with perf or vtune
4. Check for memory allocations in hot path
5. Ensure benchmark loop doesn't optimize away work

## Additional Resources

- [Google Benchmark Documentation](https://github.com/google/benchmark)
- [Project Benchmark Guidelines](../../docs/benchmark_guidelines.md) (to be created)
- [Algorithm Complexity Reference](../algorithms/README.md)
