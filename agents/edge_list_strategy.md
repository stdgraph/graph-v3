# Edge List Implementation Strategy

This document provides a detailed strategy for implementing the abstract edge list support as 
outlined in [edge_list_goal.md](edge_list_goal.md).

## Overview

The edge list implementation provides an alternative graph representation focused on edges rather 
than vertices. It complements the existing adjacency list (`graph::adj_list`) implementation by 
allowing algorithms to work directly with edge ranges.

**The key unification goal**: `source_id(g,uv)`, `target_id(g,uv)`, and `edge_value(g,uv)` CPOs 
must work uniformly across:
1. **Adjacency list edges** - edges from `edges(g, u)` where `g` is an adjacency list
2. **Edge list values** - elements in a `graph::edge_list` range
3. **Edgelist views** - edges from an `edgelist(g)` view over an adjacency list

This unification enables algorithms to work with any edge source without specialization.

### Key Terminology

| Term | Description |
|------|-------------|
| `edge list` | The abstract concept: a range of edges with source_id, target_id, and optional edge_value |
| `edge_list` | The namespace `graph::edge_list` containing implementations (underscore separator) |
| `edgelist` | A view that iterates over all edges in an adjacency list (one word, no separator) |
| `edgelist(g)` | CPO that creates an edgelist view from an adjacency list graph |

---

## Current State Analysis

### Existing Components

1. **`include/graph/edge_list/edge_list.hpp`** - Edge list concepts and types:
   - `graph::edge_list::basic_sourced_edgelist<EL>` - Core edgelist concept
   - `graph::edge_list::basic_sourced_index_edgelist<EL>` - Integral vertex IDs
   - `graph::edge_list::has_edge_value<EL>` - Concept for valued edges
   - Type aliases: `edge_range_t`, `edge_iterator_t`, `edge_t`, `edge_reference_t`, `edge_value_t`, `vertex_id_t`
   - Currently uses unqualified `source_id(e)`, `target_id(e)` - needs adjustment for unified CPOs

2. **`include/graph/graph_info.hpp`** - Shared types:
   - `edge_info<VId, Sourced, E, EV>` - Template specializations for edge projections
   - `edgelist_edge<VId, E, EV>` - Alias for sourced edge info
   - `copyable_edge_t<VId, EV>` - Lightweight copyable edge

3. **`include/graph/adj_list/detail/graph_cpo.hpp`** - CPO implementations:
   - `source_id(g, uv)` - Four-tier resolution (native member, graph member, ADL, descriptor)
   - `target_id(g, uv)` - Similar four-tier resolution
   - `edge_value(g, uv)` - Similar pattern
   - **Key issue**: These CPOs require a graph parameter `g`, but edge_list concepts expect `source_id(e)` without graph

4. **Reference implementation** at `/mnt/d/dev_graph/graph-v2/`:
   - `include/graph/edgelist.hpp` - Edge list concepts, types, and CPOs (in `graph::edge_list` namespace)
   - `include/graph/views/edgelist.hpp` - Edgelist view for adjacency lists

### Gap Analysis

| Component | Current Status | Required |
|-----------|---------------|----------|
| `graph::edge_list` namespace | ✅ Exists in v3 | Extend with CPO support |
| Edge list concepts | ✅ Exists in v3 | Adjust to use unified CPOs |
| Edge list type aliases | ✅ Exists in v3 | Complete |
| Unified CPO support | ❌ Missing | **Critical**: Extend CPOs for edge_list |
| Edge descriptor for edge_list | ❌ Missing | Design needed |
| `edgelist(g)` view | ❌ Missing | Future phase |

### Key Design Challenge

The current architecture has a fundamental tension:

**Adjacency List CPOs**: `source_id(g, uv)` - requires graph context because:
- The edge descriptor needs the graph to resolve vertex references
- The graph provides the context for source/target resolution

**Edge List CPOs**: `source_id(e)` - edge-only because:
- Edge list elements are self-contained (source_id, target_id, value stored directly)
- No graph context needed

