# Graph Container Interface

This document summarizes the Graph Container Interface (GCI) from P1709/D3130. The GCI defines the primitive concepts, traits, types, and functions used to define and access adjacency lists and edgelists, independent of their internal design and organization.

**Reference:** This document follows the conventions defined in [common_graph_guidelines.md](common_graph_guidelines.md).

## Overview

The Graph Container Interface provides:
- **Flexibility**: Supports various graph representations (vector of lists, CSR, adjacency matrix, etc.)
- **Customization**: All functions are customization point objects (CPOs) that can be overridden
- **Generality**: Designed to support future graph data models beyond current requirements
- **Default Implementations**: Reasonable defaults minimize work needed to adapt existing data structures
- **Outgoing Edge Focus**: The interface focuses on outgoing edges from vertices. Incoming edges, if needed, are handled through separate views that present them using the same outgoing-edge interface

### Design Principles

1. **Descriptors**: Opaque objects representing vertices and edges that abstract implementation details
   - For random-access containers: descriptors are integral indices
   - For bidirectional containers: descriptors are iterators
   - Reduces number of functions and concepts needed

2. **Customization Points**: All interface functions can be overridden via CPO mechanism

3. **Range-of-Ranges**: Adjacency lists treated as ranges of vertices, where each vertex is a range of edges

4. **Partitions**: Support for unipartite (1), bipartite (2), and multipartite (n) graphs

## Adjacency List Interface

### Concepts

**Qualifiers used in concept names:**
- **`index`**: Vertex range is random-access and vertex ID is integral
- **`sourced`**: Edge has a source ID

#### Edge Concepts

| Concept | Description |
|---------|-------------|
| `basic_edge<G, E>` | Edge `E` with `target_id(g,uv)` available |
| `adjacency_edge<G, E>` | Edge `E` with `target(g,uv)` available |
| `sourced_edge<G, E>` | Edge `E` with `source_id(g,uv)` and `source(g,uv)` available |

#### Edge Range Concepts

| Concept | Description |
|---------|-------------|
| `targeted_edge_range<G, ER>` | Range `ER` of edges with `target_id(g,uv)` |

#### Vertex Concepts

| Concept | Description |
|---------|-------------|
| `vertex_range<G, VR>` | General vertex range concept |
| `index_vertex_range<G, VR>` | Vertex range with random-access and integral vertex IDs (high-performance graphs) |

#### Adjacency List Concepts

| Concept | Description |
|---------|-------------|
| `adjacency_list<G>` | General adjacency list (allows associative containers for vertices) |
| `index_adjacency_list<G>` | High-performance adjacency list (vertices in random-access containers, integral IDs) |

**Note:** All algorithms initially proposed use `index_adjacency_list<G>`. Future proposals may use the more general `adjacency_list<G>`.

### Type Traits

| Trait | Type | Description |
|-------|------|-------------|
| `has_degree<G>` | concept | Is `degree(g,u)` available? |
| `has_find_vertex<G>` | concept | Are `find_vertex(g,_)` functions available? |
| `has_find_vertex_edge<G>` | concept | Are `find_vertex_edge(g,_)` functions available? |
| `has_contains_edge<G>` | concept | Is `contains_edge(g,uid,vid)` available? |
| `unordered_edge<G>` | concept | Graph has unordered edges |
| `ordered_edge<G>` | concept | Graph has ordered edges |
| `define_adjacency_matrix<G>` | struct (false_type) | Specialize to derive from `true_type` for adjacency matrix |
| `is_adjacency_matrix<G>` | struct | Type trait for adjacency matrix |
| `is_adjacency_matrix_v<G>` | bool | Variable template for adjacency matrix |
| `adjacency_matrix<G>` | concept | Graph is an adjacency matrix |

### Type Aliases

Type aliases follow the `_t<G>` convention where `G` is the graph type.

#### Graph Types

| Type Alias | Definition | Comment |
|------------|------------|---------|
| `graph_reference_t<G>` | `add_lvalue_reference<G>` | |
| `graph_value_t<G>` | `decltype(graph_value(g))` | Optional |

#### Vertex Types

| Type Alias | Definition | Comment |
|------------|------------|---------|
| `vertex_range_t<G>` | `decltype(vertices(g))` | Range of vertices |
| `vertex_iterator_t<G>` | `iterator_t<vertex_range_t<G>>` | Iterator over vertices |
| `vertex_t<G>` | `range_value_t<vertex_range_t<G>>` | Vertex descriptor type |
| `vertex_id_t<G>` | `decltype(vertex_id(g,u))` | Vertex identifier type |
| `vertex_value_t<G>` | `decltype(vertex_value(g,u))` | Optional vertex value |

