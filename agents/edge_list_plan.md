# Edge List Unification Implementation Plan

This document provides a detailed, step-by-step implementation plan for the edge list unification
work described in [edge_list_strategy.md](edge_list_strategy.md). Each step is designed to be
executed by an agent and includes specific tasks, files to modify, and tests to create.

**Branch**: `feature/edge-list-unification`

**Reference Documents**:
- [edge_list_goal.md](edge_list_goal.md) - Goals and requirements
- [edge_list_strategy.md](edge_list_strategy.md) - Technical strategy and design decisions

---

## Progress Tracking

| Step | Description | Status | Commit |
|------|-------------|--------|--------|
| 1.1 | Add `is_edge_list_descriptor_v` trait | ✅ Complete | 4997cb3 |
| 1.2 | Add `_has_edge_info_member` concept to `source_id` CPO | ✅ Complete | |
| 1.3 | Add `_is_tuple_like_edge` concept to `source_id` CPO | ✅ Complete | |
| 1.3a | Add `_has_edge_info_member` and `_is_tuple_like_edge` to `target_id` CPO | ✅ Complete | |
| 1.3b | Add `_has_edge_info_member` and `_is_tuple_like_edge` to `edge_value` CPO | ✅ Complete | |
| 1.4 | Extend `source_id` CPO with tiers 5-7 | ✅ Complete | |
| 1.5 | Create tests for `source_id` CPO extensions | ✅ Complete (partial - source_id only) | |
| 2.1 | Extend `target_id` CPO with tiers 5-7 | ✅ Complete | |
| 2.2 | Create tests for `target_id` CPO extensions | ✅ Complete | |
| 3.1 | Extend `edge_value` CPO with tiers 5-7 | ⬜ Not Started | |
| 3.2 | Create tests for `edge_value` CPO extensions | ⬜ Not Started | |
| 4.1 | Create `edge_list::edge_descriptor` type | ⬜ Not Started | |
| 4.2 | Add `is_edge_list_descriptor` specialization | ⬜ Not Started | |
| 4.3 | Create tests for `edge_list::edge_descriptor` | ⬜ Not Started | |
| 5.1 | Update `edge_list.hpp` concepts to use unified CPOs | ⬜ Not Started | |
| 5.2 | Create tests for updated edge_list concepts | ⬜ Not Started | |
| 6.1 | Update `graph.hpp` imports | ⬜ Not Started | |
| 6.2 | Final integration tests | ⬜ Not Started | |

**Legend**: ⬜ Not Started | 🔄 In Progress | ✅ Complete | ❌ Blocked

---

## Phase 1: Extend `source_id` CPO

### Step 1.1: Add `is_edge_list_descriptor_v` Trait

**Goal**: Create the type trait that identifies `edge_list::edge_descriptor` types.

**Files to create/modify**:
- `include/graph/edge_list/edge_list_traits.hpp` (NEW)

**Tasks**:
1. Create new file `include/graph/edge_list/edge_list_traits.hpp`
2. Define forward declaration of `edge_list::edge_descriptor` template
3. Define `is_edge_list_descriptor<T>` trait (defaults to `false_type`)
4. Define `is_edge_list_descriptor_v<T>` variable template
5. Add include guard and namespace structure

**Code to implement**:
```cpp
#pragma once

#include <type_traits>

namespace graph::edge_list {

// Forward declaration - actual type defined in edge_list_descriptor.hpp
template<typename VId, typename EV>
struct edge_descriptor;

// Type trait to identify edge_list descriptors
template<typename T>
struct is_edge_list_descriptor : std::false_type {};

template<typename VId, typename EV>
struct is_edge_list_descriptor<edge_descriptor<VId, EV>> : std::true_type {};

template<typename T>
inline constexpr bool is_edge_list_descriptor_v = is_edge_list_descriptor<T>::value;

} // namespace graph::edge_list
```

