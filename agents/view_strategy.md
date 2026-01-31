# Graph Views Implementation Strategy

This document describes the strategy for implementing graph views as described in D3129.

## 1. Overview

Views provide lazy, range-based access to graph elements during traversal. They enable algorithms 
to iterate over vertices, edges, and neighbors using standard range-based for loops with structured 
bindings. Views do not own data—they reference the underlying graph and synthesize info structs 
on iteration.

### 1.1 Design Principles

1. **Lazy Evaluation**: Views compute elements on-demand during iteration
2. **Zero-Copy**: Return references where possible; IDs are copied (cheap)
3. **Structured Bindings**: All views yield info structs supporting `auto&& [a, b, ...]`
4. **Value Functions**: Optional callable parameters to extract custom values
5. **Const-Correct**: Views from const graphs yield const references

### 1.2 View Categories

| Category | Views | Description |
|----------|-------|-------------|
| Basic | `vertexlist`, `incidence`, `neighbors`, `edgelist` | Direct graph element access |
| Search | `vertices_dfs`, `edges_dfs`, `sourced_edges_dfs` | Depth-first traversal |
| Search | `vertices_bfs`, `edges_bfs`, `sourced_edges_bfs` | Breadth-first traversal |
| Topological | `vertices_topological_sort`, `edges_topological_sort`, `sourced_edges_topological_sort` | DAG linearization |

---

## 2. Info Structs

Info structs are the value types yielded by view iterators. The existing implementation in 
`graph_info.hpp` provides a solid foundation. The current design uses template specializations 
with `void` to conditionally include/exclude members.

### 2.1 Current Implementation (graph_info.hpp)

The codebase already has:

```cpp
template <class VId, class V, class VV>
struct vertex_info { source_id_type id; vertex_type vertex; value_type value; };
// + specializations for void combinations

template <class VId, bool Sourced, class E, class EV>  
struct edge_info { source_id_type source_id; target_id_type target_id; edge_type edge; value_type value; };
// + 8 specializations for Sourced × E × EV combinations

template <class VId, bool Sourced, class V, class VV>
struct neighbor_info { source_id_type source_id; target_id_type target_id; vertex_type target; value_type value; };
// + 8 specializations for Sourced × V × VV combinations
```

**Note**: These structs will be refactored to use descriptor-based design (see Section 8). The signatures below reflect the **target** design after refactoring.

### 2.2 Required Enhancements

**Task 2.2.1**: Add search-specific info struct extensions for DFS/BFS/topological views:

```cpp
// For search views that need depth information
template <class VId, class V, class VV>
struct search_vertex_info : vertex_info<VId, V, VV> {
    std::size_t depth;  // Distance from search root
};

template <class VId, bool Sourced, class E, class EV>
struct search_edge_info : edge_info<VId, Sourced, E, EV> {
    std::size_t depth;  // Distance from search root
};

template <class VId, bool Sourced, class V, class VV>
struct search_neighbor_info : neighbor_info<VId, Sourced, V, VV> {
    std::size_t depth;  // Distance from search root
};
```

**Design Decision**: Consider whether depth should be:
- A member of the info struct (current proposal)
- Accessed via `depth(info)` CPO (more flexible, enables lazy computation)
- Stored only in the view state and accessed via `view.depth()` (D3129 approach)

**Recommendation**: Follow D3129—depth accessible via the view, not the info struct.

---

## 3. Basic Views

### 3.1 vertexlist View

**Purpose**: Iterate over all vertices in the graph.

**Signature**:
```cpp
template<index_adjacency_list G, class VVF = void>
auto vertexlist(G&& g, VVF&& vvf = {})
    -> /* range of vertex_info<vertex_descriptor<G>, invoke_result_t<VVF, const vertex_value_t<G>&>> */
```

**Parameters**:
- `g`: The graph (lvalue or rvalue reference)
- `vvf`: Optional vertex value function `VV vvf(const vertex_value_t<G>&)`

**Returns**: Range yielding `vertex_info<V, VV>` where:
- `V = vertex_descriptor<...>` (vertex descriptor, always present)
- `VV = invoke_result_t<VVF, const vertex_value_t<G>&>` (or `void` if no vvf)

**Implementation Strategy**:
```cpp
template<index_adjacency_list G>
class vertexlist_view : public std::ranges::view_interface<vertexlist_view<G>> {
    G* g_;
    // Optional VVF stored if provided
    
    class iterator {
        G* g_;
        vertex_id_t<G> current_;
        
        auto operator*() const -> vertex_info<...> {
            auto v_desc = vertex_descriptor(*g_, current_);  // vertex descriptor
            if constexpr (!std::is_void_v<VVF>) {
                auto&& val = vertex_value(*g_, v_desc);
                return {v_desc, vvf_(val)};  // {vertex, value}
            } else {
                return {v_desc};  // {vertex} only
            }
        }
    };
};
```

**File Location**: `include/graph/views/vertexlist.hpp`

---

### 3.2 incidence View

**Purpose**: Iterate over outgoing edges from a vertex.

**Signature**:
```cpp
template<adjacency_list G, class EVF = void>
auto incidence(G&& g, vertex_id_t<G> uid, EVF&& evf = {})
    -> /* range of edge_info<edge_descriptor<G>, invoke_result_t<EVF, const edge_value_t<G>&>> */
```

**Parameters**:
- `g`: The graph
- `uid`: Source vertex ID
- `evf`: Optional edge value function `EV evf(const edge_value_t<G>&)`

**Returns**: Range yielding `edge_info<E, EV>` where:
- `E = edge_descriptor<...>` (edge descriptor with source context, always present)
- `EV = invoke_result_t<EVF, const edge_value_t<G>&>` (or `void` if no evf)

**Implementation Notes**:
- Wraps `edges(g, u)` CPO
- Synthesizes edge_info on each dereference
- Edge descriptor already contains source context (via `uid` parameter or stored source vertex)

