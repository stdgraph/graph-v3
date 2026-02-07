/**
 * @file test_bellman_ford_shortest_paths.cpp
 * @brief Tests for Bellman-Ford's shortest path algorithms from bellman_ford_shortest_paths.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/bellman_ford_shortest_paths.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// Simple visitor to count events (needs to be at namespace scope for member templates)
struct BellmanCountingVisitor {
    int edges_examined = 0;
    int edges_relaxed = 0;
    int edges_not_relaxed = 0;
    int edges_minimized = 0;
    int edges_not_minimized = 0;
    
    template<typename G, typename T> void on_examine_edge(const G&, const T&) { ++edges_examined; }
    template<typename G, typename T> void on_edge_relaxed(const G&, const T&) { ++edges_relaxed; }
    template<typename G, typename T> void on_edge_not_relaxed(const G&, const T&) { ++edges_not_relaxed; }
    template<typename G, typename T> void on_edge_minimized(const G&, const T&) { ++edges_minimized; }
    template<typename G, typename T> void on_edge_not_minimized(const G&, const T&) { ++edges_not_minimized; }
};

TEST_CASE("bellman_ford_shortest_paths - CLRS example", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    // Use same graph as Dijkstra (all non-negative weights)
    auto g = clrs_dijkstra_graph<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                                               [&g](const auto& uv) { return edge_value(g, uv); });
    
    // No negative cycle should be detected
    REQUIRE(!result.has_value());
    
    // Validate against known results from CLRS Figure 24.6
    REQUIRE(distance[0] == clrs_dijkstra_results::distances_from_0[0]); // s: 0
    REQUIRE(distance[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
    REQUIRE(distance[2] == clrs_dijkstra_results::distances_from_0[2]); // x: 9
    REQUIRE(distance[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
    REQUIRE(distance[4] == clrs_dijkstra_results::distances_from_0[4]); // z: 7
}

TEST_CASE("bellman_ford_shortest_paths - path graph", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    auto g = path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                                               [&g](const auto& uv) { return edge_value(g, uv); });
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Path: 0 -> 1 -> 2 -> 3 with weight 1 each
    for (size_t i = 0; i < path_graph_4_results::num_vertices; ++i) {
        REQUIRE(distance[i] == path_graph_4_results::distances[i]);
    }
}

TEST_CASE("bellman_ford_shortest_distances - no predecessors", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    auto g = clrs_dijkstra_graph<Graph>();
    std::vector<int> distance(num_vertices(g));
    
    init_shortest_paths(distance);
    
    // Test distances-only variant (no predecessor tracking)
    auto result = bellman_ford_shortest_distances(g, vertex_id_t<Graph>(0), distance,
                                                   [&g](const auto& uv) { return edge_value(g, uv); });
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Validate distances match expected results
    REQUIRE(distance[0] == clrs_dijkstra_results::distances_from_0[0]); // s: 0
    REQUIRE(distance[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
    REQUIRE(distance[2] == clrs_dijkstra_results::distances_from_0[2]); // x: 9
    REQUIRE(distance[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
    REQUIRE(distance[4] == clrs_dijkstra_results::distances_from_0[4]); // z: 7
}

TEST_CASE("bellman_ford_shortest_paths - multi-source", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    auto g = clrs_dijkstra_graph<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    // Start from vertices 0 and 3
    std::vector<vertex_id_t<Graph>> sources = {0, 3};
    
    auto result = bellman_ford_shortest_paths(g, sources, distance, predecessor,
                                               [&g](const auto& uv) { return edge_value(g, uv); });
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Both source vertices should have distance 0
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[3] == 0);
    
    // Other distances should be minimum from either source
    REQUIRE(distance[1] <= 8);  // Can reach from source 0 or 3
    REQUIRE(distance[4] <= 7);  // Can reach from source 0 or 3
}

TEST_CASE("bellman_ford_shortest_distances - multi-source", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    auto g = clrs_dijkstra_graph<Graph>();
    std::vector<int> distance(num_vertices(g));
    
    init_shortest_paths(distance);
    
    // Start from vertices 0 and 3
    std::vector<vertex_id_t<Graph>> sources = {0, 3};
    
    auto result = bellman_ford_shortest_distances(g, sources, distance,
                                                   [&g](const auto& uv) { return edge_value(g, uv); });
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Both source vertices should have distance 0
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[3] == 0);
}

TEST_CASE("bellman_ford_shortest_paths - with visitor", "[algorithm][bellman_ford_shortest_paths][visitor]") {
    using Graph = vov_weighted;
    
    auto g = path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    BellmanCountingVisitor visitor;
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                                               [&g](const auto& uv) { return edge_value(g, uv); },
                                               visitor);
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Verify visitor was called
    REQUIRE(visitor.edges_examined > 0);
    REQUIRE(visitor.edges_relaxed == 3); // 3 edges in path graph
}

TEST_CASE("bellman_ford_shortest_paths - unweighted graph (default weight)", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = std::vector<std::vector<int>>;
    
    // Create simple unweighted graph: 0 -> 1 -> 2 -> 3
    Graph g(4);
    g[0].push_back(1);
    g[1].push_back(2);
    g[2].push_back(3);
    
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    // Use default weight function (returns 1 for all edges)
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor);
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 1);
    REQUIRE(distance[2] == 2);
    REQUIRE(distance[3] == 3);
}

TEST_CASE("bellman_ford_shortest_paths - predecessor path reconstruction", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    auto g = path_graph_4_weighted<Graph>();
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                                               [&g](const auto& uv) { return edge_value(g, uv); });
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Reconstruct path from 0 to 3: should be 0 -> 1 -> 2 -> 3
    std::vector<vertex_id_t<Graph>> path;
    auto current = vertex_id_t<Graph>(3);
    
    while (current != vertex_id_t<Graph>(0)) {
        path.push_back(current);
        current = predecessor[current];
    }
    path.push_back(vertex_id_t<Graph>(0));
    
    std::reverse(path.begin(), path.end());
    
    REQUIRE(path.size() == 4);
    REQUIRE(path[0] == 0);
    REQUIRE(path[1] == 1);
    REQUIRE(path[2] == 2);
    REQUIRE(path[3] == 3);
}

TEST_CASE("bellman_ford_shortest_paths - unreachable vertices", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = std::vector<std::vector<int>>;
    
    // Create disconnected graph: 0 ->1, 2 is isolated, 3 has no incoming edges
    // Make it so vertex 2 and 3 have no connection to component containing 0 and 1
    Graph g(4);
    g[0].push_back(1);
    // Vertex 2 and 3 are completely disconnected - no edges
    
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor);
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Vertices 0 and 1 should be reachable
    REQUIRE(distance[0] == 0);
    REQUIRE(distance[1] == 1);
    
    // Vertices 2 and 3 should be unreachable - they remain at infinity
    // since there are NO edges from them to be examined
    constexpr int infinite = std::numeric_limits<int>::max();
    REQUIRE(distance[2] == infinite);
    REQUIRE(distance[3] == infinite);
}

TEST_CASE("bellman_ford_shortest_paths - negative weight cycle detection", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    // Create graph with negative weight cycle: 0 -> 1 -> 2 -> 0
    // Edges: (0,1,1), (1,2,1), (2,0,-3)
    // Total cycle weight: 1 + 1 + (-3) = -1 (negative!)
    Graph g({{0, 1, 1}, {1, 2, 1}, {2, 0, -3}});
    
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    BellmanCountingVisitor visitor;
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                                               [&g](const auto& uv) { return edge_value(g, uv); },
                                               visitor);
    
    // Negative cycle should be detected
    REQUIRE(result.has_value());
    
    // The returned vertex should be in the cycle (one of 0, 1, 2)
    REQUIRE((result.value() == 0 || result.value() == 1 || result.value() == 2));
    
    // Visitor should have seen a non-minimized edge
    REQUIRE(visitor.edges_not_minimized > 0);
}

TEST_CASE("bellman_ford_shortest_paths - find negative cycle vertices", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = vov_weighted;
    
    // Create graph with negative weight cycle: 0 -> 1 -> 2 -> 0
    Graph g({{0, 1, 1}, {1, 2, 1}, {2, 0, -3}});
    
    std::vector<int> distance(num_vertices(g));
    std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
    
    init_shortest_paths(distance, predecessor);
    
    auto cycle_vertex = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                                                     [&g](const auto& uv) { return edge_value(g, uv); });
    
    REQUIRE(cycle_vertex.has_value());
    
    // Extract the cycle vertices
    std::vector<vertex_id_t<Graph>> cycle;
    find_negative_cycle(g, predecessor, cycle_vertex, std::back_inserter(cycle));
    
    // Should have 3 vertices in the cycle
    REQUIRE(cycle.size() == 3);
    
    // All cycle vertices should be unique (before wrapping around)
    std::set<vertex_id_t<Graph>> unique_vertices(cycle.begin(), cycle.end());
    REQUIRE(unique_vertices.size() == 3);
}

TEST_CASE("bellman_ford_shortest_paths - empty graph", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = std::vector<std::vector<int>>;
    
    Graph g;
    std::vector<int> distance;
    std::vector<vertex_id_t<Graph>> predecessor;
    
    // Empty graph - nothing to test, just ensure it doesn't crash
    REQUIRE(num_vertices(g) == 0);
}

TEST_CASE("bellman_ford_shortest_paths - single vertex", "[algorithm][bellman_ford_shortest_paths]") {
    using Graph = std::vector<std::vector<int>>;
    
    Graph g(1);
    std::vector<int> distance(1);
    std::vector<vertex_id_t<Graph>> predecessor(1);
    
    init_shortest_paths(distance, predecessor);
    
    auto result = bellman_ford_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor);
    
    // No negative cycle
    REQUIRE(!result.has_value());
    
    // Distance to itself should be 0
    REQUIRE(distance[0] == 0);
}
