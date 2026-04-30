#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <limits>
#include <vector>

// ── BGL CSR type aliases ────────────────────────────────────────────────────

struct CsrEdgeProp {
  double weight;
};

using bgl_csr_t   = boost::compressed_sparse_row_graph<boost::directedS,
                                                        boost::no_property,
                                                        CsrEdgeProp>;
using adapted_csr_t = graph::bgl::graph_adaptor<bgl_csr_t>;

// ── Concept checks ──────────────────────────────────────────────────────────

static_assert(graph::adj_list::adjacency_list<adapted_csr_t>);
static_assert(graph::adj_list::index_adjacency_list<adapted_csr_t>);

// ── Helper: build a 5-vertex CLRS-style graph ───────────────────────────────
//
// Edges (sorted by source): 0→1(10), 0→2(5), 0→4(2),
//                            1→3(1),
//                            2→1(3), 2→3(9), 2→4(2),
//                            3→4(7),
//                            4→3(3)
//
// Shortest paths from 0: d[0]=0, d[1]=8, d[2]=5, d[3]=5, d[4]=2

static bgl_csr_t make_csr_graph() {
  using edge_t = std::pair<std::size_t, std::size_t>;
  // Edges MUST be sorted by source for CSR construction
  std::vector<edge_t> edges = {
    {0,1}, {0,2}, {0,4},
    {1,3},
    {2,1}, {2,3}, {2,4},
    {3,4},
    {4,3}
  };
  std::vector<CsrEdgeProp> props = {
    {10.0}, {5.0}, {2.0},
    {1.0},
    {3.0}, {9.0}, {2.0},
    {7.0},
    {3.0}
  };
  return bgl_csr_t(boost::edges_are_sorted, edges.begin(), edges.end(),
                    props.begin(), 5);
}

// ── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("CSR adapted graph: vertexlist view", "[bgl][csr]") {
  auto bgl_g = make_csr_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  std::vector<std::size_t> ids;
  for (auto&& [uid, u] : graph::views::vertexlist(ga)) {
    ids.push_back(uid);
  }
  REQUIRE(ids.size() == 5);
  CHECK(ids == std::vector<std::size_t>{0, 1, 2, 3, 4});
}

TEST_CASE("CSR adapted graph: incidence view", "[bgl][csr]") {
  auto bgl_g = make_csr_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  // Vertex 0 out-edges: 0→1, 0→2, 0→4
  auto u0 = *graph::find_vertex(ga, std::size_t(0));
  std::vector<std::size_t> targets;
  for (auto&& [tid, uv] : graph::views::incidence(ga, u0)) {
    targets.push_back(tid);
  }
  std::ranges::sort(targets);
  CHECK(targets == std::vector<std::size_t>{1, 2, 4});

  // Vertex 2 out-edges: 2→1, 2→3, 2→4
  auto u2 = *graph::find_vertex(ga, std::size_t(2));
  targets.clear();
  for (auto&& [tid, uv] : graph::views::incidence(ga, u2)) {
    targets.push_back(tid);
  }
  std::ranges::sort(targets);
  CHECK(targets == std::vector<std::size_t>{1, 3, 4});
}

TEST_CASE("CSR adapted graph: edgelist view", "[bgl][csr]") {
  auto bgl_g = make_csr_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  using pair_t = std::pair<std::size_t, std::size_t>;
  std::vector<pair_t> edge_pairs;
  for (auto&& [sid, tid, uv] : graph::views::edgelist(ga)) {
    edge_pairs.emplace_back(sid, tid);
  }
  std::ranges::sort(edge_pairs);

  std::vector<pair_t> expected = {
    {0,1}, {0,2}, {0,4},
    {1,3},
    {2,1}, {2,3}, {2,4},
    {3,4},
    {4,3}
  };
  CHECK(edge_pairs == expected);
}

TEST_CASE("CSR adapted graph: dijkstra_shortest_paths", "[bgl][csr][dijkstra]") {
  auto bgl_g = make_csr_graph();
  auto ga    = graph::bgl::graph_adaptor(bgl_g);

  const std::size_t n   = graph::num_vertices(ga);
  const double      inf = std::numeric_limits<double>::max();

  std::vector<double>      dist(n, inf);
  std::vector<std::size_t> pred(n);
  for (std::size_t i = 0; i < n; ++i) pred[i] = i;

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(pred);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(
    boost::get(&CsrEdgeProp::weight, bgl_g));

  std::vector<std::size_t> sources = {0};
  dist_fn(ga, std::size_t{0}) = 0.0;

  graph::dijkstra_shortest_paths(ga, sources, dist_fn, pred_fn, weight_fn);

  CHECK(dist[0] == 0.0);
  CHECK(dist[1] == 8.0);  // 0→2(5)→1(3)
  CHECK(dist[2] == 5.0);  // 0→2(5)
  CHECK(dist[3] == 5.0);  // 0→4(2)→3(3)
  CHECK(dist[4] == 2.0);  // 0→4(2)
}

TEST_CASE("CSR adapted graph: dijkstra cross-check against BGL", "[bgl][csr][dijkstra][bgl-crosscheck]") {
  auto bgl_g = make_csr_graph();

  using vertex_t = boost::graph_traits<bgl_csr_t>::vertex_descriptor;
  const std::size_t n = boost::num_vertices(bgl_g);

  // --- Run BGL's own Dijkstra ---
  std::vector<double>   bgl_dist(n, std::numeric_limits<double>::max());
  std::vector<vertex_t> bgl_pred(n);
  for (std::size_t i = 0; i < n; ++i) bgl_pred[i] = i;

  boost::dijkstra_shortest_paths(bgl_g, vertex_t(0),
    boost::predecessor_map(boost::make_iterator_property_map(bgl_pred.begin(),
                                                              boost::get(boost::vertex_index, bgl_g)))
    .distance_map(boost::make_iterator_property_map(bgl_dist.begin(),
                                                     boost::get(boost::vertex_index, bgl_g)))
    .weight_map(boost::get(&CsrEdgeProp::weight, bgl_g)));

  // --- Run graph-v3 Dijkstra on the adapted graph ---
  auto ga = graph::bgl::graph_adaptor(bgl_g);

  std::vector<double>      g3_dist(n, std::numeric_limits<double>::max());
  std::vector<std::size_t> g3_pred(n);
  for (std::size_t i = 0; i < n; ++i) g3_pred[i] = i;

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(g3_dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(g3_pred);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(
    boost::get(&CsrEdgeProp::weight, bgl_g));

  std::vector<std::size_t> sources = {0};
  dist_fn(ga, std::size_t{0}) = 0.0;

  graph::dijkstra_shortest_paths(ga, sources, dist_fn, pred_fn, weight_fn);

  // --- Compare ---
  for (std::size_t i = 0; i < n; ++i) {
    CHECK(g3_dist[i] == bgl_dist[i]);
    CHECK(g3_pred[i] == bgl_pred[i]);
  }
}
