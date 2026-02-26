# Minimum Spanning Tree

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [When to Use](#when-to-use)
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
  - [In-Place Kruskal — Lower Memory](#example-4-in-place-kruskal--lower-memory)
  - [Prim — Adjacency List MST](#example-5-prim--adjacency-list-mst)
  - [Prim — Custom Weight Function](#example-6-prim--custom-weight-function)
  - [Cross-Validation: Kruskal vs. Prim](#example-7-cross-validation-kruskal-vs-prim)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [Notes](#notes)
- [See Also](#see-also)

## Overview

The library provides two classic minimum spanning tree algorithms:

| Algorithm | Input | Approach | Best for |
|-----------|-------|----------|----------|
| **Kruskal** | Edge list | Sort + union-find | Sparse graphs, edge-list input |
| **Prim** | Adjacency list | Priority queue + greedy | Dense graphs, adjacency-list input |

Both algorithms produce an MST (or minimum spanning **forest** for disconnected
graphs). Kruskal operates on an external edge list; Prim operates on a graph
satisfying `index_adjacency_list<G>` — vertices are stored in a contiguous,
integer-indexed random-access range.

## When to Use

**Kruskal** is the natural choice when:

- You already have an edge list (e.g., from a file or external source).
- The graph is sparse (E ≈ V).
- You want a **maximum** spanning tree — just swap the comparator.
- You need easy access to the MST edges in a separate container.

**Prim** is preferred when:

- The graph is already stored as an adjacency list.
- The graph is dense (E ≈ V²) — Prim can be faster due to priority-queue
  locality.
- You want predecessor/weight arrays for path reconstruction.

**`inplace_kruskal` vs `kruskal`:**

- `inplace_kruskal` sorts the input edge list **in place**, avoiding the O(E)
  memory cost of copying and sorting. Use it when you don't need to preserve
  the original edge order.

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
    const vertex_id_t<G>& seed = 0);

// Full: custom comparator, initial distance, and weight function
auto prim(G&& g, Predecessors& predecessors, Weights& weights,
    Compare compare, range_value_t<Weights> init_dist,
    WF weight_fn, const vertex_id_t<G>& seed = 0);
```

**Returns** the total MST weight.

### Prim Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` with weighted edges |
| `predecessors` | Random-access range sized to `num_vertices(g)`. Filled with parent vertex IDs. |
| `weights` | Random-access range sized to `num_vertices(g)`. Filled with edge weights to parent. |
| `seed` | Starting vertex for the MST (default: 0) |
| `compare` | Comparator for weight values (default: `std::less<>{}`) |
| `init_dist` | Initial distance value (typically `std::numeric_limits<EV>::max()`) |
| `weight_fn` | Callable `WF(g, uv)` returning edge weight. Default: `edge_value(g, uv)`. |

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

Build an MST from an edge list. Kruskal preserves the original edge list.

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

// Kruskal picks edges in weight order:
//   1-2: weight 1 ✓
//   0-2: weight 2 ✓
//   0-1: weight 4 — skip (0 and 1 already connected via 2)
//   1-3: weight 5 ✓
//   2-3: weight 8 — skip (already connected)
//
// total_weight = 8   (1 + 2 + 5)
// mst.size() = 3     (V - 1 for a connected graph)
// num_components = 1
```

### Example 2: Kruskal — Maximum Spanning Tree

Pass `std::greater<>{}` to build a maximum spanning tree — useful for
finding bottleneck paths or maximum-weight forests.

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
The returned component count tells you how many disconnected pieces exist.

```cpp
std::vector<Edge> edges = {
    {0, 1, 1}, {1, 2, 2},   // component A
    {3, 4, 3}, {4, 5, 4}    // component B
};

std::vector<Edge> forest;
auto [forest_weight, components] = kruskal(edges, std::back_inserter(forest));

// forest_weight = 10  (1 + 2 + 3 + 4)
// components = 2  — two disconnected components
// forest.size() = 4  (V - components = 6 - 2)
```

### Example 4: In-Place Kruskal — Lower Memory

`inplace_kruskal` sorts the edge list **in place**, avoiding the O(E) copy.
Use this when you don't need the original edge order.

```cpp
std::vector<Edge> edges = {
    {0, 1, 4}, {0, 2, 2}, {1, 2, 1}, {1, 3, 5}, {2, 3, 8}
};

// WARNING: edges will be reordered after this call
std::vector<Edge> mst;
auto [total_weight, nc] = inplace_kruskal(edges, std::back_inserter(mst));

// Same result as kruskal(), but edges is now sorted by weight
// total_weight = 8, mst.size() = 3
//
// edges[0].value == 1, edges[1].value == 2, ...  (sorted)
```

### Example 5: Prim — Adjacency List MST

Build an MST from an adjacency list, starting from a seed vertex.
Prim fills predecessor and weight arrays for path reconstruction.

```cpp
#include <graph/algorithm/mst.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<int, void, void, uint32_t, false,
    container::vov_graph_traits<int>>;

// Undirected weighted graph (store both directions)
Graph g({{0,1,4}, {1,0,4}, {1,2,8}, {2,1,8},
         {0,2,2}, {2,0,2}});

size_t                n = num_vertices(g);
std::vector<uint32_t> pred(n);
std::vector<int>      wt(n);

auto total = prim(g, pred, wt, 0u);

// total = 6  (edges: 0-2:2 + 0-1:4)
// pred[0] = 0  (seed — self)
// pred[1] = 0  (1 reached from 0, weight 4)
// pred[2] = 0  (2 reached from 0, weight 2)
//
// Reconstruct MST edges:
for (size_t v = 0; v < n; ++v) {
    if (v != 0 && pred[v] != v)
        std::cout << pred[v] << " → " << v << " : " << wt[v] << "\n";
}
```

### Example 6: Prim — Custom Weight Function

Use a custom weight function to transform edge weights, e.g. negate them
to compute a maximum spanning tree with Prim.

```cpp
size_t                n = num_vertices(g);
std::vector<uint32_t> pred(n);
std::vector<int>      wt(n);

// Negate weights to find maximum spanning tree
auto neg_weight = [](const auto& g_ref, const auto& uv) {
    return -edge_value(g_ref, uv);
};

auto total = prim(g, pred, wt,
    std::less<int>{},             // comparator (still min-heap)
    std::numeric_limits<int>::max(), // init distance
    neg_weight,                      // negated weight function
    0u);                             // seed vertex

// |total| is the maximum spanning tree weight (negated)
```

### Example 7: Cross-Validation: Kruskal vs. Prim

Both algorithms should produce the same total MST weight for the same graph.
This pattern is useful in testing.

```cpp
// Build edge list for Kruskal and adjacency list for Prim from the same data
using Edge = graph::edge_descriptor<uint32_t, int>;
std::vector<Edge> edges = {
    {0, 1, 4}, {1, 2, 2}, {0, 2, 5}
};

// Kruskal
std::vector<Edge> kruskal_mst;
auto [kw, kc] = kruskal(edges, std::back_inserter(kruskal_mst));

// Prim (same graph as adjacency list, both directions)
using Graph = container::dynamic_graph<int, void, void, uint32_t, false,
    container::vov_graph_traits<int>>;
Graph g({{0,1,4},{1,0,4}, {1,2,2},{2,1,2}, {0,2,5},{2,0,5}});

size_t                n = num_vertices(g);
std::vector<uint32_t> pred(n);
std::vector<int>      wt(n);
auto pw = prim(g, pred, wt, 0u);

assert(kw == pw);  // Both produce the same total MST weight
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

## Notes

- `inplace_kruskal` **modifies the input edge list** — it is sorted by weight
  after the call returns. Use `kruskal` if you need the original order.
- The disjoint-set data structure used internally by Kruskal is a public utility
  available in the MST header.
- For a disconnected graph, Kruskal returns the number of components. Prim only
  builds a tree from the seed's component; unreachable vertices retain their
  initial predecessor/weight values.

## See Also

- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_mst.cpp](../../../tests/algorithms/test_mst.cpp) — test suite
- [mst_usage_example.cpp](../../../examples/mst_usage_example.cpp) — annotated example