**Tests**: None yet (trait needs the actual type to test against, added in Step 4.2)

**Commit message**: `Add is_edge_list_descriptor trait for edge_list type detection`

---

### Step 1.2: Add `_has_edge_info_member` Concept

**Goal**: Add concept to detect edge_info-style data member access (Tier 6).

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Locate `namespace _source_id` in `graph_cpo.hpp`
2. Add include for `edge_list_traits.hpp` at top of file
3. Add `_has_edge_info_member` concept after existing concepts

**Code to add** (inside `namespace _source_id`):
```cpp
// Tier 6: Check for edge_info-style direct data member access
// Must NOT be a descriptor type (to avoid ambiguity with method calls)
template<typename UV>
concept _has_edge_info_member = 
    !is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
    !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
    requires(const UV& uv) {
        uv.source_id;  // data member, not method
    } &&
    !requires(const UV& uv) {
        uv.source_id();  // exclude if it's callable (i.e., a method)
    };
```

**Tests**: Created in Step 1.5

**Commit message**: `Add _has_edge_info_member concept to source_id CPO`

---

### Step 1.3: Add `_is_tuple_like_edge` Concept

**Goal**: Add concept to detect tuple-like edge types (Tier 7).

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Locate `namespace _source_id` in `graph_cpo.hpp`
2. Add `_is_tuple_like_edge` concept after `_has_edge_info_member`

**Code to add** (inside `namespace _source_id`):
```cpp
// Tier 7: Check for tuple-like edge (pair, tuple)
// Must NOT be any descriptor type or have edge_info members
template<typename UV>
concept _is_tuple_like_edge = 
    !is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
    !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
    !_has_edge_info_member<UV> &&
    requires {
        std::tuple_size<std::remove_cvref_t<UV>>::value;
    } &&
    requires(const UV& uv) {
        { std::get<0>(uv) };
        { std::get<1>(uv) };
    };
```

**Tests**: Created in Step 1.5

**Commit message**: `Add _is_tuple_like_edge concept to source_id CPO`

---

### Step 1.3a: Add Concepts to `target_id` CPO

**Goal**: Add the same concepts to `target_id` for consistency.

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Locate `namespace _target_id` in `graph_cpo.hpp`
2. Add `_has_edge_info_member` concept (checks `uv.target_id` data member)
3. Add `_is_tuple_like_edge` concept (same as source_id)

**Status**: ✅ Complete (implemented alongside source_id concepts)

---

### Step 1.3b: Add Concepts to `edge_value` CPO

**Goal**: Add the same concepts to `edge_value` for consistency.

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Locate `namespace _edge_value` in `graph_cpo.hpp`
2. Add `_has_edge_info_member` concept (checks `uv.value` data member)
3. Add `_is_tuple_like_edge` concept (checks for `std::get<2>` since edge value is third element)

**Status**: ✅ Complete (implemented alongside source_id concepts)

**Commit message** (for 1.2, 1.3, 1.3a, 1.3b combined): `Add edge_info and tuple-like concepts to all three CPOs`

---

### Step 1.4: Extend `source_id` CPO with Tiers 5-7

**Goal**: Update the `_St` enum and `_Choose` function to support new tiers.

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Rename `_descriptor` to `_adj_list_descriptor` in `_St` enum
2. Add new enum values: `_edge_list_descriptor`, `_edge_info_member`, `_tuple_like`
3. Update `_Choose()` function to check new tiers in order
4. Update `_fn::operator()` to handle new strategies
5. Ensure noexcept propagation for new tiers

**Enum update**:
```cpp
enum class _St { 
    _none, 
    _native_edge_member, 
    _member, 
    _adl, 
    _adj_list_descriptor,      // RENAMED from _descriptor
    _edge_list_descriptor,     // NEW
    _edge_info_member,         // NEW
    _tuple_like                // NEW
};
```

