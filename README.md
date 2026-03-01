<table><tr>
<td><img src="docs/assets/logo_text.svg" width="400" alt="graph-v3 logo"></td>
<td>

# graph-v3

**A modern C++20 graph library — generic, header-only, works with your graphs**

</td>
</tr></table>

<!-- Canonical counts from docs/status/metrics.md — update there first. -->

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-BSL--1.0-blue.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/tests-4405%20passing-brightgreen.svg)](docs/status/metrics.md)

---

## Highlights

- **Header-only** — drop into any CMake project; no compiled components
- **Generic** - Enabled by the use of descriptors.
- **Works with your graphs** — Bring your own graph. `std::vector<std::vector<int>>` and `std::map<int, std::list<std::pair<int,double>>>` are also valid graphs out of the box.
- **13 algorithms** — Dijkstra, Bellman-Ford, BFS, DFS, topological sort, connected components, articulation points, biconnected components, MST, triangle counting, MIS, label propagation, Jaccard coefficient
- **7 lazy views** — vertexlist, edgelist, incidence, neighbors, BFS, DFS, topological sort — all composable with range adaptors
- **Bidirectional edge access** — `in_edges`, `in_degree`, reverse BFS/DFS/topological sort via `in_edge_accessor`
- **Customization Point Objects (CPOs)** — adapt existing data structures without modifying them
- **3 containers, 27 trait combinations** — `dynamic_graph`, `compressed_graph`, `undirected_adjacency_list` with mix-and-match vertex/edge storage
- **4405 tests passing** — comprehensive Catch2 test suite

---

## Quick Example — Dijkstra Shortest Paths

```cpp
#include "graph/graph.hpp"
#include "graph/algorithm/dijkstra_shortest_paths.hpp"
#include <vector>
#include <iostream>

int main() {
  // Weighted directed graph — edges are (target, weight) per vertex
  using G = std::vector<std::vector<std::pair<int,double>>>;
  G g{{{1, 10.0}, {2, 5.0}},   // vertex 0
      {{3, 1.0}},              // vertex 1
      {{1, 3.0}, {3, 9.0}},    // vertex 2
      {{4, 7.0}},              // vertex 3
      {}};                     // vertex 4

  std::vector<double>   distance(graph::num_vertices(g));
  std::vector<uint32_t> predecessor(graph::num_vertices(g));

  graph::init_shortest_paths(distance, predecessor);
  graph::dijkstra_shortest_paths(g, uint32_t(0), distance, predecessor,
        [](const auto& g, const auto& uv) { return graph::edge_value(g, uv); });

  for (size_t v = 0; v < distance.size(); ++v)
    std::cout << "0 -> " << v << " : " << distance[v] << "\n";
}
```

Output:

```
0 -> 0 : 0
0 -> 1 : 8
0 -> 2 : 5
0 -> 3 : 9
0 -> 4 : 16
```

<!--
A full example would be useful, but dijkstra_clrs_example uses a reference version of Dijkstra, not the
library's version.
> A complete, compilable example lives in
> [examples/dijkstra_clrs_example.cpp](examples/dijkstra_clrs_example.cpp).
-->

---

## Two Abstract Data Types

graph-v3 provides two peer abstractions — **adjacency lists** and **edge lists** — each first-class in the library.

| ADT | Namespace | Primary Use |
|-----|-----------|-------------|
| **Adjacency list** | `graph::adj_list` | Fast per-vertex edge traversal; required by most algorithms |
| **Edge list** | `graph::edge_list` | Compact edge-centric storage; ideal for Kruskal MST, bulk loading |

Both share a common descriptor system and customization-point interface.

---

## Feature Overview

| Category | What's Included | Details |
|----------|-----------------|---------|
| **Algorithms** | Dijkstra, Bellman-Ford, BFS, DFS, topological sort, connected components, articulation points, biconnected components, MST, triangle counting, MIS, label propagation, Jaccard | [Algorithm reference](docs/status/implementation_matrix.md#algorithms) |
| **Views** | vertexlist, edgelist, incidence, neighbors, BFS, DFS, topological sort | [View reference](docs/status/implementation_matrix.md#views) |
| **Containers** | `dynamic_graph` (27 trait combos), `compressed_graph` (CSR), `undirected_adjacency_list` | [Container reference](docs/status/implementation_matrix.md#containers) |
| **CPOs** | 19 customization point objects (vertices, edges, target_id, vertex_value, edge_value, …) | [CPO reference](docs/reference/cpo-reference.md) |
| **Concepts** | 9 graph concepts (edge, vertex, adjacency_list, …) | [Concepts reference](docs/reference/concepts.md) |

---

## Supported Compilers

| Compiler | Version | Platform | Status |
|----------|---------|----------|--------|
| GCC      | 13 +    | Linux    | Tested |
| Clang    | 18 +    | Linux, macOS | Tested |
| MSVC     | 2022 +  | Windows  | Tested |

Requires **CMake 3.20+** and a **C++20**-capable toolchain.

---

## Installation

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

### Manual / Subdirectory

```cmake
add_subdirectory(path/to/graph-v3)
target_link_libraries(your_target PRIVATE graph::graph3)
```

### System Install

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

## How graph-v3 Compares to Boost.Graph

| | graph-v3 | Boost.Graph (BGL) |
|---|---|---|
| **Language standard** | C++20 (concepts, ranges) | C++98 / C++03 |
| **Property access** | CPOs (`vertex_value`, `edge_value`) | Property maps |
| **Graph types** | Standard containers or library containers | Custom Boost types |
| **Deployment** | Header-only, single CMake target | Part of Boost (compiled components) |

---

## Documentation

The full documentation hub is at [docs/index.md](docs/index.md).

Quick links:

- [Getting Started](docs/index.md#users)
- [C++ Standardization](docs/standardization.md) — ISO C++ papers and proposal status
- [Migration from graph-v2](docs/migration-from-v2.md)
- [FAQ](docs/FAQ.md)
- [Implementation Matrix](docs/status/implementation_matrix.md)
- [Concepts Reference](docs/reference/concepts.md)
- [CPO Reference](docs/reference/cpo-reference.md)

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build instructions, testing, code style, and PR guidelines.

---

## License

Distributed under the [Boost Software License 1.0](LICENSE).

---

<!-- Status counts pinned to docs/status/metrics.md -->
**Status:** 4405 / 4405 tests passing · 13 algorithms · 7 views · 3 containers · 27 trait combinations · C++20 · BSL-1.0
