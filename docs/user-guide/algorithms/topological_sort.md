# Topological Sort

> [← Back to Algorithm Catalog](../algorithms.md)

- [Overview](#overview)
- [Include](#include)
- [Signatures](#signatures)
- [Parameters](#parameters)
- [Examples](#examples)
  - [Full-Graph Topological Sort](#example-1-full-graph-topological-sort)
  - [Single-Source Topological Sort](#example-2-single-source-topological-sort)
  - [Cycle Detection](#example-3-cycle-detection)
  - [Multi-Source Topological Sort](#example-4-multi-source-topological-sort)
- [Complexity](#complexity)
- [Preconditions](#preconditions)
- [See Also](#see-also)

## Overview

Topological sort produces a linear ordering of vertices in a **directed acyclic
graph (DAG)** such that for every directed edge (u, v), vertex u appears before
vertex v in the ordering.

The algorithm returns `bool` — `true` if the graph is a valid DAG (ordering
produced), `false` if a cycle is detected (partial output may have been written).

Three overloads cover different use cases:

- **Full-graph** — visits all vertices, produces a complete topological order.
- **Single-source** — produces an ordering of all vertices reachable from a
  specific source.
- **Multi-source** — produces an ordering of all vertices reachable from any
  source in the set.

## Include

```cpp
#include <graph/algorithm/topological_sort.hpp>
```

## Signatures

```cpp
// Full-graph topological sort
bool topological_sort(const G& g, OutputIterator result);

// Single-source topological sort
bool topological_sort(const G& g, vertex_id_t<G> source, OutputIterator result);

// Multi-source topological sort
bool topological_sort(const G& g, const Sources& sources, OutputIterator result);
```

## Parameters

| Parameter | Description |
|-----------|-------------|
| `g` | Graph satisfying `index_adjacency_list` |
| `source` / `sources` | Source vertex ID or range of source vertex IDs |
| `result` | Output iterator receiving vertex IDs in topological order |

**Return value:** `true` if the graph is a DAG (valid ordering), `false` if a
cycle was detected.

## Examples

### Example 1: Full-Graph Topological Sort

Sort all vertices in a DAG — the most common use case.

```cpp
#include <graph/algorithm/topological_sort.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>

using Graph = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void>>;

// DAG: task dependencies
// 0 → 1 → 3
// 0 → 2 → 3 → 4
Graph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}});

std::vector<uint32_t> order;
bool is_dag = topological_sort(g, std::back_inserter(order));

if (is_dag) {
    // order is a valid topological ordering
    // e.g. {0, 2, 1, 3, 4} — vertex 0 before 1 and 2, vertex 3 before 4
}
```

### Example 2: Single-Source Topological Sort

Sort only the portion of the graph reachable from a specific vertex.

```cpp
// Same DAG, but only sort from vertex 2 forward
std::vector<uint32_t> order;
bool ok = topological_sort(g, 2u, std::back_inserter(order));
// order contains vertices reachable from 2 in topological order
// e.g. {2, 3, 4}
```

### Example 3: Cycle Detection

If the graph contains a cycle, `topological_sort` returns `false`.

```cpp
// Graph with cycle: 0→1→2→0
Graph cyclic({{0, 1}, {1, 2}, {2, 0}});

std::vector<uint32_t> order;
bool is_dag = topological_sort(cyclic, std::back_inserter(order));
// is_dag == false — cycle detected

// Self-loops also prevent topological ordering
Graph self_loop({{0, 0}, {0, 1}});
bool ok = topological_sort(self_loop, std::back_inserter(order));
// ok == false
```

### Example 4: Multi-Source Topological Sort

Start from specific root vertices, useful when the DAG has multiple entry points.

```cpp
// DAG with two independent entry points
// 0 → 2 → 4
// 1 → 3 → 4
Graph g({{0, 2}, {1, 3}, {2, 4}, {3, 4}});

std::vector<uint32_t> order;
std::vector<uint32_t> sources{0u, 1u};
bool ok = topological_sort(g, sources, std::back_inserter(order));
// order contains all reachable vertices from sources {0, 1}
```

## Complexity

| Metric | Value |
|--------|-------|
| Time | O(V + E) |
| Space | O(V) for the color map |

## Preconditions

- Graph must satisfy `index_adjacency_list<G>`.
- The graph should be a DAG for a valid ordering. If cycles exist, the function
  returns `false`.
- Isolated vertices are included in full-graph sort but excluded from source-based
  sorts if not reachable.

## See Also

- [DFS](dfs.md) — underlying traversal; finish-order reversal also yields topological order
- [Views User Guide](../views.md) — `vertices_topological_sort` / `edges_topological_sort` lazy views
- [Algorithm Catalog](../algorithms.md) — full list of algorithms
- [test_topological_sort.cpp](../../../tests/algorithms/test_topological_sort.cpp) — test suite
