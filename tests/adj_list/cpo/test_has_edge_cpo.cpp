/**
 * @file test_has_edge_cpo.cpp
 * @brief Comprehensive tests for has_edge(g) CPO
 * 
 * Tests all resolution paths (member, ADL, default) and various scenarios
 */

#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>
#include <utility>
#include "graph/adj_list/descriptor.hpp"
#include "graph/adj_list/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test Graphs with Default Implementation
// =============================================================================

// Simple vector-based graph (adjacency list with vector of vectors)
using SimpleGraph = std::vector<std::vector<std::pair<std::size_t, int>>>;

TEST_CASE("has_edge - Empty graph", "[has_edge][default]") {
    SimpleGraph g;
    REQUIRE_FALSE(has_edge(g));
}

TEST_CASE("has_edge - Single vertex with no edges", "[has_edge][default]") {
    SimpleGraph g(1); // One vertex, no edges
    REQUIRE_FALSE(has_edge(g));
}

TEST_CASE("has_edge - Multiple vertices with no edges", "[has_edge][default]") {
    SimpleGraph g(5); // Five vertices, no edges
    REQUIRE_FALSE(has_edge(g));
}

TEST_CASE("has_edge - Single edge", "[has_edge][default]") {
    SimpleGraph g(2);
    g[0].push_back({1, 10}); // Edge from 0 to 1
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Multiple edges from first vertex", "[has_edge][default]") {
    SimpleGraph g(4);
    g[0].push_back({1, 10});
    g[0].push_back({2, 20});
    g[0].push_back({3, 30});
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Edge from middle vertex", "[has_edge][default]") {
    SimpleGraph g(5);
    // First two vertices have no edges
    g[2].push_back({3, 10}); // First edge is on vertex 2
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Edge from last vertex only", "[has_edge][default]") {
    SimpleGraph g(5);
    g[4].push_back({3, 10}); // Only vertex 4 has an edge
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Complete graph", "[has_edge][default]") {
    SimpleGraph g(3);
    // Complete graph: all vertices connected to all others
    g[0].push_back({1, 10});
    g[0].push_back({2, 20});
    g[1].push_back({0, 10});
    g[1].push_back({2, 30});
    g[2].push_back({0, 20});
    g[2].push_back({1, 30});
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Self-loop only", "[has_edge][default]") {
    SimpleGraph g(3);
    g[1].push_back({1, 10}); // Self-loop on vertex 1
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Linear chain", "[has_edge][default]") {
    SimpleGraph g(4);
    g[0].push_back({1, 10});
    g[1].push_back({2, 20});
    g[2].push_back({3, 30});
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Star graph", "[has_edge][default]") {
    SimpleGraph g(5);
    // Star: center vertex connected to all others
    g[0].push_back({1, 10});
    g[0].push_back({2, 20});
    g[0].push_back({3, 30});
    g[0].push_back({4, 40});
    REQUIRE(has_edge(g));
}

// =============================================================================
// Test with Map-Based Graph
// =============================================================================

using MapGraph = std::map<std::size_t, std::vector<std::pair<std::size_t, int>>>;

TEST_CASE("has_edge - Map graph with no edges", "[has_edge][default][map]") {
    MapGraph g;
    g[0] = {};
    g[1] = {};
    g[2] = {};
    REQUIRE_FALSE(has_edge(g));
}

TEST_CASE("has_edge - Map graph with edge", "[has_edge][default][map]") {
    MapGraph g;
    g[0] = {};
    g[1] = {{2, 10}};
    g[2] = {};
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Map graph sparse vertices", "[has_edge][default][map]") {
    MapGraph g;
    g[0] = {};
    g[10] = {};
    g[20] = {{30, 10}};
    g[30] = {};
    REQUIRE(has_edge(g));
}

// =============================================================================
// Test Custom Member Implementation
// =============================================================================

struct GraphWithMember {
    std::vector<std::vector<std::pair<std::size_t, int>>> data;
    bool has_edges_flag;
    
    // Custom member function that takes precedence
    bool has_edge() const {
        return has_edges_flag;
    }
    
    // Required for vertices(g) to work
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

TEST_CASE("has_edge - Custom member function returns true", "[has_edge][member]") {
    GraphWithMember g;
    g.data.resize(3);
    g.has_edges_flag = true; // Custom implementation returns true
    // Note: actual graph has no edges, but member takes precedence
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Custom member function returns false", "[has_edge][member]") {
    GraphWithMember g;
    g.data.resize(3);
    g.data[0].push_back({1, 10}); // Graph has edge
    g.has_edges_flag = false; // But custom implementation returns false
    REQUIRE_FALSE(has_edge(g));
}

// =============================================================================
// Test Custom ADL Implementation
// =============================================================================

namespace custom {
    struct Graph {
        std::vector<std::vector<std::pair<std::size_t, int>>> data;
        bool adl_result;
        
        auto begin() { return data.begin(); }
        auto end() { return data.end(); }
        auto begin() const { return data.begin(); }
        auto end() const { return data.end(); }
    };
    
