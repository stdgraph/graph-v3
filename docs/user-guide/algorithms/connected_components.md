# Connected Components

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Algorithms](#algorithms)
  - [`connected_components` — undirected](#connected_components--undirected)
  - [`kosaraju` — strongly connected components](#kosaraju--strongly-connected-components)
  - [`afforest` — union-find with neighbor sampling](#afforest--union-find-with-neighbor-sampling)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Undirected Connected Components](#example-1-undirected-connected-components)
  - [Strongly Connected Components (Kosaraju)](#example-2-strongly-connected-components-kosaraju)
  - [Union-Find Components (Afforest)](#example-3-union-find-components-afforest)
  - [Undirected Adjacency List](#example-4-undirected-adjacency-list)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

The connected components header provides three algorithms for partitioning a
graph into connected components:

| Algorithm | Use case | Approach |
|-----------|----------|----------|
| `connected_components` | Undirected graphs | DFS-based |
| `kosaraju` | Directed graphs (SCC) | Two DFS passes (requires transpose) |
| `afforest` | Large graphs, parallel-friendly | Union-find with neighbor sampling |

All three fill a `component` array where `component[v]` is the component ID for
vertex v.

## Include

```cpp
#include <graph/algorithm/connected_components.hpp>
```

## Algorithms

### `connected_components` — undirected

```cpp
size_t connected_components(G&& g, Component& component);
```

DFS-based algorithm for undirected graphs. Returns the number of connected
components. Assigns `component[v]` = component ID (0-based).

### `kosaraju` — strongly connected components

```cpp
void kosaraju(G&& g, GT&& g_transpose, Component& component);
```

Kosaraju's two-pass DFS algorithm for directed graphs. Requires both the
original graph `g` and its transpose `g_transpose` (edges reversed). Fills
`component[v]` with the SCC ID.

### `afforest` — union-find with neighbor sampling

```cpp
void afforest(G&& g, Component& component, size_t neighbor_rounds = 2);

void afforest(G&& g, GT&& g_transpose, Component& component,
    size_t neighbor_rounds = 2);
```

Union-find-based algorithm with neighbor sampling for large or parallel-friendly
workloads. The `neighbor_rounds` parameter controls how many initial sampling
rounds are performed before falling back to full edge iteration. Call
`compress(component)` afterwards for canonical (root) component IDs.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `g_transpose` | Transpose graph (for `kosaraju` and `afforest` with transpose) |
| `component` | Random-access range sized to `num_vertices(g)`. Filled with component IDs. |
| `neighbor_rounds` | Number of neighbor-sampling rounds for `afforest` (default: 2) |

## Examples

### Example 1: Undirected Connected Components

Find connected components in an undirected graph simulated with bidirectional
edges.

```cpp
#include <graph/algorithm/connected_components.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Undirected graph (both directions stored):
// Component 0: {0, 1, 2}
// Component 1: {3, 4}
Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {3, 4}, {4, 3}});

std::vector<uint32_t> comp(num_vertices(g));
size_t num_cc = connected_components(g, comp);

// num_cc == 2
// comp[0] == comp[1] == comp[2]  (same component)
// comp[3] == comp[4]             (same component)
```

### Example 2: Strongly Connected Components (Kosaraju)

Find SCCs in a directed graph using Kosaraju's algorithm.

```cpp
// Directed graph with 2 SCCs:
//   SCC 0: {0, 1, 2} (cycle: 0→1→2→0)
//   SCC 1: {3}
Graph g({{0, 1}, {1, 2}, {2, 0}, {2, 3}});

// Transpose: reverse all edges
Graph g_t({{1, 0}, {2, 1}, {0, 2}, {3, 2}});

std::vector<uint32_t> comp(num_vertices(g));
kosaraju(g, g_t, comp);

// comp[0] == comp[1] == comp[2]  (cycle forms an SCC)
// comp[3] != comp[0]             (3 is its own SCC)
```

### Example 3: Union-Find Components (Afforest)

Afforest is suitable for large graphs and parallel-friendly workloads.

```cpp
std::vector<uint32_t> comp(num_vertices(g));
afforest(g, comp, 2);  // 2 neighbor-sampling rounds (default)

// comp[v] may not be canonical — compress for root IDs
// Component IDs are valid for equality testing without compression
```

### Example 4: Undirected Adjacency List

With `undirected_adjacency_list`, each logical edge is stored once — no need for
bidirectional edge pairs.

```cpp
#include <graph/container/undirected_adjacency_list.hpp>

undirected_adjacency_list<int, int> g({{0, 1, 1}, {1, 2, 1}, {3, 4, 1}});

std::vector<uint32_t> comp(num_vertices(g));
size_t num_cc = connected_components(g, comp);

// num_cc == 2
// Same results as vov_void with bidirectional edges
```

## Complexity

| Algorithm | Time | Space |
|-----------|------|-------|
| `connected_components` | O(V + E) | O(V) |
| `kosaraju` | O(V + E) | O(V) |
| `afforest` | O(V + E) | O(V) |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- `component` must be sized to `num_vertices(g)`.
- For `connected_components`: undirected graphs must store both directions of
  each edge (or use `undirected_adjacency_list`).
- For `kosaraju`: the transpose graph must contain all edges reversed.

## See Also

- [Biconnected Components](biconnected_components.md) — maximal 2-connected subgraphs
- [Articulation Points](articulation_points.md) — cut vertices
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_connected_components.cpp](../../../tests/algorithms/test_connected_components.cpp) — test suite