#### Edge Types

| Type Alias | Definition | Comment |
|------------|------------|---------|
| `vertex_edge_range_t<G>` | `decltype(edges(g,u))` | Range of outgoing edges from vertex |
| `vertex_edge_iterator_t<G>` | `iterator_t<vertex_edge_range_t<G>>` | Iterator over edges |
| `edge_t<G>` | `range_value_t<vertex_edge_range_t<G>>` | Edge descriptor type |
| `edge_value_t<G>` | `decltype(edge_value(g,uv))` | Optional edge value |

#### Partition Types

| Type Alias | Definition | Comment |
|------------|------------|---------|
| `partition_id_t<G>` | `decltype(partition_id(g,u))` | Optional partition identifier |
| `partition_vertex_range_t<G>` | `decltype(vertices(g,pid))` | Optional partition vertex range |

### Functions

**Note:** Complexity notation: `|V|` = number of vertices, `|E|` = number of edges

#### Graph Functions

| Function | Return Type | Complexity | Default Implementation |
|----------|-------------|------------|------------------------|
| `graph_value(g)` | `graph_value_t<G>` | constant | n/a, optional |
| `vertices(g)` | `vertex_range_t<G>` | constant | `vertex_descriptor_view(g)` if `g` follows inner value patterns, n/a otherwise |
| `num_vertices(g)` | integral | constant | `size(vertices(g))` |
| `num_edges(g)` | integral | \|E\| | `n=0; for(u: vertices(g)) n+=distance(edges(g,u)); return n;` |
| `has_edge(g)` | bool | \|V\| | `for(u: vertices(g)) if !empty(edges(g,u)) return true; return false;` |
| `num_partitions(g)` | integral | constant | 1 |
| `vertices(g,pid)` | `partition_vertex_range_t<G>` | constant | `vertices(g)` |
| `num_vertices(g,pid)` | integral | constant | `size(vertices(g))` |

**IMPORTANT - vertices(g) Return Type:** `vertices(g)` MUST always return a `vertex_descriptor_view`. The implementation should:
1. Use `g.vertices()` if available (must return `vertex_descriptor_view`)
2. Use ADL `vertices(g)` if available (must return `vertex_descriptor_view`)
3. Otherwise, if `g` follows inner value patterns, return `vertex_descriptor_view(g)` wrapping the container itself

**Note:** Complexity shown for `num_edges(g)` and `has_edge(g)` is for the default implementation. Specific implementations may have better characteristics.

#### Vertex Functions

| Function | Return Type | Complexity | Default Implementation |
|----------|-------------|------------|------------------------|
| `find_vertex(g,uid)` | `vertex_iterator_t<G>` | constant | `begin(vertices(g)) + uid` if random-access |
| `vertex_id(g,u)` | `vertex_id_t<G>` | constant | (see Determining vertex_id below) |
| `vertex_value(g,u)` | `vertex_value_t<G>` | constant | n/a, optional |
| `vertex_value(g,uid)` | `vertex_value_t<G>` | constant | `vertex_value(g,*find_vertex(g,uid))`, optional |
| `degree(g,u)` | integral | constant | `size(edges(g,u))` if sized_range |
| `degree(g,uid)` | integral | constant | `degree(g,*find_vertex(g,uid))` if sized_range |
| `edges(g,u)` | `vertex_edge_range_t<G>` | constant | `edge_descriptor_view(u.inner_value(), u)` if `u.inner_value()` follows edge value patterns, n/a otherwise |
| `edges(g,uid)` | `vertex_edge_range_t<G>` | constant | `edges(g,*find_vertex(g,uid))` |
| `partition_id(g,u)` | `partition_id_t<G>` | constant | Optional |
| `partition_id(g,uid)` | `partition_id_t<G>` | constant | Optional |

**IMPORTANT - edges(g, u) Return Type:** `edges(g, u)` MUST always return an `edge_descriptor_view`. The implementation should:
1. Use `g.edges(u)` if available (must return `edge_descriptor_view`)
2. Use ADL `edges(g, u)` if available (must return `edge_descriptor_view`)
3. Otherwise, if the vertex descriptor's inner value follows edge value patterns, return `edge_descriptor_view(u.inner_value(), u)` wrapping the edge range

**Note:** Functions with `uid` parameter are convenience functions that call `find_vertex(g,uid)` first.

#### Edge Functions

