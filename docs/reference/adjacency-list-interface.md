# Adjacency List Interface Reference

> [← Back to Documentation Index](../index.md) · [User Guide: Adjacency Lists](../user-guide/adjacency-lists.md)

This is the formal **Graph Container Interface (GCI)** specification for
adjacency lists, derived from P1709/D3130.

---

## Concepts

All concepts are in `graph::adj_list`, re-exported to `graph`.

**Qualifier conventions in concept names:**

- **`index`** — vertex range is random-access with integral vertex IDs
- **`sourced`** — edge descriptor carries a source vertex ID

### Edge Concepts

| Concept | Parameters | Description |
|---------|------------|-------------|
| `edge<G, E>` | Graph `G`, edge `E` | `E` is an `edge_descriptor`; `source_id(g,e)`, `source(g,e)`, `target_id(g,e)`, `target(g,e)` are valid |

### Edge Range Concepts

| Concept | Parameters | Description |
|---------|------------|-------------|
| `out_edge_range<R, G>` | Range `R`, graph `G` | `forward_range<R>` whose values satisfy `edge<G, …>` |

### Vertex Concepts

| Concept | Parameters | Description |
|---------|------------|-------------|
| `vertex<G, V>` | Graph `G`, vertex `V` | `V` is a `vertex_descriptor`; `vertex_id(g,u)` and `find_vertex(g,uid)` are valid |
| `vertex_range<R, G>` | Range `R`, graph `G` | `forward_range<R>` + `sized_range<R>` whose values satisfy `vertex<G, …>` |
| `index_vertex_range<G>` | Graph `G` | `vertex_range` + `vertex_id_t<G>` is `std::integral` + vertex range iterator is `random_access_iterator` |

### Adjacency List Concepts

| Concept | Parameters | Description |
|---------|------------|-------------|
| `adjacency_list<G>` | Graph `G` | `vertices(g)` → `vertex_range`, `edges(g,u)` → `out_edge_range` |
| `index_adjacency_list<G>` | Graph `G` | `adjacency_list<G>` + `index_vertex_range<G>` (random-access vertices, integral IDs) |
| `ordered_vertex_edges<G>` | Graph `G` | `adjacency_list<G>` + edge ranges sorted by `target_id` |
| `bidirectional_adjacency_list<G>` | Graph `G` | `index_adjacency_list<G>` + `in_edges(g,u)` and `in_degree(g,u)` |

> **Note:** All current algorithms require `index_adjacency_list<G>`. The more
> general `adjacency_list<G>` supports associative vertex containers and may be
> used by future algorithms.

---

## Type Traits

| Trait | Type | Description |
|-------|------|-------------|
| `has_degree<G>` / `has_degree_v<G>` | concept / bool | Is `degree(g,u)` available? |
| `has_find_vertex<G>` / `has_find_vertex_v<G>` | concept / bool | Is `find_vertex(g,uid)` available? |
| `has_find_vertex_edge<G>` / `has_find_vertex_edge_v<G>` | concept / bool | Is `find_vertex_edge(g,…)` available? |
| `has_contains_edge<G>` / `has_contains_edge_v<G>` | concept / bool | Is `contains_edge(g,uid,vid)` available? |
| `has_basic_queries<G>` / `has_basic_queries_v<G>` | concept / bool | Graph supports `degree`, `find_vertex`, and `find_vertex_edge` |
| `has_full_queries<G>` / `has_full_queries_v<G>` | concept / bool | `has_basic_queries` + `has_contains_edge` |
| `unordered_edge<G>` | concept | Graph has unordered (undirected) edges |
| `ordered_edge<G>` | concept | Graph has ordered (directed) edges |
| `define_adjacency_matrix<G>` | struct (`false_type`) | Specialize to `true_type` for adjacency matrix |
| `is_adjacency_matrix<G>` / `is_adjacency_matrix_v<G>` | struct / bool | Type trait for adjacency matrix |
| `adjacency_matrix<G>` | concept | Graph is an adjacency matrix |

---

## Type Aliases

All follow the `_t<G>` naming convention.

### Graph Types

| Alias | Definition |
|-------|------------|
| `graph_reference_t<G>` | `std::add_lvalue_reference_t<G>` |

### Vertex Types

| Alias | Definition |
|-------|------------|
| `vertex_range_t<G>` | `decltype(vertices(g))` |
| `vertex_iterator_t<G>` | `std::ranges::iterator_t<vertex_range_t<G>>` |
| `vertex_t<G>` | `std::ranges::range_value_t<vertex_range_t<G>>` |
| `vertex_id_t<G>` | `decltype(vertex_id(g, u))` |

### Edge Types

