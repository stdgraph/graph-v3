# Change the function signature of VVF and EVF

## The Problem: Capturing Lambdas Break View Chaining

### Current Signatures
Currently, VVF (Vertex Value Function) and EVF (Edge Value Function) have simple signatures:
- VVF: `vvf(u)` - takes only the vertex descriptor
- EVF: `evf(uv)` - takes only the edge descriptor

### The Capture Requirement
When users need to access graph operations (like `vertex_id`, `edge_value`, etc.), they must 
**capture** the graph reference in their lambda:

```cpp
// Current pattern - MUST capture &g
auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
//          ^^^^ Captured variable makes lambda stateful

auto view = g | vertexlist(vvf);
```

### Why This Is a Problem
**Capturing lambdas cannot be chained with `std::views`** because:

1. Lambdas with captures are **not default constructible**
   - The compiler doesn't know what value to give `g` when default-constructing
2. This makes them **not semiregular** (not default_initializable + copyable)
3. Views storing non-semiregular lambdas **don't satisfy `std::ranges::view`**
4. `std::views` adaptors **refuse to work** with non-view types

**Result**: This FAILS to compile:
```cpp
auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf) 
              | std::views::take(2);  // ❌ Compilation error!
```

See [view_chaining_limitations.md](../docs/view_chaining_limitations.md) for detailed explanation.

## The Solution: Pass Graph as Parameter

Change the signatures to:
- VVF: `vvf(g, u)` - graph and vertex descriptor as parameters
- EVF: `evf(g, uv)` - graph and edge descriptor as parameters

### Key Benefit: Stateless Lambdas
By passing the graph as a **parameter** instead of requiring **capture**, lambdas become stateless:

```cpp
// Proposed pattern - NO captures needed!
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
//         ^^ Empty capture list - stateless!
//            ^^^^^^^^^^^ Use reference to avoid copying graph

// Stateless lambdas ARE default constructible (capture list matters, not param types)
static_assert(std::default_initializable<decltype(vvf)>);  // ✅ true!
static_assert(std::semiregular<decltype(vvf)>);            // ✅ true!
```

**Important**: The **capture list `[]`** determines statefulness, not the parameter types!
- `[](auto g, ...)` - stateless, but copies graph (expensive ❌)
- `[](auto& g, ...)` - stateless, no copy ✅
- `[](const auto& g, ...)` - stateless, no copy, const-correct ✅✅
- `[&g](auto v, ...)` - **NOT stateless** due to capture ❌

Use `const auto&` for the graph parameter to avoid expensive copies while maintaining the
stateless property that enables view chaining.

### This Enables View Chaining
With stateless lambdas, views become semiregular and **can chain with `std::views`**:

```cpp
auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
auto view = g | vertexlist(vvf) 
              | std::views::take(2)        // ✅ Now works!
              | std::views::filter(...);   // ✅ Chains perfectly!
```

### Additional Benefits
1. **More explicit** - Clear that the function needs graph access
2. **More flexible** - Function can use any graph operation without pre-capturing
3. **No C++26 required** - Works in C++20 today (unlike `std::copyable_function` approach)
4. **Consistent pattern** - Similar to how many algorithm helper functions work

## Before/After Examples

### Vertex Value Functions
```cpp
// BEFORE: Must capture, can't chain
auto vvf = [&g](auto v) { 
    return vertex_id(g, v) * 10; 
};
auto view = g | vertexlist(vvf);  // Can't chain std::views

// AFTER: No capture, chains perfectly (use const& to avoid copying)
auto vvf = [](const auto& g, auto v) { 
    return vertex_id(g, v) * 10; 
};
auto view = g | vertexlist(vvf) | std::views::take(2);  // ✅ Chains!
```

### Edge Value Functions
```cpp
// BEFORE: Must capture
auto evf = [&g](auto e) { 
    return edge_value(g, e); 
};

// AFTER: No capture needed (const& avoids copy)
auto evf = [](const auto& g, auto e) { 
    return edge_value(g, e); 
};
```

## Implementation Scope

### 1. Update Concepts (include/graph/views/view_concepts.hpp)
The concepts already exist but need signature updates:

