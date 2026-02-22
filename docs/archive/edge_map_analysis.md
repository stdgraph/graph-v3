# Edge Map Analysis: Phase 4.3 Design Document

**Date:** December 28, 2024  
**Phase:** 4.3.1 - Analysis for Map-Based Edge Containers  
**Status:** COMPLETE

## Executive Summary

This document analyzes the changes needed to support `std::map<VId, edge_type>` as an edge container in `dynamic_graph`, enabling O(log n) edge lookup by target vertex ID. The analysis reveals that **minimal changes are required** - the existing infrastructure already supports map-based edge containers through the `push_or_insert` utility and generic iterator patterns.

## 1. Current Edge Container Architecture

### 1.1 Edge Storage Interface

Currently supported edge containers in `dynamic_vertex_base`:

```cpp
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex_base {
  using edges_type = typename Traits::edges_type;
  // Can be: vector, list, forward_list, deque, set, unordered_set
  
private:
  edges_type edges_;  // Direct storage of edge container
  
public:
  constexpr edges_type& edges() noexcept { return edges_; }
  constexpr auto begin() noexcept { return edges_.begin(); }
  constexpr auto end() noexcept { return edges_.end(); }
};
```

**Key observation:** The `edges_type` is already a generic type from `Traits`, so any STL-compatible container can be used.

### 1.2 Edge Insertion Mechanism

Edge insertion uses the generic `push_or_insert` utility from `container_utility.hpp`:

```cpp
template <class C>
constexpr auto push_or_insert(C& container) {
  if constexpr (has_emplace_back<C>)
    return [&](auto&& value) { container.emplace_back(std::move(value)); };
  else if constexpr (has_emplace<C>)
    return [&](auto&& value) { container.emplace(std::move(value)); };
  else if constexpr (has_insert<C>)
    return [&](const auto& value) { container.insert(value); };
  // ... other variants
}
```

**Key observation:** This already supports `std::map::insert()` via the `has_insert<C>` branch.

### 1.3 Edge Iteration Pattern

All edge iteration uses standard iterator protocols:

```cpp
for (auto& edge : vertex.edges()) {
  // Process edge via iterator dereference
}
```

**Key observation:** Works uniformly across all container types.

## 2. Map-Based Edge Container Design

### 2.1 Proposed Edge Map Type

```cpp
using edges_type = std::map<VId, edge_type>;
```

**Semantics:**
- **Key:** Target vertex ID (for Sourced=false) or `std::pair<VId, VId>` (for Sourced=true)
- **Value:** `dynamic_edge<EV, VV, GV, VId, Sourced, Traits>`
- **Ordering:** Automatically sorted by target ID
- **Uniqueness:** Only one edge per target (no parallel edges)
- **Lookup:** O(log n) by target ID
- **Insertion:** O(log n)

### 2.2 Alternative for Sourced Graphs

For `Sourced=true`, we have two design options:

**Option A: Key = target_id only (simpler, recommended)**
```cpp
using edges_type = std::map<VId, edge_type>;
```
- Map key is target_id
- Source ID is stored in edge_type
- Simple, consistent with Sourced=false

**Option B: Key = pair<source_id, target_id> (more complex)**
```cpp
using edges_type = std::map<std::pair<VId, VId>, edge_type>;
```
- Map key contains both IDs
- Redundant with edge_type storage
- More complex, not recommended

**Recommendation:** Use Option A - key by target_id only, matching current edge comparison semantics.

## 3. Required Changes Analysis

### 3.1 Container Utility (`container_utility.hpp`)

**Status:** ✅ **NO CHANGES REQUIRED**

The existing `push_or_insert` already handles map insertion:

```cpp
else if constexpr (has_insert<C>) {
  return [&container](const typename C::value_type& value) { 
    container.insert(value); 
  };
}
```

For `std::map<VId, edge_type>`, the `value_type` is `std::pair<const VId, edge_type>`.

**However**, there's a semantic consideration:

Current edge construction:
```cpp
auto edge = dynamic_edge<...>(target_id, edge_value);
edge_adder(edge);  // For vector/set, this works
```

