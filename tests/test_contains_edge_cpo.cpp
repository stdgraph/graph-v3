/**
 * @file test_contains_edge_cpo.cpp
 * @brief Comprehensive tests for contains_edge(g,u,v) and contains_edge(g,uid,vid) CPOs
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/detail/graph_cpo.hpp>
#include <vector>
#include <map>
#include <utility>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test graphs with custom contains_edge member
// =============================================================================

struct GraphWithContainsEdgeMember {
    std::vector<std::vector<int>> adj_list;
    
    GraphWithContainsEdgeMember(size_t n) : adj_list(n) {}
    
    void add_edge(size_t from, size_t to) {
        adj_list[from].push_back(static_cast<int>(to));
    }
    
    // Custom member: always returns true for testing
    bool contains_edge(size_t u, size_t v) const {
        // Custom implementation - for testing, just check if v is in adj_list[u]
        for (auto targ : adj_list[u]) {
            if (static_cast<size_t>(targ) == v) {
                return true;
            }
        }
        return false;
    }
};

// =============================================================================
// Test graphs with ADL contains_edge
// =============================================================================

namespace test_adl {
    struct GraphWithADLContainsEdge {
        std::vector<std::vector<int>> adj_list;
        
        GraphWithADLContainsEdge(size_t n) : adj_list(n) {}
        
        void add_edge(size_t from, size_t to) {
            adj_list[from].push_back(static_cast<int>(to));
        }
    };

    // ADL contains_edge function
    inline bool contains_edge(const GraphWithADLContainsEdge& g, size_t u, size_t v) {
        for (auto targ : g.adj_list[u]) {
            if (static_cast<size_t>(targ) == v) {
                return true;
            }
        }
        return false;
    }

    inline bool contains_edge(GraphWithADLContainsEdge& g, size_t u, size_t v) {
        for (auto targ : g.adj_list[u]) {
            if (static_cast<size_t>(targ) == v) {
                return true;
            }
        }
        return false;
    }
}

// =============================================================================
// Tests: Default implementation with contains_edge(g, u, v)
// =============================================================================

TEST_CASE("contains_edge(g, u, v) detects existing edges", "[contains_edge][cpo][vector][uv]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},    // vertex 0 -> 1, 2, 3
        {2, 3},       // vertex 1 -> 2, 3
        {3},          // vertex 2 -> 3
        {}            // vertex 3 -> no edges
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it++;
    auto v3 = *it;
    
    SECTION("Existing edges return true") {
        REQUIRE(contains_edge(graph, v0, v1) == true);
        REQUIRE(contains_edge(graph, v0, v2) == true);
        REQUIRE(contains_edge(graph, v0, v3) == true);
        REQUIRE(contains_edge(graph, v1, v2) == true);
        REQUIRE(contains_edge(graph, v1, v3) == true);
        REQUIRE(contains_edge(graph, v2, v3) == true);
    }
    
    SECTION("Non-existing edges return false") {
        REQUIRE(contains_edge(graph, v0, v0) == false);
        REQUIRE(contains_edge(graph, v1, v0) == false);
        REQUIRE(contains_edge(graph, v1, v1) == false);
        REQUIRE(contains_edge(graph, v2, v0) == false);
        REQUIRE(contains_edge(graph, v2, v1) == false);
        REQUIRE(contains_edge(graph, v2, v2) == false);
        REQUIRE(contains_edge(graph, v3, v0) == false);
        REQUIRE(contains_edge(graph, v3, v1) == false);
        REQUIRE(contains_edge(graph, v3, v2) == false);
        REQUIRE(contains_edge(graph, v3, v3) == false);
    }
}

TEST_CASE("contains_edge(g, u, v) works with weighted edges", "[contains_edge][cpo][weighted][uv]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 1.2}},
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it++;
    auto v3 = *it;
    
    REQUIRE(contains_edge(graph, v0, v1) == true);
    REQUIRE(contains_edge(graph, v0, v2) == true);
    REQUIRE(contains_edge(graph, v1, v2) == true);
    REQUIRE(contains_edge(graph, v0, v3) == false);
    REQUIRE(contains_edge(graph, v1, v0) == false);
    REQUIRE(contains_edge(graph, v2, v0) == false);
}

TEST_CASE("contains_edge(g, u, v) handles empty edge lists", "[contains_edge][cpo][empty][uv]") {
    std::vector<std::vector<int>> graph = {
        {},  // No edges
        {},
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it;
    
    REQUIRE(contains_edge(graph, v0, v1) == false);
    REQUIRE(contains_edge(graph, v0, v2) == false);
    REQUIRE(contains_edge(graph, v1, v2) == false);
    REQUIRE(contains_edge(graph, v0, v0) == false);
}

// =============================================================================
// Tests: Default implementation with contains_edge(g, uid, vid)
// =============================================================================

TEST_CASE("contains_edge(g, uid, vid) detects existing edges by ID", "[contains_edge][cpo][vector][uidvid]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {}
    };
    
    SECTION("Existing edges return true") {
        REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(1)) == true);
        REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(2)) == true);
        REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(3)) == true);
        REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(2)) == true);
        REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(3)) == true);
        REQUIRE(contains_edge(graph, static_cast<size_t>(2), static_cast<size_t>(3)) == true);
    }
    
    SECTION("Non-existing edges return false") {
        REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(0)) == false);
        REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(0)) == false);
        REQUIRE(contains_edge(graph, static_cast<size_t>(2), static_cast<size_t>(0)) == false);
        REQUIRE(contains_edge(graph, static_cast<size_t>(3), static_cast<size_t>(0)) == false);
        REQUIRE(contains_edge(graph, static_cast<size_t>(3), static_cast<size_t>(1)) == false);
    }
}

TEST_CASE("contains_edge(g, uid, vid) works with weighted edges", "[contains_edge][cpo][weighted][uidvid]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 10.5}, {2, 20.5}},
        {{3, 30.5}},
        {},
        {}
    };
    
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(1)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(2)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(3)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(3)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(2), static_cast<size_t>(0)) == false);
}

// =============================================================================
// Tests: Custom member implementation
// =============================================================================

TEST_CASE("contains_edge(g, u, v) uses custom member function", "[contains_edge][cpo][custom][member]") {
    GraphWithContainsEdgeMember graph(4);
    graph.add_edge(0, 1);
    graph.add_edge(0, 2);
    graph.add_edge(1, 3);
    
    // Should use custom member
    REQUIRE(contains_edge(graph, 0, 1) == true);
    REQUIRE(contains_edge(graph, 0, 2) == true);
    REQUIRE(contains_edge(graph, 1, 3) == true);
    REQUIRE(contains_edge(graph, 0, 3) == false);
    REQUIRE(contains_edge(graph, 2, 0) == false);
}

// =============================================================================
// Tests: ADL implementation
// =============================================================================

TEST_CASE("contains_edge(g, u, v) uses ADL when available", "[contains_edge][cpo][adl]") {
    test_adl::GraphWithADLContainsEdge graph(4);
    graph.add_edge(0, 1);
    graph.add_edge(0, 2);
    graph.add_edge(1, 3);
    
    // Should use ADL function
    REQUIRE(contains_edge(graph, 0, 1) == true);
    REQUIRE(contains_edge(graph, 0, 2) == true);
    REQUIRE(contains_edge(graph, 1, 3) == true);
    REQUIRE(contains_edge(graph, 0, 3) == false);
    REQUIRE(contains_edge(graph, 2, 0) == false);
}

// =============================================================================
// Tests: Const correctness
// =============================================================================

TEST_CASE("contains_edge works with const graph", "[contains_edge][cpo][const]") {
    const std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {}
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it;
    
    REQUIRE(contains_edge(graph, v0, v1) == true);
    REQUIRE(contains_edge(graph, v0, v2) == true);
    REQUIRE(contains_edge(graph, v1, v2) == true);
    REQUIRE(contains_edge(graph, v2, v0) == false);
}

// =============================================================================
// Tests: Self-loops
// =============================================================================

TEST_CASE("contains_edge detects self-loops", "[contains_edge][cpo][self_loop]") {
    std::vector<std::vector<int>> graph = {
        {0, 1, 2},  // Self-loop at 0
        {1, 2},     // Self-loop at 1
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it;
    
    REQUIRE(contains_edge(graph, v0, v0) == true);
    REQUIRE(contains_edge(graph, v1, v1) == true);
    REQUIRE(contains_edge(graph, v2, v2) == false);
}

// =============================================================================
// Tests: Multiple edges to same target
// =============================================================================

TEST_CASE("contains_edge returns true for multigraph edges", "[contains_edge][cpo][multigraph]") {
    // Graph with multiple edges from 0 to 2
    std::vector<std::vector<int>> graph = {
        {1, 2, 2, 3},  // Two edges to 2
        {},
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    auto v2 = *std::next(verts.begin(), 2);
    
    // Should still return true (edge exists)
    REQUIRE(contains_edge(graph, v0, v2) == true);
}

// =============================================================================
// Tests: Integration with find_vertex_edge
// =============================================================================

TEST_CASE("contains_edge consistent with find_vertex_edge", "[contains_edge][cpo][integration]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it;
    
    SECTION("When contains_edge is true, find_vertex_edge finds the edge") {
        if (contains_edge(graph, v0, v1)) {
            auto e = find_vertex_edge(graph, v0, v1);
            REQUIRE(target_id(graph, e) == 1);
        }
        
        if (contains_edge(graph, v0, v2)) {
            auto e = find_vertex_edge(graph, v0, v2);
            REQUIRE(target_id(graph, e) == 2);
        }
    }
    
    SECTION("When contains_edge is false, edge doesn't exist") {
        REQUIRE(contains_edge(graph, v2, v0) == false);
        REQUIRE(contains_edge(graph, v2, v1) == false);
    }
}

// =============================================================================
// Tests: Different graph topologies
// =============================================================================

TEST_CASE("contains_edge works with complete graph", "[contains_edge][cpo][topology][complete]") {
    // Complete graph K4
    std::vector<std::vector<int>> graph(4);
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            if (i != j) {
                graph[i].push_back(static_cast<int>(j));
            }
        }
    }
    
    auto verts = vertices(graph);
    
    // Every pair of distinct vertices should have an edge
    int count = 0;
    for (auto u : verts) {
        for (auto v : verts) {
            if (vertex_id(graph, u) != vertex_id(graph, v)) {
                REQUIRE(contains_edge(graph, u, v) == true);
                count++;
            } else {
                REQUIRE(contains_edge(graph, u, v) == false);
            }
        }
    }
    REQUIRE(count == 12); // K4 has 4*3 = 12 directed edges
}

TEST_CASE("contains_edge works with DAG", "[contains_edge][cpo][topology][dag]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},     // 0 -> 1, 2
        {3},        // 1 -> 3
        {3},        // 2 -> 3
        {}          // 3
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    auto v2 = *it++;
    auto v3 = *it;
    
    // Forward edges exist
    REQUIRE(contains_edge(graph, v0, v1) == true);
    REQUIRE(contains_edge(graph, v0, v2) == true);
    REQUIRE(contains_edge(graph, v1, v3) == true);
    REQUIRE(contains_edge(graph, v2, v3) == true);
    
    // Backward edges don't exist (DAG property)
    REQUIRE(contains_edge(graph, v1, v0) == false);
    REQUIRE(contains_edge(graph, v2, v0) == false);
    REQUIRE(contains_edge(graph, v3, v0) == false);
    REQUIRE(contains_edge(graph, v3, v1) == false);
    REQUIRE(contains_edge(graph, v3, v2) == false);
}

TEST_CASE("contains_edge works with path graph", "[contains_edge][cpo][topology][path]") {
    std::vector<std::vector<int>> graph = {
        {1},    // 0 -> 1
        {2},    // 1 -> 2
        {3},    // 2 -> 3
        {}      // 3
    };
    
    // Only consecutive vertices have edges
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(1)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(2)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(2), static_cast<size_t>(3)) == true);
    
    // No shortcuts
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(2)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(3)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(3)) == false);
}

TEST_CASE("contains_edge works with star graph", "[contains_edge][cpo][topology][star]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3, 4},  // Center vertex 0 connects to all others
        {},
        {},
        {},
        {}
    };
    
    // Center to periphery edges exist
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(1)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(2)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(3)) == true);
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(4)) == true);
    
    // Periphery to periphery edges don't exist
    REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(2)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(2), static_cast<size_t>(3)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(3), static_cast<size_t>(4)) == false);
    
    // Periphery to center edges don't exist
    REQUIRE(contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(0)) == false);
}

// =============================================================================
// Tests: Return type
// =============================================================================

TEST_CASE("contains_edge returns bool", "[contains_edge][cpo][return_type]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    auto v1 = *std::next(verts.begin());
    
    auto result = contains_edge(graph, v0, v1);
    REQUIRE(std::is_same_v<decltype(result), bool>);
    REQUIRE(result == true);
}

// =============================================================================
// Tests: Overload resolution
// =============================================================================

TEST_CASE("contains_edge overloads resolve correctly", "[contains_edge][cpo][overload]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2},
        {},
        {}
    };
    
    auto verts = vertices(graph);
    auto it = verts.begin();
    auto v0 = *it++;
    auto v1 = *it++;
    [[maybe_unused]] auto v2 = *it;
    
    // Test (u, v) overload - both descriptors
    bool result1 = contains_edge(graph, v0, v1);
    REQUIRE(result1 == true);
    
    // Test (uid, vid) overload - both IDs
    bool result2 = contains_edge(graph, static_cast<size_t>(1), static_cast<size_t>(2));
    REQUIRE(result2 == true);
    
    // Test non-existing edge
    bool result3 = contains_edge(graph, static_cast<size_t>(2), static_cast<size_t>(0));
    REQUIRE(result3 == false);
}

// =============================================================================
// Tests: Large graph
// =============================================================================

TEST_CASE("contains_edge works with larger graphs", "[contains_edge][cpo][large]") {
    std::vector<std::vector<int>> graph(100);
    
    // Create a chain: 0->1->2->...->99
    for (size_t i = 0; i < 99; ++i) {
        graph[i].push_back(static_cast<int>(i + 1));
    }
    
    // Check consecutive edges exist
    for (size_t i = 0; i < 99; ++i) {
        REQUIRE(contains_edge(graph, i, i + 1) == true);
    }
    
    // Check some non-edges
    REQUIRE(contains_edge(graph, static_cast<size_t>(0), static_cast<size_t>(50)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(50), static_cast<size_t>(0)) == false);
    REQUIRE(contains_edge(graph, static_cast<size_t>(99), static_cast<size_t>(0)) == false);
}

// =============================================================================
// Tests: Empty graph
// =============================================================================

TEST_CASE("contains_edge works with empty graph", "[contains_edge][cpo][empty_graph]") {
    std::vector<std::vector<int>> graph; // No vertices
    
    // Can't test with descriptors since there are no vertices
    // But the function should be callable
    REQUIRE(graph.empty());
}

// =============================================================================
// Tests: Single vertex
// =============================================================================

TEST_CASE("contains_edge works with single vertex", "[contains_edge][cpo][single_vertex]") {
    std::vector<std::vector<int>> graph = {
        {} // Single vertex with no edges
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    
    REQUIRE(contains_edge(graph, v0, v0) == false);
}

TEST_CASE("contains_edge works with single vertex with self-loop", "[contains_edge][cpo][single_vertex_loop]") {
    std::vector<std::vector<int>> graph = {
        {0} // Single vertex with self-loop
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    
    REQUIRE(contains_edge(graph, v0, v0) == true);
}
