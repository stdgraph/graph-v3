#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <list>
#include <string>

#include "graph/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Default Implementation with Vector
// =============================================================================

TEST_CASE("num_vertices(g) - vector<vector<int>> default", "[num_vertices][cpo][default]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {}
    };
    
    REQUIRE(num_vertices(graph) == 4);
}

TEST_CASE("num_vertices(g) - empty vector", "[num_vertices][cpo][default]") {
    std::vector<std::vector<int>> graph;
    
    REQUIRE(num_vertices(graph) == 0);
}

TEST_CASE("num_vertices(g) - single vertex", "[num_vertices][cpo][default]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3}
    };
    
    REQUIRE(num_vertices(graph) == 1);
}

// =============================================================================
// Test: Default Implementation with Deque
// =============================================================================

TEST_CASE("num_vertices(g) - deque storage", "[num_vertices][cpo][default]") {
    std::deque<std::deque<int>> graph = {
        {10, 20},
        {30},
        {},
        {40, 50, 60}
    };
    
    REQUIRE(num_vertices(graph) == 4);
}

TEST_CASE("num_vertices(g) - empty deque", "[num_vertices][cpo][default]") {
    std::deque<std::deque<int>> graph;
    
    REQUIRE(num_vertices(graph) == 0);
}

// =============================================================================
// Test: Default Implementation with Map
// =============================================================================

TEST_CASE("num_vertices(g) - map storage", "[num_vertices][cpo][default]") {
    std::map<int, std::vector<int>> graph = {
        {0, {1, 2}},
        {1, {2}},
        {2, {}}
    };
    
    REQUIRE(num_vertices(graph) == 3);
}

TEST_CASE("num_vertices(g) - empty map", "[num_vertices][cpo][default]") {
    std::map<int, std::vector<int>> graph;
    
    REQUIRE(num_vertices(graph) == 0);
}

TEST_CASE("num_vertices(g) - unordered_map storage", "[num_vertices][cpo][default]") {
    std::unordered_map<int, std::vector<int>> graph = {
        {10, {20, 30}},
        {20, {30}},
        {30, {}},
        {40, {50}}
    };
    
    REQUIRE(num_vertices(graph) == 4);
}

// =============================================================================
// Test: Different Edge Types
// =============================================================================

TEST_CASE("num_vertices(g) - pair edges", "[num_vertices][cpo][edge_types]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 3.5}},
        {},
        {{0, 4.5}}
    };
    
    REQUIRE(num_vertices(graph) == 4);
}

TEST_CASE("num_vertices(g) - tuple edges", "[num_vertices][cpo][edge_types]") {
    using Edge = std::tuple<int, double, std::string>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5, "a"}, {2, 2.5, "b"}},
        {{2, 3.5, "c"}},
        {{0, 4.5, "d"}},
        {},
        {{1, 5.5, "e"}}
    };
    
    REQUIRE(num_vertices(graph) == 5);
}

// =============================================================================
// Test: Custom Member Function Override
// =============================================================================

TEST_CASE("num_vertices(g) - custom member function", "[num_vertices][cpo][member]") {
    struct CustomGraph {
        std::vector<std::vector<int>> data = {
            {1, 2},
            {2},
            {}
        };
        
        // Custom num_vertices member
        size_t num_vertices() const {
            return data.size() * 10;  // Custom logic: multiply by 10
        }
    };
    
    CustomGraph g;
    
    REQUIRE(num_vertices(g) == 30);  // 3 * 10
}

TEST_CASE("num_vertices(g) - custom member returns different type", "[num_vertices][cpo][member]") {
    struct CustomGraph {
        std::vector<std::vector<int>> data = {
            {1}, {2}, {3}, {4}, {5}
        };
        
        // Custom num_vertices returns int
        int num_vertices() const {
            return static_cast<int>(data.size()) + 100;
        }
    };
    
    CustomGraph g;
    
    REQUIRE(num_vertices(g) == 105);  // 5 + 100
}

// =============================================================================
// Test: Custom ADL Override
// =============================================================================

namespace custom_adl_ns {
    struct CustomGraph {
        std::vector<std::vector<int>> data = {
            {10, 20},
            {30},
            {}
        };
    };
    
    // ADL-findable num_vertices function
    size_t num_vertices(const CustomGraph& g) {
        return g.data.size() + 1000;  // Custom logic: add 1000
    }
}

TEST_CASE("num_vertices(g) - ADL customization", "[num_vertices][cpo][adl]") {
    custom_adl_ns::CustomGraph g;
    
    REQUIRE(num_vertices(g) == 1003);  // 3 + 1000
}

