# Edge List Implementation Strategy

This document provides a detailed strategy for implementing the abstract edge list support as 
outlined in [edge_list_goal.md](edge_list_goal.md).

## Overview

The edge list implementation provides an alternative graph representation focused on edges rather 
than vertices. It complements the existing adjacency list (`graph::adj_list`) implementation by 
allowing algorithms to work directly with edge ranges.

### Key Terminology

| Term | Description |
|------|-------------|
| `edge list` | The abstract concept of a list of edges |
| `edge_list` | The namespace `graph::edge_list` containing the implementation |
| `edgelist` | Views for iterating over all edges (e.g., `edgelist(g)` view over adjacency list) |

---

## Current State Analysis

### Existing Components

1. **`include/graph/edgelist.hpp`** - Contains a basic `edgelist` container class:
   - `graph::edge<VId, EV>` - Simple edge struct with source/target/value
   - `graph::edgelist<VId, EV, Alloc>` - Vector-based edge container
   - Basic iterator support and modifiers

2. **`include/graph/graph_info.hpp`** - Shared types:
   - `edge_info<VId, Sourced, E, EV>` - Template specializations for edge projections
   - `edgelist_edge<VId, E, EV>` - Alias for sourced edge info
   - `copyable_edge_t<VId, EV>` - Lightweight copyable edge

3. **Reference implementation** at `/mnt/d/dev_graph/graph-v2/`:
   - `include/graph/edgelist.hpp` - Edge list concepts, types, and CPOs (in `graph::edge_list` namespace)
   - `include/graph/views/edgelist.hpp` - Edgelist view for adjacency lists

### Gap Analysis

| Component | Current Status | Required |
|-----------|---------------|----------|
| `graph::edge_list` namespace | Exists in graph-v2, not in v3 | ✅ Needed |
| Edge list concepts | Not in v3 | ✅ Needed |
| Edge list type aliases | Not in v3 | ✅ Needed |
| Edge list CPOs | Not in v3 | ✅ Needed |
| Edgelist view for adj_list | Not in v3 | ✅ Needed (future) |
| Descriptor support | Not in graph-v2 | ✅ Needs design |

---

## Implementation Strategy

### Phase 1: Core Edge List Namespace

**Goal**: Create the `graph::edge_list` namespace with concepts, types, and CPOs.

#### 1.1 Create `include/graph/edge_list/edge_list_concepts.hpp`

Define edge list concepts modeled after the adjacency list pattern:

```cpp
namespace graph::edge_list {

// Concept: An edge list is a range of edges with source_id and target_id
template <class EL>
concept basic_sourced_edgelist = 
    std::ranges::input_range<EL> &&
    !std::ranges::range<std::ranges::range_value_t<EL>> &&  // distinguish from adj_list
    requires(std::ranges::range_value_t<EL> e) {
        { source_id(e) };
        { target_id(e) } -> std::same_as<decltype(source_id(e))>;
    };

template <class EL>
concept basic_sourced_index_edgelist = 
    basic_sourced_edgelist<EL> &&
    requires(std::ranges::range_value_t<EL> e) {
        { source_id(e) } -> std::integral;
    };

template <class EL>
concept has_edge_value = 
    basic_sourced_edgelist<EL> &&
    requires(std::ranges::range_value_t<EL> e) {
        { edge_value(e) };
    };

} // namespace graph::edge_list
```

#### 1.2 Create `include/graph/edge_list/edge_list_types.hpp`

Define type aliases for edge lists:

```cpp
namespace graph::edge_list {

template <basic_sourced_edgelist EL>
using edge_range_t = EL;

template <basic_sourced_edgelist EL>
using edge_iterator_t = std::ranges::iterator_t<edge_range_t<EL>>;

template <basic_sourced_edgelist EL>
using edge_t = std::ranges::range_value_t<edge_range_t<EL>>;

template <basic_sourced_edgelist EL>
using edge_reference_t = std::ranges::range_reference_t<edge_range_t<EL>>;

template <basic_sourced_edgelist EL>
using vertex_id_t = decltype(source_id(std::declval<edge_t<EL>>()));

template <has_edge_value EL>
using edge_value_t = decltype(edge_value(std::declval<edge_t<EL>>()));

} // namespace graph::edge_list
```

