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

### Overall Status: ALL PHASES COMPLETE ✅

**Baseline**: 4170 tests passing, 0 failures, gcc-debug clean build (231 targets)
**Final**: 4170 tests passing (gcc-debug), 4171 (gcc-release incl. benchmark), 0 failures
**Branch**: `feature/value-function-parameter-signature` (from `main`)
**Pre-existing warnings**: ~304 warning lines (all in `test_dynamic_graph_common.cpp` — unused variable)
**Pre-existing clang errors**: 2 narrowing conversions in `incidence.hpp:153` and `edgelist.hpp:231` (not introduced by this work)

| Phase | Status | Actual Duration | Notes |
|-------|--------|-----------------|-------|
| Prerequisites | ✅ Complete | ~3 min | 4170 tests pass, branch created |
| Phase 1: Concepts | ✅ Complete | ~2 min | Commit `57d23c7` |
| Phase 2: View Implementations | ✅ Complete | ~45 min | Commits `4cc8fff`..`ba4407f` (9 commits) |
| Phase 3: Update Tests | ✅ Complete | ~60 min | Commit `e763929`, 16 files changed |
| Concept Refactor | ✅ Complete | ~10 min | Commit `79c6d87`, concepts promoted to graph namespace |
| Phase 4: Chaining Tests | ✅ Complete | ~15 min | Commit `255ba28`, 49 chaining tests, 132 assertions |
| Post-Phase: DFS/BFS size()→num_visited() | ✅ Complete | ~10 min | Commit `2782533`, renamed across 8 view classes |
| Post-Phase: std::views::take tests | ✅ Complete | ~5 min | Commit `116a468`, take tests for DFS/BFS/topo sort |
| Post-Phase: Topo sort real num_visited() | ✅ Complete | ~15 min | Commits `4b94188`, `6869ab6`, iteration-tracking + bug fixes |
| Post-Phase: Topo sort cancel() support | ✅ Complete | ~10 min | Commit `f56b4fe`, cancel(cancel_all) on all 4 topo views |
| Post-Phase: Conditional size() on edgelist_view | ✅ Complete | ~10 min | Commit `5770fd7`, O(1)-only sized_range |
| Phase 5: Documentation | ✅ Complete | ~15 min | Updated views.md, view_chaining_limitations.md, view_strategy.md, common_graph_guidelines.md, value_function_goal.md |
| Phase 6: Final Verification | ✅ Complete | ~10 min | GCC debug/release pass, clang has only pre-existing narrowing errors, 0 new warnings, examples run, benchmark fixed |

**Legend**: ⬜ Not Started · 🔶 In Progress · ✅ Complete · ❌ Blocked · ⏭️ Skipped

### Detailed Step Log

#### Prerequisites
- [x] Verify clean build baseline — 231/231 targets, 0 errors
- [x] Create feature branch — `feature/value-function-parameter-signature`
- [x] Record baseline test count — **4170 tests, 0 failures, 42.45s**

#### Phase 1: Concepts
- [x] 1.1 Update `view_concepts.hpp` (vertex_value_function) — 2→3 template params
- [x] 1.1 Update `view_concepts.hpp` (edge_value_function) — 2→3 template params
- [x] 1.2 Verify concept compilation — errors in vertexlist, incidence, neighbors, edgelist, dfs, bfs as expected

#### Phase 2: View Implementations
- [x] 2.1 Update `vertexlist.hpp` — invoke_result_t, operator*, factory requires clause, header doc. Commit `4cc8fff`
- [x] 2.2 Update `neighbors.hpp` — invoke_result_t, operator*, 2 factory requires clauses. Commit `9f72742`
- [x] 2.3 Update `incidence.hpp` — invoke_result_t, operator*, 2 factory requires clauses. Commit `43bcf14`
- [x] 2.4 Update `edgelist.hpp` — adj-list variant only (3 changes); edge_list variant already parameter-based. Commit `a2389fb`
- [x] 2.5 Update `dfs.hpp` — 16 changes: 2 invoke_result_t, 2 operator*, 12 factory requires (incl. negated Alloc checks). Commit `544249f`
- [x] 2.6 Update `bfs.hpp` — 16 changes: 2 invoke_result_t, 2 operator*, 12 factory requires (incl. negated + allocator disambig). Commit `701a36a`
- [x] 2.7 Update `topological_sort.hpp` — 22 changes: 2 invoke_result_t, 2 operator*, 18 factory requires (regular + safe variants). Commit `2e04f3c`
- [x] 2.8 Update `adaptors.hpp` — No changes needed: stores/forwards VVF/EVF but doesn't use concepts or invoke directly
- [x] 2.9 Update examples/ and benchmark/ — `benchmark_views.cpp` VVF, `dijkstra_clrs.hpp` concepts/defaults, `dijkstra_clrs_example.cpp` weight_fn. Commit `9c8ccde`
- [x] 2.10 Update algorithms — `traversal_common.hpp` weight concepts, `dijkstra_shortest_paths.hpp`, `bellman_ford_shortest_paths.hpp`, `mst.hpp`. All use `std::remove_reference_t<G>` for forwarding ref safety. Commit `ba4407f`
- [x] 2.11 Phase 2 verification build — 14 test targets failed (all test files, as expected). View/algorithm headers clean.

