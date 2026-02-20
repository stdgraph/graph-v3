# CPO Reference

> [← Back to Documentation Index](../index.md) · [CPO Implementation Guide](../contributing/cpo-implementation.md)

**Customization Point Objects (CPOs)** are the primary API for querying graph
structure. Each CPO resolves at compile time via a three-step priority:
1. Member function on the graph/edge type
2. ADL free function
3. Library-provided default (where available)

All CPOs listed below are available in `namespace graph` after
`#include <graph/graph.hpp>`.

---

## Quick Reference Table

| CPO | Parameters | Return Type | Complexity | Has Default? |
|-----|-----------|-------------|------------|:------------:|
| `vertices` | `(g)` | `vertex_range_t<G>` | O(1) | Yes |
| `vertices` | `(g, pid)` | `vertex_range_t<G>` | O(1) | No |
| `vertex_id` | `(g, ui)` | `vertex_id_t<G>` | O(1) | Yes |
| `find_vertex` | `(g, uid)` | `vertex_iterator_t<G>` | O(1) | Yes |
| `edges` | `(g, u)` | `vertex_edge_range_t<G>` | O(1) | Yes |
| `edges` | `(g, uid)` | `vertex_edge_range_t<G>` | O(1) | Yes |
| `target_id` | `(g, uv)` | `vertex_id_t<G>` | O(1) | Yes |
| `target` | `(g, uv)` | `vertex_t<G>&` | O(1) | Yes |
| `source_id` | `(g, uv)` | `vertex_id_t<G>` | O(1) | Yes |
| `source` | `(g, uv)` | `vertex_t<G>&` | O(1) | Yes |
| `num_vertices` | `(g)` | integral | O(1) | Yes |
| `num_vertices` | `(g, pid)` | integral | O(1) | No |
| `num_edges` | `(g)` | integral | O(V) | Yes |
| `num_edges` | `(g, u)` / `(g, uid)` | integral | O(1) | Yes |
| `degree` | `(g, u)` / `(g, uid)` | integral | O(1) | Yes |
| `find_vertex_edge` | `(g, u, v)` / `(g, uid, vid)` | `vertex_edge_iterator_t<G>` | O(deg) | Yes |
| `contains_edge` | `(g, uid, vid)` | `bool` | O(deg) | Yes |
| `has_edge` | `(g)` | `bool` | O(V) | Yes |
| `vertex_value` | `(g, u)` | deduced | O(1) | No |
| `edge_value` | `(g, uv)` | deduced | O(1) | Yes |
| `graph_value` | `(g)` | deduced | O(1) | No |
| `partition_id` | `(g, u)` | `partition_id_t<G>` | O(1) | No |
| `num_partitions` | `(g)` | integral | O(1) | Yes (1) |

---

## Vertex CPOs

### `vertices(g)` / `vertices(g, pid)`

```cpp
auto vertices(G& g)                      -> vertex_range_t<G>;
auto vertices(G& g, partition_id_t<G> pid) -> vertex_range_t<G>;
```

Returns the vertex range for graph `g`, or the vertex range for partition
`pid`.

| Property | Value |
|----------|-------|
| **Constraint** | `vertex_range<G>` |
| **Default** | `std::forward<G>(g)` (range-of-ranges) |
| **Complexity** | O(1) |

### `vertex_id(g, ui)`

```cpp
auto vertex_id(G& g, vertex_iterator_t<G> ui) -> vertex_id_t<G>;
```

Returns the vertex ID for the vertex pointed to by iterator `ui`.

| Property | Value |
|----------|-------|
| **Constraint** | `vertex_range<G>` |
| **Default** | `static_cast<vertex_id_t<G>>(ui - ranges::begin(vertices(g)))` |
| **Complexity** | O(1) |

### `find_vertex(g, uid)`

```cpp
auto find_vertex(G& g, vertex_id_t<G> uid) -> vertex_iterator_t<G>;
```

Returns an iterator to the vertex with ID `uid`.

| Property | Value |
|----------|-------|
| **Constraint** | `vertex_range<G>` |
| **Default** | `ranges::begin(vertices(g)) + uid` (random access) |
| **Complexity** | O(1) |

### `num_vertices(g)` / `num_vertices(g, pid)`

```cpp
auto num_vertices(G& g)                        -> integral;
auto num_vertices(G& g, partition_id_t<G> pid)  -> integral;
```

Returns the number of vertices.

| Property | Value |
|----------|-------|
| **Constraint** | `vertex_range<G>` |
| **Default** | `ranges::size(vertices(g))` |
| **Complexity** | O(1) |

---

## Edge CPOs

### `edges(g, u)` / `edges(g, uid)`

```cpp
auto edges(G& g, vertex_t<G>& u)       -> vertex_edge_range_t<G>;
auto edges(G& g, vertex_id_t<G> uid)    -> vertex_edge_range_t<G>;
```

Returns the edge range for vertex `u` (by descriptor or ID).

| Property | Value |
|----------|-------|
| **Constraint** | `vertex_edge_range<G>` |
| **Default** | Descriptor: dereference `u` (range-of-ranges). ID: `edges(g, *find_vertex(g, uid))` |
| **Complexity** | O(1) |

### `target_id(g, uv)`

```cpp
auto target_id(G& g, edge_t<G>& uv) -> vertex_id_t<G>;
```

Returns the target vertex ID for edge `uv`.

| Property | Value |
|----------|-------|
| **Constraint** | `edge<G>` |
| **Default** | Resolution chain: `uv.target_id(g)` → ADL → descriptor fallback → `uv.target_id` member → tuple `get<1>(uv)` (adj_list) / `get<0>(uv)` (edge list) |
| **Complexity** | O(1) |

### `target(g, uv)`

