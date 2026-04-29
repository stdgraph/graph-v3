#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <algorithm>
#include <limits>
#include <vector>

// ── BGL type aliases ────────────────────────────────────────────────────────

struct EdgeW { double weight; };

using bgl_bidir_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                           boost::no_property, EdgeW>;
using bgl_undir_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                           boost::no_property, EdgeW>;

using adapted_bidir_t = graph::bgl::graph_adaptor<bgl_bidir_t>;
using adapted_undir_t = graph::bgl::graph_adaptor<bgl_undir_t>;

// ── Concept checks ──────────────────────────────────────────────────────────

static_assert(graph::adj_list::adjacency_list<adapted_bidir_t>);
static_assert(graph::adj_list::index_adjacency_list<adapted_bidir_t>);
static_assert(graph::adj_list::bidirectional_adjacency_list<adapted_bidir_t>);
static_assert(graph::adj_list::index_bidirectional_adjacency_list<adapted_bidir_t>);

static_assert(graph::adj_list::adjacency_list<adapted_undir_t>);
static_assert(graph::adj_list::index_adjacency_list<adapted_undir_t>);
static_assert(graph::adj_list::bidirectional_adjacency_list<adapted_undir_t>);

// ── bidirectionalS tests ─────────────────────────────────────────────────────
//
// Graph:  0→1(5), 0→2(3), 2→1(2)
//
//   0 --5--> 1
//   |       /
//   3      2
//   v     /
//   2 ---

static bgl_bidir_t make_bidir_graph() {
  bgl_bidir_t g(3);
  boost::add_edge(0, 1, EdgeW{5.0}, g);
  boost::add_edge(0, 2, EdgeW{3.0}, g);
  boost::add_edge(2, 1, EdgeW{2.0}, g);
  return g;
}

TEST_CASE("in_edges on bidirectionalS adapted graph", "[bgl][bidirectional]") {
  auto bgl_g   = make_bidir_graph();
  auto ga      = graph::bgl::graph_adaptor(bgl_g);
  auto verts   = graph::vertices(ga);

  // Vertex 1 has two in-edges: 0→1 and 2→1
  auto u1 = *std::ranges::next(verts.begin(), 1);
  auto ie_range = graph::in_edges(ga, u1);

  std::vector<graph::vertex_id_t<adapted_bidir_t>> sources;
  for (auto ie : ie_range) {
    sources.push_back(graph::source_id(ga, ie));
  }
  std::ranges::sort(sources);
  REQUIRE(sources.size() == 2);
  CHECK(sources[0] == 0);
  CHECK(sources[1] == 2);

  // Vertex 0 has no in-edges
  auto u0       = *verts.begin();
  auto ie_empty = graph::in_edges(ga, u0);
  CHECK(std::ranges::distance(ie_empty) == 0);

  // Vertex 2 has one in-edge: 0→2
  auto u2      = *std::ranges::next(verts.begin(), 2);
  auto ie2     = graph::in_edges(ga, u2);
  REQUIRE(std::ranges::distance(ie2) == 1);
  // Re-iterate to check source
  for (auto ie : graph::in_edges(ga, u2)) {
    CHECK(graph::source_id(ga, ie) == 0);
    CHECK(graph::target_id(ga, ie) == 2);
  }
}

TEST_CASE("dijkstra on bidirectionalS adapted graph", "[bgl][bidirectional][dijkstra]") {
  // Graph: 0→1(5), 0→2(3), 2→1(2)  =>  d[0]=0, d[1]=5, d[2]=3
  auto bgl_g = make_bidir_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  const double inf = std::numeric_limits<double>::max();
  std::vector<double>                              dist(3, inf);
  std::vector<graph::vertex_id_t<adapted_bidir_t>> pred(3);
  for (std::size_t i = 0; i < 3; ++i) pred[i] = i;

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(pred);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(
    boost::get(&EdgeW::weight, bgl_g));

  std::vector<graph::vertex_id_t<adapted_bidir_t>> sources = {0};
  dist_fn(ga, graph::vertex_id_t<adapted_bidir_t>{0}) = 0.0;

  graph::dijkstra_shortest_paths(ga, sources, dist_fn, pred_fn, weight_fn);

  CHECK(dist[0] == 0.0);
  CHECK(dist[1] == 5.0);  // 0→1(5), or 0→2(3)+2→1(2)=5 — either path
  CHECK(dist[2] == 3.0);  // 0→2(3)
}

// ── undirectedS tests ────────────────────────────────────────────────────────
//
// Graph (undirected):  0--1(4), 0--2(2), 1--2(1), 1--3(5), 2--3(8)
// Shortest paths from 0: d[0]=0, d[1]=3 (0-2-1), d[2]=2, d[3]=8 (0-2-1-3)

static bgl_undir_t make_undir_graph() {
  bgl_undir_t g(4);
  boost::add_edge(0, 1, EdgeW{4.0}, g);
  boost::add_edge(0, 2, EdgeW{2.0}, g);
  boost::add_edge(1, 2, EdgeW{1.0}, g);
  boost::add_edge(1, 3, EdgeW{5.0}, g);
  boost::add_edge(2, 3, EdgeW{8.0}, g);
  return g;
}

