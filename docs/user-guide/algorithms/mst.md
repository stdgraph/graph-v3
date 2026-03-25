<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Minimum Spanning Tree

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
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
- [Supported Graph Properties](#supported-graph-properties)
- [Examples](#examples)
  - [Kruskal — Basic MST](#example-1-kruskal--basic-mst)
  - [Kruskal — Maximum Spanning Tree](#example-2-kruskal--maximum-spanning-tree)
  - [Kruskal — Disconnected Graph (Forest)](#example-3-kruskal--disconnected-graph-forest)
  - [In-Place Kruskal — Lower Memory](#example-4-in-place-kruskal--lower-memory)
  - [Prim — Adjacency List MST](#example-5-prim--adjacency-list-mst)
  - [Prim — Custom Weight Function](#example-6-prim--custom-weight-function)
  - [Cross-Validation: Kruskal vs. Prim](#example-7-cross-validation-kruskal-vs-prim)
- [Mandates](#mandates)
- [Preconditions](#preconditions)
- [Effects](#effects)
- [Returns](#returns)
- [Throws](#throws)
- [Complexity](#complexity)
- [Remarks](#remarks)
- [See Also](#see-also)

## Overview

The library provides two classic minimum spanning tree algorithms:

| Algorithm | Input | Approach | Best for |
|-----------|-------|----------|----------|
| **Kruskal** | Edge list | Sort + union-find | Sparse graphs, edge-list input |
| **Prim** | Adjacency list | Priority queue + greedy | Dense graphs, adjacency-list input |

Both algorithms produce an MST (or minimum spanning **forest** for disconnected
graphs). Kruskal operates on an external edge list; Prim operates on a graph
satisfying `adjacency_list<G>` — both index-based (contiguous integer-indexed)
and map-based (sparse vertex ID) graphs are supported.

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
// weight_fn and compare are optional
auto prim(G&& g,
    const vertex_id_t<G>& seed,
    WeightFn&& weight, PredecessorFn&& predecessor,
    WF weight_fn = edge_value(g, uv),
    Compare compare = std::less<>{});
```

**Returns** the total MST weight.

### Prim Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `adjacency_list` with weighted edges |
| `seed` | Starting vertex for MST growth. Must be a valid vertex ID. |
| `weight` | Callable `(const G&, vertex_id_t<G>) -> W&` returning a mutable reference to the per-vertex edge weight. For containers: wrap with `container_value_fn(wt)`. Must satisfy `distance_fn_for<WeightFn, G>`. |
| `predecessor` | Callable `(const G&, vertex_id_t<G>) -> P&` returning a mutable reference to the per-vertex predecessor. For containers: wrap with `container_value_fn(pred)`. Must satisfy `predecessor_fn_for<PredecessorFn, G>`. |
| `weight_fn` | Callable `WF(g, uv)` returning edge weight. Default: `edge_value(g, uv)`. |
| `compare` | Comparator for weight values (default: `std::less<>{}`) |

## Edge Descriptor

Kruskal works with edge descriptors that expose `source_id`, `target_id`, and
`value` members. The library provides `edge_descriptor<VId, EV>`:

```cpp
#include <graph/edge_list/edge_list_descriptor.hpp>

using Edge = graph::edge_descriptor<uint32_t, int>;
// Edge{source_id, target_id, value}
```

## Supported Graph Properties

**Directedness:**
- ✅ Undirected graphs (primary use case for MST)
- ⚠️ Directed graphs — Kruskal treats edges as undirected; Prim requires
  both directions stored

**Edge Properties:**
- ✅ Weighted edges (required — weights determine MST)
- ❌ Unweighted edges — MST is trivial (any spanning tree is minimal)
- ✅ Multi-edges (lightest edge between each pair is selected)
- ✅ Self-loops (ignored — cannot be part of a spanning tree)

**Graph Structure:**
- ✅ Connected graphs (produces MST)
- ✅ Disconnected graphs (Kruskal produces minimum spanning forest;
  Prim only spans seed's component)
- ✅ Empty graphs (no-op)

**Container Requirements:**
- **Kruskal:** edge descriptors with `source_id`, `target_id`, `value` members
- **Prim:** `adjacency_list<G>` with weighted edges;
  `weight` must satisfy `distance_fn_for<WeightFn, G>`;
  `predecessor` must satisfy `predecessor_fn_for<PredecessorFn, G>`;
  use `container_value_fn(vec)` to adapt a `std::vector`

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

init_shortest_paths(g, wt, pred);
auto total = prim(g, 0u, container_value_fn(wt), container_value_fn(pred));

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

init_shortest_paths(g, wt, pred);
auto total = prim(g, 0u, container_value_fn(wt), container_value_fn(pred),
    neg_weight,        // negated weight function
    std::less<int>{}); // comparator (still min-heap)

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
init_shortest_paths(g, wt, pred);
auto pw = prim(g, 0u, container_value_fn(wt), container_value_fn(pred));

assert(kw == pw);  // Both produce the same total MST weight
```

## Mandates

- **Kruskal:** edge descriptors must have `source_id`, `target_id`, and `value`
  members (or use `edge_descriptor<VId, EV>`)
- **Prim:** `G` must satisfy `adjacency_list<G>`; `WeightFn` must satisfy
  `distance_fn_for<WeightFn, G>`; `PredecessorFn` must satisfy
  `predecessor_fn_for<PredecessorFn, G>`; `WF` must satisfy
  `basic_edge_weight_function`

## Preconditions

- **Prim:** `weight(g, uid)` and `predecessor(g, uid)` must be valid for all vertex IDs.
  Call `init_shortest_paths(wt, pred)` on the underlying containers first.
  Invalid seed vertex throws `std::out_of_range`.
- For undirected graphs with Prim, both directions of each edge must be stored.

## Effects

- **Kruskal:** writes MST edges to the output iterator; `inplace_kruskal`
  sorts the input edge list in-place
- **Prim:** writes weight and predecessor values via `weight(g, v)` and
  `predecessor(g, v)`; does not modify the graph `g`

## Returns

- **Kruskal / inplace_kruskal:** `std::pair<EV, size_t>` — total MST weight
  and number of connected components
- **Prim:** total MST weight (type determined by weight function)

## Throws

- **Prim:** `std::out_of_range` if the seed vertex is invalid
- `std::bad_alloc` if internal allocations fail
- Exception guarantee: Basic. Graph `g` remains unchanged; output may be partial.

## Complexity

| Algorithm | Time | Space |
|-----------|------|-------|
| `kruskal` | O(E log E) | O(E + V) — sorted copy + union-find |
| `inplace_kruskal` | O(E log E) | O(V) — in-place sort + union-find |
| `prim` | O(E log V) | O(V) — priority queue + predecessor/weight arrays |

## Remarks

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
