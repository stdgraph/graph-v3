<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Bellman-Ford Shortest Paths

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Single-Source Shortest Paths](#example-1-single-source-shortest-paths)
  - [Negative Cycle Detection](#example-2-negative-cycle-detection)
  - [Extracting a Negative Cycle](#example-3-extracting-a-negative-cycle)
  - [Multi-Source Shortest Paths](#example-4-multi-source-shortest-paths)
  - [Distances Only](#example-5-distances-only)
  - [Visitor: Monitoring the Final Check Pass](#example-6-visitor-monitoring-the-final-check-pass)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [Postconditions](#postconditions)
- [Throws](#throws)
- [Remarks](#remarks)
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

> **`[[nodiscard]]`:** All Bellman-Ford overloads are marked `[[nodiscard]]` —
> the compiler warns if you ignore the return value, because failing to check
> for negative cycles leaves distances in an undefined state.

## When to Use

- **Graphs with negative edge weights** — the primary reason to choose
  Bellman-Ford over Dijkstra. Common in financial modeling (arbitrage detection),
  network flow, and difference constraints.
- **Negative cycle detection** — determine whether a negative-weight cycle
  exists and extract it.
- **Correctness over speed** — Bellman-Ford is O(V·E) vs. Dijkstra's
  O((V+E) log V), so prefer Dijkstra when all weights are non-negative.

**Not suitable when:**

- All edge weights are **non-negative** → use [Dijkstra](dijkstra.md) for better
  performance.
- You need unweighted traversal → use [BFS](bfs.md) or [DFS](dfs.md).

## Include

```cpp
#include <graph/algorithm/bellman_ford_shortest_paths.hpp>
```

## Signatures

```cpp
// Multi-source, distances + predecessors
[[nodiscard]] constexpr optional<vertex_id_t<G>>
bellman_ford_shortest_paths(G&& g, const Sources& sources,
    Distances& distances, Predecessors& predecessors,
    WF&& weight = /* default returns 1 */,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances + predecessors
[[nodiscard]] constexpr optional<vertex_id_t<G>>
bellman_ford_shortest_paths(G&& g, const vertex_id_t<G>& source,
    Distances& distances, Predecessors& predecessors,
    WF&& weight,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Multi-source, distances only
[[nodiscard]] constexpr optional<vertex_id_t<G>>
bellman_ford_shortest_distances(G&& g, const Sources& sources,
    Distances& distances,
    WF&& weight,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Single-source, distances only
[[nodiscard]] constexpr optional<vertex_id_t<G>>
bellman_ford_shortest_distances(G&& g, const vertex_id_t<G>& source,
    Distances& distances,
    WF&& weight,
    Visitor&& visitor = empty_visitor(),
    Compare&& compare = less<>{},
    Combine&& combine = plus<>{});

// Extract negative cycle from predecessor array
void find_negative_cycle(G& g, const Predecessors& predecessor,
    const optional<vertex_id_t<G>>& cycle_vertex_id,
    OutputIterator out_cycle);
```

## Parameters

| Parameter            | Description                                                                                                                                                      |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `g`                  | Graph satisfying `index_adjacency_list<G>`. `G` must have integral vertex IDs.                                                                                   |
| `source` / `sources` | Source vertex ID or input range of source vertex IDs. `range_value_t<Sources>` must be convertible to `vertex_id_t<G>`.                                          |
| `distances`          | Random-access range sized to `num_vertices(g)`. Filled with shortest distances. `range_value_t<Distances>` must satisfy `is_arithmetic_v`.                       |
| `predecessors`       | Random-access range sized to `num_vertices(g)`. Filled with predecessor vertex IDs. `range_value_t<Predecessors>` must be convertible from `vertex_id_t<G>`.     |
| `weight`             | Callable `WF(const G&, const edge_t<G>&) -> Distance`. Must satisfy `basic_edge_weight_function<G, WF, Distance, Compare, Combine>`. May return negative values. |
| `visitor`            | Optional visitor struct with callback methods (see Visitor Events). Default: `empty_visitor{}`.                                                                  |
| `compare`            | Comparison function `(Distance, Distance) -> bool`. Default: `std::less<>{}`.                                                                                    |
| `combine`            | Combination function `(Distance, Weight) -> Distance`. Default: `std::plus<>{}`.                                                                                 |
| `cycle_vertex_id`    | A vertex ID on the negative cycle (from the return value of the main algorithm).                                                                                 |
| `out_cycle`          | Output iterator for the cycle vertex sequence. Must satisfy `output_iterator<OutputIterator, vertex_id_t<G>>`.                                                   |

**Returns:** `[[nodiscard]] constexpr optional<vertex_id_t<G>>` — returns
`nullopt` if no negative-weight cycle is detected. Returns the id of a vertex on
a negative-weight cycle if one exists. Use `find_negative_cycle()` to extract
all cycle vertices from the predecessor array. **Always check this value** —
if a negative cycle exists, distances for affected vertices are undefined.

## Visitor Events

Bellman-Ford supports an optional visitor with the following callbacks. Vertex
events have `_id` variants (e.g., `on_discover_vertex_id(g, uid)`). You only
need to define the events you care about.

| Event                          | Called when                                                           |
| ------------------------------ | --------------------------------------------------------------------- |
| `on_discover_vertex(g, u)`     | Vertex first reached                                                  |
| `on_examine_edge(g, uv)`       | Edge examined for relaxation (each of V−1 passes)                     |
| `on_edge_relaxed(g, uv)`       | Edge improved a shorter path                                          |
| `on_edge_not_relaxed(g, uv)`   | Edge did not improve the current best path                            |
| `on_edge_minimized(g, uv)`     | **Final check pass:** edge is at minimum distance (no negative cycle) |
| `on_edge_not_minimized(g, uv)` | **Final check pass:** edge NOT at minimum (negative cycle indicator)  |

The `on_edge_minimized` / `on_edge_not_minimized` events are unique to
Bellman-Ford. They fire during the V-th pass (verification) and indicate whether
each edge satisfies the triangle inequality.

## Examples

### Example 1: Single-Source Shortest Paths

Basic usage with a graph containing negative edge weights but no negative cycle.

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
    // WARNING: dist[] values for vertices on or reachable from the cycle
    // are undefined — do not use them
}
```

### Example 3: Extracting a Negative Cycle

Use `find_negative_cycle` to obtain the full cycle from the predecessor array.

```cpp
auto cycle = bellman_ford_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

if (cycle) {
    std::vector<uint32_t> cycle_path;
    find_negative_cycle(g, pred, cycle, std::back_inserter(cycle_path));

    // cycle_path contains vertex IDs forming the negative cycle
    std::cout << "Negative cycle: ";
    for (auto v : cycle_path)
        std::cout << v << " → ";
    std::cout << cycle_path.front() << "\n";
    // Output: "Negative cycle: 0 → 1 → 2 → 0" (or rotated equivalent)
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

### Example 5: Distances Only

When you don't need predecessors (and can't reconstruct paths), use
`bellman_ford_shortest_distances` for a lighter-weight call.

```cpp
std::vector<int> dist(num_vertices(g));
init_shortest_paths(dist);

auto cycle = bellman_ford_shortest_distances(g, 0u, dist,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });

if (!cycle) {
    // dist[v] = shortest distance from source 0 to v
    // No path reconstruction available (no predecessors)
}
```

### Example 6: Visitor — Monitoring the Final Check Pass

The `on_edge_minimized` / `on_edge_not_minimized` events fire during the final
verification pass, giving you per-edge information about negative cycle
involvement.

```cpp
struct NegativeCycleInspector {
    int minimized = 0;
    int not_minimized = 0;

    void on_edge_minimized(const auto& g, const auto& uv) {
        ++minimized;
    }
    void on_edge_not_minimized(const auto& g, const auto& uv) {
        ++not_minimized;
        // This edge violates the triangle inequality —
        // it's part of or affected by a negative cycle
        std::cout << source_id(g, uv) << " → " << target_id(g, uv)
                  << " not minimized (negative cycle)\n";
    }
};

NegativeCycleInspector inspector;
auto cycle = bellman_ford_shortest_paths(g, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return edge_value(g, uv); },
    inspector);

// inspector.not_minimized > 0 if and only if cycle.has_value()
```

## Complexity

| Metric | Value                                       |
| ------ | ------------------------------------------- |
| Time   | O(V · E)                                    |
| Space  | O(1) auxiliary (beyond input/output arrays) |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- `distances` and `predecessors` must be sized to `num_vertices(g)`.
- Call `init_shortest_paths(distances, predecessors)` before invoking the algorithm.
- **Always check the return value** — if a negative cycle exists, distances are
  undefined for affected vertices.

## Postconditions

- For every source vertex `s`: `distances[s] == 0`.
- When the return value is `nullopt` (no negative cycle):
  - For every reachable vertex `v`: `distances[v]` contains the shortest
    distance from the nearest source; `predecessors[v]` contains the id of
    `v`'s predecessor on the shortest path, or `v` itself if `v` is a source.
  - For every unreachable vertex `v`: `distances[v]` is unchanged
    (typically `numeric_limits<Distance>::max()` after `init_shortest_paths`).
- When the return value is `optional<vertex_id_t<G>> id` (negative cycle detected):
  - `id` is a vertex on the negative-weight cycle. The full cycle can be
    recovered by calling `find_negative_cycle(g, predecessors, id, out)`.
  - `distances` and `predecessors` may contain intermediate values; they are
    not guaranteed to hold shortest distances for vertices on or reachable
    from the cycle.
- The graph `g` is not modified.

## Throws

- `std::out_of_range` — any source vertex id is outside `[0, num_vertices(g))`.
- `std::out_of_range` — `distances` or `predecessors` is undersized
  (size < `num_vertices(g)`).
- `std::bad_alloc` — internal allocation fails.
- Exceptions from the weight function or visitor callbacks are propagated
  unchanged.

**Exception guarantee:** Basic. If an exception is thrown, the graph `g` is
unchanged but `distances` and `predecessors` may be partially modified.

## Remarks

- The weight function must be pure: no side effects, no graph modification.
- The `O(V·E)` time bound holds regardless of graph connectivity.
- `bellman_ford_shortest_distances` internally uses a null-predecessor range,
  avoiding the cost of maintaining predecessor state. Negative-cycle detection
  is still performed and the return value is still meaningful, but path
  reconstruction is not available.
- The `on_edge_minimized` / `on_edge_not_minimized` visitor events fire only
  during the final (V-th) verification pass.

## See Also

- [Dijkstra's Shortest Paths](dijkstra.md) — faster for non-negative weights: O((V+E) log V)
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_bellman_ford_shortest_paths.cpp](../../../tests/algorithms/test_bellman_ford_shortest_paths.cpp) — test suite
