# Dijkstra Shortest Paths Refactoring Plan

## Overview
Refactor `dijkstra_shortest_paths.hpp` to use the CPO (Customization Point Object) architecture consistently, aligning it with the current graph-v3 library design and the recently completed `dijkstra.hpp`.

## Current State Analysis

### Issues in Current Implementation
1. **Redundant Include**: Includes `graph/views/incidence.hpp` which is already included by `graph.hpp`
2. **Old Using Declarations**: Uses `using adj_list::index_adjacency_list` and similar declarations that are unnecessary
3. **Invalid Type Alias**: Uses `edge_reference_t<G>` which doesn't exist in graph-v3 (should be `edge_t<G>`)
4. **Missing CPO Using Declarations**: Doesn't explicitly bring CPOs into scope for clarity

### What Works Currently
- The algorithm logic itself is sound
- `index_adjacency_list` concept is correct (requires integral vertex IDs for indexing)
- Visitor pattern implementation is good
- Multi-source and single-source variants work correctly
- `_null_range_type` and `_null_predecessors` are defined in `common_shortest_paths.hpp`
- CPO calls (vertices, find_vertex, target_id, etc.) work via ADL
- The pattern of checking `is_same_v<Predecessors, _null_range_type>` for optional predecessor tracking

## Refactoring Strategy

### 1. Header and Includes
**Current:**
```cpp
#include "graph/graph.hpp"
#include "graph/views/incidence.hpp"
#include "graph/algorithm/common_shortest_paths.hpp"
```

**Change to:**
```cpp
#include "graph/graph.hpp"
#include "graph/algorithm/common_shortest_paths.hpp"
```

**Action:** 
- Remove `#include "graph/views/incidence.hpp"` (already included by graph.hpp)
- Remove the old `using` declarations:
```cpp
// Remove these:
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::edge_reference_t;  // This doesn't exist in graph-v3!
```

### 2. Concept Updates
**Current:**
```cpp
template <index_adjacency_list G, ...>
```

**Keep as-is** - `index_adjacency_list` is correct.

**Rationale:** The algorithm uses integral vertex IDs for indexing into `distances` and `predecessors` arrays (e.g., `distances[static_cast<size_t>(uid)]`). The `index_adjacency_list` concept ensures vertex IDs are integral types suitable for array indexing. The more general `adjacency_list` concept doesn't guarantee this property.

### 3. Type Trait Updates

**Current (INCORRECT):**
```cpp
vertex_id_t<G>
edge_reference_t<G>  // DOES NOT EXIST in graph-v3
```

**Change to:**
```cpp
vertex_id_t<G>  → Keep (defined in adjacency_list_traits.hpp)
edge_t<G>       → Use this instead of edge_reference_t<G>
```

**Note:** Graph-v3 is value-based, not reference-based like graph-v2. Therefore:
- `edge_reference_t<G>` does not exist and must be replaced with `edge_t<G>`
- `edge_t<G>` is the edge descriptor/value type (defined via `range_value_t<vertex_edge_range_t<G>>`)

### 4. Function Call Updates

**Current Direct Calls:**
```cpp
target_id(g, e)         // Used in algorithm
find_vertex(g, uid)     // Used in visitor calls
num_vertices(g)         // Used for size checks
vertices(g)             // Used for initialization
```

**Action:** Ensure all use proper namespace qualification:
- These should resolve via ADL to `graph::adj_list::target_id`, etc.
- The CPOs are defined in `graph::adj_list::detail::graph_cpo.hpp` and brought into `graph::adj_list` namespace
- Current usage should work, but needs verification

### 5. Type Definitions in Algorithm

**Current (INCORRECT):**
```cpp
using id_type       = vertex_id_t<G>;
using distance_type = range_value_t<Distances>;
using weight_type   = invoke_result_t<WF, edge_reference_t<G>>;  // WRONG!
```

**Change to:**
```cpp
using id_type       = vertex_id_t<G>;              // OK as-is
using distance_type = range_value_t<Distances>;    // OK as-is  
using weight_type   = invoke_result_t<WF, edge_t<G>>;  // FIX: use edge_t, not edge_reference_t
```

### 6. CPO Namespace Usage

The CPOs are defined in:
- `graph::adj_list::detail::_cpo_impls` (implementation)
- Exported to `graph::adj_list` namespace via `inline constexpr` declarations