| Alias | Definition |
|-------|------------|
| `out_edge_range_t<G>` | `decltype(out_edges(g, u))` |
| `out_edge_iterator_t<G>` | `std::ranges::iterator_t<out_edge_range_t<G>>` |
| `out_edge_t<G>` | `std::ranges::range_value_t<out_edge_range_t<G>>` |

> **Convenience aliases:** `vertex_edge_range_t<G>`, `vertex_edge_iterator_t<G>`, and `edge_t<G>` remain available as aliases for the above.

### Incoming Edge Types (Bidirectional)

Available on graphs satisfying `bidirectional_adjacency_list<G>`.

| Alias | Definition |
|-------|------------|
| `in_edge_range_t<G>` | `decltype(in_edges(g, u))` |
| `in_edge_iterator_t<G>` | `std::ranges::iterator_t<in_edge_range_t<G>>` |
| `in_edge_t<G>` | `std::ranges::range_value_t<in_edge_range_t<G>>` |

### Value Types (Optional)

| Alias | Definition |
|-------|------------|
| `graph_value_t<G>` | `decltype(graph_value(g))` |
| `vertex_value_t<G>` | `decltype(vertex_value(g, u))` |
| `edge_value_t<G>` | `decltype(edge_value(g, uv))` |

### Partition Types (Optional)

| Alias | Definition |
|-------|------------|
| `partition_id_t<G>` | `decltype(partition_id(g, u))` |
| `partition_vertex_range_t<G>` | `decltype(vertices(g, pid))` |

---

## Functions (CPOs)

All functions are Customization Point Objects (CPOs) following the Niebloid
pattern. See [CPO Reference](cpo-reference.md) for full resolution order.

### Graph Functions

| Function | Return Type | Default Complexity | Default Implementation |
|----------|-------------|-------------------|------------------------|
| `graph_value(g)` | `graph_value_t<G>` | O(1) | n/a (optional) |
| `vertices(g)` | `vertex_range_t<G>` | O(1) | `vertex_descriptor_view(g)` if inner-value pattern matches |
| `vertices(g, pid)` | `partition_vertex_range_t<G>` | O(1) | `vertices(g)` |
| `num_vertices(g)` | integral | O(1) | `size(vertices(g))` |
| `num_vertices(g, pid)` | integral | O(1) | `size(vertices(g))` |
| `num_edges(g)` | integral | O(V + E) | Sum of `distance(edges(g, u))` over all vertices |
| `has_edges(g)` | bool | O(V) | First vertex with non-empty edge range |
| `num_partitions(g)` | integral | O(1) | `1` |

### Vertex Functions

| Function | Return Type | Default Complexity | Default Implementation |
|----------|-------------|-------------------|------------------------|
| `find_vertex(g, uid)` | `vertex_iterator_t<G>` | O(1) if random-access | `begin(vertices(g)) + uid` |
| `vertex_id(g, u)` | `vertex_id_t<G>` | O(1) | Index or key from descriptor |
| `vertex_value(g, u)` | `vertex_value_t<G>` | O(1) | n/a (optional) |
| `vertex_value(g, uid)` | `vertex_value_t<G>` | O(1) | `vertex_value(g, *find_vertex(g, uid))` |
| `degree(g, u)` | integral | O(1) | `size(edges(g, u))` if `sized_range` |
| `degree(g, uid)` | integral | O(1) | `degree(g, *find_vertex(g, uid))` |
| `edges(g, u)` | `vertex_edge_range_t<G>` | O(1) | `edge_descriptor_view(u.inner_value(), u)` if edge-value pattern matches |
| `edges(g, uid)` | `vertex_edge_range_t<G>` | O(1) | `edges(g, *find_vertex(g, uid))` |
| `partition_id(g, u)` | `partition_id_t<G>` | O(1) | `0` |
| `partition_id(g, uid)` | `partition_id_t<G>` | O(1) | `0` |

### Edge Functions

| Function | Return Type | Default Complexity | Default Implementation |
|----------|-------------|-------------------|------------------------|
| `target_id(g, uv)` | `vertex_id_t<G>` | O(1) | Automatic for integral, pair, tuple patterns |
| `target(g, uv)` | `vertex_t<G>` | O(1) | `*(begin(vertices(g)) + target_id(g, uv))` |
| `source_id(g, uv)` | `vertex_id_t<G>` | O(1) | From edge descriptor (optional) |
| `source(g, uv)` | `vertex_t<G>` | O(1) | `*(begin(vertices(g)) + source_id(g, uv))` |
| `edge_value(g, uv)` | `edge_value_t<G>` | O(1) | Automatic for pair, tuple patterns (optional) |
| `find_vertex_edge(g, u, vid)` | `vertex_edge_iterator_t<G>` | O(degree) | Linear search through `edges(g, u)` |
| `find_vertex_edge(g, uid, vid)` | `vertex_edge_iterator_t<G>` | O(degree) | `find_vertex_edge(g, *find_vertex(g, uid), vid)` |
| `contains_edge(g, uid, vid)` | bool | O(degree) | `find_vertex_edge(g, uid, vid) != end(edges(g, uid))` |

