/**
 * @file test_graph_hpp_includes_views.cpp
 * @brief Tests that the graph views library works correctly with graph.hpp
 * 
 * This test verifies that users can access all views by including both
 * <graph/graph.hpp> and <graph/views.hpp>. While the view syntax would  
 * prefer to have everything in one header, circular dependencies between
 * graph.hpp and views.hpp make this impractical.
 */

#include <catch2/catch_test_macros.hpp>

// Include the main graph.hpp header
#include <graph/graph.hpp>
// Include views - required separately to avoid circular dependencies
#include <graph/views.hpp>

#include <vector>
#include <algorithm>

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

TEST_CASE("graph.hpp - basic views accessible", "[graph_hpp][basic_views]") {
    auto g = make_test_graph();
    
    // Test that basic views work through graph.hpp
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
    for (auto [tid, n] : g | neighbors(0)) {
        ++neighbor_count;
    }
    REQUIRE(neighbor_count == 2);
    
    int total_edges = 0;
    for (auto [sid, tid, e] : g | edgelist()) {
        ++total_edges;
    }
    REQUIRE(total_edges == 3);
}

TEST_CASE("graph.hpp - search views accessible", "[graph_hpp][search_views]") {
    auto g = make_test_graph();
    
    // Test that DFS views work through graph.hpp
    int dfs_vertices = 0;
    for (auto [v] : g | vertices_dfs(0)) {
        ++dfs_vertices;
    }
    REQUIRE(dfs_vertices == 3);
    
    int dfs_edges = 0;
    for (auto [e] : g | edges_dfs(0)) {
        ++dfs_edges;
    }
    REQUIRE(dfs_edges == 2);  // DFS tree has 2 edges
    
    // Test that BFS views work through graph.hpp
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
    
    // Test that topological sort views work through graph.hpp
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

TEST_CASE("graph.hpp - value functions work", "[graph_hpp][value_functions]") {
    auto g = make_test_graph();
    
    // Test that value functions work through graph.hpp
    auto vvf = [&g](auto v) { return vertex_id(g, v); };
    
    std::vector<int> values;
    for (auto [id, v, val] : g | vertexlist(vvf)) {
        values.push_back(val);
    }
    
    REQUIRE(values.size() == 3);
    // Values should be vertex IDs 0, 1, 2
    std::sort(values.begin(), values.end());
    REQUIRE(values == std::vector<int>{0, 1, 2});
}

TEST_CASE("graph.hpp - chaining with std::views", "[graph_hpp][chaining]") {
    auto g = make_test_graph();
    
    // Test that chaining with std::views works through graph.hpp
    auto vertex_view = g 
        | vertexlist() 
        | std::views::take(2);
    
    int count = 0;
    for (auto [id, v] : vertex_view) {
        ++count;
    }
    REQUIRE(count == 2);
    
    // Test complex chain
    auto dfs_view = g 
        | vertices_dfs(0) 
        | std::views::drop(1);
    
    count = 0;
    for (auto [v] : dfs_view) {
        ++count;
    }
    REQUIRE(count == 2);  // 3 vertices - 1 dropped = 2
}
