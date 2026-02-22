# Graph CPO Implementation Order

This document specifies the correct implementation order for CPOs in `graph_cpo.hpp`, matching the reference implementation structure.

## CPO Implementation Order

The following order should be followed for implementing CPOs and their corresponding type aliases:

### 1. Graph and Vertex Range Access

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 1 | `vertices(g)` | `vertex_range_t<G>`<br>`vertex_iterator_t<G>`<br>`vertex_t<G>` | Get range of vertices in graph (MUST return `vertex_descriptor_view`) |
| 2 | `vertex_id(g, u)` | `vertex_id_t<G>` | Get unique ID for vertex |
| 3 | `find_vertex(g, uid)` | - | Find vertex by ID |

**IMPORTANT:** `vertices(g)` MUST always return a `vertex_descriptor_view`:
- If `g.vertices()` exists, it must return `vertex_descriptor_view`
- If ADL `vertices(g)` exists, it must return `vertex_descriptor_view`
- Otherwise, if `g` follows inner value patterns, return `vertex_descriptor_view(g)`

### 2. Edge Range Access

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 4 | `out_edges(g, u)` | `out_edge_range_t<G>`<br>`out_edge_iterator_t<G>`<br>`edge_descriptor_t<G>`<br>`out_edge_t<G>` | Get outgoing edges from vertex (MUST return `edge_descriptor_view`). Aliases: `edges(g, u)`, `vertex_edge_range_t`, `vertex_edge_iterator_t`, `edge_t`. |
| 5 | `num_edges(g)` | - | Count total edges in graph |

**IMPORTANT:** `out_edges(g, u)` MUST always return an `edge_descriptor_view`:
- If `g.edges(u)` exists, it must return `edge_descriptor_view`
- If ADL `out_edges(g, u)` exists, it must return `edge_descriptor_view`
- Otherwise, if the vertex descriptor's inner value follows edge value patterns, return `edge_descriptor_view(u.inner_value(), u)`

### 3. Edge Target/Source Access

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 6 | `target_id(g, uv)` | - | Get target vertex ID from edge |
| 7 | `source_id(g, uv)` | - | Get source vertex ID from edge (optional) |
| 8 | `target(g, uv)` | - | Get target vertex descriptor from edge |
| 9 | `source(g, uv)` | - | Get source vertex descriptor from edge (optional) |

### 4. Edge Query Functions

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 10 | `find_out_edge(g, u, vid)` | - | Find edge from u to vid. Alias: `find_vertex_edge(g, u, vid)`. |
| 11 | `contains_edge(g, uid, vid)` | - | Check if edge exists |

### 5. Partition Support

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 12 | `partition_id(g, u)` | `partition_id_t<G>` | Get partition ID for vertex (optional) |

### 6. Vertex Queries

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 13 | `num_vertices(g)` | - | Count vertices in graph |
| 14 | `out_degree(g, u)` | - | Get out-degree of vertex. Alias: `degree(g, u)`. |

### 7. Value Access

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 15 | `vertex_value(g, u)` | `vertex_value_t<G>` | Get user-defined vertex value (optional) |
| 16 | `edge_value(g, uv)` | `edge_value_t<G>` | Get user-defined edge value (optional) |
| 17 | `graph_value(g)` | `graph_value_t<G>` | Get user-defined graph value (optional) |

### 8. Graph Properties

| # | CPO Function | Type Aliases | Description |
|---|--------------|--------------|-------------|
| 18 | `num_partitions(g)` | - | Get number of partitions (optional) |
| 19 | `has_edge(g)` | - | Check if graph has any edges |

## Implementation Priority

### Phase 1: Core Foundation (Essential)
**Required for basic graph operations**
- ✅ `vertices(g)` + type aliases **COMPLETE** (18 CPO tests + 5 type alias tests = 23 total)
  - ✅ `vertex_range_t<G>` - Range type returned by vertices(g)
  - ✅ `vertex_iterator_t<G>` - Iterator over vertex range
  - ✅ `vertex_t<G>` - Vertex descriptor type
