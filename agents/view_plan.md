# Graph Views Implementation Plan

**Branch**: `feature/views-implementation`  
**Based on**: [view_strategy.md](view_strategy.md)  
**Status**: Phase 0 Complete (2026-01-31)

---

## Overview

This plan implements graph views as described in D3129 and detailed in view_strategy.md. Views provide lazy, range-based access to graph elements using structured bindings. Implementation is broken into discrete steps, each with tests and progress tracking.

### Key Design Principles
- Value functions receive descriptors (not underlying values)
- Info structs use `void` template parameters for optional members
- Primary pattern: `VId=void` with descriptors (IDs accessible via descriptor)
- Edge descriptors contain source vertex descriptor as member
- Views are lazy, zero-copy where possible

---

## Progress Tracking

### Phase 0: Info Struct Refactoring ✅ (2026-01-31)
- [x] **Step 0.1**: Refactor vertex_info (all members optional via void)
- [x] **Step 0.2**: Refactor edge_info (all members optional via void)
- [x] **Step 0.3**: Refactor neighbor_info (all members optional via void)

### Phase 1: Foundation
- [x] **Step 1.1**: Create directory structure ✅ (2026-02-01)
- [x] **Step 1.2**: Implement search_base.hpp (cancel_search, visited_tracker) ✅ (2026-02-01)
- [x] **Step 1.3**: Create view_concepts.hpp ✅ (2026-02-01)

### Phase 2: Basic Views
- [ ] **Step 2.1**: Implement vertexlist view + tests
- [ ] **Step 2.2**: Implement incidence view + tests
- [ ] **Step 2.3**: Implement neighbors view + tests
- [ ] **Step 2.4**: Implement edgelist view + tests
- [ ] **Step 2.5**: Create basic_views.hpp header

### Phase 3: DFS Views
- [ ] **Step 3.1**: Implement DFS infrastructure + vertices_dfs + tests
- [ ] **Step 3.2**: Implement edges_dfs + tests
- [ ] **Step 3.3**: Implement sourced_edges_dfs + tests
- [ ] **Step 3.4**: Test DFS cancel functionality

### Phase 4: BFS Views
- [ ] **Step 4.1**: Implement BFS infrastructure + vertices_bfs + tests
- [ ] **Step 4.2**: Implement edges_bfs + tests
- [ ] **Step 4.3**: Implement sourced_edges_bfs + tests
- [ ] **Step 4.4**: Test BFS depth/size accessors

### Phase 5: Topological Sort Views
- [ ] **Step 5.1**: Implement topological sort algorithm + vertices_topological_sort + tests
- [ ] **Step 5.2**: Implement edges_topological_sort + tests
- [ ] **Step 5.3**: Implement sourced_edges_topological_sort + tests
- [ ] **Step 5.4**: Test cycle detection

### Phase 6: Range Adaptors
- [ ] **Step 6.1**: Implement range adaptor closures for basic views
- [ ] **Step 6.2**: Implement range adaptor closures for search views
- [ ] **Step 6.3**: Test pipe syntax and chaining

### Phase 7: Integration & Polish
- [ ] **Step 7.1**: Create unified views.hpp header
- [ ] **Step 7.2**: Update graph.hpp to include views
- [ ] **Step 7.3**: Write documentation
- [ ] **Step 7.4**: Performance benchmarks
- [ ] **Step 7.5**: Edge case testing

---

## Phase 0: Info Struct Refactoring ✅ COMPLETE

**Completion Date**: 2026-01-31  
**Commit**: 330c7d8 "[views] Phase 0: Info struct refactoring complete"  
**Test Results**: ✅ 27 test cases, 392 assertions, all passing

**Implementation Summary**:
- Added 20 new VId=void specializations (4 vertex_info, 8 edge_info, 8 neighbor_info)
- Total specializations: 40 (8 vertex_info, 16 edge_info, 16 neighbor_info)
- All void template parameters physically omit corresponding members
- Comprehensive test suite created in `tests/views/`

**Key Implementation Details Discovered**:
1. **edge_info member naming**: Uses `source_id` and `target_id` (vertex IDs), NOT `edge_id`
2. **neighbor_info member naming**: 
   - Uses `source_id` and `target_id` (vertex IDs), NOT `vertex_id`
   - Uses `target` member when VId is present
   - Uses `vertex` member when VId=void (descriptor-based pattern)
3. **Sourced parameter behavior**:
   - `Sourced=true`: Includes `source_id` member (when VId present)
   - `Sourced=false`: Omits `source_id` member
4. **Padding considerations**: sizeof tests account for struct padding

---

### Step 0.1: Refactor vertex_info ✅ COMPLETE

**Goal**: Make all members of vertex_info optional via void template parameters.

**Files to Modify**:
- `include/graph/graph_info.hpp`

**Implementation**:
```cpp
// Primary template - all members present
template <class VId, class V, class VV>
struct vertex_info {
  using id_type     = VId;
  using vertex_type = V;
  using value_type  = VV;

  id_type     id;
  vertex_type vertex;
  value_type  value;
};

// Specializations for void combinations (8 total: 2^3)
// Example specialization: VId=void
template <class V, class VV>
struct vertex_info<void, V, VV> {
  using id_type     = void;
  using vertex_type = V;
  using value_type  = VV;

  vertex_type vertex;
  value_type  value;
  // No 'id' member
};

// Example specialization: VId=void, VV=void
template <class V>
struct vertex_info<void, V, void> {
  using id_type     = void;
  using vertex_type = V;
  using value_type  = void;

  vertex_type vertex;
  // No 'id' or 'value' members
};

// ... 6 more specializations for other void combinations
// Key: Members are physically absent when their type parameter is void
```

**Tests to Create**:
- `tests/views/test_vertex_info.cpp`
  - Test all 8 specializations compile
  - Test structured bindings for each variant
  - Test `vertex_info<void, vertex_descriptor<...>, int>` pattern
  - Test `vertex_info<size_t, void, int>` for external data
  - Test copyability and movability

**Acceptance Criteria**:
- All 8 specializations compile without errors
- Structured bindings work for all variants
- Void template parameters result in members being physically absent (not just zero-sized)
- `sizeof()` confirms space savings for void specializations
- Tests pass with sanitizers

**Status**: ✅ COMPLETE

**Commit Message**:
```
[views] Refactor vertex_info: all members optional via void

- VId, V, VV can all be void to suppress corresponding members
- Primary pattern: vertex_info<void, vertex_descriptor, VV>
- External data pattern: vertex_info<VId, void, VV>
- Add 8 specializations for void combinations
- Tests cover all variants and structured bindings
```

---

### Step 0.2: Refactor edge_info ✅ COMPLETE

**Goal**: Make all members of edge_info optional via void template parameters.

