# Graph Views Implementation Plan

This document provides a detailed, step-by-step implementation plan for the Graph Views feature.
Each step is designed to be executed by an agent with clear acceptance criteria and tests.

**Branch**: `feature/views`  
**Strategy Document**: [view_strategy.md](view_strategy.md)

---

## Progress Tracking

| Step | Description | Status | Tests Pass |
|------|-------------|--------|------------|
| 0.1 | Refactor vertex_info | ⬜ Not Started | ⬜ |
| 0.2 | Refactor edge_info | ⬜ Not Started | ⬜ |
| 0.3 | Refactor neighbor_info | ⬜ Not Started | ⬜ |
| 1.1 | Create directory structure | ⬜ Not Started | N/A |
| 1.2 | Implement search_base.hpp | ⬜ Not Started | ⬜ |
| 1.3 | Implement view_concepts.hpp | ⬜ Not Started | ⬜ |
| 2.1 | Implement vertexlist view | ⬜ Not Started | ⬜ |
| 2.2 | Implement incidence view | ⬜ Not Started | ⬜ |
| 2.3 | Implement neighbors view | ⬜ Not Started | ⬜ |
| 2.4 | Implement edgelist view | ⬜ Not Started | ⬜ |
| 2.5 | Implement range adaptor closures | ⬜ Not Started | ⬜ |
| 3.1 | Implement DFS state management | ⬜ Not Started | ⬜ |
| 3.2 | Implement vertices_dfs | ⬜ Not Started | ⬜ |
| 3.3 | Implement edges_dfs | ⬜ Not Started | ⬜ |
| 3.4 | Implement search cancel | ⬜ Not Started | ⬜ |
| 4.1 | Implement BFS state management | ⬜ Not Started | ⬜ |
| 4.2 | Implement vertices_bfs | ⬜ Not Started | ⬜ |
| 4.3 | Implement edges_bfs | ⬜ Not Started | ⬜ |
| 4.4 | Implement depth/size tracking | ⬜ Not Started | ⬜ |
| 5.1 | Implement topological_sort | ⬜ Not Started | ⬜ |
| 5.2 | Implement cycle detection | ⬜ Not Started | ⬜ |
| 6.1 | Create views.hpp header | ⬜ Not Started | ⬜ |
| 6.2 | Update graph.hpp | ⬜ Not Started | ⬜ |
| 6.3 | Write documentation | ⬜ Not Started | N/A |
| 6.4 | Add benchmarks | ⬜ Not Started | N/A |

**Legend**: ⬜ Not Started | 🔄 In Progress | ✅ Complete | ❌ Blocked

---

## Phase 0: Info Struct Refactoring

### Step 0.1: Refactor vertex_info

**Goal**: Simplify `vertex_info` to use descriptor-based design.

**File**: `include/graph/graph_info.hpp`

**Tasks**:
1. Create new `vertex_info<V, VV>` template (2 params instead of 3)
2. Create `vertex_info<V, void>` specialization
3. Keep old templates temporarily with `[[deprecated]]` attribute
4. Update `copyable_vertex_t` alias to use new template
5. Verify compilation of existing tests

**New Design**:
```cpp
template <class V, class VV>
struct vertex_info {
  using vertex_type = V;   // vertex_descriptor<...>
  using value_type  = VV;

  vertex_type vertex;
  value_type  value;
};

template <class V>
struct vertex_info<V, void> {
  using vertex_type = V;
  using value_type  = void;

  vertex_type vertex;
};
```

**Tests**: `tests/views/test_info_structs.cpp`
- [ ] `vertex_info<V, VV>` can be constructed with descriptor and value
- [ ] `vertex_info<V, void>` can be constructed with descriptor only
- [ ] Structured binding `auto&& [v, val]` works for `vertex_info<V, VV>`
- [ ] Structured binding `auto&& [v]` works for `vertex_info<V, void>`
- [ ] `vertex.vertex_id()` returns correct ID from descriptor

**Acceptance Criteria**:
- New templates compile and work
- Existing code still compiles (deprecated warnings OK)
- All existing tests pass

---

### Step 0.2: Refactor edge_info

**Goal**: Simplify `edge_info` to use descriptor-based design (no Sourced bool).

**File**: `include/graph/graph_info.hpp`

