<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Graph Generators

> Create synthetic graphs for testing, benchmarking, and prototyping.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md)

---

## Table of Contents

- [Overview](#overview)
- [Header](#header)
- [Generators](#generators)
  - [path_graph](#path_graph)
  - [grid_graph](#grid_graph)
  - [erdos_renyi_graph](#erdos_renyi_graph)
  - [barabasi_albert_graph](#barabasi_albert_graph)
- [Example: Building and Querying a Generated Graph](#example)

---

## Overview

The `graph::generators` namespace provides functions that return edge lists suitable for loading into any graph container. Each generator produces a `std::vector<graph::copyable_edge_t<VId, EV>>` that can be passed to `dynamic_graph::load_edges()` or used to construct a `compressed_graph`.

All generators are header-only and require no external dependencies.

---

## Header

```cpp
#include <graph/generators.hpp>   // umbrella — includes all generators

// Or include individually:
#include <graph/generators/path.hpp>
#include <graph/generators/grid.hpp>
#include <graph/generators/erdos_renyi.hpp>
#include <graph/generators/barabasi_albert.hpp>
```

---

## Generators

### `path_graph`

Creates a linear path graph: `0 → 1 → 2 → … → n-1`.

```cpp
template <std::unsigned_integral VId = uint32_t>
auto path_graph(VId num_vertices) -> std::vector<copyable_edge_t<VId, void>>;
```

| Parameter | Description |
|-----------|-------------|
| `num_vertices` | Number of vertices in the path |

**Returns:** `n-1` directed edges forming a simple path.

```cpp
auto edges = graph::generators::path_graph(5u);
// edges: {0→1, 1→2, 2→3, 3→4}
```

---

### `grid_graph`

Creates a 2D grid (lattice) graph with `rows × cols` vertices.

```cpp
template <std::unsigned_integral VId = uint32_t>
auto grid_graph(VId rows, VId cols) -> std::vector<copyable_edge_t<VId, void>>;
```

| Parameter | Description |
|-----------|-------------|
| `rows` | Number of rows |
| `cols` | Number of columns |

**Returns:** Directed edges connecting each vertex to its right and bottom neighbors. Vertex IDs are laid out row-major: vertex at `(r, c)` has ID `r * cols + c`.

```cpp
auto edges = graph::generators::grid_graph(3u, 4u);
// 12 vertices in a 3×4 grid, edges go right and down
```

---

### `erdos_renyi_graph`

Generates a random graph using the Erdős–Rényi G(n, p) model.

```cpp
template <std::unsigned_integral VId = uint32_t>
auto erdos_renyi_graph(VId num_vertices, double edge_probability,
                       uint64_t seed = 42)
    -> std::vector<copyable_edge_t<VId, void>>;
```

| Parameter | Description |
|-----------|-------------|
| `num_vertices` | Number of vertices |
| `edge_probability` | Probability `p` that each directed edge exists (0.0–1.0) |
| `seed` | Random seed for reproducibility |

**Returns:** A random directed graph where each potential edge `(u, v)` with `u ≠ v` exists independently with probability `p`.

```cpp
auto edges = graph::generators::erdos_renyi_graph(100u, 0.05);
// ~495 edges on average (100*99*0.05)
```

---

### `barabasi_albert_graph`

Generates a scale-free graph using the Barabási–Albert preferential attachment model.

```cpp
template <std::unsigned_integral VId = uint32_t>
auto barabasi_albert_graph(VId num_vertices, VId edges_per_vertex,
                           uint64_t seed = 42)
    -> std::vector<copyable_edge_t<VId, void>>;
```

| Parameter | Description |
|-----------|-------------|
| `num_vertices` | Total number of vertices |
| `edges_per_vertex` | Number of edges each new vertex attaches to existing vertices (`m`) |
| `seed` | Random seed for reproducibility |

**Returns:** A directed graph exhibiting power-law degree distribution. The initial seed graph is a complete graph on `edges_per_vertex + 1` vertices.

```cpp
auto edges = graph::generators::barabasi_albert_graph(1000u, 3u);
// ~3000 edges, scale-free topology
```

---

## Example

```cpp
#include <graph/generators.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/graph.hpp>
#include <iostream>

int main() {
  // Generate a 10×10 grid graph
  auto edge_list = graph::generators::grid_graph(10u, 10u);

  // Load into a dynamic_graph
  using G = graph::container::dynamic_graph<void, void, void, uint32_t, false,
      graph::container::vov_graph_traits<void, void, void, uint32_t, false>>;
  G g;
  g.load_edges(edge_list, std::identity{}, uint32_t{100});

  std::cout << "Vertices: " << graph::num_vertices(g) << "\n";
  // Output: Vertices: 100

  // Count total edges
  size_t edge_count = 0;
  for (auto u : graph::vertices(g))
    for ([[maybe_unused]] auto uv : graph::edges(g, u))
      ++edge_count;
  std::cout << "Edges: " << edge_count << "\n";
  // Output: Edges: 180
}
```

---

> [← Back to Documentation Index](../index.md)