For map, we need:
```cpp
auto edge = dynamic_edge<...>(target_id, edge_value);
edge_adder(std::pair{target_id, edge});  // Wrap in pair for map
```

**Action Required:** Add a map-aware wrapper function to construct pairs automatically.

### 3.2 Edge Descriptor (`edge_descriptor.hpp`)

**Status:** ✅ **NO CHANGES REQUIRED**

The `edge_descriptor` already uses generic iterators:

```cpp
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
class edge_descriptor {
  using edge_storage_type = std::conditional_t<
    std::random_access_iterator<EdgeIter>,
    std::size_t,        // Store index for vector
    EdgeIter            // Store iterator for set/map
  >;
};
```

For `std::map`, `EdgeIter` is a bidirectional iterator (not random access), so the descriptor stores the iterator itself. This already works.

**Target ID Extraction:**

Current code:
```cpp
const auto& edge_val = *edge_storage_;  // Dereference iterator

if constexpr (requires { edge_val.target_id(); }) {
  return edge_val.target_id();  // For dynamic_edge
}
```

For map iterator, `*it` returns `std::pair<const VId, edge_type>&`, so we need:

```cpp
if constexpr (requires { edge_val.second.target_id(); }) {
  return edge_val.second.target_id();  // Access pair's second element
}
else if constexpr (requires { edge_val.target_id(); }) {
  return edge_val.target_id();  // Direct edge access
}
```

**Action Required:** Update `target_id()` extraction logic to handle pair-wrapped edges.

### 3.3 Dynamic Edge (`dynamic_edge` classes)

**Status:** ✅ **NO CHANGES REQUIRED**

The edge types already store target_id internally:

```cpp
class dynamic_edge_target {
  VId target_id_;
public:
  constexpr VId target_id() const { return target_id_; }
};
```

This works regardless of whether edges are stored in a map or vector.

**Comparison Operators:**

Already defined:
```cpp
constexpr auto operator<=>(const dynamic_edge& rhs) const noexcept {
  if constexpr (Sourced) {
    if (auto cmp = source_id() <=> rhs.source_id(); cmp != 0)
      return cmp;
  }
  return target_id() <=> rhs.target_id();
}
```

This is compatible with map ordering.

### 3.4 Graph Construction (`load_edges`)

**Status:** ⚠️ **MINOR CHANGES REQUIRED**

Current edge loading uses `push_or_insert`:

```cpp
auto&& edge_adder = push_or_insert(vertices_[e.source_id].edges());
if constexpr (Sourced)
  edge_adder(edge_type(e.source_id, e.target_id, e.value));
else
  edge_adder(edge_type(e.target_id, e.value));
```

For map-based edges, this needs to wrap the edge in a pair:

```cpp
if constexpr (is_map_based_edge_container<edges_type>) {
  if constexpr (Sourced)
    edge_adder(std::pair{e.target_id, edge_type(e.source_id, e.target_id, e.value)});
  else
    edge_adder(std::pair{e.target_id, edge_type(e.target_id, e.value)});
} else {
  // Current code path for vector/set
}
```

**Action Required:** Add conditional logic in `load_edges` to detect map-based containers and wrap edges appropriately.

### 3.5 CPO Functions

Most CPO functions work generically. Key considerations:

#### `edges(g, u)` - Edge Range
**Status:** ✅ **NO CHANGES REQUIRED**

Returns `edge_descriptor_view` which wraps any iterator type.

#### `find_vertex_edge(g, u, vid)` - Find Specific Edge
**Status:** ⚠️ **OPTIMIZATION OPPORTUNITY**

Current implementation (commented out):
```cpp
// return std::ranges::find(g[uid].edges_, 
//   [&](const edge_type& uv) { return target_id(g, uv) == vid; });
```

This is O(n) linear search. For map-based edges, we can optimize:

```cpp
if constexpr (is_map_based_edge_container<edges_type>) {
  auto it = g[uid].edges_.find(vid);  // O(log n)
  return it != g[uid].edges_.end() ? 
    edge_descriptor{it, vertex_descriptor{...}} : 
    end_sentinel;
} else {
  // Current linear search
}
```

**Action Required:** Implement optimized `find_vertex_edge` for map-based containers.

