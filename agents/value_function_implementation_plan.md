# VVF/EVF Signature Change - Phased Implementation Plan

**Related Document**: [value_function_goal.md](value_function_goal.md)

## Overview

This document provides a safe, phased approach to implementing parameter-based value function signatures. The implementation is structured to minimize risk, enable incremental verification, and provide clear rollback points.

**Key Principle**: Update in small, verifiable increments with frequent testing.

## Prerequisites

Before starting implementation:

### 1. Environment Verification
```bash
# Clean build from scratch
rm -rf build/
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug

# Verify all tests pass
ctest --preset linux-gcc-debug --output-on-failure
```

**Success Criteria**: 
- Zero compilation errors
- All tests pass
- Zero warnings (if possible)

### 2. Create Feature Branch
```bash
git checkout -b feature/value-function-parameter-signature
git push -u origin feature/value-function-parameter-signature
```

### 3. Baseline Documentation
Document current state:
- Number of passing tests: `ctest --preset linux-gcc-debug | grep "tests passed"`
- Build time baseline: Record for comparison
- Any existing warnings: Document to distinguish new from old

## Implementation Phases

### Phase 1: Update Concepts [~15 minutes]

**Goal**: Update concept definitions to accept graph parameter.

**Status**: ⚠️ BREAKING - Changes concept arity, which will break view files that reference these concepts.
Breakage is expected and will be resolved in Phase 2.

#### 1.1 Update `include/graph/views/view_concepts.hpp`

**File**: `include/graph/views/view_concepts.hpp`

**Changes**:
```cpp
// BEFORE (Line ~13-15)
template <class VVF, class VertexDescriptor>
concept vertex_value_function =
    std::invocable<VVF, VertexDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<VVF, VertexDescriptor>>);

// AFTER - Add Graph parameter
template <class VVF, class Graph, class VertexDescriptor>
concept vertex_value_function =
    std::invocable<VVF, const Graph&, VertexDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<VVF, const Graph&, VertexDescriptor>>);

// BEFORE (Line ~19-21)
template <class EVF, class EdgeDescriptor>
concept edge_value_function =
    std::invocable<EVF, EdgeDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<EVF, EdgeDescriptor>>);

// AFTER - Add Graph parameter
template <class EVF, class Graph, class EdgeDescriptor>
concept edge_value_function =
    std::invocable<EVF, const Graph&, EdgeDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<EVF, const Graph&, EdgeDescriptor>>);
```

#### 1.2 Verification
```bash
# This change alone will BREAK compilation because view implementations
# use the old concept signature. This is EXPECTED.
cmake --build --preset linux-gcc-debug 2>&1 | head -50

# Expected errors: concept template parameter count mismatch in view files
```

**Result**: Compilation errors are expected. These will be fixed in Phase 2.

**Checkpoint**: Commit the concept changes alone for clarity:
```bash
git add include/graph/views/view_concepts.hpp
git commit -m "Update VVF/EVF concepts to accept graph parameter"
```

---

### Phase 2: Update View Implementations [~1-2 hours]

**Goal**: Update each view to use new concept signature and pass graph to value functions.

**Status**: ⚠️ BREAKING - Will cause compilation errors until tests are updated.

> **⚠️ Important**: The code will NOT compile between Phase 1 and the end of Phase 3.
> This is expected. Do not attempt a full test run until Phase 3 is complete.

**Strategy**: Update ONE view at a time, following this pattern for each:

1. Update concept usage in view class
2. Update value function invocations
3. Update factory functions
4. **Update deduction guides** if they have concept constraints
5. Verify compilation (will fail until tests updated)
6. Move to next view

#### 2.1 Update vertexlist.hpp

**File**: `include/graph/views/vertexlist.hpp`

**Changes needed**:

1. **Update concept checks** (~line 130):
```cpp
// BEFORE
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>

// AFTER
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
```

2. **Update value function invocation** (~line 180-185 in iterator::operator*):
```cpp
// BEFORE
return value_type{id, v, std::invoke(*vvf_, v)};

// AFTER
return value_type{id, v, std::invoke(*vvf_, std::as_const(*g_), v)};
```

3. **Update factory function** (~line 275):
```cpp
// BEFORE
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto vertexlist(G& g, VVF&& vvf)

// AFTER
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto vertexlist(G& g, VVF&& vvf)
```

4. **Check deduction guides** (~line 253-254):
```cpp
// If deduction guides have concept constraints, update them too:
template <adj_list::adjacency_list G, class VVF>
vertexlist_view(G&, VVF) -> vertexlist_view<G, VVF>;
// Deduction guides typically don't use requires clauses, but verify.
```

**Verification**:
```bash
cmake --build --preset linux-gcc-debug 2>&1 | grep -A5 "vertexlist"
# Will show test compilation errors - this is expected
```

**Checkpoint**:
```bash
git add include/graph/views/vertexlist.hpp
git commit -m "Update vertexlist view for parameter-based VVF"
```

#### 2.2 Update neighbors.hpp

**File**: `include/graph/views/neighbors.hpp`