**Tasks**:
1. Create new `edge_info<E, EV>` template (2 params instead of 4)
2. Create `edge_info<E, void>` specialization
3. Remove all 8 old specializations (or deprecate)
4. Update `copyable_edge_t` and `edgelist_edge` aliases
5. Update `is_sourced_v` trait if needed (may become obsolete)
6. Verify CPO detection in `graph_cpo.hpp` still works

**New Design**:
```cpp
template <class E, class EV>
struct edge_info {
  using edge_type  = E;   // edge_descriptor<...>
  using value_type = EV;

  edge_type  edge;
  value_type value;
};

template <class E>
struct edge_info<E, void> {
  using edge_type  = E;
  using value_type = void;

  edge_type edge;
};
```

**Tests**: `tests/views/test_info_structs.cpp`
- [ ] `edge_info<E, EV>` can be constructed with descriptor and value
- [ ] `edge_info<E, void>` can be constructed with descriptor only
- [ ] Structured binding `auto&& [e, val]` works for `edge_info<E, EV>`
- [ ] Structured binding `auto&& [e]` works for `edge_info<E, void>`
- [ ] `edge.source_id()` returns correct source ID
- [ ] `edge.target_id(g)` returns correct target ID

**Acceptance Criteria**:
- New templates compile and work
- CPO detection for `edge_info` still works in `graph_cpo.hpp`
- All existing tests pass

---

### Step 0.3: Refactor neighbor_info

**Goal**: Simplify `neighbor_info` to use edge descriptor for navigation.

**File**: `include/graph/graph_info.hpp`

**Tasks**:
1. Create new `neighbor_info<E, VV>` template (2 params instead of 4)
2. Create `neighbor_info<E, void>` specialization
3. Remove all 8 old specializations (or deprecate)
4. Update `copyable_neighbor_t` alias if it exists
5. Update `is_sourced_v` trait for neighbor_info

**New Design**:
```cpp
template <class E, class VV>
struct neighbor_info {
  using edge_type  = E;   // edge_descriptor<...> (for navigation)
  using value_type = VV;  // from vvf applied to target vertex

  edge_type  edge;
  value_type value;
};

template <class E>
struct neighbor_info<E, void> {
  using edge_type  = E;
  using value_type = void;

  edge_type edge;
};
```

**Tests**: `tests/views/test_info_structs.cpp`
- [ ] `neighbor_info<E, VV>` can be constructed with edge descriptor and value
- [ ] `neighbor_info<E, void>` can be constructed with edge descriptor only
- [ ] Structured binding `auto&& [e, val]` works
- [ ] `target(g, edge)` returns correct target vertex
- [ ] `edge.source_id()` returns source ID
- [ ] `edge.target_id(g)` returns target ID

**Acceptance Criteria**:
- New templates compile and work
- All existing tests pass

---

## Phase 1: Foundation

### Step 1.1: Create Directory Structure

**Goal**: Set up the views directory and CMake integration.

**Tasks**:
1. Create `include/graph/views/` directory
2. Create `tests/views/` directory
3. Update `tests/CMakeLists.txt` to include views test subdirectory
4. Create `tests/views/CMakeLists.txt`

**Files to Create**:
- `include/graph/views/.gitkeep` (placeholder)
- `tests/views/CMakeLists.txt`

**CMake for tests/views/CMakeLists.txt**:
```cmake
add_executable(graph3_views_tests
    test_main.cpp
)
target_link_libraries(graph3_views_tests PRIVATE graph3 Catch2::Catch2)
include(Catch)
catch_discover_tests(graph3_views_tests)
```

**Acceptance Criteria**:
- Directories exist
- CMake configures without errors
- Empty test target builds

---

### Step 1.2: Implement search_base.hpp

**Goal**: Create common infrastructure for search views.

**File**: `include/graph/views/search_base.hpp`

**Tasks**:
1. Define `cancel_search` enum
2. Implement `visited_tracker<VId, Alloc>` template class
3. Define common search state base class (if needed)