    // ADL-findable function
    bool has_edge(const Graph& g) {
        return g.adl_result;
    }
}

TEST_CASE("has_edge - ADL function returns true", "[has_edge][adl]") {
    custom::Graph g;
    g.data.resize(3);
    g.adl_result = true;
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - ADL function returns false", "[has_edge][adl]") {
    custom::Graph g;
    g.data.resize(3);
    g.data[0].push_back({1, 10}); // Graph has edge
    g.adl_result = false; // But ADL returns false
    REQUIRE_FALSE(has_edge(g));
}

// =============================================================================
// Test Const Correctness
// =============================================================================

TEST_CASE("has_edge - Const graph with edges", "[has_edge][const]") {
    SimpleGraph g_mutable(3);
    g_mutable[0].push_back({1, 10});
    const SimpleGraph& g = g_mutable;
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Const graph without edges", "[has_edge][const]") {
    SimpleGraph g_mutable(3);
    const SimpleGraph& g = g_mutable;
    REQUIRE_FALSE(has_edge(g));
}

// =============================================================================
// Test Edge Cases
// =============================================================================

TEST_CASE("has_edge - Large graph with many vertices, first has edge", "[has_edge][large]") {
    SimpleGraph g(1000);
    g[0].push_back({1, 10}); // First vertex has edge
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Large graph with many vertices, last has edge", "[has_edge][large]") {
    SimpleGraph g(1000);
    g[999].push_back({998, 10}); // Only last vertex has edge
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Large graph with many vertices, none have edges", "[has_edge][large]") {
    SimpleGraph g(1000); // Many vertices, no edges
    REQUIRE_FALSE(has_edge(g));
}

TEST_CASE("has_edge - Multigraph with parallel edges", "[has_edge][multigraph]") {
    SimpleGraph g(2);
    // Multiple edges between same vertices
    g[0].push_back({1, 10});
    g[0].push_back({1, 20});
    g[0].push_back({1, 30});
    REQUIRE(has_edge(g));
}

// =============================================================================
// Test Integration with Other CPOs
// =============================================================================

TEST_CASE("has_edge - Consistent with num_edges for empty graph", "[has_edge][integration]") {
    SimpleGraph g(3);
    REQUIRE_FALSE(has_edge(g));
    REQUIRE(num_edges(g) == 0);
}

TEST_CASE("has_edge - Consistent with num_edges for graph with edges", "[has_edge][integration]") {
    SimpleGraph g(3);
    g[0].push_back({1, 10});
    g[1].push_back({2, 20});
    REQUIRE(has_edge(g));
    REQUIRE(num_edges(g) > 0);
}

TEST_CASE("has_edge - After adding edge with edges CPO", "[has_edge][integration]") {
    SimpleGraph g(3);
    REQUIRE_FALSE(has_edge(g));
    
    // Add edge
    g[0].push_back({1, 10});
    
    REQUIRE(has_edge(g));
    
    // Verify we can find it
    auto u = *vertices(g).begin(); // First vertex
    auto edge_range = edges(g, u);
    REQUIRE(!std::ranges::empty(edge_range));
}

TEST_CASE("has_edge - Verify short-circuit behavior", "[has_edge][performance]") {
    // This test verifies that has_edge stops at first vertex with edges
    // We can't directly test performance, but we can verify behavior
    SimpleGraph g(100);
    g[0].push_back({1, 10}); // First vertex has edge
    // All other vertices empty
    
    REQUIRE(has_edge(g));
    // If implementation is correct, it should stop at first vertex
}

// =============================================================================
// Test Different Graph Topologies
// =============================================================================

TEST_CASE("has_edge - Directed acyclic graph", "[has_edge][topology]") {
    SimpleGraph g(5);
    // DAG topology
    g[0].push_back({1, 10});
    g[0].push_back({2, 20});
    g[1].push_back({3, 30});
    g[2].push_back({3, 40});
    g[3].push_back({4, 50});
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Cyclic graph", "[has_edge][topology]") {
    SimpleGraph g(3);
    // Cycle: 0 -> 1 -> 2 -> 0
    g[0].push_back({1, 10});
    g[1].push_back({2, 20});
    g[2].push_back({0, 30});
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Disconnected components, some have edges", "[has_edge][topology]") {
    SimpleGraph g(6);
    // Component 1: 0-1 (has edge)
    g[0].push_back({1, 10});
    // Component 2: 2 (isolated)
    // Component 3: 3-4-5 (has edges)
    g[3].push_back({4, 20});
    g[4].push_back({5, 30});
    REQUIRE(has_edge(g));
}

TEST_CASE("has_edge - Disconnected components, none have edges", "[has_edge][topology]") {
    SimpleGraph g(5);
    // All vertices isolated, no edges
    REQUIRE_FALSE(has_edge(g));
}

TEST_CASE("has_edge - Tree structure", "[has_edge][topology]") {
    SimpleGraph g(7);
    // Tree rooted at 0
    g[0].push_back({1, 10});
    g[0].push_back({2, 20});
    g[1].push_back({3, 30});
    g[1].push_back({4, 40});
    g[2].push_back({5, 50});
    g[2].push_back({6, 60});
    REQUIRE(has_edge(g));
}