**Status**: ✅ COMPLETE

**Files to Modify**:
- `include/graph/graph_info.hpp`

**Implementation**:
```cpp
// Primary template - all members present
template <class VId, bool Sourced, class E, class EV>
struct edge_info {
  using source_id_type = conditional_t<Sourced, VId, void>;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = EV;

  source_id_type source_id;  // Present only when Sourced==true
  target_id_type target_id;
  edge_type      edge;
  value_type     value;
};

// Example specialization: VId=void (suppresses source_id/target_id)
template <bool Sourced, class E, class EV>
struct edge_info<void, Sourced, E, EV> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = E;
  using value_type     = EV;

  edge_type edge;
  value_type value;
  // No source_id or target_id members
};

// Example specialization: VId=void, EV=void
template <bool Sourced, class E>
struct edge_info<void, Sourced, E, void> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = E;
  using value_type     = void;

  edge_type edge;
  // No source_id, target_id, or value members
};

// ... 14 more specializations for Sourced × void combinations (16 total: 2 × 2^3)
// Key: source_id only present when Sourced==true AND VId != void
```

**Tests to Create**:
- `tests/views/test_edge_info.cpp`
  - Test all 16 specializations compile
  - Test structured bindings for each variant
  - Test `edge_info<void, true, edge_descriptor<...>, EV>` pattern
  - Test `edge_info<size_t, true, void, EV>` for external data
  - Test Sourced=true vs Sourced=false behavior
  - Test copyability and movability

**Acceptance Criteria**:
- All specializations compile without errors
- Structured bindings work for all variants
- Sourced bool correctly controls source_id presence
- Tests pass with sanitizers

**Commit Message**:
```
[views] Refactor edge_info: all members optional via void

- VId, E, EV can all be void to suppress corresponding members
- Sourced bool controls source_id presence (when VId != void)
- Primary pattern: edge_info<void, true, edge_descriptor, EV>
- External data pattern: edge_info<VId, true, void, EV>
- Add 16 specializations for Sourced × void combinations
- Tests cover all variants and structured bindings
```

---

### Step 0.3: Refactor neighbor_info ✅ COMPLETE

**Goal**: Make all members of neighbor_info optional via void template parameters.

**Status**: ✅ COMPLETE

**Implementation Note**: The actual implementation uses `target` as the member name when VId is present, and `vertex` when VId=void. This differs from the original plan which assumed consistent naming.

**Files to Modify**:
- `include/graph/graph_info.hpp`

**Implementation**:
```cpp
// Primary template - all members present
template <class VId, bool Sourced, class V, class VV>
struct neighbor_info {
  using source_id_type = conditional_t<Sourced, VId, void>;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = VV;

  source_id_type source_id;  // Present only when Sourced==true
  target_id_type target_id;
  vertex_type    vertex;
  value_type     value;
};

// Example specialization: VId=void (suppresses source_id/target_id)
// ACTUAL IMPLEMENTATION: member named 'vertex' when VId=void
template <bool Sourced, class V, class VV>
struct neighbor_info<void, Sourced, V, VV> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = V;
  using value_type     = VV;

  vertex_type vertex;  // NOTE: 'vertex' not 'target' when VId=void
  value_type  value;
  // No source_id or target_id members
};

// Example specialization: VId=void, VV=void (primary pattern for neighbors)
template <bool Sourced, class V>
struct neighbor_info<void, Sourced, V, void> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = V;
  using value_type     = void;

  vertex_type vertex;  // NOTE: 'vertex' not 'target' when VId=void
  // No source_id, target_id, or value members
};

// ... 14 more specializations for Sourced × void combinations (16 total: 2 × 2^3)
// Key: source_id only present when Sourced==true AND VId != void
```

**Tests to Create**:
- `tests/views/test_neighbor_info.cpp`
  - Test all 16 specializations compile
  - Test structured bindings for each variant
  - Test `neighbor_info<void, false, vertex_descriptor<...>, VV>` pattern
  - Test `neighbor_info<size_t, true, void, VV>` for external data
  - Test Sourced=true vs Sourced=false behavior
  - Test copyability and movability

**Acceptance Criteria**:
- All specializations compile without errors
- Structured bindings work for all variants
- Primary pattern yields {vertex, value} for neighbor iteration
- Tests pass with sanitizers

**Commit Message**:
```
[views] Refactor neighbor_info: all members optional via void

- VId, V, VV can all be void to suppress corresponding members
- Sourced bool controls source_id presence (when VId != void)
- Primary pattern: neighbor_info<void, false, vertex_descriptor, VV>
- External data pattern: neighbor_info<VId, true, void, VV>
- Add 16 specializations for Sourced × void combinations
- Tests cover all variants and structured bindings
```

---

## Phase 1: Foundation

### Step 1.1: Create directory structure

**Goal**: Set up the directory structure for view implementation.

**Files to Create**:
- `include/graph/views/` (directory)
- `tests/views/` (directory) ✅ DONE in Phase 0
- `tests/views/CMakeLists.txt` ✅ DONE in Phase 0

**Note**: `tests/views/` directory and CMakeLists.txt already created during Phase 0.

**Implementation**:
```cmake
# tests/views/CMakeLists.txt
add_executable(graph3_views_tests
    test_main.cpp
    test_vertex_info.cpp
    test_edge_info.cpp
    test_neighbor_info.cpp
)

target_link_libraries(graph3_views_tests
    PRIVATE
        graph3::graph3
        Catch2::Catch2
)

add_test(NAME views_tests COMMAND graph3_views_tests)
```

**Tests to Create**:
- `tests/views/test_main.cpp` (Catch2 main)

**Acceptance Criteria**:
- Directories created
- CMakeLists.txt properly configured
- Test executable builds and runs (even if empty)
- Integrated with parent CMakeLists.txt

**Status**: ✅ COMPLETE (2026-02-01)

**Implementation Notes**:
- `include/graph/views/` directory created
- `tests/views/` directory and infrastructure already existed from Phase 0
- CMakeLists.txt already configured with test files from Phase 0
- Test executable already builds and runs with Phase 0 tests

**Commit Message**:
```
[views] Phase 1.1: Create views directory structure

- Add include/graph/views/ directory for view implementations
- tests/views/ infrastructure already completed in Phase 0
```

---

### Step 1.2: Implement search_base.hpp

**Goal**: Implement common search infrastructure (cancel_search, visited_tracker).

**Files to Create**:
- `include/graph/views/search_base.hpp`

