<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Jaccard Coefficient

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Supported Graph Properties](#supported-graph-properties)
- [Examples](#examples)
  - [Collecting Similarity Scores](#example-1-collecting-similarity-scores)
  - [Triangle — Shared Neighbors](#example-2-triangle--shared-neighbors)
  - [Star Graph — No Shared Neighbors](#example-3-star-graph--no-shared-neighbors)
  - [Threshold Filtering — Link Prediction](#example-4-threshold-filtering--link-prediction)
  - [Most Similar Vertex Pairs](#example-5-most-similar-vertex-pairs)
- [Mandates](#mandates)
- [Preconditions](#preconditions)
- [Effects](#effects)
- [Throws](#throws)
- [Complexity](#complexity)
- [Remarks](#remarks)
- [See Also](#see-also)

## Overview

Computes the **Jaccard similarity coefficient** for every directed edge in the
graph. The graph must satisfy `adjacency_list<G>` — both index-based
(contiguous integer-indexed) and map-based (sparse vertex ID) graphs are
supported.

For an edge (u, v), the coefficient is:

$$J(u, v) = \frac{|N(u) \cap N(v)|}{|N(u) \cup N(v)|}$$

where $N(u)$ is the neighbor set of vertex $u$. The coefficient ranges from
0.0 (no shared neighbors) to 1.0 (identical neighbor sets).

The algorithm is **callback-driven**: it invokes a user-provided callable for
each directed edge with its Jaccard coefficient. Self-loops are skipped.

**How it works:** The algorithm precomputes a neighbor set (`unordered_set`) for
each vertex, then iterates over all directed edges. For each edge (u, v), it
computes the intersection and union sizes from the precomputed sets.

## When to Use

- **Link prediction** — edges with high Jaccard coefficients indicate strong
  structural similarity; non-edges between vertices with high J scores are
  prime candidates for new connections.
- **Recommendation systems** — users (vertices) with similar neighborhoods
  (shared connections) receive similar recommendations.
- **Duplicate detection** — vertices with J ≈ 1.0 have nearly identical
  neighbor sets and may represent duplicates.
- **Community analysis** — clusters of vertices with high pairwise Jaccard
  scores form tightly integrated communities.

**Consider alternatives when:**

- You need community labels directly → use
  [label_propagation](label_propagation.md).
- You need to compare vertices that are not currently connected — this algorithm
  only computes J for existing edges.

## Include

```cpp
#include <graph/algorithm/jaccard.hpp>
```

## Signature

```cpp
void jaccard_coefficient(G&& g, OutOp out);
```

Where `out` is a callable with signature:

```cpp
void out(vertex_id_t<G> uid, vertex_id_t<G> vid,
    edge_reference_t<G> uv, double coefficient);
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `adjacency_list` |
| `out` | Callback invoked for each directed edge (u, v) with the Jaccard coefficient. Self-loops are skipped. |

## Supported Graph Properties

**Directedness:**
- ✅ Directed graphs (coefficient computed per directed edge)
- ✅ Undirected graphs (with bidirectional storage; J(u,v) == J(v,u))

**Edge Properties:**
- ✅ Unweighted edges
- ✅ Weighted edges (weights ignored — only adjacency matters)
- ✅ Multi-edges (each directed edge triggers the callback)
- ❌ Self-loops — skipped (not passed to callback, excluded from neighbor sets)

**Graph Structure:**
- ✅ Connected graphs
- ✅ Disconnected graphs
- ✅ Empty graphs (no-op — no edges to process)

**Container Requirements:**
- Required: `adjacency_list<G>`

## Examples

### Example 1: Collecting Similarity Scores

Collect all Jaccard coefficients into a map.

```cpp
#include <graph/algorithm/jaccard.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <map>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Graph: 0-1, 0-2, 1-2, 1-3 (bidirectional for undirected)
Graph g({{0,1},{1,0}, {0,2},{2,0}, {1,2},{2,1}, {1,3},{3,1}});

std::map<std::pair<uint32_t, uint32_t>, double> scores;
jaccard_coefficient(g, [&](auto uid, auto vid, auto& uv, double j) {
    scores[{uid, vid}] = j;
});

// scores[{0,1}]:
//   N(0) = {1, 2}, N(1) = {0, 2, 3}
//   intersection = {2}, union = {0, 1, 2, 3}
//   J = 1/4 = 0.25
//
// For undirected graphs, J(u,v) == J(v,u)
```

### Example 2: Triangle — Shared Neighbors

In a triangle (K3), each pair of vertices shares exactly one neighbor.

```cpp
// Triangle: 0-1-2
Graph g({{0,1},{1,0}, {0,2},{2,0}, {1,2},{2,1}});

jaccard_coefficient(g, [](auto uid, auto vid, auto& uv, double j) {
    std::cout << uid << " → " << vid << " : J = " << j << "\n";
});

// Every edge has J = 1/3:
//   N(0) = {1, 2}, N(1) = {0, 2}
//   intersection = {2}, union = {0, 1, 2} → 1/3
//
// 6 directed edges, each with coefficient ≈ 0.333
```

### Example 3: Star Graph — No Shared Neighbors

In a star graph, no edge endpoint pair shares a neighbor.

```cpp
// Star: center 0, leaves 1, 2, 3
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}});

jaccard_coefficient(star, [](auto uid, auto vid, auto& uv, double j) {
    std::cout << uid << " → " << vid << " : J = " << j << "\n";
});
// All coefficients are 0.0:
//   N(0) = {1, 2, 3}, N(1) = {0}
//   intersection = {} → J = 0
// Leaf-to-center: same — no shared neighbors
```

### Example 4: Threshold Filtering — Link Prediction

Filter edges to find the most strongly similar pairs, e.g., for link
prediction or community boundary identification.

```cpp
// More complex graph: two near-cliques connected by a bridge
//   Clique A: 0-1-2-3 (K4 minus one edge)
//   Clique B: 4-5-6
//   Bridge: 3-4
Graph g({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
         {1,2},{2,1}, {1,3},{3,1},   // 2-3 missing from K4
         {3,4},{4,3},                  // bridge
         {4,5},{5,4}, {4,6},{6,4}, {5,6},{6,5}});

double threshold = 0.4;
std::vector<std::pair<uint32_t, uint32_t>> strong_edges;

jaccard_coefficient(g, [&](auto uid, auto vid, auto& uv, double j) {
    if (j >= threshold)
        strong_edges.emplace_back(uid, vid);
});

// Intra-clique edges have high J (many shared neighbors)
// Bridge edge 3-4 has low J (few shared neighbors)
// strong_edges contains the intra-clique edges — the "core" structure
```

### Example 5: Most Similar Vertex Pairs

Find the pair of connected vertices with the highest Jaccard coefficient.

```cpp
uint32_t best_u = 0, best_v = 0;
double best_j = -1.0;

jaccard_coefficient(g, [&](auto uid, auto vid, auto& uv, double j) {
    if (j > best_j) {
        best_j = j;
        best_u = uid;
        best_v = vid;
    }
});

std::cout << "Most similar: " << best_u << " - " << best_v
          << " (J = " << best_j << ")\n";
// In a community structure, the most similar pair is typically deep
// inside a community, far from the boundary.
```

## Mandates

- `G` must satisfy `adjacency_list<G>`
- `OutOp` must be callable with `(vertex_id_t<G>, vertex_id_t<G>, edge_reference_t<G>, double)`

## Preconditions

- Self-loops are skipped (not passed to the callback)
- The callback is invoked once per **directed** edge — for undirected graphs
  with bidirectional storage, expect two calls per logical edge

## Effects

- Invokes `out(uid, vid, uv, coefficient)` for each non-self-loop directed edge
- Does not modify the graph `g`
- Precomputes neighbor sets (`unordered_set` per vertex) internally

## Throws

- `std::bad_alloc` if internal allocations fail (neighbor set construction)
- Exception guarantee: Basic. Graph `g` remains unchanged; callback may have been
  invoked for a subset of edges.

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V · d_max + E · d_min) where d_max = max degree, d_min = min degree of edge endpoints |
| Space | O(V + E) for precomputed neighbor sets (`unordered_set` per vertex) |

The `unordered_set` construction is O(V + E) total. Each edge intersection is
proportional to the size of the smaller neighbor set.

## Remarks

- **Self-loops are excluded** from the neighbor sets. A vertex with a self-loop
  does not count itself as a neighbor for Jaccard computation (unlike
  [label_propagation](label_propagation.md), which does count self-loops).
- If both vertices have empty neighbor sets (isolated edge), the coefficient
  is 0.0 (empty intersection / empty union is defined as 0).
- The callback receives a reference to the edge (`uv`), which can be used to
  access edge properties if needed.

## See Also

- [Label Propagation](label_propagation.md) — community detection
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_jaccard.cpp](../../../tests/algorithms/test_jaccard.cpp) — test suite
