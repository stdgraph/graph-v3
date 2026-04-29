#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <limits>
#include <vector>

// ── BGL graph type ──────────────────────────────────────────────────────────

struct EdgeProp {
  double weight;
};

using bgl_directed_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                              boost::no_property, EdgeProp>;
using adapted_t = graph::bgl::graph_adaptor<bgl_directed_t>;

// ── CLRS Dijkstra example graph ─────────────────────────────────────────────
//
//        (0)
//       / | \
//     10  5   2
//     /   |    \
//   (1)  (2)    (4)
//    \   / \   /
//     1  3   2  3
//      \ |    \ |
//       (3)----+
//           7
//
// Edges: 0→1(10), 0→2(5), 0→4(2), 1→3(1), 2→1(3), 2→3(9), 2→4(2), 3→4(7), 4→3(3)
//
// Shortest paths from 0:  0→0:0, 0→1:8, 0→2:5, 0→3:9, 0→4:2

static bgl_directed_t make_clrs_graph() {
  bgl_directed_t g(5);
  boost::add_edge(0, 1, EdgeProp{10.0}, g);
  boost::add_edge(0, 2, EdgeProp{5.0},  g);
  boost::add_edge(0, 4, EdgeProp{2.0},  g);
  boost::add_edge(1, 3, EdgeProp{1.0},  g);
  boost::add_edge(2, 1, EdgeProp{3.0},  g);
  boost::add_edge(2, 3, EdgeProp{9.0},  g);
  boost::add_edge(2, 4, EdgeProp{2.0},  g);
  boost::add_edge(3, 4, EdgeProp{7.0},  g);
  boost::add_edge(4, 3, EdgeProp{3.0},  g);
  return g;
}

// ── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("dijkstra_shortest_paths on adapted BGL graph", "[bgl][dijkstra]") {
  auto bgl_g = make_clrs_graph();
  adapted_t g(bgl_g);

  const std::size_t n   = graph::num_vertices(g);
  const double      inf = std::numeric_limits<double>::max();

  std::vector<double>      dist(n, inf);
  std::vector<std::size_t> pred(n);
  for (std::size_t i = 0; i < n; ++i) pred[i] = i; // self-loop = no predecessor

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(pred);
  auto bgl_pm    = boost::get(&EdgeProp::weight, bgl_g);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);

  std::vector<std::size_t> sources = {0};
  dist_fn(g, std::size_t{0}) = 0.0;

  graph::dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn);

  // Verify known CLRS distances
  CHECK(dist[0] == 0.0);
  CHECK(dist[1] == 8.0);  // 0→2(5)→1(3)
  CHECK(dist[2] == 5.0);  // 0→2(5)
  CHECK(dist[3] == 5.0);  // 0→4(2)→3(3)
  CHECK(dist[4] == 2.0);  // 0→4(2)

  // Verify predecessors
  CHECK(pred[1] == 2);  // 0→2→1
  CHECK(pred[2] == 0);
  CHECK(pred[3] == 4);  // 0→4→3
  CHECK(pred[4] == 0);
}

TEST_CASE("dijkstra produces same distances as manual BFS relaxation", "[bgl][dijkstra]") {
  // Simpler 4-vertex graph for a quick sanity check
  bgl_directed_t bgl_g(4);
  boost::add_edge(0, 1, EdgeProp{1.0}, bgl_g);
  boost::add_edge(0, 2, EdgeProp{4.0}, bgl_g);
  boost::add_edge(1, 2, EdgeProp{2.0}, bgl_g);
  boost::add_edge(1, 3, EdgeProp{5.0}, bgl_g);
  boost::add_edge(2, 3, EdgeProp{1.0}, bgl_g);

  adapted_t g(bgl_g);

  const double inf = std::numeric_limits<double>::max();
  std::vector<double>      dist(4, inf);
  std::vector<std::size_t> pred(4);
  for (std::size_t i = 0; i < 4; ++i) pred[i] = i;

  auto dist_fn   = graph::bgl::make_vertex_id_property_fn(dist);
  auto pred_fn   = graph::bgl::make_vertex_id_property_fn(pred);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(boost::get(&EdgeProp::weight, bgl_g));

  dist_fn(g, std::size_t{0}) = 0.0;
  std::vector<std::size_t> sources = {0};
  graph::dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn);

  // 0→0:0, 0→1:1, 0→2:3 (via 1), 0→3:4 (via 1→2→3)
  CHECK(dist[0] == 0.0);
  CHECK(dist[1] == 1.0);
  CHECK(dist[2] == 3.0);
  CHECK(dist[3] == 4.0);

  CHECK(pred[1] == 0);
  CHECK(pred[2] == 1);
  CHECK(pred[3] == 2);
}
