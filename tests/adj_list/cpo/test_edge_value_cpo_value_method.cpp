#include "graph/container/undirected_adjacency_list.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace graph;
using namespace graph::container;

// Type alias for vertex id
using VKey = unsigned int;

// Helper to convert iterator to vertex id
template <typename Iter, typename G>
constexpr VKey vertex_id(Iter it, const G& g) {
  return static_cast<VKey>(it - g.vertices().begin());
}

// Test that edge_value CPO recognizes the .value() member function pattern
TEST_CASE("edge_value CPO with .value() method", "[cpo][edge_value][value_method]") {
  undirected_adjacency_list<int, int> g({{0, 1, 100}});
  VKey                                k1 = 0;
  VKey                                k2 = 1;

  SECTION("edge_value CPO works with undirected_adjacency_list edges") {
    // Get edge through vertex edge list
    auto& v          = g.vertices()[k1];
    auto  edge_range = v.edges(g, k1);
    auto  edge_it    = edge_range.begin();

    // The edge type (ual_edge) has a .value() member function
    // The edge_value CPO should recognize this pattern
    auto& ev = edge_value(g, *edge_it);

    REQUIRE(ev == 100);

    // Verify we can modify through the CPO
    edge_value(g, *edge_it) = 999;
    REQUIRE(edge_value(g, *edge_it) == 999);

    // Verify the change is visible through direct .value() call
    REQUIRE(edge_it->value() == 999);
  }

  SECTION("edge_value CPO const access") {
    const auto& cg         = g;
    auto&       v          = cg.vertices()[k1];
    auto        edge_range = v.edges(cg, k1);
    auto        edge_it    = edge_range.begin();

    // Const access through CPO
    const auto& ev = edge_value(cg, *edge_it);
    REQUIRE(ev == 100);

    // Should match direct .value() call
    REQUIRE(edge_it->value() == 100);
  }
}

// Test to verify the resolution priority order
TEST_CASE("edge_value CPO resolution priority", "[cpo][edge_value][priority]") {
  undirected_adjacency_list<int, int> g({{0, 1, 42}});
  VKey                                k1 = 0;
  VKey                                k2 = 1;

  // Get edge
  auto& v       = g.vertices()[k1];
  auto  edge_it = v.edges(g, k1).begin();
  auto& edge    = *edge_it;

  // The CPO should resolve in this priority order:
  // 1. g.edge_value(edge) - Member function (if it existed)
  // 2. edge_value(g, edge) - ADL (if it existed)
  // 3. edge.value() - Member function pattern (this is what we're testing)
  // 4. Default using descriptor (fallback)

  // For undirected_adjacency_list, the edge type has .value() method
  // So CPO should use the _value_fn strategy

  int& val = edge_value(g, edge);
  REQUIRE(val == 42);

  // Modify and verify
  val = 123;
  REQUIRE(edge.value() == 123);
  REQUIRE(edge_value(g, edge) == 123);
}
