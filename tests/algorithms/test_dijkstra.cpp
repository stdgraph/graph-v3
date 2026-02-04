/**
 * @file test_dijkstra.cpp
 * @brief Tests for Dijkstra's shortest path algorithm
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/dijkstra.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"

using namespace graph;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

TEST_CASE("Dijkstra correctness - CLRS example", "[algorithm][dijkstra]") {
    using Graph = vov_weighted;
    
    auto g = clrs_dijkstra_graph<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    // Validate against known results from CLRS Figure 24.6
    REQUIRE(distance[0] == clrs_dijkstra_results::distances_from_0[0]); // s: 0
    REQUIRE(distance[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
    REQUIRE(distance[2] == clrs_dijkstra_results::distances_from_0[2]); // x: 9
    REQUIRE(distance[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
    REQUIRE(distance[4] == clrs_dijkstra_results::distances_from_0[4]); // z: 7
}

TEST_CASE("Dijkstra correctness - path graph", "[algorithm][dijkstra]") {
    using Graph = vov_weighted;
    
    auto g = path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    // Path: 0 -> 1 -> 2 -> 3 with weight 1 each
    for (size_t i = 0; i < path_graph_4_results::num_vertices; ++i) {
        REQUIRE(distance[i] == path_graph_4_results::distances[i]);
    }
}

TEST_CASE("Dijkstra correctness - diamond DAG", "[algorithm][dijkstra]") {
    using Graph = vov_weighted;
    
    auto g = diamond_dag_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    // Shortest paths: 0->1: 5, 0->2: 3, 0->1->3: 7, 0->2->3: 10
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 5);
    REQUIRE(distance[2] == 3);
    REQUIRE(distance[3] == 7);  // min(5+2, 3+7) = 7 via vertex 1
}

TEST_CASE("Dijkstra edge cases", "[algorithm][dijkstra]") {
    SECTION("single vertex") {
        using Graph = vov_weighted;
        auto g = single_vertex<Graph>();
        std::vector<int> distance(num_vertices(g));
        std::vector<uint32_t> predecessor(num_vertices(g));
        
        dijkstra(g, 0, distance, predecessor);
        
        REQUIRE(distance[0] == 0);
    }
    
    SECTION("single edge") {
        using Graph = vov_weighted;
        auto g = single_edge_weighted<Graph>();
        std::vector<int> distance(num_vertices(g));
        std::vector<uint32_t> predecessor(num_vertices(g));
        
        dijkstra(g, 0, distance, predecessor,
                 [&g](const auto& uv) { return edge_value(g, uv); });
        
        REQUIRE(distance[0] == 0);
        REQUIRE(distance[1] == 10);
        REQUIRE(predecessor[1] == 0);
    }
    
    SECTION("disconnected graph") {
        using Graph = vov_weighted;
        auto g = disconnected_graph<Graph>();
        std::vector<int> distance(num_vertices(g));
        std::vector<uint32_t> predecessor(num_vertices(g));
        
        dijkstra(g, 0, distance, predecessor);
        
        // Component 1: vertices 0, 1 (reachable)
        REQUIRE(distance[0] == 0);
        REQUIRE(distance[1] == 1);
        
        // Component 2: vertices 2, 3, 4 (unreachable from 0)
        REQUIRE(distance[2] == std::numeric_limits<int>::max());
        REQUIRE(distance[3] == std::numeric_limits<int>::max());
        REQUIRE(distance[4] == std::numeric_limits<int>::max());
    }
}

TEST_CASE("Dijkstra without predecessors", "[algorithm][dijkstra]") {
    using Graph = vov_weighted;
    
    auto g = path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    
    // Use overload without predecessor tracking
    dijkstra(g, 0, distance,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    for (size_t i = 0; i < path_graph_4_results::num_vertices; ++i) {
        REQUIRE(distance[i] == path_graph_4_results::distances[i]);
    }
}

TEST_CASE("Dijkstra with null_predecessors", "[algorithm][dijkstra]") {
    using Graph = vov_weighted;
    
    auto g = clrs_dijkstra_graph<Graph>();
    std::vector<int> distance(num_vertices(g));
    
    dijkstra(g, 0, distance, _null_predecessors,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    for (size_t i = 0; i < clrs_dijkstra_results::num_vertices; ++i) {
        REQUIRE(distance[i] == clrs_dijkstra_results::distances_from_0[i]);
    }
}

TEST_CASE("Dijkstra unweighted (default weight)", "[algorithm][dijkstra]") {
    using Graph = vov_void;
    
    auto g = path_graph_4<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    // Use default weight function (returns 1 for all edges)
    dijkstra(g, 0, distance, predecessor);
    
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 1);
    REQUIRE(distance[2] == 2);
    REQUIRE(distance[3] == 3);
}

// Parameterized tests across container types
TEMPLATE_TEST_CASE("Dijkstra container compatibility - basic",
                   "[algorithm][dijkstra][template]",
                   BASIC_WEIGHTED_TYPES) {
    using Graph = TestType;
    
    auto g = path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    for (size_t i = 0; i < path_graph_4_results::num_vertices; ++i) {
        REQUIRE(distance[i] == path_graph_4_results::distances[i]);
    }
}

TEMPLATE_TEST_CASE("Dijkstra container compatibility - comprehensive",
                   "[algorithm][dijkstra][comprehensive]",
                   ALL_DIRECTED_WEIGHTED_TYPES) {
    using Graph = TestType;
    
    auto g = diamond_dag_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 5);
    REQUIRE(distance[2] == 3);
    REQUIRE(distance[3] == 7);
}

TEST_CASE("Dijkstra cycle graph", "[algorithm][dijkstra]") {
    using Graph = vov_weighted;
    
    auto g = cycle_graph_5_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor,
             [&g](const auto& uv) { return edge_value(g, uv); });
    
    // In a cycle with uniform weights, shortest paths go clockwise
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 1);
    REQUIRE(distance[2] == 2);
    REQUIRE(distance[3] == 3);
    REQUIRE(distance[4] == 4);
}

TEST_CASE("Dijkstra complete graph", "[algorithm][dijkstra]") {
    using Graph = vov_void;
    
    auto g = complete_graph_4<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<uint32_t> predecessor(num_vertices(g));
    
    dijkstra(g, 0, distance, predecessor);
    
    // All vertices directly connected with weight 1
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 1);
    REQUIRE(distance[2] == 1);
    REQUIRE(distance[3] == 1);
}