**Pattern**: Same as vertexlist
- Update concept check (~line 126)
- Update invocation in iterator (~line 156)
- Update factory function (~line 200+)

**Checkpoint**:
```bash
git add include/graph/views/neighbors.hpp
git commit -m "Update neighbors view for parameter-based VVF"
```

#### 2.3 Update incidence.hpp

**File**: `include/graph/views/incidence.hpp`

**Pattern**: Uses EVF instead of VVF
- Update concept check (edge_value_function)
- Update invocation with graph and edge descriptor

**Checkpoint**:
```bash
git add include/graph/views/incidence.hpp
git commit -m "Update incidence view for parameter-based EVF"
```

#### 2.4 Update edgelist.hpp

**File**: `include/graph/views/edgelist.hpp`

**Pattern**: Uses EVF
- Update concept checks
- Update invocations

**Checkpoint**:
```bash
git add include/graph/views/edgelist.hpp
git commit -m "Update edgelist view for parameter-based EVF"
```

#### 2.5 Update dfs.hpp

**File**: `include/graph/views/dfs.hpp`

**Pattern**: Uses both VVF and EVF
- Update vertices_dfs_view for VVF
- Update edges_dfs_view for EVF
- Many overloads to update

**Checkpoint**:
```bash
git add include/graph/views/dfs.hpp
git commit -m "Update DFS views for parameter-based value functions"
```

#### 2.6 Update bfs.hpp

**File**: `include/graph/views/bfs.hpp`

**Pattern**: Similar to dfs.hpp

**Checkpoint**:
```bash
git add include/graph/views/bfs.hpp
git commit -m "Update BFS views for parameter-based value functions"
```

#### 2.7 Update topological_sort.hpp

**File**: `include/graph/views/topological_sort.hpp`

**Pattern**: Uses VVF

**Checkpoint**:
```bash
git add include/graph/views/topological_sort.hpp
git commit -m "Update topological_sort view for parameter-based VVF"
```

#### 2.8 Update adaptors.hpp

**File**: `include/graph/views/adaptors.hpp`

This file creates pipe adaptor closures (`vertexlist_adaptor_closure`, `incidence_adaptor_closure`)
that store and forward VVF/EVF to views. Check for:
- Template parameters or concept constraints that reference `vertex_value_function` or `edge_value_function`
- Factory function signatures in `vertexlist_adaptor_fn`, `incidence_adaptor_fn`, etc.
- The adaptor closures themselves don't invoke VVF directly (they forward to views), but their
  template parameter constraints may need updating.

**Checkpoint**:
```bash
git add include/graph/views/adaptors.hpp
git commit -m "Update view adaptors for parameter-based value functions"
```

#### 2.9 Check examples/ and benchmark/ for value function lambdas

Before proceeding to tests, check whether examples or benchmarks also use value functions.
If so, they must be updated in this phase to avoid compilation failures in Phase 6.

```bash
grep -rn "\[&g\](auto" examples/ benchmark/
grep -rn "\[&" examples/ benchmark/ | grep -E "vvf|evf|value"
```

Update any matches to use the new `[](const auto& g, ...)` signature.

**Checkpoint** (if changes made):
```bash
git add examples/ benchmark/
git commit -m "Update examples and benchmarks for parameter-based value functions"
```

#### 2.10 Investigate algorithms for VVF/EVF patterns

Check whether any algorithm files use VVF/EVF-style patterns:

```bash
grep -rn "VVF\|EVF\|vertex_value_function\|edge_value_function" include/graph/algorithm/
grep -rn "\[&g\](auto" include/graph/algorithm/
```

Algorithms currently use **edge weight functions** with signature `weight(uv)`, which is a
different pattern (defined by `basic_edge_weight_function` / `edge_weight_function` concepts
in `traversal_common.hpp`). These do NOT currently use VVF/EVF concepts.

**Decision**:
- If algorithms reference `vertex_value_function` or `edge_value_function` → update them
- If algorithms only use `edge_weight_function` → no change needed now
- Consider a follow-up task for consistency if weight functions share similar capture issues

**Document findings** in commit message:
```bash
git commit --allow-empty -m "Algorithms: [no changes needed / updated X files] for VVF/EVF"
```

#### 2.11 Phase 2 Verification

After all views updated:
```bash
# Try to build - should fail with test errors
cmake --build --preset linux-gcc-debug 2>&1 | tee build_errors.log

# Count errors by test file
grep "error:" build_errors.log | grep "test_" | cut -d: -f1 | sort | uniq -c

# This gives you a list of which test files need updating
```

**Expected Result**: Compilation fails in test files due to lambda signature mismatch.
Errors in `examples/` and `benchmark/` should already be resolved (step 2.9).

---

### Phase 3: Update Tests [~1-2 hours]

**Goal**: Fix all test compilation errors by updating lambda signatures.

**Status**: ✅ FIXES COMPILATION - Each test file fixed brings code back to working state.

**Strategy**: Update tests in same order as views were updated.

#### 3.1 Update test_vertexlist.cpp

**File**: `tests/views/test_vertexlist.cpp`

