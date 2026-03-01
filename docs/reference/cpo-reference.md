<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# CPO Reference

</td>
</tr></table>

> [← Back to Documentation Index](../index.md) · [CPO Implementation Guide](../contributing/cpo-implementation.md)


**Customization Point Objects (CPOs)** are the primary API for querying graph
structure. Each CPO resolves at compile time via a three-step priority:
1. Member function on the graph/edge type
2. ADL free function
3. Library-provided default (where available)

All CPOs listed below are available in `namespace graph` after
`#include <graph/graph.hpp>`.

> **Convention:** The explicit directional names (`out_edges`, `out_degree`,
> `find_out_edge`, `out_edge_range_t`, etc.) are the primary definitions.
> Shorter convenience aliases (`edges`, `degree`, `find_vertex_edge`,
> `vertex_edge_range_t`, `edge_t`, etc.) are provided and **preferred in
> examples and user code** for brevity.

---

## Quick Reference Table

| CPO | Parameters | Return Type | Complexity | Has Default? |
|-----|-----------|-------------|------------|:------------:|
| `vertices` | `(g)` | `vertex_range_t<G>` | O(1) | Yes |
| `vertices` | `(g, pid)` | `vertex_range_t<G>` | O(1) | No |
| `vertex_id` | `(g, ui)` | `vertex_id_t<G>` | O(1) | Yes |
| `find_vertex` | `(g, uid)` | `vertex_iterator_t<G>` | O(1) | Yes |
| `out_edges` | `(g, u)` / `(g, uid)` | `out_edge_range_t<G>` | O(1) | Yes |
| `in_edges` | `(g, u)` / `(g, uid)` | `in_edge_range_t<G>` | O(1) | Yes |
| `target_id` | `(g, uv)` | `vertex_id_t<G>` | O(1) | Yes |
| `target` | `(g, uv)` | `vertex_t<G>&` | O(1) | Yes |
| `source_id` | `(g, uv)` | `vertex_id_t<G>` | O(1) | Yes |
| `source` | `(g, uv)` | `vertex_t<G>&` | O(1) | Yes |
| `num_vertices` | `(g)` | integral | O(1) | Yes |
| `num_vertices` | `(g, pid)` | integral | O(1) | No |
| `num_edges` | `(g)` | integral | O(V) | Yes |
| `num_edges` | `(g, u)` / `(g, uid)` | integral | O(1) | Yes |
| `out_degree` | `(g, u)` / `(g, uid)` | integral | O(1) | Yes |
| `in_degree` | `(g, u)` / `(g, uid)` | integral | O(1) | Yes |
| `find_out_edge` | `(g, u, v)` / `(g, uid, vid)` | `out_edge_iterator_t<G>` | O(deg) | Yes |
| `find_in_edge` | `(g, u, v)` / `(g, uid, vid)` | `in_edge_iterator_t<G>` | O(deg) | Yes |
| `contains_out_edge` | `(g, uid, vid)` | `bool` | O(deg) | Yes |
| `contains_in_edge` | `(g, uid, vid)` | `bool` | O(deg) | Yes |
| `has_edges` | `(g)` | `bool` | O(V) | Yes |
| `vertex_value` | `(g, u)` | `decltype(auto)` | O(1) | No |
| `edge_value` | `(g, uv)` | `decltype(auto)` | O(1) | Yes |
| `graph_value` | `(g)` | `decltype(auto)` | O(1) | No |
| `partition_id` | `(g, u)` | `partition_id_t<G>` | O(1) | No |
| `num_partitions` | `(g)` | integral | O(1) | Yes (1) |

### Convenience Aliases

Several CPOs have shorter alias names that forward to the primary definition:

| Alias | Forwards To |
|-------|-------------|
| `edges(g, u)` / `edges(g, uid)` | `out_edges(g, u)` / `out_edges(g, uid)` |
| `degree(g, u)` / `degree(g, uid)` | `out_degree(g, u)` / `out_degree(g, uid)` |
| `find_vertex_edge(g, ...)` | `find_out_edge(g, ...)` |
| `contains_edge(g, uid, vid)` | `contains_out_edge(g, uid, vid)` |

Similarly for type aliases:

| Alias | Primary |
|-------|---------|
| `vertex_edge_range_t<G>` | `out_edge_range_t<G>` |
| `vertex_edge_iterator_t<G>` | `out_edge_iterator_t<G>` |
| `edge_t<G>` | `out_edge_t<G>` |

> Note: It's tempting to replace `find_vertex_edge(g, ...)` and `vertex_edge_range_t<G>` with the simpler `find_edge(g, ...)` and `edge_range_t<G>`. However, that could run into name conflicts with the abstract edge data structure and its types. They are in different namespaces to isolate the names, but it's safer to keep them the way they are until more thought is put into it. -PR 2/22/2026

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

### `out_edges(g, u)` / `out_edges(g, uid)`

Alias: `edges(g, u)` / `edges(g, uid)`

```cpp
auto out_edges(G& g, vertex_t<G>& u)       -> out_edge_range_t<G>;
auto out_edges(G& g, vertex_id_t<G> uid)    -> out_edge_range_t<G>;
```

Returns the outgoing edge range for vertex `u` (by descriptor or ID).

| Property | Value |
|----------|-------|
| **Constraint** | `out_edge_range<G>` |
| **Default** | Descriptor: dereference `u` (range-of-ranges). ID: `out_edges(g, *find_vertex(g, uid))` |
| **Complexity** | O(1) |

### `target_id(g, uv)`

```cpp
auto target_id(G& g, out_edge_t<G>& uv) -> vertex_id_t<G>;
```

Returns the target vertex ID for edge `uv`.

| Property | Value |
|----------|-------|
| **Constraint** | `edge<G>` |
| **Default** | Resolution chain: `uv.target_id(g)` → ADL → descriptor fallback → `uv.target_id` member → tuple `get<1>(uv)` (adj_list) / `get<0>(uv)` (edge list) |
| **Complexity** | O(1) |

### `target(g, uv)`

```cpp
auto target(G& g, out_edge_t<G>& uv) -> vertex_t<G>&;
```

Returns a reference to the target vertex for edge `uv`.

| Property | Value |
|----------|-------|
| **Constraint** | `adjacency_list<G>`-like |
| **Default** | `*find_vertex(g, target_id(g, uv))` |
| **Complexity** | O(1) |

### `source_id(g, uv)`

