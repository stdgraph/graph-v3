/**
 * @file test_edges_cpo.cpp
 * @brief Comprehensive tests for edges(g, u) CPO
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/adj_list/vertex_descriptor_view.hpp>
#include <graph/adj_list/edge_descriptor_view.hpp>
#include <vector>
#include <deque>
#include <map>
#include <list>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Default Implementation - Simple Edge Pattern (vector<int>)
// =============================================================================

TEST_CASE("edges(g,u) - vector<vector<int>> simple edges", "[edges][cpo][default]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},    // vertex 0 -> edges to 1, 2, 3
        {2, 3},       // vertex 1 -> edges to 2, 3
        {3},          // vertex 2 -> edge to 3
        {}            // vertex 3 -> no edges
    };
    
    SECTION("Get edges from vertex 0") {
        auto verts = vertices(graph);
        auto v0 = *verts.begin();
        
        auto edge_range = edges(graph, v0);
        
        // Check type
        REQUIRE(std::ranges::forward_range<decltype(edge_range)>);
        
        // Collect target IDs
        std::vector<int> targets;
        for (auto e : edge_range) {
            targets.push_back(e.target_id(graph[0]));
        }
        
        REQUIRE(targets.size() == 3);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
        REQUIRE(targets[2] == 3);
    }
    
    SECTION("Get edges from vertex 1") {
        auto verts = vertices(graph);
        auto it = verts.begin();
        ++it;
        auto v1 = *it;
        
        auto edge_range = edges(graph, v1);
        
        std::vector<int> targets;
        for (auto e : edge_range) {
            targets.push_back(e.target_id(graph[1]));
        }
        
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == 2);
        REQUIRE(targets[1] == 3);
    }
    
    SECTION("Get edges from vertex with no edges") {
        auto verts = vertices(graph);
        auto it = verts.begin();
        std::advance(it, 3);
        auto v3 = *it;
        
        auto edge_range = edges(graph, v3);
        
        REQUIRE(std::ranges::empty(edge_range));
    }
    
    SECTION("Edge descriptors maintain source vertex") {
        auto verts = vertices(graph);
        auto v0 = *verts.begin();
        
        auto edge_range = edges(graph, v0);
        for (auto e : edge_range) {
            REQUIRE(e.source().vertex_id() == 0);
        }
    }
}

// =============================================================================
// Test: Default Implementation - Pair Edge Pattern
// =============================================================================

TEST_CASE("edges(g,u) - vector<vector<pair<int,double>>> weighted edges", "[edges][cpo][default][pair]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}, {3, 3.5}},  // vertex 0
        {{2, 1.2}, {3, 2.3}},            // vertex 1
        {{3, 1.0}},                      // vertex 2
        {}                               // vertex 3
    };
    
    SECTION("Extract target IDs from pair edges") {
        auto verts = vertices(graph);
        auto v0 = *verts.begin();
        
        auto edge_range = edges(graph, v0);
        
        std::vector<int> targets;
        std::vector<double> weights;
        for (auto e : edge_range) {
            targets.push_back(e.target_id(graph[0]));
            weights.push_back(e.underlying_value(graph[0]).second);
        }
        
        REQUIRE(targets.size() == 3);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
        REQUIRE(targets[2] == 3);
        
        REQUIRE(weights[0] == 1.5);
        REQUIRE(weights[1] == 2.5);
        REQUIRE(weights[2] == 3.5);
    }
    
    SECTION("Access edge properties via inner_value") {
        auto verts = vertices(graph);
        auto v0 = *verts.begin();
        
        auto edge_range = edges(graph, v0);
        auto e = *edge_range.begin();
        
        // inner_value returns the .second (weight) for pair edges
        auto& weight = e.inner_value(graph[0]);
        REQUIRE(weight == 1.5);
    }
}

// =============================================================================
// Test: Default Implementation - Tuple Edge Pattern
// =============================================================================

TEST_CASE("edges(g,u) - vector<vector<tuple<...>>> multi-property edges", "[edges][cpo][default][tuple]") {
    using Edge = std::tuple<int, double, std::string>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5, "road"}, {2, 2.5, "rail"}},
        {{3, 3.5, "air"}},
        {}
    };
    
    SECTION("Extract target IDs from tuple edges") {
        auto verts = vertices(graph);
        auto v0 = *verts.begin();
        
        auto edge_range = edges(graph, v0);
        
        std::vector<int> targets;
        for (auto e : edge_range) {
            targets.push_back(e.target_id(graph[0]));
        }
        
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
    }
    
    SECTION("Access full tuple edge data") {
        auto verts = vertices(graph);
        auto v0 = *verts.begin();
        
        auto edge_range = edges(graph, v0);
        auto e = *edge_range.begin();
        
        auto& full_edge = e.underlying_value(graph[0]);
        REQUIRE(std::get<0>(full_edge) == 1);
        REQUIRE(std::get<1>(full_edge) == 1.5);
        REQUIRE(std::get<2>(full_edge) == "road");
    }
}

// =============================================================================
// Test: Deque Storage
// =============================================================================

TEST_CASE("edges(g,u) - deque<deque<int>> simple edges", "[edges][cpo][default][deque]") {
    std::deque<std::deque<int>> graph = {
        {1, 2},
        {2},
        {}
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    
    auto edge_range = edges(graph, v0);
    
    std::vector<int> targets;
    for (auto e : edge_range) {
        targets.push_back(e.target_id(graph[0]));
    }
    
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
}

// =============================================================================
// Test: Map Storage
// =============================================================================

TEST_CASE("edges(g,u) - map<int, vector<int>>", "[edges][cpo][default][map]") {
    std::map<int, std::vector<int>> graph = {
        {100, {200, 300}},
        {200, {300}},
        {300, {}}
    };
    
    auto verts = vertices(graph);
    auto v100 = *verts.begin();
    
    REQUIRE(v100.vertex_id() == 100);
    
    // For maps, inner_value already extracts the .second (the vector of edges)
    auto& edge_container = v100.inner_value(graph);
    auto edge_range = edges(graph, v100);
    
    std::vector<int> targets;
    for (auto e : edge_range) {
        targets.push_back(e.target_id(edge_container));
    }
    
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 200);
    REQUIRE(targets[1] == 300);
}

// =============================================================================
// Test: Custom Member Function Override
// =============================================================================

TEST_CASE("edges(g,u) - custom member function", "[edges][cpo][member]") {
    struct CustomGraph {
        std::vector<std::vector<int>> adj_list = {
            {1, 2},
            {2},
            {}
        };
        
        // Custom edges member function - return mutable reference to allow edge_descriptor_view construction
        auto edges(vertex_descriptor<std::vector<std::vector<int>>::iterator> u) {
            auto& edge_container = adj_list[u.vertex_id()];
            return edge_descriptor_view(edge_container, u);
        }
    };
    
    CustomGraph g;
    auto verts = vertices(g.adj_list);
    auto v0 = *verts.begin();
    
    // This should use g.edges(v0) member function
    auto edge_range = edges(g, v0);
    
    std::vector<int> targets;
    for (auto e : edge_range) {
        targets.push_back(e.target_id(g.adj_list[0]));
    }
    
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
}

// =============================================================================
// Test: Custom ADL Override
// =============================================================================

namespace custom_ns {
    struct CustomGraph {
        std::vector<std::vector<int>> data = {
            {10, 20},
            {20},
            {}
        };
    };
    
    // ADL-findable edges function - take mutable reference to allow edge_descriptor_view construction
    auto edges(CustomGraph& g, vertex_descriptor<std::vector<std::vector<int>>::iterator> u) {
        auto& edge_container = g.data[u.vertex_id()];
        return edge_descriptor_view(edge_container, u);
    }
}

TEST_CASE("edges(g,u) - ADL customization", "[edges][cpo][adl]") {
    custom_ns::CustomGraph g;
    auto verts = vertices(g.data);
    auto v0 = *verts.begin();
    
    // This should find edges via ADL
    auto edge_range = edges(g, v0);
    
    std::vector<int> targets;
    for (auto e : edge_range) {
        targets.push_back(e.target_id(g.data[0]));
    }
    
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 10);
    REQUIRE(targets[1] == 20);
}

// =============================================================================
// Test: Integration with vertices(g)
// =============================================================================

TEST_CASE("edges(g,u) - integration with vertices(g)", "[edges][cpo][integration]") {
    std::vector<std::vector<int>> graph = {
        {1, 2},
        {2, 3},
        {3},
        {}
    };
    
    SECTION("Iterate all vertices and their edges") {
        std::vector<std::pair<int, int>> all_edges; // (source, target)
        
        for (auto u : vertices(graph)) {
            int uid = static_cast<int>(u.vertex_id());
            for (auto e : edges(graph, u)) {
                int vid = e.target_id(graph[static_cast<size_t>(uid)]);
                all_edges.emplace_back(uid, vid);
            }
        }
        
        REQUIRE(all_edges.size() == 5);  // 5 edges total
        REQUIRE(all_edges[0] == std::pair{0, 1});
        REQUIRE(all_edges[1] == std::pair{0, 2});
        REQUIRE(all_edges[2] == std::pair{1, 2});
        REQUIRE(all_edges[3] == std::pair{1, 3});
        REQUIRE(all_edges[4] == std::pair{2, 3});
    }
}

// =============================================================================
// Test: Const Graph Access (reads only)
// =============================================================================

TEST_CASE("edges(g,u) - const target_id access", "[edges][cpo][const]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {}
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    
    auto edge_range = edges(graph, v0);
    
    // Read target IDs (const operation)
    std::vector<int> targets;
    for (auto e : edge_range) {
        const auto& edge_container = graph[0];
        targets.push_back(e.target_id(edge_container));
    }
    
    REQUIRE(targets.size() == 3);
}

// =============================================================================
// Test: Edge Range Properties
// =============================================================================

TEST_CASE("edges(g,u) - range properties", "[edges][cpo][range]") {
    std::vector<std::vector<int>> graph = {
        {1, 2, 3},
        {},
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    
    auto edge_range = edges(graph, v0);
    
    SECTION("Forward range") {
        REQUIRE(std::ranges::forward_range<decltype(edge_range)>);
    }
    
    SECTION("Has begin/end") {
        auto begin_it = edge_range.begin();
        auto end_it = edge_range.end();
        REQUIRE(begin_it != end_it);
    }
    
    SECTION("Can iterate multiple times") {
        int count1 = 0;
        for (auto e : edge_range) {
            ++count1;
            (void)e;
        }
        
        int count2 = 0;
        for (auto e : edge_range) {
            ++count2;
            (void)e;
        }
        
        REQUIRE(count1 == 3);
        REQUIRE(count2 == 3);
    }
    
    SECTION("Sized range for random access") {
        REQUIRE(std::ranges::sized_range<decltype(edge_range)>);
        REQUIRE(edge_range.size() == 3);
    }
}

// =============================================================================
// Test: Empty Edge Ranges
// =============================================================================

TEST_CASE("edges(g,u) - empty edge ranges", "[edges][cpo][empty]") {
    std::vector<std::vector<int>> graph = {
        {},
        {1},
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    
    auto edge_range = edges(graph, v0);
    
    REQUIRE(std::ranges::empty(edge_range));
    REQUIRE(edge_range.begin() == edge_range.end());
    REQUIRE(edge_range.size() == 0);
}

// =============================================================================
// Test: Type Deduction
// =============================================================================

TEST_CASE("edges(g,u) - type deduction", "[edges][cpo][types]") {
    std::vector<std::vector<int>> graph = {{1, 2}};
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    auto edge_range = edges(graph, v0);
    
    // Check that types are correctly deduced
    using EdgeRange = decltype(edge_range);
    using EdgeIter = std::vector<int>::iterator;
    using VertexIter = std::vector<std::vector<int>>::iterator;
    
    STATIC_REQUIRE(std::same_as<EdgeRange, edge_descriptor_view<EdgeIter, VertexIter>>);
}

// =============================================================================
// Test: Edge Descriptor Functionality
// =============================================================================

TEST_CASE("edges(g,u) - edge descriptor functionality", "[edges][cpo][descriptor]") {
    std::vector<std::vector<std::pair<int, double>>> graph = {
        {{1, 1.5}, {2, 2.5}},
        {}
    };
    
    auto verts = vertices(graph);
    auto v0 = *verts.begin();
    auto edge_range = edges(graph, v0);
    
    SECTION("Edge has source vertex") {
        for (auto e : edge_range) {
            REQUIRE(e.source().vertex_id() == 0);
        }
    }
    
    SECTION("Edge can access target ID") {
        auto e = *edge_range.begin();
        REQUIRE(e.target_id(graph[0]) == 1);
    }
    
    SECTION("Edge can access underlying value") {
        auto e = *edge_range.begin();
        auto& pair = e.underlying_value(graph[0]);
        REQUIRE(pair.first == 1);
        REQUIRE(pair.second == 1.5);
    }
    
    SECTION("Edge can access inner value") {
        auto e = *edge_range.begin();
        auto& weight = e.inner_value(graph[0]);
        REQUIRE(weight == 1.5);
    }
}

// =============================================================================
// Test: Note about unsupported container types
// =============================================================================

// NOTE: list<list<T>> is not supported because list<T>& (the dereferenced type)
// doesn't satisfy the vertex_iterator concept requirements (must be fundamental or pair).
// Supported patterns:
// - vector<vector<T>> - random access, fundamental inner type
// - vector<list<T>> - random access vertices, bidirectional edges
// - map<K, vector<T>> - keyed vertices (pair-like), random access edges
// - deque<deque<T>> - random access

// =============================================================================
// Test: Mixed Graph Types
// =============================================================================

TEST_CASE("edges(g,u) - map with weighted edges", "[edges][cpo][map][weighted]") {
    using Edge = std::pair<int, double>;
    std::map<int, std::vector<Edge>> graph = {
        {100, {{200, 1.5}, {300, 2.5}}},
        {200, {{300, 3.5}}},
        {300, {}}
    };
    
    auto verts = vertices(graph);
    auto v100 = *verts.begin();
    
    // For maps, inner_value already extracts the .second (the vector of edges)
    auto& edge_container = v100.inner_value(graph);
    
    auto edge_range = edges(graph, v100);
    
    std::vector<int> targets;
    std::vector<double> weights;
    for (auto e : edge_range) {
        targets.push_back(e.target_id(edge_container));
        weights.push_back(e.underlying_value(edge_container).second);
    }
    
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 200);
    REQUIRE(targets[1] == 300);
    REQUIRE(weights[0] == 1.5);
    REQUIRE(weights[1] == 2.5);
}