**Functions that need proper CPO resolution:**
- `vertices(g)` → `graph::adj_list::vertices`
- `num_vertices(g)` → `graph::adj_list::num_vertices`
- `find_vertex(g, uid)` → `graph::adj_list::find_vertex`
- `target_id(g, e)` → `graph::adj_list::target_id`
- `target(g, e)` → `graph::adj_list::target`

**Current calls in algorithm:**
```cpp
size(vertices(g))           // Line 109
num_vertices(g)             // Used with N
find_vertex(g, uid)         // Lines 135, 147, 156, 183, 210
target_id(g, e)             // Line 94
```

### 7. Namespace Qualification Strategy

**Option A - Explicit Qualification (Recommended):**
Add proper using declarations at top of namespace:
```cpp
namespace graph {
  // Bring CPOs into scope
  using adj_list::vertices;
  using adj_list::num_vertices;
  using adj_list::find_vertex;
  using adj_list::target_id;
  using adj_list::target;
  // ... etc
```

**Option B - ADL (Current approach):**
Rely on ADL to find the CPOs - this should work since graph types bring their namespace.

**Recommendation:** Use Option A for clarity and explicit control.

### 8. Concept Requirements Updates

**Current:**
```cpp
requires convertible_to<range_value_t<Sources>, vertex_id_t<G>> && //
         is_arithmetic_v<range_value_t<Distances>> &&              //
         sized_range<Distances> &&                                 //
         sized_range<Predecessors> &&                              //
         convertible_to<vertex_id_t<G>, range_value_t<Predecessors>> &&
         basic_edge_weight_function<G, WF, range_value_t<Distances>, Compare, Combine>
```

**Keep as-is** - these concepts are well-defined and CPO-compatible.

### 9. Edge Weight Function Lambda

**Current (INCORRECT):**
```cpp
WF&& weight = [](edge_reference_t<G> uv) { return range_value_t<Distances>(1); }
```

**Change to:**
```cpp
WF&& weight = [](edge_t<G> uv) { return range_value_t<Distances>(1); }
```

**Must fix:** `edge_reference_t<G>` doesn't exist. Use `edge_t<G>` (the edge descriptor value type).

### 10. Views Usage

**Current:**
```cpp
for (auto&& [vid, uv, w] : views::incidence(g, uid, weight))
```

**Keep as-is** - The views are already CPO-based and work correctly.

## Detailed Changes Required

### Change 1: Remove redundant include and old namespace using declarations
**Location:** Line 16 (include), Lines 28-31 (using declarations)

**Remove:**
```cpp
#include "graph/views/incidence.hpp"  // Line 16 - already in graph.hpp

// Using declarations for new namespace structure
using adj_list::index_adjacency_list;
using adj_list::vertex_id_t;
using adj_list::edge_reference_t;  // ERROR: doesn't exist in graph-v3
```

### Change 2: Add CPO using declarations (Optional - for clarity)
**Location:** After namespace declaration (line 27)

**Optional Add:**
```cpp
namespace graph {

// Import CPOs and types for use in algorithms (optional - ADL works, but this adds clarity)
using adj_list::vertices;
using adj_list::num_vertices;
using adj_list::find_vertex;
using adj_list::target_id;
using adj_list::target;
using adj_list::vertex_id_t;
using adj_list::edge_t;  // FIX: use edge_t, not edge_reference_t
using adj_list::index_adjacency_list;  // Keep this concept
```

**Note:** The CPO calls already work via ADL, so this is optional. However, it makes the code more explicit about what CPOs are being used.
Verify _null_predecessors is available
**Location:** Lines 100, 113 (type checks), Lines 285, 306 (usage)

**Status:** ✅ Already defined in `common_shortest_paths.hpp` (lines 142-165)

The code uses `_null_range_type` and `_null_predecessors` which are already properly defined in the included `common_shortest_paths.hpp` file. No changes needed here
**Note:** This type is a dummy range that satisfies the `random_access_range` concept but never actually stores data. The algorithm uses `if constexpr (is_same_v<Predecessors, _null_range_type>)` to skip predecessor updates when only distances are needed.

## Phased Implementation

### Phase 1: Common Infrastructure (✅ COMPLETE)
**Goal**: Fix shared dependencies that both algorithms rely on