**Implementation**:
```cpp
namespace graph::views {

// Search cancellation control
enum class cancel_search {
    continue_search,  // Continue normal traversal
    cancel_branch,    // Skip subtree, continue with siblings
    cancel_all        // Stop entire search
};

// Visited tracking for search views
template<class VId, class Alloc = std::allocator<bool>>
class visited_tracker {
    std::vector<bool, Alloc> visited_;
public:
    explicit visited_tracker(std::size_t num_vertices, Alloc alloc = {})
        : visited_(num_vertices, false, alloc) {}
    
    bool is_visited(VId id) const {
        return visited_[static_cast<std::size_t>(id)];
    }
    
    void mark_visited(VId id) {
        visited_[static_cast<std::size_t>(id)] = true;
    }
    
    void reset() {
        std::ranges::fill(visited_, false);
    }
    
    std::size_t size() const { return visited_.size(); }
};

} // namespace graph::views
```

**Tests to Create**:
- `tests/views/test_search_base.cpp`
  - Test cancel_search enum values
  - Test visited_tracker with various VId types (size_t, int, etc.)
  - Test mark_visited and is_visited correctness
  - Test reset functionality
  - Test custom allocator support

**Acceptance Criteria**:
- cancel_search enum compiles and is usable
- visited_tracker works with size_t and other integral types
- Tests cover edge cases (empty tracker, all visited, etc.)
- Custom allocator support verified

**Status**: ✅ COMPLETE (2026-02-01)

**Implementation Notes**:
- Created `include/graph/views/search_base.hpp` with cancel_search enum and visited_tracker
- Used `std::fill` instead of `std::ranges::fill` for vector<bool> compatibility
- Added comprehensive tests covering all functionality and edge cases
- All 61 assertions in 6 test cases passing

**Commit Message**:
```
[views] Phase 1.2: Implement search_base.hpp infrastructure

- Add cancel_search enum for traversal control
- Implement visited_tracker template for DFS/BFS
- Support custom allocators for visited storage
- Tests verify correctness and edge cases (61 assertions)
```

---

### Step 1.3: Create view_concepts.hpp

**Goal**: Define concepts specific to view implementation (if needed beyond existing graph concepts).

**Files to Create**:
- `include/graph/views/view_concepts.hpp`

**Implementation**:
```cpp
namespace graph::views {

// Concept for types that can be used as value functions
template<class VF, class Descriptor>
concept vertex_value_function = 
    std::invocable<VF, Descriptor> &&
    (!std::is_void_v<std::invoke_result_t<VF, Descriptor>>);

template<class EF, class Descriptor>
concept edge_value_function = 
    std::invocable<EF, Descriptor> &&
    (!std::is_void_v<std::invoke_result_t<EF, Descriptor>>);

// Concept for search views (has depth, size, cancel)
template<class V>
concept search_view = requires(V& v, const V& cv) {
    { v.cancel() } -> std::convertible_to<cancel_search>;
    { cv.depth() } -> std::convertible_to<std::size_t>;
    { cv.size() } -> std::convertible_to<std::size_t>;
};

} // namespace graph::views
```

**Tests to Create**:
- `tests/views/test_view_concepts.cpp`
  - Test vertex_value_function concept with valid/invalid types
  - Test edge_value_function concept with valid/invalid types
  - Test search_view concept (will be used later with actual views)

**Acceptance Criteria**:
- Concepts compile and correctly constrain types
- Static assertions pass for valid/invalid cases
- Concepts integrate with existing graph concepts

**Status**: ✅ COMPLETE (2026-02-01)

**Implementation Notes**:
- Created `include/graph/views/view_concepts.hpp` with three key concepts:
  - `vertex_value_function<VVF, VertexDescriptor>` - constrains vertex value functions
  - `edge_value_function<EVF, EdgeDescriptor>` - constrains edge value functions
  - `search_view<V>` - constrains search views (requires cancel(), depth(), size())
- Comprehensive test suite with static assertions and runtime tests
- All 27 assertions in 4 test cases passing
- Tests cover valid/invalid types, different return types, mutable/capturing lambdas

**Commit Message**:
```
[views] Phase 1.3: Add view_concepts.hpp

- Define vertex_value_function and edge_value_function concepts
- Define search_view concept for DFS/BFS/topo views
- Comprehensive tests verify concept constraints (27 assertions)
- Phase 1 (Foundation) complete
```

---

## Phase 2: Basic Views

### Step 2.1: Implement vertexlist view ✅ COMPLETE

**Status**: Implemented and tested (67 new assertions, 547 total)

**Files Created**:
- `include/graph/views/vertexlist.hpp` - View implementation
- `tests/views/test_vertexlist.cpp` - Comprehensive test suite

**Implementation Summary**:
- `vertexlist_view<G, void>` - No value function variant
- `vertexlist_view<G, VVF>` - With value function variant  
- Yields `vertex_info<void, vertex_t<G>, VV>` where VV is void or invoke result
- Factory functions: `vertexlist(g)` and `vertexlist(g, vvf)`
- Uses `adjacency_list` concept (not `index_adjacency_list`)
- Constrained with `vertex_value_function` concept

**Test Coverage** (11 test cases, 67 assertions):
- Empty graph iteration
- Single and multiple vertex iteration  
- Structured bindings: `[v]` and `[v, val]`
- Various value function types (string, double, capturing, mutable)
- Deque-based graph support
- Range concepts verified (input_range, forward_range, sized_range, view)
- Iterator properties (pre/post increment, equality)
- vertex_info type verification
- Const graph access
- Weighted graph (pair edges)
- std::ranges algorithms (distance, count_if)

**Commit Message**:
```
[views] Phase 2.1: Implement vertexlist view

- Yields vertex_info<void, vertex_descriptor, VV>
- Value function receives vertex descriptor
- Supports structured bindings: [v] and [v, val]
- Tests cover iteration, value functions, const correctness
- 67 new assertions (547 total views tests)
```

---

### Step 2.2: Implement incidence view

**Goal**: Implement incidence view yielding `edge_info<void, true, edge_descriptor, EV>`.

**Files to Create**:
- `include/graph/views/incidence.hpp`