**File Location**: `include/graph/views/incidence.hpp`

---

### 3.3 neighbors View

**Purpose**: Iterate over adjacent vertices (neighbors) from a vertex.

**Signature**:
```cpp
template<adjacency_list G, class VVF = void>
auto neighbors(G&& g, vertex_id_t<G> uid, VVF&& vvf = {})
    -> /* range of neighbor_info<edge_descriptor<G>, invoke_result_t<VVF, const vertex_value_t<G>&>> */
```

**Parameters**:
- `g`: The graph
- `uid`: Source vertex ID  
- `vvf`: Optional vertex value function applied to target vertices (const vertex_value_t<G>&)

**Returns**: Range yielding `neighbor_info<E, VV>` where:
- `E = edge_descriptor<...>` (provides source_id, target_id, and access to target vertex)
- `VV = invoke_result_t<VVF, const vertex_value_t<G>&>` (or `void` if no vvf)

**Implementation Notes**:
- Iterates over `edges(g, u)` but yields neighbor (target vertex) info
- Edge descriptor provides both source and target context
- Access target vertex via `target(g, edge)` CPO

**File Location**: `include/graph/views/neighbors.hpp`

---

### 3.4 edgelist View

**Purpose**: Iterate over all edges in the graph (flat edge list).

**Signature**:
```cpp
template<adjacency_list G, class EVF = void>
auto edgelist(G&& g, EVF&& evf = {})
    -> /* range of edge_info<edge_descriptor<G>, invoke_result_t<EVF, const edge_value_t<G>&>> */
```

**Parameters**:
- `g`: The graph
- `evf`: Optional edge value function `EV evf(const edge_value_t<G>&)`

**Returns**: Range yielding `edge_info<E, EV>` where:
- `E = edge_descriptor<...>` (always includes source context)
- `EV = invoke_result_t<EVF, const edge_value_t<G>&>` (or `void` if no evf)

**Implementation Notes**:
- Flattens the two-level `for u in vertices(g): for e in edges(g, u)` iteration
- Uses `std::views::join` or custom join iterator
- Edge descriptors naturally carry source context from outer vertex loop

**File Location**: `include/graph/views/edgelist.hpp`

---

## 4. Search Views

Search views maintain internal state (visited set, stack/queue) and provide traversal control.

### 4.1 Common Search Infrastructure

**Search Control (from D3129)**:
```cpp
enum class cancel_search {
    continue_search,  // Continue normal traversal
    cancel_branch,    // Skip subtree, continue with siblings
    cancel_all        // Stop entire search
};

// Accessible on search view iterators/ranges:
cancel_search cancel(search_view_type& view);  // Get/set cancel state
std::size_t depth(const search_view_type& view);  // Current depth from root
std::size_t size(const search_view_type& view);   // Elements processed so far
```

**Visited Tracking**:
```cpp
template<class VId, class Alloc = std::allocator<bool>>
class visited_tracker {
    std::vector<bool, Alloc> visited_;
public:
    bool is_visited(VId id) const;
    void mark_visited(VId id);
    void reset();
};
```

**File Location**: `include/graph/views/search_base.hpp`

---

### 4.2 DFS Views

**Purpose**: Depth-first traversal from a source vertex.

#### 4.2.1 vertices_dfs

```cpp
template<index_adjacency_list G, class VVF = void, class Alloc = std::allocator<...>>
auto vertices_dfs(G&& g, vertex_id_t<G> seed, VVF&& vvf = {}, Alloc alloc = {})
    -> /* dfs_view yielding vertex_info<vertex_descriptor<G>, VV> */
```

**Yields**: `vertex_info<V, VV>` in DFS order where:
- `V = vertex_descriptor<...>`
- `VV = invoke_result_t<VVF, const vertex_value_t<G>&>` (or `void`)

#### 4.2.2 edges_dfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto edges_dfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* dfs_view yielding edge_info<edge_descriptor<G>, EV> */
```

**Yields**: `edge_info<E, EV>` in DFS order where:
- `E = edge_descriptor<...>` (with source context)
- `EV = invoke_result_t<EVF, const edge_value_t<G>&>` (or `void`)

#### 4.2.3 sourced_edges_dfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto sourced_edges_dfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* dfs_view yielding edge_info<edge_descriptor<G>, EV> */
```

**Yields**: `edge_info<E, EV>` in DFS order (same as edges_dfs; distinction is in traversal behavior)

**DFS Implementation Strategy**:
```cpp
template<index_adjacency_list G, SearchKind Kind, class VF, class Alloc>
class dfs_view : public std::ranges::view_interface<dfs_view<G, Kind, VF, Alloc>> {
    G* g_;
    VF value_fn_;
    
    struct state_t {
        using stack_entry = std::pair<vertex_id_t<G>, edge_iterator_t<G>>;
        std::stack<stack_entry, std::vector<stack_entry, Alloc>> stack_;
        std::vector<bool, Alloc> visited_;
        std::size_t depth_ = 0;
        std::size_t count_ = 0;
        cancel_search cancel_ = cancel_search::continue_search;
    };
    std::shared_ptr<state_t> state_;  // Shared for iterator copies
    
    class iterator {
        // Forward iterator over DFS traversal
    };
};
```

**File Location**: `include/graph/views/dfs.hpp`

---

### 4.3 BFS Views

**Purpose**: Breadth-first traversal from a source vertex.

#### 4.3.1 vertices_bfs

```cpp
template<index_adjacency_list G, class VVF = void, class Alloc = std::allocator<...>>
auto vertices_bfs(G&& g, vertex_id_t<G> seed, VVF&& vvf = {}, Alloc alloc = {})
    -> /* bfs_view yielding vertex_info<vertex_descriptor<G>, VV> */
```

**Yields**: `vertex_info<V, VV>`