- ✅ `vertex_id(g, u)` + type alias **COMPLETE** (15 CPO tests + 2 type alias tests = 17 total)
  - ✅ `vertex_id_t<G>` - Vertex ID type
  - **Resolution order** (checks inner_value first, then descriptor):
    1. `u.inner_value(g).vertex_id(g)` - Inner value member (highest priority)
    2. `vertex_id(g, u.inner_value(g))` - ADL with inner_value
    3. `vertex_id(g, u)` - ADL with descriptor
    4. `u.vertex_id()` - Descriptor default (lowest priority)
  - Where `u` is a `vertex_descriptor<Iter>` and `u.inner_value(g)` extracts the actual vertex data from the graph
- `find_vertex(g, uid)`
- `out_edges(g, u)` + type aliases
- `target_id(g, uv)`

### Phase 2: Query Functions (High Priority)
**Required for graph algorithms**
- `num_vertices(g)`
- `num_edges(g)`
- `target(g, uv)`
- `out_degree(g, u)`

### Phase 3: Edge Queries (Medium Priority)
**Required for advanced graph operations**
- `find_out_edge(g, u, vid)`
- `contains_edge(g, uid, vid)`

### Phase 4: Optional Features (Low Priority)
**For specialized graph types**
- `source_id(g, uv)` - For sourced edges
- `source(g, uv)` - For sourced edges
- `partition_id(g, u)` - For multipartite graphs
- `num_partitions(g)` - For multipartite graphs
- `has_edge(g)` - Convenience function

### Phase 5: Value Access (Optional)
**For graphs with user-defined values**
- `vertex_value(g, u)` + type alias
- `edge_value(g, uv)` + type alias
- `graph_value(g)` + type alias

## Type Alias Grouping

Type aliases should be defined immediately after their corresponding CPO:

```cpp
// After vertices CPO
using vertex_range_t = decltype(vertices(declval<G&&>()));
using vertex_iterator_t = iterator_t<vertex_range_t<G>>;
using vertex_t = range_value_t<vertex_range_t<G>>;  // vertex_descriptor<Iter>

// After vertex_id CPO
using vertex_id_t = decltype(vertex_id(declval<G&&>(), declval<vertex_t<G>>()));

// After out_edges CPO (primary names)
using out_edge_range_t = decltype(out_edges(declval<G&&>(), declval<vertex_t<G>>()));
using out_edge_iterator_t = iterator_t<out_edge_range_t<G>>;
using edge_descriptor_t = range_value_t<out_edge_range_t<G>>;
using out_edge_t = range_value_t<out_edge_range_t<G>>;  // edge_descriptor<EdgeIter, VertexIter>

// Convenience aliases (old names)
using vertex_edge_range_t = out_edge_range_t<G>;
using vertex_edge_iterator_t = out_edge_iterator_t<G>;
using edge_t = out_edge_t<G>;

// After partition_id CPO (optional)
using partition_id_t = decltype(partition_id(declval<G>(), declval<vertex_t<G>>()));

// After vertex_value CPO (optional)
using vertex_value_t = decltype(vertex_value(declval<G&&>(), declval<vertex_t<G>>()));

// After edge_value CPO (optional)
using edge_value_t = decltype(edge_value(declval<G&&>(), declval<out_edge_t<G>>()));

// After graph_value CPO (optional)
using graph_value_t = decltype(graph_value(declval<G&&>()));
```

## Notes

- CPOs marked as "optional" may not be needed for all graph types
- Convenience aliases (`edges(g, u)` → `out_edges(g, u)`, `degree(g, u)` → `out_degree(g, u)`, `find_vertex_edge` → `find_out_edge`) can be used interchangeably with the primary names
- The order optimizes for dependency resolution (earlier CPOs are used by later ones)