**Implementation**:
```cpp
namespace graph::views {

template<adjacency_list G, class EVF>
class incidence_view : public std::ranges::view_interface<incidence_view<G, EVF>> {
    G* g_;
    vertex_id_t<G> source_id_;
    [[no_unique_address]] EVF evf_;
    
public:
    incidence_view(G& g, vertex_id_t<G> uid, EVF evf) 
        : g_(&g), source_id_(uid), evf_(std::move(evf)) {}
    
    class iterator {
        G* g_;
        vertex_descriptor_t<G> source_;
        edge_iterator_t<G> current_;
        [[no_unique_address]] EVF* evf_;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = edge_info<void, true, edge_descriptor_t<G>, 
                                           std::invoke_result_t<EVF, edge_descriptor_t<G>>>;
        
        iterator(G* g, vertex_descriptor_t<G> src, edge_iterator_t<G> it, EVF* evf)
            : g_(g), source_(src), current_(it), evf_(evf) {}
        
        auto operator*() const {
            auto edesc = create_edge_descriptor(*g_, current_, source_);
            if constexpr (std::is_void_v<EVF>) {
                return edge_info<void, true, edge_descriptor_t<G>, void>{edesc};
            } else {
                return edge_info<void, true, edge_descriptor_t<G>, 
                               std::invoke_result_t<EVF, edge_descriptor_t<G>>>{
                    edesc, (*evf_)(edesc)
                };
            }
        }
        
        iterator& operator++() {
            ++current_;
            return *this;
        }
        
        iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        
        bool operator==(const iterator& other) const = default;
    };
    
    auto begin() { 
        auto src_desc = create_vertex_descriptor(*g_, source_id_);
        auto [first, last] = edges(*g_, source_id_);
        return iterator(g_, src_desc, first, &evf_); 
    }
    
    auto end() { 
        auto src_desc = create_vertex_descriptor(*g_, source_id_);
        auto [first, last] = edges(*g_, source_id_);
        return iterator(g_, src_desc, last, &evf_); 
    }
};

// Factory function - no value function
template<adjacency_list G>
auto incidence(G&& g, vertex_id_t<G> uid) {
    return incidence_view<std::remove_reference_t<G>, void>(g, uid, void{});
}

// Factory function - with value function
template<adjacency_list G, class EVF>
    requires edge_value_function<EVF, edge_descriptor_t<G>>
auto incidence(G&& g, vertex_id_t<G> uid, EVF&& evf) {
    return incidence_view<std::remove_reference_t<G>, std::decay_t<EVF>>(
        g, uid, std::forward<EVF>(evf)
    );
}

} // namespace graph::views
```

**Tests to Create**:
- `tests/views/test_incidence.cpp`
  - Test iteration over edges from a vertex
  - Test structured binding `[e]` and `[e, val]`
  - Test value function receives edge descriptor
  - Test edge descriptor provides source_id, target_id, edge data access
  - Test with vertices having 0, 1, many edges
  - Test const graph behavior
  - Test edge descriptor contains source vertex descriptor

**Acceptance Criteria**:
- View iterates over outgoing edges correctly
- Edge descriptor contains source vertex descriptor member
- Value function receives descriptor
- Structured bindings work
- Tests pass with sanitizers

**Commit Message**:
```
[views] Implement incidence view

- Yields edge_info<void, true, edge_descriptor, EV>
- Edge descriptor contains source vertex descriptor
- Value function receives edge descriptor
- Supports structured bindings: [e] and [e, val]
- Tests verify source context and value functions
```

---

### Step 2.3: Implement neighbors view

**Goal**: Implement neighbors view yielding `neighbor_info<void, false, vertex_descriptor, VV>`.

**Files to Create**:
- `include/graph/views/neighbors.hpp`

**Implementation**:
```cpp
namespace graph::views {

template<adjacency_list G, class VVF>
class neighbors_view : public std::ranges::view_interface<neighbors_view<G, VVF>> {
    G* g_;
    vertex_id_t<G> source_id_;
    [[no_unique_address]] VVF vvf_;
    
public:
    neighbors_view(G& g, vertex_id_t<G> uid, VVF vvf) 
        : g_(&g), source_id_(uid), vvf_(std::move(vvf)) {}
    
    class iterator {
        G* g_;
        edge_iterator_t<G> current_;
        [[no_unique_address]] VVF* vvf_;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = neighbor_info<void, false, vertex_descriptor_t<G>, 
                                               std::invoke_result_t<VVF, vertex_descriptor_t<G>>>;
        
        iterator(G* g, edge_iterator_t<G> it, VVF* vvf)
            : g_(g), current_(it), vvf_(vvf) {}
        
        auto operator*() const {
            auto target_id = graph::target_id(*g_, *current_);
            auto target_desc = create_vertex_descriptor(*g_, target_id);
            
            if constexpr (std::is_void_v<VVF>) {
                return neighbor_info<void, false, vertex_descriptor_t<G>, void>{target_desc};
            } else {
                return neighbor_info<void, false, vertex_descriptor_t<G>, 
                                   std::invoke_result_t<VVF, vertex_descriptor_t<G>>>{
                    target_desc, (*vvf_)(target_desc)
                };
            }
        }
        
        iterator& operator++() {
            ++current_;
            return *this;
        }
        
        iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        
        bool operator==(const iterator& other) const = default;
    };
    
    auto begin() { 
        auto [first, last] = edges(*g_, source_id_);
        return iterator(g_, first, &vvf_); 
    }
    
    auto end() { 
        auto [first, last] = edges(*g_, source_id_);
        return iterator(g_, last, &vvf_); 
    }
};

// Factory function - no value function
template<adjacency_list G>
auto neighbors(G&& g, vertex_id_t<G> uid) {
    return neighbors_view<std::remove_reference_t<G>, void>(g, uid, void{});
}

// Factory function - with value function
template<adjacency_list G, class VVF>
    requires vertex_value_function<VVF, vertex_descriptor_t<G>>
auto neighbors(G&& g, vertex_id_t<G> uid, VVF&& vvf) {
    return neighbors_view<std::remove_reference_t<G>, std::decay_t<VVF>>(
        g, uid, std::forward<VVF>(vvf)
    );
}

} // namespace graph::views
```

**Tests to Create**:
- `tests/views/test_neighbors.cpp`
  - Test iteration over neighbor vertices
  - Test structured binding `[v]` and `[v, val]`
  - Test value function receives target vertex descriptor
  - Test with vertices having 0, 1, many neighbors
  - Test descriptor provides access to target vertex data
  - Test const graph behavior

**Acceptance Criteria**:
- View iterates over neighbor vertices correctly
- Yields target vertex descriptor
- Value function receives target descriptor
- Structured bindings work
- Tests pass with sanitizers

**Commit Message**:
```
[views] Implement neighbors view

- Yields neighbor_info<void, false, vertex_descriptor, VV>
- Provides target vertex descriptors
- Value function receives target vertex descriptor
- Supports structured bindings: [v] and [v, val]
- Tests verify neighbor access and value functions
```

---

### Step 2.4: Implement edgelist view

**Goal**: Implement edgelist view that flattens all edges, yielding `edge_info<void, true, edge_descriptor, EV>`.

**Files to Create**:
- `include/graph/views/edgelist.hpp`