**Implementation**:
```cpp
#pragma once
#include <vector>
#include <cstddef>

namespace graph::views {

enum class cancel_search : uint8_t {
    continue_search,  // Continue normal traversal
    cancel_branch,    // Skip subtree, continue with siblings
    cancel_all        // Stop entire search
};

template<class VId, class Alloc = std::allocator<bool>>
class visited_tracker {
    std::vector<bool, Alloc> visited_;
public:
    explicit visited_tracker(std::size_t num_vertices, Alloc alloc = {})
        : visited_(num_vertices, false, alloc) {}
    
    bool is_visited(VId id) const { return visited_[static_cast<std::size_t>(id)]; }
    void mark_visited(VId id) { visited_[static_cast<std::size_t>(id)] = true; }
    void reset() { std::fill(visited_.begin(), visited_.end(), false); }
    std::size_t size() const { return visited_.size(); }
};

} // namespace graph::views
```

**Tests**: `tests/views/test_search_base.cpp`
- [ ] `cancel_search` enum values are distinct
- [ ] `visited_tracker` default constructs with size
- [ ] `is_visited()` returns false initially
- [ ] `mark_visited()` sets visited flag
- [ ] `is_visited()` returns true after marking
- [ ] `reset()` clears all flags
- [ ] Custom allocator works

**Acceptance Criteria**:
- Header compiles standalone
- All tests pass
- No memory leaks (valgrind/ASan clean)

---

### Step 1.3: Implement view_concepts.hpp

**Goal**: Define concepts for graph views.

**File**: `include/graph/views/view_concepts.hpp`

**Tasks**:
1. Define `vertex_view` concept
2. Define `edge_view` concept  
3. Define `neighbor_view` concept
4. Define `search_view` concept (extends base view concepts)

**Implementation**:
```cpp
#pragma once
#include <ranges>
#include <concepts>

namespace graph::views {

// A view that yields vertex_info<V, VV> elements
template<class R>
concept vertex_view = std::ranges::input_range<R> && requires(R r) {
    { *std::ranges::begin(r) } -> std::convertible_to<...>;  // yields vertex_info
};

// A view that yields edge_info<E, EV> elements
template<class R>
concept edge_view = std::ranges::input_range<R>;

// A view that yields neighbor_info<E, VV> elements
template<class R>
concept neighbor_view = std::ranges::input_range<R>;

// Search views additionally provide depth() and size()
template<class R>
concept search_view = std::ranges::input_range<R> && requires(R& r) {
    { r.depth() } -> std::convertible_to<std::size_t>;
    { r.size() } -> std::convertible_to<std::size_t>;
    { r.cancel() } -> std::same_as<cancel_search&>;
};

} // namespace graph::views
```

**Tests**: `tests/views/test_view_concepts.cpp`
- [ ] Concepts are well-formed (compile)
- [ ] Standard ranges satisfy input_range
- [ ] Concept checks can be used in requires clauses

**Acceptance Criteria**:
- Header compiles standalone
- Concepts are usable in template constraints

---

## Phase 2: Basic Views

### Step 2.1: Implement vertexlist View

**Goal**: Implement the `vertexlist` view for iterating over all vertices.

**File**: `include/graph/views/vertexlist.hpp`

**Tasks**:
1. Implement `vertexlist_view<G, VVF>` class
2. Implement `vertexlist_view::iterator`
3. Implement `vertexlist(G&&)` overload (no VVF)
4. Implement `vertexlist(G&&, VVF&&)` overload (with VVF)
5. Ensure ranges concepts are satisfied (`input_range`, `view`)

**Signature**:
```cpp
template<index_adjacency_list G, class VVF = void>
auto vertexlist(G&& g, VVF&& vvf = {})
    -> vertexlist_view<G, VVF>;
```

**Iterator Dereference**:
```cpp
auto operator*() const -> vertex_info<vertex_descriptor<...>, VV> {
    auto v_desc = vertex_descriptor(*g_, current_);
    if constexpr (!std::is_void_v<VVF>) {
        auto&& val = vertex_value(*g_, v_desc);
        return {v_desc, vvf_(val)};
    } else {
        return {v_desc};
    }
}
```

**Tests**: `tests/views/test_basic_views.cpp`
- [ ] Empty graph yields empty range
- [ ] Single vertex graph yields one element
- [ ] Multi-vertex graph yields all vertices in order
- [ ] Vertex descriptor ID matches expected
- [ ] VVF is applied correctly to each vertex
- [ ] Structured binding `auto&& [v]` works
- [ ] Structured binding `auto&& [v, val]` works with VVF
- [ ] Const graph yields const references
- [ ] Range-based for loop works
- [ ] `std::ranges::distance()` returns vertex count