| Function | Return Type | Complexity | Default Implementation |
|----------|-------------|------------|------------------------|
| `target_id(g,uv)` | `vertex_id_t<G>` | constant | Automatic for patterns: `random_access_range<forward_range<integral>>` or `random_access_range<forward_range<tuple<integral,...>>>` |
| `target(g,uv)` | `vertex_t<G>` | constant | `*(begin(vertices(g)) + target_id(g,uv))` if random-access & integral |
| `edge_value(g,uv)` | `edge_value_t<G>` | constant | `uv` if forward_range, n/a otherwise, optional |
| `find_vertex_edge(g,u,vid)` | `vertex_edge_iterator_t<G>` | linear | `find(edges(g,u), [](uv) { return target_id(g,uv)==vid; })` |
| `find_vertex_edge(g,uid,vid)` | `vertex_edge_iterator_t<G>` | linear | `find_vertex_edge(g, *find_vertex(g,uid), vid)` |
| `contains_edge(g,uid,vid)` | bool | constant (matrix) / linear | Adjacency matrix: `uid < size(vertices(g)) && vid < size(vertices(g))`<br>Otherwise: `find_vertex_edge(g,uid,vid) != end(edges(g,uid))` |

**Sourced Edge Functions** (only when `source_id(g,uv)` is defined):

| Function | Return Type | Complexity | Default Implementation |
|----------|-------------|------------|------------------------|
| `source_id(g,uv)` | `vertex_id_t<G>` | constant | n/a, optional |
| `source(g,uv)` | `vertex_t<G>` | constant | `*(begin(vertices(g)) + source_id(g,uv))` if random-access & integral |

### Determining vertex_id and its Type

The type for `vertex_id_t<G>` is determined in the following order:

1. Use the type returned by `vertex_id(g,u)` when overridden for a graph
2. When the graph matches pattern `random_access_range<forward_range<integral>>` or `random_access_range<forward_range<tuple<integral,...>>>`, use the integral type
3. Use `size_t` in all other cases

The value of `vertex_id(g,u)` for a descriptor is determined by:

1. Use the value returned by `vertex_id(g,u)` when overridden for a graph
2. Use the index value on the descriptor

**Recommendation:** Override `vertex_id(g,u)` to use a smaller type (e.g., `int32_t` or `int16_t`) for space and performance advantages, ensuring it's large enough for the number of vertices and edges.

### Descriptor Views

Descriptors are opaque, abstract objects representing vertices and edges. They provide:

- **Equality comparison**: `==`, `!=`
- **Ordering**: `<`, `<=`, `>`, `>=` (if supported by underlying container)
- **Copy and assignment**: Standard semantics
- **Default construction**: Valid but may not represent valid vertex/edge
- **`inner_value()`**: Member function returning reference to underlying container value (for CPO customization)

**Implementation:**
- Random-access containers: descriptor is an integral index
- Bidirectional containers: descriptor is an iterator

**Views:**
- `descriptor_range_view<I>`: View over a range with descriptors
- `descriptor_subrange_view<I>`: View for edges from multiple vertices in single container (CSR, adjacency matrix)

### Partition Support

- **Unipartite**: `num_partitions(g) == 1` (default)
- **Bipartite**: `num_partitions(g) == 2`
- **Multipartite**: `num_partitions(g) >= 2`

**Vertex Value Types:**
Multiple partition types can be handled using:
- `std::variant`: Lambda returns appropriate variant based on partition
- Base class pointer: Call member function based on partition
- `void*`: Cast to concrete type based on partition

**Edge Filtering:**
- `edges(g,uid,pid)` / `edges(g,u,pid)`: Filter edges where target is in partition `pid`

## Edgelist Interface

An edgelist is a range of values with `source_id`, `target_id`, and optional `edge_value`. It's similar to edges in adjacency lists but is a distinct, separate range.

**Namespace:** `graph::container::edgelist` (to avoid conflicts with adjacency list)

### Concepts

| Concept | Description |
|---------|-------------|
| `basic_sourced_edge<EL, E>` | Edge with `source_id(e)` and `target_id(e)` |
| `basic_sourced_edgelist<EL>` | Range of basic sourced edges |
| `sourced_edge<EL, E>` | Edge with source/target descriptors and optional value |
| `sourced_edgelist<EL>` | Range of sourced edges |
| `index_sourced_edgelist<EL>` | Edgelist with integral vertex IDs |

### Type Traits

| Trait | Type | Description |
|-------|------|-------------|
| `is_directed<EL>` | struct (false_type) | Specialize to derive from `true_type` for directed graphs |
| `is_directed_v<EL>` | bool | Variable template for directed edgelist |

