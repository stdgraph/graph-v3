# Jaccard Coefficient

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Collecting Similarity Scores](#example-1-collecting-similarity-scores)
  - [Triangle — Shared Neighbors](#example-2-triangle--shared-neighbors)
  - [Star Graph — No Shared Neighbors](#example-3-star-graph--no-shared-neighbors)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Computes the **Jaccard similarity coefficient** for every directed edge in the
graph. For an edge (u, v), the coefficient is:

$$J(u, v) = \frac{|N(u) \cap N(v)|}{|N(u) \cup N(v)|}$$

where $N(u)$ is the neighbor set of vertex $u$. The coefficient ranges from
0.0 (no shared neighbors) to 1.0 (identical neighbor sets).

The algorithm is **callback-driven**: it invokes a user-provided callable for
each directed edge with its Jaccard coefficient. Self-loops are skipped.

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
| `g` | Graph satisfying `index_adjacency_list` |
| `out` | Callback invoked for each directed edge (u, v) with the Jaccard coefficient |

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

Graph g({{0,1},{1,0}, {0,2},{2,0}, {1,2},{2,1}, {1,3},{3,1}});

std::map<std::pair<uint32_t, uint32_t>, double> scores;
jaccard_coefficient(g, [&](auto uid, auto vid, auto& uv, double j) {
    scores[{uid, vid}] = j;
});

// scores[{0,1}] = 1/3  (shared: {2}, union: {1,2,3} for u=0, {0,2,3} for u=1
//                        → intersection={2}, union={0,1,2,3})
// J(u,v) == J(v,u) for undirected graphs
```

### Example 2: Triangle — Shared Neighbors

In a triangle (K3), each pair of vertices shares exactly one neighbor.

```cpp
// Triangle: 0-1-2
Graph g({{0,1},{1,0}, {0,2},{2,0}, {1,2},{2,1}});

std::vector<double> coefficients;
jaccard_coefficient(g, [&](auto uid, auto vid, auto& uv, double j) {
    coefficients.push_back(j);
});

// Every edge has J = 1/3:
//   N(0)={1,2}, N(1)={0,2} → intersection={2}, union={0,1,2} → 1/3
// 6 directed edges, each with coefficient ≈ 0.333
```

### Example 3: Star Graph — No Shared Neighbors

In a star graph, no pair of leaf vertices shares a neighbor with the center.

```cpp
// Star: center 0, leaves 1, 2, 3
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}});

jaccard_coefficient(star, [](auto uid, auto vid, auto& uv, double j) {
    // Center-to-leaf edges: J(0, leaf) = 0.0
    //   N(0) = {1,2,3}, N(1) = {0} → intersection = {} → J = 0
    //   (0 is excluded from N(0) since self-loops are skipped)
});
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E · d_min) where d_min = min degree of edge endpoints |
| Space | O(V + E) for precomputed neighbor sets |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- Self-loops are skipped (not passed to the callback).
- The callback is invoked once per **directed** edge — for undirected graphs
  with bidirectional storage, expect two calls per logical edge.

## See Also

- [Label Propagation](label_propagation.md) — community detection
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_jaccard.cpp](../../../tests/algorithms/test_jaccard.cpp) — test suite