**Pattern**: Change all lambdas from `[&g](auto v)` to `[](const auto& g, auto v)`

**Script to find lambdas**:
```bash
grep -n "\[&g\](auto v)" tests/views/test_vertexlist.cpp
# Shows all lines that need updating
```

**Example changes**:
```cpp
// BEFORE (~line 245)
auto vvf = [&g](auto v) { 
    return g[v.vertex_id()].front();
};

// AFTER
auto vvf = [](const auto& g, auto v) { 
    return g[v.vertex_id()].front();
};
```

**Count**: ~7 lambda updates in this file.

**Verification**:
```bash
# Build just the vertexlist tests
cmake --build --preset linux-gcc-debug --target test_vertexlist

# Run the tests
./build/linux-gcc-debug/tests/views/test_vertexlist
```

**Success Criteria**: All vertexlist tests compile and pass.

**Checkpoint**:
```bash
git add tests/views/test_vertexlist.cpp
git commit -m "Update test_vertexlist for parameter-based VVF"
```

#### 3.2 Update test_neighbors.cpp

**File**: `tests/views/test_neighbors.cpp`

**Pattern**: Same lambda signature changes

**Verification**:
```bash
cmake --build --preset linux-gcc-debug --target test_neighbors
./build/linux-gcc-debug/tests/views/test_neighbors
```

**Checkpoint**:
```bash
git add tests/views/test_neighbors.cpp
git commit -m "Update test_neighbors for parameter-based VVF"
```

#### 3.3 Update test_incidence.cpp

**File**: `tests/views/test_incidence.cpp`

**Pattern**: Change lambdas for EVF: `[&g](auto e)` to `[](const auto& g, auto e)`

**Checkpoint**:
```bash
git add tests/views/test_incidence.cpp
git commit -m "Update test_incidence for parameter-based EVF"
```

#### 3.4 Update test_edgelist.cpp

**File**: `tests/views/test_edgelist.cpp`

**Pattern**: EVF signature changes

**⚠️ CRITICAL: Two distinct EVF patterns exist in this file!**

Edgelist tests cover TWO different graph types with different existing signatures:

1. **Adjacency list edgelists** (e.g., `vector<vector<int>>`): 
   - Current: `[&g](auto e) { return edge_value(g, e); }` → captures graph
   - New: `[](const auto& g, auto e) { return edge_value(g, e); }` → parameter

2. **Edge_list edgelists** (e.g., `vector<tuple<int,int,double>>`):
   - Current: `[](auto& el, auto e) { return edge_value(el, e); }` → **already** takes container as parameter!
   - These may NOT need changes, or may already match the new pattern.
   - **Do NOT blindly add a graph parameter** — this would create a double-parameter `(el, el, e)` bug.

**How to distinguish**: Search for the graph type in each test section:
```bash
grep -B10 "edgelist(" tests/views/test_edgelist.cpp | grep -E "EdgeList|vector<tuple"
```

Only update lambdas where the graph/container is **captured** (`[&g]`, `[&el]`), not where
it's already a parameter.

**Checkpoint**:
```bash
git add tests/views/test_edgelist.cpp
git commit -m "Update test_edgelist for parameter-based EVF"
```

#### 3.5 Update test_dfs.cpp

**File**: `tests/views/test_dfs.cpp`

**Count**: ~25 lambda updates (largest file)

**Verification**:
```bash
cmake --build --preset linux-gcc-debug --target test_dfs
./build/linux-gcc-debug/tests/views/test_dfs
```

**Checkpoint**:
```bash
git add tests/views/test_dfs.cpp
git commit -m "Update test_dfs for parameter-based value functions"
```

#### 3.6 Update test_bfs.cpp

**File**: `tests/views/test_bfs.cpp`

**Pattern**: Similar to test_dfs.cpp

**Checkpoint**:
```bash
git add tests/views/test_bfs.cpp
git commit -m "Update test_bfs for parameter-based value functions"
```

#### 3.7 Update test_topological_sort.cpp

**File**: `tests/views/test_topological_sort.cpp`

**Checkpoint**:
```bash
git add tests/views/test_topological_sort.cpp
git commit -m "Update test_topological_sort for parameter-based VVF"
```

#### 3.8 Update test_view_concepts.cpp

**File**: `tests/views/test_view_concepts.cpp`

**Special attention**: TWO things must be updated in this file:

**1. Update the mock lambda DEFINITIONS at file scope** (~lines 16-21):
```cpp
// BEFORE - file-scope mock lambdas take only descriptor
auto valid_vertex_value_fn = [](mock_vertex_descriptor) { return 42; };
auto valid_edge_value_fn   = [](mock_edge_descriptor) { return 3.14; };
auto void_vertex_fn = [](mock_vertex_descriptor) {};
auto void_edge_fn   = [](mock_edge_descriptor) {};

// AFTER - mock lambdas must now accept graph as first parameter
using mock_graph = std::vector<std::vector<int>>;
auto valid_vertex_value_fn = [](const auto&, mock_vertex_descriptor) { return 42; };
auto valid_edge_value_fn   = [](const auto&, mock_edge_descriptor) { return 3.14; };
auto void_vertex_fn = [](const auto&, mock_vertex_descriptor) {};
auto void_edge_fn   = [](const auto&, mock_edge_descriptor) {};
```