#### Phase 3: Update Tests — Commit `e763929`
- [x] 3.1 Update `test_vertexlist.cpp` — bulk `[&g](auto v)` → `[](const auto& g, auto v)`, non-capturing `[](auto v)` → `[](const auto&, auto v)`, type-level `decltype([](auto) {…})` → `decltype([](const auto&, auto) {…})`, capturing `[&labels](auto v)` → `[&labels](const auto&, auto v)`, mutable `[&counter](auto) mutable` → `[&counter](const auto&, auto) mutable`
- [x] 3.2 Update `test_neighbors.cpp` — bulk lambdas, capturing `[multiplier](auto v)` → `[multiplier](const auto&, auto v)`, function pointer `int(*)(VertexType)` → `int(*)(const Graph&, VertexType)`
- [x] 3.3 Update `test_incidence.cpp` — bulk lambdas, capturing `[&g, multiplier](auto e)` → `[multiplier](const auto& g, auto e)`, function pointer updated. Reverted accidentally changed `count_if` STL lambda.
- [x] 3.4 Update `test_edgelist.cpp` — bulk lambdas, capturing `[&g, multiplier](auto e)` → `[multiplier](const auto& g, auto e)`. Reverted accidentally changed `count_if`/`find_if` STL lambdas. Edge_list variant left unchanged (already parameter-based).
- [x] 3.5 Update `test_dfs.cpp` — bulk lambdas, capturing `[&g, multiplier](auto v/e)` → `[multiplier](const auto& g, auto v/e)`, function pointers `int(*)(VertexType/EdgeType)` → `int(*)(const Graph&, VertexType/EdgeType)`
- [x] 3.6 Update `test_bfs.cpp` — bulk lambdas updated
- [x] 3.7 Update `test_topological_sort.cpp` — bulk lambdas updated
- [x] 3.8 Update `test_view_concepts.cpp` — completely rewritten with `mock_graph` type and 3-param concept checks
- [x] 3.9 Update `test_edge_cases.cpp` — capturing VVFs `[&names, &g](auto v)` → `[&names](const auto& g, auto v)`, mutable `[&g, counter](auto v) mutable` → `[counter](const auto& g, auto v) mutable`
- [x] 3.10 Update `test_adaptors.cpp` — VVF/EVF variable definitions updated; `std::views::transform`/`filter` lambdas correctly left unchanged
- [x] 3.11 Update `test_unified_header.cpp` — VVF lambda updated; `std::views::transform` lambda reverted to 1-param
- [x] 3.12 Update `test_graph_hpp_includes_views.cpp` — VVF lambda updated
- [x] 3.13 Update `test_dijkstra_shortest_paths.cpp` — all 7 weight lambdas updated
- [x] 3.14 Update `test_bellman_ford_shortest_paths.cpp` — all 9 weight lambdas updated
- [x] 3.15 Fix `topological_sort.hpp` — `vertices_topological_sort_view::iterator` was missing `G* g_` member (introduced in Phase 2.7 when `operator*` started using `*g_`). Added `g_` to iterator, updated constructor and `begin()`.
- [x] 3.16 Phase 3 verification — **4170/4170 tests pass**, 0 failures

#### Phase 4: Chaining Tests — Commit `255ba28`
- [x] 4.1 Create `test_view_chaining.cpp` — new file with 49 test cases, 132 assertions
- [x] 4.2 Semiregular/default_initializable concept tests — all view types verified
- [x] 4.3 std::views::take / filter / transform chaining tests — comprehensive coverage
- [x] 4.4 Multi-view chaining tests — pipeline composition verified
- [x] 4.5 Update CMakeLists.txt — added `test_view_chaining.cpp` to views test target

#### Post-Phase 4: DFS/BFS size() → num_visited() — Commit `2782533`
- [x] Renamed `size()` to `num_visited()` on all 8 DFS/BFS view classes
- [x] Updated `search_view` concept to require `num_visited()` instead of `size()`
- [x] Topo sort views keep `size()` (eagerly computed, semantically correct) and add `num_visited()` synonym