```cpp
// BEFORE
template <class VVF, class VertexDescriptor>
concept vertex_value_function =
    std::invocable<VVF, VertexDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<VVF, VertexDescriptor>>);

// AFTER - add Graph parameter (as const reference to avoid copies)
template <class VVF, class Graph, class VertexDescriptor>
concept vertex_value_function =
    std::invocable<VVF, const Graph&, VertexDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<VVF, const Graph&, VertexDescriptor>>);
```

Similar update needed for `edge_value_function`.

**Note**: The concept checks invocability with `const Graph&` to ensure the function can accept
the graph by reference, avoiding expensive copies while maintaining statelessness.

### 2. Update All Views (include/graph/views/*.hpp)
Views that use VVF or EVF need to pass graph as first argument:

Files to update:
- `vertexlist.hpp` - uses VVF
- `neighbors.hpp` - uses VVF  
- `incidence.hpp` - uses EVF
- `edgelist.hpp` - uses EVF
- `dfs.hpp` - uses VVF/EVF
- `bfs.hpp` - uses VVF/EVF
- `topological_sort.hpp` - uses VVF/EVF

Change invocations from:
```cpp
std::invoke(vvf_, v)           // Current
```
to:
```cpp
std::invoke(vvf_, *g_, v)      // Proposed (views already store g_)
// or for const correctness:
std::invoke(vvf_, std::as_const(*g_), v)  // If VVF shouldn't modify graph
```

### 3. Update Tests (tests/views/*.cpp)
All test files using value functions need lambda signature updates:

- `test_vertexlist.cpp`
- `test_neighbors.cpp`
- `test_incidence.cpp`
- `test_edgelist.cpp`
- `test_dfs.cpp`
- `test_bfs.cpp`
- `test_topological_sort.cpp`
- `test_view_concepts.cpp`
- `test_edge_cases.cpp`

#### 3.1 Add View Chaining Tests

**Critical**: Add comprehensive test cases to demonstrate that view chaining now works with value functions.

Create new test sections in each view test file to verify chaining with `std::views`:

```cpp
TEST_CASE("vertexlist - view chaining with value function", "[vertexlist][chaining]") {
    std::vector<std::vector<int>> g = {{1, 2}, {0, 2}, {0, 1}};
    
    SECTION("chain with std::views::take") {
        auto vvf = [](const auto& g, auto v) { 
            return vertex_id(g, v) * 10; 
        };
        
        auto view = g | vertexlist(vvf) 
                      | std::views::take(2);
        
        std::vector<int> values;
        for (auto [vid, v, val] : view) {
            values.push_back(val);
        }
        
        REQUIRE(values.size() == 2);
        REQUIRE(values[0] == 0);
        REQUIRE(values[1] == 10);
    }
    
    SECTION("chain with std::views::filter") {
        auto vvf = [](const auto& g, auto v) { 
            return vertex_id(g, v); 
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
        
        REQUIRE(count == 2);  // vertices 1 and 2
    }
    
    SECTION("chain with std::views::transform") {
        auto vvf = [](const auto& g, auto v) { 
            return vertex_id(g, v); 
        };
        
        auto view = g | vertexlist(vvf) 
                      | std::views::transform([](auto vi) {
                          auto [vid, v, val] = vi;
                          return val * 100;  // Transform the value
                        });
        
        std::vector<size_t> transformed;
        for (auto val : view) {
            transformed.push_back(val);
        }
        
        REQUIRE(transformed == std::vector<size_t>{0, 100, 200});
    }
    
    SECTION("multiple chained operations") {
        auto vvf = [](const auto& g, auto v) { 
            return static_cast<int>(vertex_id(g, v)); 
        };
        
        auto view = g | vertexlist(vvf) 
                      | std::views::filter([](auto vi) {
                          auto [vid, v, val] = vi;
                          return val >= 1;  // Skip vertex 0
                        })
                      | std::views::transform([](auto vi) {
                          auto [vid, v, val] = vi;
                          return val * 10;  // Multiply by 10
                        })
                      | std::views::take(1);  // Take just the first
        
        std::vector<int> result;
        for (auto val : view) {
            result.push_back(val);
        }
        
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 10);
    }
    
    SECTION("verify view is semiregular") {
        auto vvf = [](const auto& g, auto v) { 
            return vertex_id(g, v); 
        };
        
        using ViewType = decltype(g | vertexlist(vvf));
        
        // These should all be true now
        STATIC_REQUIRE(std::ranges::range<ViewType>);
        STATIC_REQUIRE(std::ranges::view<ViewType>);
        STATIC_REQUIRE(std::semiregular<ViewType>);
        STATIC_REQUIRE(std::default_initializable<ViewType>);
    }
}
```