**Acceptance Criteria**:
- View satisfies `std::ranges::input_range`
- View satisfies `std::ranges::view`
- All tests pass

---

### Step 2.2: Implement incidence View

**Goal**: Implement the `incidence` view for iterating over outgoing edges.

**File**: `include/graph/views/incidence.hpp`

**Tasks**:
1. Implement `incidence_view<G, EVF>` class
2. Implement iterator that wraps edge iteration
3. Implement `incidence(G&&, vertex_id_t<G>)` overload
4. Implement `incidence(G&&, vertex_id_t<G>, EVF&&)` overload

**Iterator Dereference**:
```cpp
auto operator*() const -> edge_info<edge_descriptor<...>, EV> {
    auto e_desc = edge_descriptor(*current_edge_, source_vertex_);
    if constexpr (!std::is_void_v<EVF>) {
        auto&& val = edge_value(*g_, e_desc);
        return {e_desc, evf_(val)};
    } else {
        return {e_desc};
    }
}
```

**Tests**: `tests/views/test_basic_views.cpp`
- [ ] Vertex with no edges yields empty range
- [ ] Vertex with edges yields all outgoing edges
- [ ] Edge descriptor source_id matches input vertex
- [ ] Edge descriptor target_id is correct
- [ ] EVF is applied correctly
- [ ] Structured binding works
- [ ] Invalid vertex ID behavior (bounds check or UB documented)

**Acceptance Criteria**:
- View satisfies `std::ranges::input_range`
- All tests pass

---

### Step 2.3: Implement neighbors View

**Goal**: Implement the `neighbors` view for iterating over adjacent vertices.

**File**: `include/graph/views/neighbors.hpp`

**Tasks**:
1. Implement `neighbors_view<G, VVF>` class
2. Implement iterator that yields neighbor info
3. Implement `neighbors(G&&, vertex_id_t<G>)` overload
4. Implement `neighbors(G&&, vertex_id_t<G>, VVF&&)` overload

**Iterator Dereference**:
```cpp
auto operator*() const -> neighbor_info<edge_descriptor<...>, VV> {
    auto e_desc = edge_descriptor(*current_edge_, source_vertex_);
    if constexpr (!std::is_void_v<VVF>) {
        auto&& tgt = target(*g_, e_desc);
        auto&& tgt_val = vertex_value(*g_, tgt);
        return {e_desc, vvf_(tgt_val)};
    } else {
        return {e_desc};
    }
}
```

**Tests**: `tests/views/test_basic_views.cpp`
- [ ] Vertex with no neighbors yields empty range
- [ ] Vertex with neighbors yields all adjacent vertices
- [ ] Target vertex accessible via `target(g, edge)`
- [ ] VVF applied to target vertex value
- [ ] Structured binding works

**Acceptance Criteria**:
- View satisfies `std::ranges::input_range`
- All tests pass

---

### Step 2.4: Implement edgelist View

**Goal**: Implement the `edgelist` view for iterating over all edges.

**File**: `include/graph/views/edgelist.hpp`

**Tasks**:
1. Implement `edgelist_view<G, EVF>` class
2. Implement flattening iterator (outer: vertices, inner: edges)
3. Implement `edgelist(G&&)` overload
4. Implement `edgelist(G&&, EVF&&)` overload
5. Handle dispatch based on `edge_list<G>` vs `adjacency_list<G>`

**Flattening Logic**:
```cpp
// For adjacency_list: flatten nested iteration
for (auto uid : vertex_ids(g)) {
    for (auto& e : edges(g, uid)) {
        // yield edge_info with source = uid
    }
}
```

**Tests**: `tests/views/test_basic_views.cpp`
- [ ] Empty graph yields empty range
- [ ] Graph with edges yields all edges
- [ ] Edge source_id is correct for each edge
- [ ] Edge target_id is correct for each edge
- [ ] EVF applied correctly
- [ ] Order: edges grouped by source vertex
- [ ] Structured binding works

**Acceptance Criteria**:
- View satisfies `std::ranges::input_range`
- All tests pass

---

### Step 2.5: Implement Range Adaptor Closures

**Goal**: Enable pipe syntax `g | view(args...)` for all basic views.

**File**: `include/graph/views/basic_views.hpp`