Without updating these definitions, the concept checks will fail because the lambdas
won't be invocable with `(Graph&, Descriptor)` — they only accept `(Descriptor)`.

**2. Update the STATIC_REQUIRE checks** to include graph type:
```cpp
// BEFORE
STATIC_REQUIRE(vertex_value_function<decltype(valid_vertex_value_fn),
                                      mock_vertex_descriptor>);

// AFTER - Add mock graph type
STATIC_REQUIRE(vertex_value_function<decltype(valid_vertex_value_fn),
                                      mock_graph,
                                      mock_vertex_descriptor>);
```

**Also update**: All other lambdas defined inside test sections (mutable lambdas,
capturing lambdas, generic lambdas, etc.) to accept the graph parameter.

**Checkpoint**:
```bash
git add tests/views/test_view_concepts.cpp
git commit -m "Update test_view_concepts for parameter-based concepts"
```

#### 3.9 Update test_edge_cases.cpp

**File**: `tests/views/test_edge_cases.cpp`

**Checkpoint**:
```bash
git add tests/views/test_edge_cases.cpp
git commit -m "Update test_edge_cases for parameter-based value functions"
```

#### 3.10 Phase 3 Verification

**Full rebuild and test**:
```bash
# Clean build
cmake --build --preset linux-gcc-debug --clean-first

# Run all tests
ctest --preset linux-gcc-debug --output-on-failure

# Should have same number of passing tests as baseline
```

**Success Criteria**:
- ✅ Zero compilation errors
- ✅ All existing tests pass
- ✅ Same test count as baseline
- ✅ No new warnings introduced

**Major Checkpoint**:
```bash
git push origin feature/value-function-parameter-signature
# Create draft PR for review at this point
```

---

### Phase 4: Add View Chaining Tests [~2-3 hours]

**Goal**: Demonstrate that view chaining now works - the whole point of this change!

**Status**: ✅ NEW FUNCTIONALITY - Proves the benefit.

**Strategy**: Add comprehensive chaining tests to demonstrate success.

**Recommended approach**: Option B (dedicated test file) — provides a central location for
all chaining validation and avoids bloating individual view test files.

#### 4.1 Option A: Add to Existing Test Files (Alternative)

Add new test sections to each existing test file.

**Example for test_vertexlist.cpp**:

```cpp
// Add at end of file, before closing
// =============================================================================
// View Chaining Tests - NEW FUNCTIONALITY
// =============================================================================

TEST_CASE("vertexlist - chains with std::views::take", "[vertexlist][chaining]") {
    std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};
    
    auto vvf = [](const auto& g, auto v) { 
        return vertex_id(g, v) * 10; 
    };
    
    // This now works! Previously would fail to compile.
    auto view = g | vertexlist(vvf) 
                  | std::views::take(2);
    
    std::vector<size_t> values;
    for (auto [vid, v, val] : view) {
        values.push_back(val);
    }
    
    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 0);
    REQUIRE(values[1] == 10);
}

TEST_CASE("vertexlist - chains with std::views::filter", "[vertexlist][chaining]") {
    std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};
    
    auto vvf = [](const auto& g, auto v) { 
        return static_cast<int>(vertex_id(g, v)); 
    };
    
    auto view = g | vertexlist(vvf) 
                  | std::views::filter([](auto vi) {
                      auto [vid, v, val] = vi;
                      return val > 0;
                    });
    
    size_t count = 0;
    for (auto [vid, v, val] : view) {
        REQUIRE(val > 0);
        count++;
    }
    
    REQUIRE(count == 2);
}

TEST_CASE("vertexlist - chains with std::views::transform", "[vertexlist][chaining]") {
    std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};
    
    auto vvf = [](const auto& g, auto v) { 
        return static_cast<int>(vertex_id(g, v)); 
    };
    
    auto view = g | vertexlist(vvf) 
                  | std::views::transform([](auto vi) {
                      auto [vid, v, val] = vi;
                      return val * 100;
                    });
    
    std::vector<int> transformed;
    for (auto val : view) {
        transformed.push_back(val);
    }
    
    REQUIRE(transformed == std::vector<int>{0, 100, 200});
}

TEST_CASE("vertexlist - multiple chained operations", "[vertexlist][chaining]") {
    std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};
    
    auto vvf = [](const auto& g, auto v) { 
        return static_cast<int>(vertex_id(g, v)); 
    };
    
    auto view = g | vertexlist(vvf) 
                  | std::views::filter([](auto vi) {
                      auto [vid, v, val] = vi;
                      return val >= 1;
                    })
                  | std::views::transform([](auto vi) {
                      auto [vid, v, val] = vi;
                      return val * 10;
                    })
                  | std::views::take(1);
    
    std::vector<int> result;
    for (auto val : view) {
        result.push_back(val);
    }
    
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 10);
}

TEST_CASE("vertexlist - verify view is semiregular", "[vertexlist][chaining][concepts]") {
    std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};
    
    auto vvf = [](const auto& g, auto v) { 
        return vertex_id(g, v); 
    };
    
    using ViewType = decltype(g | vertexlist(vvf));
    
    // These are the requirements for std::views chaining
    STATIC_REQUIRE(std::ranges::range<ViewType>);
    STATIC_REQUIRE(std::ranges::view<ViewType>);
    STATIC_REQUIRE(std::semiregular<ViewType>);
    STATIC_REQUIRE(std::default_initializable<ViewType>);
    STATIC_REQUIRE(std::copyable<ViewType>);
}
```

