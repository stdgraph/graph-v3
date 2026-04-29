/**
 * @file test_bgl_non_vecs.cpp
 * @brief Tests for the BGL adaptor with non-vecS vertex containers (listS, setS).
 *
 * These BGL graph types use void* as vertex_descriptor instead of integral indices.
 * The adaptor supports them via the non-integral path: vertices() returns the
 * actual BGL vertex iterator pair, and property storage uses unordered_map.
 */

#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ── Graph types ─────────────────────────────────────────────────────────────

struct EdgeWeight {
  double weight;
};

// listS vertex container — vertex_descriptor is void*
using bgl_list_graph_t = boost::adjacency_list<boost::vecS, boost::listS, boost::directedS,
                                                boost::no_property, EdgeWeight>;

// setS vertex container — vertex_descriptor is void*
using bgl_set_graph_t = boost::adjacency_list<boost::vecS, boost::setS, boost::directedS,
                                               boost::no_property, EdgeWeight>;

// Verify the descriptor is actually void* (non-integral)
static_assert(!std::integral<boost::graph_traits<bgl_list_graph_t>::vertex_descriptor>);
static_assert(!std::integral<boost::graph_traits<bgl_set_graph_t>::vertex_descriptor>);

// ── Helper: build a small graph, return vertex descriptors ──────────────────

template <typename BGL_Graph>
struct test_graph {
  BGL_Graph g;
  typename boost::graph_traits<BGL_Graph>::vertex_descriptor v0, v1, v2, v3;

  test_graph() {
    v0 = boost::add_vertex(g);
    v1 = boost::add_vertex(g);
    v2 = boost::add_vertex(g);
    v3 = boost::add_vertex(g);
    // 0→1(10), 0→2(5), 1→3(1), 2→3(3)
    boost::add_edge(v0, v1, EdgeWeight{10.0}, g);
    boost::add_edge(v0, v2, EdgeWeight{5.0}, g);
    boost::add_edge(v1, v3, EdgeWeight{1.0}, g);
    boost::add_edge(v2, v3, EdgeWeight{3.0}, g);
  }
};

// ===========================================================================
// listS tests
// ===========================================================================

TEST_CASE("listS: graph_adaptor wraps BGL listS graph", "[bgl][listS]") {
  test_graph<bgl_list_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  REQUIRE(graph::num_vertices(ga) == 4);
}

TEST_CASE("listS: vertices() iterates all vertex descriptors", "[bgl][listS]") {
  test_graph<bgl_list_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  std::unordered_set<void*> seen;
  for (auto u : graph::vertices(ga)) {
    auto vid = graph::vertex_id(ga, u);
    seen.insert(vid);
  }
  REQUIRE(seen.size() == 4);
  REQUIRE(seen.count(tg.v0) == 1);
  REQUIRE(seen.count(tg.v1) == 1);
  REQUIRE(seen.count(tg.v2) == 1);
  REQUIRE(seen.count(tg.v3) == 1);
}

TEST_CASE("listS: edges() and target_id() work", "[bgl][listS]") {
  test_graph<bgl_list_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  // Find vertex v0 and iterate its out-edges
  std::unordered_set<void*> targets;
  for (auto u : graph::vertices(ga)) {
    if (graph::vertex_id(ga, u) == tg.v0) {
      for (auto uv : graph::edges(ga, u)) {
        targets.insert(graph::target_id(ga, uv));
      }
      break;
    }
  }
  REQUIRE(targets.size() == 2);
  REQUIRE(targets.count(tg.v1) == 1);
  REQUIRE(targets.count(tg.v2) == 1);
}

TEST_CASE("listS: source_id() returns correct source", "[bgl][listS]") {
  test_graph<bgl_list_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  for (auto u : graph::vertices(ga)) {
    if (graph::vertex_id(ga, u) == tg.v0) {
      for (auto uv : graph::edges(ga, u)) {
        REQUIRE(graph::source_id(ga, uv) == tg.v0);
      }
      break;
    }
  }
}