**Tasks**:
1. Implement `vertexlist_adaptor<VVF>` closure
2. Implement `incidence_adaptor<EVF>` closure  
3. Implement `neighbors_adaptor<VVF>` closure
4. Implement `edgelist_adaptor<EVF>` closure
5. Create factory CPOs for pipe syntax

**Implementation Pattern**:
```cpp
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

inline constexpr auto vertexlist = []<class VVF = void>(VVF&& vvf = {}) {
    return vertexlist_adaptor<std::decay_t<VVF>>{std::forward<VVF>(vvf)};
};
```

**Tests**: `tests/views/test_basic_views.cpp`
- [ ] `g | vertexlist()` works
- [ ] `g | vertexlist(vvf)` works
- [ ] `g | incidence(uid)` works
- [ ] `g | incidence(uid, evf)` works
- [ ] `g | neighbors(uid)` works
- [ ] `g | edgelist()` works
- [ ] Chaining with `std::views::take` works
- [ ] Chaining with `std::views::filter` works

**Acceptance Criteria**:
- All pipe syntax forms compile and work
- Chaining with standard views works
- All tests pass

---

## Phase 3: DFS Views

### Step 3.1: Implement DFS State Management

**Goal**: Create DFS-specific state and iterator infrastructure.

**File**: `include/graph/views/dfs.hpp`

**Tasks**:
1. Define `dfs_frame` struct for stack entries
2. Implement `dfs_state<G, Alloc>` class
3. Implement stack push/pop with edge iterator tracking
4. Implement visited checking

**DFS Frame**:
```cpp
template<class G>
struct dfs_frame {
    vertex_id_t<G> vertex;
    edge_iterator_t<G> edge_iter;
    edge_iterator_t<G> edge_end;
};
```

**Tests**: `tests/views/test_dfs_views.cpp`
- [ ] DFS state initializes with seed vertex
- [ ] Stack push/pop works correctly
- [ ] Visited tracking prevents revisits
- [ ] State can be shared between iterators

**Acceptance Criteria**:
- State management compiles
- Basic state operations work

---

### Step 3.2: Implement vertices_dfs

**Goal**: Implement DFS view that yields vertices.

**File**: `include/graph/views/dfs.hpp`

**Tasks**:
1. Implement `vertices_dfs_view<G, VVF, Alloc>` class
2. Implement DFS iterator with proper traversal logic
3. Implement `vertices_dfs(G&&, vertex_id_t<G>)` overload
4. Implement `vertices_dfs(G&&, vertex_id_t<G>, VVF&&)` overload

**DFS Logic**:
```cpp
// On increment:
// 1. Get current vertex's next unvisited neighbor
// 2. If found, push frame and descend
// 3. If not found, pop frame and continue parent's edges
// 4. Repeat until stack empty
```

**Tests**: `tests/views/test_dfs_views.cpp`
- [ ] Empty graph yields empty range
- [ ] Single vertex yields one element
- [ ] Path graph yields vertices in DFS order
- [ ] Tree graph yields vertices in pre-order
- [ ] Cycle is handled (no infinite loop)
- [ ] Disconnected components: only reachable vertices yielded
- [ ] VVF applied correctly
- [ ] Structured binding works

**Acceptance Criteria**:
- DFS traversal order is correct
- No revisits of vertices
- All tests pass

---

### Step 3.3: Implement edges_dfs

**Goal**: Implement DFS view that yields edges.

**File**: `include/graph/views/dfs.hpp`

**Tasks**:
1. Implement `edges_dfs_view<G, EVF, Alloc>` class
2. Reuse DFS state from Step 3.1
3. Yield edge_info instead of vertex_info
4. Implement both overloads

**Tests**: `tests/views/test_dfs_views.cpp`
- [ ] Edges yielded in DFS tree edge order
- [ ] Edge source_id and target_id correct
- [ ] EVF applied correctly
- [ ] Back edges handled appropriately (or skipped)

**Acceptance Criteria**:
- Edge DFS order matches vertex DFS order
- All tests pass

---

### Step 3.4: Implement Search Cancel

**Goal**: Implement cancel_branch and cancel_all functionality.

**File**: `include/graph/views/dfs.hpp`

**Tasks**:
1. Add `cancel_` member to DFS state
2. Add `cancel()` accessor to view
3. Check cancel state in iterator increment
4. Implement cancel_branch (skip subtree)
5. Implement cancel_all (stop iteration)

