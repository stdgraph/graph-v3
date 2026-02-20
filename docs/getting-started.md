# Getting Started

> [← Back to Documentation Index](index.md)

This guide walks you through installing graph-v3, building your first graph,
running an algorithm, and working with edge lists.

---

## 1. Requirements

| Requirement | Minimum |
|-------------|---------|
| C++ standard | C++20 |
| CMake | 3.20 |
| Compiler | GCC 13+, Clang 10+, or MSVC 2022+ |

Dependencies (Catch2, `tl::expected`) are fetched automatically by CMake.

---

## 2. Installation

### CMake FetchContent (recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
  graph3
  GIT_REPOSITORY https://github.com/pratzl/desc.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(graph3)

target_link_libraries(your_target PRIVATE graph::graph3)
```

### CPM

```cmake
CPMAddPackage("gh:pratzl/desc@main")
target_link_libraries(your_target PRIVATE graph::graph3)
```

### Git submodule

```bash
git submodule add https://github.com/pratzl/desc.git extern/graph3
```

```cmake
add_subdirectory(extern/graph3)
target_link_libraries(your_target PRIVATE graph::graph3)
```

### System install

```bash
cmake --preset linux-gcc-release
cmake --install build/linux-gcc-release --prefix /usr/local
```

Then in your project:

```cmake
find_package(graph3 REQUIRED)
target_link_libraries(your_target PRIVATE graph::graph3)
```

---

## 3. Your First Graph

graph-v3 works with **your own containers** out of the box.
A `std::vector<std::vector<int>>` is already a valid graph — no wrapper needed.

```cpp
#include <graph/graph.hpp>
#include <iostream>

int main() {
  // Each inner vector is a vertex's outgoing edges (target vertex IDs).
  std::vector<std::vector<int>> g = {
    {1, 2},   // vertex 0 → 1, 2
    {2, 3},   // vertex 1 → 2, 3
    {3},      // vertex 2 → 3
    {}        // vertex 3 (no outgoing edges)
  };

  std::cout << "Vertices: " << graph::num_vertices(g) << "\n";

  for (auto&& u : graph::vertices(g)) {
    auto uid = graph::vertex_id(g, u);
    for (auto&& uv : graph::edges(g, u)) {
      std::cout << uid << " -> " << graph::target_id(g, uv) << "\n";
    }
  }
}
```

Output:

```
Vertices: 4
0 -> 1
0 -> 2
1 -> 2
1 -> 3
2 -> 3
```

### Using `dynamic_graph`

For richer features — edge values, flexible container selection, partitioning —
use `dynamic_graph` with one of the 26 trait combinations.

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <iostream>

using namespace graph;
using namespace graph::container;

// EV=double (edge value), VV=void, GV=void, VId=uint32_t
using Graph = vov_graph_traits<double>::graph_type;

int main() {
  // Construct from an initializer list of {source, target, weight} edges.
  Graph g({{0, 1, 1.0}, {0, 2, 4.0}, {1, 2, 2.0}, {2, 3, 3.0}});

  std::cout << num_vertices(g) << " vertices\n";

  for (auto&& u : vertices(g)) {
    for (auto&& uv : edges(g, u)) {
      std::cout << vertex_id(g, u) << " -(" << edge_value(g, uv) << ")-> "
                << target_id(g, uv) << "\n";
    }
  }

  // The same loop using views with structured bindings:
  for (auto&& [uid, u] : views::vertexlist(g)) {
    for (auto&& [tid, uv, wt] : views::incidence(g, u, [](auto& g, auto& uv) {
           return edge_value(g, uv);
         })) {
      std::cout << uid << " -(" << wt << ")-> " << tid << "\n";
    }
  }
}
```

Output:

```
4 vertices
0 -(1)-> 1
0 -(4)-> 2
1 -(2)-> 2
2 -(3)-> 3
```

### Using `compressed_graph`

When you need **maximum read performance** on a static graph, use `compressed_graph`.
It stores the adjacency structure in Compressed Sparse Row (CSR) format — two flat
arrays (row offsets + column indices) that are contiguous in memory.

**Benefits over `dynamic_graph`:**

| | `compressed_graph` | `dynamic_graph` |
|---|---|---|
| **Memory layout** | Two contiguous vectors (cache-friendly) | Vector of per-vertex edge containers |
| **Iteration speed** | Fastest — linear memory scan | Depends on edge container choice |
| **Memory overhead** | Minimal — no per-vertex container bookkeeping | Higher — each vertex owns a separate container |
| **Mutability** | Immutable after construction | Fully mutable |
| **Best for** | Analytics, read-heavy workloads, large graphs | Graph construction, dynamic updates |