```cpp
auto source_id(G& g, out_edge_t<G>& uv) -> vertex_id_t<G>;
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
auto source(G& g, out_edge_t<G>& uv) -> vertex_t<G>&;
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

### `out_degree(g, u)` / `out_degree(g, uid)`

Alias: `degree(g, u)` / `degree(g, uid)`

```cpp
auto out_degree(G& g, vertex_t<G>& u)       -> integral;
auto out_degree(G& g, vertex_id_t<G> uid)    -> integral;
```

Returns the out-degree of vertex `u`.

| Property | Value |
|----------|-------|
| **Default** | `ranges::size(out_edges(g, u))` |
| **Complexity** | O(1) |

### `find_out_edge(g, ...)`

Alias: `find_vertex_edge(g, ...)`

```cpp
auto find_out_edge(G& g, vertex_t<G>& u, vertex_t<G>& v)       -> out_edge_iterator_t<G>;
auto find_out_edge(G& g, vertex_t<G>& u, vertex_id_t<G> vid)   -> out_edge_iterator_t<G>;
auto find_out_edge(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) -> out_edge_iterator_t<G>;
```

Finds the outgoing edge from `u` to `v` (or `uid` to `vid`).

| Property | Value |
|----------|-------|
| **Default** | Linear scan: `ranges::find_if(out_edges(g, u), [&](auto& uv) { return target_id(g, uv) == vid; })` |
| **Complexity** | O(degree(u)) |

### `contains_out_edge(g, uid, vid)`

Alias: `contains_edge(g, uid, vid)`

```cpp
auto contains_out_edge(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) -> bool;
```

Tests whether an outgoing edge from `uid` to `vid` exists.

| Property | Value |
|----------|-------|
| **Default** | `find_vertex_edge(g, uid, vid) != ranges::end(edges(g, uid))` |
| **Complexity** | O(degree(uid)) |

### `has_edges(g)`

```cpp
auto has_edges(G& g) -> bool;
```

Tests whether the graph has at least one edge.

| Property | Value |
|----------|-------|
| **Default** | Iterates vertices checking for non-empty edge ranges |
| **Complexity** | O(V) worst case |

---

## Incoming Edge CPOs

### `in_edges(g, u)` / `in_edges(g, uid)`

```cpp
auto in_edges(G& g, vertex_t<G>& u)       -> in_edge_range_t<G>;
auto in_edges(G& g, vertex_id_t<G> uid)    -> in_edge_range_t<G>;
```

Returns the incoming edge range for vertex `u` (by descriptor or ID).

| Property | Value |
|----------|-------|
| **Constraint** | `in_edge_range<G>` |
| **Default** | Descriptor: `u.in_edges(g)` → ADL. ID: `in_edges(g, *find_vertex(g, uid))` |
| **Complexity** | O(1) |

### `in_degree(g, u)` / `in_degree(g, uid)`

```cpp
auto in_degree(G& g, vertex_t<G>& u)       -> integral;
auto in_degree(G& g, vertex_id_t<G> uid)    -> integral;
```

Returns the in-degree of vertex `u`.

| Property | Value |
|----------|-------|
| **Default** | `ranges::size(in_edges(g, u))` |
| **Complexity** | O(1) |

### `find_in_edge(g, ...)`

```cpp
auto find_in_edge(G& g, vertex_t<G>& u, vertex_t<G>& v)       -> in_edge_iterator_t<G>;
auto find_in_edge(G& g, vertex_t<G>& u, vertex_id_t<G> vid)   -> in_edge_iterator_t<G>;
auto find_in_edge(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) -> in_edge_iterator_t<G>;
```

Finds the incoming edge to `u` from `v` (or `uid` from `vid`).

| Property | Value |
|----------|-------|
| **Default** | Linear scan: `ranges::find_if(in_edges(g, u), [&](auto& uv) { return source_id(g, uv) == vid; })` |
| **Complexity** | O(in_degree(u)) |

### `contains_in_edge(g, uid, vid)`

```cpp
auto contains_in_edge(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) -> bool;
```

Tests whether an incoming edge to `uid` from `vid` exists.

| Property | Value |
|----------|-------|
| **Default** | `find_in_edge(g, uid, vid) != ranges::end(in_edges(g, uid))` |
| **Complexity** | O(in_degree(uid)) |

---

## Value CPOs

All three value CPOs (`vertex_value`, `edge_value`, `graph_value`) return
`decltype(auto)`, which **preserves the exact return type** of the resolved
member, ADL, or default function.  Return-by-value (`T`),
return-by-reference (`T&`), return-by-const-reference (`const T&`), and
return-by-rvalue-reference (`T&&`) are all faithfully forwarded without
decay or copy.

### `vertex_value(g, u)`

```cpp
auto vertex_value(G& g, vertex_t<G>& u) -> /* decltype(auto) */;
```

Returns the user-defined value associated with vertex `u`. **No default** —
the graph must provide this via member or ADL.

| Property | Value |
|----------|-------|
| **Return type** | `decltype(auto)` — preserves by-value, by-ref, by-const-ref, and by-rvalue-ref |
| **Complexity** | O(1) |

### `edge_value(g, uv)`

```cpp
auto edge_value(G& g, edge_t<G>& uv) -> /* decltype(auto) */;
```

Returns the user-defined value associated with edge `uv`.

| Property | Value |
|----------|-------|
| **Default** | Resolution chain: `uv.edge_value(g)` → ADL → descriptor → `uv.value` member → tuple `get<1>(uv)` (adj_list) / `get<2>(uv)` (edge list) |
| **Return type** | `decltype(auto)` — preserves by-value, by-ref, by-const-ref, and by-rvalue-ref |
| **Complexity** | O(1) |

### `graph_value(g)`

```cpp
auto graph_value(G& g) -> /* decltype(auto) */;
```

Returns the user-defined value associated with the graph itself. **No
default** — the graph must provide this.

| Property | Value |
|----------|-------|
| **Return type** | `decltype(auto)` — preserves by-value, by-ref, by-const-ref, and by-rvalue-ref |
| **Complexity** | O(1) |

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