**Cancel Logic**:
```cpp
if (state_->cancel_ == cancel_search::cancel_branch) {
    // Pop current frame, continue with parent's siblings
    state_->cancel_ = cancel_search::continue_search;
}
if (state_->cancel_ == cancel_search::cancel_all) {
    // Set iterator to end
    return;
}
```

**Tests**: `tests/views/test_dfs_views.cpp`
- [ ] `cancel() = cancel_branch` skips subtree
- [ ] `cancel() = cancel_all` stops entire search
- [ ] Cancel resets after branch skip
- [ ] Cancel from value function works

**Acceptance Criteria**:
- Cancel functionality works correctly
- All tests pass

---

## Phase 4: BFS Views

### Step 4.1: Implement BFS State Management

**Goal**: Create BFS-specific state with queue instead of stack.

**File**: `include/graph/views/bfs.hpp`

**Tasks**:
1. Implement `bfs_state<G, Alloc>` class with queue
2. Track depth per vertex
3. Implement queue operations

**BFS State**:
```cpp
template<class G, class Alloc>
struct bfs_state {
    std::queue<vertex_id_t<G>, std::deque<vertex_id_t<G>, Alloc>> queue_;
    std::vector<bool, Alloc> visited_;
    std::vector<std::size_t, Alloc> depth_;  // depth per vertex
    std::size_t current_depth_ = 0;
    std::size_t count_ = 0;
    cancel_search cancel_ = cancel_search::continue_search;
};
```

**Tests**: `tests/views/test_bfs_views.cpp`
- [ ] BFS state initializes correctly
- [ ] Queue operations work
- [ ] Depth tracking per vertex works

**Acceptance Criteria**:
- State management compiles and works

---

### Step 4.2: Implement vertices_bfs

**Goal**: Implement BFS view that yields vertices.

**File**: `include/graph/views/bfs.hpp`

**Tasks**:
1. Implement `vertices_bfs_view<G, VVF, Alloc>` class
2. Implement BFS iterator (dequeue, enqueue neighbors)
3. Implement both overloads

**BFS Logic**:
```cpp
// On increment:
// 1. Dequeue front vertex
// 2. Enqueue all unvisited neighbors
// 3. Update depth tracking
```

**Tests**: `tests/views/test_bfs_views.cpp`
- [ ] Empty graph yields empty range
- [ ] Single vertex yields one element
- [ ] Path graph yields vertices in BFS order
- [ ] Tree graph yields level-order traversal
- [ ] Cycle handled (no infinite loop)
- [ ] VVF applied correctly

**Acceptance Criteria**:
- BFS traversal order is correct (level by level)
- All tests pass

---

### Step 4.3: Implement edges_bfs

**Goal**: Implement BFS view that yields edges.

**File**: `include/graph/views/bfs.hpp`

**Tasks**:
1. Implement `edges_bfs_view<G, EVF, Alloc>` class
2. Track current source vertex for edge construction
3. Yield edges in BFS order

**Tests**: `tests/views/test_bfs_views.cpp`
- [ ] Edges yielded in BFS order
- [ ] Edge source_id and target_id correct
- [ ] EVF applied correctly

**Acceptance Criteria**:
- All tests pass

---

### Step 4.4: Implement Depth and Size Tracking

**Goal**: Expose depth() and size() accessors on BFS views.

**File**: `include/graph/views/bfs.hpp`

**Tasks**:
1. Add `depth()` method returning current traversal depth
2. Add `size()` method returning elements processed
3. Ensure accessible from view and iterator

**Interface**:
```cpp
class vertices_bfs_view {
    // ...
    std::size_t depth() const { return state_->current_depth_; }
    std::size_t size() const { return state_->count_; }
};
```

**Tests**: `tests/views/test_bfs_views.cpp`
- [ ] `depth()` returns 0 for seed vertex
- [ ] `depth()` increments at each level
- [ ] `size()` starts at 0
- [ ] `size()` increments with each element
- [ ] Depth matches shortest path from seed

**Acceptance Criteria**:
- depth() and size() are accurate
- All tests pass

---

## Phase 5: Topological Sort

### Step 5.1: Implement topological_sort

**Goal**: Implement topological sort view for DAGs.

**File**: `include/graph/views/topological_sort.hpp`