#### `contains_edge(g, uid, vid)` - Edge Existence Check
**Status:** ⚠️ **OPTIMIZATION OPPORTUNITY**

Current implementation likely uses `find_vertex_edge` and checks for end. With map:

```cpp
if constexpr (is_map_based_edge_container<edges_type>) {
  return g[uid].edges_.contains(vid);  // O(log n), C++20
} else {
  // Current implementation
}
```

## 4. Implementation Strategy

### 4.1 Minimal Viable Changes

**Required for Phase 4.3.2 (vom_graph_traits):**

1. **Add `is_map_based_edge_container` concept** (container_utility.hpp)
   ```cpp
   template <class C>
   concept is_map_based_edge_container = 
     is_associative_container<C> && 
     requires { typename C::key_type; } &&
     // Key is VId or pair<VId, VId>
     (std::same_as<typename C::key_type, typename C::mapped_type::vertex_id_type> ||
      std::is_pair_v<typename C::key_type>);
   ```

2. **Add `emplace_edge` helper** (container_utility.hpp)
   ```cpp
   template <class C, class... Args>
   void emplace_edge(C& edges, VId target_id, Args&&... args) {
     if constexpr (is_map_based_edge_container<C>) {
       edges.emplace(target_id, edge_type(std::forward<Args>(args)...));
     } else {
       edges.emplace(edge_type(std::forward<Args>(args)...));
     }
   }
   ```

3. **Update `load_edges` functions** (dynamic_graph.hpp)
   - Replace direct `push_or_insert` calls with `emplace_edge`
   - Automatically constructs pairs for map-based containers

4. **Update `edge_descriptor::target_id()`** (edge_descriptor.hpp)
   - Add pair-detection for map iterator dereference
   - Extract `.second` when edge is wrapped in pair

### 4.2 Optional Optimizations

These can be added later in Phase 4.3:

1. **Optimized `find_vertex_edge`** - Use `map::find()` instead of linear search
2. **Optimized `contains_edge`** - Use `map::contains()` for O(log n) vs O(n)
3. **Optimized edge iteration** - Could skip pair unwrapping in some contexts

### 4.3 Testing Strategy

For `vom_graph_traits` (vector vertices + map edges):

1. **Basic functionality tests:**
   - Edge insertion (automatic target_id keying)
   - Edge uniqueness (no parallel edges)
   - Edge ordering (sorted by target_id)
   - Edge iteration (bidirectional)

2. **CPO tests:**
   - `edges(g, u)` returns correct range
   - `target_id(g, uv)` extracts from map pair
   - `find_vertex_edge(g, u, vid)` finds edges (initially O(n), optimize later)
   - `contains_edge(g, uid, vid)` works correctly

3. **Comparison with existing traits:**
   - vom should behave like vov except:
     - No parallel edges (map uniqueness)
     - Edges sorted by target_id
     - O(log n) lookup instead of O(n)

## 5. Edge Map vs. Edge Set Comparison

| Feature | `std::set<edge_type>` | `std::map<VId, edge_type>` |
|---------|----------------------|---------------------------|
| **Key** | Entire edge (target + source if Sourced) | Target VId only |
| **Lookup by target** | O(log n) with find + predicate | O(log n) with map::find(vid) |
| **Storage** | Edges stored directly | Edges wrapped in pair<VId, edge> |
| **Memory** | edge_type only | pair overhead (~8-16 bytes/edge) |
| **Ordering** | By full edge comparison | By target_id only |
| **Parallel edges** | Can have multiple edges to same target (if source differs) | Only one edge per target |
| **Iterator deref** | `edge_type&` | `pair<const VId, edge_type>&` |

**Use Cases:**

- **Edge Set:** Multigraphs, source-distinguished edges, minimal memory
- **Edge Map:** Fast target lookup, guaranteed unique edges per target, direct target access

## 6. Design Decisions

### 6.1 Map Key Selection

**Decision:** Use `VId` (target_id) as the map key, NOT `std::pair<VId, VId>`.

**Rationale:**
1. **Consistency:** Matches the natural lookup pattern `g[uid].edges_.find(vid)`
2. **Simplicity:** Single-key maps are simpler than pair-key maps
3. **Storage:** Source ID still stored in edge_type when Sourced=true
4. **Semantics:** "Edges from vertex u to vertex v" naturally keyed by v

