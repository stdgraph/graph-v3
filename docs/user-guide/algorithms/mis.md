# Maximal Independent Set

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Basic MIS](#example-1-basic-mis)
  - [Seed Sensitivity](#example-2-seed-sensitivity)
  - [Star Graph — Seed Matters](#example-3-star-graph--seed-matters)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Finds a **maximal independent set** (MIS) — a set of non-adjacent vertices
that cannot be extended by adding any other vertex without violating the
independence property.

The algorithm is **greedy**: starting from a seed vertex, it includes that
vertex in the MIS, marks all its neighbors as excluded, and repeats for
remaining unmarked vertices in order.

> **Note:** This produces a *maximal* set, not a *maximum* one. The result
> depends on the seed vertex and adjacency ordering. Different seeds may yield
> different-sized MIS results.

## Include

```cpp
#include <graph/algorithm/mis.hpp>
```

## Signature

```cpp
size_t maximal_independent_set(G&& g, OutputIterator mis,
    vertex_id_t<G> seed = 0);
```

**Returns** the number of vertices in the MIS. Selected vertex IDs are written
to the output iterator.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `mis` | Output iterator receiving vertex IDs in the MIS |
| `seed` | Starting vertex ID (default: 0). Always included in the MIS. |

## Examples

### Example 1: Basic MIS

Find an MIS in a simple graph.

```cpp
#include <graph/algorithm/mis.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Path graph: 0 - 1 - 2 - 3 - 4  (bidirectional edges)
Graph g({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}, {3,4},{4,3}});

std::vector<uint32_t> result;
size_t count = maximal_independent_set(g, std::back_inserter(result), 0u);

// Starting from vertex 0:
//   Include 0, exclude 1
//   Include 2, exclude 3
//   Include 4
// result = {0, 2, 4}, count = 3
```

### Example 2: Seed Sensitivity

Different seeds produce different MIS results.

```cpp
// Same path graph
std::vector<uint32_t> from_0, from_1;

maximal_independent_set(g, std::back_inserter(from_0), 0u);
// from_0 = {0, 2, 4}

maximal_independent_set(g, std::back_inserter(from_1), 1u);
// from_1 = {1, 3}

// Both are valid maximal independent sets, but different sizes!
```

### Example 3: Star Graph — Seed Matters

In a star graph, choosing the center vs. a leaf yields dramatically different
results.

```cpp
// Star: center 0 connected to leaves 1, 2, 3, 4
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}, {0,4},{4,0}});

std::vector<uint32_t> from_center, from_leaf;

maximal_independent_set(star, std::back_inserter(from_center), 0u);
// from_center = {0} — center excludes all leaves, MIS size = 1

maximal_independent_set(star, std::back_inserter(from_leaf), 1u);
// from_leaf = {1, 2, 3, 4} — all leaves are mutually non-adjacent, MIS size = 4
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V) for the exclusion set |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- Seed must be a valid vertex ID (`0 ≤ seed < num_vertices(g)`).
- Self-loops exclude a vertex from the MIS (a vertex adjacent to itself is not
  independent).

## See Also

- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_mis.cpp](../../../tests/algorithms/test_mis.cpp) — test suite
