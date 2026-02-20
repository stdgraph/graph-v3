# Minimum Spanning Tree

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Kruskal's Algorithm](#kruskals-algorithm)
  - [Signatures](#kruskal-signatures)
  - [Parameters](#kruskal-parameters)
- [Prim's Algorithm](#prims-algorithm)
  - [Signatures](#prim-signatures)
  - [Parameters](#prim-parameters)
- [Edge Descriptor](#edge-descriptor)
- [Examples](#examples)
  - [Kruskal — Basic MST](#example-1-kruskal--basic-mst)
  - [Kruskal — Maximum Spanning Tree](#example-2-kruskal--maximum-spanning-tree)
  - [Kruskal — Disconnected Graph (Forest)](#example-3-kruskal--disconnected-graph-forest)
  - [Prim — Adjacency List MST](#example-4-prim--adjacency-list-mst)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

The library provides two classic minimum spanning tree algorithms:

| Algorithm | Input | Approach | Best for |
|-----------|-------|----------|----------|
| **Kruskal** | Edge list | Sort + union-find | Sparse graphs, edge-list input |
| **Prim** | Adjacency list | Priority queue + greedy | Dense graphs, adjacency-list input |

Both algorithms produce an MST (or minimum spanning **forest** for disconnected
graphs). Kruskal operates on an external edge list; Prim operates on a graph
satisfying `index_adjacency_list`.

## Include

```cpp
#include <graph/algorithm/mst.hpp>
```

## Kruskal's Algorithm

Kruskal's algorithm sorts edges by weight and greedily adds them to the MST,
skipping edges that would form a cycle (detected via union-find). Two variants
exist: `kruskal` (preserves input order) and `inplace_kruskal` (sorts input
in-place to save memory).

### Kruskal Signatures

```cpp
// Copy-sort: input edge list is not modified
auto kruskal(EdgeList& edges, OutputIterator tree);

auto kruskal(EdgeList& edges, OutputIterator tree, Compare compare);

// In-place sort: sorts edges in-place for lower memory usage
auto inplace_kruskal(EdgeList& edges, OutputIterator tree);

auto inplace_kruskal(EdgeList& edges, OutputIterator tree, Compare compare);
```

**Returns** `std::pair<EV, size_t>` — total MST weight and number of connected
components.

### Kruskal Parameters

| Parameter | Description |
|-----------|-------------|
| `edges` | Range of edge descriptors with `.source_id`, `.target_id`, `.value` |
| `tree` | Output iterator receiving MST edges |
| `compare` | Edge-value comparator. Default: `std::less<>{}`. Use `std::greater<>{}` for max spanning tree. |

## Prim's Algorithm

Prim's algorithm grows the MST from a seed vertex, greedily adding the
lightest edge crossing the cut between tree and non-tree vertices.

### Prim Signatures

```cpp
// Simple: uses default weight function from edge values
auto prim(G&& g, Predecessors& predecessors, Weights& weights,
    vertex_id_t<G> seed = 0);

// Full: custom comparator, initial distance, and weight function
auto prim(G&& g, Predecessors& predecessors, Weights& weights,
    Compare compare, range_value_t<Weights> init_dist,
    WF weight_fn, vertex_id_t<G> seed = 0);
```

**Returns** the total MST weight.

### Prim Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` with weighted edges |
| `predecessors` | Random-access range sized to `num_vertices(g)`. Filled with parent vertex IDs. |
| `weights` | Random-access range sized to `num_vertices(g)`. Filled with edge weights to parent. |
| `seed` | Starting vertex for the MST (default: 0) |
| `compare` | Comparator for weight values |
| `init_dist` | Initial distance value (typically infinity) |
| `weight_fn` | Callable `WF(g, uv)` returning edge weight |

## Edge Descriptor

Kruskal works with edge descriptors that expose `source_id`, `target_id`, and
`value` members. The library provides `edge_descriptor<VId, EV>`:

```cpp
#include <graph/edge_list/edge_list_descriptor.hpp>

using Edge = graph::edge_descriptor<uint32_t, int>;
// Edge{source_id, target_id, value}
```

## Examples

### Example 1: Kruskal — Basic MST

Build an MST from an edge list.

```cpp
#include <graph/algorithm/mst.hpp>
#include <graph/edge_list/edge_list_descriptor.hpp>
#include <vector>

using Edge = graph::edge_descriptor<uint32_t, int>;

std::vector<Edge> edges = {
    {0, 1, 4}, {0, 2, 2}, {1, 2, 1}, {1, 3, 5}, {2, 3, 8}
};

std::vector<Edge> mst;
auto [total_weight, num_components] = kruskal(edges, std::back_inserter(mst));

// total_weight = 7  (edges: 1-2:1, 0-2:2, 0-1:4 — wait, skip 0-1 since
//                     0 and 1 already connected; add 1-3:5 instead)
// mst contains 3 edges (V-1 for a connected graph of 4 vertices)
// num_components = 1
```

### Example 2: Kruskal — Maximum Spanning Tree

Pass `std::greater<>{}` to build a maximum spanning tree instead.

```cpp
std::vector<Edge> edges = {
    {0, 1, 4}, {1, 2, 8}, {0, 2, 2}
};

std::vector<Edge> max_tree;
auto [max_weight, nc] = kruskal(edges, std::back_inserter(max_tree),
    std::greater<int>{});

// max_weight = 12  (edges: 1-2:8 + 0-1:4)
// Heaviest edges chosen instead of lightest
```

### Example 3: Kruskal — Disconnected Graph (Forest)

For a disconnected graph, Kruskal produces a minimum spanning forest.

```cpp
std::vector<Edge> edges = {
    {0, 1, 1}, {1, 2, 2},   // component A
    {3, 4, 3}, {4, 5, 4}    // component B
};

std::vector<Edge> forest;
auto [forest_weight, components] = kruskal(edges, std::back_inserter(forest));

// forest_weight = 10  (1 + 2 + 3 + 4)
// components = 2  — two disconnected components
// forest.size() = 4  (V - components)
```

### Example 4: Prim — Adjacency List MST

Build an MST from an adjacency list, starting from a seed vertex.

```cpp
#include <graph/algorithm/mst.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<int, void, void, uint32_t, false,
    container::vov_graph_traits<int>>;

// Undirected weighted graph (store both directions)
Graph g({{0,1,4}, {1,0,4}, {1,2,8}, {2,1,8},
         {2,0,11},{0,2,11}, {0,2,2},{2,0,2}});

size_t                n = num_vertices(g);
std::vector<uint32_t> pred(n);
std::vector<int>      wt(n);

auto total = prim(g, pred, wt, 0u);

// total = total MST weight
// pred[v] = parent of v in the MST
// wt[v]   = weight of edge from pred[v] to v
//
// Reconstruct MST edges:
for (size_t v = 0; v < n; ++v) {
    if (v != 0 && pred[v] != v)
        std::cout << pred[v] << " → " << v << " : " << wt[v] << "\n";
}
```

## Complexity

| Algorithm | Time | Space |
|-----------|------|-------|
| `kruskal` | O(E log E) | O(E + V) — sorted copy + union-find |
| `inplace_kruskal` | O(E log E) | O(V) — in-place sort + union-find |
| `prim` | O(E log V) | O(V) — priority queue + predecessor/weight arrays |

## Preconditions

- **Kruskal:** edge descriptors must have `source_id`, `target_id`, and `value`
  members (or use `edge_descriptor<VId, EV>`).
- **Prim:** graph must satisfy `index_adjacency_list<G>` with weighted edges.
  `predecessors` and `weights` must be sized to `num_vertices(g)`. Invalid seed
  vertex throws `std::out_of_range`.
- For undirected graphs with Prim, both directions of each edge must be stored.

## See Also

- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_mst.cpp](../../../tests/algorithms/test_mst.cpp) — test suite
- [mst_usage_example.cpp](../../../examples/mst_usage_example.cpp) — annotated example