**Test coverage required for each view type:**

1. **vertexlist.hpp**
   - Chain with `std::views::take`, `filter`, `transform`
   - Multiple chained operations
   - Verify semiregular property
   - Complex value functions (accessing graph properties)

2. **neighbors.hpp**
   - Chain neighbor iteration with std::views
   - Filter neighbors by computed values
   - Transform neighbor values

3. **incidence.hpp**
   - Chain edge iteration with std::views
   - Filter edges by weight/value
   - Access edge values through chained views

4. **edgelist.hpp**
   - Chain all edges with std::views
   - Filter by source/target properties
   - Transform edge values

5. **dfs.hpp / bfs.hpp / topological_sort.hpp**
   - Chain search views with std::views
   - Filter/transform during traversal
   - Early termination via `std::views::take_while`

6. **Complex scenarios**
   - Nested view chaining (view of view)
   - Value functions returning complex types
   - Const-correctness verification
   - Mutable vs const graph access

**Example edge case tests:**

```cpp
TEST_CASE("view chaining - edge cases", "[views][chaining][edge_cases]") {
    SECTION("empty graph with chaining") {
        std::vector<std::vector<int>> g;
        auto vvf = [](const auto& g, auto v) { return 0; };
        
        auto view = g | vertexlist(vvf) 
                      | std::views::take(10);
        
        REQUIRE(std::ranges::empty(view));
    }
    
    SECTION("value function with complex return type") {
        std::vector<std::vector<int>> g = {{1}, {2}, {}};
        
        struct VertexData {
            size_t id;
            size_t degree;
        };
        
        auto vvf = [](const auto& g, auto v) -> VertexData {
            auto vid = vertex_id(g, v);
            return {vid, g[vid].size()};
        };
        
        auto view = g | vertexlist(vvf)
                      | std::views::filter([](auto vi) {
                          auto [vid, v, data] = vi;
                          return data.degree > 0;
                        });
        
        size_t count = 0;
        for (auto [vid, v, data] : view) {
            REQUIRE(data.degree > 0);
            count++;
        }
        
        REQUIRE(count == 2);
    }
    
    SECTION("const graph with chaining") {
        const std::vector<std::vector<int>> g = {{1, 2}, {2}, {}};
        
        auto vvf = [](const auto& g, auto v) { 
            return vertex_id(g, v); 
        };
        
        auto view = g | vertexlist(vvf) 
                      | std::views::take(2);
        
        REQUIRE(std::ranges::distance(view) == 2);
    }
}
```

**Performance tests:**

Create benchmarks to ensure no performance regression:

```cpp
BENCHMARK("view chaining - baseline") {
    auto g = make_large_test_graph(10000);
    auto vvf = [](const auto& g, auto v) { 
        return vertex_id(g, v); 
    };
    
    size_t sum = 0;
    for (auto [vid, v, val] : g | vertexlist(vvf)) {
        sum += val;
    }
    return sum;
};

BENCHMARK("view chaining - with std::views") {
    auto g = make_large_test_graph(10000);
    auto vvf = [](const auto& g, auto v) { 
        return vertex_id(g, v); 
    };
    
    size_t sum = 0;
    for (auto [vid, v, val] : g | vertexlist(vvf) 
                                 | std::views::take(5000)) {
        sum += val;
    }
    return sum;
};
```

### 4. Update Documentation
- `docs/views.md` - Update VVF/EVF examples with new signatures
- `docs/view_chaining_limitations.md` - **Update to document that the problem is SOLVED**
  - Change title to "View Chaining with Value Functions" (remove "Limitations")
  - Document the new parameter-based approach
  - Update all examples to show `[](const auto& g, ...)` pattern
  - Add success examples showing chaining now works
  - Keep C++26 section for historical context
- `agents/view_strategy.md` - Update Section 9.3 design decision
  - Document that parameter-based approach was chosen
  - Explain the benefits (enables chaining, stateless lambdas)

### 5. Optional: Create Dedicated Chaining Test File

Consider creating `tests/views/test_view_chaining.cpp` to consolidate all chaining tests:

```cpp
// tests/views/test_view_chaining.cpp
#include <catch2/catch_test_macros.hpp>
#include <graph/views.hpp>
#include <ranges>
#include <vector>

using namespace graph;
using namespace graph::views;

// Comprehensive test suite for view chaining with std::views
// Tests that value functions with parameter-based signatures
// enable full compatibility with C++20 ranges

TEST_CASE("view chaining - vertexlist combinations") { ... }
TEST_CASE("view chaining - neighbors combinations") { ... }
TEST_CASE("view chaining - incidence combinations") { ... }
TEST_CASE("view chaining - edgelist combinations") { ... }
TEST_CASE("view chaining - dfs combinations") { ... }
TEST_CASE("view chaining - bfs combinations") { ... }
TEST_CASE("view chaining - complex pipelines") { ... }
TEST_CASE("view chaining - concept verification") { ... }
```

This provides a central location to verify that all views work correctly with `std::views` adaptors.

### 6. Algorithms (if applicable)
Check `include/graph/algorithm/*.hpp` for any use of VVF/EVF patterns. Currently, 
algorithms use weight functions with signature `weight(uv)` which may or may not need 
similar updates depending on usage patterns.

**Investigation needed:**
- Dijkstra, Bellman-Ford, Prim's MST use edge weight functions
- These typically access `edge_value(g, e)` which requires graph access
- Current pattern may already handle this differently (inline access vs. lambda)
- If they use lambdas, consider similar refactoring for consistency

**Decision criteria:**
- If weight functions commonly need graph access → apply same pattern
- If weight functions typically just extract edge values → may not need change
- Consistency with views is beneficial for user mental model

## Migration Path

Since this is in active development:
- **No backward compatibility needed** - breaking change is acceptable
- Update all code in one commit/PR to maintain consistency
- Ensure all tests pass before merging

**Testing checklist:**
1. ✅ Update existing view tests to use new signature
2. ✅ Add comprehensive view chaining tests for all views
3. ✅ Add concept verification tests (semiregular, default_initializable)
4. ✅ Add edge case tests (empty graphs, const graphs, complex return types)
5. ✅ Add performance benchmarks to ensure no regression
6. ✅ Verify all examples in documentation compile and run
7. ✅ Test with both mutable and const graphs

**Success criteria:**
- All existing tests pass with updated signatures
- New chaining tests demonstrate full `std::views` compatibility
- No compilation errors when chaining value functions with std::views
- Static assertions confirm views are semiregular
- Documentation examples all compile correctly

## Notes

### Graph Container Constructors
A similar pattern could be used in graph container constructors, but they typically use
copyable vertex/edge types rather than value functions. Evaluate if similar changes would
be beneficial there.

### Alternative Considered: std::copyable_function (C++26)
We could wait for C++26's `std::copyable_function` which makes capturing lambdas semiregular
via type erasure. However:
- C++26 is not yet available (current date: February 2026)
- The parameter-based approach is simpler and works today
- Type erasure has runtime overhead vs. direct lambda storage

The parameter-based approach is the better solution.

## Quick Reference: Test Files to Update/Create

### Files to Update (change lambda signatures):
- `tests/views/test_vertexlist.cpp` - ~7 lambda updates
- `tests/views/test_neighbors.cpp` - ~14 lambda updates
- `tests/views/test_incidence.cpp` - ~25 lambda updates
- `tests/views/test_edgelist.cpp` - ~24 lambda updates (see edgelist special case below)
- `tests/views/test_dfs.cpp` - ~10 lambda updates
- `tests/views/test_bfs.cpp` - ~4 lambda updates
- `tests/views/test_topological_sort.cpp` - ~9 lambda updates
- `tests/views/test_view_concepts.cpp` - ~14 lambda updates + STATIC_REQUIRE changes
- `tests/views/test_edge_cases.cpp` - check for lambda updates

### Files to Create (new chaining tests):
- `tests/views/test_view_chaining.cpp` - **New file** with comprehensive chaining tests
  - Or add "view chaining" test sections to existing files
  - Minimum 5-10 chaining tests per view type
  - Include concept verification tests
  - Include edge cases and complex scenarios

### Test Count Estimate:
- Existing test updates: ~80-100 lambda signature changes across all test files
- New chaining tests: ~50-70 new test cases
- Total effort: 2-4 hours for thorough implementation