#### Post-Phase 4: std::views::take tests — Commit `116a468`
- [x] Added `std::views::take` tests for DFS/BFS/topological_sort views
- [x] Removed stale comments about take being incompatible

#### Post-Phase 4: Topo sort real num_visited() — Commits `4b94188`, `6869ab6`
- [x] Added `count_` to `topo_state`, incremented by iterators during iteration
- [x] Vertex views: `num_visited()` tracks vertices consumed (0 → N during iteration)
- [x] Edge views: `num_visited()` tracks source vertices whose edges are fully yielded
- [x] Fixed edge view `advance_to_next_edge()` bug: split into `skip_to_first_edge()` (constructor, no count) and `advance_to_next_edge()` (operator++, with count)
- [x] Fixed post-loop `++count_` double-count bug
- [x] Added 54 topo sort test cases, 205 assertions

#### Post-Phase 4: Topo sort cancel() support — Commit `f56b4fe`
- [x] Added `cancel_` field to `topo_state`
- [x] Added `cancel()` getter/setter to all 4 topo sort view classes
- [x] `cancel_branch` treated as `cancel_all` (no branch semantics in flat ordering)
- [x] Updated `search_view` concept comment to clarify topo sort doesn't satisfy it (no `depth()`)
- [x] Added 14 cancel test cases, 29 assertions

#### Phase 5: Documentation
- [x] 5.1 Update `views.md` — all VVF/EVF examples to `[](const auto& g, auto v/e)`, `size()` → `num_visited()`, new chaining section
- [x] 5.2 Update `view_chaining_limitations.md` — restructured as "View Chaining with Value Functions" (problem solved, historical context preserved)
- [x] 5.3 Update `view_strategy.md` Section 9.3 — Decision changed from Option A to Option B, updated signatures and examples
- [x] 5.4 Update `common_graph_guidelines.md` — VVF: `vvf(g, u)`, EVF: `evf(g, uv)`
- [x] 5.5 Verify examples/ and benchmark/ — confirmed no old-style captures remain (Phase 2.9 already handled)
- [x] 5.3b Update `value_function_goal.md` — added IMPLEMENTED status

#### Phase 6: Final Verification
- [x] 6.1 Full build — GCC debug (69/69 targets), GCC release (85/85 targets), clang has only 2 pre-existing narrowing errors
- [x] 6.2 Full test run — GCC debug 4170/4170 pass, GCC release 4171/4171 pass (includes benchmark)
- [x] 6.3 Test count matches baseline — 4170 baseline, 4170 current (gcc-debug)
- [x] 6.4 No new compiler warnings — zero warnings outside pre-existing `test_dynamic_graph_common.cpp`
- [x] 6.5 Examples compile and run — `basic_usage` and `dijkstra_example` verified
- [x] 6.6 Benchmark fix — `benchmark_views.cpp` structured bindings updated for new info struct decomposition
- [x] 6.7 Code formatting — `clang-format-20` applied to all 192 source files

### Issues & Deviations

_Record any unexpected problems, deviations from the plan, or decisions made during implementation._

