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

  init_shortest_paths(g, distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
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

  init_shortest_paths(g, distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
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

  init_shortest_paths(g, distance);

  // Test distances-only variant (no predecessor tracking)
  dijkstra_shortest_distances(g, vertex_id_t<Graph>(0),
                              container_value_fn(distance),
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

  init_shortest_paths(g, distance, predecessor);

  // Start from vertices 0 and 3
  std::vector<vertex_id_t<Graph>> sources = {0, 3};

  dijkstra_shortest_paths(g, sources,
                          container_value_fn(distance),
                          container_value_fn(predecessor),
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

  init_shortest_paths(g, distance);

  // Start from vertices 0 and 3
  std::vector<vertex_id_t<Graph>> sources = {0, 3};

  dijkstra_shortest_distances(g, sources,
                              container_value_fn(distance),
                              [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Both source vertices should have distance 0
  REQUIRE(distance[0] == 0);
  REQUIRE(distance[3] == 0);
}

TEST_CASE("dijkstra_shortest_paths - with visitor", "[algorithm][dijkstra_shortest_paths][visitor]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));

  init_shortest_paths(g, distance, predecessor);

  CountingVisitor visitor;

  dijkstra_shortest_paths(
        g, vertex_id_t<Graph>(0),
        container_value_fn(distance),
        container_value_fn(predecessor),
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

  init_shortest_paths(g, distance, predecessor);

  // Use default weight function (returns 1 for all edges)
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor));

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

  init_shortest_paths(g, distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
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

  init_shortest_paths(g, distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor));

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

  init_shortest_paths(g, distance, predecessor);

  IdCountingVisitor visitor;
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
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

TEST_CASE("dijkstra_shortest_paths - source vertex out of range throws", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>(); // 4 vertices (ids 0-3)
  std::vector<int>                distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(g, distances, predecessor);

  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(99),
                                          container_value_fn(distances),
                                          container_value_fn(predecessor),
                                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }),
                  std::out_of_range);
}

TEST_CASE("dijkstra_shortest_paths - negative edge weight throws", "[algorithm][dijkstra_shortest_paths][error]") {
  using Graph = vov_weighted; // int edge values (signed)

  auto                            g = path_graph_4_weighted<Graph>(); // 0->1->2->3
  std::vector<int>                distances(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(g, distances, predecessor);

  // Weight function that always returns a negative value triggers the signed-weight guard
  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                                          container_value_fn(distances),
                                          container_value_fn(predecessor),
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
  init_shortest_paths(g, distances, predecessor);

  const auto INF = shortest_path_infinite_distance<distance_type>();
  CHECK_THROWS_AS(dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                                          container_value_fn(distances),
                                          container_value_fn(predecessor),
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
  init_shortest_paths(g, distances, predecessor);

  CountingVisitor visitor;
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distances),
                          container_value_fn(predecessor),
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }, visitor);

  CHECK(visitor.edges_not_relaxed > 0);
  REQUIRE(distances[0] == 0);
  REQUIRE(distances[1] == clrs_dijkstra_results::distances_from_0[1]); // t: 8
  REQUIRE(distances[3] == clrs_dijkstra_results::distances_from_0[3]); // y: 5
}

// =============================================================================
// Sparse (Map-Based) Graph Tests
// =============================================================================

#include "../common/map_graph_fixtures.hpp"
#include <graph/adj_list/vertex_property_map.hpp>

using namespace graph::test::map_fixtures;

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - sparse CLRS example",
                   "[algorithm][dijkstra_shortest_paths][sparse]",
                   SPARSE_VERTEX_TYPES) {
  using Graph       = TestType;
  using id_type     = vertex_id_t<Graph>;
  using DistMap     = decltype(make_vertex_property_map<Graph, int>(std::declval<const Graph&>(), 0));
  using PredMap     = decltype(make_vertex_property_map<Graph, id_type>(std::declval<const Graph&>(), id_type{}));
  const auto& exp   = clrs_dijkstra_sparse_expected{};

  auto g            = map_fixtures::clrs_dijkstra_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  // Initialize predecessors: each vertex points to itself
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  dijkstra_shortest_paths(g, id_type(exp.s),
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Validate distances against known CLRS results
  for (size_t i = 0; i < exp.num_vertices; ++i) {
    REQUIRE(distances[exp.vertex_ids[i]] == exp.distances[i]);
  }

  // Source predecessor is itself
  REQUIRE(predecessors[exp.s] == exp.s);
}

TEMPLATE_TEST_CASE("dijkstra_shortest_distances - sparse CLRS example",
                   "[algorithm][dijkstra_shortest_paths][sparse]",
                   SPARSE_VERTEX_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;
  const auto& exp = clrs_dijkstra_sparse_expected{};

  auto g         = map_fixtures::clrs_dijkstra_graph<Graph>();
  auto distances = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());

  dijkstra_shortest_distances(g, id_type(exp.s),
                              container_value_fn(distances),
                              [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  for (size_t i = 0; i < exp.num_vertices; ++i) {
    REQUIRE(distances[exp.vertex_ids[i]] == exp.distances[i]);
  }
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - sparse multi-source",
                   "[algorithm][dijkstra_shortest_paths][sparse][multi_source]",
                   SPARSE_VERTEX_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;
  const auto& exp = clrs_dijkstra_sparse_expected{};

  auto g            = map_fixtures::clrs_dijkstra_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  // Start from s (10) and y (40)
  std::vector<id_type> sources = {exp.s, exp.y};
  dijkstra_shortest_paths(g, sources,
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Both sources should have distance 0
  REQUIRE(distances[exp.s] == 0);
  REQUIRE(distances[exp.y] == 0);

  // All vertices should be reachable
  for (size_t i = 0; i < exp.num_vertices; ++i) {
    REQUIRE(distances[exp.vertex_ids[i]] < shortest_path_infinite_distance<int>());
  }
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - sparse with visitor",
                   "[algorithm][dijkstra_shortest_paths][sparse][visitor]",
                   SPARSE_VERTEX_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;
  const auto& exp = clrs_dijkstra_sparse_expected{};

  auto g            = map_fixtures::clrs_dijkstra_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  CountingVisitor visitor;
  dijkstra_shortest_paths(
        g, id_type(exp.s),
        container_value_fn(distances),
        container_value_fn(predecessors),
        [](const auto& g, const auto& uv) { return edge_value(g, uv); }, visitor);

  REQUIRE(visitor.vertices_discovered == static_cast<int>(exp.num_vertices));
  REQUIRE(visitor.vertices_examined >= static_cast<int>(exp.num_vertices));
  REQUIRE(visitor.edges_relaxed > 0);
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - sparse source not in graph throws",
                   "[algorithm][dijkstra_shortest_paths][sparse][error]",
                   SPARSE_VERTEX_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;

  auto g            = map_fixtures::clrs_dijkstra_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  // Vertex ID 999 does not exist in the sparse graph
  CHECK_THROWS_AS(dijkstra_shortest_paths(g, id_type(999),
                                          container_value_fn(distances),
                                          container_value_fn(predecessors),
                                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }),
                  std::out_of_range);
}

// =============================================================================
// Non-Integral Vertex ID Tests (String IDs)
//
// These tests verify Dijkstra works with mapped_adjacency_list graphs whose
// vertex_id_t is std::string — a non-integral, non-arithmetic key type.
// =============================================================================

#include <graph/container/dynamic_graph.hpp>

// String-VId graph types with int edge weights (EV=int, VV=void, GV=void, VId=string)
using mov_string_int = graph::container::dynamic_graph<
      int, void, void, std::string, false,
      graph::container::mov_graph_traits<int, void, void, std::string, false>>;

using uov_string_int = graph::container::dynamic_graph<
      int, void, void, std::string, false,
      graph::container::uov_graph_traits<int, void, void, std::string, false>>;

using mos_string_int = graph::container::dynamic_graph<
      int, void, void, std::string, false,
      graph::container::mos_graph_traits<int, void, void, std::string, false>>;

// CLRS Dijkstra graph with string vertex IDs: s, t, x, y, z
// Same topology/weights as CLRS Figure 24.6
template <typename Graph>
Graph clrs_dijkstra_string_graph() {
  using S = std::string;
  return Graph({
        {S("s"), S("t"), 10}, {S("s"), S("y"), 5},   // s -> t(10), s -> y(5)
        {S("t"), S("x"), 1},  {S("t"), S("y"), 2},   // t -> x(1),  t -> y(2)
        {S("x"), S("z"), 4},                          // x -> z(4)
        {S("y"), S("t"), 3},  {S("y"), S("x"), 9},
        {S("y"), S("z"), 2},                          // y -> t(3),  y -> x(9), y -> z(2)
        {S("z"), S("s"), 7},  {S("z"), S("x"), 6},   // z -> s(7),  z -> x(6)
  });
}

// Expected shortest distances from "s"
struct clrs_string_expected {
  static constexpr size_t num_vertices = 5;
  // Ordered by CLRS labeling: s=0, t=8, x=9, y=5, z=7
  static inline const std::vector<std::pair<std::string, int>> distances = {
        {"s", 0}, {"t", 8}, {"x", 9}, {"y", 5}, {"z", 7}};
  static inline const std::string source = "s";
};

#define STRING_VID_TYPES mov_string_int, uov_string_int, mos_string_int

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - string vertex IDs",
                   "[algorithm][dijkstra_shortest_paths][string_id]",
                   STRING_VID_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;
  static_assert(std::is_same_v<id_type, std::string>, "vertex_id_t must be std::string");

  auto g            = clrs_dijkstra_string_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  dijkstra_shortest_paths(g, std::string("s"),
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Validate all distances
  for (const auto& [vid, expected_dist] : clrs_string_expected::distances) {
    REQUIRE(distances[vid] == expected_dist);
  }

  // Source predecessor is itself
  REQUIRE(predecessors[std::string("s")] == std::string("s"));

  // Verify predecessor chain for t: t's predecessor should be y (path s->y->t, cost 5+3=8)
  REQUIRE(predecessors[std::string("t")] == std::string("y"));
}

TEMPLATE_TEST_CASE("dijkstra_shortest_distances - string vertex IDs",
                   "[algorithm][dijkstra_shortest_paths][string_id]",
                   STRING_VID_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;

  auto g         = clrs_dijkstra_string_graph<Graph>();
  auto distances = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());

  dijkstra_shortest_distances(g, std::string("s"),
                              container_value_fn(distances),
                              [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  for (const auto& [vid, expected_dist] : clrs_string_expected::distances) {
    REQUIRE(distances[vid] == expected_dist);
  }
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - string vertex IDs multi-source",
                   "[algorithm][dijkstra_shortest_paths][string_id][multi_source]",
                   STRING_VID_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;

  auto g            = clrs_dijkstra_string_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  // Start from "s" and "y"
  std::vector<id_type> sources = {std::string("s"), std::string("y")};
  dijkstra_shortest_paths(g, sources,
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Both sources should have distance 0
  REQUIRE(distances[std::string("s")] == 0);
  REQUIRE(distances[std::string("y")] == 0);

  // All vertices should be reachable
  for (const auto& [vid, _] : clrs_string_expected::distances) {
    REQUIRE(distances[vid] < shortest_path_infinite_distance<int>());
  }
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - string vertex IDs with visitor",
                   "[algorithm][dijkstra_shortest_paths][string_id][visitor]",
                   STRING_VID_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;

  auto g            = clrs_dijkstra_string_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  CountingVisitor visitor;
  dijkstra_shortest_paths(
        g, std::string("s"),
        container_value_fn(distances),
        container_value_fn(predecessors),
        [](const auto& g, const auto& uv) { return edge_value(g, uv); }, visitor);

  REQUIRE(visitor.vertices_discovered == static_cast<int>(clrs_string_expected::num_vertices));
  REQUIRE(visitor.vertices_examined >= static_cast<int>(clrs_string_expected::num_vertices));
  REQUIRE(visitor.edges_relaxed > 0);
  REQUIRE(visitor.edges_not_relaxed > 0);
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - string vertex IDs invalid source throws",
                   "[algorithm][dijkstra_shortest_paths][string_id][error]",
                   STRING_VID_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;

  auto g            = clrs_dijkstra_string_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  // "nonexistent" is not a vertex in the graph
  CHECK_THROWS_AS(dijkstra_shortest_paths(g, std::string("nonexistent"),
                                          container_value_fn(distances),
                                          container_value_fn(predecessors),
                                          [](const auto& g, const auto& uv) { return edge_value(g, uv); }),
                  std::out_of_range);
}

TEMPLATE_TEST_CASE("dijkstra_shortest_paths - string vertex IDs path reconstruction",
                   "[algorithm][dijkstra_shortest_paths][string_id]",
                   STRING_VID_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;

  auto g            = clrs_dijkstra_string_graph<Graph>();
  auto distances    = make_vertex_property_map<Graph, int>(g, shortest_path_infinite_distance<int>());
  auto predecessors = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g))
    predecessors[uid] = uid;

  dijkstra_shortest_paths(g, std::string("s"),
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          [](const auto& g, const auto& uv) { return edge_value(g, uv); });

  // Reconstruct path from "s" to "x": should be s -> y -> t -> x (cost 5+3+1=9)
  std::vector<std::string> path;
  std::string              current = "x";
  while (current != "s") {
    path.push_back(current);
    current = predecessors[current];
  }
  path.push_back("s");
  std::reverse(path.begin(), path.end());

  REQUIRE(path.size() == 4);
  REQUIRE(path[0] == "s");
  REQUIRE(path[1] == "y");
  REQUIRE(path[2] == "t");
  REQUIRE(path[3] == "x");
}
