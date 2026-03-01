<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Articulation Points

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
  - [Finding Cut Vertices](#example-1-finding-cut-vertices)
  - [Star Graph — Single Articulation Point](#example-2-star-graph--single-articulation-point)
  - [Path Graph vs. Cycle](#example-3-path-graph-vs-cycle)
  - [Complete Graph — No Articulation Points](#example-4-complete-graph--no-articulation-points)
  - [Disconnected Graph](#example-5-disconnected-graph)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

An **articulation point** (cut vertex) is a vertex whose removal disconnects the
graph (or increases the number of connected components). The algorithm uses the
iterative **Hopcroft-Tarjan** approach with discovery times and low-link values.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

Each articulation point is emitted exactly once to the output iterator, though
the order is unspecified.

## When to Use

- **Network vulnerability analysis** — identify single points of failure in
  communication, transportation, or power networks.
- **Graph connectivity hardening** — after finding articulation points, add
  redundant links to eliminate them and make the graph biconnected.
- **Fast structural check** — at O(V + E), this is a cheap way to verify
  whether a graph is 2-connected (biconnected): if no articulation points
  exist, the graph is biconnected.

**Consider alternatives when:**

- You need the actual biconnected **subgraph decomposition**, not just the
  cut vertices → use [biconnected_components](biconnected_components.md).
- You need **bridge (cut edge) detection** → use
  [biconnected_components](biconnected_components.md) and look for 2-vertex
  components.

## Include

```cpp
#include <graph/algorithm/articulation_points.hpp>
```

## Signature

```cpp
void articulation_points(G&& g, OutputIterator cut_vertices);
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `cut_vertices` | Output iterator receiving vertex IDs of articulation points. Each vertex appears exactly once. |

## Examples

### Example 1: Finding Cut Vertices

Find all vertices whose removal would disconnect the graph.

```cpp
#include <graph/algorithm/articulation_points.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Bridge graph: two triangles connected by a bridge at vertices 2-3
//   0 - 1     3 - 4
//    \ |  --- |  /
//     2  ---- 3
Graph g({{0,1},{1,0}, {1,2},{2,1}, {0,2},{2,0},
         {2,3},{3,2},
         {3,4},{4,3}, {3,5},{5,3}, {4,5},{5,4}});

std::vector<uint32_t> cuts;
articulation_points(g, std::back_inserter(cuts));

// cuts = {2, 3} (order may vary)
// Removing vertex 2 disconnects {0,1} from {3,4,5}
// Removing vertex 3 disconnects {0,1,2} from {4,5}
```

### Example 2: Star Graph — Single Articulation Point

In a star topology the center vertex is the sole articulation point, since
every leaf communicates only through the center.

```cpp
// Star: center vertex 0 connected to leaves 1, 2, 3, 4
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}, {0,4},{4,0}});

std::vector<uint32_t> cuts;
articulation_points(star, std::back_inserter(cuts));
// cuts = {0}  — only the center is a cut vertex
```

### Example 3: Path Graph vs. Cycle

In a path graph, every interior vertex is an articulation point. Adding
one edge to close the path into a cycle eliminates all of them.

```cpp
// Path: 0 - 1 - 2 - 3
Graph path({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}});

std::vector<uint32_t> cuts;
articulation_points(path, std::back_inserter(cuts));
// cuts = {1, 2}  — both interior vertices are cut vertices
// Leaf vertices 0 and 3 are not articulation points

// Closing the path into a cycle removes all articulation points:
// 0 - 1 - 2 - 3 - 0
Graph cycle({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}, {3,0},{0,3}});

std::vector<uint32_t> cuts2;
articulation_points(cycle, std::back_inserter(cuts2));
// cuts2 is empty — every vertex can be removed without disconnection
```

### Example 4: Complete Graph — No Articulation Points

A complete graph (K_n) with n ≥ 3 has no articulation points. This is a
quick litmus test: if the algorithm returns an empty result, the graph is
biconnected.

```cpp
// K4: complete graph on 4 vertices (all pairs connected)
Graph k4({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0},
          {1,2},{2,1}, {1,3},{3,1}, {2,3},{3,2}});

std::vector<uint32_t> cuts;
articulation_points(k4, std::back_inserter(cuts));
// cuts is empty — K4 is biconnected
// Removing any single vertex still leaves a connected graph
```

### Example 5: Disconnected Graph

Articulation points are identified independently within each connected
component. Isolated vertices are never articulation points.

```cpp
// Component A: path 0-1-2 (interior vertex 1 is an AP)
// Component B: triangle 3-4-5 (no APs)
// Vertex 6: isolated
Graph g({{0,1},{1,0}, {1,2},{2,1},
         {3,4},{4,3}, {4,5},{5,4}, {3,5},{5,3}});
// Vertex 6 exists but has no edges

std::vector<uint32_t> cuts;
articulation_points(g, std::back_inserter(cuts));
// cuts = {1}  — only the interior vertex of the path component
// Isolated vertex 6 is not an articulation point
// The triangle has no articulation points
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V) for discovery times, low-link values, and the DFS stack |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- For undirected graphs, **both directions** of each edge must be stored (or use
  `undirected_adjacency_list`).
- Self-loops do not affect the result.
- Parallel edges: a vertex connecting two otherwise-disconnected subgraphs via
  parallel edges is still considered an articulation point.

## See Also

- [Biconnected Components](biconnected_components.md) — find maximal 2-connected subgraphs
- [Connected Components](connected_components.md) — connected/strongly connected components
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_articulation_points.cpp](../../../tests/algorithms/test_articulation_points.cpp) — test suite