**`_Choose()` additions** (after existing tier 4 check):
```cpp
} else if constexpr (_has_edge_list_descriptor<UV>) {
    return {_St::_edge_list_descriptor,
            noexcept(std::declval<const UV&>().source_id())};
} else if constexpr (_has_edge_info_member<UV>) {
    return {_St::_edge_info_member,
            noexcept(std::declval<const UV&>().source_id)};
} else if constexpr (_is_tuple_like_edge<UV>) {
    return {_St::_tuple_like,
            noexcept(std::get<0>(std::declval<const UV&>()))};
```

**`operator()` additions**:
```cpp
} else if constexpr (_Choice<_G, _UV>._Strategy == _St::_edge_list_descriptor) {
    return uv.source_id();
} else if constexpr (_Choice<_G, _UV>._Strategy == _St::_edge_info_member) {
    return uv.source_id;
} else if constexpr (_Choice<_G, _UV>._Strategy == _St::_tuple_like) {
    return std::get<0>(uv);
```

**Tests**: Created in Step 1.5

**Commit message**: `Extend source_id CPO with tiers 5-7 for edge_list support`

---

### Step 1.5: Create Tests for `source_id` CPO Extensions

**Goal**: Test all new tiers and ambiguity cases for `source_id`.

**Files to create**:
- `tests/edge_list/test_edge_list_cpo.cpp` (NEW)
- `tests/edge_list/CMakeLists.txt` (NEW or modify parent)

**Tasks**:
1. Create test directory structure if needed
2. Create test file with Catch2 framework
3. Test each new tier with appropriate edge types
4. Test ambiguity guards

**Test cases to implement**:
```cpp
// Test Tier 6: edge_info data member
TEST_CASE("source_id with edge_info", "[cpo][source_id]") {
    using EI = graph::edge_info<int, true, void, void>;
    EI ei{1, 2};
    std::vector<EI> el{ei};
    
    auto uid = graph::source_id(el, ei);
    REQUIRE(uid == 1);
}

// Test Tier 7: tuple-like
TEST_CASE("source_id with pair", "[cpo][source_id]") {
    std::pair<int, int> edge{3, 4};
    std::vector<std::pair<int, int>> el{edge};
    
    auto uid = graph::source_id(el, edge);
    REQUIRE(uid == 3);
}

TEST_CASE("source_id with tuple", "[cpo][source_id]") {
    std::tuple<int, int, double> edge{5, 6, 1.5};
    std::vector<std::tuple<int, int, double>> el{edge};
    
    auto uid = graph::source_id(el, edge);
    REQUIRE(uid == 5);
}

// Ambiguity tests
TEST_CASE("source_id prefers data member over tuple", "[cpo][source_id][ambiguity]") {
    // Type with both source_id member and tuple-like interface
    // Should pick data member (Tier 6)
    struct EdgeWithBoth {
        int source_id;
        int target_id;
        template<std::size_t I>
        friend auto get(const EdgeWithBoth& e) {
            if constexpr (I == 0) return e.source_id;
            else return e.target_id;
        }
    };
    // ... test that data member tier is selected
}
```

**CMakeLists.txt update**:
```cmake
add_executable(test_edge_list_cpo test_edge_list_cpo.cpp)
target_link_libraries(test_edge_list_cpo PRIVATE graph3::graph3 Catch2::Catch2WithMain)
add_test(NAME test_edge_list_cpo COMMAND test_edge_list_cpo)
```

**Commit message**: `Add tests for source_id CPO tiers 5-7`

---

## Phase 2: Extend `target_id` CPO

### Step 2.1: Extend `target_id` CPO with Tiers 5-7

**Goal**: Mirror the `source_id` changes to `target_id`.

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Locate `namespace _target_id` in `graph_cpo.hpp`
2. Add same concepts as `source_id` but checking `target_id` member
3. Update `_St` enum with new values
4. Update `_Choose()` and `operator()` functions

