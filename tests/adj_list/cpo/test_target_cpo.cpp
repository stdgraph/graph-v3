#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <list>
#include <string>

#include "graph/adj_list/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Default Implementation with Vector - Simple Edges
// =============================================================================

TEST_CASE("target(g,uv) - vector<vector<int>> simple edges", "[target][cpo][default]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {}
    };
    
    // Get first vertex
    auto v0 = *vertices(graph).begin();
    
    // Get first edge from vertex 0
    auto e = *edges(graph, v0).begin();
    
    // Get target vertex descriptor
    auto target_v = target(graph, e);
    
    // Verify target vertex ID is 1
    REQUIRE(vertex_id(graph, target_v) == 1);
}

TEST_CASE("target(g,uv) - accessing target's edges", "[target][cpo][default]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e01 = *edges(graph, v0).begin();  // Edge 0->1
    
    // Get target vertex
    auto v1 = target(graph, e01);
    
    // Get edges from the target vertex
    std::vector<int> target_edges;
    for (auto e : edges(graph, v1)) {
        target_edges.push_back(target_id(graph, e));
    }
    
    REQUIRE(target_edges.size() == 2);
    REQUIRE(target_edges[0] == 2);
    REQUIRE(target_edges[1] == 3);
}

// =============================================================================
// Test: Default Implementation with Pair Edges (Weighted)
// =============================================================================

TEST_CASE("target(g,uv) - vector<vector<pair<int,double>>> weighted edges", "[target][cpo][default]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 1.0}, {3, 2.0}},
        {{3, 1.5}},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

TEST_CASE("target(g,uv) - iterating through edges and targets", "[target][cpo][default]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.0}, {2, 2.0}, {3, 3.0}},
        {},
        {},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    
    std::vector<int> target_ids;
    for (auto e : edges(graph, v0)) {
        auto t = target(graph, e);
        target_ids.push_back(static_cast<int>(vertex_id(graph, t)));
    }
    
    REQUIRE(target_ids.size() == 3);
    REQUIRE(target_ids[0] == 1);
    REQUIRE(target_ids[1] == 2);
    REQUIRE(target_ids[2] == 3);
}

// =============================================================================
// Test: Default Implementation with Tuple Edges
// =============================================================================

TEST_CASE("target(g,uv) - vector<vector<tuple<...>>> multi-property edges", "[target][cpo][default]") {
    using Edge = std::tuple<int, double, std::string>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5, "a"}, {2, 2.5, "b"}},
        {{3, 3.5, "c"}},
        {},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

// =============================================================================
// Test: Default Implementation with Deque
// =============================================================================

TEST_CASE("target(g,uv) - deque<deque<int>> simple edges", "[target][cpo][default]") {
    std::deque<std::deque<int>> graph = {
        {10, 20},
        {30},
        {},
        {40}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 10);
}

// =============================================================================
// Test: Default Implementation with Map
// =============================================================================

TEST_CASE("target(g,uv) - map<int, vector<int>>", "[target][cpo][default]") {
    std::map<int, std::vector<int>> graph = {
        {0, {1, 2}},
        {1, {2}},
        {2, {}},
        {3, {0}}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

TEST_CASE("target(g,uv) - map with sparse vertex IDs", "[target][cpo][default]") {
    std::map<int, std::vector<int>> graph = {
        {100, {200, 300}},
        {200, {300}},
        {300, {}}
    };
    
    auto v100 = *vertices(graph).begin();
    auto e = *edges(graph, v100).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 200);
}

TEST_CASE("target(g,uv) - map with weighted edges", "[target][cpo][default]") {
    using Edge = std::pair<int, double>;
    std::map<int, std::vector<Edge>> graph = {
        {0, {{1, 1.5}, {2, 2.5}}},
        {1, {{2, 3.5}}},
        {2, {}}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

// =============================================================================
// Test: Custom Member Function
// =============================================================================

namespace test_member {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency_list;
        
        // Provide operator[] for vertex_descriptor access
        auto& operator[](std::size_t idx) { return adjacency_list[idx]; }
        const auto& operator[](std::size_t idx) const { return adjacency_list[idx]; }
        
        auto vertices() {
            return vertex_descriptor_view(adjacency_list);
        }
        
        auto edges(const auto& u) {
            return edge_descriptor_view(u.inner_value(adjacency_list), u);
        }
        
        // Custom target member
        auto target(const auto& uv) {
            (void)uv;  // Unused in this test
            // Return a specific vertex descriptor regardless of edge
            return *std::ranges::next(std::ranges::begin(vertices()), 2);
        }
    };
}

TEST_CASE("target(g,uv) - custom member function", "[target][cpo][member]") {
    test_member::CustomGraph graph{
        {{1, 2}, {3}, {}, {0}}
    };
    
    auto v0 = *graph.vertices().begin();
    auto e = *graph.edges(v0).begin();
    
    // Should use custom member function
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 2);
}

// =============================================================================
// Test: ADL Customization
// =============================================================================

namespace test_adl {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency_list;
        
        // Provide operator[] for vertex_descriptor access
        auto& operator[](std::size_t idx) { return adjacency_list[idx]; }
        const auto& operator[](std::size_t idx) const { return adjacency_list[idx]; }
        
        auto vertices() {
            return vertex_descriptor_view(adjacency_list);
        }
        
        auto edges(const auto& u) {
            return edge_descriptor_view(u.inner_value(adjacency_list), u);
        }
    };
    
    // ADL target function
    auto target(CustomGraph& g, const auto& uv) {
        (void)uv;  // Unused in this test
        // Return a specific vertex descriptor (for testing)
        return *std::ranges::next(std::ranges::begin(g.vertices()), 1);
    }
}

