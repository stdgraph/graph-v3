# Triangle Count

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Counting Triangles](#example-1-counting-triangles)
  - [Complete Graph K4](#example-2-complete-graph-k4)
  - [No Triangles](#example-3-no-triangles)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Counts the number of **triangles** (3-cliques) in an undirected graph using
merge-based sorted-list intersection. A triangle is a set of three mutually
adjacent vertices {u, v, w}.

The algorithm requires that each vertex's adjacency list is **sorted by target
ID**. This is naturally the case for set-based containers (e.g., `vos` traits)
and `undirected_adjacency_list`.

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

K4 has $\binom{4}{3} = 4$ triangles.

```cpp
// K4: every pair of 4 vertices connected
Graph g({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
         {1,2},{2,1}, {1,3},{3,1}, {2,3},{3,2}});

size_t count = triangle_count(g);
// count == 4
```

### Example 3: No Triangles

Graphs without triangles return 0 — paths, stars, trees, and bipartite graphs
are triangle-free.

```cpp
// Star graph: center 0 connected to leaves 1, 2, 3
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}});
// triangle_count(star) == 0  — no 3 leaves are mutually connected

// Path: 0 - 1 - 2 - 3
Graph path({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}});
// triangle_count(path) == 0

// Bipartite K3,3 — no odd cycles
// triangle_count(k33) == 0
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(m^{3/2}) average for sparse graphs, where m = number of edges |
| Space | O(1) auxiliary — uses sorted adjacency lists directly |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>` **and** `ordered_vertex_edges<G>`.
- Adjacency lists must be **sorted by target ID**. Use sorted-edge traits
  (`vos`, `dos`) or `undirected_adjacency_list`.
- For `vov`-based graphs, pre-sort each adjacency list before calling.
- Self-loops are ignored and do not count as triangles.

## See Also

- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_triangle_count.cpp](../../../tests/algorithms/test_triangle_count.cpp) — test suite