#### 1.3 Create `include/graph/edge_list/edge_list_cpo.hpp`

Define CPOs for edge lists (edge-level operations):

```cpp
namespace graph::edge_list {

// CPO: source_id(e) - Get source vertex ID from edge
inline constexpr auto source_id = /* CPO implementation */;

// CPO: target_id(e) - Get target vertex ID from edge  
inline constexpr auto target_id = /* CPO implementation */;

// CPO: edge_value(e) - Get edge value (optional)
inline constexpr auto edge_value = /* CPO implementation */;

// Edge list-level CPOs
// CPO: num_edges(el) - Get number of edges
inline constexpr auto num_edges = /* CPO implementation */;

} // namespace graph::edge_list
```

#### 1.4 Create `include/graph/edge_list/edge_list.hpp`

Main header that includes all edge list components:

```cpp
#pragma once

#include "edge_list_concepts.hpp"
#include "edge_list_types.hpp"
#include "edge_list_cpo.hpp"
```

---

### Phase 2: Edge List Container

**Goal**: Refactor the existing `edgelist.hpp` to conform to the edge list concepts.

#### 2.1 Refactor `include/graph/edgelist.hpp`

Move the container to the edge_list namespace and ensure it satisfies concepts:

```cpp
namespace graph::edge_list {

template<typename VId = std::size_t, 
         typename EV = void,
         typename Alloc = std::allocator<edge<VId, EV>>>
class edgelist {
    // ... existing implementation
    
    // Ensure CPO customization points work
    friend constexpr VId source_id(const edge<VId, EV>& e) noexcept { return e.source; }
    friend constexpr VId target_id(const edge<VId, EV>& e) noexcept { return e.target; }
    // ... etc
};

} // namespace graph::edge_list
```

#### 2.2 Add Descriptor Support

Design consideration: Edge lists need descriptor support for consistency with v3.

```cpp
namespace graph::edge_list {

// Edge descriptor for edge lists
template<typename VId, typename EV>
struct edge_descriptor {
    VId source_id;
    VId target_id;
    // Optional: pointer/iterator to underlying edge for value access
};

} // namespace graph::edge_list
```

---

### Phase 3: Default CPO Implementations

**Goal**: Provide default implementations for common edge types.

#### 3.1 Support Standard Types

The following types should work with edge list CPOs out-of-the-box:

| Type | `source_id(e)` | `target_id(e)` | `edge_value(e)` |
|------|----------------|----------------|-----------------|
| `pair<T,T>` | `e.first` | `e.second` | N/A |
| `tuple<T,T>` | `get<0>(e)` | `get<1>(e)` | N/A |
| `tuple<T,T,EV,...>` | `get<0>(e)` | `get<1>(e)` | `get<2>(e)` |
| `edge_info<VId,true,void,void>` | `e.source_id` | `e.target_id` | N/A |
| `edge_info<VId,true,void,EV>` | `e.source_id` | `e.target_id` | `e.value` |
| `edge<VId,void>` | `e.source` | `e.target` | N/A |
| `edge<VId,EV>` | `e.source` | `e.target` | `e.value` |

---

### Phase 4: Edgelist Views (Future)

**Goal**: Create views that iterate over all edges in an adjacency list.

#### 4.1 Create `include/graph/views/edgelist_view.hpp`