TEST_CASE("in_edges == out_edges for undirectedS adapted graph", "[bgl][undirected]") {
  auto bgl_g = make_undir_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);
  auto verts = graph::vertices(ga);

  // For undirected, in_edges and out_edges have the same count per vertex
  for (auto u : verts) {
    auto oe_count = std::ranges::distance(graph::out_edges(ga, u));
    auto ie_count = std::ranges::distance(graph::in_edges(ga, u));
    CHECK(oe_count == ie_count);
  }

  // Vertex 0 is incident to 2 edges: 0-1, 0-2
  auto u0 = *verts.begin();
  CHECK(std::ranges::distance(graph::in_edges(ga, u0)) == 2);

  // Vertex 1 is incident to 3 edges: 0-1, 1-2, 1-3
  auto u1 = *std::ranges::next(verts.begin(), 1);
  CHECK(std::ranges::distance(graph::in_edges(ga, u1)) == 3);
}

TEST_CASE("dijkstra on undirectedS adapted graph", "[bgl][undirected][dijkstra]") {
  auto bgl_g = make_undir_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  const double inf = std::numeric_limits<double>::max();
  std::vector<double>                              dist(4, inf);
  std::vector<graph::vertex_id_t<adapted_undir_t>> pred(4);
  for (std::size_t i = 0; i < 4; ++i) pred[i] = i;

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(pred);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(
    boost::get(&EdgeW::weight, bgl_g));

  std::vector<graph::vertex_id_t<adapted_undir_t>> sources = {0};
  dist_fn(ga, graph::vertex_id_t<adapted_undir_t>{0}) = 0.0;

  graph::dijkstra_shortest_paths(ga, sources, dist_fn, pred_fn, weight_fn);

  CHECK(dist[0] == 0.0);
  CHECK(dist[1] == 3.0);  // 0→2(2)→1(1)
  CHECK(dist[2] == 2.0);  // 0→2(2)
  CHECK(dist[3] == 8.0);  // 0→2(2)→1(1)→3(5)
}

TEST_CASE("edgelist view on undirectedS adapted graph", "[bgl][undirected][edgelist]") {
  auto bgl_g = make_undir_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  // Collect all (source, target) pairs from edgelist view
  using pair_t = std::pair<std::size_t, std::size_t>;
  std::vector<pair_t> edge_pairs;
  for (auto&& [sid, tid, uv] : graph::views::edgelist(ga)) {
    edge_pairs.emplace_back(sid, tid);
  }

  // Undirected graph: each edge appears twice in the adjacency structure
  // (once from each endpoint), so edgelist (which iterates all out_edges
  // across all vertices) yields 2 * num_edges entries.
  // 5 undirected edges → 10 directed entries
  CHECK(edge_pairs.size() == 10);

  // Normalize pairs so smaller vertex is first, then count unique
  std::vector<pair_t> normalized;
  for (auto [s, t] : edge_pairs) {
    normalized.emplace_back(std::min(s, t), std::max(s, t));
  }
  std::ranges::sort(normalized);
  auto unique_end = std::ranges::unique(normalized);
  normalized.erase(unique_end.begin(), unique_end.end());

  std::vector<pair_t> expected = {{0,1}, {0,2}, {1,2}, {1,3}, {2,3}};
  CHECK(normalized == expected);
}

TEST_CASE("dijkstra on undirectedS: cross-check against BGL", "[bgl][undirected][dijkstra][bgl-crosscheck]") {
  auto bgl_g = make_undir_graph();

  using vertex_t = boost::graph_traits<bgl_undir_t>::vertex_descriptor;
  const std::size_t n = boost::num_vertices(bgl_g);

  // --- Run BGL's own Dijkstra ---
  std::vector<double> bgl_dist(n, std::numeric_limits<double>::max());
  std::vector<vertex_t> bgl_pred(n);
  for (std::size_t i = 0; i < n; ++i) bgl_pred[i] = i;

  boost::dijkstra_shortest_paths(bgl_g, vertex_t(0),
    boost::predecessor_map(bgl_pred.data())
    .distance_map(bgl_dist.data())
    .weight_map(boost::get(&EdgeW::weight, bgl_g)));

  // --- Run graph-v3 Dijkstra on the adapted graph ---
  auto ga = graph::bgl::graph_adaptor(bgl_g);

  std::vector<double>                              g3_dist(n, std::numeric_limits<double>::max());
  std::vector<graph::vertex_id_t<adapted_undir_t>> g3_pred(n);
  for (std::size_t i = 0; i < n; ++i) g3_pred[i] = i;

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(g3_dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(g3_pred);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(
    boost::get(&EdgeW::weight, bgl_g));

  std::vector<graph::vertex_id_t<adapted_undir_t>> sources = {0};
  dist_fn(ga, graph::vertex_id_t<adapted_undir_t>{0}) = 0.0;

  graph::dijkstra_shortest_paths(ga, sources, dist_fn, pred_fn, weight_fn);

  // --- Compare: distances must match exactly ---
  for (std::size_t i = 0; i < n; ++i) {
    CHECK(g3_dist[i] == bgl_dist[i]);
  }

  // Predecessor trees may differ when ties exist, but the induced
  // distances must be consistent: dist[pred[v]] + weight(pred[v]→v) == dist[v]
  // (We already checked distances match, so equal predecessors is sufficient
  // when the shortest-path tree is unique.)
  for (std::size_t i = 0; i < n; ++i) {
    CHECK(g3_pred[i] == bgl_pred[i]);
  }
}