**Implementation**:
```cpp
namespace graph::views {

template<adjacency_list G, class EVF>
class edgelist_view : public std::ranges::view_interface<edgelist_view<G, EVF>> {
    G* g_;
    [[no_unique_address]] EVF evf_;
    
public:
    edgelist_view(G& g, EVF evf) : g_(&g), evf_(std::move(evf)) {}
    
    class iterator {
        G* g_;
        vertex_id_t<G> vertex_id_;
        vertex_descriptor_t<G> vertex_desc_;
        edge_iterator_t<G> edge_it_;
        edge_iterator_t<G> edge_end_;
        [[no_unique_address]] EVF* evf_;
        
        void advance_to_next_edge() {
            while (edge_it_ == edge_end_ && vertex_id_ < num_vertices(*g_)) {
                ++vertex_id_;
                if (vertex_id_ < num_vertices(*g_)) {
                    vertex_desc_ = create_vertex_descriptor(*g_, vertex_id_);
                    auto [first, last] = edges(*g_, vertex_id_);
                    edge_it_ = first;
                    edge_end_ = last;
                }
            }
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = edge_info<void, true, edge_descriptor_t<G>, 
                                           std::invoke_result_t<EVF, edge_descriptor_t<G>>>;
        
        iterator(G* g, vertex_id_t<G> vid, EVF* evf, bool is_end = false)
            : g_(g), vertex_id_(vid), evf_(evf) {
            if (!is_end && vertex_id_ < num_vertices(*g_)) {
                vertex_desc_ = create_vertex_descriptor(*g_, vertex_id_);
                auto [first, last] = edges(*g_, vertex_id_);
                edge_it_ = first;
                edge_end_ = last;
                advance_to_next_edge();
            }
        }
        
        auto operator*() const {
            auto edesc = create_edge_descriptor(*g_, edge_it_, vertex_desc_);
            if constexpr (std::is_void_v<EVF>) {
                return edge_info<void, true, edge_descriptor_t<G>, void>{edesc};
            } else {
                return edge_info<void, true, edge_descriptor_t<G>, 
                               std::invoke_result_t<EVF, edge_descriptor_t<G>>>{
                    edesc, (*evf_)(edesc)
                };
            }
        }
        
        iterator& operator++() {
            ++edge_it_;
            advance_to_next_edge();
            return *this;
        }
        
        iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        
        bool operator==(const iterator& other) const {
            return vertex_id_ == other.vertex_id_ && 
                   (vertex_id_ >= num_vertices(*g_) || edge_it_ == other.edge_it_);
        }
    };
    
    auto begin() { return iterator(g_, 0, &evf_); }
    auto end() { return iterator(g_, num_vertices(*g_), &evf_, true); }
};

// Factory function - no value function
template<adjacency_list G>
auto edgelist(G&& g) {
    return edgelist_view<std::remove_reference_t<G>, void>(g, void{});
}

// Factory function - with value function
template<adjacency_list G, class EVF>
    requires edge_value_function<EVF, edge_descriptor_t<G>>
auto edgelist(G&& g, EVF&& evf) {
    return edgelist_view<std::remove_reference_t<G>, std::decay_t<EVF>>(
        g, std::forward<EVF>(evf)
    );
}

} // namespace graph::views
```

**Tests to Create**:
- `tests/views/test_edgelist.cpp`
  - Test iteration over all edges in graph
  - Test structured binding `[e]` and `[e, val]`
  - Test edge descriptor contains source vertex descriptor
  - Test with empty graph, single edge, multiple edges
  - Test correct flattening of adjacency list structure
  - Test value function receives edge descriptor
  - Test const graph behavior

**Acceptance Criteria**:
- View correctly flattens all edges
- Edge descriptors contain source context
- Value function receives descriptor
- Structured bindings work
- Tests pass with sanitizers

**Commit Message**:
```
[views] Implement edgelist view

- Yields edge_info<void, true, edge_descriptor, EV>
- Flattens adjacency list structure
- Edge descriptor contains source vertex descriptor
- Value function receives edge descriptor
- Tests verify flattening and edge access
```

---

### Step 2.5: Create basic_views.hpp header

**Goal**: Create convenience header that includes all basic views.

**Files to Create**:
- `include/graph/views/basic_views.hpp`

**Implementation**:
```cpp
#pragma once

#include <graph/views/vertexlist.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/edgelist.hpp>
```

**Tests to Create**:
- Update existing tests to include from basic_views.hpp
- Verify no compilation issues

**Acceptance Criteria**:
- Header compiles cleanly
- All basic views accessible through single include
- No circular dependencies

**Commit Message**:
```
[views] Add basic_views.hpp convenience header

- Includes all basic view headers
- Single include for vertexlist, incidence, neighbors, edgelist
- Tests verify compilation
```

---

## Phase 3: DFS Views

### Step 3.1: Implement DFS infrastructure + vertices_dfs

**Goal**: Implement DFS traversal infrastructure and vertices_dfs view.

**Files to Create**:
- `include/graph/views/dfs.hpp`

**Implementation**:
```cpp
namespace graph::views {

template<index_adjacency_list G, class VVF, class Alloc>
class dfs_vertex_view : public std::ranges::view_interface<dfs_vertex_view<G, VVF, Alloc>> {
    struct state_t {
        using stack_entry = std::pair<vertex_id_t<G>, edge_iterator_t<G>>;
        
        std::stack<stack_entry, std::vector<stack_entry, Alloc>> stack_;
        visited_tracker<vertex_id_t<G>, Alloc> visited_;
        cancel_search cancel_ = cancel_search::continue_search;
        std::size_t depth_ = 0;
        std::size_t count_ = 0;
        
        state_t(vertex_id_t<G> seed, std::size_t num_vertices, Alloc alloc)
            : stack_(alloc), visited_(num_vertices, alloc) {
            stack_.push({seed, {}});
            visited_.mark_visited(seed);
        }
    };
    
    G* g_;
    [[no_unique_address]] VVF vvf_;
    std::shared_ptr<state_t> state_;
    
public:
    dfs_vertex_view(G& g, vertex_id_t<G> seed, VVF vvf, Alloc alloc)
        : g_(&g), vvf_(std::move(vvf)), 
          state_(std::make_shared<state_t>(seed, num_vertices(g), alloc)) {}
    
    cancel_search cancel() const { return state_->cancel_; }
    void cancel(cancel_search c) { state_->cancel_ = c; }
    std::size_t depth() const { return state_->depth_; }
    std::size_t size() const { return state_->count_; }
    
    class iterator {
        // Forward iterator implementing DFS traversal
        G* g_;
        std::shared_ptr<state_t> state_;
        [[no_unique_address]] VVF* vvf_;
        bool at_end_ = false;
        
        void advance() {
            if (state_->cancel_ == cancel_search::cancel_all || state_->stack_.empty()) {
                at_end_ = true;
                return;
            }
            
            auto [vid, edge_it] = state_->stack_.top();
            state_->stack_.pop();
            
            // Visit neighbors
            auto [first, last] = edges(*g_, vid);
            for (auto it = first; it != last; ++it) {
                auto target = target_id(*g_, *it);
                if (!state_->visited_.is_visited(target)) {
                    state_->visited_.mark_visited(target);
                    state_->stack_.push({target, {}});
                }
            }
            
            ++state_->count_;
        }
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = vertex_info<void, vertex_descriptor_t<G>, 
                                             std::invoke_result_t<VVF, vertex_descriptor_t<G>>>;
        
        iterator(G* g, std::shared_ptr<state_t> state, VVF* vvf, bool at_end)
            : g_(g), state_(std::move(state)), vvf_(vvf), at_end_(at_end) {}
        
        auto operator*() const {
            auto vid = state_->stack_.top().first;
            auto vdesc = create_vertex_descriptor(*g_, vid);
            
            if constexpr (std::is_void_v<VVF>) {
                return vertex_info<void, vertex_descriptor_t<G>, void>{vdesc};
            } else {
                return vertex_info<void, vertex_descriptor_t<G>, 
                                 std::invoke_result_t<VVF, vertex_descriptor_t<G>>>{
                    vdesc, (*vvf_)(vdesc)
                };
            }
        }
        
        iterator& operator++() {
            advance();
            return *this;
        }
        
        iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }
        
        bool operator==(const iterator& other) const {
            return at_end_ == other.at_end_;
        }
    };
    
    auto begin() { return iterator(g_, state_, &vvf_, false); }
    auto end() { return iterator(g_, state_, &vvf_, true); }
};

// Factory function
template<index_adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>>
auto vertices_dfs(G&& g, vertex_id_t<G> seed, VVF&& vvf = {}, Alloc alloc = {}) {
    if constexpr (std::is_void_v<VVF>) {
        return dfs_vertex_view<std::remove_reference_t<G>, void, Alloc>(
            g, seed, void{}, alloc
        );
    } else {
        return dfs_vertex_view<std::remove_reference_t<G>, std::decay_t<VVF>, Alloc>(
            g, seed, std::forward<VVF>(vvf), alloc
        );
    }
}

} // namespace graph::views
```