**Resolution Strategy**: We need a unified approach where both forms work:
- For adjacency lists: `source_id(g, uv)` remains the canonical form
- For edge lists: `source_id(g, e)` where `g` is the edge_list container (or use sentinel)
- CPOs detect edge type and dispatch appropriately

---

## Implementation Strategy

### Phase 1: Unified CPO Architecture

**Goal**: Extend existing CPOs to support edge_list values alongside adjacency list edge descriptors.

#### 1.1 CPO Unification Design

The unified CPO approach extends the existing `adj_list` CPOs to handle edge_list types:

```cpp
// Current: source_id(g, uv) where uv is edge_descriptor<G>
// Extended: source_id(g, e) where e can be:
//   - edge_descriptor (adjacency list edge)
//   - edge_info (edge list value)
//   - tuple/pair (generic edge representation)
//   - any type satisfying edge_list::basic_edge concept
```

**CPO Resolution Order** (extended from current 4-tier to 6-tier):

| Priority | Strategy | Description |
|----------|----------|-------------|
| 1 | Native edge member | `(*uv.value()).source_id()` - Native edge type member |
| 2 | Graph member | `g.source_id(uv)` - Graph's member function |
| 3 | ADL with graph | `source_id(g, uv)` - ADL-findable free function |
| 4 | Descriptor member | `uv.source_id()` - Edge descriptor's member |
| 5 | **Edge-only member** | `e.source_id` - Direct member access (edge_info) |
| 6 | **Tuple/pair access** | `get<0>(e)` - For tuple-like edge types |

#### 1.2 Extend `graph_cpo.hpp`

Add edge_list support to existing CPOs in `include/graph/adj_list/detail/graph_cpo.hpp`:

```cpp
namespace _source_id {
    enum class _St { 
        _none, 
        _native_edge_member, 
        _member, 
        _adl, 
        _descriptor,
        _edge_info_member,    // NEW: for edge_info<VId, true, ...>
        _tuple_like           // NEW: for pair/tuple
    };
    
    // NEW: Check for edge_info-style direct member access
    template<typename G, typename E>
    concept _has_edge_info_member = requires(const E& e) {
        { e.source_id } -> std::integral;
    };
    
    // NEW: Check for tuple-like edge (pair, tuple)
    template<typename G, typename E>
    concept _is_tuple_like_edge = 
        !is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(const E& e) {
            { std::get<0>(e) } -> std::integral;
            { std::get<1>(e) } -> std::integral;
        };
    
    // Extended _Choose function...
}
```

#### 1.3 Update Edge List Concepts

Modify `include/graph/edge_list/edge_list.hpp` concepts to use unified CPOs:

```cpp
namespace graph::edge_list {

// Forward declare the unified CPO usage
// Edge list concepts now check if CPOs work with a "dummy" graph context

template <class EL>
concept basic_sourced_edgelist = 
    std::ranges::input_range<EL> &&
    !std::ranges::range<std::ranges::range_value_t<EL>> && // distinguish from adj_list
    requires(EL& el, std::ranges::range_value_t<EL> e) {
        // Use unified CPO with edge_list as context
        { graph::source_id(el, e) };
        { graph::target_id(el, e) } -> std::same_as<decltype(graph::source_id(el, e))>;
    };

} // namespace graph::edge_list
```

**Alternative Design**: If the edge types are self-describing (have source_id/target_id members), 
the graph parameter becomes a no-op context. This preserves API consistency.

---

### Phase 2: Edge Descriptor for Edge Lists

**Goal**: Define an edge descriptor type for edge lists that enables uniform algorithm usage.

#### 2.1 Edge List Descriptor Design

Unlike adjacency list edge descriptors (which wrap iterators), edge list descriptors are simpler:

```cpp
namespace graph::edge_list {

/**
 * @brief Lightweight edge descriptor for edge lists
 * 
 * For edge lists, the "descriptor" is essentially the edge value itself
 * since edges in an edge list are self-contained.
 */
template<typename VId, typename EV = void>
struct edge_descriptor {
    VId source_id_;
    VId target_id_;
    [[no_unique_address]] std::conditional_t<std::is_void_v<EV>, 
        detail::empty_value, EV> value_;
    
    constexpr VId source_id() const noexcept { return source_id_; }
    constexpr VId target_id() const noexcept { return target_id_; }
    
    // Only available when EV is not void
    constexpr auto& value() const noexcept 
        requires (!std::is_void_v<EV>) { return value_; }
};

// Type trait to distinguish edge_list descriptors
template<typename T>
struct is_edge_list_descriptor : std::false_type {};

template<typename VId, typename EV>
struct is_edge_list_descriptor<edge_descriptor<VId, EV>> : std::true_type {};

template<typename T>
inline constexpr bool is_edge_list_descriptor_v = is_edge_list_descriptor<T>::value;

} // namespace graph::edge_list
```

#### 2.2 CPO Support for Edge List Descriptors

The unified CPOs detect edge_list descriptors and access members directly:

```cpp
// In graph_cpo.hpp _source_id namespace
template<typename E>
concept _is_edge_list_descriptor = 
    edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<E>>;

// Resolution adds: if constexpr (_is_edge_list_descriptor<E>) { return e.source_id(); }
```

---

### Phase 3: Support Standard Edge Types

**Goal**: Provide out-of-box support for common edge representations.

#### 3.1 Supported Types Matrix

| Type | `source_id(g,e)` | `target_id(g,e)` | `edge_value(g,e)` |
|------|------------------|------------------|-------------------|
| `pair<T,T>` | `e.first` | `e.second` | N/A |
| `tuple<T,T>` | `get<0>(e)` | `get<1>(e)` | N/A |
| `tuple<T,T,EV>` | `get<0>(e)` | `get<1>(e)` | `get<2>(e)` |
| `tuple<T,T,EV,...>` | `get<0>(e)` | `get<1>(e)` | `get<2>(e)` |
| `edge_info<VId,true,void,void>` | `e.source_id` | `e.target_id` | N/A |
| `edge_info<VId,true,void,EV>` | `e.source_id` | `e.target_id` | `e.value` |
| `edge_info<VId,true,E&,void>` | `e.source_id` | `e.target_id` | N/A |
| `edge_info<VId,true,E&,EV>` | `e.source_id` | `e.target_id` | `e.value` |
| `edge_list::edge_descriptor<VId,EV>` | `e.source_id()` | `e.target_id()` | `e.value()` |
| Custom types | ADL or member | ADL or member | ADL or member |

#### 3.2 Implementation in CPO

```cpp
namespace _source_id {
    // Tuple-like detection and access
    template<typename E>
    concept _is_tuple_like = requires(const E& e) {
        { std::tuple_size_v<std::remove_cvref_t<E>> } -> std::convertible_to<std::size_t>;
        { std::get<0>(e) };
    };
    
    template<typename E>
    concept _has_source_id_member = requires(const E& e) {
        { e.source_id } -> std::integral;
    };
    
    // In _fn::operator():
    // ... existing priority checks ...
    // else if constexpr (_has_source_id_member<E>) {
    //     return e.source_id;
    // } else if constexpr (_is_tuple_like<E>) {
    //     return std::get<0>(e);
    // }
}
```

---

### Phase 4: Edgelist View (Future)

**Goal**: Create a view that presents an adjacency list as an edge list.

#### 4.1 Edgelist View Design

```cpp
namespace graph::views {

/**
 * @brief View that iterates over all edges in an adjacency list
 * 
 * Transforms an adjacency list into an edge list view, allowing algorithms
 * designed for edge lists to work with adjacency lists.
 */
template <adj_list::adjacency_list G>
class edgelist_view : public std::ranges::view_interface<edgelist_view<G>> {
public:
    class iterator {
        // Iterates through vertices, then through each vertex's edges
        // Projects each edge to edge_info<VId, true, edge_t<G>, void>
    };
    
    explicit edgelist_view(G& g) : g_(&g) {}
    
    iterator begin() const;
    std::default_sentinel_t end() const { return {}; }
    
private:
    G* g_;
};

// With value projection
template <adj_list::adjacency_list G, typename EVF>
class edgelist_view_with_value;

// CPO
inline constexpr auto edgelist = /* range adaptor closure */;

} // namespace graph::views
```