**Key differences from `source_id`**:
- `_has_edge_info_member` checks `uv.target_id` instead of `uv.source_id`
- Tuple tier uses `std::get<1>(uv)` instead of `std::get<0>(uv)`

**Commit message**: `Extend target_id CPO with tiers 5-7 for edge_list support`

---

### Step 2.2: Create Tests for `target_id` CPO Extensions

**Goal**: Test all new tiers for `target_id`.

**Files to modify**:
- `tests/edge_list/test_edge_list_cpo.cpp`

**Tasks**:
1. Add test cases mirroring `source_id` tests but for `target_id`
2. Verify `target_id` returns correct element (second, not first)

**Commit message**: `Add tests for target_id CPO tiers 5-7`

---

## Phase 3: Extend `edge_value` CPO

### Step 3.1: Extend `edge_value` CPO with Tiers 5-7

**Goal**: Mirror the changes to `edge_value`.

**Files to modify**:
- `include/graph/adj_list/detail/graph_cpo.hpp`

**Tasks**:
1. Locate `namespace _edge_value` in `graph_cpo.hpp`
2. Add concepts checking `uv.value` member or `std::get<2>(uv)`
3. Update `_St` enum with new values
4. Update `_Choose()` and `operator()` functions

**Key differences**:
- `_has_edge_info_member` checks `uv.value` instead of `uv.source_id`
- Tuple tier uses `std::get<2>(uv)` for the edge value
- Not all edge types have values (pair<T,T> does not)

**Commit message**: `Extend edge_value CPO with tiers 5-7 for edge_list support`

---

### Step 3.2: Create Tests for `edge_value` CPO Extensions

**Goal**: Test edge_value with edge_info and tuple types.

**Files to modify**:
- `tests/edge_list/test_edge_list_cpo.cpp`

**Tasks**:
1. Add test cases for `edge_value` with `edge_info<VId,true,void,EV>`
2. Add test cases for `edge_value` with `tuple<T,T,EV>`
3. Verify types without values (pair, edge_info without EV) don't satisfy edge_value

**Commit message**: `Add tests for edge_value CPO tiers 5-7`

---

## Phase 4: Create Edge List Descriptor

### Step 4.1: Create `edge_list::edge_descriptor` Type

**Goal**: Implement the lightweight edge descriptor for edge lists.

**Files to create**:
- `include/graph/edge_list/edge_list_descriptor.hpp` (NEW)

**Tasks**:
1. Create new file with proper includes
2. Implement `edge_descriptor<VId, EV>` template
3. Handle `EV = void` case with empty value optimization
4. Implement `source_id()`, `target_id()`, and `value()` methods

**Code to implement**:
```cpp
#pragma once

#include <type_traits>
#include <concepts>

namespace graph::edge_list {

namespace detail {
    struct empty_value {};
}

template<typename VId, typename EV = void>
struct edge_descriptor {
    VId source_id_;
    VId target_id_;
    [[no_unique_address]] std::conditional_t<std::is_void_v<EV>, 
        detail::empty_value, EV> value_;
    
    constexpr edge_descriptor() = default;
    
    constexpr edge_descriptor(VId src, VId tgt) 
        requires std::is_void_v<EV>
        : source_id_(src), target_id_(tgt) {}
    
    constexpr edge_descriptor(VId src, VId tgt, EV val) 
        requires (!std::is_void_v<EV>)
        : source_id_(src), target_id_(tgt), value_(std::move(val)) {}
    
    [[nodiscard]] constexpr VId source_id() const noexcept { return source_id_; }
    [[nodiscard]] constexpr VId target_id() const noexcept { return target_id_; }
    
    [[nodiscard]] constexpr const EV& value() const noexcept 
        requires (!std::is_void_v<EV>) 
    { return value_; }
    
    [[nodiscard]] constexpr EV& value() noexcept 
        requires (!std::is_void_v<EV>) 
    { return value_; }
};

} // namespace graph::edge_list
```