**Usage:** During graph construction, may add a second edge with reversed source_id and target_id when true.

### Type Aliases

| Type Alias | Definition | Comment |
|------------|------------|---------|
| `edge_range_t<EL>` | `EL` | |
| `edge_iterator_t<EL>` | `iterator_t<edge_range_t<EL>>` | |
| `edge_t<EL>` | `range_value_t<edge_range_t<EL>>` | |
| `edge_value_t<EL>` | `decltype(edge_value(e))` | Optional |
| `vertex_id_t<EL>` | `decltype(target_id(e))` | |

### Functions

| Function | Return Type | Complexity | Default Implementation |
|----------|-------------|------------|------------------------|
| `target_id(e)` | `vertex_id_t<EL>` | constant | Automatic for tuple and edge_info patterns (see below) |
| `source_id(e)` | `vertex_id_t<EL>` | constant | Automatic for tuple and edge_info patterns (see below) |
| `edge_value(e)` | `edge_value_t<EL>` | constant | Automatic for tuple and edge_info patterns (see below), optional |
| `contains_edge(el,uid,vid)` | bool | linear | `find_if(el, [](e) { return source_id(e)==uid && target_id(e)==vid; })` |
| `num_edges(el)` | integral | constant | `size(el)` |
| `has_edge(el)` | bool | constant | `num_edges(el) > 0` |

### Pattern Matching for Edge Types

The interface automatically recognizes these patterns:

#### Tuple Patterns

| Pattern | Provides |
|---------|----------|
| `tuple<integral, integral>` | `source_id(e)`, `target_id(e)` |
| `tuple<integral, integral, scalar>` | `source_id(e)`, `target_id(e)`, `edge_value(e)` |

#### edge_info Patterns

| Pattern | Provides |
|---------|----------|
| `edge_info<VId, true, void, void>` | `source_id(e)`, `target_id(e)` |
| `edge_info<VId, true, void, EV>` | `source_id(e)`, `target_id(e)`, `edge_value(e)` |

**Note:** For other edge types, override the functions.

## Exception Handling

**`graph_error`**: Exception class inherited from `runtime_error`. May be used by any function but primarily anticipated for `load` functions.

## Using Existing Data Structures

The GCI provides reasonable defaults to minimize adaptation work:

- **Default implementations**: Leverage standard library types and containers
- **Override capability**: All functions are CPOs that can be customized
- **Pattern recognition**: Automatic support for common patterns (tuples, edge_info)
- **Minimal boilerplate**: Most adaptations require overriding only a few key functions

**Recommendation:** See the Graph Library Containers paper for detailed examples of adapting external data structures.

## Adjacency List Test Types

The following table lists adjacency list types based on STL containers useful for comprehensive testing. These cover the spectrum of vertex and edge storage patterns, descriptor behaviors, and edge value representations.

### Adjacency List Type Combinations