**Tasks**:
1. ✅ Create this refactoring plan document
2. ✅ Fix `edge_reference_t<G>` → `edge_t<G>` in `common_shortest_paths.hpp` (6 locations)
   - Line 15: `basic_edge_weight_function` concept
   - Line 19: `edge_weight_function` concept
   - Line 106: `has_on_examine_edge` concept
   - Line 111: `has_on_edge_relaxed` concept
   - Line 116: `has_on_edge_not_relaxed` concept
   - Line 121: `has_on_edge_minimized` concept
   - Line 126: `has_on_edge_not_minimized` concept

**Verification**:
- File compiles successfully
- No new warnings introduced
- `_null_range_type` and `_null_predecessors` remain intact

**Status**: ✅ Complete

---

### Phase 2: Update dijkstra_shortest_paths.hpp
**Goal**: Refactor main algorithm file to use CPOs correctly

**Dependencies**: Phase 1 must be complete

**Tasks**:
1. Remove redundant include
   - **Line 16**: Delete `#include "graph/views/incidence.hpp"`

2. Remove old using declarations
   - **Lines 28-31**: Delete:
     ```cpp
     using adj_list::index_adjacency_list;
     using adj_list::vertex_id_t;
     using adj_list::edge_reference_t;
     ```

3. (Optional) Add explicit CPO using declarations
   - **After line 27** (after `namespace graph {`), add:
     ```cpp
     // Import CPOs and types for use in algorithms
     using adj_list::vertices;
     using adj_list::num_vertices;
     using adj_list::find_vertex;
     using adj_list::target_id;
     using adj_list::vertex_id_t;
     using adj_list::edge_t;
     using adj_list::index_adjacency_list;
     ```

4. Fix `edge_reference_t<G>` → `edge_t<G>` (4 locations)
   - **Line 69**: Template parameter `WF`
   - **Line 80**: Default weight lambda parameter
   - **Line 269**: Template parameter `WF`
   - **Line 280**: Default weight lambda parameter
   - **Line 293**: Template parameter `WF`
   - **Line 304**: Default weight lambda parameter

**Verification**:
- File compiles successfully
- Run: `ctest -R dijkstra_shortest_paths --output-on-failure`
- Verify both predecessors and distances-only versions work

**Checkpoint**: Commit changes after successful compilation and tests

---

### Phase 3: Update dijkstra.hpp (Cleanup)
**Goal**: Remove duplicate definitions now that they're in common header

**Dependencies**: Phase 1 must be complete

**Tasks**:
1. Check if `dijkstra.hpp` includes `common_shortest_paths.hpp`
   - If not, add: `#include "graph/algorithm/common_shortest_paths.hpp"`

2. Remove duplicate `_null_range_type` class definition
   - Currently at lines 50-70 in `dijkstra.hpp`

3. Remove duplicate `null_predecessors` variable
   - Should be right after `_null_range_type` definition

4. Verify `_null_predecessors` is used (not `null_predecessors`)
   - Use consistent naming with `common_shortest_paths.hpp`

**Verification**:
- File compiles successfully
- Run: `ctest -R dijkstra --output-on-failure`
- Verify dijkstra() function still works correctly

**Checkpoint**: Commit changes after successful compilation and tests

---

### Phase 4: Comprehensive Testing
**Goal**: Ensure all changes work together correctly

**Dependencies**: Phases 2 and 3 must be complete

**Tasks**:
1. Clean build
   ```bash
   rm -rf build/linux-gcc-debug
   cmake --preset linux-gcc-debug
   cmake --build build/linux-gcc-debug
   ```

2. Run full test suite
   ```bash
   cd build/linux-gcc-debug
   ctest --output-on-failure
   ```

3. Run algorithm-specific tests
   ```bash
   ctest -R dijkstra --output-on-failure -V
   ```

4. Test with example code (see Testing Instructions section below)

5. Verify no performance regression
   ```bash
   cmake --preset linux-gcc-release
   cmake --build build/linux-gcc-release
   cd build/linux-gcc-release/benchmark
   ./benchmark_algorithms
   ```

6. Test with multiple compilers
   ```bash
   cmake --preset linux-clang-debug
   cmake --build build/linux-clang-debug
   cd build/linux-clang-debug
   ctest --output-on-failure
   ```

