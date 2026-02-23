# Bidirectional Edge Access

> [← Back to Documentation Index](../index.md) · [Views](views.md) · [Containers](containers.md)

- [Overview](#overview)
- [Creating a Bidirectional Graph](#creating-a-bidirectional-graph)
- [Incoming Edge CPOs](#incoming-edge-cpos)
- [Incoming Edge Views](#incoming-edge-views)
- [Reverse BFS](#reverse-bfs)
- [Reverse DFS](#reverse-dfs)
- [Reverse Topological Sort](#reverse-topological-sort)
- [The Edge Accessor Pattern](#the-edge-accessor-pattern)
- [Undirected Graphs](#undirected-graphs)
- [Algorithms](#algorithms)
- [Performance Notes](#performance-notes)
- [API Reference Summary](#api-reference-summary)

## Overview

graph-v3 supports **bidirectional edge access** — querying both outgoing and
incoming edges of any vertex.  This enables:

- Finding all predecessors of a vertex
- Reverse BFS/DFS traversal (from sinks to sources)
- Reverse topological sort
- Strongly connected components via Kosaraju's algorithm
- Transpose graph construction

Bidirectional access is opt-in.  Graphs must explicitly store incoming-edge
lists to support the incoming-edge CPOs.  Two containers provide this:

| Container | How |
|-----------|-----|
| `dynamic_graph` | Set `Bidirectional = true` (sixth template parameter) |
| `undirected_adjacency_list` | Incoming edges = outgoing edges (symmetric) |

---

## Creating a Bidirectional Graph

### `dynamic_graph` with Bidirectional

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>

using namespace graph;
using namespace graph::container;

// Key: Bidirectional = true (sixth template parameter)
// EV=void, VV=void, GV=void, VId=uint32_t, Sourced=true, Bidirectional=true
using Graph = dynamic_graph<void, void, void, uint32_t, true, true,
                            vov_graph_traits<void, void, void, uint32_t, true>>;

Graph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}});

//     0 ──→ 1
//     │     │
//     ↓     ↓
//     2 ──→ 3
```

When `Bidirectional` is `true`, each vertex maintains an **incoming-edge list**
in addition to the outgoing-edge list.  Adding an edge from `u` to `v`
automatically inserts an in-edge record at vertex `v`.

> **Note:** `Sourced` must also be `true` for bidirectional graphs, because
> in-edge descriptors need to resolve their source vertex.

### `undirected_adjacency_list`

```cpp
#include <graph/container/undirected_adjacency_list.hpp>

using namespace graph::container;

undirected_adjacency_list<double> g;
// ... populate ...
```

For undirected graphs, every edge appears in both endpoints' edge lists, so
`in_edges(g, u)` returns the same edges as `out_edges(g, u)`.  The
`bidirectional_adjacency_list` concept is automatically satisfied.

---

## Incoming Edge CPOs

Once you have a bidirectional graph, the following CPOs are available:

```cpp
#include <graph/graph.hpp>

Graph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}});

auto v3 = *find_vertex(g, 3);

// Iterate incoming edges to vertex 3
for (auto&& e : in_edges(g, v3)) {
    std::cout << source_id(g, e) << " -> 3\n";
}
// Output:
//   1 -> 3
//   2 -> 3

// In-degree
std::cout << "in_degree(3) = " << in_degree(g, v3) << "\n";  // 2

// Find a specific incoming edge
auto it = find_in_edge(g, 3u, 1u);  // edge from 1 to 3
if (it != std::ranges::end(in_edges(g, v3))) {
    std::cout << "Found edge from " << source_id(g, *it) << "\n";
}

// Check existence
bool has_edge = contains_in_edge(g, 3u, 1u);  // true
```

| CPO | Parameters | Return | Complexity |
|-----|-----------|--------|------------|
| `in_edges(g, u)` | graph, vertex descriptor | `in_edge_range_t<G>` | O(1) |
| `in_edges(g, uid)` | graph, vertex id | `in_edge_range_t<G>` | O(1) |
| `in_degree(g, u)` | graph, vertex descriptor | integral | O(1) |
| `in_degree(g, uid)` | graph, vertex id | integral | O(1) |
| `find_in_edge(g, uid, vid)` | graph, target id, source id | `in_edge_iterator_t<G>` | O(in-degree) |
| `contains_in_edge(g, uid, vid)` | graph, target id, source id | `bool` | O(in-degree) |

---

## Incoming Edge Views

The library provides `in_incidence` and `in_neighbors` views that iterate over
incoming edges, mirroring the outgoing `incidence` and `neighbors` views:

```cpp
#include <graph/graph.hpp>

using namespace graph::views;

// Incoming edges to vertex 3, with structured bindings
for (auto [sid, uv] : in_incidence(g, 3)) {
    std::cout << "Edge from " << sid << "\n";
}

// Predecessor vertices of vertex 3
for (auto [sid, n] : in_neighbors(g, 3)) {
    std::cout << "Predecessor: " << sid << "\n";
}

// With value function
auto evf = [](const auto& g, auto& uv) { return edge_value(g, uv); };
for (auto [sid, uv, w] : in_incidence(g, 3, evf)) {
    std::cout << sid << " weight " << w << "\n";
}
```

### Pipe syntax

```cpp
using namespace graph::views::adaptors;

auto view = g | in_incidence(3);
auto view = g | in_neighbors(3);
```

---

## Reverse BFS

Pass `in_edge_accessor` as the first explicit template argument to any BFS
view factory to traverse **incoming** edges:

```cpp
#include <graph/views/bfs.hpp>
#include <graph/views/edge_accessor.hpp>

using namespace graph::views;

// Forward BFS from vertex 0 (default — outgoing edges)
std::cout << "Forward BFS from 0: ";
for (auto [v] : vertices_bfs(g, 0)) {
    std::cout << vertex_id(g, v) << " ";
}
// Output: 1 2 3

// Reverse BFS from vertex 3 (incoming edges)
std::cout << "\nReverse BFS from 3: ";
for (auto [v] : vertices_bfs<in_edge_accessor>(g, 3)) {
    std::cout << vertex_id(g, v) << " ";
}
// Output: 1 2 0
```

**Edge variant:**

```cpp
for (auto [uv] : edges_bfs<in_edge_accessor>(g, 3)) {
    std::cout << source_id(g, uv) << " <- " << target_id(g, uv) << "\n";
}
```

**With value function:**

```cpp
auto vvf = [](const auto& g, auto& v) { return vertex_id(g, v); };
for (auto [v, id] : vertices_bfs<in_edge_accessor>(g, 3, vvf)) {
    std::cout << "Visited vertex " << id << "\n";
}
```

---

## Reverse DFS

The same pattern works for DFS:

```cpp
#include <graph/views/dfs.hpp>
#include <graph/views/edge_accessor.hpp>

using namespace graph::views;

// Reverse DFS from vertex 3
for (auto [v] : vertices_dfs<in_edge_accessor>(g, 3)) {
    std::cout << vertex_id(g, v) << " ";
}

// Reverse DFS edges
for (auto [uv] : edges_dfs<in_edge_accessor>(g, 3)) {
    std::cout << source_id(g, uv) << " <- " << target_id(g, uv) << "\n";
}
```

---

## Reverse Topological Sort

Topological sort views also accept an Accessor:

```cpp
#include <graph/views/topological_sort.hpp>
#include <graph/views/edge_accessor.hpp>

using namespace graph::views;

// Standard topological sort (outgoing edges)
for (auto [v] : vertices_topological_sort(g)) {
    std::cout << vertex_id(g, v) << " ";
}

// Reverse topological sort (incoming edges)
for (auto [v] : vertices_topological_sort<in_edge_accessor>(g)) {
    std::cout << vertex_id(g, v) << " ";
}
```

**Safe variant with cycle detection:**

```cpp
auto result = vertices_topological_sort_safe<in_edge_accessor>(g);
if (result) {
    for (auto [v] : *result) {
        std::cout << vertex_id(g, v) << " ";
    }
} else {
    std::cerr << "Cycle detected at vertex " << result.error() << "\n";
}
```

---

## The Edge Accessor Pattern

The `out_edge_accessor` and `in_edge_accessor` are **stateless policy structs**
that bundle three operations:

| Operation | `out_edge_accessor` | `in_edge_accessor` |
|-----------|--------------------|--------------------|
| `edges(g, u)` | `out_edges(g, u)` | `in_edges(g, u)` |
| `neighbor_id(g, e)` | `target_id(g, e)` | `source_id(g, e)` |
| `neighbor(g, e)` | `target(g, e)` | `source(g, e)` |

Each accessor also exposes type aliases:

```cpp
template <class G>
using edge_range_t = ...;  // out_edge_range_t<G> or in_edge_range_t<G>

template <class G>
using edge_t = ...;        // out_edge_t<G> or in_edge_t<G>
```

**Header:** `<graph/views/edge_accessor.hpp>`

**Key design properties:**

- **Zero-cost** — accessors are stateless (`sizeof == 1`, stored with
  `[[no_unique_address]]`) and instantiated inline as `Accessor{}`.
- **Source-compatible** — all view classes default `Accessor = out_edge_accessor`,
  so existing code compiles unchanged.
- **Deducible** — pass the Accessor as the first explicit template argument:
  `vertices_bfs<in_edge_accessor>(g, seed)`.  The graph type `G` is deduced.

---

## Undirected Graphs

`undirected_adjacency_list` automatically satisfies `bidirectional_adjacency_list`
because every edge appears at both endpoints.  All incoming-edge CPOs and
reverse traversal views work without any special setup:

```cpp
#include <graph/container/undirected_adjacency_list.hpp>
#include <graph/views/bfs.hpp>
#include <graph/views/edge_accessor.hpp>

undirected_adjacency_list<void> g;
// ... populate ...

// These are equivalent for undirected graphs
for (auto [v] : views::vertices_bfs(g, 0)) { ... }
for (auto [v] : views::vertices_bfs<in_edge_accessor>(g, 0)) { ... }
```

---

## Algorithms

### Kosaraju's Strongly Connected Components

Kosaraju's algorithm finds strongly connected components in directed graphs
using two DFS passes — the second pass traverses edges in reverse. With
bidirectional graphs, this operates efficiently using `in_edge_accessor`:

```cpp
#include <graph/algorithm/connected_components.hpp>

std::vector<size_t> component(num_vertices(g));
size_t num_components = kosaraju(g, component);
```

### Transpose Graph

A `transpose_graph` view provides a transposed adjacency structure where
outgoing edges become incoming and vice versa:

```cpp
#include <graph/algorithm/transpose_graph.hpp>

auto gt = transpose_graph(g);
// gt.out_edges(v) returns g.in_edges(v)
```

---

## Performance Notes

| Operation | Bidirectional Cost | Non-Bidirectional Cost |
|-----------|-------------------|----------------------|
| Memory per edge | ~2× (out-edge + in-edge records) | 1× |
| Edge insertion | O(1) amortized (two list inserts) | O(1) amortized |
| `in_edges(g, u)` | O(1) | Not available |
| `in_degree(g, u)` | O(1) | Not available |
| `find_in_edge(g, uid, vid)` | O(in-degree) | Not available |
| Reverse BFS/DFS | Same as forward | Not available |

The Accessor parameter is compiled away entirely — `Accessor{}` is stateless and
all calls are resolved at compile time via `if constexpr` or inlining. There is
**zero runtime overhead** compared to directly calling the underlying CPOs.

---

## API Reference Summary

### CPOs

| CPO | Header | Description |
|-----|--------|-------------|
| `in_edges(g, u)` | `<graph/graph.hpp>` | Incoming edge range |
| `in_degree(g, u)` | `<graph/graph.hpp>` | Number of incoming edges |
| `find_in_edge(g, uid, vid)` | `<graph/graph.hpp>` | Find specific incoming edge |
| `contains_in_edge(g, uid, vid)` | `<graph/graph.hpp>` | Test incoming edge existence |

### Views

| View | Header | Description |
|------|--------|-------------|
| `in_incidence(g, uid)` | `<graph/graph.hpp>` | Incoming edges with structured bindings |
| `in_neighbors(g, uid)` | `<graph/graph.hpp>` | Predecessor vertices |
| `vertices_bfs<in_edge_accessor>(g, seed)` | `<graph/views/bfs.hpp>` | Reverse BFS |
| `edges_bfs<in_edge_accessor>(g, seed)` | `<graph/views/bfs.hpp>` | Reverse BFS edges |
| `vertices_dfs<in_edge_accessor>(g, seed)` | `<graph/views/dfs.hpp>` | Reverse DFS |
| `edges_dfs<in_edge_accessor>(g, seed)` | `<graph/views/dfs.hpp>` | Reverse DFS edges |
| `vertices_topological_sort<in_edge_accessor>(g)` | `<graph/views/topological_sort.hpp>` | Reverse topo sort |

### Concepts

| Concept | Header | Description |
|---------|--------|-------------|
| `bidirectional_adjacency_list<G>` | `<graph/adj_list/adjacency_list_concepts.hpp>` | Graph supports incoming edges |

### Type Aliases

| Alias | Description |
|-------|-------------|
| `in_edge_range_t<G>` | Range type of `in_edges(g, u)` |
| `in_edge_iterator_t<G>` | Iterator type for incoming edges |
| `in_edge_t<G>` | Edge descriptor type for incoming edges |

### Edge Accessors

| Accessor | Header | Description |
|----------|--------|-------------|
| `out_edge_accessor` | `<graph/views/edge_accessor.hpp>` | Default — outgoing edges |
| `in_edge_accessor` | `<graph/views/edge_accessor.hpp>` | Incoming edges |

---

## See Also

- [Views User Guide](views.md) — all graph views including `in_incidence`, `in_neighbors`
- [Containers User Guide](containers.md) — `dynamic_graph` with `Bidirectional` parameter
- [Concepts Reference](../reference/concepts.md) — `bidirectional_adjacency_list`
- [CPO Reference](../reference/cpo-reference.md) — incoming edge CPO signatures
- [Type Aliases Reference](../reference/type-aliases.md) — `in_edge_*` type aliases
- [Getting Started](../getting-started.md) — quick introduction to incoming edges
