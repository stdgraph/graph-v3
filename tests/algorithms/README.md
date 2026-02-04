# Algorithm Tests

This directory contains tests for graph algorithms implemented in the graph-v3 library.

## Overview

Tests use the Catch2 framework with parameterized testing to validate algorithms across:
- Multiple container types (vov, dov, vol, dol, mov, etc.)
- Various edge value types (void, int, double, string)
- Different graph configurations (sparse, dense, directed, undirected)

## Test Organization

### File Naming Convention

```
test_<algorithm_name>.cpp
```

Examples:
- `test_dijkstra.cpp` - Dijkstra's shortest path algorithm
- `test_bfs.cpp` - Breadth-first search
- `test_dfs.cpp` - Depth-first search
- `test_mst.cpp` - Minimum spanning tree algorithms

### Test Structure Pattern

Each algorithm test file follows this structure:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include "tests/common/graph_fixtures.hpp"
#include "tests/common/algorithm_test_types.hpp"

using namespace graph;
using namespace graph::test;

// Basic correctness tests with known results
TEST_CASE("Dijkstra correctness", "[algorithm][dijkstra]") {
    // Test with specific known graph
    auto g = fixtures::clrs_dijkstra_graph<vov_weighted>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [](const auto& e) { return edge_value(e); });
    
    // Validate against known results
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 8);
    REQUIRE(distance[2] == 9);
    // ... more assertions
}

// Edge case tests
TEST_CASE("Dijkstra edge cases", "[algorithm][dijkstra]") {
    SECTION("empty graph") {
        auto g = fixtures::empty_graph<vov_weighted>();
        // ... test behavior
    }
    
    SECTION("single vertex") {
        auto g = fixtures::single_vertex<vov_weighted>();
        // ... test behavior
    }
    
    SECTION("disconnected graph") {
        auto g = fixtures::disconnected_graph<vov_weighted>();
        // ... test unreachable vertices have infinity distance
    }
}

// Parameterized tests across container types
TEMPLATE_TEST_CASE("Dijkstra container compatibility",
                   "[algorithm][dijkstra][template]",
                   BASIC_WEIGHTED_TYPES) {
    using Graph = TestType;
    
    auto g = fixtures::path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [](const auto& e) { return edge_value(e); });
    
    // Validate results
    auto expected = fixtures::path_graph_4_results;
    for (size_t i = 0; i < expected.num_vertices; ++i) {
        REQUIRE(distance[i] == expected.distances[i]);
    }
}

// Comprehensive container tests
TEMPLATE_TEST_CASE("Dijkstra all containers",
                   "[algorithm][dijkstra][comprehensive]",
                   ALL_DIRECTED_WEIGHTED_TYPES) {
    // ... more extensive testing
}
```

## Test Categories

### 1. Correctness Tests

Validate algorithm produces correct results:
- Known example graphs (e.g., CLRS textbook examples)
- Path reconstruction validation
- Expected distances from source
- Predecessor chain verification

### 2. Edge Case Tests

Test boundary conditions and special cases:
- Empty graph (0 vertices, 0 edges)
- Single vertex
- Single edge
- Self-loops
- Disconnected components
- Unreachable vertices
- All vertices reachable from source

### 3. Container Type Tests

Verify algorithm works with all supported containers:
- Vector-based: `vov`, `vod`, `vol`
- Deque-based: `dov`, `dod`, `dol`
- Map-based (sparse): `mov`, `mod`, `mol`
- Set-based edges: `vos`, `dos`, `mos`
- Forward list edges: `vofl`, `dofl`, `mofl`

### 4. Value Type Tests

Test with different edge value types:
- `void` (unweighted)
- `int` (integer weights)
- `double` (floating-point weights)
- `float` (single precision)

### 5. Property Tests

Test algorithm properties:
- Optimal substructure
- Shortest path triangle inequality
- Path existence ⇔ finite distance
- Predecessor chain forms valid tree

## Using Test Fixtures

### Simple Usage

```cpp
// Get a standard test graph
auto g = fixtures::path_graph_4_weighted<Graph>();