```cpp
auto target(G& g, edge_t<G>& uv) -> vertex_t<G>&;
```

Returns a reference to the target vertex for edge `uv`.

| Property | Value |
|----------|-------|
| **Constraint** | `adjacency_list<G>`-like |
| **Default** | `*find_vertex(g, target_id(g, uv))` |
| **Complexity** | O(1) |

### `source_id(g, uv)`

```cpp
auto source_id(G& g, edge_t<G>& uv) -> vertex_id_t<G>;
```

Returns the source vertex ID for edge `uv`. Not all graph models support this
(adjacency lists do via descriptors; edge lists do natively).

| Property | Value |
|----------|-------|
| **Constraint** | Edge must expose a source id |
| **Default** | Resolution chain: `uv.source_id(g)` → ADL → descriptor fallback → `uv.source_id` member → tuple `get<0>(uv)` |
| **Complexity** | O(1) |

### `source(g, uv)`

```cpp
auto source(G& g, edge_t<G>& uv) -> vertex_t<G>&;
```

Returns a reference to the source vertex for edge `uv`.

| Property | Value |
|----------|-------|
| **Default** | `*find_vertex(g, source_id(g, uv))` |
| **Complexity** | O(1) |

### `num_edges(g)` / `num_edges(g, u)`

```cpp
auto num_edges(G& g)                     -> integral;
auto num_edges(G& g, vertex_t<G>& u)     -> integral;
auto num_edges(G& g, vertex_id_t<G> uid) -> integral;
```

Returns the number of edges (whole graph or per vertex).

| Property | Value |
|----------|-------|
| **Default (whole graph)** | Sum of `ranges::size(edges(g, u))` over all vertices — **O(V)** |
| **Default (per vertex)** | `ranges::size(edges(g, u))` — **O(1)** |

### `degree(g, u)` / `degree(g, uid)`

```cpp
auto degree(G& g, vertex_t<G>& u)       -> integral;
auto degree(G& g, vertex_id_t<G> uid)    -> integral;
```

Returns the out-degree of vertex `u`.

| Property | Value |
|----------|-------|
| **Default** | `ranges::size(edges(g, u))` |
| **Complexity** | O(1) |

### `find_vertex_edge(g, ...)`

```cpp
auto find_vertex_edge(G& g, vertex_t<G>& u, vertex_t<G>& v)       -> vertex_edge_iterator_t<G>;
auto find_vertex_edge(G& g, vertex_t<G>& u, vertex_id_t<G> vid)   -> vertex_edge_iterator_t<G>;
auto find_vertex_edge(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) -> vertex_edge_iterator_t<G>;
```

Finds the edge from `u` to `v` (or `uid` to `vid`).

| Property | Value |
|----------|-------|
| **Default** | Linear scan: `ranges::find_if(edges(g, u), [&](auto& uv) { return target_id(g, uv) == vid; })` |
| **Complexity** | O(degree(u)) |

### `contains_edge(g, uid, vid)`

```cpp
auto contains_edge(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) -> bool;
```

Tests whether an edge from `uid` to `vid` exists.

| Property | Value |
|----------|-------|
| **Default** | `find_vertex_edge(g, uid, vid) != ranges::end(edges(g, uid))` |
| **Complexity** | O(degree(uid)) |

### `has_edge(g)`

```cpp
auto has_edge(G& g) -> bool;
```

Tests whether the graph has at least one edge.

| Property | Value |
|----------|-------|
| **Default** | Iterates vertices checking for non-empty edge ranges |
| **Complexity** | O(V) worst case |

---

## Value CPOs

### `vertex_value(g, u)`

```cpp
auto vertex_value(G& g, vertex_t<G>& u) -> /* deduced */;
```

Returns the user-defined value associated with vertex `u`. **No default** —
the graph must provide this via member or ADL.

### `edge_value(g, uv)`

```cpp
auto edge_value(G& g, edge_t<G>& uv) -> /* deduced */;
```

Returns the user-defined value associated with edge `uv`.

| Property | Value |
|----------|-------|
| **Default** | Resolution chain: `uv.edge_value(g)` → ADL → descriptor → `uv.value` member → tuple `get<1>(uv)` (adj_list) / `get<2>(uv)` (edge list) |
| **Complexity** | O(1) |

### `graph_value(g)`

```cpp
auto graph_value(G& g) -> /* deduced */;
```

Returns the user-defined value associated with the graph itself. **No
default** — the graph must provide this.

---

## Partition CPOs

### `partition_id(g, u)`

```cpp
auto partition_id(G& g, vertex_t<G>& u) -> partition_id_t<G>;
```

Returns the partition ID for vertex `u`. **No default** — the graph must
provide this.

### `num_partitions(g)`

```cpp
auto num_partitions(G& g) -> integral;
```

Returns the number of partitions.

| Property | Value |
|----------|-------|
| **Default** | `1` |
| **Complexity** | O(1) |

---

## Resolution Order

Every CPO follows the same three-step customization point protocol:

1. **Member function**: `g.cpo_name(args...)` or `uv.cpo_name(args...)`
2. **ADL function**: unqualified `cpo_name(args...)` found via argument-dependent lookup
3. **Default**: library-provided fallback (if available)

The `edge_value` and `target_id` CPOs have an extended resolution chain that
also checks for data members (`.value`, `.target_id`) and tuple-like access
(`get<N>`).

---

## See Also

- [Concepts Reference](concepts.md) — concepts required by these CPOs
- [Adjacency List Interface](adjacency-list-interface.md) — GCI spec
- [Edge List Interface](edge-list-interface.md) — edge list GCI spec
- [Type Aliases Reference](type-aliases.md) — return types used above
- [CPO Implementation Guide](../contributing/cpo-implementation.md) — internal implementation details