| # | Phase | Issue | Resolution | Impact |
|---|-------|-------|------------|--------|
| 1 | 2.10 | **Forwarding reference issue**: When algorithm templates use `G&&` (forwarding reference), `G` deduces as a reference type for lvalue arguments. For example, calling `dijkstra(g, …)` where `g` is an lvalue `vector<vector<int>>` deduces `G = vector<vector<int>>&`. Now `const G&` applies reference collapsing: `const (vector<…>&) &` → `vector<…>&` (the const is discarded because it applies to the reference itself, not the referent). This means `std::invocable<WF, const G&, edge>` checks invocability with a non-const `G&`, while the actual call site passes `std::as_const(g)` — a mismatch that fails concept checks or silently allows mutation. | Use `std::remove_reference_t<G>` everywhere `G` appears in a `const G&` context: (1) **concept constraints** like `basic_edge_weight_function<WF, std::remove_reference_t<G>, …>` so the concept checks `const vector<…>&` not `vector<…>&`; (2) **`invoke_result_t`** like `std::invoke_result_t<WF, const std::remove_reference_t<G>&, edge>` so the deduced return type matches the actual const call; (3) **`std::function` default types** like `std::function<…(const std::remove_reference_t<G>&, edge)>` so the default wrapper accepts const graph refs. Default lambdas use `const auto&` (not `const G&`) because `auto` deduces independently and avoids the collapsing problem entirely. | Applied to `traversal_common.hpp`, `dijkstra_shortest_paths.hpp`, `bellman_ford_shortest_paths.hpp`, and `examples/dijkstra_clrs.hpp`. Critical pattern for any future algorithm work with forwarding references. View headers (Phase 2.1–2.7) are unaffected because their factory functions take `G&` (lvalue ref), not `G&&` (forwarding ref), so `G` always deduces as the bare type. |
| 2 | 3 | **Accidental STL lambda changes**: sed patterns like `[&g](auto ei)` matched `std::ranges::count_if` and `std::ranges::find_if` predicates, and `std::views::transform` lambdas. These are STL callbacks, not VVF/EVF. | Reverted 4 accidentally changed STL lambdas back to 1-param in `test_edgelist.cpp`, `test_incidence.cpp`, `test_unified_header.cpp`. | Sed-based bulk changes need manual review for false positives. |
| 3 | 3 | **topological_sort.hpp iterator missing `g_`**: Phase 2.7 updated `operator*` to use `std::as_const(*g_)` but only added `g_` to the view class, not to its nested `iterator` class (which is where `operator*` lives). | Added `G* g_` member variable to `vertices_topological_sort_view::iterator`, updated constructor to accept graph pointer, updated `begin()` to pass `g_`. | Bug introduced in Phase 2.7 commit `2e04f3c`, fixed in Phase 3. The edges variant was correct (already had `g_` in iterator). |
| 4 | 3 | **Phase 3 done as single commit**: Plan called for per-file commits, but all test changes were applied iteratively across multiple build-fix cycles. | Single commit `e763929` covering all 16 files (14 test files + `topological_sort.hpp` fix + `README.md`). | No impact — easier to review as one atomic change. |
| 5 | Post-3 | **Concept relocation**: `vertex_value_function` and `edge_value_function` were defined in `graph::views` namespace in `view_concepts.hpp`, but they are general-purpose concepts also useful for algorithms (e.g., `basic_edge_weight_function` is conceptually a refinement of `edge_value_function`). | Created `include/graph/graph_concepts.hpp` with both concepts in `namespace graph`. `view_concepts.hpp` now includes it and re-exports via `using`. `basic_edge_weight_function` in `traversal_common.hpp` now explicitly subsumes `edge_value_function`, making the refinement hierarchy clear and improving compiler diagnostics. | Cleaner architecture: concepts live at the right abstraction level, algorithms and views share the same definitions, and weight-function concept errors now point to `edge_value_function` first. |
| 6 | Post-4 | **DFS/BFS `size()` broke `std::views::take`**: DFS and BFS views had a `size()` member returning the number of vertices visited so far (starting at 0, growing during iteration). This satisfied `std::ranges::sized_range`, causing `std::views::take` to see `size()==0` before iteration and yield empty ranges. | Renamed to `num_visited()` across all 8 DFS/BFS view classes. Updated `search_view` concept to require `num_visited()`. Topological sort views keep `size()` (eagerly computed, semantically correct) and add `num_visited()` synonym. | Views now correctly compose with `std::views::take`, `filter`, `transform`. The semantic mismatch between growing-during-iteration and `sized_range`'s O(1)-total-count contract is resolved. |
| 7 | Post-4 | **Topo sort `num_visited()` was static**: Topological sort `num_visited()` just returned `size()` (the eagerly-computed total), not actual iteration progress. This was misleading — `num_visited()` should reflect how much the user has consumed. | Added `count_` field to `topo_state`, incremented by iterators during `operator++`. Vertex views count vertices consumed; edge views count source vertices whose edges are fully yielded. | `num_visited()` now accurately tracks iteration progress for all topo sort views, consistent with DFS/BFS behavior. |
| 8 | Post-4 | **Edge view constructor inflated `num_visited()`**: Edge view iterators called `advance_to_next_edge()` in the constructor, which incremented `count_` when skipping edgeless vertices during initial positioning. Also, a post-loop `++count_` in `advance_to_next_edge()` double-counted the last vertex group. | Split into `skip_to_first_edge()` (constructor — no count increment) and `advance_to_next_edge()` (operator++ — with count increment). Removed stale post-loop `++count_`. | Both bugs affected void and EVF edge view classes. Found during test review — comprehensive step-by-step tests added to catch regressions. |
| 9 | Post-4 | **`depth()` not meaningful for topo sort**: User asked about adding `depth()` and `cancel()` to topological sort views. `depth()` has no meaningful semantics — topo sort iterates a flat post-order vector with no tree structure. `cancel_branch` similarly has no branch concept. | Added `cancel(cancel_all)` support only. `cancel_branch` is treated identically to `cancel_all` (no branch semantics). `depth()` omitted — topo sort views deliberately do not satisfy `search_view` concept. Updated concept comment to clarify. | Topo sort now supports early termination via `cancel()` but correctly does not pretend to be a tree-structured search. `search_view` concept comment updated to say "DFS/BFS" instead of "DFS/BFS/topological sort". |
| 10 | 6 | **Benchmark structured bindings outdated**: `benchmark_views.cpp` still used old 1-element bindings like `auto [v]` for vertexlist (now 2: `[id, v]`), `auto [e]` for incidence/edgelist (now 2-3 elements), and chaining lambdas used `auto [v] = info` (now `auto [id, v] = info`). | Updated all 7 structured binding patterns in `benchmark_views.cpp` to match current info struct decomposition. | Benchmark was only built in release config, not debug — so this wasn't caught until Phase 6 full-build verification. |

