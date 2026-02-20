# Breadth-First Search

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Level-Order Traversal](#example-1-level-order-traversal)
  - [Multi-Source BFS](#example-2-multi-source-bfs)
  - [Computing Distances (Unweighted)](#example-3-computing-distances-unweighted)
  - [Counting Events with a Visitor](#example-4-counting-events-with-a-visitor)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Breadth-first search explores vertices in level-order (FIFO) from one or more
source vertices. It is entirely **visitor-driven** — the algorithm itself has no
output arrays. Instead, you provide a visitor struct whose callback methods are
invoked at each stage of the traversal.

BFS is the foundation for unweighted shortest paths, connected-component
discovery, and level-based graph analysis.

## Include

```cpp
#include <graph/algorithm/breadth_first_search.hpp>
```

## Signatures

```cpp
// Multi-source BFS
void breadth_first_search(G&& g, const Sources& sources,
    Visitor&& visitor = empty_visitor());

// Single-source BFS
void breadth_first_search(G&& g, vertex_id_t<G> source,
    Visitor&& visitor = empty_visitor());
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` / `sources` | Source vertex ID or range of source vertex IDs |
| `visitor` | Optional visitor struct with callback methods (see below) |

## Visitor Events

| Event | Called when |
|-------|------------|
| `on_initialize_vertex(g, u)` | Before traversal, for each vertex during color initialization |
| `on_discover_vertex(g, u)` | Vertex first reached (pushed into queue) |
| `on_examine_vertex(g, u)` | Vertex popped from queue for processing |
| `on_examine_edge(g, uv)` | Outgoing edge examined during vertex processing |
| `on_finish_vertex(g, u)` | All adjacent edges of vertex explored |

Each event also has an `_id` variant that receives `vertex_id_t<G>` instead
of vertex/edge references.

## Examples

### Example 1: Level-Order Traversal

Record the order in which vertices are discovered.

```cpp
#include <graph/algorithm/breadth_first_search.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

Graph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}});

struct DiscoveryOrder {
    std::vector<uint32_t>& order;
    void on_discover_vertex(const auto& g, const auto& u) {
        order.push_back(vertex_id(g, u));
    }
};

std::vector<uint32_t> order;
breadth_first_search(g, 0u, DiscoveryOrder{order});
// order = {0, 1, 2, 3, 4}  (level-order from vertex 0)
```

### Example 2: Multi-Source BFS

Start BFS from multiple source vertices simultaneously — useful for
nearest-facility queries or multi-root traversals.

```cpp
struct DiscoveryOrder {
    std::vector<uint32_t>& order;
    void on_discover_vertex(const auto& g, const auto& u) {
        order.push_back(vertex_id(g, u));
    }
};

std::vector<uint32_t> order;
std::vector<uint32_t> sources = {0u, 4u};
breadth_first_search(g, sources, DiscoveryOrder{order});
// Both sources are discovered at level 0, then their neighbors at level 1, etc.
```

### Example 3: Computing Distances (Unweighted)

BFS naturally computes shortest hop-count distances in unweighted graphs.

```cpp
struct DistanceRecorder {
    std::vector<int>& dist;

    void on_discover_vertex(const auto& g, const auto& u) {
        // Source vertex already set to 0
    }
    void on_examine_edge(const auto& g, const auto& uv) {
        auto uid = source_id(g, uv);
        auto vid = target_id(g, uv);
        if (dist[vid] < 0) {  // unvisited
            dist[vid] = dist[uid] + 1;
        }
    }
};

std::vector<int> dist(num_vertices(g), -1);
dist[0] = 0;
breadth_first_search(g, 0u, DistanceRecorder{dist});
// dist[v] = shortest hop count from vertex 0 to v, or -1 if unreachable
```

### Example 4: Counting Events with a Visitor

Track how many times each event fires — useful for testing and debugging.

```cpp
struct CountingVisitor {
    int initialized = 0, discovered = 0, examined = 0;
    int edges_examined = 0, finished = 0;

    void on_initialize_vertex(const auto&, const auto&) { ++initialized; }
    void on_discover_vertex(const auto&, const auto&)    { ++discovered; }
    void on_examine_vertex(const auto&, const auto&)     { ++examined; }
    void on_examine_edge(const auto&, const auto&)       { ++edges_examined; }
    void on_finish_vertex(const auto&, const auto&)      { ++finished; }
};

CountingVisitor vis;
breadth_first_search(g, 0u, vis);
// vis.discovered == num reachable vertices
// vis.edges_examined == num edges explored
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V) for the color map and queue |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- Duplicate sources are allowed — each duplicate causes an extra discovery event.

## See Also

- [DFS](dfs.md) — depth-first traversal with edge classification
- [Views User Guide](../views.md) — `vertices_bfs` / `edges_bfs` lazy view wrappers
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_breadth_first_search.cpp](../../../tests/algorithms/test_breadth_first_search.cpp) — test suite
