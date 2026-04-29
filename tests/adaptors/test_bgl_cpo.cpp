#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <vector>

// ── BGL graph type ──────────────────────────────────────────────────────────

struct EdgeProp {
  double weight;
};

using bgl_directed_t =
      boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property, EdgeProp>;

// ── Helper: build test graph ────────────────────────────────────────────────
// 4 vertices, edges: 0→1(w=1.0), 0→2(w=2.0), 1→3(w=3.0), 2→3(w=4.0)

static bgl_directed_t make_test_graph() {
  bgl_directed_t g(4);
  boost::add_edge(0, 1, EdgeProp{1.0}, g);
  boost::add_edge(0, 2, EdgeProp{2.0}, g);
  boost::add_edge(1, 3, EdgeProp{3.0}, g);
  boost::add_edge(2, 3, EdgeProp{4.0}, g);
  return g;
}

// ── CPO tests ───────────────────────────────────────────────────────────────

TEST_CASE("vertices CPO on adapted BGL graph", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  auto verts = graph::vertices(g);
  REQUIRE(std::ranges::distance(verts) == 4);

  std::vector<std::size_t> ids;
  for (auto&& u : verts) {
    ids.push_back(graph::vertex_id(g, u));
  }
  CHECK(ids == std::vector<std::size_t>{0, 1, 2, 3});
}

TEST_CASE("num_vertices CPO on adapted BGL graph", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  CHECK(graph::num_vertices(g) == 4);
}

TEST_CASE("out_edges CPO on adapted BGL graph", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  // Get vertex 0's descriptor
  auto u0 = *graph::find_vertex(g, 0);

  auto oe = graph::out_edges(g, u0);
  REQUIRE(std::ranges::distance(oe) == 2);
}

TEST_CASE("target_id CPO on adapted BGL graph", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  auto u0    = *graph::find_vertex(g, 0);
  auto oe    = graph::out_edges(g, u0);
  auto first = std::ranges::begin(oe);

  auto tid = graph::target_id(g, *first);
  // First out-edge of vertex 0 goes to 1 or 2 (depends on insertion order)
  CHECK((tid == 1 || tid == 2));
}

TEST_CASE("source_id CPO on adapted BGL graph", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  auto u0    = *graph::find_vertex(g, 0);
  auto oe    = graph::out_edges(g, u0);
  auto first = std::ranges::begin(oe);

  auto sid = graph::source_id(g, *first);
  CHECK(sid == 0);
}

TEST_CASE("find_vertex CPO on adapted BGL graph", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  auto it = graph::find_vertex(g, 2);
  REQUIRE(it != std::ranges::end(graph::vertices(g)));
  CHECK(graph::vertex_id(g, *it) == 2);
}

TEST_CASE("all out-edge targets correct", "[bgl][cpo]") {
  auto bgl_g = make_test_graph();
  auto g     = graph::bgl::graph_adaptor(bgl_g);

  auto u0 = *graph::find_vertex(g, 0);
  auto oe = graph::out_edges(g, u0);

  std::vector<std::size_t> targets;
  for (auto&& uv : oe) {
    targets.push_back(graph::target_id(g, uv));
  }
  std::ranges::sort(targets);
  CHECK(targets == std::vector<std::size_t>{1, 2});
}
