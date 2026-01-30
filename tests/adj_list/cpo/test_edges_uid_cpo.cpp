/**
 * @file test_edges_uid_cpo.cpp
 * @brief Tests for edges(g, uid) CPO - get edges by vertex ID
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <ranges>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Default Implementation - Vector Graph
// =============================================================================

TEST_CASE("edges(g,uid) - vector graph default", "[edges][cpo][uid][vector]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},      // Vertex 0 -> edges to 1, 2
        {2, 3},      // Vertex 1 -> edges to 2, 3
        {3},         // Vertex 2 -> edge to 3
        {}           // Vertex 3 -> no edges
    };
    
    // Get edges by vertex ID using default implementation
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    auto e2 = edges(graph, 2);
    auto e3 = edges(graph, 3);
    
    REQUIRE(std::ranges::distance(e0) == 2);
    REQUIRE(std::ranges::distance(e1) == 2);
    REQUIRE(std::ranges::distance(e2) == 1);
    REQUIRE(std::ranges::distance(e3) == 0);
    
    // Verify edge target IDs
    std::vector<int> targets0, targets1, targets2;
    for (auto edge : e0) {
        targets0.push_back(target_id(graph, edge));
    }
    for (auto edge : e1) {
        targets1.push_back(target_id(graph, edge));
    }
    for (auto edge : e2) {
        targets2.push_back(target_id(graph, edge));
    }
    
    REQUIRE(targets0 == std::vector<int>{1, 2});
    REQUIRE(targets1 == std::vector<int>{2, 3});
    REQUIRE(targets2 == std::vector<int>{3});
}

// =============================================================================
// Test: Default Implementation - Deque Graph
// =============================================================================

TEST_CASE("edges(g,uid) - deque graph default", "[edges][cpo][uid][deque]") {
    std::deque<std::deque<int>> graph = {
        {10, 20},
        {30},
        {},
        {10, 30}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    auto e3 = edges(graph, 3);
    
    REQUIRE(std::ranges::distance(e0) == 2);
    REQUIRE(std::ranges::distance(e1) == 1);
    REQUIRE(std::ranges::distance(e3) == 2);
}

// =============================================================================
// Test: Default Implementation - Map Graph
// =============================================================================

TEST_CASE("edges(g,uid) - map graph default", "[edges][cpo][uid][map]") {
    std::map<int, std::vector<int>> graph = {
        {100, {200, 300}},
        {200, {300}},
        {300, {}},
        {400, {100, 200}}
    };
    
    auto e100 = edges(graph, 100);
    auto e200 = edges(graph, 200);
    auto e300 = edges(graph, 300);
    auto e400 = edges(graph, 400);
    
    REQUIRE(std::ranges::distance(e100) == 2);
    REQUIRE(std::ranges::distance(e200) == 1);
    REQUIRE(std::ranges::distance(e300) == 0);
    REQUIRE(std::ranges::distance(e400) == 2);
    
    // Verify target IDs
    std::vector<int> targets100;
    for (auto edge : e100) {
        targets100.push_back(target_id(graph, edge));
    }
    REQUIRE(targets100 == std::vector<int>{200, 300});
}

// =============================================================================
// Test: Weighted Graphs
// =============================================================================

TEST_CASE("edges(g,uid) - weighted graph with pairs", "[edges][cpo][uid][weighted]") {
    std::vector<std::vector<std::pair<int, double>>> graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 3.5}},
        {}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    
    REQUIRE(std::ranges::distance(e0) == 2);
    REQUIRE(std::ranges::distance(e1) == 1);
    
    // Verify target IDs
    std::vector<int> targets;
    for (auto edge : e0) {
        targets.push_back(target_id(graph, edge));
    }
    REQUIRE(targets == std::vector<int>{1, 2});
}

TEST_CASE("edges(g,uid) - weighted graph with tuples", "[edges][cpo][uid][tuple]") {
    std::vector<std::vector<std::tuple<int, double, std::string>>> graph = {
        {{1, 1.5, "a"}, {2, 2.5, "b"}},
        {{0, 0.5, "c"}}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    
    REQUIRE(std::ranges::distance(e0) == 2);
    REQUIRE(std::ranges::distance(e1) == 1);
}

// =============================================================================
// Test: Custom Member Function
// =============================================================================

namespace test_member {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency_list;
        
        auto& operator[](std::size_t idx) { return adjacency_list[idx]; }
        const auto& operator[](std::size_t idx) const { return adjacency_list[idx]; }
        
        auto vertices() {
            return vertex_descriptor_view(adjacency_list);
        }
        
        // edges(g,u) - takes vertex descriptor
        template<typename U>
            requires vertex_descriptor_type<U>
        auto edges(const U& u) {
            return edge_descriptor_view(u.inner_value(adjacency_list), u);
        }
        
        // Custom edges(uid) member function
        auto edges(std::size_t uid) {
            // Return edges from the specified vertex ID
            // First get the vertex descriptor, then call edges(g,u)
            auto v = *std::ranges::next(std::ranges::begin(vertices()), static_cast<std::iter_difference_t<decltype(std::ranges::begin(vertices()))>>(uid));
            return edges(v);  // Call the descriptor version
        }
    };
}

TEST_CASE("edges(g,uid) - custom member function", "[edges][cpo][uid][member]") {
    test_member::CustomGraph graph{
        {{1, 2}, {3}, {}, {0, 1}}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    auto e3 = edges(graph, 3);
    
    REQUIRE(std::ranges::distance(e0) == 2);
    REQUIRE(std::ranges::distance(e1) == 1);
    REQUIRE(std::ranges::distance(e3) == 2);
}

// =============================================================================
// Test: ADL Customization
// =============================================================================

namespace test_adl {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency_list;
        
        auto& operator[](std::size_t idx) { return adjacency_list[idx]; }
        const auto& operator[](std::size_t idx) const { return adjacency_list[idx]; }
        
        auto vertices() {
            return vertex_descriptor_view(adjacency_list);
        }
        
        // edges(g,u) - takes vertex descriptor
        template<typename U>
            requires vertex_descriptor_type<U>
        auto edges(const U& u) {
            return edge_descriptor_view(u.inner_value(adjacency_list), u);
        }
    };
    
    // ADL edges(g, uid) function
    auto edges(CustomGraph& g, std::size_t uid) {
        // First get the vertex descriptor, then call edges(g,u)
        auto v = *std::ranges::next(std::ranges::begin(g.vertices()), static_cast<std::iter_difference_t<decltype(std::ranges::begin(g.vertices()))>>(uid));
        return g.edges(v);  // Call the descriptor version
    }
}

TEST_CASE("edges(g,uid) - ADL customization", "[edges][cpo][uid][adl]") {
    test_adl::CustomGraph graph{
        {{1, 2, 3}, {4}, {}, {0}}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    auto e3 = edges(graph, 3);
    
    REQUIRE(std::ranges::distance(e0) == 3);
    REQUIRE(std::ranges::distance(e1) == 1);
    REQUIRE(std::ranges::distance(e3) == 1);
}

// =============================================================================
// Test: Integration with Other CPOs
// =============================================================================

TEST_CASE("edges(g,uid) - integration with vertex_id and target", "[edges][cpo][uid][integration]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3},
        {0}
    };
    
    // Get edges by ID, then use target to traverse
    for (std::size_t uid = 0; uid < 4; ++uid) {
        auto vertex_edges = edges(graph, uid);
        
        for (auto edge : vertex_edges) {
            auto target_vertex = target(graph, edge);
            auto target_vid = vertex_id(graph, target_vertex);
            
            // Verify target is valid
            REQUIRE(target_vid < graph.size());
        }
    }
}

TEST_CASE("edges(g,uid) - consistency with edges(g,u)", "[edges][cpo][uid][consistency]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {}
    };
    
    // Compare edges(g, uid) with edges(g, u)
    for (std::size_t uid = 0; uid < graph.size(); ++uid) {
        auto edges_by_id = edges(graph, uid);
        auto v = *find_vertex(graph, uid);
        auto edges_by_descriptor = edges(graph, v);
        
        REQUIRE(std::ranges::distance(edges_by_id) == std::ranges::distance(edges_by_descriptor));
        
        // Verify same targets
        std::vector<int> targets_by_id, targets_by_descriptor;
        for (auto e : edges_by_id) {
            targets_by_id.push_back(target_id(graph, e));
        }
        for (auto e : edges_by_descriptor) {
            targets_by_descriptor.push_back(target_id(graph, e));
        }
        
        REQUIRE(targets_by_id == targets_by_descriptor);
    }
}

// =============================================================================
// Test: Edge Cases
// =============================================================================

TEST_CASE("edges(g,uid) - vertex with no edges", "[edges][cpo][uid][empty]") {
    std::vector<std::vector<int>> graph = {
        {},
        {},
        {}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    auto e2 = edges(graph, 2);
    
    REQUIRE(std::ranges::distance(e0) == 0);
    REQUIRE(std::ranges::distance(e1) == 0);
    REQUIRE(std::ranges::distance(e2) == 0);
}

TEST_CASE("edges(g,uid) - single vertex graph", "[edges][cpo][uid][single]") {
    std::vector<std::vector<int>> graph = {
        {0}  // Self-loop
    };
    
    auto e0 = edges(graph, 0);
    REQUIRE(std::ranges::distance(e0) == 1);
    
    auto edge = *e0.begin();
    REQUIRE(target_id(graph, edge) == 0);
}

TEST_CASE("edges(g,uid) - map with sparse IDs", "[edges][cpo][uid][sparse]") {
    std::map<int, std::vector<int>> graph = {
        {10, {50, 100}},
        {50, {100}},
        {100, {10}}
    };
    
    auto e10 = edges(graph, 10);
    auto e50 = edges(graph, 50);
    auto e100 = edges(graph, 100);
    
    REQUIRE(std::ranges::distance(e10) == 2);
    REQUIRE(std::ranges::distance(e50) == 1);
    REQUIRE(std::ranges::distance(e100) == 1);
}

// =============================================================================
// Test: Return Type and Properties
// =============================================================================

TEST_CASE("edges(g,uid) - return type is edge_descriptor_view", "[edges][cpo][uid][type]") {
    std::vector<std::vector<int>> graph = {{1, 2}, {3}, {}};
    
    auto result = edges(graph, 0);
    
    // Verify return type
    using ResultType = decltype(result);
    REQUIRE(is_edge_descriptor_view_v<ResultType>);
    
    // Verify it's a forward range
    REQUIRE(std::ranges::forward_range<ResultType>);
}

TEST_CASE("edges(g,uid) - works with const graph", "[edges][cpo][uid][const]") {
    const std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3},
        {}
    };
    
    auto e0 = edges(graph, 0);
    auto e1 = edges(graph, 1);
    
    REQUIRE(std::ranges::distance(e0) == 2);
    REQUIRE(std::ranges::distance(e1) == 2);
}

// =============================================================================
// Test: Performance Characteristics
// =============================================================================

TEST_CASE("edges(g,uid) - vector O(1) access", "[edges][cpo][uid][performance]") {
    std::vector<std::vector<int>> graph(1000);
    for (std::size_t i = 0; i < graph.size(); ++i) {
        graph[i] = {static_cast<int>((i + 1) % graph.size())};
    }
    
    // Should be fast even for large indices (O(1) access)
    auto e500 = edges(graph, 500);
    auto e999 = edges(graph, 999);
    
    REQUIRE(std::ranges::distance(e500) == 1);
    REQUIRE(std::ranges::distance(e999) == 1);
}

TEST_CASE("edges(g,uid) - map O(log n) access", "[edges][cpo][uid][map_performance]") {
    std::map<int, std::vector<int>> graph;
    for (int i = 0; i < 100; ++i) {
        graph[i * 10] = {((i + 1) % 100) * 10};
    }
    
    // Should work with sparse keys
    auto e0 = edges(graph, 0);
    auto e500 = edges(graph, 500);
    auto e990 = edges(graph, 990);
    
    REQUIRE(std::ranges::distance(e0) == 1);
    REQUIRE(std::ranges::distance(e500) == 1);
    REQUIRE(std::ranges::distance(e990) == 1);
}

// =============================================================================
// Test: Complete Graph Traversal
// =============================================================================

TEST_CASE("edges(g,uid) - full graph traversal by ID", "[edges][cpo][uid][traversal]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3},
        {0}
    };
    
    std::vector<std::pair<int, int>> edge_list;
    
    for (std::size_t uid = 0; uid < graph.size(); ++uid) {
        for (auto edge : edges(graph, uid)) {
            auto tid = target_id(graph, edge);
            edge_list.emplace_back(static_cast<int>(uid), tid);
        }
    }
    
    REQUIRE(edge_list.size() == 6);
    REQUIRE(edge_list[0] == std::make_pair(0, 1));
    REQUIRE(edge_list[1] == std::make_pair(0, 2));
    REQUIRE(edge_list[2] == std::make_pair(1, 2));
    REQUIRE(edge_list[3] == std::make_pair(1, 3));
    REQUIRE(edge_list[4] == std::make_pair(2, 3));
    REQUIRE(edge_list[5] == std::make_pair(3, 0));
}

TEST_CASE("edges(g,uid) - directed acyclic graph", "[edges][cpo][uid][dag]") {
    std::vector<std::vector<int>> dag = {
        {1, 2},     // 0 -> 1, 2
        {3},        // 1 -> 3
        {3},        // 2 -> 3
        {}          // 3 -> (none)
    };
    
    // Verify topological structure
    REQUIRE(std::ranges::distance(edges(dag, 0)) == 2);
    REQUIRE(std::ranges::distance(edges(dag, 1)) == 1);
    REQUIRE(std::ranges::distance(edges(dag, 2)) == 1);
    REQUIRE(std::ranges::distance(edges(dag, 3)) == 0);
}

TEST_CASE("edges(g,uid) - complete graph K4", "[edges][cpo][uid][complete]") {
    std::vector<std::vector<int>> k4 = {
        {1, 2, 3},
        {0, 2, 3},
        {0, 1, 3},
        {0, 1, 2}
    };
    
    // Every vertex has 3 edges
    for (std::size_t uid = 0; uid < 4; ++uid) {
        REQUIRE(std::ranges::distance(edges(k4, uid)) == 3);
    }
}