**Apply similar pattern to**:
- test_neighbors.cpp
- test_incidence.cpp  
- test_edgelist.cpp
- test_dfs.cpp
- test_bfs.cpp
- test_topological_sort.cpp

**Checkpoint per file**:
```bash
git add tests/views/test_vertexlist.cpp
git commit -m "Add view chaining tests to test_vertexlist"
```

#### 4.2 Option B: Create Dedicated test_view_chaining.cpp (RECOMMENDED)

**File**: `tests/views/test_view_chaining.cpp` (new file)

Create comprehensive chaining test file with all view types:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <graph/views.hpp>
#include <ranges>
#include <vector>
#include <concepts>

using namespace graph;
using namespace graph::views;

// =============================================================================
// Vertexlist Chaining
// =============================================================================

TEST_CASE("chaining - vertexlist with std::views", "[chaining][vertexlist]") {
    // Tests here...
}

// =============================================================================
// Neighbors Chaining
// =============================================================================

TEST_CASE("chaining - neighbors with std::views", "[chaining][neighbors]") {
    // Tests here...
}

// ... etc for all views
```

**Add to** `tests/views/CMakeLists.txt`:
```cmake
add_executable(test_view_chaining test_view_chaining.cpp)
target_link_libraries(test_view_chaining PRIVATE graph Catch2::Catch2WithMain)
add_test(NAME test_view_chaining COMMAND test_view_chaining)
```

**Checkpoint**:
```bash
git add tests/views/test_view_chaining.cpp tests/views/CMakeLists.txt
git commit -m "Add comprehensive view chaining test suite"
```

#### 4.3 Phase 4 Verification

```bash
# Build and run new tests
cmake --build --preset linux-gcc-debug

# Run all view tests including new chaining tests
ctest --preset linux-gcc-debug -R "views|chaining" --output-on-failure

# Specifically run chaining tests
ctest --preset linux-gcc-debug -R "chaining" --output-on-failure
```

**Success Criteria**:
- ✅ All new chaining tests pass
- ✅ Views chain correctly with std::views::take, filter, transform
- ✅ Multiple chained operations work
- ✅ Static assertions confirm views are semiregular

---

### Phase 5: Update Documentation [~30-45 minutes]

**Goal**: Update documentation to reflect new signatures and capabilities.

**Status**: ✅ SAFE - Documentation only, no code changes.

#### 5.1 Update docs/views.md

**File**: `docs/views.md`

**Changes**:
- Update all VVF/EVF examples to use parameter-based signatures
- Add note about view chaining support
- Update "Value Functions" section (~line 399)

**Example**:
```markdown
## Value Functions

Value functions receive the graph and descriptor as parameters:

**Vertex Value Function**:
```cpp
auto vvf = [](const auto& g, auto v) {
    return vertex_id(g, v) * 2;
};
for (auto [v, val] : g | vertexlist(vvf)) {
    // val = vertex_id * 2
}
```

**Edge Value Function**:
```cpp
auto evf = [](const auto& g, auto e) {
    return target_id(g, e);
};
for (auto [e, target] : g | incidence(0, evf)) {
    // target = target vertex ID
}
```

**View Chaining Support** (New in v3):
```cpp
// Value functions now enable full std::views chaining!
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
auto view = g | vertexlist(vvf)
              | std::views::filter([](auto vi) { 
                  auto [vid, v, val] = vi;
                  return val > 0; 
                })
              | std::views::take(10);
```
```

**Checkpoint**:
```bash
git add docs/views.md
git commit -m "Update views.md for parameter-based value functions"
```

#### 5.2 Update docs/view_chaining_limitations.md

**File**: `docs/view_chaining_limitations.md`

**Major changes**:
- Change title to "View Chaining with Value Functions"
- Update opening to note the problem is SOLVED
- Keep historical context
- Add success examples

**New introduction**:
```markdown
# View Chaining with Value Functions

## Overview

As of v3, graph views support **full C++20 range chaining with `std::views`**, including
when using value functions. This document explains the design decision that enabled this
capability.

## The Solution: Parameter-Based Value Functions

Value functions now receive the graph as a parameter instead of requiring capture:

```cpp
// Current approach - graph is a parameter
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };

// This now works perfectly!
auto view = g | vertexlist(vvf) 
              | std::views::take(2);  // ✅ Full chaining support!
```

## Historical Context: Why This Changed

