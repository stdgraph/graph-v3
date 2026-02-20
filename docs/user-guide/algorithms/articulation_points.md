# Articulation Points

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Finding Cut Vertices](#example-1-finding-cut-vertices)
  - [Star Graph](#example-2-star-graph)
  - [Path Graph](#example-3-path-graph)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

An **articulation point** (cut vertex) is a vertex whose removal disconnects the
graph (or increases the number of connected components). The algorithm uses the
iterative **Hopcroft-Tarjan** approach with discovery times and low-link values.

Each articulation point is emitted exactly once to the output iterator, though
the order is unspecified.

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
| `cut_vertices` | Output iterator receiving vertex IDs of articulation points |

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

### Example 2: Star Graph

In a star graph, the center is the only articulation point.

```cpp
// Star: center vertex 0 connected to leaves 1, 2, 3, 4
Graph star({{0,1},{1,0}, {0,2},{2,0}, {0,3},{3,0}, {0,4},{4,0}});

std::vector<uint32_t> cuts;
articulation_points(star, std::back_inserter(cuts));
// cuts = {0}  — only the center is a cut vertex
```

### Example 3: Path Graph

In a path graph, all interior vertices are articulation points.

```cpp
// Path: 0 - 1 - 2 - 3
Graph path({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}});

std::vector<uint32_t> cuts;
articulation_points(path, std::back_inserter(cuts));
// cuts = {1, 2}  — both interior vertices are cut vertices
// Leaf vertices 0 and 3 are not articulation points

// A cycle has NO articulation points:
// 0 - 1 - 2 - 3 - 0
Graph cycle({{0,1},{1,0}, {1,2},{2,1}, {2,3},{3,2}, {3,0},{0,3}});

std::vector<uint32_t> cuts2;
articulation_points(cycle, std::back_inserter(cuts2));
// cuts2 is empty — every vertex can be removed without disconnection
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