TEST_CASE("listS: vertex_map_property_fn works for storage", "[bgl][listS]") {
  test_graph<bgl_list_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  using vid_t = boost::graph_traits<bgl_list_graph_t>::vertex_descriptor;
  std::unordered_map<vid_t, double> dist;
  std::unordered_map<vid_t, vid_t>  pred;

  // Initialize
  for (auto u : graph::vertices(ga)) {
    auto vid = graph::vertex_id(ga, u);
    dist[vid] = std::numeric_limits<double>::max();
    pred[vid] = vid;
  }

  auto dist_fn = graph::bgl::make_vertex_map_property_fn(dist);
  auto pred_fn = graph::bgl::make_vertex_map_property_fn(pred);

  // Verify accessors work
  dist_fn(ga, tg.v0) = 0.0;
  REQUIRE(dist[tg.v0] == 0.0);
  REQUIRE(dist_fn(ga, tg.v0) == 0.0);

  pred_fn(ga, tg.v1) = tg.v0;
  REQUIRE(pred[tg.v1] == tg.v0);
}

TEST_CASE("listS: edge weight function works", "[bgl][listS]") {
  test_graph<bgl_list_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  auto bgl_pm    = boost::get(&EdgeWeight::weight, tg.g);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);

  // Check edge weights from v0
  std::vector<double> weights;
  for (auto u : graph::vertices(ga)) {
    if (graph::vertex_id(ga, u) == tg.v0) {
      for (auto uv : graph::edges(ga, u)) {
        weights.push_back(weight_fn(ga, uv));
      }
      break;
    }
  }
  std::sort(weights.begin(), weights.end());
  REQUIRE(weights.size() == 2);
  REQUIRE(weights[0] == 5.0);
  REQUIRE(weights[1] == 10.0);
}

// ===========================================================================
// setS tests
// ===========================================================================

TEST_CASE("setS: graph_adaptor wraps BGL setS graph", "[bgl][setS]") {
  test_graph<bgl_set_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  REQUIRE(graph::num_vertices(ga) == 4);
}

TEST_CASE("setS: vertices() iterates all vertex descriptors", "[bgl][setS]") {
  test_graph<bgl_set_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  std::unordered_set<void*> seen;
  for (auto u : graph::vertices(ga)) {
    auto vid = graph::vertex_id(ga, u);
    seen.insert(vid);
  }
  REQUIRE(seen.size() == 4);
  REQUIRE(seen.count(tg.v0) == 1);
  REQUIRE(seen.count(tg.v1) == 1);
  REQUIRE(seen.count(tg.v2) == 1);
  REQUIRE(seen.count(tg.v3) == 1);
}

TEST_CASE("setS: edges() and target_id() work", "[bgl][setS]") {
  test_graph<bgl_set_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  std::unordered_set<void*> targets;
  for (auto u : graph::vertices(ga)) {
    if (graph::vertex_id(ga, u) == tg.v0) {
      for (auto uv : graph::edges(ga, u)) {
        targets.insert(graph::target_id(ga, uv));
      }
      break;
    }
  }
  REQUIRE(targets.size() == 2);
  REQUIRE(targets.count(tg.v1) == 1);
  REQUIRE(targets.count(tg.v2) == 1);
}

TEST_CASE("setS: source_id() returns correct source", "[bgl][setS]") {
  test_graph<bgl_set_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  for (auto u : graph::vertices(ga)) {
    if (graph::vertex_id(ga, u) == tg.v0) {
      for (auto uv : graph::edges(ga, u)) {
        REQUIRE(graph::source_id(ga, uv) == tg.v0);
      }
      break;
    }
  }
}

TEST_CASE("setS: vertex_map_property_fn works for storage", "[bgl][setS]") {
  test_graph<bgl_set_graph_t> tg;
  auto ga = graph::bgl::graph_adaptor(tg.g);

  using vid_t = boost::graph_traits<bgl_set_graph_t>::vertex_descriptor;
  std::unordered_map<vid_t, double> dist;

  for (auto u : graph::vertices(ga)) {
    dist[graph::vertex_id(ga, u)] = std::numeric_limits<double>::max();
  }

  auto dist_fn = graph::bgl::make_vertex_map_property_fn(dist);
  dist_fn(ga, tg.v0) = 42.0;
  REQUIRE(dist[tg.v0] == 42.0);
}
