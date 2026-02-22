/**
 * @file test_in_edges_cpo.cpp
 * @brief Comprehensive tests for in_edges, in_degree CPOs, outgoing aliases, and type aliases
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/adj_list/vertex_descriptor_view.hpp>
#include <graph/adj_list/edge_descriptor_view.hpp>
#include <vector>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Scenario 1: Stub graph with in_edges() vertex member
//
// The graph IS a vector<InEdgeVertex>. Each vertex's inner value has an
// in_edges() member.  The _vertex_member tier should fire.
// =============================================================================

namespace test_vertex_member {

struct InEdgeVertex {
  std::vector<int> edges_out;  // outgoing — satisfies default edges() pattern
  std::vector<int> in_list;    // incoming

  auto in_edges() const -> const std::vector<int>& { return in_list; }
};

using MemberGraph = std::vector<InEdgeVertex>;

} // namespace test_vertex_member

TEST_CASE("in_edges(g,u) - vertex member tier", "[in_edges][cpo][member]") {
  test_vertex_member::MemberGraph graph = {
        {{3}, {1, 2}},   // vertex 0: out={3}, in={1,2}
        {{0, 2}, {0}},   // vertex 1: out={0,2}, in={0}
        {{}, {}},         // vertex 2: out={}, in={}
  };

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  auto in_range = in_edges(graph, v0);
  REQUIRE(std::ranges::forward_range<decltype(in_range)>);

  // Should have 2 incoming edges (from vertices 1 and 2)
  size_t count = 0;
  for ([[maybe_unused]] auto e : in_range) {
    ++count;
  }
  REQUIRE(count == 2);

  // Vertex 2 has no incoming edges
  auto it = verts.begin();
  std::advance(it, 2);
  auto v2          = *it;
  auto in_range_v2 = in_edges(graph, v2);
  REQUIRE(std::ranges::empty(in_range_v2));
}

// =============================================================================
// Scenario 2: Stub graph with ADL in_edges(g, u)
//
// The graph inherits from vector<vector<int>> (so edges/vertices work) and
// provides a separate in_adj member. ADL in_edges accesses it.
// =============================================================================

namespace test_adl_in_edges {

struct ADLGraph : std::vector<std::vector<int>> {
  std::vector<std::vector<int>> in_adj;

  explicit ADLGraph(size_t n) : std::vector<std::vector<int>>(n), in_adj(n) {}

  void add_edge(size_t from, size_t to) {
    (*this)[from].push_back(static_cast<int>(to));
    in_adj[to].push_back(static_cast<int>(from));
  }
};

// ADL in_edges for vertex descriptor
template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(ADLGraph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_adl_in_edges

TEST_CASE("in_edges(g,u) - ADL tier", "[in_edges][cpo][adl]") {
  test_adl_in_edges::ADLGraph graph(4);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 0);

  // Vertex 2 should have in-edges from 0 and 1
  auto verts = vertices(graph);
  auto it    = verts.begin();
  std::advance(it, 2);
  auto v2 = *it;

  auto in_range = in_edges(graph, v2);
  REQUIRE(std::ranges::forward_range<decltype(in_range)>);

  size_t count = 0;
  for ([[maybe_unused]] auto e : in_range) {
    ++count;
  }
  REQUIRE(count == 2);
}

// =============================================================================
// Scenario 3: (g, uid) overload with default tier
//
// Uses ADLGraph. The (g, uid) default delegates through find_vertex +
// in_edges(g, u).
// =============================================================================

TEST_CASE("in_edges(g,uid) - default tier via find_vertex + in_edges(g,u)", "[in_edges][cpo][uid][default]") {
  test_adl_in_edges::ADLGraph graph(3);
  graph.add_edge(0, 1);
  graph.add_edge(2, 1);

  // Vertex 1 has in-edges from 0 and 2
  auto in_range = in_edges(graph, size_t(1));
  REQUIRE(std::ranges::forward_range<decltype(in_range)>);

  size_t count = 0;
  for ([[maybe_unused]] auto e : in_range) {
    ++count;
  }
  REQUIRE(count == 2);
}

// =============================================================================
// Scenario 4: Type alias verification
// =============================================================================

TEST_CASE("in_edges type aliases compile correctly", "[in_edges][cpo][type_alias]") {
  using Graph = test_adl_in_edges::ADLGraph;

  // These should compile without error
  using InRange = in_edge_range_t<Graph>;
  using InIter  = in_edge_iterator_t<Graph>;
  using InEdge  = in_edge_t<Graph>;

  STATIC_REQUIRE(std::ranges::forward_range<InRange>);
  STATIC_REQUIRE(std::input_iterator<InIter>);
  STATIC_REQUIRE(std::is_same_v<InEdge, std::ranges::range_value_t<InRange>>);
}

// =============================================================================
// Scenario 5: Mixed-type test - in_edge_t<G> != edge_t<G>
//
// The graph inherits from vector<vector<pair<int,double>>> so edges() uses
// the _edge_value_pattern tier (pair = weighted). ADL in_edges() returns
// plain vector<int>& (unweighted source IDs). The two edge types differ.
// =============================================================================

namespace test_mixed_types {

struct MixedGraph : std::vector<std::vector<std::pair<int, double>>> {
  std::vector<std::vector<int>> in_adj;

  explicit MixedGraph(size_t n) : std::vector<std::vector<std::pair<int, double>>>(n), in_adj(n) {}

  void add_edge(size_t from, size_t to, double w) {
    (*this)[from].push_back({static_cast<int>(to), w});
    in_adj[to].push_back(static_cast<int>(from));
  }
};

template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(MixedGraph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_mixed_types

TEST_CASE("in_edge_t<G> differs from edge_t<G> when edges differ", "[in_edges][cpo][mixed_types]") {
  using Graph = test_mixed_types::MixedGraph;

  // edge_t<G> wraps pair<int,double> (from outgoing edges via _edge_value_pattern)
  using OutEdge = edge_t<Graph>;
  // in_edge_t<G> wraps int (from incoming edges via ADL)
  using InEdge = in_edge_t<Graph>;

  // They should be different types since the underlying containers differ
  STATIC_REQUIRE(!std::is_same_v<OutEdge, InEdge>);
}

// =============================================================================
// Scenario 6: out_edges / out_degree / find_out_edge alias identity
// =============================================================================

TEST_CASE("out_edges, out_degree, find_out_edge are aliases for edges, degree, find_vertex_edge",
          "[out_edges][cpo][alias]") {
  STATIC_REQUIRE(&out_edges == &edges);
  STATIC_REQUIRE(&out_degree == &degree);
  STATIC_REQUIRE(&find_out_edge == &find_vertex_edge);
}

// =============================================================================
// Scenario 7: in_degree CPO — member, ADL, default tiers
// =============================================================================

namespace test_in_degree_member {

// Graph where g.in_degree(u) member returns doubled count
struct Graph : std::vector<std::vector<int>> {
  std::vector<std::vector<int>> in_adj;

  explicit Graph(size_t n) : std::vector<std::vector<int>>(n), in_adj(n) {}

  template <typename U>
    requires vertex_descriptor_type<U>
  size_t in_degree(const U& u) const {
    return in_adj[u.vertex_id()].size() * 2; // doubled for testing
  }
};

// Also provide ADL in_edges so default tier would also be available
template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(Graph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_in_degree_member

namespace test_in_degree_adl {

struct Graph : std::vector<std::vector<int>> {
  std::vector<std::vector<int>> in_adj;

  explicit Graph(size_t n) : std::vector<std::vector<int>>(n), in_adj(n) {}
};

// ADL in_degree — tripled count
template <typename U>
  requires vertex_descriptor_type<U>
size_t in_degree(Graph& g, const U& u) {
  return g.in_adj[u.vertex_id()].size() * 3; // tripled for testing
}

// Also provide ADL in_edges
template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(Graph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_in_degree_adl

TEST_CASE("in_degree(g,u) - member tier", "[in_degree][cpo][member]") {
  test_in_degree_member::Graph graph(3);
  graph.in_adj[0] = {1, 2};
  graph.in_adj[1] = {0};
  graph.in_adj[2] = {};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  // Member returns doubled: 2 * 2 = 4
  REQUIRE(in_degree(graph, v0) == 4);
}

TEST_CASE("in_degree(g,u) - ADL tier", "[in_degree][cpo][adl]") {
  test_in_degree_adl::Graph graph(3);
  graph.in_adj[0] = {1, 2};
  graph.in_adj[1] = {0};
  graph.in_adj[2] = {};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  // ADL returns tripled: 2 * 3 = 6
  REQUIRE(in_degree(graph, v0) == 6);
}

TEST_CASE("in_degree(g,u) - default tier via size(in_edges)", "[in_degree][cpo][default]") {
  test_adl_in_edges::ADLGraph graph(4);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 2);

  auto verts = vertices(graph);
  auto it    = verts.begin();
  std::advance(it, 2);
  auto v2 = *it;

  // Default: count in_edges(g, u) => 3
  REQUIRE(in_degree(graph, v2) == 3);
}

TEST_CASE("in_degree(g,uid) - default tier", "[in_degree][cpo][uid][default]") {
  test_adl_in_edges::ADLGraph graph(3);
  graph.add_edge(0, 1);
  graph.add_edge(2, 1);

  // uid overload: find_vertex then in_degree(g, u)
  REQUIRE(in_degree(graph, size_t(1)) == 2);
  REQUIRE(in_degree(graph, size_t(0)) == 0);
}

// =============================================================================
// Scenario 8: Outgoing type aliases
// =============================================================================

TEST_CASE("out_edge_range_t, out_edge_iterator_t, out_edge_t match existing aliases",
          "[out_edges][cpo][type_alias]") {
  using Graph = std::vector<std::vector<int>>;

  STATIC_REQUIRE(std::is_same_v<out_edge_range_t<Graph>, vertex_edge_range_t<Graph>>);
  STATIC_REQUIRE(std::is_same_v<out_edge_iterator_t<Graph>, vertex_edge_iterator_t<Graph>>);
  STATIC_REQUIRE(std::is_same_v<out_edge_t<Graph>, edge_t<Graph>>);
}