**Tests to Create**:
- `tests/views/test_dfs.cpp`
  - Test DFS traversal order (pre-order)
  - Test structured binding `[v]` and `[v, val]`
  - Test visited tracking prevents revisiting
  - Test value function receives descriptor
  - Test depth() and size() accessors
  - Test with various graph topologies (tree, cycle, DAG, disconnected)
  - Test search_view concept satisfied

**Acceptance Criteria**:
- DFS traversal order is correct
- Visited tracking works
- Shared state allows iterator copies
- Value function receives descriptor
- Tests pass with sanitizers

**Commit Message**:
```
[views] Implement DFS vertices view

- Yields vertex_info<void, vertex_descriptor, VV>
- Maintains visited set and DFS stack
- Supports depth(), size(), cancel() accessors
- Value function receives vertex descriptor
- Tests verify traversal order and state management
```

---

### Step 3.2: Implement edges_dfs

**Goal**: Implement DFS edge traversal yielding `edge_info<void, false, edge_descriptor, EV>`.

**Files to Modify**:
- `include/graph/views/dfs.hpp` (add edges_dfs implementation)

**Implementation**: Similar structure to vertices_dfs but yields edges instead of vertices.

**Tests to Create**:
- Extend `tests/views/test_dfs.cpp`
  - Test edges_dfs traversal order
  - Test structured binding `[e]` and `[e, val]`
  - Test value function receives edge descriptor
  - Test edge descriptor provides source/target access

**Acceptance Criteria**:
- Edges visited in DFS order
- Edge descriptors contain source context
- Value function receives descriptor
- Tests pass

**Commit Message**:
```
[views] Implement DFS edges view

- Yields edge_info<void, false, edge_descriptor, EV>
- Tree edges visited in DFS order
- Edge descriptor contains source vertex descriptor
- Value function receives edge descriptor
- Tests verify edge traversal
```

---

### Step 3.3: Implement sourced_edges_dfs

**Goal**: Implement sourced DFS edge traversal (Sourced=true in edge_info).

**Files to Modify**:
- `include/graph/views/dfs.hpp` (add sourced_edges_dfs)

**Implementation**: Same as edges_dfs but with Sourced=true.

**Tests to Create**:
- Extend `tests/views/test_dfs.cpp`
  - Test sourced_edges_dfs
  - Verify source context accessible

**Acceptance Criteria**:
- Sourced edges yield correct info
- Tests pass

**Commit Message**:
```
[views] Implement sourced DFS edges view

- Yields edge_info<void, true, edge_descriptor, EV>
- Source context always available via edge descriptor
- Tests verify sourced behavior
```

---

### Step 3.4: Test DFS cancel functionality

**Goal**: Verify cancel_search control works correctly.

**Tests to Create**:
- Extend `tests/views/test_dfs.cpp`
  - Test cancel_branch skips subtree
  - Test cancel_all stops traversal
  - Test continue_search normal behavior
  - Verify cancel state propagates through iterator copies

**Acceptance Criteria**:
- Cancel functionality works as specified
- Tests verify all three cancel modes

**Commit Message**:
```
[views] Test DFS cancel functionality

- Verify cancel_branch skips subtrees
- Verify cancel_all stops traversal
- Verify continue_search normal behavior
- Tests cover all cancel modes
```

---

## Phase 4: BFS Views

### Step 4.1: Implement BFS infrastructure + vertices_bfs

**Goal**: Implement BFS traversal infrastructure and vertices_bfs view.

**Files to Create**:
- `include/graph/views/bfs.hpp`

**Implementation**: Similar to DFS but using queue instead of stack.

**Tests to Create**:
- `tests/views/test_bfs.cpp`
  - Test BFS traversal order (level-order)
  - Test structured binding `[v]` and `[v, val]`
  - Test visited tracking
  - Test value function receives descriptor
  - Test depth() and size() accessors
  - Test with various graph topologies

**Acceptance Criteria**:
- BFS traversal order is correct (level-by-level)
- Visited tracking works
- Value function receives descriptor
- Tests pass with sanitizers

**Commit Message**:
```
[views] Implement BFS vertices view

- Yields vertex_info<void, vertex_descriptor, VV>
- Maintains visited set and BFS queue
- Supports depth(), size(), cancel() accessors
- Value function receives vertex descriptor
- Tests verify level-order traversal
```

---

### Step 4.2: Implement edges_bfs

**Goal**: Implement BFS edge traversal.

**Files to Modify**:
- `include/graph/views/bfs.hpp`

**Tests to Create**:
- Extend `tests/views/test_bfs.cpp`

**Acceptance Criteria**:
- BFS edge traversal works correctly
- Tests pass

**Commit Message**:
```
[views] Implement BFS edges view

- Yields edge_info<void, false, edge_descriptor, EV>
- Edges visited in BFS order
- Tests verify edge traversal
```