// Access expected results
auto expected = fixtures::path_graph_4_results;
REQUIRE(num_vertices(g) == expected.num_vertices);
```

### Parameterized Testing

```cpp
// Test across multiple container types
TEMPLATE_TEST_CASE("algorithm test", "[tag]",
                   BASIC_WEIGHTED_TYPES) {
    using Graph = TestType;
    auto g = fixtures::diamond_dag_weighted<Graph>();
    // ... run algorithm and validate
}
```

### Custom Fixture Selection

```cpp
// Use fixture selector for container-specific fixtures
if constexpr (algorithm::fixture_selector<Graph>::use_sparse) {
    // Use sparse vertex ID fixtures
    auto g = map_data::make_sparse_graph_int<Graph>();
} else {
    // Use contiguous vertex ID fixtures
    auto g = fixtures::clrs_dijkstra_graph<Graph>();
}
```

## Test Macros

### Available Type Lists

From `algorithm_test_types.hpp`:

```cpp
BASIC_DIRECTED_TYPES          // vov_void, dov_void
BASIC_WEIGHTED_TYPES          // vov_weighted, dov_weighted
ALL_DIRECTED_TYPES            // All void edge types
ALL_DIRECTED_WEIGHTED_TYPES   // All int edge types
SPARSE_VERTEX_TYPES           // Map-based containers
ORDERED_EDGE_TYPES            // Set-based edge containers
FORWARD_EDGE_TYPES            // Forward_list edge containers
EXTENDED_TYPES                // Large selection (slow compile)
ALL_ALGORITHM_TYPES           // Everything (very slow compile)
```

### Choosing the Right Type List

- **Development:** Use `BASIC_WEIGHTED_TYPES` for fast iteration
- **Pre-commit:** Use `ALL_DIRECTED_WEIGHTED_TYPES` for good coverage
- **CI/Full test:** Use `EXTENDED_TYPES` or `ALL_ALGORITHM_TYPES`

## Running Tests

### Run All Algorithm Tests

```bash
cd build
ctest -R algorithms
```

### Run Specific Algorithm Tests

```bash
./build/tests/test_main "[dijkstra]"
```

### Run Specific Test Case

```bash
./build/tests/test_main "Dijkstra correctness"
```

### Run with Verbosity

```bash
./build/tests/test_main "[dijkstra]" -s  # Show all assertions
./build/tests/test_main "[dijkstra]" -d yes  # Debug output
```

### Run Subset of Template Instantiations

```bash
# Only run tests for vov and dov containers
./build/tests/test_main "[dijkstra][template]" -# "*vov*,*dov*"
```

## Writing New Algorithm Tests

### Checklist

- [ ] Create `test_<algorithm>.cpp` in `tests/algorithms/`
- [ ] Include correctness tests with known results
- [ ] Include edge case tests (empty, single vertex, disconnected, etc.)
- [ ] Add parameterized tests with `BASIC_WEIGHTED_TYPES`
- [ ] Add comprehensive tests with `ALL_DIRECTED_WEIGHTED_TYPES`
- [ ] Test path reconstruction if applicable
- [ ] Test with different weight types if applicable
- [ ] Document any algorithm-specific requirements
- [ ] Add tests to CMakeLists.txt
- [ ] Verify tests pass before committing

### Template for New Test File

```cpp
/**
 * @file test_<algorithm>.cpp
 * @brief Tests for <algorithm> algorithm
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/<algorithm>.hpp>
#include "tests/common/graph_fixtures.hpp"
#include "tests/common/algorithm_test_types.hpp"

using namespace graph;
using namespace graph::test;
using namespace graph::test::fixtures;

TEST_CASE("<Algorithm> correctness", "[algorithm][<tag>]") {
    // Known result validation
}

TEST_CASE("<Algorithm> edge cases", "[algorithm][<tag>]") {
    SECTION("empty graph") { }
    SECTION("single vertex") { }
    SECTION("single edge") { }
    SECTION("disconnected") { }
}

TEMPLATE_TEST_CASE("<Algorithm> container compatibility",
                   "[algorithm][<tag>][template]",
                   BASIC_WEIGHTED_TYPES) {
    using Graph = TestType;
    // Test with multiple containers
}
```

## Test Coverage Goals

Aim for comprehensive coverage:
- ✅ All public APIs tested
- ✅ All major container types validated
- ✅ Edge cases documented and tested
- ✅ Known algorithms verified against reference implementations
- ✅ Property-based invariants checked
- ⚠️ Performance characteristics validated (use benchmarks)
- ⚠️ Exception safety tested where applicable

## Debugging Failed Tests

### View Test Output

```bash
./build/tests/test_main "[failing_tag]" -s
```

### Isolate Specific Container Type

```cpp
// Temporarily replace macro with specific type
TEMPLATE_TEST_CASE("test", "[tag]", vov_weighted) {
    // Now only tests vov_weighted
}
```

### Print Intermediate Results

```cpp
std::cout << "Distance: ";
for (auto d : distance) std::cout << d << " ";
std::cout << "\n";
```

### Use Debugger

```bash
gdb ./build/tests/test_main
(gdb) break "test case name"
(gdb) run "[tag]"
```

## Integration with CI

Tests should:
- Run on every commit
- Complete in reasonable time (<5 minutes for basic tests)
- Use `EXTENDED_TYPES` or `ALL_ALGORITHM_TYPES` only in nightly builds
- Report coverage metrics
- Fail the build if any test fails

## Additional Resources

- [Catch2 Documentation](https://github.com/catchorg/Catch2)
- [Graph Fixtures Documentation](../common/graph_fixtures.hpp)
- [Algorithm Type Lists](../common/algorithm_test_types.hpp)
- [Project Testing Guidelines](../../docs/testing_guidelines.md) (to be created)