#### 4.3.2 edges_bfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>  
auto edges_bfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* bfs_view yielding edge_info<edge_descriptor<G>, EV> */
```

**Yields**: `edge_info<E, EV>`

#### 4.3.3 sourced_edges_bfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto sourced_edges_bfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* bfs_view yielding edge_info<edge_descriptor<G>, EV> */
```

**Yields**: `edge_info<E, EV>` (same type as edges_bfs)

**BFS Implementation**: Same as DFS but uses `std::queue` instead of `std::stack`.

**File Location**: `include/graph/views/bfs.hpp`

---

### 4.4 Topological Sort Views

**Purpose**: Linear ordering of vertices in a DAG.

#### 4.4.1 vertices_topological_sort

```cpp
template<index_adjacency_list G, class VVF = void, class Alloc = std::allocator<...>>
auto vertices_topological_sort(G&& g, VVF&& vvf = {}, Alloc alloc = {})
    -> /* topological_view yielding vertex_info<vertex_descriptor<G>, VV> */
```

**Yields**: `vertex_info<V, VV>`

#### 4.4.2 edges_topological_sort

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto edges_topological_sort(G&& g, EVF&& evf = {}, Alloc alloc = {})
    -> /* topological_view yielding edge_info<edge_descriptor<G>, EV> */
```

**Yields**: `edge_info<E, EV>`

#### 4.4.3 sourced_edges_topological_sort

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto sourced_edges_topological_sort(G&& g, EVF&& evf = {}, Alloc alloc = {})
    -> /* topological_view yielding edge_info<edge_descriptor<G>, EV> */
```

**Yields**: `edge_info<E, EV>` (same type as edges_topological_sort)

