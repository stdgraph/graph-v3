# Graph Views Implementation Strategy

This document describes the strategy for implementing graph views as described in D3129.

**Status**: Phase 0 Complete (2026-01-31)
**Next Phase**: Phase 1 (Foundation)

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
    -> /* range of vertex_info<void, vertex_descriptor_t<G>, invoke_result_t<VVF, vertex_descriptor_t<G>>> */
```

**Parameters**:
- `g`: The graph (lvalue or rvalue reference)
- `vvf`: Optional vertex value function `VV vvf(vertex_descriptor_t<G>)`

**Returns**: Range yielding `vertex_info<void, V, VV>` where:
- `VId = void` (descriptor contains ID)
- `V = vertex_descriptor_t<G>`
- `VV = invoke_result_t<VVF, vertex_descriptor_t<G>>` (or `void` if no vvf)

**Implementation Strategy**:
```cpp
template<index_adjacency_list G, class VVF>
class vertexlist_view : public std::ranges::view_interface<vertexlist_view<G, VVF>> {
    G* g_;
    VVF vvf_;  // Stored if provided
    
    class iterator {
        G* g_;
        vertex_id_t<G> current_;
        VVF* vvf_;
        
        auto operator*() const -> vertex_info<void, vertex_descriptor_t<G>, ...> {
            auto vdesc = vertex_descriptor<...>(current_);
            if constexpr (std::is_void_v<VVF>) {
                return {vdesc};  // No value
            } else {
                return {vdesc, (*vvf_)(vdesc)};  // With value
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
    -> /* range of edge_info<void, true, edge_descriptor_t<G>, invoke_result_t<EVF, edge_descriptor_t<G>>> */
```

**Parameters**:
- `g`: The graph
- `uid`: Source vertex ID
- `evf`: Optional edge value function `EV evf(edge_descriptor_t<G>)`

**Returns**: Range yielding `edge_info<void, true, E, EV>` where:
- `VId = void` (descriptor contains source/target IDs)
- `Sourced = true` (edge descriptor contains source vertex descriptor)
- `E = edge_descriptor_t<G>`
- `EV = invoke_result_t<EVF, edge_descriptor_t<G>>` (or `void` if no evf)

**Implementation Notes**:
- Wraps `edges(g, u)` CPO
- Creates edge descriptors with source vertex context (uid parameter)
- Edge descriptor provides access to source_id, target_id, and edge data
- Value function receives edge descriptor for maximum flexibility

**File Location**: `include/graph/views/incidence.hpp`

---

### 3.3 neighbors View

**Purpose**: Iterate over adjacent vertices (neighbors) from a vertex.

**Signature**:
```cpp
template<adjacency_list G, class VVF = void>
auto neighbors(G&& g, vertex_id_t<G> uid, VVF&& vvf = {})
    -> /* range of neighbor_info<void, false, vertex_descriptor_t<G>, invoke_result_t<VVF, vertex_descriptor_t<G>>> */
```

**Parameters**:
- `g`: The graph
- `uid`: Source vertex ID  
- `vvf`: Optional vertex value function applied to target vertex descriptors `VV vvf(vertex_descriptor_t<G>)`

**Returns**: Range yielding `neighbor_info<void, false, V, VV>` where:
- `VId = void` (descriptor contains target ID)
- `Sourced = false` (focus on target vertex, not edge source)
- `V = vertex_descriptor_t<G>` (target vertex descriptor)
- `VV = invoke_result_t<VVF, vertex_descriptor_t<G>>` (or `void` if no vvf)

**Implementation Notes**:
- Iterates over `edges(g, u)` internally to navigate adjacency
- Yields neighbor_info containing target vertex descriptor
- Value function receives target vertex descriptor (not source)

**File Location**: `include/graph/views/neighbors.hpp`

---

### 3.4 edgelist View

**Purpose**: Iterate over all edges in the graph (flat edge list).

**Signature**:
```cpp
template<adjacency_list G, class EVF = void>
auto edgelist(G&& g, EVF&& evf = {})
    -> /* range of edge_info<void, true, edge_descriptor_t<G>, invoke_result_t<EVF, edge_descriptor_t<G>>> */
```

**Parameters**:
- `g`: The graph
- `evf`: Optional edge value function `EV evf(edge_descriptor_t<G>)`

**Returns**: Range yielding `edge_info<void, true, E, EV>` where:
- `VId = void` (descriptor contains IDs)
- `Sourced = true` (always sourced)
- `E = edge_descriptor_t<G>`
- `EV = invoke_result_t<EVF, edge_descriptor_t<G>>` (or `void` if no evf)

**Implementation Notes**:
- Flattens the two-level `for u in vertices(g): for e in edges(g, u)` iteration
- Uses `std::views::join` or custom join iterator
- Each edge includes source_id from the outer loop

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

**Allocator Requirements**:
- Search views accept an `Alloc` template parameter for custom allocators
- The allocator type must satisfy the standard allocator requirements
- For DFS/BFS: allocates storage for visited tracking (`std::vector<bool>`) and stack/queue entries
- Default: `std::allocator<bool>` for visited vector; stack/queue use standard allocators for their entry types
- Custom allocators useful for memory pools, PMR, or embedded systems

**File Location**: `include/graph/views/search_base.hpp`

---

### 4.2 DFS Views

**Purpose**: Depth-first traversal from a source vertex.

#### 4.2.1 vertices_dfs

```cpp
template<index_adjacency_list G, class VVF = void, class Alloc = std::allocator<...>>
auto vertices_dfs(G&& g, vertex_id_t<G> seed, VVF&& vvf = {}, Alloc alloc = {})
    -> /* dfs_view yielding vertex_info */
```

**Yields**: `vertex_info<void, V, VV>` in DFS order (VId=void; IDs accessible via descriptor)

#### 4.2.2 edges_dfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto edges_dfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* dfs_view yielding edge_info */
```

**Yields**: `edge_info<void, false, E, EV>` (non-sourced) in DFS order (VId=void; IDs accessible via descriptor)

#### 4.2.3 sourced_edges_dfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto sourced_edges_dfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* dfs_view yielding sourced edge_info */
```

**Yields**: `edge_info<void, true, E, EV>` (sourced) in DFS order (VId=void; IDs accessible via descriptor)

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
    -> /* bfs_view yielding vertex_info */
```

**Yields**: `vertex_info<void, V, VV>` in BFS order (VId=void; IDs accessible via descriptor)

#### 4.3.2 edges_bfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>  
auto edges_bfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* bfs_view yielding edge_info */
```

**Yields**: `edge_info<void, false, E, EV>` (non-sourced) in BFS order (VId=void; IDs accessible via descriptor)

#### 4.3.3 sourced_edges_bfs

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto sourced_edges_bfs(G&& g, vertex_id_t<G> seed, EVF&& evf = {}, Alloc alloc = {})
    -> /* bfs_view yielding sourced edge_info */
```

**Yields**: `edge_info<void, true, E, EV>` (sourced) in BFS order (VId=void; IDs accessible via descriptor)

**BFS Implementation**: Same as DFS but uses `std::queue` instead of `std::stack`.

**File Location**: `include/graph/views/bfs.hpp`

---

### 4.4 Topological Sort Views

**Purpose**: Linear ordering of vertices in a DAG.

#### 4.4.1 vertices_topological_sort

```cpp
template<index_adjacency_list G, class VVF = void, class Alloc = std::allocator<...>>
auto vertices_topological_sort(G&& g, VVF&& vvf = {}, Alloc alloc = {})
    -> /* topological_view yielding vertex_info */
```

**Yields**: `vertex_info<void, V, VV>` in topological order (VId=void; IDs accessible via descriptor)

#### 4.4.2 edges_topological_sort

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto edges_topological_sort(G&& g, EVF&& evf = {}, Alloc alloc = {})
    -> /* topological_view yielding edge_info */
```

**Yields**: `edge_info<void, false, E, EV>` (non-sourced) in topological order (VId=void; IDs accessible via descriptor)

#### 4.4.3 sourced_edges_topological_sort

```cpp
template<index_adjacency_list G, class EVF = void, class Alloc = std::allocator<...>>
auto sourced_edges_topological_sort(G&& g, EVF&& evf = {}, Alloc alloc = {})
    -> /* topological_view yielding sourced edge_info */
```

**Yields**: `edge_info<void, true, E, EV>` (sourced) in topological order (VId=void; IDs accessible via descriptor)

**Implementation**: Uses reverse DFS post-order (Kahn's algorithm alternative available).

**File Location**: `include/graph/views/topological_sort.hpp`

---

## 5. Implementation Phases

### Phase 0: Info Struct Refactoring ✅ COMPLETE (2026-01-31)

**Completion Date**: 2026-01-31  
**Duration**: 1 day  
**Commit**: 330c7d8 "[views] Phase 0: Info struct refactoring complete"

**Tasks Completed**:
1. ✅ Refactored vertex_info (4 specializations for VId × V × VV void combinations)
2. ✅ Refactored edge_info (8 specializations for Sourced × VId × E × EV void combinations)
3. ✅ Refactored neighbor_info (8 specializations for Sourced × VId × V × VV void combinations)
4. ✅ Implemented all 20 new VId=void specializations
5. ✅ Created comprehensive test suite (27 test cases, 392 assertions)

**Key Discoveries**:
- edge_info uses `source_id` and `target_id` (vertex IDs), not `edge_id`
- neighbor_info uses `target` member when VId present, `vertex` when VId=void
- Sourced parameter controls presence of `source_id` member
- sizeof tests account for struct padding

**Files Modified**:
- `include/graph/graph_info.hpp` - Added 20 new specializations
- `tests/views/test_info_structs.cpp` - Created comprehensive test suite

---

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

## 8. Info Struct Refactoring (Phase 0) ✅ COMPLETE

**Completion Date**: 2026-01-31  
**Commit**: 330c7d8 "[views] Phase 0: Info struct refactoring complete"  
**Test Results**: ✅ 27 test cases, 392 assertions, all passing

The existing info structs in `graph_info.hpp` were designed for compatibility with the 
reference-based graph-v2 model. They have been refactored to align with the descriptor-based 
architecture of graph-v3.

### 8.1 vertex_info Refactoring ✅ COMPLETE

**Original Design** (graph-v2 compatible):
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

**However**: The `{id, value}` combination (`vertex_info<VId, void, VV>`) remains useful for 
external data scenarios, such as:
- Providing vertex data to graph constructors before the graph exists
- Exporting vertex data from one graph for use in another
- Serialization/deserialization where descriptors aren't available
- External algorithms that operate on ID-value pairs without graph context

**New Design** (all members optional via void):
```cpp
template <class VId, class V, class VV>
struct vertex_info {
  using id_type     = VId;  // vertex_id_t<G> - optional (void to omit)
  using vertex_type = V;    // vertex_t<G> or vertex_descriptor<...> - optional (void to omit)
  using value_type  = VV;   // from vvf - optional (void to omit)

  id_type     id;      // Conditionally present when VId != void
  vertex_type vertex;  // Conditionally present when V != void
  value_type  value;   // Conditionally present when VV != void
};
// Specializations for void combinations follow...
```

**Key Changes**:
1. **All three template parameters can now be void** (previously only V and VV could be void)
2. **VId = void** suppresses the `id` member (useful when V is a descriptor that contains ID)
3. **V = void** suppresses the `vertex` member (useful for ID-only iteration)
4. **VV = void** suppresses the `value` member (useful when no value function provided)
5. This creates flexibility: with descriptors, `id` becomes redundant; without descriptors, `id` is essential

**Usage Comparison**:
```cpp
// Current design (all members present):
for (auto&& [id, v, val] : vertexlist(g, vvf)) {
    // id is vertex_id_t<G>
    // v is vertex_t<G> (reference or descriptor)
}

// With descriptor (VId=void, V=descriptor, VV present):
vertex_info<void, vertex_descriptor<...>, int>  // only vertex and value members
for (auto&& [v, val] : vertexlist(g, vvf)) {
    auto id = v.vertex_id();  // extract ID from descriptor
}

// ID-only (VId present, V=void, VV=void):
vertex_info<size_t, void, void>  // only id member
for (auto&& [id] : vertexlist(g)) {
    // Just the ID, no vertex reference or value
}

// Descriptor-only (VId=void, V=descriptor, VV=void):
vertex_info<void, vertex_descriptor<...>, void>  // only vertex member
for (auto&& [v] : vertexlist(g)) {
    auto id = v.vertex_id();
}
```

**Structured Binding Patterns**:
- `vertex_info<VId, V, VV>`: `auto&& [id, vertex, value]`
- `vertex_info<VId, V, void>`: `auto&& [id, vertex]`
- `vertex_info<VId, void, VV>`: `auto&& [id, value]`
- `vertex_info<VId, void, void>`: `auto&& [id]`
- `vertex_info<void, V, VV>`: `auto&& [vertex, value]`
- `vertex_info<void, V, void>`: `auto&& [vertex]`
- `vertex_info<void, void, VV>`: `auto&& [value]` (unlikely use case)

**Migration Helpers** (optional):
```cpp
// Convenience accessor if users frequently need just the ID
template <class V, class VV>
constexpr auto id(const vertex_info<V, VV>& vi) {
    return vi.vertex.vertex_id();
}
```

### 8.2 edge_info Refactoring ✅ COMPLETE

**Original Design** (graph-v2 compatible):
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

**However**: The `{source_id, target_id, value}` combination (`edge_info<VId, true, void, EV>`) 
remains useful for external data scenarios, such as:
- Providing edge data to graph constructors before the graph exists
- Exporting edge lists from one graph for use in another
- Serialization/deserialization where descriptors aren't available
- External algorithms that operate on edge triples (src, tgt, value) without graph context
- CSV/JSON edge list import/export

**New Design** (all members optional via void):
```cpp
template <class VId, bool Sourced, class E, class EV>
struct edge_info {
  using source_id_type = conditional_t<Sourced, VId, void>;  // optional based on Sourced
  using target_id_type = VId;    // optional (void to omit)
  using edge_type      = E;      // optional (void to omit)
  using value_type     = EV;     // optional (void to omit)

  source_id_type source_id;  // Conditionally present when Sourced==true && VId != void
  target_id_type target_id;  // Conditionally present when VId != void
  edge_type      edge;       // Conditionally present when E != void
  value_type     value;      // Conditionally present when EV != void
};
// Specializations for Sourced × void combinations follow...
```

**Key Changes**:
1. **VId can now be void** to suppress both `source_id` and `target_id` members
2. When VId=void and E=descriptor, IDs are accessible via `edge.source_id()` and `edge.target_id(g)`
3. **Sourced bool** determines if source_id is present (when VId != void)
4. **E = void** suppresses the `edge` member (for ID-only edge info)
5. **EV = void** suppresses the `value` member
6. Specialization count depends on combinations: Sourced × VId × E × EV

**Usage Comparison**:
```cpp
// Current design with IDs (sourced):
for (auto&& [src, tgt, e, val] : sourced_edges(g, u, evf)) {
    // src is source vertex ID
    // tgt is target vertex ID
    // e is edge_t<G>
}

// With descriptor (VId=void, Sourced=true, E=descriptor, EV present):
edge_info<void, true, edge_descriptor<...>, int>  // only edge and value members
for (auto&& [e, val] : incidence(g, u, evf)) {
    auto src = e.source_id();       // extract from descriptor
    auto tgt = e.target_id(g);      // extract from descriptor
}

// IDs-only (VId present, Sourced=true, E=void, EV=void):
edge_info<size_t, true, void, void>  // only source_id and target_id members
for (auto&& [src, tgt] : incidence(g, u)) {
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

### 8.3 neighbor_info Refactoring ✅ COMPLETE

**Original Design** (graph-v2 compatible):
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
Internally, the view has an edge descriptor for navigation (it knows source and target), 
and it creates a `neighbor_info` for the target edge. The user primarily cares about the 
target vertex, but the edge descriptor provides the necessary context for accessing it.

**However**: Similar to vertex_info and edge_info, the `{source_id, target_id, value}` 
combination (`neighbor_info<VId, true, void, VV>`) remains useful for external data 
scenarios where the graph isn't available.

**New Design** (all members optional via void):
```cpp
template <class VId, bool Sourced, class V, class VV>
struct neighbor_info {
  using source_id_type = conditional_t<Sourced, VId, void>;  // optional based on Sourced
  using target_id_type = VId;    // optional (void to omit)
  using vertex_type    = V;      // optional (void to omit) - typically vertex_descriptor<...>
  using value_type     = VV;     // optional (void to omit)

  source_id_type source_id;  // Conditionally present when Sourced==true && VId != void
  target_id_type target_id;  // Conditionally present when VId != void
  vertex_type    vertex;     // Conditionally present when V != void
  value_type     value;      // Conditionally present when VV != void
};
// Specializations for Sourced × void combinations follow...
```

**IMPLEMENTATION NOTE** (as of 2026-01-31):
The actual implementation uses different member names based on VId:
- When **VId is present**: member is named `target` (not `vertex`)
- When **VId=void**: member is named `vertex`

This naming distinction exists in the codebase and is reflected in Phase 0 tests.

**Key Changes**:
1. **VId can now be void** to suppress both `source_id` and `target_id` members
2. When VId=void and V=vertex_descriptor, IDs are accessible via `vertex.vertex_id()` or CPO `vertex_id(g, vertex)`
3. **Sourced bool** determines if source_id is present (when VId != void)
4. **V = void** suppresses the `vertex` member (for ID-only neighbor iteration)
5. **VV = void** suppresses the `value` member
6. **Primary use case**: `neighbor_info<void, false, vertex_descriptor<...>, VV>` yields `{vertex, value}` for target vertex

**Usage Comparison**:
```cpp
// Current design with IDs (sourced):
for (auto&& [src, tgt_id, tgt, val] : neighbors(g, u, vvf)) {
    // src is source vertex ID
    // tgt_id is target vertex ID
    // tgt is target vertex reference
}

// With vertex descriptor (VId=void, Sourced=false, V=vertex_descriptor, VV present):
neighbor_info<void, false, vertex_descriptor<...>, int>  // {vertex, value}
for (auto&& [v, val] : neighbors(g, u, vvf)) {
    auto tgt_id = v.vertex_id();           // extract ID from descriptor
    auto& tgt_data = vertex_value(g, v);   // access underlying vertex data
    // v is the target vertex descriptor
}

// Without value function (VId=void, Sourced=false, V=vertex_descriptor, VV=void):
neighbor_info<void, false, vertex_descriptor<...>, void>  // {vertex}
for (auto&& [v] : neighbors(g, u)) {
    auto tgt_id = v.vertex_id();
    // Just the vertex descriptor, no value projection
}

// IDs-only for external data (VId present, Sourced=false, V=void, VV=void):
neighbor_info<size_t, false, void, void>  // {target_id}
for (auto&& [tgt_id] : neighbors(g, u)) {
    // Just target ID, no descriptor or value
}

// With IDs and value for external data (VId present, Sourced=true, V=void, VV present):
neighbor_info<size_t, true, void, int>  // {source_id, target_id, value}
for (auto&& [src, tgt, val] : neighbors(g, u, vvf)) {
    // Useful for exporting edges with values
}
```

**Design Rationale**: 
- **Primary pattern**: VId=void, Sourced=false, V=vertex_descriptor yields `{vertex, value}`
- Vertex descriptor provides access to vertex ID and underlying data
- VId present with V=void supports external data scenarios (export, serialization)
- Sourced=true adds source_id when VId != void (for edge-like neighbor info)
- Consistent with vertex_info and edge_info optional-members design

### 8.4 Implementation Summary ✅ COMPLETE

**Phase 0 was completed in three steps:**

**Step 0.1: vertex_info Refactoring** ✅
1. ✅ Made VId template parameter optional (void to suppress id member)
2. ✅ Ensured all three members (id, vertex, value) can be conditionally present
3. ✅ Updated specializations to handle all void combinations (4 specializations)
4. ✅ Kept `copyable_vertex_t<VId, VV>` = `vertex_info<VId, void, VV>` alias
5. ✅ Created comprehensive tests

**Step 0.2: edge_info Refactoring** ✅
1. ✅ Made VId template parameter optional (void to suppress source_id/target_id members)
2. ✅ Ensured all four members (source_id, target_id, edge, value) can be conditionally present
3. ✅ Updated specializations to handle Sourced × void combinations (8 specializations)
4. ✅ Kept `copyable_edge_t<VId, EV>` = `edge_info<VId, true, void, EV>` alias
5. ✅ Kept `is_sourced_v` trait
6. ✅ Created comprehensive tests

**Step 0.3: neighbor_info Refactoring** ✅
1. ✅ Made VId template parameter optional (void to suppress source_id/target_id members)
2. ✅ Ensured all four members (source_id, target_id, vertex, value) can be conditionally present
3. ✅ Updated specializations to handle Sourced × void combinations (8 specializations)
4. ✅ Implemented naming convention: `target` when VId present, `vertex` when VId=void
5. ✅ Kept `copyable_neighbor_t` alias
6. ✅ Kept `is_sourced_v` trait for neighbor_info
7. ✅ Created comprehensive tests

### 8.5 Summary of Info Struct Changes

**Key Insight**: All members of info structs are now **optional via void template parameters**.
This provides maximum flexibility for different use cases:
- **With descriptors**: VId can be void (descriptor contains ID)
- **Without descriptors**: VId is required, V/E can be void for lightweight iteration
- **Copyable types**: Use `copyable_vertex_t<VId, VV>` = `vertex_info<VId, void, VV>`

| Struct | Template Parameters | Members (when not void) | Flexibility |
|--------|---------------------|-------------------------|-------------|
| `vertex_info` | `<VId, V, VV>` | `id`, `vertex`, `value` | All 3 can be void |
| `edge_info` | `<VId, Sourced, E, EV>` | `source_id`, `target_id`, `edge`, `value` | VId, E, EV can be void |
| `neighbor_info` | `<VId, Sourced, V, VV>` | `source_id`, `target_id`, `target`/`vertex`*, `value` | VId, V, VV can be void |

*`neighbor_info` uses `target` when VId present, `vertex` when VId=void (implementation detail)

**Primary Usage Patterns**:
- **vertex_info**: `vertex_info<void, vertex_descriptor<...>, VV>` → `{vertex, value}`
- **edge_info**: `edge_info<void, true, edge_descriptor<...>, EV>` → `{edge, value}` (Sourced always true for edge descriptors)
- **neighbor_info**: `neighbor_info<void, false, vertex_descriptor<...>, VV>` → `{vertex, value}` (Sourced=false; target vertex only)

**Specialization Impact**:
- `vertex_info`: 8 specializations (2³ void combinations: VId, V, VV)
- `edge_info`: 16 specializations (2 Sourced × 2³ void combinations: VId, E, EV)
- `neighbor_info`: 16 specializations (2 Sourced × 2³ void combinations: VId, V, VV)
- **Total**: 40 specializations (implementation can reduce with SFINAE or conditional members)

**Implementation Status** (as of 2026-01-31):
- ✅ **Phase 0 Complete**: All 40 specializations implemented in `graph_info.hpp`
- ✅ **Tests**: 27 test cases, 392 assertions, all passing
- ✅ **Commit**: 330c7d8 "[views] Phase 0: Info struct refactoring complete"

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

### 9.2 Edge Descriptor Source Context

**Note**: `edge_descriptor` contains a source vertex descriptor as a member variable, 
providing complete source vertex context. This design simplifies edge-yielding views:

1. **edgelist view**: When flattening `for u: for e in edges(g,u)`, the edge descriptor 
   created from the edge iterator naturally captures the source vertex.

2. **Search views returning edges**: DFS/BFS edge views create edge descriptors that 
   inherently contain the source vertex context.

**Implementation**: Views that yield edges create descriptors with source context:

```cpp
class edgelist_iterator {
    G* g_;
    vertex_descriptor<...> current_source_;  // Current source vertex descriptor
    edge_iterator_t<G> current_edge_;
    edge_iterator_t<G> edge_end_;
    
    auto operator*() const {
        // Construct edge descriptor with source vertex descriptor
        auto edge_desc = edge_descriptor{current_edge_, current_source_};
        return edge_info{edge_desc, ...};
    }
};
```

The source vertex descriptor member in `edge_descriptor` eliminates the need for separate 
source ID tracking—it's built into the descriptor itself.

### 9.3 Value Function Design

**Problem**: Should value functions receive descriptors or underlying values?

**In graph-v2 (reference-based)**:
```cpp
vvf(vertex_reference)  // Direct access to vertex data
```

**In graph-v3 (descriptor-based)**:
```cpp
// Option A: Pass descriptor
vvf(vertex_descriptor)  // User can access ID, underlying value, etc.

// Option B: Pass descriptor + graph
vvf(g, vertex_descriptor)  // View passes both (redundant - descriptor works with graph already)

// Option C: Pass underlying value (view extracts it)
vvf(underlying_vertex_value)  // View does: vvf(vertex_value(g, v))
```

**Decision**: **Option A** - Value functions receive descriptors.

Descriptors provide complete access while maintaining the value-based semantics:
```cpp
// Vertex value function receives vertex_descriptor
for (auto&& [v, val] : vertexlist(g, [&g](auto vdesc) { 
    return vertex_value(g, vdesc).name;  // Access underlying value via descriptor
})) {
    // val is the extracted name
}

// Edge value function receives edge_descriptor
for (auto&& [e, val] : incidence(g, u, [&g](auto edesc) {
    return edge_value(g, edesc).weight;  // Access underlying edge value
})) {
    // val is the extracted weight
}

// Neighbor value function receives target vertex_descriptor
for (auto&& [v, val] : neighbors(g, u, [&g](auto vdesc) {
    return vertex_value(g, vdesc).name;  // Access target vertex value
})) {
    // v is target vertex descriptor, val is extracted name
}
```

**Rationale**:
- Descriptors are lightweight values that can be copied efficiently
- Value functions can access IDs, underlying values, or both as needed
- Consistent with descriptor-based architecture throughout graph-v3
- Allows value functions to perform lookups or computations using the descriptor
- User captures graph reference once in the lambda, not passed separately

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
        requires std::invocable<VVF, vertex_descriptor_t<G>>
   auto vertices_bfs(G&& g, vertex_id_t<G> seed, VVF&& vvf);
   
   // Multi-seed overloads
   template<index_adjacency_list G, std::ranges::input_range Seeds>
       requires std::convertible_to<std::ranges::range_value_t<Seeds>, vertex_id_t<G>>
   auto vertices_bfs(G&& g, Seeds&& seeds);
   
   template<index_adjacency_list G, std::ranges::input_range Seeds, class VVF>
       requires std::convertible_to<std::ranges::range_value_t<Seeds>, vertex_id_t<G>>
              && std::invocable<VVF, vertex_descriptor_t<G>>
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
    - `vertices_dfs`: identical constraints with `VVF` on `vertex_descriptor_t<G>`
    - `edges_*` / `sourced_edges_*` variants: use `EVF` with `std::invocable<EVF, edge_descriptor_t<G>>` and seed(s) as above
    - **Note**: `vertices_topological_sort` does NOT take seed parameter(s); it processes all vertices in the DAG

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

    template<class VVF = void>
    struct vertices_topo_adaptor {
        VVF vvf;
        
        template<index_adjacency_list G>
        friend auto operator|(G&& g, vertices_topo_adaptor adaptor) {
            if constexpr (std::is_void_v<VVF>) {
                return vertices_topological_sort(std::forward<G>(g));
            } else {
                return vertices_topological_sort(std::forward<G>(g),
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
    inline constexpr auto vertices_topological_sort = []<class VVF = void>(VVF&& vvf = {}) {
        return vertices_topo_adaptor<std::decay_t<VVF>>{std::forward<VVF>(vvf)};
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
   for (auto&& [v, val] : g | vertices_dfs(seed, [&g](auto vdesc) { return vertex_value(g, vdesc).name; })) { ... }
   for (auto&& [v] : g | vertices_topological_sort()) { ... }
   
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
