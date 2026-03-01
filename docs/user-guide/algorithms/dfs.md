<table><tr>
<td><img src="../../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Depth-First Search

</td>
</tr></table>

> [← Back to Algorithm Catalog](../algorithms.md)

## Table of Contents
- [Overview](#overview)
- [When to Use](#when-to-use)
- [Include](#include)
- [Signature](#signature)
- [Parameters](#parameters)
- [Visitor Events](#visitor-events)
- [Examples](#examples)
  - [Basic DFS Traversal](#example-1-basic-dfs-traversal)
  - [Edge Classification](#example-2-edge-classification)
  - [Cycle Detection](#example-3-cycle-detection)
  - [Reverse Topological Order](#example-4-reverse-topological-order)
  - [Covering a Disconnected Graph](#example-5-covering-a-disconnected-graph)
  - [Using on_start_vertex and on_finish_edge](#example-6-using-on_start_vertex-and-on_finish_edge)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Depth-first search performs iterative DFS from a single source vertex using a
three-color marking scheme (White → Gray → Black). It is entirely
**visitor-driven** — the algorithm itself has no output arrays.

The graph must satisfy `index_adjacency_list<G>` — vertices are stored in a
contiguous, integer-indexed random-access range.

The three-color scheme enables precise **edge classification**:

| Edge Type | Target Color | Meaning |
|-----------|-------------|---------|
| Tree edge | White | Edge to a newly discovered vertex |
| Back edge | Gray | Edge to an ancestor (cycle indicator) |
| Forward/cross edge | Black | Edge to an already-finished vertex |

This classification makes DFS the foundation for cycle detection, topological
sorting, strongly connected components, and many other graph analyses.

> **Single-source only:** DFS has no multi-source overload. To cover all
> vertices in a disconnected graph, call DFS once per unvisited component
> (see [Example 5](#example-5-covering-a-disconnected-graph)).

## When to Use

- **Cycle detection** — back edges indicate cycles. A directed graph has a cycle
  if and only if DFS finds a back edge.
- **Edge classification** — categorize every edge as tree, back, or
  forward/cross, which is essential for many advanced algorithms.
- **Topological ordering** — the reverse of DFS finish order is a valid
  topological sort of a DAG (though the dedicated
  [topological_sort](topological_sort.md) algorithm is more convenient).
- **Connectivity analysis** — discover all vertices reachable from a source.
- **Parenthesis structure** — the discovery/finish timestamps form a nested
  parenthesis structure useful for many graph properties.

**Not suitable when:**

- You need **shortest paths** (even unweighted) → use [BFS](bfs.md).
- You need level-order traversal → use [BFS](bfs.md).

## Include

```cpp
#include <graph/algorithm/depth_first_search.hpp>
```

## Signature

```cpp
void depth_first_search(G&& g, const vertex_id_t<G>& source,
    Visitor&& visitor = empty_visitor());
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` | Source vertex ID to start DFS from |
| `visitor` | Optional visitor struct with callback methods (see below). Default: `empty_visitor{}`. |

## Visitor Events

DFS supports the richest set of visitor events among all graph-v3 algorithms.
Each vertex event also has an `_id` variant that receives `vertex_id_t<G>`
instead of a vertex reference. You only need to define the events you care about.

| Event | Called when |
|-------|------------|
| `on_initialize_vertex(g, u)` | Before traversal (source vertex only) |
| `on_start_vertex(g, u)` | DFS root vertex selected for traversal |
| `on_discover_vertex(g, u)` | Vertex first reached (White → Gray) |
| `on_examine_edge(g, uv)` | Outgoing edge examined |
| `on_tree_edge(g, uv)` | Edge to unvisited (White) vertex — part of DFS tree |
| `on_back_edge(g, uv)` | Edge to ancestor (Gray) vertex — **cycle indicator** |
| `on_forward_or_cross_edge(g, uv)` | Edge to already-finished (Black) vertex |
| `on_finish_edge(g, uv)` | All descendants of edge target fully explored |
| `on_finish_vertex(g, u)` | All adjacent edges explored (Gray → Black) |

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
// disc: vertices in discovery (pre-order) order
// fin:  vertices in finish (post-order) order
// Exact order depends on adjacency list ordering
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
// For the DAG above: tree=4, back=0, forward/cross=1 (edge 2→3)
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

// Self-loops are also detected as back edges
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

### Example 5: Covering a Disconnected Graph

DFS only visits vertices reachable from the source. For a disconnected graph,
iterate over all vertices and start DFS from each unvisited one.

```cpp
// Disconnected graph: {0,1,2} and {3,4}
Graph g({{0, 1}, {1, 2}, {3, 4}});

struct ComponentDiscoverer {
    std::vector<int>& component;
    int label;
    void on_discover_vertex(const auto& g, const auto& u) {
        component[vertex_id(g, u)] = label;
    }
};

size_t n = num_vertices(g);
std::vector<int> component(n, -1);
int label = 0;

for (uint32_t v = 0; v < n; ++v) {
    if (component[v] < 0) {
        depth_first_search(g, v, ComponentDiscoverer{component, label});
        ++label;
    }
}
// component = {0, 0, 0, 1, 1}
```

### Example 6: Using on_start_vertex and on_finish_edge

DFS has two unique events not found in BFS: `on_start_vertex` (fires when the
DFS root is selected) and `on_finish_edge` (fires when all descendants of an
edge's target have been explored — useful for computing subtree properties).

```cpp
struct SubtreeCounter {
    std::vector<int>& subtree_size;

    // When a vertex finishes, its subtree size is known
    void on_discover_vertex(const auto& g, const auto& u) {
        subtree_size[vertex_id(g, u)] = 1;  // count self
    }

    // After exploring subtree rooted at target, accumulate size to parent
    void on_finish_edge(const auto& g, const auto& uv) {
        auto uid = source_id(g, uv);
        auto vid = target_id(g, uv);
        subtree_size[uid] += subtree_size[vid];
    }

    void on_start_vertex(const auto& g, const auto& u) {
        // DFS root selected — only fires once
    }
};

// DAG: 0→1→3, 0→2→3
Graph dag({{0, 1}, {0, 2}, {1, 3}, {2, 3}});

std::vector<int> sizes(num_vertices(dag), 0);
depth_first_search(dag, 0u, SubtreeCounter{sizes});
// sizes[0] includes all descendants in the DFS tree
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