[Keep the old "The Problem" section as historical context]
...
```

**Checkpoint**:
```bash
git add docs/view_chaining_limitations.md
git commit -m "Update view_chaining_limitations to document solution"
```

#### 5.3 Update agents/view_strategy.md

**File**: `agents/view_strategy.md`

**Changes**: Update Section 9.3 (~line 1120)

```markdown
### 9.3 Value Function Design

**Decision**: Value functions receive graph as first parameter: `vvf(g, v)` and `evf(g, e)`

**Rationale**:
1. **Enables view chaining**: Lambdas don't need to capture, making them stateless and semiregular
2. **More explicit**: Clear that the function needs graph access
3. **More flexible**: Function can use any graph operation without pre-capturing
4. **Works in C++20**: No need to wait for C++26's `std::copyable_function`

**Usage**:
```cpp
// Value functions receive graph and descriptor
for (auto&& [v, val] : vertexlist(g, [](const auto& g, auto vdesc) { 
    return vertex_value(g, vdesc).name;
})) {
    // val is the extracted name
}

// Enables full std::views chaining
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
auto view = g | vertexlist(vvf) | std::views::take(10);  // Works!
```

**Alternative Considered**: Capturing lambdas `[&g](auto v) { ... }`
- **Rejected**: Makes lambdas non-semiregular, breaking std::views chaining
- See [view_chaining_limitations.md](../docs/view_chaining_limitations.md) for details
```

**Checkpoint**:
```bash
git add agents/view_strategy.md
git commit -m "Update view_strategy.md with parameter-based decision"
```

#### 5.4 Update docs/common_graph_guidelines.md

**File**: `docs/common_graph_guidelines.md`

This file defines naming conventions for VVF and EVF. Currently says:
- `VVF`: `vvf(u)` → vertex value
- `EVF`: `evf(uv)` → edge value

Update to:
- `VVF`: `vvf(g, u)` → vertex value (graph passed as first parameter)
- `EVF`: `evf(g, uv)` → edge value (graph passed as first parameter)

**Checkpoint**:
```bash
git add docs/common_graph_guidelines.md
git commit -m "Update naming conventions for parameter-based VVF/EVF"
```

#### 5.5 Update Examples (if any exist)

**Note**: Examples and benchmarks should already be updated in Phase 2.9. Verify here:

```bash
grep -r "\[&g\](auto" examples/ benchmark/
# Should return no matches if Phase 2.9 was completed
```

If any remain, update them now.

**Checkpoint** (if changes made):
```bash
git add examples/ benchmark/
git commit -m "Update remaining examples for parameter-based value functions"
```

---

### Phase 6: Final Verification & Cleanup [~30 minutes]

**Goal**: Comprehensive verification that everything works correctly.

#### 6.1 Full Clean Build - All Configurations

```bash
# GCC Debug
cmake --build --preset linux-gcc-debug --clean-first
ctest --preset linux-gcc-debug --output-on-failure

# GCC Release
cmake --build --preset linux-gcc-release --clean-first
ctest --preset linux-gcc-release --output-on-failure

# Clang Debug
cmake --build --preset linux-clang-debug --clean-first
ctest --preset linux-clang-debug --output-on-failure

# Clang Release
cmake --build --preset linux-clang-release --clean-first
ctest --preset linux-clang-release --output-on-failure
```

**Success Criteria**: All configurations build and pass tests.

#### 6.2 Run Specific Test Categories

```bash
# All view tests
ctest --preset linux-gcc-debug -R "views" --output-on-failure

# All chaining tests
ctest --preset linux-gcc-debug -R "chaining" --output-on-failure

# Concept tests
ctest --preset linux-gcc-debug -R "concept" --output-on-failure
```

#### 6.3 Verify Examples Compile

```bash
cmake --build --preset linux-gcc-debug --target examples
./build/linux-gcc-debug/examples/basic_usage
./build/linux-gcc-debug/examples/dijkstra_clrs_example
./build/linux-gcc-debug/examples/mst_usage_example
```

#### 6.4 Check for Warnings

```bash
cmake --build --preset linux-gcc-debug 2>&1 | grep -i "warning"
# Should be no new warnings, or address them
```

#### 6.5 Performance Baseline (Optional but Recommended)

```bash
# Run benchmarks if they exist
cmake --build --preset linux-gcc-release --target benchmark
./build/linux-gcc-release/benchmark/benchmark_views

# Compare to baseline from prerequisites
# Ensure no significant performance regression
```

#### 6.6 Documentation Verification

Verify all documentation examples compile:

```bash
# Extract code blocks from markdown and try to compile them
# (if you have a script for this)
./scripts/verify_documentation_examples.sh
```

#### 6.7 Final Checklist

- [ ] All configurations (gcc/clang, debug/release) build successfully
- [ ] All existing tests pass (same count as baseline)
- [ ] All new chaining tests pass
- [ ] No new compilation warnings
- [ ] Examples compile and run
- [ ] Documentation is consistent
- [ ] No performance regression
- [ ] Code is formatted correctly (`./scripts/format.sh`)

#### 6.8 Final Commit

```bash
# Format all code
./scripts/format.sh

# Stage any formatting changes
git add -u

# Final commit if formatting changed anything
git commit -m "Apply code formatting"