```cpp
namespace graph::views {

// edgelist(g) - Iterate all edges in adjacency list as edge_info
template <adj_list::adjacency_list G>
class edgelist_view {
    // Iterator that walks vertices and their edges
    // Projects to edge_info<VId, true, E, void>
};

// edgelist(g, evf) - With edge value function
template <adj_list::adjacency_list G, typename EVF>
class edgelist_view {
    // Projects to edge_info<VId, true, E, EV>
};

// CPO: edgelist(g) and edgelist(g, evf)
inline constexpr auto edgelist = /* range adaptor */;

} // namespace graph::views
```

---

## File Structure

```
include/graph/
├── edge_list/
│   ├── edge_list.hpp              # Main include header
│   ├── edge_list_concepts.hpp     # Concepts
│   ├── edge_list_types.hpp        # Type aliases
│   ├── edge_list_cpo.hpp          # CPO definitions
│   └── edge_list_container.hpp    # edgelist container
├── views/
│   └── edgelist_view.hpp          # Edgelist view for adj_list (Phase 4)
├── edgelist.hpp                   # Backward compat → includes edge_list/
├── graph_info.hpp                 # (existing) shared edge_info types
└── graph.hpp                      # Update to import edge_list namespace
```

---

## Implementation Order

### Milestone 1: Core Framework
1. [ ] Create `include/graph/edge_list/` directory
2. [ ] Implement `edge_list_concepts.hpp`
3. [ ] Implement `edge_list_types.hpp`
4. [ ] Implement `edge_list_cpo.hpp` with CPO infrastructure

### Milestone 2: Container Integration
5. [ ] Refactor existing `edgelist.hpp` into `edge_list_container.hpp`
6. [ ] Add friend functions for CPO customization
7. [ ] Create backward-compatible `edgelist.hpp` wrapper
8. [ ] Design and implement edge descriptors

### Milestone 3: Testing
9. [ ] Create `tests/test_edge_list_concepts.cpp`
10. [ ] Create `tests/test_edge_list_cpo.cpp`
11. [ ] Test with standard types (pair, tuple)
12. [ ] Test with custom edge types

### Milestone 4: Edgelist Views (Future)
13. [ ] Implement `edgelist_view.hpp`
14. [ ] Create edgelist view CPO
15. [ ] Test edgelist view with adjacency list graphs

---

## Design Decisions

### Decision 1: CPO Architecture

**Options**:
- A) Follow `adj_list` pattern with `_cpo_impls` namespace
- B) Simpler direct CPO implementation
- C) Niebloid-style function objects

**Recommendation**: Option A - Follow `adj_list` pattern for consistency.

### Decision 2: Descriptor Support

**Options**:
- A) No descriptors (like graph-v2)
- B) Lightweight edge descriptors (VId pair only)
- C) Full descriptor support with edge reference

**Recommendation**: Option B - Start lightweight, extend as needed.

### Decision 3: Namespace Organization

**Options**:
- A) Everything in `graph::edge_list`
- B) Split: `graph::edge_list` for types, `graph::edge_list::views` for views

**Recommendation**: Option A initially, refactor if needed.

---

## Testing Strategy

### Unit Tests

1. **Concept satisfaction tests**:
   - `static_assert(basic_sourced_edgelist<vector<pair<int,int>>>)`
   - `static_assert(basic_sourced_edgelist<edgelist<int>>)`

2. **CPO tests**:
   - Test `source_id`, `target_id`, `edge_value` with various types
   - Test `num_edges` with edge list containers

3. **Container tests**:
   - Add/remove edges
   - Iteration
   - Size queries

### Integration Tests

1. Algorithm compatibility (future)
2. Conversion between edge list and adjacency list (future)

---

## References

- [edge_list_goal.md](edge_list_goal.md) - Goals document
- [graph_cpo_implementation.md](../docs/graph_cpo_implementation.md) - CPO patterns
- `/mnt/d/dev_graph/graph-v2/include/graph/edgelist.hpp` - Reference implementation
- `/mnt/d/dev_graph/graph-v2/include/graph/views/edgelist.hpp` - Reference view implementation

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-25 | 1.0 | Initial strategy document |
