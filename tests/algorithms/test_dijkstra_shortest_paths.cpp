/**
 * @file test_dijkstra_shortest_paths.cpp
 * @brief Tests for Dijkstra's shortest path algorithms from dijkstra_shortest_paths.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// Simple visitor to count events (needs to be at namespace scope for member templates)
struct CountingVisitor {
  int vertices_discovered = 0;
  int vertices_examined   = 0;
  int edges_relaxed       = 0;
  int edges_not_relaxed   = 0;

  template <typename G, typename T>
  void on_discover_vertex(const G&, const T&) {
    ++vertices_discovered;
  }
  template <typename G, typename T>
  void on_examine_vertex(const G&, const T&) {
    ++vertices_examined;
  }
  template <typename G, typename T>
  void on_edge_relaxed(const G&, const T&) {
    ++edges_relaxed;
  }
  template <typename G, typename T>
  void on_edge_not_relaxed(const G&, const T&) {
    ++edges_not_relaxed;
  }
};

TEST_CASE("dijkstra_shortest_paths - CLRS example", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = vov_weighted;

  auto                            g = clrs_dijkstra_graph<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Validate against known results from CLRS Figure 24.6
  REQUIRE(distance[0] == clrs_dijkstra_results::distances_from_0[0]); // s: 0
  REQUIRE(distance[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
  REQUIRE(distance[2] == clrs_dijkstra_results::distances_from_0[2]); // x: 9
  REQUIRE(distance[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
  REQUIRE(distance[4] == clrs_dijkstra_results::distances_from_0[4]); // z: 7
}

TEST_CASE("dijkstra_shortest_paths - path graph", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Path: 0 -> 1 -> 2 -> 3 with weight 1 each
  for (size_t i = 0; i < path_graph_4_results::num_vertices; ++i) {
    REQUIRE(distance[i] == path_graph_4_results::distances[i]);
  }
}

TEST_CASE("dijkstra_shortest_distances - no predecessors", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = vov_weighted;

  auto             g = clrs_dijkstra_graph<Graph>();
  std::vector<int> distance(num_vertices(g));

  init_shortest_paths(distance);

  // Test distances-only variant (no predecessor tracking)
  dijkstra_shortest_distances(g, vertex_id_t<Graph>(0), distance,
                              [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Validate distances match expected results
  REQUIRE(distance[0] == clrs_dijkstra_results::distances_from_0[0]); // s: 0
  REQUIRE(distance[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
  REQUIRE(distance[2] == clrs_dijkstra_results::distances_from_0[2]); // x: 9
  REQUIRE(distance[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
  REQUIRE(distance[4] == clrs_dijkstra_results::distances_from_0[4]); // z: 7
}

TEST_CASE("dijkstra_shortest_paths - multi-source", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = vov_weighted;

  auto                            g = clrs_dijkstra_graph<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  // Start from vertices 0 and 3
  std::vector<vertex_id_t<Graph>> sources = {0, 3};

  dijkstra_shortest_paths(g, sources, distance, predecessor,
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Both source vertices should have distance 0
  REQUIRE(distance[0] == 0);
  REQUIRE(distance[3] == 0);

  // Other distances should be minimum from either source
  REQUIRE(distance[1] <= 8); // Can reach from source 0 or 3
  REQUIRE(distance[4] <= 7); // Can reach from source 0 or 3
}

TEST_CASE("dijkstra_shortest_distances - multi-source", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = vov_weighted;

  auto             g = clrs_dijkstra_graph<Graph>();
  std::vector<int> distance(num_vertices(g));

  init_shortest_paths(distance);

  // Start from vertices 0 and 3
  std::vector<vertex_id_t<Graph>> sources = {0, 3};

  dijkstra_shortest_distances(g, sources, distance, [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Both source vertices should have distance 0
  REQUIRE(distance[0] == 0);
  REQUIRE(distance[3] == 0);
}

TEST_CASE("dijkstra_shortest_paths - with visitor", "[algorithm][dijkstra_shortest_paths][visitor]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  CountingVisitor visitor;

  dijkstra_shortest_paths(
        g, vertex_id_t<Graph>(0), distance, predecessor,
        [](const auto& g, const auto& uv) { return edge_value(g, uv); }, visitor);

  // Verify visitor was called (should have discovered all 4 vertices, examined them, and relaxed edges)
  REQUIRE(visitor.vertices_discovered == 4);
  REQUIRE(visitor.vertices_examined == 4);
  REQUIRE(visitor.edges_relaxed == 3); // 3 edges in path graph
}

TEST_CASE("dijkstra_shortest_paths - unweighted graph (default weight)", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = std::vector<std::vector<int>>;

  // Create simple unweighted graph: 0 -> 1 -> 2 -> 3
  Graph g(4);
  g[0].push_back(1);
  g[1].push_back(2);
  g[2].push_back(3);

  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  // Use default weight function (returns 1 for all edges)
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor);

  REQUIRE(distance[0] == 0);
  REQUIRE(distance[1] == 1);
  REQUIRE(distance[2] == 2);
  REQUIRE(distance[3] == 3);
}

TEST_CASE("dijkstra_shortest_paths - predecessor path reconstruction", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Reconstruct path from 0 to 3: should be 0 -> 1 -> 2 -> 3
  std::vector<vertex_id_t<Graph>> path;
  auto                            current = vertex_id_t<Graph>(3);

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

TEST_CASE("dijkstra_shortest_paths - unreachable vertices", "[algorithm][dijkstra_shortest_paths]") {
  using Graph = std::vector<std::vector<int>>;

  // Create disconnected graph: 0 -> 1, 2 -> 3 (0,1 and 2,3 are separate)
  Graph g(4);
  g[0].push_back(1);
  g[2].push_back(3);

  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor);

  // Vertices 0 and 1 should be reachable
  REQUIRE(distance[0] == 0);
  REQUIRE(distance[1] == 1);

  // Vertices 2 and 3 should be unreachable (infinite distance)
  REQUIRE(distance[2] == std::numeric_limits<int>::max());
  REQUIRE(distance[3] == std::numeric_limits<int>::max());
}

// =============================================================================
// Vertex-ID Visitor Tests
// =============================================================================

// Visitor that accepts vertex ids instead of vertex descriptors
struct IdCountingVisitor {
  int vertices_discovered = 0;
  int vertices_examined   = 0;

  template <typename G>
  void on_discover_vertex(const G&, const vertex_id_t<G>&) {
    ++vertices_discovered;
  }
  template <typename G>
  void on_examine_vertex(const G&, const vertex_id_t<G>&) {
    ++vertices_examined;
  }
  template <typename G, typename T>
  void on_edge_relaxed(const G&, const T&) {}
};

TEST_CASE("dijkstra_shortest_paths - vertex id visitor", "[algorithm][dijkstra_shortest_paths][visitor_id]") {
  using Graph = vov_weighted;

  auto                            g = clrs_dijkstra_graph<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(distance, predecessor);

  IdCountingVisitor visitor;
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distance, predecessor,
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }, visitor);

  // All 5 vertices should be discovered via id-based callbacks
  // (examine may be called more than once per vertex due to re-enqueue)
  REQUIRE(visitor.vertices_discovered == 5);
  REQUIRE(visitor.vertices_examined >= 5);

  // Results should still be correct
  REQUIRE(distance[0] == 0);
  REQUIRE(distance[1] == clrs_dijkstra_results::distances_from_0[1]);
}

// =============================================================================
// Error / Exception Condition Tests
// =============================================================================

TEST_CASE("dijkstra_shortest_paths - distances too small throws", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph = vov_weighted;

  auto                            g = clrs_dijkstra_graph<Graph>(); // 5 vertices
  std::vector<int>                distances(2);                     // too small
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(distances, predecessor);

  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distances, predecessor,
                                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }),
                  std::out_of_range);
}

TEST_CASE("dijkstra_shortest_paths - predecessor too small throws", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph = vov_weighted;

  auto                            g = clrs_dijkstra_graph<Graph>(); // 5 vertices
  std::vector<int>                distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(2); // too small
  init_shortest_paths(distances, predecessor);

  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distances, predecessor,
                                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }),
                  std::out_of_range);
}

TEST_CASE("dijkstra_shortest_paths - source vertex out of range throws", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>(); // 4 vertices (ids 0-3)
  std::vector<int>                distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(distances, predecessor);

  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(99), distances, predecessor,
                                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }),
                  std::out_of_range);
}

TEST_CASE("dijkstra_shortest_paths - negative edge weight throws", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph = vov_weighted; // int edge values (signed)

  auto                            g = path_graph_4_weighted<Graph>(); // 0->1->2->3
  std::vector<int>                distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(distances, predecessor);

  // Weight function that always returns a negative value triggers the signed-weight guard
  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distances, predecessor,
                                          [](const auto&, const auto&) { return -1; }),
                  std::out_of_range);
}

TEST_CASE("dijkstra_shortest_paths - infinite weight edge triggers logic_error", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph         = vov_weighted;
  using distance_type = int;

  // When the edge weight equals the infinity sentinel, combine(0, INF) = INF which is
  // NOT strictly less than INF, so relax_target returns false for an undiscovered vertex.
  // The algorithm treats this as an internal invariant violation and throws std::logic_error.
  auto                            g = path_graph_4_weighted<Graph>(); // 0->1->2->3
  std::vector<distance_type>      distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(distances, predecessor);

  const auto INF = shortest_path_infinite_distance<distance_type>();
  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distances, predecessor,
                                          [INF](const auto&, const auto&) { return INF; }),
                  std::logic_error);
}

TEST_CASE("dijkstra_shortest_paths - on_edge_not_relaxed visitor callback", "[algorithm][dijkstra_shortest_paths][visitor]") {
  using Graph = vov_weighted;

  // The CLRS graph has multiple paths to the same vertex; revisiting an already-optimal
  // vertex triggers on_edge_not_relaxed (e.g. z->s, t->y, x->z are not relaxed).
  auto                            g = clrs_dijkstra_graph<Graph>();
  std::vector<int>                distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(distances, predecessor);

  CountingVisitor visitor;
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0), distances, predecessor,
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }, visitor);

  CHECK(visitor.edges_not_relaxed > 0);
  REQUIRE(distances[0] == 0);
  REQUIRE(distances[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
  REQUIRE(distances[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
}