# Push feature branch
git push origin feature/value-function-parameter-signature
```

---

## Rollback Strategy

If issues arise at any phase:

### Emergency Rollback
```bash
# Quick rollback to last known good state
git reset --hard HEAD~N  # where N is number of commits to undo
git push -f origin feature/value-function-parameter-signature
```

### Phase-Specific Rollback

**If Phase 1 (Concepts) causes issues**:
- Revert that single commit
- Phase 1 changes concept arity which breaks downstream code — this is expected and resolved in Phase 2

**If Phase 2 (Views) causes unfixable errors**:
- Revert all Phase 2 commits
- Views and concepts revert together

**If Phase 3 (Tests) has issues**:
- Fix forward: Test updates are straightforward mechanical changes
- Use search/replace to fix batch issues

**If Phase 4 (Chaining Tests) has failures**:
- Chaining tests are additive, can be fixed after merge if needed
- Failures here indicate the feature doesn't work - investigate root cause

---

## Common Issues & Solutions

### Issue: Concept Template Parameter Count Mismatch

**Symptom**: 
```
error: wrong number of template arguments for concept 'vertex_value_function'
```

**Solution**: Update all concept usages to include Graph parameter:
```cpp
// Old: vertex_value_function<VVF, VertexDesc>
// New: vertex_value_function<VVF, G, VertexDesc>
```

### Issue: Lambda Capture Still Present

**Symptom**: View chaining tests still fail to compile

**Solution**: Ensure lambda has empty capture list `[]`, not `[&g]`

### Issue: Compilation Extremely Slow

**Symptom**: Build takes much longer than baseline

**Solution**: 
- Use parallel builds: `cmake --build --preset linux-gcc-debug -j8`
- May be normal for views due to template instantiation

### Issue: Tests Fail Due to Graph Mutability

**Symptom**: 
```
error: binding reference of type 'auto&' to 'const auto' discards qualifiers
```

**Solution**: Use `const auto& g` in lambda signature, not `auto& g`

### Issue: Edgelist Tests Fail With Wrong Number of Arguments

**Symptom**:
```
error: no matching function for call to object of type '(lambda)'
note: candidate function not viable: requires 3 arguments, but 2 were provided
```

**Cause**: Edge_list-based edgelist tests already use a two-parameter form `(el, e)`.
Blindly adding a graph parameter creates `(el, el, e)` — a double-application bug.

**Solution**: Only update lambdas where the graph/container is **captured** (`[&g]`, `[&el]`).
Leave lambdas that already take the container as a parameter unchanged. See Phase 3.4 for details.

### Issue: Mock Lambdas in test_view_concepts.cpp Don't Match New Concepts

**Symptom**:
```
static assertion failed: vertex_value_function<...>
```

**Cause**: File-scope mock lambdas still have old `(descriptor)` signature instead of `(graph, descriptor)`.

**Solution**: Update both the lambda definitions AND the STATIC_REQUIRE checks. See Phase 3.8 for details.

---

## Success Metrics

### Quantitative
- ✅ Zero compilation errors
- ✅ 100% existing tests pass
- ✅ 50-70 new chaining tests pass
- ✅ All 4 build configurations succeed
- ✅ Performance within 5% of baseline

### Qualitative
- ✅ Views chain naturally with std::views
- ✅ Code is cleaner (no forced captures)
- ✅ Documentation clearly explains the benefit
- ✅ Examples demonstrate chaining capability

---

## Post-Implementation

### Merge Strategy
```bash
# Ensure feature branch is up to date
git checkout main
git pull origin main
git checkout feature/value-function-parameter-signature
git rebase main

# Final test
ctest --preset linux-gcc-debug --output-on-failure

# Push and create PR
git push -f origin feature/value-function-parameter-signature
# Create PR on GitHub
```

### PR Description Template
```markdown
## VVF/EVF Signature Change to Enable View Chaining

### Summary
Changes value function signatures from `vvf(v)` to `vvf(g, v)` to enable
view chaining with `std::views` by making lambdas stateless.

### Breaking Change
⚠️ This is a breaking change. All code using value functions must update
lambda signatures from `[&g](auto v)` to `[](const auto& g, auto v)`.

### Benefits
- ✅ Full std::views chaining support (take, filter, transform, etc.)
- ✅ Stateless lambdas (semiregular views)
- ✅ More explicit graph access
- ✅ Works in C++20 (no C++26 required)

### Testing
- Updated ~80-100 lambda signatures in existing tests
- Added 50-70 new view chaining tests
- All tests pass in all configurations

