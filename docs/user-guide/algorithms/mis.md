# Maximal Independent Set

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Basic MIS](#example-1-basic-mis)
  - [Seed Sensitivity](#example-2-seed-sensitivity)
  - [Star Graph — Seed Matters](#example-3-star-graph--seed-matters)
  - [Complete Graph — MIS Is a Single Vertex](#example-4-complete-graph--mis-is-a-single-vertex)
  - [Disconnected Graph](#example-5-disconnected-graph)
  - [Self-Loop Exclusion](#example-6-self-loop-exclusion)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [Notes](#notes)
- [See Also](#see-also)

## Overview

Finds a **maximal independent set** (MIS) — a set of non-adjacent vertices
that cannot be extended by adding any other vertex without violating the
independence property.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

The algorithm is **greedy**: starting from a seed vertex, it includes that
vertex in the MIS, marks all its neighbors as excluded, and repeats for
remaining unmarked vertices in order.

> **Maximal vs. Maximum:** This produces a *maximal* set, not a *maximum* one.
> A maximal independent set cannot have any more vertices added to it without
> violating independence, but it may not be the largest possible independent set.
> Finding a maximum independent set is NP-hard; this greedy algorithm runs in
> linear time. The result depends on the seed vertex and adjacency ordering.

## When to Use

- **Resource allocation** — assign non-conflicting resources (e.g., scheduling
  non-overlapping tasks, radio frequency assignment).
- **Graph coloring approximation** — an MIS provides a single color class; 
  iterating MIS removal yields an approximate coloring.
- **Domination problems** — an MIS is both independent and dominating (every
  non-MIS vertex has at least one MIS neighbor).
- **Parallel algorithms** — MIS is a building block for parallel graph
  algorithms like Luby's algorithm.

**Consider alternatives when:**

- You need the **maximum** independent set → this is NP-hard; consider
  branch-and-bound or approximation frameworks, not this greedy algorithm.
- You need communities or clusters → use
  [label_propagation](label_propagation.md) or
  [connected_components](connected_components.md).

## Include

```cpp
#include <graph/algorithm/mis.hpp>
```

## Signature

```cpp
size_t maximal_independent_set(G&& g, OutputIterator mis,
    const vertex_id_t<G>& seed = 0);
```

**Returns** the number of vertices in the MIS. Selected vertex IDs are written
to the output iterator.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `mis` | Output iterator receiving vertex IDs in the MIS |
| `seed` | Starting vertex ID (default: 0). The seed is always included in the MIS (unless it has a self-loop). |

## Examples

### Example 1: Basic MIS

Find an MIS in a simple path graph.

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

// Greedy selection starting from vertex 0:
//   Include 0, exclude neighbor 1
//   Include 2 (not excluded), exclude neighbor 3
//   Include 4 (not excluded)
// result = {0, 2, 4}, count = 3
```

### Example 2: Seed Sensitivity

Different seeds produce different MIS results — both valid, but potentially
different sizes.

```cpp
// Same path graph: 0 - 1 - 2 - 3 - 4
std::vector<uint32_t> from_0, from_1;

maximal_independent_set(g, std::back_inserter(from_0), 0u);
// from_0 = {0, 2, 4}  — size 3

maximal_independent_set(g, std::back_inserter(from_1), 1u);
// from_1 = {1, 3}  — size 2

// Both are valid maximal independent sets (cannot add any vertex without
// creating an adjacency), but they have different sizes.
// This illustrates why "maximal" ≠ "maximum".
```

### Example 3: Star Graph — Seed Matters

In a star graph, choosing the center vs. a leaf yields dramatically different
MIS sizes.

```cpp
// Star: center 0 connected to leaves 1, 2, 3, 4
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}, {0,4},{4,0}});

std::vector<uint32_t> from_center, from_leaf;

maximal_independent_set(star, std::back_inserter(from_center), 0u);
// from_center = {0} — center excludes all leaves → MIS size = 1

maximal_independent_set(star, std::back_inserter(from_leaf), 1u);
// from_leaf = {1, 2, 3, 4} — all leaves are mutually non-adjacent → MIS size = 4
// (The center is excluded because it's adjacent to the seed leaf)
```

### Example 4: Complete Graph — MIS Is a Single Vertex

In a complete graph (K_n), every vertex is adjacent to every other vertex.
The MIS is always a single vertex: the seed.

```cpp
// K4: every pair connected
Graph k4({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
          {1,2},{2,1}, {1,3},{3,1}, {2,3},{3,2}});

std::vector<uint32_t> result;
size_t count = maximal_independent_set(k4, std::back_inserter(result), 0u);
// result = {0}, count = 1
// Including the seed excludes every other vertex

// This is also the *maximum* independent set — you can verify that no
// two non-adjacent vertices exist in K4.
```

### Example 5: Disconnected Graph

The algorithm processes all components. Each component contributes
independently to the MIS.

```cpp
// Two disconnected components:
// Component A: path 0 - 1 - 2
// Component B: triangle 3 - 4 - 5
Graph g({{0,1},{1,0}, {1,2},{2,1},
         {3,4},{4,3}, {4,5},{5,4}, {3,5},{5,3}});

std::vector<uint32_t> result;
size_t count = maximal_independent_set(g, std::back_inserter(result), 0u);
// Starting from 0:
//   Include 0, exclude 1, include 2     (from component A)
//   Include 3, exclude 4, exclude 5     (from component B)
// result = {0, 2, 3}, count = 3
```

### Example 6: Self-Loop Exclusion

A vertex with a self-loop is adjacent to itself and is therefore excluded
from the MIS — it cannot be independent.

```cpp
// Vertices: 0 (self-loop), 1, 2, edge 1-2
Graph g({{0,0},           // self-loop on vertex 0
         {1,2},{2,1}});

std::vector<uint32_t> result;
size_t count = maximal_independent_set(g, std::back_inserter(result), 1u);
// Vertex 0 has a self-loop → excluded (not independent)
// Include 1, exclude 2
// result = {1}, count = 1
// Vertex 0 is neither included nor considered, even though it has no
// neighbors (other than itself).
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

## Notes

- The greedy order processes vertices starting from the seed, then continues
  through all vertices in index order. The first non-excluded vertex after the
  seed is the next MIS member.
- The MIS is both **independent** (no two members are adjacent) and
  **dominating** (every non-member is adjacent to at least one member).
- For weighted variants or approximation guarantees, consider specialized
  algorithms outside this library.

## See Also

- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_mis.cpp](../../../tests/algorithms/test_mis.cpp) — test suite
