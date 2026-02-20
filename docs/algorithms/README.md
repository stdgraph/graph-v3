# Graph Algorithms

This directory contains implementations of standard graph algorithms for the graph-v3 library.

## Overview

The algorithm library provides efficient, STL-style implementations of classic graph algorithms that work with the graph-v3 container abstractions. All algorithms:

- Use C++20 concepts and ranges
- Work with multiple container types via CPO interface
- Follow standard library naming conventions
- Are thoroughly tested and benchmarked
- Include comprehensive documentation

## Quick Start

```cpp
#include <graph/graph.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>

using namespace graph;

// Create a weighted graph
using Graph = container::dynamic_graph<int, void, void, uint32_t, false,
                                       container::vov_graph_traits<...>>;
Graph g({{0, 1, 5}, {1, 2, 3}, {0, 2, 10}});

// Find shortest paths from vertex 0
std::vector<int> distance(num_vertices(g));
std::vector<uint32_t> predecessor(num_vertices(g));

dijkstra(g, 0, distance, predecessor,
         [](const auto& e) { return edge_value(e); });

// Use results
std::cout << "Distance to vertex 2: " << distance[2] << "\n"; // Output: 8
```

## Available Algorithms

### Shortest Path Algorithms

| Algorithm | File | Time Complexity | Space | Notes |
|-----------|------|-----------------|-------|-------|
| Dijkstra | `dijkstra_shortest_paths.hpp` | O((V+E) log V) | O(V) | Non-negative weights only |
| Bellman-Ford | `bellman_ford.hpp` | O(VE) | O(V) | Handles negative weights |
| BFS | `bfs.hpp` | O(V+E) | O(V) | Unweighted shortest path |
| A* | `astar.hpp` | O(E log V) | O(V) | Heuristic-guided |
| Floyd-Warshall | `floyd_warshall.hpp` | O(V³) | O(V²) | All-pairs shortest path |

*Note: Currently only Dijkstra is being implemented. Others are planned.*

### Search Algorithms

| Algorithm | File | Time Complexity | Space | Notes |
|-----------|------|-----------------|-------|-------|
| BFS | `bfs.hpp` | O(V+E) | O(V) | Level-order traversal |
| DFS | `dfs.hpp` | O(V+E) | O(V) | Depth-first traversal |
| Bidirectional BFS | `bidirectional_bfs.hpp` | O(V+E) | O(V) | Faster for single target |

*Planned for future implementation.*

### Minimum Spanning Tree

| Algorithm | File | Time Complexity | Space | Notes |
|-----------|------|-----------------|-------|-------|
| Prim | `prim.hpp` | O((V+E) log V) | O(V) | Dense graphs |
| Kruskal | `kruskal.hpp` | O(E log E) | O(V) | Sparse graphs |

*Planned for future implementation.*

### Connectivity

| Algorithm | File | Time Complexity | Space | Notes |
|-----------|------|-----------------|-------|-------|
| Connected Components | `connected_components.hpp` | O(V+E) | O(V) | Undirected graphs |
| Strongly Connected Components | `scc.hpp` | O(V+E) | O(V) | Directed graphs (Tarjan/Kosaraju) |
| Articulation Points | `articulation_points.hpp` | O(V+E) | O(V) | Cut vertices |
| Bridges | `bridges.hpp` | O(V+E) | O(V) | Cut edges |

*Planned for future implementation.*

### Topological Sort & DAG Algorithms

| Algorithm | File | Time Complexity | Space | Notes |
|-----------|------|-----------------|-------|-------|
| Topological Sort | `topological_sort.hpp` | O(V+E) | O(V) | DAG only |
| DAG Shortest Paths | `dag_shortest_paths.hpp` | O(V+E) | O(V) | Faster than Dijkstra |
| Longest Path (DAG) | `dag_longest_path.hpp` | O(V+E) | O(V) | NP-hard on general graphs |

*Planned for future implementation.*

### Flow Algorithms

| Algorithm | File | Time Complexity | Space | Notes |
|-----------|------|-----------------|-------|-------|
| Ford-Fulkerson | `ford_fulkerson.hpp` | O(E·max_flow) | O(V) | Integer capacities |
| Edmonds-Karp | `edmonds_karp.hpp` | O(VE²) | O(V) | BFS-based Ford-Fulkerson |
| Dinic | `dinic.hpp` | O(V²E) | O(V) | Fast for many graphs |

*Planned for future implementation.*

## Algorithm Requirements

### Graph Concepts

Most algorithms require:

```cpp
template<typename G>
concept algorithm_compatible_graph = 
    adjacency_list<G> &&
    forward_range<vertex_range_t<G>> &&
    integral<vertex_id_t<G>>;
```

Some algorithms have additional requirements:
- **Weighted algorithms:** Require edge weight function or `edge_value_t<G>` to be arithmetic
- **DAG algorithms:** Require acyclic graph (not enforced at compile time)
- **Undirected algorithms:** Work with undirected graph types

### Container Requirements

Algorithms work with any graph container that satisfies the CPO interface:
- ✅ `dynamic_graph` (all configurations)
- ✅ `compressed_graph` (when implemented)
- ✅ `undirected_adjacency_list` (when fully integrated)
- ✅ Custom containers implementing graph CPOs

## Documentation

Each algorithm includes:
- **Header documentation:** Doxygen comments with full API specification
- **Algorithm documentation:** Detailed markdown in `docs/algorithms/<name>.md`
- **Template reference:** See [Algorithm Template](../contributing/algorithm-template.md)

### Documentation Structure

