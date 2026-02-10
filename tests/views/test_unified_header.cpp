/**
 * @file test_unified_header.cpp
 * @brief Tests for unified views.hpp header
 */

#include <catch2/catch_test_macros.hpp>

// Test that single include works
#include <graph/views.hpp>

#include <vector>

using namespace graph;
using namespace graph::views::adaptors;

// Simple test graph using vector-of-vectors
// Graph structure: 0 -> {1, 2}, 1 -> {2}, 2 -> {}
using test_graph = std::vector<std::vector<int>>;

inline auto make_test_graph() {
    test_graph g(3);  // 3 vertices
    g[0] = {1, 2};    // vertex 0 connects to vertices 1 and 2
    g[1] = {2};       // vertex 1 connects to vertex 2
    g[2] = {};        // vertex 2 has no outgoing edges
    return g;
}

TEST_CASE("unified header - all basic views accessible", "[unified][basic_views]") {
    auto g = make_test_graph();
    
    // Test that all basic views are accessible
    int vertex_count = 0;
    for (auto [id, v] : g | vertexlist()) {
        ++vertex_count;
    }
    REQUIRE(vertex_count == 3);
    
    int edge_count = 0;
    for (auto [tid, e] : g | incidence(0)) {
        ++edge_count;
    }
    REQUIRE(edge_count == 2);
    
    int neighbor_count = 0;
    for (auto [n] : g | neighbors(0)) {
        ++neighbor_count;
    }
    REQUIRE(neighbor_count == 2);
    
    int total_edges = 0;
    for (auto [e] : g | edgelist()) {
        ++total_edges;
    }
    REQUIRE(total_edges == 3);
}

TEST_CASE("unified header - all search views accessible", "[unified][search_views]") {
    auto g = make_test_graph();
    
    // Test DFS views
    int dfs_vertices = 0;
    for (auto [v] : g | vertices_dfs(0)) {
        ++dfs_vertices;
    }
    REQUIRE(dfs_vertices == 3);
    
    int dfs_edges = 0;
    for (auto [e] : g | edges_dfs(0)) {
        ++dfs_edges;
    }
    REQUIRE(dfs_edges == 2);  // DFS tree has 2 edges (0->1, 1->2)
    
    // Test BFS views
    int bfs_vertices = 0;
    for (auto [v] : g | vertices_bfs(0)) {
        ++bfs_vertices;
    }
    REQUIRE(bfs_vertices == 3);
    
    int bfs_edges = 0;
    for (auto [e] : g | edges_bfs(0)) {
        ++bfs_edges;
    }
    REQUIRE(bfs_edges == 2);  // BFS tree also has 2 edges
    
    // Test topological sort views
    int topo_vertices = 0;
    for (auto [v] : g | vertices_topological_sort()) {
        ++topo_vertices;
    }
    REQUIRE(topo_vertices == 3);
    
    int topo_edges = 0;
    for (auto [e] : g | edges_topological_sort()) {
        ++topo_edges;
    }
    REQUIRE(topo_edges == 3);
}

TEST_CASE("unified header - value functions work", "[unified][value_functions]") {
    auto g = make_test_graph();
    
    auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
    
    std::vector<int> values;
    for (auto [id, v, val] : g | vertexlist(vvf)) {
        values.push_back(val);
    }
    
    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 0);
    REQUIRE(values[1] == 10);
    REQUIRE(values[2] == 20);
}

TEST_CASE("unified header - chaining with std::views works", "[unified][chaining]") {
    auto g = make_test_graph();
    
    // Test complex chaining
    std::vector<int> results;
    for (auto id : g | vertexlist()
                     | std::views::transform([&g](auto info) {
                         auto [id, v] = info;
                         return id;
                       })
                     | std::views::filter([](int id) { return id > 0; })
                     | std::views::transform([](int id) { return id * 2; })) {
        results.push_back(id);
    }
    
    REQUIRE(results.size() == 2);
    REQUIRE(results[0] == 2);  // vertex 1 * 2
    REQUIRE(results[1] == 4);  // vertex 2 * 2
}

TEST_CASE("unified header - direct calls work", "[unified][direct_calls]") {
    auto g = make_test_graph();
    
    // Test that direct calls (without pipes) also work
    auto vertex_view = graph::views::vertexlist(g);
    int count = 0;
    for (auto [id, v] : vertex_view) {
        ++count;
    }
    REQUIRE(count == 3);
    
    auto dfs_view = graph::views::vertices_dfs(g, 0);
    count = 0;
    for (auto [v] : dfs_view) {
        ++count;
    }
    REQUIRE(count == 3);
}