### 6.2 Edge Descriptor Storage

**Decision:** Continue using iterator storage for non-random-access containers.

**Rationale:**
1. **Existing pattern:** Already works for set, list, etc.
2. **Stability:** Map iterators remain valid during insert (unless the element itself is erased)
3. **Simplicity:** No special casing needed

### 6.3 Load Strategy

**Decision:** Detect map-based containers at compile time and adjust edge construction.

**Rationale:**
1. **Zero runtime cost:** All decisions made via `if constexpr`
2. **Type safety:** Compiler enforces correct pair construction
3. **Backward compatibility:** Existing code paths unchanged

## 7. Potential Issues and Mitigations

### 7.1 Issue: Parallel Edges Lost

**Problem:** Map uniqueness means only one edge per target, even if multiple edges should exist.

**Mitigation:**
- Document this limitation clearly in vom/mom trait headers
- For multigraphs, users should use vector/list/set edge containers
- This is a feature, not a bug: map-based edges are for graphs with unique edges

### 7.2 Issue: Iterator Invalidation

**Problem:** Map insert/erase can invalidate iterators to other elements (rare, but possible in some implementations).

**Mitigation:**
- Standard guarantees: map insert does NOT invalidate existing iterators (except in rebalancing, which doesn't invalidate)
- Erase only invalidates the erased element
- Edge descriptors storing iterators remain valid during most operations

### 7.3 Issue: Pair Overhead

**Problem:** `std::pair<const VId, edge_type>` adds memory overhead vs direct edge storage.

**Analysis:**
```
sizeof(pair<uint32_t, edge_type>) = 
  sizeof(uint32_t) + sizeof(edge_type) + padding
  
For typical edge (1-2 VId members): ~8-16 bytes overhead per edge
```

**Mitigation:**
- Document memory characteristics
- For memory-critical applications, recommend vector/set edge containers
- The O(log n) lookup benefit often justifies the overhead

## 8. Implementation Checklist

### Phase 4.3.1 (Analysis) - ✅ COMPLETE

- [x] Review dynamic_vertex edge container usage patterns
- [x] Identify changes needed for map-based edge access
- [x] Design edge_descriptor changes for map-based edges
- [x] Document findings in edge_map_analysis.md

### Phase 4.3.2 (vom_graph_traits) - ⏳ PENDING

- [ ] Add `is_map_based_edge_container` concept to container_utility.hpp
- [ ] Add `emplace_edge` helper function to container_utility.hpp
- [ ] Update `edge_descriptor::target_id()` to handle map pair unwrapping
- [ ] Update `load_edges` functions to use map-aware edge construction
- [ ] Create vom_graph_traits.hpp
- [ ] Create test_dynamic_graph_vom.cpp with basic tests
- [ ] Create test_dynamic_graph_cpo_vom.cpp with CPO tests
- [ ] Verify all tests pass

### Phase 4.3.3 (mom_graph_traits) - ⏳ PENDING

- [ ] Create mom_graph_traits.hpp (reuse vom infrastructure)
- [ ] Create test files for mom
- [ ] Verify all tests pass

## 9. Conclusion

**Summary:** Map-based edge containers are **fully compatible** with the existing `dynamic_graph` architecture with minimal changes:

1. **Container Utility:** Add concept detection and helper for map pair construction
2. **Edge Descriptor:** Update target_id extraction to unwrap map pairs
3. **Load Functions:** Add conditional logic for map-based edge insertion
4. **Trait Headers:** Define new vom/mom traits with `std::map<VId, edge_type>`

**Estimated Effort:**
- Container utility changes: ~50 lines
- Edge descriptor updates: ~20 lines  
- Load function updates: ~30 lines per load variant
- Trait headers: ~40 lines each (vom, mom)
- Test files: ~2000 lines per trait (existing template)

**Total:** ~200 lines of core changes + ~4000 lines of tests for two new traits.

**Risk:** LOW - Changes are localized, compile-time detected, and follow existing patterns.

**Recommendation:** Proceed with Phase 4.3.2 implementation.