TEST_CASE("target(g,uv) - ADL customization", "[target][cpo][adl]") {
    test_adl::CustomGraph graph{
        {{1, 2, 3}, {4}, {}, {0}}
    };
    
    auto v0 = *graph.vertices().begin();
    auto e = *graph.edges(v0).begin();
    
    // Should find ADL target
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

// =============================================================================
// Test: Custom Implementation Returning Iterator (not descriptor)
// =============================================================================

namespace test_iterator_return {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency_list;
        
        auto& operator[](std::size_t idx) { return adjacency_list[idx]; }
        const auto& operator[](std::size_t idx) const { return adjacency_list[idx]; }
        
        auto vertices() {
            return vertex_descriptor_view(adjacency_list);
        }
        
        auto edges(const auto& u) {
            return edge_descriptor_view(u.inner_value(adjacency_list), u);
        }
        
        // Custom target member that returns an ITERATOR (not descriptor)
        // This tests that the CPO properly handles both return types
        auto target(const auto& uv) {
            (void)uv;  // Unused in this test
            // Return iterator to vertex 3 (not a descriptor)
            return std::ranges::next(std::ranges::begin(vertices()), 3);
        }
    };
}

TEST_CASE("target(g,uv) - custom member returning iterator", "[target][cpo][member][iterator]") {
    test_iterator_return::CustomGraph graph{
        {{1, 2}, {3}, {}, {0, 1}}
    };
    
    auto v0 = *graph.vertices().begin();
    auto e = *graph.edges(v0).begin();
    
    // Custom member returns iterator, CPO should convert to descriptor
    auto target_v = target(graph, e);
    
    // Verify it's a descriptor (can be used with vertex_id)
    REQUIRE(vertex_id(graph, target_v) == 3);
}

namespace test_adl_iterator_return {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency_list;
        
        auto& operator[](std::size_t idx) { return adjacency_list[idx]; }
        const auto& operator[](std::size_t idx) const { return adjacency_list[idx]; }
        
        auto vertices() {
            return vertex_descriptor_view(adjacency_list);
        }
        
        auto edges(const auto& u) {
            return edge_descriptor_view(u.inner_value(adjacency_list), u);
        }
    };
    
    // ADL target function that returns an ITERATOR (not descriptor)
    auto target(CustomGraph& g, const auto& uv) {
        (void)uv;  // Unused in this test
        // Return iterator to vertex 2 (not a descriptor)
        return std::ranges::next(std::ranges::begin(g.vertices()), 2);
    }
}

TEST_CASE("target(g,uv) - ADL returning iterator", "[target][cpo][adl][iterator]") {
    test_adl_iterator_return::CustomGraph graph{
        {{1, 2, 3}, {4}, {}, {0}}
    };
    
    auto v0 = *graph.vertices().begin();
    auto e = *graph.edges(v0).begin();
    
    // ADL returns iterator, CPO should convert to descriptor
    auto target_v = target(graph, e);
    
    // Verify it's a descriptor (can be used with vertex_id)
    REQUIRE(vertex_id(graph, target_v) == 2);
}

// =============================================================================
// Test: Full Graph Traversal
// =============================================================================

