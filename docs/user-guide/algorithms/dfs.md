# Depth-First Search

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Basic DFS Traversal](#example-1-basic-dfs-traversal)
  - [Edge Classification](#example-2-edge-classification)
  - [Cycle Detection](#example-3-cycle-detection)
  - [Reverse Topological Order](#example-4-reverse-topological-order)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Depth-first search performs iterative DFS from a single source vertex using a
three-color marking scheme (White → Gray → Black). It is entirely
**visitor-driven** — the algorithm itself has no output arrays.

The three-color scheme enables precise **edge classification**:

| Edge Type | Target Color | Meaning |
|-----------|-------------|---------|
| Tree edge | White | Edge to a newly discovered vertex |
| Back edge | Gray | Edge to an ancestor (cycle indicator) |
| Forward/cross edge | Black | Edge to an already-finished vertex |

This classification makes DFS the foundation for cycle detection, topological
sorting, strongly connected components, and many other graph analyses.

## Include

```cpp
#include <graph/algorithm/depth_first_search.hpp>
```

## Signature

```cpp
void depth_first_search(G&& g, const vertex_id_t<G>& source,
    Visitor&& visitor = empty_visitor());
```

> **Note:** DFS has a single-source-only interface — there is no multi-source
> overload.  `on_initialize_vertex` is called only for the source vertex.

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` | Source vertex ID to start DFS from |
| `visitor` | Optional visitor struct with callback methods (see below) |

## Visitor Events

| Event | Called when |
|-------|------------|
| `on_initialize_vertex(g, u)` | Before traversal (source vertex only) |
| `on_start_vertex(g, u)` | DFS root vertex selected |
| `on_discover_vertex(g, u)` | Vertex first reached (White → Gray) |
| `on_examine_edge(g, uv)` | Outgoing edge examined |
| `on_tree_edge(g, uv)` | Edge to unvisited vertex (tree edge) |
| `on_back_edge(g, uv)` | Edge to ancestor vertex (back edge — cycle indicator) |
| `on_forward_or_cross_edge(g, uv)` | Edge to already-finished vertex |
| `on_finish_edge(g, uv)` | All descendants of edge target explored |
| `on_finish_vertex(g, u)` | All adjacent edges explored (Gray → Black) |

Each event also has an `_id` variant that receives `vertex_id_t<G>` instead
of vertex/edge references.

## Examples

### Example 1: Basic DFS Traversal

Record the discovery and finish order of vertices.

```cpp
#include <graph/algorithm/depth_first_search.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// Directed graph: 0→1, 0→2, 1→3, 2→3, 3→4
Graph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}});

struct OrderTracker {
    std::vector<uint32_t>& discovered;
    std::vector<uint32_t>& finished;

    void on_discover_vertex(const auto& g, const auto& u) {
        discovered.push_back(vertex_id(g, u));
    }
    void on_finish_vertex(const auto& g, const auto& u) {
        finished.push_back(vertex_id(g, u));
    }
};

std::vector<uint32_t> disc, fin;
depth_first_search(g, 0u, OrderTracker{disc, fin});
// DFS discovery and finish ordering depend on adjacency order
```

### Example 2: Edge Classification

Classify every edge in the graph as tree, back, or forward/cross.

```cpp
struct EdgeClassifier {
    int tree_edges = 0;
    int back_edges = 0;
    int forward_or_cross_edges = 0;

    void on_tree_edge(const auto&, const auto&)              { ++tree_edges; }
    void on_back_edge(const auto&, const auto&)              { ++back_edges; }
    void on_forward_or_cross_edge(const auto&, const auto&)  { ++forward_or_cross_edges; }
};

EdgeClassifier ec;
depth_first_search(g, 0u, ec);
// ec.tree_edges = number of tree edges in DFS tree
// ec.back_edges >= 1 means a cycle exists
```

### Example 3: Cycle Detection

A directed graph has a cycle if and only if DFS finds a back edge.

```cpp
// Graph with cycle: 0→1→2→0
Graph cyclic({{0, 1}, {1, 2}, {2, 0}});

struct CycleDetector {
    bool has_cycle = false;
    void on_back_edge(const auto&, const auto&) { has_cycle = true; }
};

CycleDetector cd;
depth_first_search(cyclic, 0u, cd);
// cd.has_cycle == true

// Self-loops also count as back edges
Graph self_loop({{0, 0}});
CycleDetector cd2;
depth_first_search(self_loop, 0u, cd2);
// cd2.has_cycle == true
```

### Example 4: Reverse Topological Order

The finish order of DFS on a DAG is the reverse of a topological ordering.

```cpp
// DAG: 0→1, 0→2, 1→3, 2→3
Graph dag({{0, 1}, {0, 2}, {1, 3}, {2, 3}});

struct FinishOrder {
    std::vector<uint32_t>& order;
    void on_finish_vertex(const auto& g, const auto& u) {
        order.push_back(vertex_id(g, u));
    }
};

std::vector<uint32_t> finish_order;
depth_first_search(dag, 0u, FinishOrder{finish_order});

// Reverse of finish_order is a valid topological order
std::ranges::reverse(finish_order);
// For the dedicated topological sort algorithm, see topological_sort.md
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V) for the color map and stack |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- Single-source only — to cover all vertices in a disconnected graph, call
  DFS once per unvisited component.

## See Also

- [BFS](bfs.md) — breadth-first (level-order) traversal
- [Topological Sort](topological_sort.md) — dedicated algorithm for DAG ordering
- [Views User Guide](../views.md) — `vertices_dfs` / `edges_dfs` lazy view wrappers
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_depth_first_search.cpp](../../../tests/algorithms/test_depth_first_search.cpp) — test suite