---

### Step 4.3: Implement sourced_edges_bfs

**Goal**: Implement sourced BFS edge traversal.

**Files to Modify**:
- `include/graph/views/bfs.hpp`

**Tests to Create**:
- Extend `tests/views/test_bfs.cpp`

**Acceptance Criteria**:
- Sourced BFS works correctly
- Tests pass

**Commit Message**:
```
[views] Implement sourced BFS edges view

- Yields edge_info<void, true, edge_descriptor, EV>
- Tests verify sourced behavior
```

---

### Step 4.4: Test BFS depth/size accessors

**Goal**: Verify depth() and size() tracking is accurate.

**Tests to Create**:
- Extend `tests/views/test_bfs.cpp`
  - Verify depth increases by level
  - Verify size counts visited vertices
  - Test on various graph structures

**Acceptance Criteria**:
- Depth and size tracking is accurate
- Tests pass

**Commit Message**:
```
[views] Test BFS depth/size accessors

- Verify depth tracks BFS levels correctly
- Verify size counts visited vertices
- Tests cover various graph topologies
```

---

## Phase 5: Topological Sort Views

### Step 5.1: Implement topological sort + vertices_topological_sort

**Goal**: Implement topological sort algorithm and vertices view.

**Files to Create**:
- `include/graph/views/topological_sort.hpp`

**Implementation**: Use reverse DFS post-order or Kahn's algorithm.

**Tests to Create**:
- `tests/views/test_topological_sort.cpp`
  - Test topological order on DAGs
  - Test structured binding `[v]` and `[v, val]`
  - Test value function receives descriptor
  - Test with various DAG structures

**Acceptance Criteria**:
- Topological order is correct (all edges point forward)
- Value function receives descriptor
- Tests pass

**Commit Message**:
```
[views] Implement topological sort vertices view

- Yields vertex_info<void, vertex_descriptor, VV>
- Uses reverse DFS post-order algorithm
- Produces valid topological ordering
- Tests verify ordering on various DAGs
```

---

### Step 5.2: Implement edges_topological_sort

**Goal**: Implement topological edge traversal.

**Files to Modify**:
- `include/graph/views/topological_sort.hpp`

**Tests to Create**:
- Extend `tests/views/test_topological_sort.cpp`

**Acceptance Criteria**:
- Edge traversal follows topological order
- Tests pass

**Commit Message**:
```
[views] Implement topological sort edges view

- Yields edge_info<void, false, edge_descriptor, EV>
- Edges follow topological ordering
- Tests verify edge order
```

---

### Step 5.3: Implement sourced_edges_topological_sort

**Goal**: Implement sourced topological edge traversal.

**Files to Modify**:
- `include/graph/views/topological_sort.hpp`

**Tests to Create**:
- Extend `tests/views/test_topological_sort.cpp`

**Acceptance Criteria**:
- Sourced topological edges work correctly
- Tests pass

**Commit Message**:
```
[views] Implement sourced topological sort edges view

- Yields edge_info<void, true, edge_descriptor, EV>
- Tests verify sourced behavior
```

---

### Step 5.4: Test cycle detection

**Goal**: Verify behavior on graphs with cycles.

**Tests to Create**:
- Extend `tests/views/test_topological_sort.cpp`
  - Test cycle detection throws or returns empty
  - Test various cycle patterns
  - Document expected behavior

**Acceptance Criteria**:
- Cycle detection works correctly
- Behavior documented
- Tests pass

**Commit Message**:
```
[views] Test topological sort cycle detection

- Verify behavior on cyclic graphs
- Document expected behavior (throw/empty)
- Tests cover various cycle patterns
```

---

## Phase 6: Range Adaptors

### Step 6.1: Implement range adaptor closures for basic views

**Goal**: Implement pipe syntax support for basic views.

**Files to Create**:
- `include/graph/views/adaptors.hpp`

**Implementation**:
```cpp
namespace graph::views {

// Adaptor for vertexlist
template<class VVF = void>
struct vertexlist_adaptor {
    [[no_unique_address]] VVF vvf;
    
    template<index_adjacency_list G>
    friend auto operator|(G&& g, vertexlist_adaptor adaptor) {
        if constexpr (std::is_void_v<VVF>) {
            return vertexlist(std::forward<G>(g));
        } else {
            return vertexlist(std::forward<G>(g), std::move(adaptor.vvf));
        }
    }
};

inline constexpr auto vertexlist_adaptor_fn = []<class VVF = void>(VVF&& vvf = {}) {
    return vertexlist_adaptor<std::decay_t<VVF>>{std::forward<VVF>(vvf)};
};

// Similar for incidence, neighbors, edgelist...

} // namespace graph::views

namespace graph::views::inline adaptors {
    inline constexpr auto vertexlist = vertexlist_adaptor_fn;
    // ... other adaptors
}
```

**Tests to Create**:
- `tests/views/test_adaptors.cpp`
  - Test `g | vertexlist()` syntax
  - Test `g | vertexlist(vvf)` with value function
  - Test `g | incidence(uid)` syntax
  - Test chaining: `g | vertexlist() | std::views::take(5)`

**Acceptance Criteria**:
- Pipe syntax compiles and works correctly
- All basic views support pipe operator
- Chaining with standard views works
- Tests pass

**Commit Message**:
```
[views] Implement range adaptor closures for basic views

- Support g | vertexlist() pipe syntax
- Support g | incidence(uid) pipe syntax
- Support g | neighbors(uid) and g | edgelist()
- Enable chaining with standard range adaptors
- Tests verify pipe syntax and chaining
```

---

### Step 6.2: Implement range adaptor closures for search views

**Goal**: Implement pipe syntax for search views.

**Files to Modify**:
- `include/graph/views/adaptors.hpp`

**Implementation**: Similar to basic views but with seed parameter.

**Tests to Create**:
- Extend `tests/views/test_adaptors.cpp`
  - Test `g | vertices_dfs(seed)` syntax
  - Test `g | vertices_bfs(seed, vvf)` syntax
  - Test `g | vertices_topological_sort()` (no seed)

**Acceptance Criteria**:
- Pipe syntax works for search views
- Tests pass

**Commit Message**:
```
[views] Implement range adaptor closures for search views

- Support g | vertices_dfs(seed) pipe syntax
- Support g | vertices_bfs(seed, vvf) pipe syntax
- Support g | vertices_topological_sort() pipe syntax
- Tests verify search view pipe syntax
```

---

### Step 6.3: Test pipe syntax and chaining

**Goal**: Comprehensive testing of range adaptor functionality.

**Tests to Create**:
- Extend `tests/views/test_adaptors.cpp`
  - Test complex chains
  - Test with std::views::transform, filter, take, etc.
  - Test const correctness with pipes

