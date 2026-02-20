# Dijkstra's Shortest Paths

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Single-Source Shortest Paths](#example-1-single-source-shortest-paths)
  - [Distances Only (No Predecessors)](#example-2-distances-only-no-predecessors)
  - [Multi-Source Shortest Paths](#example-3-multi-source-shortest-paths)
  - [Path Reconstruction](#example-4-path-reconstruction)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Dijkstra's algorithm finds single-source or multi-source shortest paths in a
graph with **non-negative** edge weights. It uses a binary-heap priority queue
to greedily expand the nearest unvisited vertex until all reachable vertices
have been settled.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

The library provides two families of overloads:

- **`dijkstra_shortest_paths`** — computes both distances and predecessor maps,
  enabling path reconstruction.
- **`dijkstra_shortest_distances`** — computes distances only, using a
  lightweight null-predecessor optimization.

All overloads accept single or multiple source vertices, an optional weight
function, an optional visitor, and optional comparator/combiner functors.

## Include

```cpp
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
```

## Signatures

```cpp
// Multi-source, distances + predecessors
void dijkstra_shortest_paths(G&& g, const Sources& sources,
    Distances& distances, Predecessors& predecessors,
    WF&& weight = /* default 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances + predecessors
void dijkstra_shortest_paths(G&& g, vertex_id_t<G> source,
    Distances& distances, Predecessors& predecessors,
    WF&& weight = /* default 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Multi-source, distances only
void dijkstra_shortest_distances(G&& g, const Sources& sources,
    Distances& distances,
    WF&& weight = /* default 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances only
void dijkstra_shortest_distances(G&& g, vertex_id_t<G> source,
    Distances& distances,
    WF&& weight = /* default 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` / `sources` | Source vertex ID or range of source vertex IDs |
| `distances` | Random-access range sized to `num_vertices(g)`. Filled with shortest distances. |
| `predecessors` | Random-access range sized to `num_vertices(g)`. Filled with predecessor vertex IDs. |
| `weight` | Callable `WF(g, uv)` returning edge weight. Defaults to returning `1` for every edge. |
| `visitor` | Optional visitor struct with callback methods (see below) |
| `compare` | Comparison function for distance values. Default: `std::less<>{}` |
| `combine` | Combine function for distance + weight. Default: `std::plus<>{}` |

## Visitor Events

| Event | Called when |
|-------|------------|
| `on_initialize_vertex(g, u)` | Before traversal starts, for each vertex |
| `on_discover_vertex(g, u)` | Vertex first reached (inserted into priority queue) |
| `on_examine_vertex(g, u)` | Vertex popped from priority queue for processing |
| `on_examine_edge(g, uv)` | Outgoing edge examined for relaxation |
| `on_edge_relaxed(g, uv)` | Edge improved a shorter path |
| `on_edge_not_relaxed(g, uv)` | Edge did not improve the current best path |
| `on_finish_vertex(g, u)` | All adjacent edges of vertex explored |

Each event also has an `_id` variant that receives `vertex_id_t<G>` instead
of vertex/edge references (e.g. `on_discover_vertex(g, uid)`).

## Examples

### Example 1: Single-Source Shortest Paths

The most common usage — find shortest paths from one vertex to all others in a
weighted directed graph.

```cpp
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

// Create a weighted directed graph (CLRS example)
//        10       1
//   0 ------→ 1 ----→ 3
//   |  \       ↑      ↑
//   5   2     3|     9|
//   ↓    ↘    |      |
//   2 ------→ 4      |
//   2 ------→ 3 ←----+
using Graph = container::dynamic_graph<double, void, void, unsigned, false,
    container::vov_graph_traits<double>>;

Graph g({{0, 1, 10.0}, {0, 2, 5.0}, {0, 4, 2.0},
         {1, 3, 1.0},
         {2, 1, 3.0}, {2, 3, 9.0}, {2, 4, 2.0},
         {3, 4, 7.0},
         {4, 1, 6.0}, {4, 3, 4.0}});

std::vector<double>   dist(num_vertices(g));
std::vector<unsigned> pred(num_vertices(g));

init_shortest_paths(dist, pred);

dijkstra_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[0] = 0, dist[1] = 8, dist[2] = 5, dist[3] = 6, dist[4] = 2
// pred[0] = 0, pred[1] = 2, pred[2] = 0, pred[3] = 4, pred[4] = 0
```

### Example 2: Distances Only (No Predecessors)

When you only need distances and don't need to reconstruct paths, use
`dijkstra_shortest_distances` — it skips predecessor tracking entirely.

```cpp
std::vector<double> dist(num_vertices(g));
init_shortest_paths(dist);

dijkstra_shortest_distances(g, 0u, dist,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[v] = shortest distance from source 0 to v
// Unreachable vertices remain at numeric_limits<double>::max()
```

### Example 3: Multi-Source Shortest Paths

Run Dijkstra from multiple source vertices simultaneously — useful for finding
nearest facility, multi-root BFS-like queries, etc.

```cpp
std::vector<double>   dist(num_vertices(g));
std::vector<unsigned> pred(num_vertices(g));

init_shortest_paths(dist, pred);

// Sources can be any range: vector, array, initializer_list
std::array sources{0u, 2u};
dijkstra_shortest_paths(g, sources, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[v] = shortest distance from nearest source to v
```

### Example 4: Path Reconstruction

After running `dijkstra_shortest_paths`, reconstruct the shortest path from
source to any target by walking the predecessor array backwards.

```cpp
// After running dijkstra_shortest_paths(g, source, dist, pred, weight) ...

unsigned target = 3;
unsigned source = 0;

if (dist[target] == std::numeric_limits<double>::max()) {
    // target is unreachable from source
} else {
    // Build path in reverse
    std::vector<unsigned> path;
    for (unsigned v = target; v != source; v = pred[v]) {
        path.push_back(v);
    }
    path.push_back(source);
    std::ranges::reverse(path);
    // path = {0, 4, 3} with total distance dist[3]
}
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O((V + E) log V) |
| Space | O(V) auxiliary (priority queue + color map) |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- All edge weights must be **non-negative**. For negative weights, use
  [Bellman-Ford](bellman_ford.md).
- `distances` and `predecessors` must be sized to `num_vertices(g)`.
- Call `init_shortest_paths(distances, predecessors)` before invoking the algorithm.

## See Also

- [Bellman-Ford Shortest Paths](bellman_ford.md) — supports negative edge weights
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_dijkstra_shortest_paths.cpp](../../../tests/algorithms/test_dijkstra_shortest_paths.cpp) — test suite