| # | Adjacency List Type | Edge Value Type | Vertex Descriptor | Edge Descriptor | Description |
|---|---------------------|-----------------|-------------------|-----------------|-------------|
| 1 | `vector<vector<int>>` | `int` (target_id) | `size_t` index | `size_t` index | Basic unweighted adjacency list, random-access vertex and edge storage |
| 2 | `vector<vector<pair<int,W>>>` | `pair<int,W>` | `size_t` index | `size_t` index | Weighted edges with pair pattern, target_id + weight |
| 3 | `vector<vector<tuple<int,W>>>` | `tuple<int,W>` | `size_t` index | `size_t` index | Weighted edges with tuple pattern, target_id + weight |
| 4 | `vector<vector<tuple<int,W,X>>>` | `tuple<int,W,X>` | `size_t` index | `size_t` index | Multi-property edges, target_id + multiple attributes |
| 5 | `vector<vector<Edge>>` | `struct Edge` | `size_t` index | `size_t` index | Custom edge struct with target_id and properties |
| 6 | `vector<list<int>>` | `int` (target_id) | `size_t` index | `list<int>::iterator` | Random-access vertices, linked-list edges |
| 7 | `vector<list<pair<int,W>>>` | `pair<int,W>` | `size_t` index | `list<>::iterator` | Weighted edges in linked list |
| 8 | `vector<set<int>>` | `int` (target_id) | `size_t` index | `set<int>::iterator` | Ordered unique edges (no multi-edges) |
| 9 | `vector<map<int,W>>` | `pair<const int,W>` | `size_t` index | `map<>::iterator` | Edges as key-value map, target_id → weight |
| 10 | `vector<unordered_set<int>>` | `int` (target_id) | `size_t` index | `unordered_set<>::iterator` | Unordered unique edges, O(1) lookup |
| 11 | `vector<unordered_map<int,W>>` | `pair<const int,W>` | `size_t` index | `unordered_map<>::iterator` | Unordered weighted edges, O(1) lookup |
| 12 | `deque<vector<int>>` | `int` (target_id) | `size_t` index | `size_t` index | Random-access vertices with deque (cheaper insertion) |
| 13 | `deque<deque<int>>` | `int` (target_id) | `size_t` index | `size_t` index | Both vertices and edges in deques |
| 14 | `map<VId,vector<int>>` | `int` (target_id) | `map<>::iterator` | `size_t` index | Sparse vertices (key-based), dense edges per vertex |
| 15 | `map<VId,list<int>>` | `int` (target_id) | `map<>::iterator` | `list<>::iterator` | Sparse vertices, linked-list edges |
| 16 | `map<VId,set<int>>` | `int` (target_id) | `map<>::iterator` | `set<>::iterator` | Sparse vertices, ordered unique edges |
| 17 | `map<VId,map<int,W>>` | `pair<const int,W>` | `map<>::iterator` | `map<>::iterator` | Fully associative, sorted vertices and edges |
| 18 | `unordered_map<VId,vector<int>>` | `int` (target_id) | `unordered_map<>::iterator` | `size_t` index | Hash-based sparse vertices, dense edges |
| 19 | `unordered_map<VId,list<int>>` | `int` (target_id) | `unordered_map<>::iterator` | `list<>::iterator` | Hash-based vertices, linked-list edges |
| 20 | `unordered_map<VId,unordered_set<int>>` | `int` (target_id) | `unordered_map<>::iterator` | `unordered_set<>::iterator` | Fully hash-based, unordered vertices and edges |
| 21 | `unordered_map<VId,unordered_map<int,W>>` | `pair<const int,W>` | `unordered_map<>::iterator` | `unordered_map<>::iterator` | Fully hash-based with weighted edges |
| 22 | `list<vector<int>>` | `int` (target_id) | `list<>::iterator` | `size_t` index | Linked-list vertices, random-access edges |
| 23 | `list<list<int>>` | `int` (target_id) | `list<>::iterator` | `list<>::iterator` | Fully linked-list representation |

### Descriptor Behavior Categories

**Random Access Descriptors (index-based):**
- **Vertex**: Types 1-13, 22-23 with non-associative vertex containers → `vertex_descriptor<Iter>` stores `size_t` index
- **Edge**: Types 1-5, 12-14, 18, 22 → `edge_descriptor<EdgeIter, VertexIter>` stores `size_t` index for edge position

**Bidirectional/Forward Descriptors (iterator-based):**
- **Vertex**: Types 14-21 with associative vertex containers → `vertex_descriptor<Iter>` stores iterator, `vertex_id()` returns key from pair
- **Edge**: Types 6-11, 15-17, 19-21, 23 → `edge_descriptor<EdgeIter, VertexIter>` stores edge iterator

### Edge Value Pattern Recognition

**Simple Target ID:**
- Types 1, 6, 8, 10, 12-23: Edge value is integral type representing target vertex ID
- Pattern: `target_id(g, uv)` returns the edge value directly

**Pair Pattern:**
- Types 2, 7, 9, 11, 17, 21: Edge value is `std::pair<int, Weight>`
- Pattern: `target_id(g, uv)` returns `uv.first`, edge weight via `uv.second` or `edge_value(g, uv)`

**Tuple Pattern:**
- Types 3-4: Edge value is `std::tuple<int, Weight, ...>`
- Pattern: `target_id(g, uv)` returns `std::get<0>(uv)`, properties via `std::get<1>(uv)`, etc.

**Custom Struct:**
- Type 5: Edge value is user-defined struct with target_id member
- Pattern: Requires CPO customization or member access convention

### Testing Strategy

1. **Core Functionality**: Test types 1-5 cover fundamental patterns (unweighted, weighted, tuple, custom)
2. **Iterator Categories**: Types 6-11 test forward/bidirectional edge iterators
3. **Sparse Graphs**: Types 14-21 test associative vertex containers (map, unordered_map)
4. **Performance**: Types 12-13 (deque), 10-11 (hash-based) test performance characteristics
5. **Comprehensive Coverage**: All 23 types ensure compatibility across storage strategies

---

**Document Status:** For exposition only (concepts marked as such pending consensus)

**Related Documents:**
- [common_graph_guidelines.md](common_graph_guidelines.md) - Naming conventions and architectural requirements
- P1709/D3130 - Full specification with implementation details