// =============================================================================
// Test: Integration with vertices(g)
// =============================================================================

TEST_CASE("num_vertices(g) - consistency with vertices(g)", "[num_vertices][cpo][integration]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {},
        {0}
    };
    
    auto verts = vertices(graph);
    size_t counted = 0;
    for (auto v : verts) {
        (void)v;  // Suppress unused warning
        ++counted;
    }
    
    REQUIRE(num_vertices(graph) == counted);
    REQUIRE(num_vertices(graph) == 5);
}

TEST_CASE("num_vertices(g) - consistency with map vertices", "[num_vertices][cpo][integration]") {
    std::map<int, std::vector<int>> graph = {
        {10, {20, 30}},
        {20, {30}},
        {30, {}},
        {40, {10}}
    };
    
    auto verts = vertices(graph);
    size_t counted = 0;
    for (auto v : verts) {
        (void)v;
        ++counted;
    }
    
    REQUIRE(num_vertices(graph) == counted);
    REQUIRE(num_vertices(graph) == 4);
}

// =============================================================================
// Test: Type Deduction
// =============================================================================

TEST_CASE("num_vertices(g) - type deduction", "[num_vertices][cpo][types]") {
    std::vector<std::vector<int>> graph(10);
    
    auto count = num_vertices(graph);
    static_assert(std::is_same_v<decltype(count), std::size_t>);
    REQUIRE(count == 10);
}

TEST_CASE("num_vertices(g) - const graph", "[num_vertices][cpo][types]") {
    const std::vector<std::vector<int>> graph = {
        {1}, {2}, {3}
    };
    
    REQUIRE(num_vertices(graph) == 3);
}

// =============================================================================
// Test: Edge Cases
// =============================================================================

TEST_CASE("num_vertices(g) - very large graph", "[num_vertices][cpo][edge_cases]") {
    std::vector<std::vector<int>> graph(10000);
    
    REQUIRE(num_vertices(graph) == 10000);
}

TEST_CASE("num_vertices(g) - map with non-contiguous keys", "[num_vertices][cpo][edge_cases]") {
    std::map<int, std::vector<int>> graph = {
        {5, {10}},
        {100, {200}},
        {1000, {}}
    };
    
    // num_vertices counts entries, not key range
    REQUIRE(num_vertices(graph) == 3);
}

TEST_CASE("num_vertices(g) - nested containers with different sizes", "[num_vertices][cpo][edge_cases]") {
    std::vector<std::vector<int>> graph = {
        {},              // 0 edges
        {1},             // 1 edge
        {1, 2},          // 2 edges
        {1, 2, 3},       // 3 edges
        {1, 2, 3, 4}     // 4 edges
    };
    
    REQUIRE(num_vertices(graph) == 5);  // 5 vertices regardless of edge counts
}

// =============================================================================
// Test: Noexcept Specifications
// =============================================================================

TEST_CASE("num_vertices(g) - noexcept for vector", "[num_vertices][cpo][noexcept]") {
    std::vector<std::vector<int>> graph;
    
    static_assert(noexcept(num_vertices(graph)));
}

TEST_CASE("num_vertices(g) - noexcept for map", "[num_vertices][cpo][noexcept]") {
    std::map<int, std::vector<int>> graph;
    
    static_assert(noexcept(num_vertices(graph)));
}

// =============================================================================
// Test: Integration with Complete Graph Operations
// =============================================================================

TEST_CASE("num_vertices(g) - full graph traversal", "[num_vertices][cpo][integration]") {
    std::vector<std::vector<std::pair<int, double>>> graph = {
        {{1, 1.0}, {2, 2.0}},
        {{2, 3.0}},
        {{0, 4.0}},
        {}
    };
    
    REQUIRE(num_vertices(graph) == 4);
    
    // Verify we can traverse all vertices
    size_t vertex_count = 0;
    size_t edge_count = 0;
    
    for (auto v : vertices(graph)) {
        ++vertex_count;
        for (auto e : edges(graph, v)) {
            (void)e;
            ++edge_count;
        }
    }
    
    REQUIRE(vertex_count == num_vertices(graph));
    REQUIRE(edge_count == 4);  // Total edges in graph
}

TEST_CASE("num_vertices(g) - integration with vertex_id", "[num_vertices][cpo][integration]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3, 0}
    };
    
    REQUIRE(num_vertices(graph) == 3);
    
    // Verify all vertex IDs are within bounds
    for (auto v : vertices(graph)) {
        auto id = vertex_id(graph, v);
        REQUIRE(id < num_vertices(graph));
    }
}
