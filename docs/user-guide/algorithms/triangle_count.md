<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Triangle Count

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Counting Triangles](#example-1-counting-triangles)
  - [Complete Graph K4](#example-2-complete-graph-k4)
  - [Triangle-Free Graphs](#example-3-triangle-free-graphs)
  - [Multiple Sharing Triangles](#example-4-multiple-sharing-triangles)
  - [Pre-sorting vov Adjacency Lists](#example-5-pre-sorting-vov-adjacency-lists)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [Notes](#notes)
- [See Also](#see-also)

## Overview

Counts the number of **triangles** (3-cliques) in an undirected graph using
merge-based sorted-list intersection. A triangle is a set of three mutually
adjacent vertices {u, v, w}.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

The algorithm additionally requires `ordered_vertex_edges<G>`, meaning each
vertex's adjacency list is **sorted by target ID**. This is naturally the case
for set-based containers (e.g., `vos` traits) and `undirected_adjacency_list`.

## When to Use

- **Social network analysis** — the triangle count and derived **clustering
  coefficient** (ratio of actual to possible triangles per vertex) measure how
  tightly knit a network is.
- **Community detection preprocessing** — a high local triangle density
  indicates a cohesive community.
- **Graph quality metrics** — triangle count is a fundamental structural
  statistic used in network science, bioinformatics, and fraud detection.

**Not suitable when:**

- You need to **enumerate** each triangle (the algorithm only counts them).
- The graph is directed — this algorithm treats the graph as undirected.
- Adjacency lists are unsorted and cannot be sorted — the algorithm requires
  the `ordered_vertex_edges` concept.

## Include

```cpp
#include <graph/algorithm/tc.hpp>
```

## Signature

```cpp
size_t triangle_count(G&& g);
```

**Returns** the total number of triangles in the graph.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` **and** `ordered_vertex_edges` (sorted adjacency lists) |

## Examples

### Example 1: Counting Triangles

Count triangles in a simple graph with one triangle.

```cpp
#include <graph/algorithm/tc.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

// Using sorted-edge traits (vos = vector of sets)
using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vos_graph_traits<void>>;

// Triangle: 0-1-2
// Edges stored bidirectionally (both directions)
Graph g({{0,1}, {1,0}, {0,2}, {2,0}, {1,2}, {2,1}});

size_t count = triangle_count(g);
// count == 1
```

Or with `undirected_adjacency_list` which keeps edges sorted automatically:

```cpp
#include <graph/container/undirected_adjacency_list.hpp>

undirected_adjacency_list<int, int> g({{0,1,1}, {0,2,1}, {1,2,1}});

size_t count = triangle_count(g);
// count == 1
```

### Example 2: Complete Graph K4

K4 has $\binom{4}{3} = 4$ triangles. Every 3-vertex subset forms a triangle.

```cpp
// K4: every pair of 4 vertices connected
Graph g({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
         {1,2},{2,1}, {1,3},{3,1}, {2,3},{3,2}});

size_t count = triangle_count(g);
// count == 4
// Triangles: {0,1,2}, {0,1,3}, {0,2,3}, {1,2,3}
```

### Example 3: Triangle-Free Graphs

Graphs without triangles return 0 — paths, stars, trees, and bipartite graphs
are all triangle-free.

```cpp
// Star graph: center 0 connected to leaves 1, 2, 3
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}});
// triangle_count(star) == 0  — no pair of leaves is connected

// Path: 0 - 1 - 2 - 3
Graph path({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}});
// triangle_count(path) == 0

// Tree: any tree is triangle-free
// triangle_count(tree) == 0

// Bipartite K3,3 — no odd cycles, hence no triangles
// triangle_count(k33) == 0
```

### Example 4: Multiple Sharing Triangles

Triangles can share edges. A "diamond" graph (K4 minus one edge) has
2 triangles that share an edge.

```cpp
// Diamond: vertices 0-1-2-3, edges {0,1},{0,2},{1,2},{1,3},{2,3}
// Missing edge: {0,3}
Graph diamond({{0,1},{1,0}, {0,2},{2,0}, {1,2},{2,1},
               {1,3},{3,1}, {2,3},{3,2}});

size_t count = triangle_count(diamond);
// count == 2
// Triangles: {0,1,2} and {1,2,3}
// Edge 1-2 is shared between both triangles
```

### Example 5: Pre-sorting vov Adjacency Lists

If you use `vov`-based traits (vectors of vectors), adjacency lists may not
be sorted. Sort them before calling `triangle_count`.

```cpp
#include <graph/algorithm/tc.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <algorithm>

// vov traits: adjacency lists are unsorted vectors
using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

Graph g({{0,2}, {2,0}, {0,1}, {1,0}, {1,2}, {2,1}});
// Adjacency list for vertex 0 might be [2, 1] — NOT sorted

// Sort each adjacency list by target ID
for (auto&& [uid, u] : vertices(g)) {
    auto& edges_of_u = edges(g, u);
    std::ranges::sort(edges_of_u, [&](auto& a, auto& b) {
        return target_id(g, a) < target_id(g, b);
    });
}

size_t count = triangle_count(g);
// count == 1

// Tip: prefer vos_graph_traits or undirected_adjacency_list to avoid
//      manual sorting.
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(m^{3/2}) average for sparse graphs, where m = number of edges |
| Space | O(1) auxiliary — uses sorted adjacency lists directly |

The merge-based intersection exploits sorted neighbor lists, making it
efficient for sparse graphs. For dense graphs with m ≈ V², the complexity
approaches O(V³).

## Preconditions

- Graph must satisfy `index_adjacency_list<G>` **and** `ordered_vertex_edges<G>`.
- Adjacency lists must be **sorted by target ID**. Use sorted-edge traits
  (`vos`, `dos`) or `undirected_adjacency_list`.
- For `vov`-based graphs, pre-sort each adjacency list before calling (see
  Example 5).
- Self-loops are ignored and do not count as triangles.

## Notes

- The algorithm counts each triangle exactly **once**, not three times (once
  per vertex).
- Only the count is returned, not the vertex triples. If you need to enumerate
  triangles, you would need to implement enumeration separately.
- `ordered_vertex_edges` is a compile-time concept — the library will give a
  concept error if the graph's edge ranges aren't sorted.

## See Also

- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_triangle_count.cpp](../../../tests/algorithms/test_triangle_count.cpp) — test suite