**Commit message**: `Add edge_list::edge_descriptor type`

---

### Step 4.2: Add `is_edge_list_descriptor` Specialization

**Goal**: Specialize the trait for the actual descriptor type.

**Files to modify**:
- `include/graph/edge_list/edge_list_traits.hpp`
- `include/graph/edge_list/edge_list_descriptor.hpp`

**Tasks**:
1. Include `edge_list_traits.hpp` in `edge_list_descriptor.hpp`
2. Ensure specialization matches the actual type definition
3. Add concept for edge_list descriptor if useful

**Commit message**: `Specialize is_edge_list_descriptor for edge_descriptor type`

---

### Step 4.3: Create Tests for `edge_list::edge_descriptor`

**Goal**: Test the edge descriptor type and its integration with CPOs.

**Files to create/modify**:
- `tests/edge_list/test_edge_list_descriptor.cpp` (NEW)

**Tasks**:
1. Test construction (with and without value)
2. Test accessor methods
3. Test `is_edge_list_descriptor_v` trait
4. Test CPOs work with edge_list::edge_descriptor (Tier 5)

**Test cases**:
```cpp
TEST_CASE("edge_list::edge_descriptor construction", "[edge_list][descriptor]") {
    using namespace graph::edge_list;
    
    // Without value
    edge_descriptor<int, void> e1(1, 2);
    REQUIRE(e1.source_id() == 1);
    REQUIRE(e1.target_id() == 2);
    
    // With value
    edge_descriptor<int, double> e2(3, 4, 1.5);
    REQUIRE(e2.source_id() == 3);
    REQUIRE(e2.target_id() == 4);
    REQUIRE(e2.value() == 1.5);
}

TEST_CASE("is_edge_list_descriptor_v trait", "[edge_list][traits]") {
    using namespace graph::edge_list;
    
    static_assert(is_edge_list_descriptor_v<edge_descriptor<int, void>>);
    static_assert(is_edge_list_descriptor_v<edge_descriptor<int, double>>);
    static_assert(!is_edge_list_descriptor_v<int>);
    static_assert(!is_edge_list_descriptor_v<std::pair<int,int>>);
}

TEST_CASE("source_id CPO with edge_list::edge_descriptor", "[cpo][edge_list]") {
    using namespace graph::edge_list;
    
    edge_descriptor<int, void> e(5, 6);
    std::vector<edge_descriptor<int, void>> el{e};
    
    auto uid = graph::source_id(el, e);
    REQUIRE(uid == 5);
}
```

**Commit message**: `Add tests for edge_list::edge_descriptor`

---

## Phase 5: Update Edge List Concepts

### Step 5.1: Update `edge_list.hpp` Concepts

**Goal**: Modify concepts to use unified CPO pattern.

**Files to modify**:
- `include/graph/edge_list/edge_list.hpp`

**Tasks**:
1. Update `basic_sourced_edgelist` concept to use `graph::source_id(el, uv)`
2. Update `basic_sourced_index_edgelist` concept similarly
3. Update `has_edge_value` concept to use `graph::edge_value(el, uv)`
4. Update type aliases to work with new concept definitions

**Updated concept**:
```cpp
template <class EL>
concept basic_sourced_edgelist = 
    std::ranges::input_range<EL> &&
    !std::ranges::range<std::ranges::range_value_t<EL>> &&
    requires(EL& el, std::ranges::range_value_t<EL> uv) {
        { graph::source_id(el, uv) };
        { graph::target_id(el, uv) } -> std::same_as<decltype(graph::source_id(el, uv))>;
    };
```

**Commit message**: `Update edge_list concepts to use unified CPOs`

---

### Step 5.2: Create Tests for Updated Edge List Concepts

**Goal**: Verify concepts work with various edge types.

**Files to create/modify**:
- `tests/edge_list/test_edge_list_concepts.cpp` (NEW)

