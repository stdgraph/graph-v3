/**
 * @file test_connected_components.cpp
 * @brief Tests for connected components algorithms from connected_components.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/connected_components.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <algorithm>
#include <set>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// =============================================================================
// Helper Functions
// =============================================================================

// Check if all vertices in a component have the same component ID
template<typename Component>
bool all_same_component(const Component& component, const std::vector<size_t>& vertices) {
    if (vertices.empty()) return true;
    auto first_comp = component[vertices[0]];
    return std::all_of(vertices.begin(), vertices.end(),
                      [&](size_t v) { return component[v] == first_comp; });
}

// Check if vertices are in different components
template<typename Component>
bool different_components(const Component& component, size_t u, size_t v) {
    return component[u] != component[v];
}

// Count number of unique components
template<typename Component>
size_t count_unique_components(const Component& component) {
    std::set<typename Component::value_type> unique(component.begin(), component.end());
    return unique.size();
}

// =============================================================================
// connected_components() Tests - Undirected Graphs with Both Graph Types
// =============================================================================
//
// These tests validate that connected_components works correctly with:
// 1. vov_void: Uses 2 physical edges {u,v} and {v,u} to simulate undirected
// 2. undirected_adjacency_list: Truly undirected with 1 physical edge
//
// Both approaches should produce identical component assignments.
// =============================================================================

TEST_CASE("connected_components - single vertex", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    auto g = single_vertex<Graph>(); 
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(component[0] == 0);
}

TEST_CASE("connected_components - single edge", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    auto g = single_edge<Graph>();
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1}));
}

TEST_CASE("connected_components - path graph", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Path: 0 - 1 - 2 - 3
    auto g = path_graph_4<Graph>(); 
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("connected_components - cycle graph", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Cycle: 0 - 1 - 2 - 3 - 4 - 0
    auto g = cycle_graph_5<Graph>();
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("connected_components - disconnected graph", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Two components: {0, 1} and {2, 3}
    Graph g({{0, 1}, {1, 0}, {2, 3}, {3, 2}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 2);
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(all_same_component(component, {2, 3}));
    REQUIRE(different_components(component, 0, 2));
}

TEST_CASE("connected_components - multiple isolated vertices", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Five isolated vertices
    Graph g;
    g.resize_vertices(5);
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 5);
    for (size_t i = 0; i < 5; ++i) {
        for (size_t j = i + 1; j < 5; ++j) {
            REQUIRE(different_components(component, i, j));
        }
    }
}

TEST_CASE("connected_components - star graph", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Star: center 0 connected to 1, 2, 3, 4
    Graph g({{0, 1}, {0, 2}, {0, 3}, {0, 4},
             {1, 0}, {2, 0}, {3, 0}, {4, 0}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("connected_components - complete graph", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Complete graph K4: all vertices connected to each other
    Graph g({{0, 1}, {0, 2}, {0, 3},
             {1, 0}, {1, 2}, {1, 3},
             {2, 0}, {2, 1}, {2, 3},
             {3, 0}, {3, 1}, {3, 2}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("connected_components - tree structure", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Binary tree: 0 is root, 1 and 2 are children, 3,4,5,6 are grandchildren
    Graph g({{0, 1}, {0, 2}, {1, 0}, {1, 3}, {1, 4},
             {2, 0}, {2, 5}, {2, 6}, {3, 1}, {4, 1},
             {5, 2}, {6, 2}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4, 5, 6}));
}

TEST_CASE("connected_components - multiple components of different sizes", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Component 1: {0, 1, 2} (triangle)
    // Component 2: {3, 4} (edge)
    // Component 3: {5} (isolated)
    Graph g({{0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 0}, {2, 1},
             {3, 4}, {4, 3}});
    g.resize_vertices(6);  // Add isolated vertex 5
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 3);
    REQUIRE(all_same_component(component, {0, 1, 2}));
    REQUIRE(all_same_component(component, {3, 4}));
    REQUIRE(different_components(component, 0, 3));
    REQUIRE(different_components(component, 0, 5));
    REQUIRE(different_components(component, 3, 5));
}

// =============================================================================
// connected_components() - Comparison Tests for Undirected Graph Approaches
// =============================================================================
//
// These tests validate that BOTH undirected graph approaches produce identical 
// results for the same graph topology:
//
// Approach 1: vov_void with bidirectional edges
//   - Requires adding {u,v} AND {v,u} for each undirected edge
//   - Uses 2 physical edges in memory per logical edge
//   - Works with compressed_graph, dynamic_graph, vector<vector<>>
//
// Approach 2: undirected_adjacency_list
//   - Truly undirected: only add {u,v} once
//   - Uses 1 physical edge stored in both adjacency lists
//   - Specifically designed for undirected graphs
//
// Both should assign the same component IDs to vertices.
// =============================================================================

TEST_CASE("connected_components - undirected single edge (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void with bidirectional edges") {
        // Undirected edge 0-1: requires {0,1} and {1,0}
        vov_void g({{0, 1}, {1, 0}});
        std::vector<uint32_t> component(2);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1}));
    }
    
    SECTION("undirected_adjacency_list with single edge") {
        // Undirected edge 0-1: only add once
        undirected_adjacency_list<int, int> g({{0, 1, 1}});
        std::vector<uint32_t> component(2);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1}));
    }
}

TEST_CASE("connected_components - undirected path (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void: Path 0-1-2-3 with bidirectional edges") {
        // Each undirected edge needs both directions
        vov_void g({
            {0, 1}, {1, 0},  // edge 0-1
            {1, 2}, {2, 1},  // edge 1-2
            {2, 3}, {3, 2}   // edge 2-3
        });
        std::vector<uint32_t> component(4);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3}));
    }
    
    SECTION("undirected_adjacency_list: Path 0-1-2-3 with single edges") {
        undirected_adjacency_list<int, int> g({
            {0, 1, 1}, {1, 2, 1}, {2, 3, 1}
        });
        std::vector<uint32_t> component(4);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3}));
    }
}

TEST_CASE("connected_components - undirected disconnected (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void: Two components {0,1} and {2,3}") {
        vov_void g({
            {0, 1}, {1, 0},  // component 1
            {2, 3}, {3, 2}   // component 2
        });
        std::vector<uint32_t> component(4);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 2);
        REQUIRE(all_same_component(component, {0, 1}));
        REQUIRE(all_same_component(component, {2, 3}));
        REQUIRE(different_components(component, 0, 2));
    }
    
    SECTION("undirected_adjacency_list: Two components {0,1} and {2,3}") {
        undirected_adjacency_list<int, int> g({
            {0, 1, 1}, {2, 3, 1}
        });
        std::vector<uint32_t> component(4);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 2);
        REQUIRE(all_same_component(component, {0, 1}));
        REQUIRE(all_same_component(component, {2, 3}));
        REQUIRE(different_components(component, 0, 2));
    }
}

TEST_CASE("connected_components - undirected cycle (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void: Cycle 0-1-2-3-4-0") {
        vov_void g({
            {0, 1}, {1, 0},
            {1, 2}, {2, 1},
            {2, 3}, {3, 2},
            {3, 4}, {4, 3},
            {4, 0}, {0, 4}
        });
        std::vector<uint32_t> component(5);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
    }
    
    SECTION("undirected_adjacency_list: Cycle 0-1-2-3-4-0") {
        undirected_adjacency_list<int, int> g({
            {0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}, {4, 0, 1}
        });
        std::vector<uint32_t> component(5);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
    }
}

TEST_CASE("connected_components - undirected triangle (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void: Triangle 0-1-2-0") {
        vov_void g({
            {0, 1}, {1, 0},
            {1, 2}, {2, 1},
            {2, 0}, {0, 2}
        });
        std::vector<uint32_t> component(3);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2}));
    }
    
    SECTION("undirected_adjacency_list: Triangle 0-1-2-0") {
        undirected_adjacency_list<int, int> g({
            {0, 1, 1}, {1, 2, 1}, {2, 0, 1}
        });
        std::vector<uint32_t> component(3);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2}));
    }
}

TEST_CASE("connected_components - undirected star (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void: Star with center 0") {
        vov_void g({
            {0, 1}, {1, 0},
            {0, 2}, {2, 0},
            {0, 3}, {3, 0},
            {0, 4}, {4, 0}
        });
        std::vector<uint32_t> component(5);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
    }
    
    SECTION("undirected_adjacency_list: Star with center 0") {
        undirected_adjacency_list<int, int> g({
            {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {0, 4, 1}
        });
        std::vector<uint32_t> component(5);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
    }
}

TEST_CASE("connected_components - undirected complex (vov vs UAL)", "[algorithm][connected_components][undirected]") {
    SECTION("vov_void: Three components of different sizes") {
        // Component 1: {0,1,2} triangle
        // Component 2: {3,4} edge  
        // Component 3: {5} isolated
        vov_void g({
            {0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 0}, {0, 2},  // triangle
            {3, 4}, {4, 3}                                    // edge
        });
        g.resize_vertices(6);  // Add isolated vertex 5
        std::vector<uint32_t> component(6);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 3);
        REQUIRE(all_same_component(component, {0, 1, 2}));
        REQUIRE(all_same_component(component, {3, 4}));
        REQUIRE(different_components(component, 0, 3));
        REQUIRE(different_components(component, 0, 5));
        REQUIRE(different_components(component, 3, 5));
    }
    
    SECTION("undirected_adjacency_list: Three components of different sizes") {
        undirected_adjacency_list<int, int> g({
            {0, 1, 1}, {1, 2, 1}, {2, 0, 1}, {3, 4, 1}
        });
        g.resize_vertices(6);  // Add isolated vertex 5
        std::vector<uint32_t> component(6);
        
        size_t num = connected_components(g, component);
        
        REQUIRE(num == 3);
        REQUIRE(all_same_component(component, {0, 1, 2}));
        REQUIRE(all_same_component(component, {3, 4}));
        REQUIRE(different_components(component, 0, 3));
        REQUIRE(different_components(component, 0, 5));
        REQUIRE(different_components(component, 3, 5));
    }
}

// =============================================================================
// connected_components() Tests - undirected_adjacency_list
// =============================================================================

TEST_CASE("connected_components (UAL) - single vertex", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    Graph g;
    g.create_vertex(0);
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(component[0] == 0);
}

TEST_CASE("connected_components (UAL) - single edge", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    Graph g({{0, 1, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1}));
}

TEST_CASE("connected_components (UAL) - path graph", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Path: 0 - 1 - 2 - 3
    Graph g({{0, 1, 1}, {1, 2, 1}, {2, 3, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("connected_components (UAL) - cycle graph", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Cycle: 0 - 1 - 2 - 3 - 4 - 0
    Graph g({{0, 1, 1}, {1, 2, 1}, {2, 3, 1}, {3, 4, 1}, {4, 0, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("connected_components (UAL) - disconnected graph", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Two components: {0, 1} and {2, 3}
    Graph g({{0, 1, 1}, {2, 3, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 2);
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(all_same_component(component, {2, 3}));
    REQUIRE(different_components(component, 0, 2));
}

TEST_CASE("connected_components (UAL) - isolated vertices", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Five isolated vertices
    Graph g;
    for (int i = 0; i < 5; ++i) {
        g.create_vertex(i);
    }
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 5);
    for (size_t i = 0; i < 5; ++i) {
        for (size_t j = i + 1; j < 5; ++j) {
            REQUIRE(different_components(component, i, j));
        }
    }
}

TEST_CASE("connected_components (UAL) - star graph", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Star: center 0 connected to 1, 2, 3, 4
    Graph g({{0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {0, 4, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("connected_components (UAL) - complete graph", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Complete graph K4: all vertices connected to each other
    Graph g({
        {0, 1, 1}, {0, 2, 1}, {0, 3, 1},
        {1, 2, 1}, {1, 3, 1}, {2, 3, 1}
    });
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("connected_components (UAL) - tree structure", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Binary tree: 0 is root, 1 and 2 are children, 3,4,5,6 are grandchildren
    Graph g({
        {0, 1, 1}, {0, 2, 1},
        {1, 3, 1}, {1, 4, 1},
        {2, 5, 1}, {2, 6, 1}
    });
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4, 5, 6}));
}

TEST_CASE("connected_components (UAL) - multiple components of different sizes", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Component 1: {0, 1, 2} (triangle)
    // Component 2: {3, 4} (edge)
    // Component 3: {5} (isolated)
    Graph g({
        {0, 1, 1}, {0, 2, 1}, {1, 2, 1},  // triangle
        {3, 4, 1}                         // edge
    });
    g.resize_vertices(6);  // Add isolated vertex 5
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 3);
    REQUIRE(all_same_component(component, {0, 1, 2}));
    REQUIRE(all_same_component(component, {3, 4}));
    REQUIRE(different_components(component, 0, 3));
    REQUIRE(different_components(component, 0, 5));
    REQUIRE(different_components(component, 3, 5));
}

TEST_CASE("connected_components (UAL) - self loop", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Graph with self-loop: 0 - 0, 0 - 1
    Graph g({
        {0, 0, 1},  // self-loop
        {0, 1, 1}
    });
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1}));
}

TEST_CASE("connected_components (UAL) - with edge values", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Path graph with different edge weights
    Graph g({{0, 1, 10}, {1, 2, 20}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1, 2}));
}

TEST_CASE("connected_components (UAL) - with vertex values", "[algorithm][connected_components][ual]") {
    using Graph = undirected_adjacency_list<int, int>;
    
    // Disconnected with vertex values
    Graph g;
    g.create_vertex(100);
    g.create_vertex(200);
    g.create_vertex(300);
    g.create_edge(0, 1, 1);
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 2);
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(different_components(component, 0, 2));
}

// =============================================================================
// kosaraju() Tests - Strongly Connected Components (Directed Graphs)
// =============================================================================

TEST_CASE("kosaraju - single vertex", "[algorithm][kosaraju][scc]") {
    using Graph = vov_void;
    
    auto g = single_vertex<Graph>();
    auto g_t = single_vertex<Graph>(); // Transpose
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(component[0] == 0);
    REQUIRE(count_unique_components(component) == 1);
}

TEST_CASE("kosaraju - simple cycle", "[algorithm][kosaraju][scc]") {
    using Graph = vov_void;
    
    // Directed cycle: 0 -> 1 -> 2 -> 0
    Graph g({{0, 1}, {1, 2}, {2, 0}});
    Graph g_t({{1, 0}, {2, 1}, {0, 2}}); // Transpose
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // All three vertices should be in the same SCC
    REQUIRE(all_same_component(component, {0, 1, 2}));
}

TEST_CASE("kosaraju - two SCCs", "[algorithm][kosaraju][scc]") {
    using Graph = vov_void;
    
    // Two SCCs: {0, 1} and {2, 3}
    // 0 <-> 1, 2 <-> 3, with edge 1 -> 2 (not 2 -> 1)
    Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 3}, {3, 2}});
    Graph g_t({{1, 0}, {0, 1}, {2, 1}, {3, 2}, {2, 3}}); // Transpose
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // Vertices 0 and 1 should be in same SCC
    REQUIRE(all_same_component(component, {0, 1}));
    // Vertices 2 and 3 should be in same SCC
    REQUIRE(all_same_component(component, {2, 3}));
    // But different from each other
    REQUIRE(different_components(component, 0, 2));
}

TEST_CASE("kosaraju - no cycles (DAG)", "[algorithm][kosaraju][scc]") {
    using Graph = vov_void;
    
    // DAG: 0 -> 1 -> 2 -> 3 (no cycles)
    Graph g({{0, 1}, {1, 2}, {2, 3}});
    Graph g_t({{1, 0}, {2, 1}, {3, 2}}); // Transpose
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // Each vertex should be in its own SCC
    REQUIRE(count_unique_components(component) == 4);
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = i + 1; j < 4; ++j) {
            REQUIRE(different_components(component, i, j));
        }
    }
}

TEST_CASE("kosaraju - complex SCC structure", "[algorithm][kosaraju][scc]") {
    using Graph = vov_void;
    
    // Complex structure with 3 SCCs:
    // SCC1: {0, 1, 2} with cycle
    // SCC2: {3, 4} with cycle  
    // SCC3: {5} singleton
    // Edges between SCCs: 2 -> 3, 4 -> 5
    Graph g({{0, 1}, {1, 2}, {2, 0},  // SCC1 cycle
             {2, 3},                   // Cross-SCC edge
             {3, 4}, {4, 3},           // SCC2 cycle
             {4, 5}});                 // Cross-SCC edge
    Graph g_t({{1, 0}, {2, 1}, {0, 2},  // SCC1 cycle transposed
               {3, 2},                   // Cross-SCC edge transposed
               {4, 3}, {3, 4},           // SCC2 cycle transposed
               {5, 4}});                 // Cross-SCC edge transposed
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 3);
    REQUIRE(all_same_component(component, {0, 1, 2}));
    REQUIRE(all_same_component(component, {3, 4}));
    REQUIRE(different_components(component, 0, 3));
    REQUIRE(different_components(component, 0, 5));
    REQUIRE(different_components(component, 3, 5));
}

// =============================================================================
// afforest() Tests - Parallel-Friendly Connected Components
// =============================================================================

TEST_CASE("afforest - single vertex", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    auto g = single_vertex<Graph>();
    std::vector<uint32_t> component(num_vertices(g));
    
    afforest(g, component);
    
    REQUIRE(component[0] == 0);
}

TEST_CASE("afforest - single edge", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    auto g = single_edge<Graph>();
    std::vector<uint32_t> component(num_vertices(g));
    
    afforest(g, component);
    
    REQUIRE(all_same_component(component, {0, 1}));
}

TEST_CASE("afforest - path graph", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    auto g = path_graph_4<Graph>();
    std::vector<uint32_t> component(num_vertices(g));
    
    afforest(g, component);
    
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("afforest - disconnected components", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    // Two components: {0, 1, 2} and {3, 4}
    Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1},
             {3, 4}, {4, 3}});
    std::vector<uint32_t> component(num_vertices(g));
    
    afforest(g, component);
    
    REQUIRE(all_same_component(component, {0, 1, 2}));
    REQUIRE(all_same_component(component, {3, 4}));
    REQUIRE(different_components(component, 0, 3));
}

TEST_CASE("afforest - custom neighbor rounds", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    auto g = cycle_graph_5<Graph>();
    std::vector<uint32_t> component(num_vertices(g));
    
    // Test with different neighbor_rounds values
    SECTION("neighbor_rounds = 1") {
        afforest(g, component, 1);
        REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
    }
    
    SECTION("neighbor_rounds = 3") {
        afforest(g, component, 3);
        REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
    }
}

TEST_CASE("afforest - with transpose (directed graph)", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    // Directed graph with bidirectional edges
    Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}});
    Graph g_t({{1, 0}, {0, 1}, {2, 1}, {1, 2}}); // Transpose
    std::vector<uint32_t> component(num_vertices(g));
    
    afforest(g, g_t, component);
    
    REQUIRE(all_same_component(component, {0, 1, 2}));
}

TEST_CASE("afforest - large disconnected graph", "[algorithm][afforest]") {
    using Graph = vov_void;
    
    // Multiple small components
    Graph g({{0, 1}, {1, 0},
             {2, 3}, {3, 2},
             {4, 5}, {5, 4}, {5, 6}, {6, 5},
             {7, 8}, {8, 7}, {8, 9}, {9, 8}});
    std::vector<uint32_t> component(num_vertices(g));
    
    afforest(g, component);
    
    REQUIRE(count_unique_components(component) == 4);
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(all_same_component(component, {2, 3}));
    REQUIRE(all_same_component(component, {4, 5, 6}));
    REQUIRE(all_same_component(component, {7, 8, 9}));
}

// =============================================================================
// Edge Cases and Special Scenarios
// =============================================================================

TEST_CASE("connected_components - empty graph", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    auto g = empty_graph<Graph>();
    std::vector<uint32_t> component;
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 0);
}

TEST_CASE("connected_components - self loops", "[algorithm][connected_components]") {
    using Graph = vov_void;
    
    // Vertices with self-loops: 0->0, 1->1, with edge 0-1
    Graph g({{0, 0}, {0, 1}, {1, 0}, {1, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    size_t num_components = connected_components(g, component);
    
    REQUIRE(num_components == 1);
    REQUIRE(all_same_component(component, {0, 1}));
}

TEST_CASE("kosaraju - self loops", "[algorithm][kosaraju][scc]") {
    using Graph = vov_void;
    
    // Directed graph with self-loops
    Graph g({{0, 0}, {0, 1}, {1, 1}});
    Graph g_t({{0, 0}, {1, 0}, {1, 1}}); // Transpose
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // Self-loops don't create SCCs by themselves without cycles
    // 0 -> 1 is a DAG, so they should be in different SCCs
    REQUIRE(different_components(component, 0, 1));
}

// =============================================================================
// Comprehensive kosaraju() Tests - Additional Coverage
// =============================================================================

TEST_CASE("kosaraju - graph with singleton SCCs", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Graph with mix: {0,1} cycle and singletons 2, 3
    // Edges: 0<->1, 1->2, 2->3 (latter two are DAG)
    Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 3}});
    Graph g_t({{1, 0}, {0, 1}, {2, 1}, {3, 2}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // Verify structure
    REQUIRE(count_unique_components(component) == 3); // {0,1}, {2}, {3}
    REQUIRE(all_same_component(component, {0, 1})); // Cycle forms SCC
    REQUIRE(different_components(component, 0, 2)); // Different
    REQUIRE(different_components(component, 1, 2)); // Different  
    REQUIRE(different_components(component, 2, 3)); // DAG edges
}

TEST_CASE("kosaraju - multiple cycles sharing vertices", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Overlapping cycles: 0->1->2->0 and 1->3->4->1
    // This creates one large SCC containing all vertices
    Graph g({{0, 1}, {1, 2}, {2, 0},     // First cycle
             {1, 3}, {3, 4}, {4, 1}});   // Second cycle
    Graph g_t({{1, 0}, {2, 1}, {0, 2},   // First cycle transposed
               {3, 1}, {4, 3}, {1, 4}}); // Second cycle transposed
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // All vertices should be in the same SCC
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("kosaraju - nested SCCs with bridges", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Structure: SCC1 {0,1} -> bridge -> SCC2 {2,3} -> bridge -> SCC3 {4,5}
    Graph g({{0, 1}, {1, 0},        // SCC1
             {1, 2},                 // Bridge
             {2, 3}, {3, 2},        // SCC2
             {3, 4},                 // Bridge
             {4, 5}, {5, 4}});      // SCC3
    Graph g_t({{1, 0}, {0, 1},      // SCC1 transposed
               {2, 1},               // Bridge transposed
               {3, 2}, {2, 3},      // SCC2 transposed
               {4, 3},               // Bridge transposed
               {5, 4}, {4, 5}});    // SCC3 transposed
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 3);
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(all_same_component(component, {2, 3}));
    REQUIRE(all_same_component(component, {4, 5}));
    REQUIRE(different_components(component, 0, 2));
    REQUIRE(different_components(component, 2, 4));
}

TEST_CASE("kosaraju - complete directed graph", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Complete directed graph on 4 vertices (every pair has edges in both directions)
    Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0},
             {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});
    Graph g_t({{1, 0}, {0, 1}, {2, 0}, {0, 2}, {3, 0}, {0, 3},
               {2, 1}, {1, 2}, {3, 1}, {1, 3}, {3, 2}, {2, 3}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // All vertices should be in one SCC
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("kosaraju - star topology DAG", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Star: center vertex 0 with edges to 1,2,3,4 (no reverse edges)
    Graph g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});
    Graph g_t({{1, 0}, {2, 0}, {3, 0}, {4, 0}}); // All point to center
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // No cycles, each vertex is its own SCC
    REQUIRE(count_unique_components(component) == 5);
    for (size_t i = 0; i < 5; ++i) {
        for (size_t j = i + 1; j < 5; ++j) {
            REQUIRE(different_components(component, i, j));
        }
    }
}

TEST_CASE("kosaraju - bidirectional star", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Star with bidirectional edges: 0 <-> 1, 0 <-> 2, 0 <-> 3
    Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}});
    Graph g_t({{1, 0}, {0, 1}, {2, 0}, {0, 2}, {3, 0}, {0, 3}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // All vertices in one SCC (can reach each other through center)
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
}

TEST_CASE("kosaraju - long chain of SCCs", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Chain: SCC0 -> SCC1 -> SCC2 -> SCC3 (each SCC is a pair with cycle)
    Graph g({{0, 1}, {1, 0},        // SCC0
             {1, 2},                 // Bridge
             {2, 3}, {3, 2},        // SCC1
             {3, 4},                 // Bridge
             {4, 5}, {5, 4},        // SCC2
             {5, 6},                 // Bridge
             {6, 7}, {7, 6}});      // SCC3
    Graph g_t({{1, 0}, {0, 1},      // SCC0
               {2, 1},               // Bridge
               {3, 2}, {2, 3},      // SCC1
               {4, 3},               // Bridge
               {5, 4}, {4, 5},      // SCC2
               {6, 5},               // Bridge
               {7, 6}, {6, 7}});    // SCC3
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 4);
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(all_same_component(component, {2, 3}));
    REQUIRE(all_same_component(component, {4, 5}));
    REQUIRE(all_same_component(component, {6, 7}));
}

TEST_CASE("kosaraju - converging paths", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Multiple paths converging: 0->2, 1->2, 2->3->4, 4->2 (creates SCC {2,3,4})
    Graph g({{0, 2}, {1, 2}, {2, 3}, {3, 4}, {4, 2}});
    Graph g_t({{2, 0}, {2, 1}, {3, 2}, {4, 3}, {2, 4}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 3); // {0}, {1}, {2,3,4}
    REQUIRE(all_same_component(component, {2, 3, 4}));
    REQUIRE(different_components(component, 0, 1));
    REQUIRE(different_components(component, 0, 2));
    REQUIRE(different_components(component, 1, 2));
}

TEST_CASE("kosaraju - diverging paths", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Diverging from SCC: {0,1} cycle, then 1->2 and 1->3 (separate paths)
    Graph g({{0, 1}, {1, 0}, {1, 2}, {1, 3}});
    Graph g_t({{1, 0}, {0, 1}, {2, 1}, {3, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 3); // {0,1}, {2}, {3}
    REQUIRE(all_same_component(component, {0, 1}));
    REQUIRE(different_components(component, 0, 2));
    REQUIRE(different_components(component, 0, 3));
    REQUIRE(different_components(component, 2, 3));
}

TEST_CASE("kosaraju - cross edges between SCCs", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Two SCCs with multiple cross edges: {0,1,2} and {3,4,5}
    // Cross edges: 0->3, 1->4, 2->5 (one direction only)
    Graph g({{0, 1}, {1, 2}, {2, 0},           // SCC1
             {3, 4}, {4, 5}, {5, 3},           // SCC2
             {0, 3}, {1, 4}, {2, 5}});         // Cross edges
    Graph g_t({{1, 0}, {2, 1}, {0, 2},         // SCC1
               {4, 3}, {5, 4}, {3, 5},         // SCC2
               {3, 0}, {4, 1}, {5, 2}});       // Cross edges
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 2);
    REQUIRE(all_same_component(component, {0, 1, 2}));
    REQUIRE(all_same_component(component, {3, 4, 5}));
    REQUIRE(different_components(component, 0, 3));
}

TEST_CASE("kosaraju - triangle with tail", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Triangle cycle 0->1->2->0, with tail 2->3->4
    Graph g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}});
    Graph g_t({{1, 0}, {2, 1}, {0, 2}, {3, 2}, {4, 3}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 3); // {0,1,2}, {3}, {4}
    REQUIRE(all_same_component(component, {0, 1, 2}));
    REQUIRE(different_components(component, 0, 3));
    REQUIRE(different_components(component, 3, 4));
}

TEST_CASE("kosaraju - back edges creating large SCC", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Path 0->1->2->3->4 with back edge 4->0 creates one SCC
    Graph g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});
    Graph g_t({{1, 0}, {2, 1}, {3, 2}, {4, 3}, {0, 4}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("kosaraju - multiple self-loops in cycle", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Cycle 0->1->2->0 where each vertex also has a self-loop
    Graph g({{0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}, {2, 0}});
    Graph g_t({{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}, {0, 2}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // Self-loops don't affect the SCC structure
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1, 2}));
}

TEST_CASE("kosaraju - single vertex self-loop only", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Single vertex with self-loop only
    Graph g({{0, 0}});
    Graph g_t({{0, 0}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(component[0] == 0);
}

TEST_CASE("kosaraju - parallel edges", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Multiple edges between same vertices: 0->1 (twice), 1->0 (twice)
    Graph g({{0, 1}, {0, 1}, {1, 0}, {1, 0}});
    Graph g_t({{1, 0}, {1, 0}, {0, 1}, {0, 1}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // Multiple edges don't change SCC structure
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1}));
}

TEST_CASE("kosaraju - butterfly pattern", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Butterfly: two triangles sharing a vertex
    // Triangle 1: 0->1->2->0, Triangle 2: 2->3->4->2 (share vertex 2)
    Graph g({{0, 1}, {1, 2}, {2, 0},     // Triangle 1
             {2, 3}, {3, 4}, {4, 2}});   // Triangle 2
    Graph g_t({{1, 0}, {2, 1}, {0, 2},   // Triangle 1
               {3, 2}, {4, 3}, {2, 4}}); // Triangle 2
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    // All vertices in one SCC (connected through shared vertex 2)
    REQUIRE(count_unique_components(component) == 1);
    REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}

TEST_CASE("kosaraju - weakly connected but not strongly", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Weakly connected: 0->1->2, 2->3, but no back edges
    // If treated as undirected, would be one component
    // But as directed, each vertex is its own SCC
    Graph g({{0, 1}, {1, 2}, {2, 3}});
    Graph g_t({{1, 0}, {2, 1}, {3, 2}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 4);
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = i + 1; j < 4; ++j) {
            REQUIRE(different_components(component, i, j));
        }
    }
}

TEST_CASE("kosaraju - large component count verification", "[algorithm][kosaraju][scc][comprehensive]") {
    using Graph = vov_void;
    
    // Create graph with known SCC count
    // 3 SCCs: {0,1,2,3}, {4,5}, {6}
    Graph g({{0, 1}, {1, 2}, {2, 3}, {3, 0},  // SCC1: 4-cycle
             {3, 4},                           // Bridge
             {4, 5}, {5, 4},                  // SCC2: 2-cycle
             {5, 6}});                         // Bridge to isolated
    Graph g_t({{1, 0}, {2, 1}, {3, 2}, {0, 3},
               {4, 3},
               {5, 4}, {4, 5},
               {6, 5}});
    std::vector<uint32_t> component(num_vertices(g));
    
    kosaraju(g, g_t, component);
    
    REQUIRE(count_unique_components(component) == 3);
    REQUIRE(all_same_component(component, {0, 1, 2, 3}));
    REQUIRE(all_same_component(component, {4, 5}));
    // Vertex 6 is alone
    REQUIRE(different_components(component, 0, 6));
    REQUIRE(different_components(component, 4, 6));
}
