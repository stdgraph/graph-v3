# Bellman-Ford Shortest Paths

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Single-Source Shortest Paths](#example-1-single-source-shortest-paths)
  - [Negative Cycle Detection](#example-2-negative-cycle-detection)
  - [Extracting a Negative Cycle](#example-3-extracting-a-negative-cycle)
  - [Multi-Source Shortest Paths](#example-4-multi-source-shortest-paths)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

The Bellman-Ford algorithm computes single-source (or multi-source) shortest
paths in a weighted directed graph, supporting **negative edge weights**. Unlike
Dijkstra's algorithm, it can detect negative-weight cycles.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

The algorithm relaxes all edges V−1 times, then performs a final pass to detect
negative cycles. It returns `std::optional<vertex_id_t<G>>`: empty if no
negative cycle exists, or a vertex ID on the cycle if one is detected.

The library also provides `find_negative_cycle` to extract the full cycle path
from the predecessor array.

## Include

```cpp
#include <graph/algorithm/bellman_ford_shortest_paths.hpp>
```

## Signatures

```cpp
// Multi-source, distances + predecessors
[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_paths(G&& g, const Sources& sources,
    Distances& distances, Predecessors& predecessors,
    WF&& weight = /* default 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances + predecessors
[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_paths(G&& g, vertex_id_t<G> source,
    Distances& distances, Predecessors& predecessors,
    WF&& weight,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Multi-source, distances only
[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_distances(G&& g, const Sources& sources,
    Distances& distances,
    WF&& weight,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances only
[[nodiscard]] optional<vertex_id_t<G>>
bellman_ford_shortest_distances(G&& g, vertex_id_t<G> source,
    Distances& distances,
    WF&& weight,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Extract negative cycle from predecessor array
void find_negative_cycle(G&& g, Predecessors& predecessors,
    vertex_id_t<G> cycle_vertex, OutputIterator out_cycle);
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` / `sources` | Source vertex ID or range of source vertex IDs |
| `distances` | Random-access range sized to `num_vertices(g)`. Filled with shortest distances. |
| `predecessors` | Random-access range sized to `num_vertices(g)`. Filled with predecessor vertex IDs. |
| `weight` | Callable `WF(g, uv)` returning edge weight (may be negative) |
| `visitor` | Optional visitor struct with callback methods (see below) |
| `compare` | Comparison function for distance values. Default: `std::less<>{}` |
| `combine` | Combine function for distance + weight. Default: `std::plus<>{}` |
| `cycle_vertex` | A vertex ID on the negative cycle (from the return value) |
| `out_cycle` | Output iterator for the cycle vertex sequence |

**Return value:** `std::optional<vertex_id_t<G>>` — empty if no negative cycle,
or a vertex ID on the cycle.

## Visitor Events

| Event | Called when |
|-------|------------|
| `on_discover_vertex(g, u)` | Vertex first reached |
| `on_examine_edge(g, uv)` | Edge examined for relaxation |
| `on_edge_relaxed(g, uv)` | Edge improved a shorter path |
| `on_edge_not_relaxed(g, uv)` | Edge did not improve the current best path |
| `on_edge_minimized(g, uv)` | Final check pass: edge at minimum distance |
| `on_edge_not_minimized(g, uv)` | Final check pass: edge NOT at minimum (negative cycle indicator) |

## Examples

### Example 1: Single-Source Shortest Paths

Basic usage with a graph that has no negative cycles.

```cpp
#include <graph/algorithm/bellman_ford_shortest_paths.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<int, void, void, uint32_t, false,
    container::vov_graph_traits<int>>;

// Directed graph with a negative edge weight (but no negative cycle)
Graph g({{0, 1, 6}, {0, 2, 7}, {1, 2, 8}, {1, 3, -4},
         {1, 4, 5}, {2, 3, 9}, {2, 4, -3}, {4, 3, 7}});

std::vector<int>      dist(num_vertices(g));
std::vector<uint32_t> pred(num_vertices(g));

init_shortest_paths(dist, pred);

auto cycle = bellman_ford_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

if (!cycle) {
    // No negative cycle — dist[v] is the shortest distance from source 0
    // dist[0] = 0, dist[1] = 6, dist[2] = 7, dist[3] = 2, dist[4] = 4
}
```

### Example 2: Negative Cycle Detection

The algorithm detects negative-weight cycles reachable from the source.

```cpp
// Graph with a negative cycle: 0→1 (1), 1→2 (1), 2→0 (-3)
// Cycle weight: 1 + 1 + (-3) = -1
Graph g({{0, 1, 1}, {1, 2, 1}, {2, 0, -3}});

std::vector<int>      dist(num_vertices(g));
std::vector<uint32_t> pred(num_vertices(g));

init_shortest_paths(dist, pred);

auto cycle = bellman_ford_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

if (cycle) {
    // Negative cycle detected!
    // *cycle is a vertex ID on the cycle
    std::cout << "Negative cycle found involving vertex " << *cycle << "\n";
}
```

### Example 3: Extracting a Negative Cycle

Use `find_negative_cycle` to obtain the full cycle from the predecessor array.

```cpp
auto cycle = bellman_ford_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

if (cycle) {
    std::vector<uint32_t> cycle_path;
    find_negative_cycle(g, pred, *cycle, std::back_inserter(cycle_path));

    // cycle_path contains vertex IDs forming the negative cycle
    std::cout << "Negative cycle: ";
    for (auto v : cycle_path)
        std::cout << v << " → ";
    std::cout << cycle_path.front() << "\n";
}
```

### Example 4: Multi-Source Shortest Paths

Run from multiple sources simultaneously.

```cpp
std::vector<int>      dist(num_vertices(g));
std::vector<uint32_t> pred(num_vertices(g));

init_shortest_paths(dist, pred);

std::array sources{0u, 3u};
auto cycle = bellman_ford_shortest_paths(g, sources, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

// dist[v] = shortest distance from nearest source to v
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V · E) |
| Space | O(1) auxiliary (beyond input/output arrays) |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- `distances` and `predecessors` must be sized to `num_vertices(g)`.
- Call `init_shortest_paths(distances, predecessors)` before invoking the algorithm.
- **Always check the return value** — if a negative cycle exists, distances are
  undefined for affected vertices.

## See Also

- [Dijkstra's Shortest Paths](dijkstra.md) — faster for non-negative weights: O((V+E) log V)
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_bellman_ford_shortest_paths.cpp](../../../tests/algorithms/test_bellman_ford_shortest_paths.cpp) — test suite