### Related
- Closes #XXX
- See agents/value_function_goal.md for rationale
- See agents/value_function_implementation_plan.md for details
```

### Follow-up Tasks
- [ ] Update project README if it mentions value functions
- [ ] Create migration guide for external users (if applicable)
- [ ] Update any external documentation/wiki
- [ ] Celebrate successful implementation! 🎉

---

## Time Estimates

| Phase | Duration | Difficulty |
|-------|----------|------------|
| Prerequisites | 15 min | Easy |
| Phase 1: Concepts | 15 min | Easy |
| Phase 2: View Implementations | 1-2 hours | Medium |
| Phase 3: Update Tests | 1-2 hours | Easy but tedious |
| Phase 4: Chaining Tests | 2-3 hours | Medium |
| Phase 5: Documentation | 30-45 min | Easy |
| Phase 6: Final Verification | 30 min | Easy |
| **Total** | **6-9 hours** | **Medium** |

**Note**: Times assume familiarity with codebase. First-time implementation may take longer.

---

## Agent Execution Notes

For automated agent execution:

1. **Follow phases strictly in order** - dependencies matter
2. **Verify after each phase** - don't proceed if verification fails
3. **Commit frequently** - one commit per major change for rollback capability
4. **Run tests incrementally** - catch issues early
5. **Use search/replace carefully** - mechanical changes are error-prone
6. **Ask for clarification** if encountering unexpected errors
7. **Document deviations** if the plan needs adjustment

Good luck! 🚀

---

## Progress Summary

> **Instructions**: Update this section after completing each step. Mark status, record
> actual duration, and note any deviations or issues encountered.

### Overall Status: NOT STARTED

| Phase | Status | Actual Duration | Notes |
|-------|--------|-----------------|-------|
| Prerequisites | ⬜ Not Started | — | |
| Phase 1: Concepts | ⬜ Not Started | — | |
| Phase 2: View Implementations | ⬜ Not Started | — | |
| Phase 3: Update Tests | ⬜ Not Started | — | |
| Phase 4: Chaining Tests | ⬜ Not Started | — | |
| Phase 5: Documentation | ⬜ Not Started | — | |
| Phase 6: Final Verification | ⬜ Not Started | — | |

**Legend**: ⬜ Not Started · 🔶 In Progress · ✅ Complete · ❌ Blocked · ⏭️ Skipped

### Detailed Step Log

#### Prerequisites
- [ ] Verify clean build baseline
- [ ] Create feature branch
- [ ] Record baseline test count

#### Phase 1: Concepts
- [ ] 1.1 Update `view_concepts.hpp` (vertex_value_function)
- [ ] 1.1 Update `view_concepts.hpp` (edge_value_function)
- [ ] 1.2 Verify concept compilation

#### Phase 2: View Implementations
- [ ] 2.1 Update `vertexlist.hpp`
- [ ] 2.2 Update `neighbors.hpp`
- [ ] 2.3 Update `incidence.hpp`
- [ ] 2.4 Update `edgelist.hpp`
- [ ] 2.5 Update `dfs.hpp`
- [ ] 2.6 Update `bfs.hpp`
- [ ] 2.7 Update `topological_sort.hpp`
- [ ] 2.8 Update `adaptors.hpp`
- [ ] 2.9 Check examples/ and benchmark/
- [ ] 2.10 Investigate algorithms for VVF/EVF patterns
- [ ] 2.11 Phase 2 verification build

#### Phase 3: Update Tests
- [ ] 3.1 Update `test_vertexlist.cpp` (~7 lambdas)
- [ ] 3.2 Update `test_neighbors.cpp` (~14 lambdas)
- [ ] 3.3 Update `test_incidence.cpp` (~25 lambdas)
- [ ] 3.4 Update `test_edgelist.cpp` (~24 lambdas, watch for 2-param edge_list pattern)
- [ ] 3.5 Update `test_dfs.cpp` (~10 lambdas)
- [ ] 3.6 Update `test_bfs.cpp` (~4 lambdas)
- [ ] 3.7 Update `test_topological_sort.cpp` (~9 lambdas)
- [ ] 3.8 Update `test_view_concepts.cpp` (mock definitions + STATIC_REQUIRE)
- [ ] 3.9 Phase 3 verification (full test run)

#### Phase 4: Chaining Tests
- [ ] 4.1 Create `test_view_chaining.cpp` (or add to existing files)
- [ ] 4.2 Semiregular/default_initializable concept tests
- [ ] 4.3 std::views::take / filter / transform chaining tests
- [ ] 4.4 Multi-view chaining tests
- [ ] 4.5 Update CMakeLists.txt if new file created

#### Phase 5: Documentation
- [ ] 5.1 Update `view_chaining_limitations.md`
- [ ] 5.2 Update `view_strategy.md` Section 9.3
- [ ] 5.3 Update `value_function_goal.md` status
- [ ] 5.4 Update `common_graph_guidelines.md`
- [ ] 5.5 Verify examples/ and benchmark/ (should already be done)

#### Phase 6: Final Verification
- [ ] 6.1 Full build (all presets)
- [ ] 6.2 Full test run (all presets)
- [ ] 6.3 Test count matches baseline
- [ ] 6.4 No compiler warnings introduced
- [ ] 6.5 Squash/clean commit history

### Issues & Deviations

_Record any unexpected problems, deviations from the plan, or decisions made during implementation._

| # | Phase | Issue | Resolution | Impact |
|---|-------|-------|------------|--------|
| | | | | |

### Session History

| Date | Phases Worked | Duration | Agent/Person |
|------|---------------|----------|--------------|
| | | | |