**Tasks**:
1. Implement using reverse DFS post-order
2. Implement `vertices_topological_sort_view<G, VVF, Alloc>`
3. Implement `edges_topological_sort_view<G, EVF, Alloc>`
4. Implement all overloads

**Algorithm**:
```cpp
// 1. Perform DFS from all vertices
// 2. Push to result when backtracking (post-order)
// 3. Reverse for topological order
// OR use Kahn's algorithm (in-degree based)
```

**Tests**: `tests/views/test_topological_views.cpp`
- [ ] Empty graph yields empty range
- [ ] Single vertex yields one element
- [ ] DAG yields valid topological order
- [ ] All edges go from earlier to later in order
- [ ] VVF/EVF applied correctly

**Acceptance Criteria**:
- Topological order is valid
- All tests pass

---

### Step 5.2: Implement Cycle Detection

**Goal**: Handle cycles gracefully (DAG requirement).

**File**: `include/graph/views/topological_sort.hpp`

**Tasks**:
1. Detect back edges during DFS
2. Option A: Throw `graph_error` on cycle
3. Option B: Return empty range
4. Document chosen behavior

**Tests**: `tests/views/test_topological_views.cpp`
- [ ] Cycle detection identifies cycles
- [ ] Appropriate error handling (exception or empty)
- [ ] Self-loop detected
- [ ] Simple cycle detected

**Acceptance Criteria**:
- Cycles are detected and handled
- Behavior is documented

---

## Phase 6: Integration and Polish

### Step 6.1: Create views.hpp Header

**Goal**: Create unified include header for all views.

**File**: `include/graph/views.hpp`

**Tasks**:
1. Include all view headers
2. Ensure no include order issues
3. Test standalone compilation

**Content**:
```cpp
#pragma once

#include "graph/views/search_base.hpp"
#include "graph/views/view_concepts.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/neighbors.hpp"
#include "graph/views/edgelist.hpp"
#include "graph/views/basic_views.hpp"
#include "graph/views/dfs.hpp"
#include "graph/views/bfs.hpp"
#include "graph/views/topological_sort.hpp"
```

**Tests**: Compile test with just `#include "graph/views.hpp"`

**Acceptance Criteria**:
- Single include works
- No include order dependencies

---

### Step 6.2: Update graph.hpp

**Goal**: Optionally include views from main graph header.

**File**: `include/graph/graph.hpp`

**Tasks**:
1. Add `#include "graph/views.hpp"`
2. Ensure no circular dependencies
3. Update any documentation

**Acceptance Criteria**:
- Including `graph.hpp` provides views
- All existing tests still pass

---

### Step 6.3: Write Documentation

**Goal**: Document the views API.

**File**: `docs/views.md`

**Tasks**:
1. Overview of view categories
2. API reference for each view
3. Usage examples
4. Performance notes
5. Migration from graph-v2 (if applicable)

**Acceptance Criteria**:
- Documentation is complete and accurate
- Examples compile and run

---

### Step 6.4: Add Benchmarks

**Goal**: Benchmark view performance.

**File**: `benchmark/benchmark_views.cpp`

**Tasks**:
1. Benchmark vertexlist iteration
2. Benchmark edgelist iteration
3. Benchmark DFS vs BFS
4. Compare with raw iteration

**Acceptance Criteria**:
- Benchmarks run
- Views have acceptable overhead (< 10% vs raw)

---

## Test Graphs Utility

Create shared test graphs for all view tests.

**File**: `tests/views/test_graphs.hpp`

**Graphs to Define**:
```cpp
namespace test {
    auto make_empty_graph();           // 0 vertices
    auto make_single_vertex();         // 1 vertex
    auto make_path_graph(size_t n);    // 0-1-2-...-n
    auto make_cycle_graph(size_t n);   // 0-1-2-...-n-0
    auto make_complete_graph(size_t n);// K_n
    auto make_binary_tree(size_t depth);
    auto make_dag();                   // Example DAG
    auto make_disconnected();          // Multiple components
}
```

---

## Commit Strategy

After each step:
1. Run all tests: `ctest --test-dir build/linux-gcc-debug`
2. Fix any failures
3. Commit with message: `[views] Step X.Y: <description>`

Example:
```
[views] Step 2.1: Implement vertexlist view

- Added vertexlist_view class with iterator
- Added vertexlist() function overloads
- Added tests for basic functionality
- Satisfies input_range and view concepts
```