### Incoming Edge Functions (Bidirectional)

Available on graphs satisfying `bidirectional_adjacency_list<G>`.

| Function | Return Type | Default Complexity | Default Implementation |
|----------|-------------|-------------------|------------------------|
| `in_edges(g, u)` | `in_edge_range_t<G>` | O(1) | From container’s incoming-edge list |
| `in_edges(g, uid)` | `in_edge_range_t<G>` | O(1) | `in_edges(g, *find_vertex(g, uid))` |
| `in_degree(g, u)` | integral | O(1) | `size(in_edges(g, u))` |
| `in_degree(g, uid)` | integral | O(1) | `in_degree(g, *find_vertex(g, uid))` |
| `find_in_edge(g, u, vid)` | `in_edge_iterator_t<G>` | O(in-degree) | Linear search through `in_edges(g, u)` |
| `find_in_edge(g, uid, vid)` | `in_edge_iterator_t<G>` | O(in-degree) | `find_in_edge(g, *find_vertex(g, uid), vid)` |
| `contains_in_edge(g, uid, vid)` | bool | O(in-degree) | `find_in_edge(g, uid, vid) != end(in_edges(g, uid))` |

---

## Descriptor System

### Descriptors

Descriptors are opaque objects representing vertices and edges. They abstract
over the underlying container's iterator/index representation.

| Descriptor | Description |
|------------|-------------|
| `vertex_descriptor` | Wraps a vertex iterator or index. Provides `vertex_id()` and `inner_value()`. |
| `edge_descriptor` | Wraps an edge iterator or index. Provides `target_id()`, `source_id()`, and `inner_value()`. |

**Properties:** equality comparison, ordering (if supported), copy/assignment,
default construction, `inner_value()` accessor.

### Descriptor Views

| View | Description |
|------|-------------|
| `vertex_descriptor_view` | Range adaptor that wraps a vertex container into a range of `vertex_descriptor` |
| `edge_descriptor_view` | Range adaptor that wraps an edge container into a range of `edge_descriptor` |

### Descriptor Concepts

| Concept / Trait | Description |
|-----------------|-------------|
| `descriptor_type<T>` | `T` is any descriptor |
| `vertex_descriptor_type<T>` | `T` is a `vertex_descriptor` |
| `edge_descriptor_type<T>` | `T` is an `edge_descriptor` |
| `vertex_iterator<T>` | `T` is a valid vertex iterator (direct or keyed) |
| `edge_iterator<T>` | `T` is a valid edge iterator |
| `is_descriptor_v<T>` | `true` if `T` is a descriptor |
| `is_vertex_descriptor_v<T>` | `true` if `T` is a `vertex_descriptor` |
| `is_edge_descriptor_v<T>` | `true` if `T` is an `edge_descriptor` |
| `is_vertex_descriptor_view_v<T>` | `true` if `T` is a `vertex_descriptor_view` |
| `is_edge_descriptor_view_v<T>` | `true` if `T` is an `edge_descriptor_view` |

---

## Determining `vertex_id` and Its Type

The type `vertex_id_t<G>` is resolved in priority order:

1. Type returned by an overridden `vertex_id(g, u)` for graph `G`
2. For `random_access_range<forward_range<integral>>` or
   `random_access_range<forward_range<tuple<integral, …>>>` patterns — the
   integral type
3. `size_t` in all other cases

The value of `vertex_id(g, u)` is resolved similarly — from override, or from
the descriptor's stored index.

---

## Partition Support

- **Unipartite:** `num_partitions(g) == 1` (default)
- **Bipartite:** `num_partitions(g) == 2`
- **Multipartite:** `num_partitions(g) >= 2`

Use `vertices(g, pid)` and `num_vertices(g, pid)` for partition-specific access.

---

## See Also

- [User Guide: Adjacency Lists](../user-guide/adjacency-lists.md) — tutorial-style guide
- [Edge List Interface](edge-list-interface.md) — GCI spec for edge lists
- [CPO Reference](cpo-reference.md) — full CPO documentation
- [Concepts Reference](concepts.md) — all concepts in one place
- [Containers](../user-guide/containers.md) — concrete graph containers