**Acceptance Criteria**:
- All chaining scenarios work correctly
- Tests demonstrate composability
- Tests pass

**Commit Message**:
```
[views] Test range adaptor pipe syntax and chaining

- Verify complex chains work correctly
- Test integration with standard range adaptors
- Test const correctness with pipes
- Comprehensive adaptor tests
```

---

## Phase 7: Integration & Polish

### Step 7.1: Create unified views.hpp header

**Goal**: Create master header for all views.

**Files to Create**:
- `include/graph/views.hpp`

**Implementation**:
```cpp
#pragma once

#include <graph/views/basic_views.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/bfs.hpp>
#include <graph/views/topological_sort.hpp>
#include <graph/views/adaptors.hpp>
```

**Tests to Create**:
- Verify single include works
- Verify no compilation issues

**Acceptance Criteria**:
- Master header compiles cleanly
- All views accessible
- No circular dependencies

**Commit Message**:
```
[views] Add unified views.hpp master header

- Includes all view headers
- Single include for complete views API
- Tests verify compilation
```

---

### Step 7.2: Update graph.hpp to include views

**Goal**: Make views available through main graph header.

**Files to Modify**:
- `include/graph/graph.hpp`

**Implementation**:
```cpp
#include <graph/views.hpp>
```

**Tests to Create**:
- Verify graph.hpp includes views
- Test that including graph.hpp gives access to all views

**Acceptance Criteria**:
- Views available through graph.hpp
- No compilation issues
- Tests pass

**Commit Message**:
```
[views] Include views in main graph.hpp header

- Add #include <graph/views.hpp> to graph.hpp
- Views now available through main header
- Tests verify availability
```

---

### Step 7.3: Write documentation

**Goal**: Create comprehensive documentation for views.

**Files to Create**:
- `docs/views.md`

**Content**:
- Overview of views
- Basic views documentation with examples
- Search views documentation with examples
- Range adaptor syntax examples
- Value function usage patterns
- Performance considerations
- Best practices

**Acceptance Criteria**:
- Documentation is comprehensive and clear
- All views documented with examples
- Code examples compile and run

**Commit Message**:
```
[views] Add comprehensive views documentation

- Document all basic and search views
- Provide usage examples for each view
- Document range adaptor syntax
- Add value function patterns
- Include performance notes
```

---

### Step 7.4: Performance benchmarks

**Goal**: Create benchmarks to measure view performance.

**Files to Create**:
- `benchmark/benchmark_views.cpp`

**Implementation**:
```cpp
// Benchmark vertexlist iteration
BENCHMARK("vertexlist_iteration") {
    auto g = create_large_graph();
    for (auto [v] : views::vertexlist(g)) {
        benchmark::do_not_optimize(v);
    }
};

// Benchmark incidence iteration
BENCHMARK("incidence_iteration") {
    auto g = create_large_graph();
    for (vertex_id_t<decltype(g)> u = 0; u < num_vertices(g); ++u) {
        for (auto [e] : views::incidence(g, u)) {
            benchmark::do_not_optimize(e);
        }
    }
};

// Benchmark DFS traversal
BENCHMARK("dfs_traversal") {
    auto g = create_large_graph();
    for (auto [v] : views::vertices_dfs(g, 0)) {
        benchmark::do_not_optimize(v);
    }
};

// Similar for BFS, topological sort, etc.
```

**Acceptance Criteria**:
- Benchmarks compile and run
- Performance is reasonable (comparable to manual iteration)
- Results documented

**Commit Message**:
```
[views] Add performance benchmarks for all views

- Benchmark basic view iteration
- Benchmark search view traversal
- Compare with manual iteration where applicable
- Document performance characteristics
```

---

### Step 7.5: Edge case testing

**Goal**: Comprehensive edge case coverage.

**Tests to Create**:
- `tests/views/test_edge_cases.cpp`
  - Empty graphs
  - Single vertex graphs
  - Disconnected graphs
  - Self-loops
  - Parallel edges
  - Very large graphs (stress test)
  - Const graphs
  - Move-only value types in value functions
  - Exception safety

**Acceptance Criteria**:
- All edge cases handled correctly
- No crashes or undefined behavior
- Tests pass with sanitizers (ASAN, UBSAN, TSAN)

**Commit Message**:
```
[views] Add comprehensive edge case tests

- Test empty and single-vertex graphs
- Test disconnected graphs
- Test self-loops and parallel edges
- Test const correctness
- Test exception safety
- All tests pass with sanitizers
```

---

## Final Integration

### Merge to main

**Prerequisites**:
- All steps completed
- All tests passing
- Documentation complete
- Code reviewed
- Benchmarks run and documented

**Process**:
1. Rebase feature branch on latest main
2. Run full test suite
3. Run sanitizers
4. Run benchmarks
5. Create pull request
6. Address review comments
7. Merge to main

**Final Commit Message**:
```
[views] Complete graph views implementation

This PR implements graph views as described in D3129:

**Basic Views**:
- vertexlist: iterate over all vertices
- incidence: iterate over outgoing edges
- neighbors: iterate over adjacent vertices
- edgelist: iterate over all edges (flattened)

**Search Views**:
- vertices_dfs/edges_dfs/sourced_edges_dfs
- vertices_bfs/edges_bfs/sourced_edges_bfs
- vertices_topological_sort/edges_topological_sort/sourced_edges_topological_sort

**Features**:
- Descriptor-based design (value functions receive descriptors)
- Info structs with optional members via void template parameters
- Range adaptor pipe syntax (g | view(...))
- Lazy evaluation with zero-copy where possible
- Comprehensive test coverage
- Full documentation
- Performance benchmarks

Closes #<issue-number>
```

---

## Notes for Agent Execution

### General Guidelines
- **Incremental commits**: Commit after each step completion
- **Test-driven**: Write tests first or alongside implementation
- **Code quality**: Follow project style guidelines
- **Error handling**: Check for errors, use concepts for constraints
- **Documentation**: Add inline comments for complex logic
- **Sanitizers**: Run with ASAN/UBSAN/TSAN to catch issues

### Testing Strategy
- Use Catch2 test framework
- Test with both vector and deque based graphs
- Include const correctness tests
- Test with empty graphs
- Test boundary conditions
- Use structured bindings in tests to verify info struct layout

### Performance Considerations
- Use `[[no_unique_address]]` for empty base optimization
- Avoid unnecessary copies (use references where appropriate)
- Consider lazy evaluation for value functions
- Share state between iterators using `std::shared_ptr`

### Common Pitfalls to Avoid
- Don't forget to handle empty graphs
- Don't forget const correctness
- Don't assume random access (some graphs use deque)
- Don't hardcode vertex/edge types
- Test with both void and non-void value function cases

---

**End of Implementation Plan**