**Verification**:
- All tests pass
- No new warnings
- Performance matches baseline
- Examples run correctly

---

### Phase 5: Documentation and Finalization
**Goal**: Update documentation and verify completeness

**Dependencies**: Phase 4 must be complete

**Tasks**:
1. Update any algorithm documentation if needed
2. Verify all files have proper copyright headers
3. Check for consistent code style
4. Update CHANGELOG or version notes if applicable
5. Final review of changes

**Verification**:
- Documentation is accurate
- Code style is consistent
- All TODOs resolved
- Ready for merge/commit

---

## Quick Reference: Implementation Steps

For those who want a condensed view:

1. ✅ Fix `common_shortest_paths.hpp` (edge_reference_t → edge_t)
2. ⏳ Fix `dijkstra_shortest_paths.hpp` (remove include, fix using declarations, fix edge_reference_t)
3. ⏳ Fix `dijkstra.hpp` (remove duplicate _null_range_type)
4. ⏳ Test compilation
5. ⏳ Run full test suite
6. ⏳ Final verification

---

## Implementation Steps (Original)

1. ✅ Create this refactoring plan document
2. Update `dijkstra_shortest_paths.hpp`:
   - Remove redundant `#include "graph/views/incidence.hpp"`
   - Remove old using declarations (or replace with cleaner ones)
   - **FIX: Replace all `edge_reference_t<G>` with `edge_t<G>`** (critical fix - 4 locations)
   - **FIX: Add `_null_range_type` and `_null_predecessors` definitions** (copy from dijkstra.hpp)
   - Add explicit CPO using declarations (optional, for clarity)
   - Keep `index_adjacency_list` concept (required for integral vertex ID indexing)
3. Test compilation
4. Run tests to verify correctness

## Risk Assessment

**Low Risk Changes:**
- Removing redundant include (compile-time catch)
- Removing/updating using declarations (compile-time catch)
- Adding explicit CPO using declarations (improves clarity, no functional change)
- Replacing `edge_reference_t<G>` with `edge_t<G>` (compile-time catch)
- Removing duplicate `_null_range_type` from `dijkstra.hpp` (already in common header)

**Medium Risk Changes:**
- None - all changes are low risk and caught at compile time

**Verification Steps:**
1. Compile with example code
2. Run existing algorithm tests
3. Check that type deduction still works correctly
4. Verify visitor pattern still functions

## Testing Instructions

### Pre-Refactoring Tests
Before making changes, establish a baseline:

```bash
# Build the project
cmake --preset linux-gcc-debug
cmake --build build/linux-gcc-debug

# Run all tests to ensure current state
cd build/linux-gcc-debug
ctest --output-on-failure

# Run specific algorithm tests
ctest -R dijkstra --output-on-failure
```

### Post-Refactoring Compilation Tests

After each major change, verify compilation:

```bash
# Clean build to catch any issues
rm -rf build/linux-gcc-debug
cmake --preset linux-gcc-debug
cmake --build build/linux-gcc-debug 2>&1 | tee build.log

# Check for compilation errors related to:
# - edge_reference_t -> edge_t changes
# - Missing CPO definitions
# - Type deduction failures
grep -i "error" build.log
```

### Specific Test Cases to Verify

1. **Basic Dijkstra Shortest Paths Test**
   - Location: `tests/algorithms/` (or similar)
   - Run: `ctest -R dijkstra_shortest_paths -V`
   - Verify: Single-source shortest paths computation
   - Check: Distances and predecessors are correct

2. **Multi-Source Test**
   - Verify the multi-source variant works correctly
   - Test with 2+ source vertices
   - Ensure all distances computed correctly

3. **Dijkstra Shortest Distances Test**
   - Verify the version without predecessors works
   - Uses `_null_predecessors` internally
   - Should compile and run without predecessor tracking overhead

4. **Visitor Pattern Test**
   - Verify visitor callbacks are invoked correctly
   - Test with custom visitor implementation
   - Check: on_examine_vertex, on_examine_edge, on_edge_relaxed, etc.

5. **Edge Weight Function Test**
   - Test with custom weight functions
   - Verify `edge_t<G>` parameter works correctly (not `edge_reference_t<G>`)
   - Test with different edge types (weighted, unweighted)