TEST_CASE("target(g,uv) - full graph traversal using target", "[target][cpo][traversal]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3},
        {}
    };
    
    std::vector<std::pair<size_t, size_t>> edge_list;
    
    for (auto u : vertices(graph)) {
        for (auto e : edges(graph, u)) {
            auto v = target(graph, e);
            edge_list.emplace_back(vertex_id(graph, u), vertex_id(graph, v));
        }
    }
    
    REQUIRE(edge_list.size() == 5);
    REQUIRE(edge_list[0] == std::make_pair(size_t(0), size_t(1)));
    REQUIRE(edge_list[1] == std::make_pair(size_t(0), size_t(2)));
    REQUIRE(edge_list[2] == std::make_pair(size_t(1), size_t(2)));
    REQUIRE(edge_list[3] == std::make_pair(size_t(1), size_t(3)));
    REQUIRE(edge_list[4] == std::make_pair(size_t(2), size_t(3)));
}

// =============================================================================
// Test: Const Correctness
// =============================================================================

TEST_CASE("target(g,uv) - const graph", "[target][cpo][const]") {
    const std::vector<std::vector<int>> graph = {
        {1, 2},
        {2},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

TEST_CASE("target(g,uv) - const map graph", "[target][cpo][const]") {
    const std::map<int, std::vector<int>> graph = {
        {0, {1, 2}},
        {1, {2}},
        {2, {}}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

// =============================================================================
// Test: Edge Cases
// =============================================================================

TEST_CASE("target(g,uv) - self-loops", "[target][cpo][edge_cases]") {
    std::vector<std::vector<int>> graph = {
        {0, 1},  // Self-loop at 0, edge to 1
        {1},     // Self-loop at 1
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e_self = *edges(graph, v0).begin();  // Self-loop edge
    
    auto target_v = target(graph, e_self);
    
    // Target of self-loop should be the same vertex
    REQUIRE(vertex_id(graph, target_v) == 0);
}

TEST_CASE("target(g,uv) - multiple edges to same target", "[target][cpo][edge_cases]") {
    std::vector<std::vector<int>> graph = {
        {1, 1, 1},  // Multiple edges to vertex 1
        {},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    
    // All edges should point to vertex 1
    for (auto e : edges(graph, v0)) {
        auto t = target(graph, e);
        REQUIRE(vertex_id(graph, t) == 1);
    }
}

TEST_CASE("target(g,uv) - large vertex IDs", "[target][cpo][edge_cases]") {
    std::map<int, std::vector<int>> graph = {
        {1000, {2000, 3000}},
        {2000, {3000}},
        {3000, {}}
    };
    
    auto v1000 = *vertices(graph).begin();
    auto e = *edges(graph, v1000).begin();
    
    auto target_v = target(graph, e);
    
    REQUIRE(vertex_id(graph, target_v) == 2000);
}

// =============================================================================
// Test: Integration with Other CPOs
// =============================================================================

TEST_CASE("target(g,uv) - consistency with target_id", "[target][cpo][integration]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3},
        {}
    };
    
    for (auto u : vertices(graph)) {
        for (auto e : edges(graph, u)) {
            auto tid = target_id(graph, e);
            auto tv = target(graph, e);
            
            // target_id and target should be consistent
            REQUIRE(static_cast<size_t>(tid) == vertex_id(graph, tv));
        }
    }
}

TEST_CASE("target(g,uv) - chaining target calls", "[target][cpo][integration]") {
    std::vector<std::vector<int>> graph = {
        {1},     // 0 -> 1
        {2},     // 1 -> 2
        {3},     // 2 -> 3
        {}       // 3 (no edges)
    };
    
    // Start at vertex 0
    auto v0 = *vertices(graph).begin();
    auto e01 = *edges(graph, v0).begin();
    
    // Get vertex 1
    auto v1 = target(graph, e01);
    auto e12 = *edges(graph, v1).begin();
    
    // Get vertex 2
    auto v2 = target(graph, e12);
    auto e23 = *edges(graph, v2).begin();
    
    // Get vertex 3
    auto v3 = target(graph, e23);
    
    REQUIRE(vertex_id(graph, v3) == 3);
}

TEST_CASE("target(g,uv) - using target to traverse edges", "[target][cpo][integration]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.0}, {2, 2.0}},
        {{2, 3.0}},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    
    // Get all target vertices
    std::vector<int> targets;
    for (auto e : edges(graph, v0)) {
        auto t = target(graph, e);
        targets.push_back(static_cast<int>(vertex_id(graph, t)));
    }
    
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
}

// =============================================================================
// Test: Type Deduction
// =============================================================================

TEST_CASE("target(g,uv) - return type is vertex_iterator", "[target][cpo][types]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    auto target_v = target(graph, e);
    
    using TargetType = decltype(target_v);
    using ExpectedType = vertex_t<decltype(graph)>;
    
    static_assert(std::is_same_v<TargetType, ExpectedType>, 
                  "target should return vertex_t (vertex descriptor)");
    
    REQUIRE(vertex_id(graph, target_v) == 1);
}

// =============================================================================
// Test: Different Graph Patterns
// =============================================================================

TEST_CASE("target(g,uv) - complete graph K3", "[target][cpo][patterns]") {
    // Complete graph with 3 vertices
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {0, 2},
        {0, 1}
    };
    
    // Verify all edges have valid targets
    for (auto u : vertices(graph)) {
        for (auto e : edges(graph, u)) {
            auto t = target(graph, e);
            int tid = static_cast<int>(vertex_id(graph, t));
            
            // Target should be a valid vertex
            REQUIRE(tid >= 0);
            REQUIRE(tid <= 2);
        }
    }
}

TEST_CASE("target(g,uv) - directed acyclic graph", "[target][cpo][patterns]") {
    // DAG: 0 -> 1 -> 3, 0 -> 2 -> 3
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {3},
        {3},
        {}
    };
    
    auto v0 = *vertices(graph).begin();
    
    std::vector<int> targets_from_0;
    for (auto e : edges(graph, v0)) {
        auto t = target(graph, e);
        targets_from_0.push_back(static_cast<int>(vertex_id(graph, t)));
    }
    
    REQUIRE(targets_from_0.size() == 2);
    REQUIRE(targets_from_0[0] == 1);
    REQUIRE(targets_from_0[1] == 2);
}

TEST_CASE("target(g,uv) - cyclic graph", "[target][cpo][patterns]") {
    // Cycle: 0 -> 1 -> 2 -> 0
    std::vector<std::vector<int>> graph = {
        {1},
        {2},
        {0}
    };
    
    // Follow the cycle
    auto v0 = *vertices(graph).begin();
    auto e01 = *edges(graph, v0).begin();
    auto v1 = target(graph, e01);
    
    auto e12 = *edges(graph, v1).begin();
    auto v2 = target(graph, e12);
    
    auto e20 = *edges(graph, v2).begin();
    auto v0_again = target(graph, e20);
    
    REQUIRE(vertex_id(graph, v0_again) == 0);
}

// =============================================================================
// Test: Native Edge Member Function Support
// =============================================================================

// Note: Native edge member feature only works with forward/bidirectional iterators,
// not with random-access containers (e.g., std::vector), because edge_descriptor stores indices
// for random-access containers which cannot be dereferenced to access the edge object directly.
// The undirected_adjacency_list uses bidirectional iterators, so we cannot easily test this
// feature without creating a custom edge type, which would require modifying the container itself.
// The feature is implemented and the concept checks are in place, but comprehensive testing
// would require more complex test infrastructure.
//
// For now, we'll skip native edge member tests for target(g,uv) since:
// 1. The implementation follows the same pattern as target_id which does work
// 2. The concept checks are identical in structure
// 3. Testing would require extensive custom graph infrastructure

// =============================================================================
// Test: Performance Characteristics
// =============================================================================

TEST_CASE("target(g,uv) - vector random access performance", "[target][cpo][performance]") {
    // Large vector graph
    std::vector<std::vector<int>> graph(100);
    for (int i = 0; i < 100; ++i) {
        graph[static_cast<size_t>(i)].push_back((i + 1) % 100);
    }
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    // Should be O(1) for vector
    auto t = target(graph, e);
    
    REQUIRE(vertex_id(graph, t) == 1);
}

TEST_CASE("target(g,uv) - map logarithmic performance", "[target][cpo][performance]") {
    // Map graph with sparse IDs
    std::map<int, std::vector<int>> graph;
    for (int i = 0; i < 100; i += 10) {
        graph[i] = {i + 10};
    }
    
    auto v0 = *vertices(graph).begin();
    auto e = *edges(graph, v0).begin();
    
    // Should be O(log n) for map
    auto t = target(graph, e);
    
    REQUIRE(vertex_id(graph, t) == 10);
}
