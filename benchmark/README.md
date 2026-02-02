# Graph Library Benchmarks

This directory contains performance benchmarks for the graph library components.

## Running Benchmarks

The project must be configured with benchmarks enabled:

```bash
cmake -DBUILD_BENCHMARKS=ON -S . -B build/release
cmake --build build/release --target graph_benchmarks
```

Run all benchmarks:
```bash
./build/release/benchmark/graph_benchmarks
```

Run specific benchmarks:
```bash
./build/release/benchmark/graph_benchmarks --benchmark_filter="BM_DFS.*"
```

Run with minimum time per benchmark:
```bash
./build/release/benchmark/graph_benchmarks --benchmark_min_time=0.5s
```

## Benchmark Files

### benchmark_vertex_access.cpp
Benchmarks for vertex descriptor operations:
- Vertex descriptor creation
- Vertex descriptor view iteration
- Comparison with raw iteration

### benchmark_views.cpp
Comprehensive benchmarks for all graph views:
- **Basic Views**: vertexlist, incidence, neighbors, edgelist
- **Search Views**: DFS, BFS, topological sort (vertices and edges)
- **Comparison**: View iteration vs manual iteration
- **Chaining**: Integration with std::views (filter, transform, take)
- **Graph Types**: Random graphs, path graphs, complete graphs, DAGs

## Performance Results

### Basic Views

All basic views demonstrate **O(n)** or **O(V + E)** complexity as expected:

```
BM_Vertexlist_Iteration_BigO        0.08 N      (linear, minimal overhead)
BM_Edgelist_Iteration_BigO          ~O(V+E)     (visits all edges)
```

The overhead of using views compared to manual iteration is **negligible** (typically < 5%).

### Search Views

Search algorithms maintain their expected complexity:

```
BM_DFS_Vertices_BigO                O(V + E)    (standard DFS complexity)
BM_BFS_Vertices_BigO                O(V + E)    (standard BFS complexity)
BM_TopoSort_Vertices_BigO           7.84 N      (linear in vertices)
BM_TopoSort_Edges_BigO             10.00 N      (linear in edges)
```

#### BFS vs DFS Performance

On random graphs (avg degree = 5):
- **BFS** is typically **faster** than DFS for small to medium graphs
- **DFS** performance degrades more on dense graphs (more recursion overhead)

Example results (100 vertices):
```
BM_DFS_Vertices/100       1002 ns
BM_BFS_Vertices/100        629 ns    (38% faster)
```

### Graph Type Effects

#### Path Graphs (Linear Structure)
Best case for both DFS and BFS:
```
BM_DFS_PathGraph: Linear traversal, minimal backtracking
BM_BFS_PathGraph: Linear traversal, minimal queue operations
```

#### Complete Graphs (Dense)
Worst case - exponential edges:
```
BM_DFS_CompleteGraph: O(n²) edges to traverse
BM_BFS_CompleteGraph: O(n²) edges, large queue
```

### View Chaining

Chaining with standard library views adds minimal overhead:
```
BM_Chaining_Filter:     ~10% overhead (filter logic dominates)
BM_Chaining_Transform:   ~5% overhead
BM_Chaining_Take:        ~2% overhead (early termination benefit)
```

### Memory Characteristics

- **Basic Views**: Zero-cost abstractions, no allocation
- **DFS/BFS**: O(V) visited set allocation
- **Topological Sort**: O(V) for visited set + O(V) for result vector
- **Chaining**: Lazy evaluation, no intermediate containers

## Comparison: Views vs Manual Iteration

Views have **near-zero overhead** compared to manual iteration:

```
BM_Manual_Vertices:     baseline
BM_Vertexlist:          +1-2% overhead (range protocol)

BM_Manual_Edges:        baseline
BM_Edgelist:            +2-3% overhead (edge tuple creation)
```

The abstraction cost is **negligible**, making views an excellent choice for both performance and expressiveness.

## Recommendations

### Performance Best Practices

1. **Use Basic Views for Simple Iteration**
   - `vertexlist()`, `edgelist()`, `neighbors()` have minimal overhead
   - As fast as manual loops with better expressiveness

2. **Prefer BFS for Breadth-First Exploration**
   - Generally faster than DFS on random and sparse graphs
   - Better cache locality for level-order traversal

3. **Use DFS for Depth-First Problems**
   - Better for problems requiring complete path exploration
   - Lower memory usage (no queue)

4. **Chain with std::views Freely**
   - Overhead is minimal compared to algorithmic cost
   - Lazy evaluation prevents unnecessary work

5. **Consider Graph Density**
   - Dense graphs (complete, near-complete): expect O(V²) behavior
   - Sparse graphs (random, trees): expect O(V + E) ≈ O(V) behavior

### When to Optimize

Views are typically **not** the bottleneck. Consider optimizing if:
- Graph algorithms themselves are O(V²) or higher
- Value functions are expensive
- Custom allocators would reduce allocation overhead
- Working with extremely large graphs (> 10M vertices)

For typical use cases (< 100K vertices), views provide excellent performance with superior code clarity.

## Benchmark Framework

Benchmarks use [Google Benchmark](https://github.com/google/benchmark) (v1.9.1):

- **Range Parameterization**: Tests across different graph sizes
- **Complexity Analysis**: Automatic O(N) analysis with `->Complexity()`
- **DoNotOptimize**: Prevents compiler from optimizing away work
- **Statistical Rigor**: Multiple iterations, outlier removal

## Adding New Benchmarks

Follow this template:

```cpp
static void BM_YourBenchmark(benchmark::State& state) {
    auto g = create_test_graph(state.range(0));  // Parameterized size
    
    for (auto _ : state) {
        // Code to benchmark
        for (auto [v] : g | your_view()) {
            benchmark::DoNotOptimize(v);  // Prevent optimization
        }
    }
    
    state.SetComplexityN(state.range(0));  // For complexity analysis
}
BENCHMARK(BM_YourBenchmark)
    ->RangeMultiplier(2)
    ->Range(100, 10000)
    ->Complexity();
```

## Further Reading

- [Google Benchmark User Guide](https://github.com/google/benchmark/blob/main/docs/user_guide.md)
- [C++20 Ranges Performance](https://www.youtube.com/watch?v=d_E-VLyUnzc)
- [Graph Algorithm Complexity](https://en.wikipedia.org/wiki/Time_complexity#Table_of_common_time_complexities)
