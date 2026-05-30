/**
 * @file test_visitor_factory.cpp
 * @brief Tests for the composable visitor utilities in visitor_factory.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/algorithm/visitor_factory.hpp>
#include <graph/algorithm/breadth_first_search.hpp>
#include <graph/algorithm/depth_first_search.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <vector>

using namespace graph;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// =============================================================================
// Compile-time integration with the strict visitor concept
// =============================================================================

namespace {
struct CountingChild {
  int discovered = 0;
  template <typename G, typename V>
  void on_discover_vertex(const G&, const V&) {
    ++discovered;
  }
};

// A sub-visitor that consumes the vertex id form of an event.
struct IdChild {
  std::vector<std::size_t>* out;
  template <typename G>
  void on_discover_vertex(const G&, const vertex_id_t<G>& uid) {
    out->push_back(static_cast<std::size_t>(uid));
  }
};
} // namespace

// A single-event adaptor is a valid visitor.
static_assert(valid_visitor<vov_void, decltype(on_discover_vertex([](auto&, auto&) {}))>,
              "single-event adaptor must be a valid visitor");
// A composite exposing at least one event is a valid visitor.
static_assert(valid_visitor<vov_void, composite_visitor<CountingChild>>,
              "composite with a handled event must be a valid visitor");
// The composite must advertise on_discover_vertex when a child handles it.
static_assert(has_on_discover_vertex<vov_void, composite_visitor<CountingChild>>,
              "composite must expose on_discover_vertex when a child provides it");
// The composite must NOT advertise events no child handles.
static_assert(!has_on_examine_edge<vov_void, composite_visitor<CountingChild>>,
              "composite must not expose events no child handles");

// =============================================================================
// Single-event adaptors
// =============================================================================

TEST_CASE("on_discover_vertex adaptor fires only on discovery", "[visitor_factory][adaptor]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  int discovered = 0;
  auto vis       = on_discover_vertex([&](const auto&, const auto&) { ++discovered; });

  breadth_first_search(g, 0u, vis);
  REQUIRE(discovered == 4);
}

TEST_CASE("on_examine_edge adaptor fires only on edge examination", "[visitor_factory][adaptor]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  int edges = 0;
  auto vis  = on_examine_edge([&](const auto&, const auto&) { ++edges; });

  breadth_first_search(g, 0u, vis);
  REQUIRE(edges == 3); // 3 edges in a 4-vertex path
}

// =============================================================================
// composite_visitor / make_visitor
// =============================================================================

TEST_CASE("make_visitor fans events out to multiple sub-visitors", "[visitor_factory][composite]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  int discovered = 0;
  int examined   = 0;
  int edges      = 0;

  auto vis = make_visitor(                                                      //
        on_discover_vertex([&](const auto&, const auto&) { ++discovered; }),    //
        on_examine_vertex([&](const auto&, const auto&) { ++examined; }),       //
        on_examine_edge([&](const auto&, const auto&) { ++edges; }));

  breadth_first_search(g, 0u, vis);

  REQUIRE(discovered == 4);
  REQUIRE(examined == 4);
  REQUIRE(edges == 3);
}

TEST_CASE("make_visitor forwards one event to several handlers", "[visitor_factory][composite]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  int a = 0;
  int b = 0;

  auto vis = make_visitor(                                              //
        on_discover_vertex([&](const auto&, const auto&) { ++a; }),     //
        on_discover_vertex([&](const auto&, const auto&) { ++b; }));

  breadth_first_search(g, 0u, vis);

  REQUIRE(a == 4);
  REQUIRE(b == 4);
}

TEST_CASE("composite bridges descriptor and id sub-visitors", "[visitor_factory][composite]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  std::vector<std::size_t> by_id;
  int                      by_desc = 0;

  // One child wants the vertex id, the other a descriptor; the composite must
  // satisfy both from a single event.
  auto vis = make_visitor(                                                 //
        IdChild{&by_id},                                                   //
        on_discover_vertex([&](const auto&, const auto&) { ++by_desc; }));

  breadth_first_search(g, 0u, vis);

  REQUIRE(by_desc == 4);
  REQUIRE(by_id == std::vector<std::size_t>{0, 1, 2, 3});
}

// =============================================================================
// Prebuilt recorders
// =============================================================================

TEST_CASE("predecessor_recorder records parents on tree edges (BFS)", "[visitor_factory][recorder]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  std::vector<vertex_id_t<Graph>> pred(num_vertices(g));
  for (std::size_t i = 0; i < pred.size(); ++i)
    pred[i] = static_cast<vertex_id_t<Graph>>(i);

  // BFS examines every edge; on a tree/path each examined edge is a discovery edge.
  auto vis = on_examine_edge(predecessor_recorder(pred));
  breadth_first_search(g, 0u, vis);

  REQUIRE(pred[1] == 0);
  REQUIRE(pred[2] == 1);
  REQUIRE(pred[3] == 2);
}

TEST_CASE("distance_recorder records hop counts (BFS)", "[visitor_factory][recorder]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  std::vector<int> dist(num_vertices(g), 0);

  auto vis = on_examine_edge(distance_recorder(dist));
  breadth_first_search(g, 0u, vis);

  REQUIRE(dist[0] == 0);
  REQUIRE(dist[1] == 1);
  REQUIRE(dist[2] == 2);
  REQUIRE(dist[3] == 3);
}

TEST_CASE("time_stamper assigns discovery order (DFS)", "[visitor_factory][recorder]") {
  using Graph = vov_void;
  auto g      = path_graph_4<Graph>();

  std::vector<int> dtime(num_vertices(g), -1);
  int              clock = 0;

  auto vis = on_discover_vertex(time_stamper(dtime, clock));
  depth_first_search(g, 0u, vis);

  // Linear chain: discovery times are strictly increasing along the path.
  REQUIRE(dtime[0] == 0);
  REQUIRE(dtime[1] == 1);
  REQUIRE(dtime[2] == 2);
  REQUIRE(dtime[3] == 3);
}

TEST_CASE("predecessor_recorder on edge_relaxed reconstructs Dijkstra tree", "[visitor_factory][recorder]") {
  using Graph = vov_weighted;
  auto g      = path_graph_4_weighted<Graph>();

  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(g, distance, predecessor);

  // Record predecessors independently via the visitor, in parallel with the
  // algorithm's own predecessor map.
  std::vector<vertex_id_t<Graph>> vis_pred(num_vertices(g));
  for (std::size_t i = 0; i < vis_pred.size(); ++i)
    vis_pred[i] = static_cast<vertex_id_t<Graph>>(i);

  auto vis = on_edge_relaxed(predecessor_recorder(vis_pred));

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
                          [](const auto& gr, const auto& uv) { return edge_value(gr, uv); },
                          vis);

  REQUIRE(vis_pred[1] == 0);
  REQUIRE(vis_pred[2] == 1);
  REQUIRE(vis_pred[3] == 2);
}
