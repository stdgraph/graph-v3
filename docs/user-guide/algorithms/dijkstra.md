# Dijkstra's Shortest Paths

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Single-Source Shortest Paths](#example-1-single-source-shortest-paths)
  - [Distances Only (No Predecessors)](#example-2-distances-only-no-predecessors)
  - [Multi-Source Shortest Paths](#example-3-multi-source-shortest-paths)
  - [Path Reconstruction](#example-4-path-reconstruction)
  - [Unweighted Graph (Default Weight)](#example-5-unweighted-graph-default-weight)
  - [Custom Visitor](#example-6-custom-visitor)
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
  lightweight null-predecessor optimization internally.

All overloads accept single or multiple source vertices, an optional weight
function, an optional visitor, and optional comparator/combiner functors.

> **Initialization:** Call `init_shortest_paths(distances)` or
> `init_shortest_paths(distances, predecessors)` before invoking the algorithm.
> This sets all distances to infinity and each vertex's predecessor to itself.

## When to Use

- **Shortest paths with non-negative weights** — the classic use case. Road
  networks, weighted graphs, cost-optimized routing.
- **Unweighted shortest paths** — omit the weight function (the default returns
  `1` for every edge), giving BFS-equivalent hop-count distances with the same
  result as BFS but at O((V+E) log V) instead of O(V+E).
- **Multi-source nearest-facility queries** — pass multiple sources to find the
  closest source for every vertex in one pass.

**Not suitable when:**

- Edge weights may be **negative** → use [Bellman-Ford](bellman_ford.md).
- You only need traversal order without distances → use [BFS](bfs.md) or
  [DFS](dfs.md).

## Include

```cpp
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
```

## Signatures

```cpp
// Multi-source, distances + predecessors
constexpr void dijkstra_shortest_paths(G&& g, const Sources& sources,
    Distances& distances, Predecessors& predecessors,
    WF&& weight = /* default returns 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances + predecessors
constexpr void dijkstra_shortest_paths(G&& g, const vertex_id_t<G>& source,
    Distances& distances, Predecessors& predecessors,
    WF&& weight = /* default returns 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Multi-source, distances only
constexpr void dijkstra_shortest_distances(G&& g, const Sources& sources,
    Distances& distances,
    WF&& weight = /* default returns 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances only
constexpr void dijkstra_shortest_distances(G&& g, const vertex_id_t<G>& source,
    Distances& distances,
    WF&& weight = /* default returns 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` / `sources` | Source vertex ID or range of source vertex IDs |
| `distances` | Random-access range sized to `num_vertices(g)`. Filled with shortest distances. Must satisfy `is_arithmetic_v<range_value_t<Distances>>`. |
| `predecessors` | Random-access range sized to `num_vertices(g)`. Filled with predecessor vertex IDs. |
| `weight` | Callable `WF(g, uv)` returning edge weight. Must satisfy `basic_edge_weight_function`. Default: returns `1` for every edge (unweighted). |
| `visitor` | Optional visitor struct with callback methods (see below). Default: `empty_visitor{}`. |
| `compare` | Comparison function for distance values. Default: `std::less<>{}`. |
| `combine` | Combine function for distance + weight. Default: `std::plus<>{}`. |

**Error handling:** Throws `std::out_of_range` if a source vertex ID is invalid,
if `distances` or `predecessors` is undersized, or if a negative edge weight is
encountered (signed weight types only).

## Visitor Events

Dijkstra's algorithm supports an optional visitor with the following callbacks.
Each vertex event also has an `_id` variant that receives `vertex_id_t<G>`
instead of a vertex reference (e.g., `on_discover_vertex_id(g, uid)`). You only
need to define the events you care about — missing methods are silently skipped.

| Event | Called when |
|-------|------------|
| `on_initialize_vertex(g, u)` | Before traversal starts, for each vertex |
| `on_discover_vertex(g, u)` | Vertex first reached (inserted into priority queue) |
| `on_examine_vertex(g, u)` | Vertex popped from priority queue for processing |
| `on_examine_edge(g, uv)` | Outgoing edge examined for relaxation |
| `on_edge_relaxed(g, uv)` | Edge improved a shorter path |
| `on_edge_not_relaxed(g, uv)` | Edge did not improve the current best path |
| `on_finish_vertex(g, u)` | All adjacent edges of vertex explored |

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

// Initialize distances to infinity, predecessors to self
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
init_shortest_paths(dist);  // one-argument form: distances only

dijkstra_shortest_distances(g, 0u, dist,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[v] = shortest distance from source 0 to v
// Unreachable vertices remain at numeric_limits<double>::max()
```

### Example 3: Multi-Source Shortest Paths

Run Dijkstra from multiple source vertices simultaneously — useful for finding
the nearest facility, multi-root BFS-like queries, etc.

```cpp
std::vector<double>   dist(num_vertices(g));
std::vector<unsigned> pred(num_vertices(g));

init_shortest_paths(dist, pred);

// Sources can be any range: vector, array, initializer_list
std::array sources{0u, 2u};
dijkstra_shortest_paths(g, sources, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[v] = shortest distance from the nearest source to v
// pred[v] traces back to whichever source was closest
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
    // path = {0, 4, 3} with total distance dist[3] = 6.0
}
```

### Example 5: Unweighted Graph (Default Weight)

When no weight function is provided, the default returns `1` for every edge.
This gives BFS-equivalent hop-count distances (though at higher time complexity
than BFS itself).

```cpp
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/container/dynamic_graph.hpp>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

Graph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}});

std::vector<double>   dist(num_vertices(g));
std::vector<uint32_t> pred(num_vertices(g));
init_shortest_paths(dist, pred);

// No weight function — default returns 1 per edge
dijkstra_shortest_paths(g, 0u, dist, pred);

// dist[0] = 0, dist[1] = 1, dist[2] = 1, dist[3] = 2, dist[4] = 3
// These are hop counts — same as BFS distances
```

### Example 6: Custom Visitor

Use a visitor to observe algorithm progress — useful for logging, debugging,
early analysis, or computing derived metrics during traversal.

```cpp
struct RelaxationTracker {
    int relaxed = 0;
    int not_relaxed = 0;
    int vertices_settled = 0;

    // Track how many edges improve a path
    void on_edge_relaxed(const auto& g, const auto& uv) {
        ++relaxed;
    }
    void on_edge_not_relaxed(const auto& g, const auto& uv) {
        ++not_relaxed;
    }
    // Count settled vertices (popped from priority queue)
    void on_examine_vertex(const auto& g, const auto& u) {
        ++vertices_settled;
    }
};

std::vector<double>   dist(num_vertices(g));
std::vector<unsigned> pred(num_vertices(g));
init_shortest_paths(dist, pred);

RelaxationTracker tracker;
dijkstra_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); },
    tracker);

// tracker.relaxed = number of successful edge relaxations
// tracker.vertices_settled = number of vertices processed
```

You can also use `_id` variants to receive vertex IDs directly:

```cpp
struct IdVisitor {
    void on_discover_vertex_id(const auto& g, auto uid) {
        std::cout << "Discovered vertex " << uid << "\n";
    }
    void on_edge_relaxed(const auto& g, const auto& uv) {
        std::cout << source_id(g, uv) << " → " << target_id(g, uv) << " relaxed\n";
    }
};
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O((V + E) log V) |
| Space | O(V) auxiliary (priority queue + color map) |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- All edge weights must be **non-negative**. For negative weights, use
  [Bellman-Ford](bellman_ford.md). For signed weight types, a negative weight
  throws `std::out_of_range` at runtime.
- `distances` and `predecessors` must be sized to `num_vertices(g)`.
- Call `init_shortest_paths(distances, predecessors)` before invoking the
  algorithm.

## See Also

- [Bellman-Ford Shortest Paths](bellman_ford.md) — supports negative edge weights
- [BFS](bfs.md) — O(V+E) unweighted shortest paths via traversal
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_dijkstra_shortest_paths.cpp](../../../tests/algorithms/test_dijkstra_shortest_paths.cpp) — test suite