- **Brief description** - What problem does it solve?
- **Complexity analysis** - Time complexity (best, average, worst cases) and space complexity (including auxiliary data structures)
- **Supported graph properties** - Directed/undirected, weighted, multi-edges, self-loops, cycles, connectivity requirements
- **API reference** - Template parameters, function parameters
- **Mandates** - Compile-time requirements enforced via concepts
- **Preconditions & postconditions** - Contract specification (runtime requirements)
- **Example usage** - Complete code examples
- **Implementation notes** - Design decisions, optimizations
- **References** - Academic papers, textbooks

## Testing

All algorithms include comprehensive tests in [`tests/algorithms/`](../../tests/algorithms/):

- **Correctness tests:** Known results validation
- **Edge cases:** Empty graphs, single vertex, disconnected, etc.
- **Container compatibility:** Tested with multiple container types
- **Property tests:** Algorithm invariants

See [Testing README](../../tests/algorithms/README.md) for details.

## Benchmarking

Performance benchmarks in [`benchmark/algorithms/`](../../benchmark/algorithms/):

- Statistical analysis with Google Benchmark
- Comparison across container types
- Scalability analysis (small, medium, large graphs)
- Regression tracking over time

See [Benchmark README](../../benchmark/algorithms/README.md) for details.

## Usage Patterns

### Basic Usage

```cpp
// 1. Create or obtain a graph
auto g = /* ... */;

// 2. Prepare output containers
std::vector<int> distance(num_vertices(g));
std::vector<vertex_id_t<decltype(g)>> predecessor(num_vertices(g));

// 3. Run algorithm
dijkstra(g, source_vertex, distance, predecessor, weight_function);

// 4. Use results
for (size_t i = 0; i < num_vertices(g); ++i) {
    std::cout << "Distance to " << i << ": " << distance[i] << "\n";
}
```

### Path Reconstruction

```cpp
// After running Dijkstra or similar algorithm
auto path = reconstruct_path(source, target, predecessor);

// Helper function:
template<typename VId>
std::vector<VId> reconstruct_path(VId source, VId target,
                                  const std::vector<VId>& pred) {
    std::vector<VId> path;
    VId current = target;
    while (current != source) {
        path.push_back(current);
        current = pred[current];
    }
    path.push_back(source);
    std::ranges::reverse(path);
    return path;
}
```

### Custom Weight Functions

```cpp
// Custom edge weight
auto weight = [](const auto& edge) {
    return custom_weight_calculation(edge);
};

dijkstra(g, source, distance, predecessor, weight);
```

### Skip Predecessor Tracking

```cpp
// Use null_predecessors when you only need distances
dijkstra(g, source, distance, null_predecessors, weight);
```

## Design Principles

### 1. Generic Programming

Algorithms use templates and concepts to work with multiple container types without code duplication.

### 2. CPO-Based Interface

All graph operations go through Customization Point Objects (CPOs), enabling:
- Uniform interface across containers
- Easy addition of new container types
- Type-safe generic algorithms

### 3. STL Compatibility

Design follows STL conventions:
- Range-based interfaces
- Iterator/range categories
- Standard naming (snake_case)
- Minimal surprising behavior

### 4. Performance

Algorithms are optimized for:
- Cache efficiency
- Minimal allocations
- Inline-friendly design
- Modern CPU architectures

### 5. Composability

Algorithms can be combined:
- Output of one algorithm feeds another
- Modular weight functions
- Chainable views

## Contributing

### Adding a New Algorithm

1. **Design Phase:**
   - Review [Algorithm Template](../contributing/algorithm-template.md)
   - Check existing implementations for patterns
   - Consider container compatibility

2. **Implementation:**
   - Create `include/graph/algorithm/<name>.hpp`
   - Follow coding style guide
   - Add comprehensive Doxygen comments
   - Implement with concepts and ranges

3. **Testing:**
   - Create `tests/algorithms/test_<name>.cpp`
   - Include correctness tests with known results
   - Add parameterized tests across container types
   - Test edge cases thoroughly

4. **Benchmarking:**
   - Add benchmarks to appropriate file in `benchmark/algorithms/`
   - Test with multiple graph sizes
   - Compare with theoretical complexity
   - Document performance characteristics

5. **Documentation:**
   - Create `docs/algorithms/<name>.md` following template
   - Update this README with algorithm info
   - Add usage examples
   - Document any limitations

6. **Review:**
   - Submit pull request
   - Address review comments
   - Ensure CI passes
   - Update version history

## Future Work

- [ ] Implement remaining shortest path algorithms
- [ ] Add search algorithms (BFS, DFS)
- [ ] Implement MST algorithms
- [ ] Add connectivity algorithms
- [ ] Implement flow algorithms
- [ ] Add parallel algorithm variants
- [ ] Support for external memory graphs
- [ ] GPU-accelerated versions

## References

### Textbooks

- **Introduction to Algorithms (CLRS)** - Cormen, Leiserson, Rivest, Stein
  - Comprehensive coverage of graph algorithms
  - Reference for correctness proofs

- **Algorithm Design** - Kleinberg, Tardos
  - Problem-solving approach
  - Good for understanding algorithm design patterns

- **Algorithms** - Sedgewick, Wayne
  - Practical implementations
  - Performance analysis

### Standards & Proposals

- **P1709: Graph Library** - STL graph library proposal
  - Design inspiration for this library
  - Standard graph concepts and algorithms

### Online Resources

- [CP-Algorithms](https://cp-algorithms.com/) - Implementation reference
- [GeeksforGeeks Algorithms](https://www.geeksforgeeks.org/graph-data-structure-and-algorithms/) - Tutorials
- [VisuAlgo](https://visualgo.net/en/graphds) - Algorithm visualization

## See Also

- [CPO Implementation Guide](../contributing/cpo-implementation.md)
- [Adjacency List Interface](../reference/adjacency-list-interface.md)
- [View Documentation](../user-guide/views.md)
- [Main README](../../README.md)