#### 4.2 Algorithm Compatibility

With the unified CPO design, algorithms can accept both:

```cpp
template<typename EdgeRange>
    requires edge_list::basic_sourced_edgelist<EdgeRange>
void some_edge_algorithm(EdgeRange&& edges) {
    for (auto&& e : edges) {
        auto src = graph::source_id(edges, e);  // Works for edge_list
        auto tgt = graph::target_id(edges, e);  // Works for edgelist view
        // ...
    }
}
```

---

## File Structure

```
include/graph/
├── edge_list/
│   ├── edge_list.hpp              # Main header (existing, to be updated)
│   ├── edge_list_descriptor.hpp   # NEW: Edge list descriptor type
│   └── detail/
│       └── edge_list_cpo.hpp      # NEW: Edge list CPO helpers (if needed)
├── views/
│   └── edgelist_view.hpp          # NEW: Edgelist view for adj_list (Phase 4)
├── adj_list/
│   └── detail/
│       └── graph_cpo.hpp          # MODIFY: Extend CPOs for edge_list support
├── graph_info.hpp                 # (existing) shared edge_info types
└── graph.hpp                      # MODIFY: Update imports, remove edgelist.hpp ref
```

---

## Implementation Order

### Milestone 1: CPO Unification (Critical Path)
1. [ ] Extend `source_id` CPO in `graph_cpo.hpp` to support edge_info and tuple types
2. [ ] Extend `target_id` CPO in `graph_cpo.hpp` to support edge_info and tuple types
3. [ ] Extend `edge_value` CPO in `graph_cpo.hpp` to support edge_info and tuple types
4. [ ] Add type traits for edge_list types in descriptor_traits.hpp or new file

### Milestone 2: Edge List Descriptor
5. [ ] Create `include/graph/edge_list/edge_list_descriptor.hpp`
6. [ ] Add `is_edge_list_descriptor` type trait
7. [ ] Integrate descriptor support into CPOs

### Milestone 3: Update Edge List Concepts
8. [ ] Update `edge_list.hpp` concepts to use unified CPO pattern
9. [ ] Ensure backward compatibility with existing code
10. [ ] Update `graph.hpp` to properly include edge_list (remove edgelist.hpp reference)

### Milestone 4: Testing
11. [ ] Create `tests/edge_list/test_edge_list_concepts.cpp`
12. [ ] Create `tests/edge_list/test_edge_list_cpo.cpp`
13. [ ] Test CPOs with standard types (pair, tuple, edge_info)
14. [ ] Test CPOs with custom edge types via ADL

### Milestone 5: Edgelist View (Future)
15. [ ] Implement `edgelist_view.hpp`
16. [ ] Create `edgelist` range adaptor CPO
17. [ ] Test edgelist view with adjacency list graphs
18. [ ] Verify algorithm compatibility across edge sources

---

## Design Decisions

### Decision 1: CPO Architecture - Unified vs Separate

**Options**:
- A) Separate CPOs: `graph::adj_list::source_id(g,e)` and `graph::edge_list::source_id(e)`
- B) Unified CPOs: Single `graph::source_id(g,e)` that works with all edge types
- C) Overloaded CPOs: Both `source_id(g,e)` and `source_id(e)` forms in single namespace

**Decision**: Option B - Unified CPOs

**Rationale**:
- Algorithms can be written once to work with any edge source
- Maintains API consistency with existing adjacency list CPOs
- The graph/edge_list parameter provides context even when not strictly needed
- Follows the principle of least surprise for library users

### Decision 2: Graph Parameter for Edge Lists

**Options**:
- A) Require graph parameter: `source_id(el, e)` where `el` is the edge list container
- B) Optional graph parameter: `source_id(e)` when edge is self-describing
- C) Tag dispatch: Use `source_id(edge_list_tag{}, e)` for edge lists

**Decision**: Option A - Require graph parameter (edge list container)

