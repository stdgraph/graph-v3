/**
 * @file test_incoming_edge_traits.cpp
 * @brief Unit tests for incoming edge traits: has_in_degree, has_find_in_edge, has_contains_in_edge
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/adjacency_list_traits.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <vector>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Stub graph with ADL in_edges — provides incoming edge support
// =============================================================================

namespace test_incoming_traits {

struct IncomingGraph : std::vector<std::vector<int>> {
  std::vector<std::vector<int>> in_adj;

  explicit IncomingGraph(size_t n) : std::vector<std::vector<int>>(n), in_adj(n) {}

  void add_edge(size_t from, size_t to) {
    (*this)[from].push_back(static_cast<int>(to));
    in_adj[to].push_back(static_cast<int>(from));
  }
};

// ADL in_edges for vertex descriptor
template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(IncomingGraph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_incoming_traits

// Simple graph without in_edges support
using PlainGraph = std::vector<std::vector<int>>;

// =============================================================================
// has_in_degree Tests
// =============================================================================

TEST_CASE("has_in_degree trait for graph with in_edges support",
          "[adjacency_list_traits][has_in_degree]") {
  STATIC_REQUIRE(has_in_degree<test_incoming_traits::IncomingGraph>);
  STATIC_REQUIRE(has_in_degree_v<test_incoming_traits::IncomingGraph>);
}

TEST_CASE("has_in_degree trait is false for plain graph without in_edges",
          "[adjacency_list_traits][has_in_degree]") {
  STATIC_REQUIRE_FALSE(has_in_degree<PlainGraph>);
  STATIC_REQUIRE_FALSE(has_in_degree_v<PlainGraph>);
}

// =============================================================================
// has_find_in_edge Tests
// =============================================================================

TEST_CASE("has_find_in_edge trait for graph with in_edges support",
          "[adjacency_list_traits][has_find_in_edge]") {
  STATIC_REQUIRE(has_find_in_edge<test_incoming_traits::IncomingGraph>);
  STATIC_REQUIRE(has_find_in_edge_v<test_incoming_traits::IncomingGraph>);
}

TEST_CASE("has_find_in_edge trait is also true for plain graph (default delegates to outgoing edges)",
          "[adjacency_list_traits][has_find_in_edge]") {
  STATIC_REQUIRE(has_find_in_edge<PlainGraph>);
  STATIC_REQUIRE(has_find_in_edge_v<PlainGraph>);
}

// =============================================================================
// has_contains_in_edge Tests
// =============================================================================

TEST_CASE("has_contains_in_edge trait for graph with in_edges and vertex descriptors",
          "[adjacency_list_traits][has_contains_in_edge]") {
  using Graph = test_incoming_traits::IncomingGraph;
  using V     = vertex_t<Graph>;

  STATIC_REQUIRE(has_contains_in_edge<Graph, V>);
  STATIC_REQUIRE(has_contains_in_edge_v<Graph, V>);
}

TEST_CASE("has_contains_in_edge trait is also true for plain graph (default delegates to outgoing edges)",
          "[adjacency_list_traits][has_contains_in_edge]") {
  using V = vertex_t<PlainGraph>;

  STATIC_REQUIRE(has_contains_in_edge<PlainGraph, V>);
  STATIC_REQUIRE(has_contains_in_edge_v<PlainGraph, V>);
}