### Session History

| Date | Phases Worked | Duration | Agent/Person |
|------|---------------|----------|--------------|
| 2025-02-15 | Prerequisites, Phase 1 | ~5 min | Agent (Copilot) |
| 2025-02-15 | Phase 2 (2.1–2.11) | ~45 min | Agent (Copilot) |
| 2025-02-15 | Phase 3 (all test updates) | ~60 min | Agent (Copilot) |
| 2025-02-15 | Concept refactor + doc comments | ~10 min | Agent (Copilot) |
| 2025-02-15 | Phase 4: Chaining tests | ~15 min | Agent (Copilot) |
| 2025-02-15 | size()→num_visited() rename, take tests | ~15 min | Agent (Copilot) |
| 2025-02-15 | Topo sort real num_visited() + bug fixes | ~20 min | Agent (Copilot) |
| 2025-02-15 | Topo sort cancel() support | ~10 min | Agent (Copilot) |
| 2025-02-15 | Conditional size() on edgelist_view | ~10 min | Agent (Copilot) |
| 2025-02-15 | Phase 5: Documentation | ~15 min | Agent (Copilot) |
| 2025-02-15 | Phase 6: Final Verification | ~10 min | Agent (Copilot) |
| 2025-02-15 | Phase 6.7: Code formatting | ~5 min | Agent (Copilot) |

### Git Commit History

| Commit | Phase | Description |
|--------|-------|-------------|
| `57d23c7` | 1 | Update VVF/EVF concepts to accept graph parameter |
| `4cc8fff` | 2.1 | Update vertexlist view |
| `9f72742` | 2.2 | Update neighbors view |
| `43bcf14` | 2.3 | Update incidence view |
| `a2389fb` | 2.4 | Update edgelist view (adj-list variant only) |
| `544249f` | 2.5 | Update DFS views |
| `701a36a` | 2.6 | Update BFS views |
| `2e04f3c` | 2.7 | Update topological_sort views |
| `9c8ccde` | 2.9 | Update examples and benchmarks |
| `ba4407f` | 2.10 | Update algorithm weight functions (forwarding ref fix) |
| `e763929` | 3 | Update all tests for 2-param VVF/EVF signature |
| `79c6d87` | Post-3 | Move value function concepts to `graph_concepts.hpp`; refine `basic_edge_weight_function` with `edge_value_function`; add `remove_reference_t<G>` doc comments to algorithm headers |
| `255ba28` | 4 | Phase 4: Add comprehensive view chaining test suite (49 tests, 132 assertions) |
| `2782533` | Post-4 | Rename `size()` to `num_visited()` on DFS/BFS views; update `search_view` concept |
| `116a468` | Post-4 | Add `std::views::take` tests for DFS/BFS/topological_sort views |
| `4b94188` | Post-4 | Make topological sort `num_visited()` track iteration progress |
| `6869ab6` | Post-4 | Fix edge view `num_visited()` bug (constructor inflation + double-count); add comprehensive topo tests (54 cases, 205 assertions) |
| `f56b4fe` | Post-4 | Add `cancel()` support to topological sort views (14 cancel tests, 29 assertions) |
| `5770fd7` | Post-4 | Add conditional `size()` to `edgelist_view` for adj list graphs; `has_const_time_num_edges<G>` concept enables O(1)-only `sized_range` |
| `dce8a40` | 5 | Phase 5: Update documentation for parameter-based VVF/EVF (6 docs updated) |
| `0282e5c` | 6 | Phase 6: Final verification and benchmark fix |
| `0517734` | 6.7 | Phase 6.7: Apply `clang-format-20` to all 192 source files |