**Rationale**:
- Maintains consistent API: `source_id(context, edge)` everywhere
- For self-describing edges, the context is simply ignored by the CPO
- Enables future extensions (e.g., edge list with external vertex mapping)
- Algorithms written for adjacency lists work with edge lists with minimal changes

### Decision 3: Edge Descriptor Design

**Options**:
- A) No descriptors for edge lists (edges are self-contained values)
- B) Lightweight value-based descriptors (copy of source_id, target_id, optional value)
- C) Iterator-based descriptors (like adjacency list edge descriptors)

**Decision**: Option B - Lightweight value-based descriptors

**Rationale**:
- Edge list edges are already value types, so descriptors are essentially wrappers
- Provides uniform descriptor interface for algorithm compatibility
- Avoids iterator invalidation issues
- Simple implementation with good performance characteristics

### Decision 4: Namespace Organization

**Options**:
- A) Everything in `graph::edge_list`, views in `graph::edge_list::views`
- B) Types/concepts in `graph::edge_list`, views in `graph::views`
- C) All edge_list items exported to `graph::` namespace (like adj_list)

**Decision**: Option B with selective export to `graph::`

**Rationale**:
- Keeps implementation organized in dedicated namespaces
- `graph::views::edgelist` parallels `graph::views::vertices` pattern
- Common types/CPOs exported to `graph::` for convenience
- Matches existing `adj_list` namespace organization

### Decision 5: Concept Constraint Style

**Options**:
- A) Concepts require specific member names (e.g., `e.source_id`)
- B) Concepts require CPO expressions (e.g., `graph::source_id(el, e)`)
- C) Concepts use both approaches with fallback

**Decision**: Option B - Concepts require CPO expressions

**Rationale**:
- CPO-based concepts allow any customization mechanism to work
- More flexible for user-defined edge types
- Matches modern C++ library design patterns
- Provides consistent constraint style across the library

---

## Testing Strategy

### Unit Tests

1. **CPO Tests with Various Edge Types**:
   - `source_id(el, pair<int,int>)` → returns `e.first`
   - `source_id(el, tuple<int,int,double>)` → returns `get<0>(e)`
   - `source_id(el, edge_info<int,true,void,void>)` → returns `e.source_id`
   - `source_id(g, edge_descriptor)` → returns via descriptor (existing)

2. **Concept Satisfaction Tests**:
   ```cpp
   static_assert(edge_list::basic_sourced_edgelist<vector<pair<int,int>>>);
   static_assert(edge_list::basic_sourced_edgelist<vector<edge_info<int,true,void,void>>>);
   static_assert(!edge_list::basic_sourced_edgelist<adjacency_list>); // fails range-of-non-range check
   ```

3. **Descriptor Tests**:
   - Construct edge_list::edge_descriptor from edge_info
   - Verify CPOs work with edge_list descriptors
   - Test equality, comparison, hashing

4. **Integration with Existing CPOs**:
   - Verify `adj_list` CPOs still work unchanged
   - Test both edge types in same compilation unit

### Integration Tests

1. **Algorithm Compatibility** (future):
   - Simple edge iteration algorithm works with edge_list
   - Same algorithm works with edgelist view over adjacency list
   
2. **Conversion Tests** (future):
   - Build adjacency list from edge_list
   - Create edgelist view from adjacency list, verify iteration

---

## References

- [edge_list_goal.md](edge_list_goal.md) - Goals document (revised)
- [graph_cpo_implementation.md](../docs/graph_cpo_implementation.md) - CPO patterns
- [graph_cpo.hpp](../include/graph/adj_list/detail/graph_cpo.hpp) - Existing CPO implementations
- [edge_list.hpp](../include/graph/edge_list/edge_list.hpp) - Current edge list concepts
- [graph_info.hpp](../include/graph/graph_info.hpp) - Shared edge_info types
- `/mnt/d/dev_graph/graph-v2/include/graph/edgelist.hpp` - Reference implementation
- `/mnt/d/dev_graph/graph-v2/include/graph/views/edgelist.hpp` - Reference view implementation

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-25 | 1.0 | Initial strategy document |
| 2026-01-31 | 2.0 | Major revision: Unified CPO architecture, updated for revised goals |