```cpp
#include <graph/container/compressed_graph.hpp>
#include <iostream>

using namespace graph;
using namespace graph::container;

// EV=int (edge weight), VV=void, GV=void
using Graph = compressed_graph<int>;

int main() {
  // Same initializer-list construction as dynamic_graph: {source, target, value}
  Graph g({{0, 1, 10}, {0, 2, 5}, {1, 3, 1}, {2, 1, 3}, {2, 3, 9}});

  std::cout << num_vertices(g) << " vertices\n";

  for (auto&& u : vertices(g)) {
    for (auto&& uv : edges(g, u)) {
      std::cout << vertex_id(g, u) << " -(" << edge_value(g, uv) << ")-> "
                << target_id(g, uv) << "\n";
    }
  }
}
```

The same CPOs (`vertices`, `edges`, `target_id`, `edge_value`, …) and all
algorithms work identically on both container types — swap the type alias
and your code keeps working.

---

## 4. Your First Algorithm — Dijkstra Shortest Paths

All algorithms live in `include/graph/algorithm/`.
Here we run Dijkstra's single-source shortest paths on a weighted graph.

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <iostream>

using namespace graph;
using namespace graph::container;

using Graph = vov_graph_traits<int>::graph_type;

int main() {
  //      0 --10--> 1 --1--> 3
  //      |         ^        ^
  //      5         3        9
  //      |         |        |
  //      v         |        |
  //      2 --------+--------+

  Graph g({{0, 1, 10}, {0, 2, 5}, {1, 3, 1}, {2, 1, 3}, {2, 3, 9}});

  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  // Initialize distances to infinity, predecessors to self
  init_shortest_paths(distance, predecessor);

  // Run Dijkstra from vertex 0.
  // The weight function extracts the edge value.
  dijkstra_shortest_paths(
      g, vertex_id_t<Graph>(0), distance, predecessor,
      [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  for (size_t v = 0; v < distance.size(); ++v)
    std::cout << "0 -> " << v << " : distance = " << distance[v] << "\n";
}
```

Output:

```
0 -> 0 : distance = 0
0 -> 1 : distance = 8
0 -> 2 : distance = 5
0 -> 3 : distance = 9
```

**Key points:**

- `init_shortest_paths(distance, predecessor)` sets distances to infinity and
  predecessors to self-indices.
- The weight function `[](const auto& g, const auto& uv) { return edge_value(g, uv); }`
  extracts the edge value as the weight. If omitted, all edges have weight 1.
- There is also `dijkstra_shortest_distances()` if you don't need predecessor tracking.

---

## 5. Using Edge Lists

An **edge list** is a flat range of sourced edges — any
`std::vector<std::pair<int,int>>` or `std::vector<std::tuple<int,int,double>>`
qualifies automatically.

```cpp
#include <graph/edge_list/edge_list.hpp>
#include <graph/graph.hpp>
#include <vector>
#include <iostream>

int main() {
  // Unweighted edge list as pairs: {source, target}
  std::vector<std::pair<int, int>> edges = {
    {0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 3}
  };

  // Count self-loops using CPOs
  int self_loops = 0;
  for (auto&& uv : edges) {
    if (graph::source_id(edges, uv) == graph::target_id(edges, uv))
      ++self_loops;
  }
  std::cout << "Self-loops: " << self_loops << "\n";  // 1

  // Weighted edge list as 3-tuples: {source, target, weight}
  std::vector<std::tuple<int, int, double>> weighted = {
    {0, 1, 1.5}, {1, 2, 2.5}, {2, 3, 3.0}
  };

  double total = 0.0;
  for (auto&& uv : weighted)
    total += graph::edge_value(weighted, uv);

  std::cout << "Total weight: " << total << "\n";  // 7.0
}
```

Edge lists are useful for algorithms that operate on edges directly
(e.g., Kruskal MST) and for bulk-loading data into adjacency list containers.

---

## 6. Next Steps

- **[Adjacency Lists User Guide](user-guide/adjacency-lists.md)** — concepts, CPOs, descriptors
- **[Edge Lists User Guide](user-guide/edge-lists.md)** — edge patterns, vertex ID types
- **[Views](user-guide/views.md)** — BFS, DFS, topological sort, incidence, neighbors
- **[Container Guide](user-guide/containers.md)** — `dynamic_graph`, `compressed_graph`, `undirected_adjacency_list`
- **[Algorithm Reference](status/implementation_matrix.md#algorithms)** — all 13 algorithms
- **[Migration from v2](migration-from-v2.md)** — what changed from graph-v2
- **[FAQ](FAQ.md)** — common questions
