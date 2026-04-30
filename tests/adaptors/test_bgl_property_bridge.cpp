#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <graph/algorithm/traversal_common.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

#include <vector>

// ── BGL graph type ──────────────────────────────────────────────────────────

struct EdgeProp {
  double weight;
};

using bgl_directed_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                              boost::no_property, EdgeProp>;
using adapted_t = graph::bgl::graph_adaptor<bgl_directed_t>;

// ── Helper ──────────────────────────────────────────────────────────────────

static bgl_directed_t make_test_graph() {
  bgl_directed_t g(4);
  boost::add_edge(0, 1, EdgeProp{1.5}, g);
  boost::add_edge(0, 2, EdgeProp{2.5}, g);
  boost::add_edge(1, 3, EdgeProp{3.5}, g);
  boost::add_edge(2, 3, EdgeProp{4.5}, g);
  return g;
}

// ── Tests ───────────────────────────────────────────────────────────────────

TEST_CASE("readable property map wrapper reads edge weights", "[bgl][property]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  auto bgl_pm   = boost::get(&EdgeProp::weight, bgl_g);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);

  auto u0 = *graph::find_vertex(g, 0);
  auto oe = graph::out_edges(g, u0);

  std::vector<double> weights;
  for (auto&& uv : oe) {
    weights.push_back(weight_fn(g, uv));
  }
  std::ranges::sort(weights);
  CHECK(weights == std::vector<double>{1.5, 2.5});
}

TEST_CASE("readable property map with incidence view", "[bgl][property]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  auto bgl_pm   = boost::get(&EdgeProp::weight, bgl_g);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);

  auto u0 = *graph::find_vertex(g, 0);

  std::vector<double> weights;
  for (auto&& [tid, uv, w] : graph::views::incidence(g, u0, weight_fn)) {
    weights.push_back(w);
  }
  std::ranges::sort(weights);
  CHECK(weights == std::vector<double>{1.5, 2.5});
}

TEST_CASE("lvalue property map wrapper supports write-through", "[bgl][property]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  // BGL bundled property maps return lvalue references for vecS edge storage
  auto bgl_pm    = boost::get(&EdgeProp::weight, bgl_g);
  auto weight_fn = graph::bgl::make_bgl_lvalue_property_map_fn(bgl_pm,
                     graph::bgl::edge_key_extractor{});

  auto u0   = *graph::find_vertex(g, 0);
  auto oe   = graph::out_edges(g, u0);
  auto first = *std::ranges::begin(oe);

  // Write through the wrapper
  weight_fn(g, first) = 99.0;

  // Verify the underlying BGL graph changed
  auto [bgl_begin, bgl_end] = boost::out_edges(0, bgl_g);
  double w = bgl_g[*bgl_begin].weight;
  CHECK(w == 99.0);
}

TEST_CASE("vertex_vector_property_fn for distance storage", "[bgl][property]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  std::vector<double> dist(graph::num_vertices(g), std::numeric_limits<double>::max());
  auto dist_fn = graph::bgl::make_vertex_id_property_fn(dist);

  // Write through the function
  dist_fn(g, std::size_t{0}) = 0.0;
  dist_fn(g, std::size_t{1}) = 1.5;

  CHECK(dist[0] == 0.0);
  CHECK(dist[1] == 1.5);

  // Read back through the function
  CHECK(dist_fn(g, std::size_t{0}) == 0.0);
  CHECK(dist_fn(g, std::size_t{1}) == 1.5);
}

TEST_CASE("vertex_vector_property_fn for predecessor storage", "[bgl][property]") {
  auto bgl_g = make_test_graph();
  adapted_t g(bgl_g);

  std::vector<std::size_t> pred(graph::num_vertices(g));
  auto pred_fn = graph::bgl::make_vertex_id_property_fn(pred);

  pred_fn(g, std::size_t{1}) = std::size_t{0};
  pred_fn(g, std::size_t{3}) = std::size_t{2};

  CHECK(pred[1] == 0);
  CHECK(pred[3] == 2);
}

TEST_CASE("edge weight function satisfies basic_edge_weight_function", "[bgl][property]") {
  auto bgl_g = make_test_graph();

  auto bgl_pm    = boost::get(&EdgeProp::weight, bgl_g);
  auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);

  static_assert(graph::edge_value_function<decltype(weight_fn), adapted_t, graph::edge_t<adapted_t>>,
                "weight_fn must satisfy edge_value_function");
  static_assert(graph::basic_edge_weight_function<adapted_t, decltype(weight_fn), double,
                                                   std::less<double>, std::plus<double>>,
                "weight_fn must satisfy basic_edge_weight_function");
  SUCCEED();
}