**Tasks**:
1. Test concept satisfaction with pair, tuple, edge_info
2. Test concept satisfaction with edge_list::edge_descriptor
3. Verify adjacency_list does NOT satisfy basic_sourced_edgelist

**Test cases**:
```cpp
TEST_CASE("basic_sourced_edgelist concept", "[edge_list][concepts]") {
    using namespace graph::edge_list;
    
    static_assert(basic_sourced_edgelist<std::vector<std::pair<int,int>>>);
    static_assert(basic_sourced_edgelist<std::vector<std::tuple<int,int,double>>>);
    static_assert(basic_sourced_edgelist<std::vector<graph::edge_info<int,true,void,void>>>);
    static_assert(basic_sourced_edgelist<std::vector<edge_descriptor<int,void>>>);
    
    // Should NOT satisfy (nested range = adjacency list pattern)
    static_assert(!basic_sourced_edgelist<std::vector<std::vector<int>>>);
}
```

**Commit message**: `Add tests for edge_list concepts with unified CPOs`

---

## Phase 6: Integration

### Step 6.1: Update `graph.hpp` Imports

**Goal**: Ensure all new headers are properly included.

**Files to modify**:
- `include/graph/graph.hpp`

**Tasks**:
1. Add include for `edge_list/edge_list_traits.hpp`
2. Add include for `edge_list/edge_list_descriptor.hpp`
3. Remove reference to old `edgelist.hpp` if still present
4. Export edge_list types to graph namespace if appropriate

**Commit message**: `Update graph.hpp to include edge_list headers`

---

### Step 6.2: Final Integration Tests

**Goal**: End-to-end tests verifying unified CPO behavior.

**Files to create/modify**:
- `tests/edge_list/test_edge_list_integration.cpp` (NEW)

**Tasks**:
1. Test algorithm-like function that accepts any edge range
2. Verify it works with different edge sources
3. Test mixing adj_list edges and edge_list in same compilation unit

**Test case**:
```cpp
// Generic algorithm that works with any edge source
template<typename EdgeRange>
    requires graph::edge_list::basic_sourced_edgelist<EdgeRange>
auto count_self_loops(EdgeRange&& edges) {
    int count = 0;
    for (auto&& uv : edges) {
        if (graph::source_id(edges, uv) == graph::target_id(edges, uv)) {
            ++count;
        }
    }
    return count;
}

TEST_CASE("Algorithm works with different edge sources", "[integration]") {
    // With pair
    std::vector<std::pair<int,int>> pairs{{1,2}, {3,3}, {4,4}};
    REQUIRE(count_self_loops(pairs) == 2);
    
    // With edge_info
    using EI = graph::edge_info<int, true, void, void>;
    std::vector<EI> infos{{1,2}, {5,5}};
    REQUIRE(count_self_loops(infos) == 1);
    
    // With edge_list::edge_descriptor
    using ED = graph::edge_list::edge_descriptor<int, void>;
    std::vector<ED> descs{ED{1,1}, ED{2,3}};
    REQUIRE(count_self_loops(descs) == 1);
}
```

**Commit message**: `Add integration tests for unified edge_list CPOs`

---

## Build and Run Commands

```bash
# Configure
cmake --preset linux-gcc-debug

# Build tests
cmake --build build/linux-gcc-debug --target test_edge_list_cpo test_edge_list_descriptor test_edge_list_concepts test_edge_list_integration

# Run tests
ctest --test-dir build/linux-gcc-debug -R edge_list --output-on-failure
```

---

## Completion Checklist

- [ ] All steps marked ✅ Complete
- [ ] All tests pass
- [ ] No regressions in existing tests (`ctest --test-dir build/linux-gcc-debug`)
- [ ] Code compiles with both GCC and Clang
- [ ] Update edge_list_strategy.md revision history
- [ ] Create PR to merge `feature/edge-list-unification` into `main`

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-31 | 1.0 | Initial implementation plan |
