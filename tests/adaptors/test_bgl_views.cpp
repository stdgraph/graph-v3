#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <algorithm>
#include <vector>

// ── BGL graph type ──────────────────────────────────────────────────────────

struct EdgeProp {
  double weight;
};

using bgl_directed_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                              boost::no_property, EdgeProp>;
using adapted_t = graph::bgl::graph_adaptor<bgl_directed_t>;

// ── Helper ──────────────────────────────────────────────────────────────────
// 4 vertices, edges: 0→1(1.0), 0→2(2.0), 1→3(3.0), 2→3(4.0)

static bgl_directed_t make_test_graph() {
  bgl_directed_t g(4);
  boost::add_edge(0, 1, EdgeProp{1.0}, g);
  boost::add_edge(0, 2, EdgeProp{2.0}, g);
  boost::add_edge(1, 3, EdgeProp{3.0}, g);
  boost::add_edge(2, 3, EdgeProp{4.0}, g);
  return g;
}

// ── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("vertexlist view on adapted BGL graph", "[bgl][views]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  std::vector<std::size_t> ids;
  for (auto&& [uid, u] : graph::views::vertexlist(g)) {
    ids.push_back(uid);
  }

  REQUIRE(ids.size() == 4);
  CHECK(ids[0] == 0);
  CHECK(ids[1] == 1);
  CHECK(ids[2] == 2);
  CHECK(ids[3] == 3);
}

TEST_CASE("incidence view on adapted BGL graph", "[bgl][views]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  auto u0 = *graph::find_vertex(g, 0);

  std::vector<std::size_t> targets;
  for (auto&& [tid, uv] : graph::views::incidence(g, u0)) {
    targets.push_back(tid);
  }
  std::ranges::sort(targets);

  CHECK(targets == std::vector<std::size_t>{1, 2});
}

TEST_CASE("neighbors view on adapted BGL graph", "[bgl][views]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  auto u0 = *graph::find_vertex(g, 0);

  std::vector<std::size_t> nbrs;
  for (auto&& [nid, nbr] : graph::views::neighbors(g, u0)) {
    nbrs.push_back(nid);
  }
  std::ranges::sort(nbrs);

  // Same set as incidence targets
  CHECK(nbrs == std::vector<std::size_t>{1, 2});
}

TEST_CASE("edgelist view on adapted BGL graph", "[bgl][views]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  using pair_t = std::pair<std::size_t, std::size_t>;
  std::vector<pair_t> edges;
  for (auto&& [sid, tid, uv] : graph::views::edgelist(g)) {
    edges.emplace_back(sid, tid);
  }
  std::ranges::sort(edges);

  std::vector<pair_t> expected = {{0,1},{0,2},{1,3},{2,3}};
  CHECK(edges == expected);
}

TEST_CASE("incidence view with edge weight function", "[bgl][views]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  auto bgl_pm    = boost::get(&EdgeProp::weight, bgl_g);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);

  auto u0 = *graph::find_vertex(g, 0);

  std::vector<std::pair<std::size_t, double>> tid_weight;
  for (auto&& [tid, uv, w] : graph::views::incidence(g, u0, weight_fn)) {
    tid_weight.emplace_back(tid, w);
  }
  std::ranges::sort(tid_weight);

  CHECK(tid_weight.size() == 2);
  CHECK(tid_weight[0] == std::pair<std::size_t, double>{1, 1.0});
  CHECK(tid_weight[1] == std::pair<std::size_t, double>{2, 2.0});
}