6. **Type Deduction Test**
   - Verify template argument deduction works
   - Test calling without explicit template parameters
   - Ensure `edge_t<G>` is correctly deduced in lambdas

### Example Test Code to Run

Create a simple test file to verify the changes:

```cpp
#include <graph/graph.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/container/compressed_graph.hpp>
#include <vector>

int main() {
    using namespace graph;
    
    // Create a simple graph
    compressed_graph<int, int> g(5);
    // ... add edges ...
    
    // Test 1: dijkstra_shortest_paths with predecessors
    std::vector<int> distances(5);
    std::vector<size_t> predecessors(5);
    init_shortest_paths(distances, predecessors);
    
    auto weight = [](auto&& e) { return 1; }; // Should work with edge_t<G>
    dijkstra_shortest_paths(g, 0, distances, predecessors, weight);
    
    // Test 2: dijkstra_shortest_distances without predecessors
    std::vector<int> distances2(5);
    init_shortest_paths(distances2);
    dijkstra_shortest_distances(g, 0, distances2, weight);
    
    // Test 3: Custom visitor
    struct my_visitor {
        void on_examine_vertex(const auto& v) { /* ... */ }
        void on_examine_edge(const auto& e) { /* ... */ }
    };
    
    dijkstra_shortest_paths(g, 0, distances, predecessors, weight, my_visitor{});
    
    return 0;
}
```

Compile and run:
```bash
g++ -std=c++20 -I include test_dijkstra_refactor.cpp -o test_dijkstra_refactor
./test_dijkstra_refactor
```

### Integration Tests

Run existing example code:

```bash
# Build examples
cd build/linux-gcc-debug/examples
make

# Run dijkstra example if it exists
./dijkstra_clrs_example

# Check output matches expected results
```

### Performance Verification

Ensure refactoring didn't impact performance:

```bash
# Build in release mode
cmake --preset linux-gcc-release
cmake --build build/linux-gcc-release

# Run benchmarks
cd build/linux-gcc-release/benchmark
./benchmark_algorithms

# Compare results with baseline (before refactoring)
```

### Edge Cases to Test

1. **Empty graph**: Verify behavior with 0 vertices
2. **Single vertex**: Graph with 1 vertex, no edges
3. **Disconnected graph**: Vertices unreachable from source
4. **Self-loops**: If supported by graph type
5. **Negative weights**: Should throw exception (Dijkstra requirement)
6. **Large graphs**: Performance test with 10k+ vertices

### Regression Testing

After all changes are complete:

```bash
# Run full test suite
cd build/linux-gcc-debug
ctest --output-on-failure

# Run with multiple compilers
cmake --preset linux-clang-debug
cmake --build build/linux-clang-debug
cd build/linux-clang-debug
ctest --output-on-failure

# Check for warnings
cmake --build build/linux-gcc-debug 2>&1 | grep -i warning
```

### Success Criteria

✅ All tests pass (same as before refactoring)
✅ No new compiler warnings introduced
✅ Examples compile and run correctly
✅ Performance benchmarks show no regression
✅ Code compiles with both GCC and Clang
✅ Type deduction works as expected
✅ Visitor pattern functions correctly
✅ `_null_predecessors` optimization works

## Expected Benefits

1. ✅ Fix `edge_reference_t<G>` → `edge_t<G>` in `common_shortest_paths.hpp` (6 locations)
3. Update `dijkstra_shortest_paths.hpp`:
   - Remove redundant `#include "graph/views/incidence.hpp"`
   - Remove old using declarations (or replace with cleaner ones)
   - **FIX: Replace all `edge_reference_t<G>` with `edge_t<G>`** (critical fix - 4 locations)
   - Add explicit CPO using declarations (optional, for clarity)
   - Keep `index_adjacency_list` concept (required for integral vertex ID indexing)
   - Verify `_null_predecessors` usage works (already defined in common_shortest_paths.hpp)
4. Update `dijkstra.hpp`:
   - Remove duplicate `_null_range_type` and `_null_predecessors` definitions
   - Rely on definitions from `common_shortest_paths.hpp`
5. Test compilation
6 Should maintain backward compatibility with existing code
- Type deduction will work the same way
- Performance should be identical (CPOs optimize away)
- Binary compatibility maintained (header-only templates)
