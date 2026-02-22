/**
 * @file test_bidirectional_concepts.cpp
 * @brief Unit tests for in_edge_range, bidirectional_adjacency_list,
 *        and index_bidirectional_adjacency_list concepts
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/graph.hpp>
#include <vector>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Stub bidirectional graph with ADL in_edges
// =============================================================================

namespace test_bidir_concepts {

struct BidirGraph : std::vector<std::vector<int>> {
  std::vector<std::vector<int>> in_adj;

  explicit BidirGraph(size_t n) : std::vector<std::vector<int>>(n), in_adj(n) {}

  void add_edge(size_t from, size_t to) {
    (*this)[from].push_back(static_cast<int>(to));
    in_adj[to].push_back(static_cast<int>(from));
  }
};

// ADL in_edges for vertex descriptor
template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(BidirGraph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_bidir_concepts

// =============================================================================
// in_edge_range Concept Tests
// =============================================================================

TEST_CASE("in_edge_range concept satisfied by wrapped incoming edge range",
          "[adjacency_list][concepts][in_edge_range]") {
  // The in_edges CPO wraps raw ranges into edge_descriptor_views,
  // so in_edge_range_t<G> satisfies both forward_range and edge<G, ...>
  using Graph   = test_bidir_concepts::BidirGraph;
  using InRange = in_edge_range_t<Graph>;
  STATIC_REQUIRE(in_edge_range<InRange, Graph>);
}

TEST_CASE("in_edge_range concept NOT satisfied by raw non-edge ranges",
          "[adjacency_list][concepts][in_edge_range]") {
  using Graph = std::vector<std::vector<int>>;
  // Raw vector<int> is a forward_range but int doesn't satisfy edge<G, int>
  STATIC_REQUIRE_FALSE(in_edge_range<std::vector<int>, Graph>);
  // vector<double> similarly fails the edge requirement
  STATIC_REQUIRE_FALSE(in_edge_range<std::vector<double>, Graph>);
}

// =============================================================================
// bidirectional_adjacency_list Concept Tests
// =============================================================================

TEST_CASE("bidirectional_adjacency_list concept satisfied by bidirectional graph",
          "[adjacency_list][concepts][bidirectional_adjacency_list]") {
  using Graph = test_bidir_concepts::BidirGraph;
  STATIC_REQUIRE(bidirectional_adjacency_list<Graph>);
}

TEST_CASE("bidirectional_adjacency_list concept NOT satisfied by outgoing-only graph",
          "[adjacency_list][concepts][bidirectional_adjacency_list]") {
  // Plain vector<vector<int>> has edges but no in_edges
  using Graph = std::vector<std::vector<int>>;
  STATIC_REQUIRE_FALSE(bidirectional_adjacency_list<Graph>);
}

TEST_CASE("bidirectional_adjacency_list implies adjacency_list",
          "[adjacency_list][concepts][bidirectional_adjacency_list]") {
  using Graph = test_bidir_concepts::BidirGraph;
  // If bidirectional, then also adjacency_list
  STATIC_REQUIRE(adjacency_list<Graph>);
  STATIC_REQUIRE(bidirectional_adjacency_list<Graph>);
}

TEST_CASE("bidirectional_adjacency_list runtime validation",
          "[adjacency_list][concepts][bidirectional_adjacency_list]") {
  test_bidir_concepts::BidirGraph g(4);
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 2);
  g.add_edge(3, 0);

  auto verts = vertices(g);
  auto it    = verts.begin();
  auto v0    = *it++;
  auto v1    = *it++;
  auto v2    = *it++;

  // Verify in_edges returns correct incoming edges
  auto in0 = in_edges(g, v0);
  REQUIRE(std::ranges::distance(in0) == 1); // from vertex 3

  auto in2 = in_edges(g, v2);
  REQUIRE(std::ranges::distance(in2) == 2); // from vertices 0, 1

  // Verify source_id on incoming edges compiles and returns a vertex ID.
  // For BidirGraph (which stores in-edges as plain ints without .in_edges()
  // on the vertex), source_id() returns the owning/target vertex ID.
  // Graphs that store in-edges with .in_edges() on the vertex (like
  // dynamic_graph) get the actual source vertex ID instead.
  for (auto ie : in_edges(g, v2)) {
    auto sid = source_id(g, ie);
    REQUIRE(sid == 2); // The vertex we called in_edges on (owning vertex)
  }
}

// =============================================================================
// index_bidirectional_adjacency_list Concept Tests
// =============================================================================

TEST_CASE("index_bidirectional_adjacency_list concept satisfied by indexed bidirectional graph",
          "[adjacency_list][concepts][index_bidirectional_adjacency_list]") {
  using Graph = test_bidir_concepts::BidirGraph;
  // BidirGraph inherits from vector<vector<int>>, which is index-based
  STATIC_REQUIRE(index_vertex_range<Graph>);
  STATIC_REQUIRE(index_bidirectional_adjacency_list<Graph>);
}

TEST_CASE("index_bidirectional_adjacency_list NOT satisfied by non-bidirectional graph",
          "[adjacency_list][concepts][index_bidirectional_adjacency_list]") {
  using Graph = std::vector<std::vector<int>>;
  // Has index_vertex_range but not bidirectional
  STATIC_REQUIRE(index_vertex_range<Graph>);
  STATIC_REQUIRE_FALSE(index_bidirectional_adjacency_list<Graph>);
}

// =============================================================================
// Re-export Tests (graph:: namespace)
// =============================================================================

TEST_CASE("bidirectional concepts accessible via graph:: namespace",
          "[adjacency_list][concepts][re-export]") {
  using Graph = test_bidir_concepts::BidirGraph;
  // Verify concepts are re-exported to graph:: namespace
  STATIC_REQUIRE(graph::bidirectional_adjacency_list<Graph>);
  STATIC_REQUIRE(graph::index_bidirectional_adjacency_list<Graph>);
  STATIC_REQUIRE_FALSE(graph::bidirectional_adjacency_list<std::vector<std::vector<int>>>);
}

TEST_CASE("incoming-edge CPOs accessible via graph:: namespace",
          "[adjacency_list][concepts][re-export]") {
  test_bidir_concepts::BidirGraph g(3);
  g.add_edge(0, 1);
  g.add_edge(2, 1);

  auto verts = graph::vertices(g);
  auto it    = verts.begin();
  auto v0    = *it++;
  auto v1    = *it++;

  // in_edges via graph:: namespace
  auto in1 = graph::in_edges(g, v1);
  REQUIRE(std::ranges::distance(in1) == 2);

  // in_degree via graph:: namespace
  REQUIRE(graph::in_degree(g, v1) == 2);

  // contains_in_edge via graph:: namespace
  REQUIRE(graph::contains_in_edge(g, v1, v0) == true);
}

TEST_CASE("incoming-edge type aliases accessible via graph:: namespace",
          "[adjacency_list][concepts][re-export]") {
  using Graph = test_bidir_concepts::BidirGraph;

  // Verify type aliases are accessible from graph:: namespace
  STATIC_REQUIRE(std::is_same_v<graph::in_edge_range_t<Graph>,
                                adj_list::in_edge_range_t<Graph>>);
  STATIC_REQUIRE(std::is_same_v<graph::in_edge_t<Graph>,
                                adj_list::in_edge_t<Graph>>);

  // Outgoing aliases
  STATIC_REQUIRE(std::is_same_v<graph::out_edge_range_t<Graph>,
                                adj_list::out_edge_range_t<Graph>>);
  STATIC_REQUIRE(std::is_same_v<graph::out_edge_t<Graph>,
                                adj_list::out_edge_t<Graph>>);
}

TEST_CASE("incoming-edge traits accessible via graph:: namespace",
          "[adjacency_list][concepts][re-export]") {
  using Graph = test_bidir_concepts::BidirGraph;

  STATIC_REQUIRE(graph::has_in_degree<Graph>);
  STATIC_REQUIRE(graph::has_in_degree_v<Graph>);
  STATIC_REQUIRE(graph::has_find_in_edge<Graph>);
  STATIC_REQUIRE(graph::has_find_in_edge_v<Graph>);
}