**Implementation**: Uses reverse DFS post-order (Kahn's algorithm alternative available).

**File Location**: `include/graph/views/topological_sort.hpp`

---

## 5. Implementation Phases

### Phase 1: Foundation (2-3 days)

**Tasks**:
1. Create `include/graph/views/` directory structure
2. Create `include/graph/views/search_base.hpp`:
   - `cancel_search` enum
   - `visited_tracker` template
   - Common search state management
3. Verify/extend info structs in `graph_info.hpp` if needed
4. Create view concepts in `include/graph/views/view_concepts.hpp`

**Files**:
- `include/graph/views/search_base.hpp`
- `include/graph/views/view_concepts.hpp`

---

### Phase 2: Basic Views (3-4 days)

**Tasks**:
1. Implement `vertexlist` view and tests
2. Implement `incidence` view and tests
3. Implement `neighbors` view and tests
4. Implement `edgelist` view and tests
5. Create convenience header `include/graph/views/basic_views.hpp`

**Files**:
- `include/graph/views/vertexlist.hpp`
- `include/graph/views/incidence.hpp`
- `include/graph/views/neighbors.hpp`
- `include/graph/views/edgelist.hpp`
- `include/graph/views/basic_views.hpp`
- `tests/views/test_basic_views.cpp`

---

### Phase 3: DFS Views (2-3 days)

**Tasks**:
1. Implement DFS state management
2. Implement `vertices_dfs` view and tests
3. Implement `edges_dfs` view and tests
4. Implement `sourced_edges_dfs` view and tests
5. Test cancel functionality

**Files**:
- `include/graph/views/dfs.hpp`
- `tests/views/test_dfs_views.cpp`

---

### Phase 4: BFS Views (2-3 days)

**Tasks**:
1. Implement BFS state management (queue-based)
2. Implement `vertices_bfs` view and tests
3. Implement `edges_bfs` view and tests
4. Implement `sourced_edges_bfs` view and tests
5. Test depth() and size() accessors

**Files**:
- `include/graph/views/bfs.hpp`
- `tests/views/test_bfs_views.cpp`

---

### Phase 5: Topological Sort Views (2 days)

**Tasks**:
1. Implement topological sort algorithm
2. Implement all three view variants
3. Handle cycle detection (throw or return sentinel)
4. Test on various DAGs

**Files**:
- `include/graph/views/topological_sort.hpp`
- `tests/views/test_topological_views.cpp`

---

### Phase 6: Integration and Polish (1-2 days)

**Tasks**:
1. Create unified header `include/graph/views.hpp`
2. Update `graph.hpp` to include views
3. Write documentation
4. Performance benchmarks
5. Edge cases and error handling

**Files**:
- `include/graph/views.hpp`
- Update `include/graph/graph.hpp`
- `docs/views.md`
- `benchmark/benchmark_views.cpp`

---

## 6. Design Decisions

### 6.1 Value Functions (VVF/EVF)

**Question**: Should value functions be stored in the view or applied at each dereference?

**Decision**: Store in view, apply at dereference. This avoids redundant copies and allows 
stateful value functions.

### 6.2 Iterator Category

**Question**: What iterator category should views provide?

**Decision**: Forward iterators. Random access is not meaningful for search views, and 
bidirectional adds complexity without clear benefit.

### 6.3 State Sharing Between Iterators

**Question**: How should search views handle iterator copies?

**Decision**: Use `std::shared_ptr<state_t>` for the internal state. Multiple iterators 
from the same view share state. This is necessary because search views are not restartable.

### 6.4 Allocator Support

**Question**: Should views support custom allocators?

**Decision**: Yes. Search views need stack/queue/visited storage. Pass allocator as last 
parameter with default `std::allocator`.

### 6.5 Const Correctness

**Question**: How should const graphs behave?

**Decision**: Views from `const G&` yield `const` references in info structs. The `vertex` 
and `edge` members will be const references.

### 6.6 Freestanding Support

**Question**: Should views support graphs without random-access vertices?

**Decision**: Not for search views. DFS/BFS/topological require `index_adjacency_list` because 
they need efficient visited tracking via vertex IDs as indices. Basic views (`vertexlist`, 
`incidence`, `neighbors`) can work with non-indexed graphs.

---

## 7. Testing Strategy

### 7.1 Test Graphs

Create test utilities with standard graph types:
- Empty graph
- Single vertex
- Path graph (1-2-3-...-n)
- Cycle graph
- Complete graph K_n
- Binary tree
- DAG (for topological sort)
- Disconnected graph (multiple components)

### 7.2 Test Categories

1. **Basic Functionality**: Verify correct elements are yielded
2. **Ordering**: DFS/BFS produce correct visit order
3. **Structured Bindings**: All info struct variants work with `auto&& [a, b, ...]`
4. **Value Functions**: Custom VVF/EVF produce expected values
5. **Search Control**: cancel_branch and cancel_all work correctly
6. **Depth/Size Tracking**: Accurate depth and count during traversal
7. **Const Correctness**: Views from const graphs compile and work
8. **Edge Cases**: Empty graphs, single vertex, disconnected components

### 7.3 Test File Organization

```
tests/views/
├── test_basic_views.cpp
├── test_dfs_views.cpp
├── test_bfs_views.cpp
├── test_topological_views.cpp
└── test_view_integration.cpp
```

---

## 8. Info Struct Refactoring for Descriptors

The existing info structs in `graph_info.hpp` were designed for compatibility with the 
reference-based graph-v2 model. They need to be simplified to align with the descriptor-based 
architecture of graph-v3.

### 8.1 vertex_info Refactoring

**Current Design** (graph-v2 compatible):
```cpp
template <class VId, class V, class VV>
struct vertex_info {
  id_type     id;      // vertex_id_t<G> - always required
  vertex_type vertex;  // vertex_t<G> - optional (void to omit)
  value_type  value;   // from vvf - optional (void to omit)
};
```

**Problem**: Separate `id` and `vertex` members are redundant with descriptors. A vertex 
descriptor already contains the ID (accessible via `vertex_id()` or CPO `vertex_id(g, desc)`).

**New Design** (descriptor-based):
```cpp
template <class V, class VV>
struct vertex_info {
  using vertex_type = V;   // vertex_descriptor<...> - always required
  using value_type  = VV;  // from vvf - optional (void to omit)

  vertex_type vertex;  // The vertex descriptor (contains ID)
  value_type  value;   // User-extracted value
};

template <class V>
struct vertex_info<V, void> {
  using vertex_type = V;
  using value_type  = void;

  vertex_type vertex;
};
```

**Key Changes**:
1. Remove `VId` template parameter - ID type is derived from `V::storage_type`
2. Remove `id` member - access via `vertex.vertex_id()` or CPO
3. `vertex` member is always present and always a descriptor
4. Only `VV` (value) remains optional with void specialization

**Usage Comparison**:
```cpp
// Old (graph-v2 style):
for (auto&& [id, v, val] : vertexlist(g, vvf)) {
    // id is vertex_id_t<G>
    // v is vertex_t<G> (reference or descriptor)
}

// New (graph-v3 descriptor style):
for (auto&& [v, val] : vertexlist(g, vvf)) {
    auto id = v.vertex_id();  // or vertex_id(g, v)
    // v is vertex_descriptor<...>
}

// Without value function:
for (auto&& [v] : vertexlist(g)) {
    auto id = v.vertex_id();
}
```

**Structured Binding Impact**:
- Old: `auto&& [id, vertex, value]` or `auto&& [id, vertex]` or `auto&& [id, value]` or `auto&& [id]`
- New: `auto&& [vertex, value]` or `auto&& [vertex]`

**Migration Helpers** (optional):
```cpp
// Convenience accessor if users frequently need just the ID
template <class V, class VV>
constexpr auto id(const vertex_info<V, VV>& vi) {
    return vi.vertex.vertex_id();
}
```

### 8.2 edge_info Refactoring

**Current Design** (graph-v2 compatible):
```cpp
template <class VId, bool Sourced, class E, class EV>
struct edge_info {
  source_id_type source_id;  // VId when Sourced==true, void otherwise
  target_id_type target_id;  // VId - always required
  edge_type      edge;       // E - optional (void to omit)
  value_type     value;      // from evf - optional (void to omit)
};
// 8 specializations for Sourced × E × EV combinations
```

**Problem**: Separate `source_id`, `target_id`, and `edge` members are redundant with 
edge descriptors. An edge descriptor already provides:
- `source_id()` - the source vertex ID
- `target_id(container)` - the target vertex ID  
- Access to the underlying edge data

**New Design** (descriptor-based):
```cpp
template <class E, class EV>
struct edge_info {
  using edge_type  = E;   // edge_descriptor<...> - always required
  using value_type = EV;  // from evf - optional (void to omit)

  edge_type  edge;   // The edge descriptor (contains source_id, target_id)
  value_type value;  // User-extracted value
};

template <class E>
struct edge_info<E, void> {
  using edge_type  = E;
  using value_type = void;

  edge_type edge;
};
```

**Key Changes**:
1. Remove `VId` template parameter - ID types derived from descriptor
2. Remove `Sourced` bool parameter - descriptor always has source context
3. Remove `source_id`, `target_id` members - access via `edge.source_id()`, `edge.target_id(g)`
4. `edge` member is always present and always a descriptor
5. Only `EV` (value) remains optional with void specialization
6. Reduces from 8 specializations to 2

**Usage Comparison**:
```cpp
// Old (graph-v2 style):
for (auto&& [src, tgt, e, val] : sourced_edges(g, u, evf)) {
    // src is source vertex ID
    // tgt is target vertex ID
    // e is edge_t<G>
}

// New (graph-v3 descriptor style):
for (auto&& [e, val] : incidence(g, u, evf)) {
    auto src = e.source_id();       // or source_id(g, e)
    auto tgt = e.target_id(g);      // or target_id(g, e)
    // e is edge_descriptor<...>
}

// Without value function:
for (auto&& [e] : incidence(g, u)) {
    auto src = e.source_id();
    auto tgt = e.target_id(g);
}
```

**Structured Binding Impact**:
- Old: `auto&& [src, tgt, edge, value]` (sourced) or `auto&& [tgt, edge, value]` (non-sourced) + void variants
- New: `auto&& [edge, value]` or `auto&& [edge]`

**Note on Sourced vs Non-Sourced**: The old `Sourced` bool is no longer needed because 
edge descriptors in graph-v3 always carry their source vertex context. The distinction 
between "sourced" and "non-sourced" edges is now a property of how the view is used, 
not the info struct type.

### 8.3 neighbor_info Refactoring

**Current Design** (graph-v2 compatible):
```cpp
template <class VId, bool Sourced, class V, class VV>
struct neighbor_info {
  source_id_type source_id;  // VId when Sourced==true, void otherwise
  target_id_type target_id;  // VId - always required
  vertex_type    target;     // V (target vertex) - optional (void to omit)
  value_type     value;      // from vvf - optional (void to omit)
};
// 8 specializations for Sourced × V × VV combinations
```

**Problem**: The `neighbors` view iterates over edges but yields target vertex info. 
We need an edge descriptor to navigate (it knows source and target), but the user 
primarily cares about the target vertex.

**New Design** (descriptor-based):
```cpp
template <class E, class VV>
struct neighbor_info {
  using edge_type  = E;   // edge_descriptor<...> - always required (for navigation)
  using value_type = VV;  // from vvf applied to target vertex - optional

  edge_type  edge;   // The edge descriptor (provides source_id, target_id, access to target vertex)
  value_type value;  // User-extracted value from target vertex
};

template <class E>
struct neighbor_info<E, void> {
  using edge_type  = E;
  using value_type = void;

  edge_type edge;
};
```

**Key Changes**:
1. Remove `VId` template parameter - derived from edge descriptor
2. Remove `Sourced` bool parameter - descriptor always has source context
3. Remove `source_id`, `target_id`, `target` members - all accessible via edge descriptor
4. `edge` member is always present (needed to access target vertex)
5. Only `VV` (value) remains optional with void specialization
6. Reduces from 8 specializations to 2

**Usage Comparison**:
```cpp
// Old (graph-v2 style):
for (auto&& [src, tgt_id, tgt, val] : neighbors(g, u, vvf)) {
    // src is source vertex ID
    // tgt_id is target vertex ID
    // tgt is target vertex reference
}

// New (graph-v3 descriptor style):
for (auto&& [e, val] : neighbors(g, u, vvf)) {
    auto src = e.source_id();           // source vertex ID
    auto tgt_id = e.target_id(g);       // target vertex ID
    auto& tgt = target(g, e);           // target vertex (via CPO)
    // val is vvf(target(g, e))
}

// Without value function:
for (auto&& [e] : neighbors(g, u)) {
    auto tgt_id = e.target_id(g);
    auto& tgt = target(g, e);
}
```

**Design Rationale**: Using edge descriptor (not vertex descriptor) for neighbor_info 
because:
1. We need source context for traversal algorithms
2. Edge descriptor provides both source_id and target_id
3. Target vertex is accessible via `target(g, edge)` CPO
4. Consistent with edge_info design

### 8.4 Implementation Tasks

**Phase 0.1: vertex_info Refactoring**
1. Create new `vertex_info<V, VV>` template with void specialization
2. Add `id()` convenience function
3. Update `copyable_vertex_t` alias
4. Update any existing code using old vertex_info
5. Update tests

**Phase 0.2: edge_info Refactoring**
1. Create new `edge_info<E, EV>` template with void specialization
2. Remove all 8 old specializations
3. Update `copyable_edge_t` and `edgelist_edge` aliases
4. Update `is_sourced_v` trait (may no longer be needed)
5. Update any existing code using old edge_info
6. Update tests

**Phase 0.3: neighbor_info Refactoring**
1. Create new `neighbor_info<E, VV>` template with void specialization
2. Remove all 8 old specializations
3. Update `copyable_neighbor_t` alias
4. Update `is_sourced_v` trait for neighbor_info
5. Update any existing code using old neighbor_info
6. Update tests

### 8.5 Summary of Info Struct Changes

| Struct | Old Template | New Template | Old Members | New Members |
|--------|--------------|--------------|-------------|-------------|
| `vertex_info` | `<VId, V, VV>` | `<V, VV>` | `id`, `vertex`, `value` | `vertex`, `value` |
| `edge_info` | `<VId, Sourced, E, EV>` | `<E, EV>` | `source_id`, `target_id`, `edge`, `value` | `edge`, `value` |
| `neighbor_info` | `<VId, Sourced, V, VV>` | `<E, VV>` | `source_id`, `target_id`, `target`, `value` | `edge`, `value` |

**Specialization Count**:
- `vertex_info`: 4 → 2
- `edge_info`: 8 → 2
- `neighbor_info`: 8 → 2
- **Total**: 20 → 6

---

## 9. Descriptor-Based Architecture Considerations

The graph-v3 library uses a **value-based descriptor model** rather than the reference-based 
model in graph-v2. This section covers additional implementation concerns beyond the info 
struct refactoring in Section 8.

### 9.1 What Are Descriptors?

In graph-v3:
- `vertex_descriptor<VertexIter>` wraps either an index (`size_t`) or an iterator
- `edge_descriptor<EdgeIter, VertexIter>` wraps edge storage + source vertex descriptor
- Descriptors are **lightweight value types** (copyable, comparable)
- Descriptors require the container/graph reference to access underlying data

```cpp
// Descriptor is a handle, not the data itself
vertex_descriptor<Iter> desc(idx);  // Just stores the index
auto& vertex_data = desc.underlying_value(container);  // Needs container to access data
```

### 9.2 Edge Descriptor Source Dependency

**Problem**: `edge_descriptor` requires a source vertex descriptor to be complete. This 
creates challenges for:

1. **edgelist view**: When flattening `for u: for e in edges(g,u)`, we must capture the 
   source vertex at each outer iteration.

2. **Search views returning edges**: DFS/BFS edge views must track the current source 
   vertex as they traverse.

**Solution**: Views that yield edges must maintain source vertex context:

```cpp
class edgelist_iterator {
    G* g_;
    vertex_id_t<G> current_source_;  // Track source as we iterate
    edge_iterator_t<G> current_edge_;
    edge_iterator_t<G> edge_end_;
    
    auto operator*() const {
        // Construct edge descriptor with tracked source
        return edge_info{edge_descriptor{current_edge_, current_source_}, ...};
    }
};
```

### 9.3 Value Function Design

**Problem**: Should value functions receive descriptors or underlying values?

**In graph-v2 (reference-based)**:
```cpp
vvf(vertex_reference)  // Direct access to vertex data
```

**In graph-v3 (descriptor-based)**:
```cpp
// Option A: Pass descriptor (requires graph in closure)
vvf(vertex_descriptor)  // User must capture graph: [&g](auto v) { return vertex_value(g, v); }

// Option B: Pass descriptor + graph
vvf(g, vertex_descriptor)  // View passes both

// Option C: Pass underlying value (view extracts it)
vvf(underlying_vertex_value)  // View does: vvf(vertex_value(g, v))
```

**Decision**: **Option C** - Value functions receive the underlying value, not descriptors.

This matches user expectations and D3129 design:
```cpp
for (auto&& [v, val] : vertexlist(g, [](auto& vertex) { return vertex.name; })) {
    // val is vertex.name, as expected
}
```

The view implementation handles the descriptor-to-value extraction internally.

### 9.4 Search State Storage

**Problem**: What should DFS/BFS stacks/queues store?

| Storage Type | Memory | Resumption | Visited Check |
|--------------|--------|------------|---------------|
| Vertex ID only | Minimal | Need to restart edge iteration | O(1) with ID-indexed vector |
| Vertex ID + Edge Iterator | Larger | Can resume mid-adjacency | O(1) |
| Full Descriptor | Larger | Can resume | O(1) but wasteful |

**Decision**: Store `(vertex_id, edge_iterator)` pairs for DFS (need to resume 
after recursion), store `vertex_id` only for BFS (process all neighbors at once).

```cpp
// DFS state - needs to resume edge iteration
struct dfs_frame {
    vertex_id_t<G> vertex;
    edge_iterator_t<G> edge_iter;
    edge_iterator_t<G> edge_end;
};

// BFS state - process all neighbors when dequeued  
using bfs_queue_entry = vertex_id_t<G>;
```

### 9.5 Iterator Stability and Invalidation

**Problem**: Descriptors using iterator storage (for non-random-access containers) can be 
invalidated if the graph is mutated.

**Rules**:
1. Views do not own data; graph must outlive view
2. Graph mutation during iteration is undefined behavior
3. Index-based descriptors (from vector storage) are stable unless reallocation occurs
4. Iterator-based descriptors are invalidated by any mutation

**Documentation Required**: Views must document that:
- The graph reference must remain valid for the view's lifetime
- Mutating the graph during iteration is undefined behavior
- Copying info structs is safe (descriptors are values)

### 9.6 Const Correctness with Descriptors

**Problem**: How do descriptors from `const G&` differ from `G&`?

In graph-v3, descriptors themselves don't carry const-ness—they're just handles. The 
const-ness comes from the container access:

```cpp
vertex_descriptor<Iter> desc(idx);
auto& v = desc.underlying_value(container);        // non-const reference
auto& v = desc.underlying_value(const_container);  // const reference
```

**For views**: When constructed from `const G&`:
- Descriptors are the same type (values)
- Value functions receive `const` references to underlying data
- Info struct value members will be const if extracted from const graph

### 9.7 Summary: Descriptor-Aware Design Decisions

| Aspect | Decision | Rationale |
|--------|----------|-----------|
| Info struct members | Single descriptor + optional value (see Section 8) | Simplified, descriptor-native design |
| Value function input | Underlying value, not descriptor | User-friendly, matches expectations |
| View internal storage | IDs for visited, (ID, iterator) pairs for stack | Minimal memory, efficient operations |
| Edge source tracking | View maintains current source context | Required for edge descriptor construction |
| Const handling | Descriptors same type; const applied at access | Matches C++ const semantics |
| Iterator stability | Document UB on mutation | Standard range semantics |

---

## 10. Open Questions

1. **~~Should `edgelist` use edge_list CPO or iterate adjacency list?~~** *(Resolved)*
   - **Decision**: Use concepts to detect graph type and dispatch appropriately:
     - If `edge_list<G>` is satisfied → use `edges(g)` CPO for direct edge iteration
     - Else if `adjacency_list<G>` is satisfied → flatten via `for u: for e in edges(g,u)`
   - Implementation uses `if constexpr` with concept checks:
   ```cpp
   template<typename G, class EVF = void>
   auto edgelist(G&& g, EVF&& evf = {}) {
       if constexpr (edge_list<std::remove_cvref_t<G>>) {
           return edgelist_from_edge_list(std::forward<G>(g), std::forward<EVF>(evf));
       } else {
           static_assert(adjacency_list<std::remove_cvref_t<G>>);
           return edgelist_from_adjacency_list(std::forward<G>(g), std::forward<EVF>(evf));
       }
   }
   ```

2. **Multi-source search views?**
   - D3129 shows single-source. Should we support `vertices_bfs(g, {seeds...})`?
   - **Design**: Use concepts to disambiguate single vs multi-seed overloads:
   
   ```cpp
   // Single seed overloads
   template<index_adjacency_list G>
   auto vertices_bfs(G&& g, vertex_id_t<G> seed);
   
   template<index_adjacency_list G, class VVF>
        requires std::invocable<VVF, const vertex_value_t<G>&>
   auto vertices_bfs(G&& g, vertex_id_t<G> seed, VVF&& vvf);
   
   // Multi-seed overloads
   template<index_adjacency_list G, std::ranges::input_range Seeds>
       requires std::convertible_to<std::ranges::range_value_t<Seeds>, vertex_id_t<G>>
   auto vertices_bfs(G&& g, Seeds&& seeds);
   
   template<index_adjacency_list G, std::ranges::input_range Seeds, class VVF>
       requires std::convertible_to<std::ranges::range_value_t<Seeds>, vertex_id_t<G>>
              && std::invocable<VVF, const vertex_value_t<G>&>
   auto vertices_bfs(G&& g, Seeds&& seeds, VVF&& vvf);
   ```
   
   **Disambiguation**:
   - Second arg is scalar `vertex_id_t<G>` → single seed
   - Second arg is range of `vertex_id_t<G>` → multi-seed
   - Third arg (vvf) is always optional, same type for both
   
   **Usage**:
   ```cpp
   vertices_bfs(g, 0)                           // single seed, no vvf
   vertices_bfs(g, 0, my_vvf)                   // single seed, with vvf
   vertices_bfs(g, std::vector{0, 3, 7})        // multi-seed, no vvf
   vertices_bfs(g, std::vector{0, 3, 7}, my_vvf) // multi-seed, with vvf
   vertices_bfs(g, std::array{0, 3, 7}, my_vvf)  // also works with array
   ```
   
   - Can defer implementation to later phase, but design accommodates it.

    **Apply same overload shape to other search views** (with appropriate value functions):
    - `vertices_dfs`, `vertices_topological_sort`: identical constraints with `VVF` on `const vertex_value_t<G>&`
    - `edges_*` / `sourced_edges_*` variants: use `EVF` with `std::invocable<EVF, const edge_value_t<G>&>` and seed(s) as above

3. **Parallel/concurrent views?** *(Deferred)*
   - Out of scope for initial implementation.
   - Design should not preclude future parallelization.

4. **Range adaptor syntax?** *(Supported)*
   - **Decision**: Support `g | view_name(args...)` pipe syntax for all views.
   - **Design**: Use range adaptor closure pattern from C++23 (backportable to C++20):
   
   ```cpp
   namespace graph::views {
   
   // Adaptor closure object for vertexlist
   template<class VVF = void>
   struct vertexlist_adaptor {
       VVF vvf;
       
       template<index_adjacency_list G>
       friend auto operator|(G&& g, vertexlist_adaptor adaptor) {
           if constexpr (std::is_void_v<VVF>) {
               return vertexlist(std::forward<G>(g));
           } else {
               return vertexlist(std::forward<G>(g), std::move(adaptor.vvf));
           }
       }
   };
   
   // Factory function
   inline constexpr auto vertexlist = []<class VVF = void>(VVF&& vvf = {}) {
       return vertexlist_adaptor<std::decay_t<VVF>>{std::forward<VVF>(vvf)};
   };
   
    // Adaptor closure for incidence
    template<class EVF = void>
    struct incidence_adaptor {
        vertex_id_t_placeholder uid; 
        EVF evf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, incidence_adaptor adaptor) {
            if constexpr (std::is_void_v<EVF>) {
                return incidence(std::forward<G>(g), adaptor.uid);
            } else {
                return incidence(std::forward<G>(g), adaptor.uid, std::move(adaptor.evf));
            }
        }
    };
    inline constexpr auto incidence = []<class EVF = void>(auto uid, EVF&& evf = {}) {
        return incidence_adaptor<std::decay_t<EVF>>{uid, std::forward<EVF>(evf)};
    };

    // Adaptor closure for neighbors
    template<class VVF = void>
    struct neighbors_adaptor {
        vertex_id_t_placeholder uid;
        VVF vvf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, neighbors_adaptor adaptor) {
            if constexpr (std::is_void_v<VVF>) {
                return neighbors(std::forward<G>(g), adaptor.uid);
            } else {
                return neighbors(std::forward<G>(g), adaptor.uid, std::move(adaptor.vvf));
            }
        }
    };
    inline constexpr auto neighbors = []<class VVF = void>(auto uid, VVF&& vvf = {}) {
        return neighbors_adaptor<std::decay_t<VVF>>{uid, std::forward<VVF>(vvf)};
    };

    // Adaptor closure for edgelist
    template<class EVF = void>
    struct edgelist_adaptor {
        EVF evf;
        template<typename G>
        friend auto operator|(G&& g, edgelist_adaptor adaptor) {
            if constexpr (std::is_void_v<EVF>) {
                return edgelist(std::forward<G>(g));
            } else {
                return edgelist(std::forward<G>(g), std::move(adaptor.evf));
            }
        }
    };
    inline constexpr auto edgelist = []<class EVF = void>(EVF&& evf = {}) {
        return edgelist_adaptor<std::decay_t<EVF>>{std::forward<EVF>(evf)};
    };

    // Search view adaptors (single-seed; multi-seed mirror these with a range Seed type)
    template<class Seed, class VVF = void>
    struct vertices_bfs_adaptor {
        Seed seed;
        VVF vvf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, vertices_bfs_adaptor adaptor) {
            if constexpr (std::is_void_v<VVF>) {
                return vertices_bfs(std::forward<G>(g), std::move(adaptor.seed));
            } else {
                return vertices_bfs(std::forward<G>(g), std::move(adaptor.seed),
                                    std::move(adaptor.vvf));
            }
        }
    };

    template<class Seed, class VVF = void>
    struct vertices_dfs_adaptor {
        Seed seed;
        VVF vvf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, vertices_dfs_adaptor adaptor) {
            if constexpr (std::is_void_v<VVF>) {
                return vertices_dfs(std::forward<G>(g), std::move(adaptor.seed));
            } else {
                return vertices_dfs(std::forward<G>(g), std::move(adaptor.seed),
                                    std::move(adaptor.vvf));
            }
        }
    };

    template<class Seed, class VVF = void>
    struct vertices_topo_adaptor {
        Seed seed;
        VVF vvf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, vertices_topo_adaptor adaptor) {
            if constexpr (std::is_void_v<VVF>) {
                return vertices_topological_sort(std::forward<G>(g), std::move(adaptor.seed));
            } else {
                return vertices_topological_sort(std::forward<G>(g), std::move(adaptor.seed),
                                                 std::move(adaptor.vvf));
            }
        }
    };

    // Edge-yielding search adaptors (EVF on edge values)
    template<class Seed, class EVF = void>
    struct edges_bfs_adaptor {
        Seed seed;
        EVF evf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, edges_bfs_adaptor adaptor) {
            if constexpr (std::is_void_v<EVF>) {
                return edges_bfs(std::forward<G>(g), std::move(adaptor.seed));
            } else {
                return edges_bfs(std::forward<G>(g), std::move(adaptor.seed),
                                 std::move(adaptor.evf));
            }
        }
    };

    template<class Seed, class EVF = void>
    struct sourced_edges_bfs_adaptor {
        Seed seed;
        EVF evf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, sourced_edges_bfs_adaptor adaptor) {
            if constexpr (std::is_void_v<EVF>) {
                return sourced_edges_bfs(std::forward<G>(g), std::move(adaptor.seed));
            } else {
                return sourced_edges_bfs(std::forward<G>(g), std::move(adaptor.seed),
                                         std::move(adaptor.evf));
            }
        }
    };

    // Factories for search views
    inline constexpr auto vertices_bfs = []<class Seed, class VVF = void>(Seed&& seed, VVF&& vvf = {}) {
        return vertices_bfs_adaptor<std::decay_t<Seed>, std::decay_t<VVF>>{std::forward<Seed>(seed), std::forward<VVF>(vvf)};
    };
    inline constexpr auto vertices_dfs = []<class Seed, class VVF = void>(Seed&& seed, VVF&& vvf = {}) {
        return vertices_dfs_adaptor<std::decay_t<Seed>, std::decay_t<VVF>>{std::forward<Seed>(seed), std::forward<VVF>(vvf)};
    };
    inline constexpr auto vertices_topological_sort = []<class Seed, class VVF = void>(Seed&& seed, VVF&& vvf = {}) {
        return vertices_topo_adaptor<std::decay_t<Seed>, std::decay_t<VVF>>{std::forward<Seed>(seed), std::forward<VVF>(vvf)};
    };
    inline constexpr auto edges_bfs = []<class Seed, class EVF = void>(Seed&& seed, EVF&& evf = {}) {
        return edges_bfs_adaptor<std::decay_t<Seed>, std::decay_t<EVF>>{std::forward<Seed>(seed), std::forward<EVF>(evf)};
    };
    inline constexpr auto sourced_edges_bfs = []<class Seed, class EVF = void>(Seed&& seed, EVF&& evf = {}) {
        return sourced_edges_bfs_adaptor<std::decay_t<Seed>, std::decay_t<EVF>>{std::forward<Seed>(seed), std::forward<EVF>(evf)};
    };

    // DFS edge variants mirror BFS ones; topo edge variants mirror topo vertices
   
   } // namespace graph::views
   ```
   
    - Multi-seed adaptor closures mirror the single-seed versions with `Seeds` range parameters.
    - `vertex_id_t_placeholder` in the code sketches above represents the vertex ID type (actual implementations use template deduction).
    - `VVF`/`EVF` used in adaptors follow the same const-qualified constraints as the overloads above.

   **Usage**:
   ```cpp
   using namespace graph::views;
   
   // Basic views
   for (auto&& [v] : g | vertexlist()) { ... }
   for (auto&& [v, val] : g | vertexlist(my_vvf)) { ... }
   for (auto&& [e] : g | incidence(uid)) { ... }
   for (auto&& [e] : g | edgelist()) { ... }
   
   // Search views
   for (auto&& [v] : g | vertices_bfs(seed)) { ... }
   for (auto&& [v, val] : g | vertices_dfs(seed, my_vvf)) { ... }
   
   // Chaining with standard views
   auto names = g | vertexlist(get_name) | std::views::take(10);
   ```
   
   **Implementation Notes**:
   - Adaptor closures are lightweight objects (store only captured args)
   - `operator|` is a hidden friend for ADL
   - No-arg adaptors (like `vertexlist()`) can be constexpr objects
   - Compatible with standard library range adaptors for chaining

---

## 11. Dependencies

### External
- C++20 concepts, ranges
- Standard library containers (vector, stack, queue)

### Internal
- `graph_info.hpp` - info structs
- `adjacency_list_concepts.hpp` - graph concepts
- Graph CPOs (vertices, edges, vertex, target_id, etc.)

---

## 12. Success Criteria

1. All basic views work with `index_adjacency_list` graphs
2. All search views correctly traverse and yield elements
3. Structured bindings work for all info struct variants
4. Value functions correctly transform yielded values
5. Search control (cancel) works as specified
6. All tests pass with both vector-based and deque-based graphs
7. Documentation covers all public API
8. No memory leaks (sanitizer clean)

---

## 13. File Structure Summary

```
include/graph/
├── views/
│   ├── basic_views.hpp      # Convenience header for basic views
│   ├── vertexlist.hpp       # vertexlist view
│   ├── incidence.hpp        # incidence view
│   ├── neighbors.hpp        # neighbors view
│   ├── edgelist.hpp         # edgelist view
│   ├── search_base.hpp      # Common search infrastructure
│   ├── view_concepts.hpp    # View-related concepts
│   ├── dfs.hpp              # DFS views (vertices_dfs, edges_dfs, sourced_edges_dfs)
│   ├── bfs.hpp              # BFS views (vertices_bfs, edges_bfs, sourced_edges_bfs)
│   └── topological_sort.hpp # Topological sort views
├── views.hpp                # Master include for all views
└── graph.hpp                # Updated to include views.hpp

tests/views/
├── test_basic_views.cpp
├── test_dfs_views.cpp
├── test_bfs_views.cpp
├── test_topological_views.cpp
└── test_view_integration.cpp
```
