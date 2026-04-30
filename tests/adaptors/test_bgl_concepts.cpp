#include <catch2/catch_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/views.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <vector>

// ── BGL type aliases ────────────────────────────────────────────────────────

struct EdgeProp {
  double weight;
};

using bgl_directed_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                              boost::no_property, EdgeProp>;
using adapted_t = graph::bgl::graph_adaptor<bgl_directed_t>;

// ── Concept static_asserts ──────────────────────────────────────────────────

static_assert(graph::adj_list::adjacency_list<adapted_t>,
              "graph_adaptor<adjacency_list> must satisfy graph::adj_list::adjacency_list");
static_assert(graph::adj_list::index_adjacency_list<adapted_t>,
              "graph_adaptor<adjacency_list<vecS,vecS>> must satisfy index_adjacency_list");

// ── Runtime smoke test ──────────────────────────────────────────────────────

TEST_CASE("vertexlist view smoke test on adapted BGL graph", "[bgl][concepts]") {
  bgl_directed_t bgl_g(4);
  boost::add_edge(0, 1, EdgeProp{1.0}, bgl_g);
  boost::add_edge(0, 2, EdgeProp{2.0}, bgl_g);
  boost::add_edge(1, 3, EdgeProp{3.0}, bgl_g);
  boost::add_edge(2, 3, EdgeProp{4.0}, bgl_g);

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
