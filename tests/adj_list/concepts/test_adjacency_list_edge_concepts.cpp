/**
 * @file test_adjacency_list_edge_concepts.cpp
 * @brief Unit tests for adjacency list edge concepts
 * 
 * NOTE: These tests check that the concepts are correctly defined by testing
 * with the existing std::vector<std::vector<int>> adjacency list structure
 * which already has working CPO implementations (target_id via edge_descriptor).
 * 
 * We avoid ADL/CPO name conflicts by not defining custom ADL functions and
 * instead relying on the existing descriptor infrastructure.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <deque>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// edge Concept Tests with edge_descriptor
// =============================================================================

TEST_CASE("edge concept - edge_descriptor from vector<vector<int>>", "[adjacency_list][concepts][edge]") {
  using Graph      = std::vector<std::vector<int>>;
  using VertexIter = typename Graph::iterator;
  using EdgeIter   = typename std::vector<int>::iterator;
  using EdgeDesc   = edge_descriptor<EdgeIter, VertexIter>;

  // edge_descriptor has source/target CPOs implemented, so it satisfies edge
  STATIC_REQUIRE(edge<Graph, EdgeDesc>);

  Graph g     = {{1, 2}, {2, 3}, {0}};
  auto  verts = vertices(g);
  auto  v_it  = verts.begin();
  auto  v0    = *v_it;

  auto edge_range = edges(g, v0);
  auto e_it       = edge_range.begin();
  auto e          = *e_it;

  // Should be able to get target_id from edge descriptor
  REQUIRE(target_id(g, e) == 1);
}

TEST_CASE("edge concept - edge_descriptor from vector<vector<pair>>", "[adjacency_list][concepts][edge]") {
  using Graph      = std::vector<std::vector<std::pair<int, double>>>;
  using VertexIter = typename Graph::iterator;
  using EdgeIter   = typename std::vector<std::pair<int, double>>::iterator;
  using EdgeDesc   = edge_descriptor<EdgeIter, VertexIter>;

  STATIC_REQUIRE(edge<Graph, EdgeDesc>);

  Graph g     = {{{1, 1.0}, {2, 2.0}}, {{2, 3.0}}, {}};
  auto  verts = vertices(g);
  auto  v0    = *verts.begin();

  auto edge_range = edges(g, v0);
  auto e          = *edge_range.begin();

  REQUIRE(target_id(g, e) == 1);
}

TEST_CASE("edge concept - edge_descriptor from deque<deque<int>>", "[adjacency_list][concepts][edge]") {
  using Graph      = std::deque<std::deque<int>>;
  using VertexIter = typename Graph::iterator;
  using EdgeIter   = typename std::deque<int>::iterator;
  using EdgeDesc   = edge_descriptor<EdgeIter, VertexIter>;

  STATIC_REQUIRE(edge<Graph, EdgeDesc>);

  Graph g     = {{1, 2}, {2, 3}};
  auto  verts = vertices(g);
  auto  v0    = *verts.begin();

  auto edge_range = edges(g, v0);
  auto e          = *edge_range.begin();

  REQUIRE(target_id(g, e) == 1);
}

// =============================================================================
// out_edge_range Concept Tests
// =============================================================================

TEST_CASE("out_edge_range concept - edge_descriptor_view from vector<vector<int>>",
          "[adjacency_list][concepts][range]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIter   = typename Graph::iterator;
  using EdgeIter     = typename std::vector<int>::iterator;
  using EdgeDescView = edge_descriptor_view<EdgeIter, VertexIter>;

  STATIC_REQUIRE(out_edge_range<EdgeDescView, Graph>);

  Graph g     = {{1, 2, 3}, {0, 2}, {0, 1}};
  auto  verts = vertices(g);
  auto  v0    = *verts.begin();

  auto edge_range = edges(g, v0);

  // Iterate over all edges and check target_id
  int count = 0;
  for (auto e : edge_range) {
    auto tid = target_id(g, e);
    REQUIRE(tid >= 1);
    REQUIRE(tid <= 3);
    count++;
  }
  REQUIRE(count == 3);
}

TEST_CASE("out_edge_range concept - edge_descriptor_view from vector<vector<pair>>",
          "[adjacency_list][concepts][range]") {
  using Graph        = std::vector<std::vector<std::pair<int, double>>>;
  using VertexIter   = typename Graph::iterator;
  using EdgeIter     = typename std::vector<std::pair<int, double>>::iterator;
  using EdgeDescView = edge_descriptor_view<EdgeIter, VertexIter>;

  STATIC_REQUIRE(out_edge_range<EdgeDescView, Graph>);

  Graph g     = {{{1, 1.5}, {2, 2.5}}, {{0, 0.5}}, {}};
  auto  verts = vertices(g);
  auto  v_it  = verts.begin();
  auto  v0    = *v_it;

  auto edge_range = edges(g, v0);

  int count = 0;
  for (auto e : edge_range) {
    auto tid = target_id(g, e);
    REQUIRE((tid == 1 || tid == 2));
    count++;
  }
  REQUIRE(count == 2);
}

TEST_CASE("out_edge_range concept - multiple vertices", "[adjacency_list][concepts][range]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIter   = typename Graph::iterator;
  using EdgeIter     = typename std::vector<int>::iterator;
  using EdgeDescView = edge_descriptor_view<EdgeIter, VertexIter>;

  STATIC_REQUIRE(out_edge_range<EdgeDescView, Graph>);

  Graph g     = {{1, 2}, {2, 3}, {0, 1}};
  auto  verts = vertices(g);

  // Check all vertices
  int v_count = 0;
  for (auto v : verts) {
    auto edge_range = edges(g, v);

    // Each vertex should have edges
    int e_count = 0;
    for (auto e : edge_range) {
      auto tid = target_id(g, e);
      REQUIRE(tid >= 0);
      REQUIRE(tid <= 3);
      e_count++;
    }
    REQUIRE(e_count == 2);
    v_count++;
  }
  REQUIRE(v_count == 3);
}

// =============================================================================
// Edge Concept Documentation Tests
// =============================================================================

TEST_CASE("Edge concepts - concept requirements documented", "[adjacency_list][concepts][edge][documentation]") {
  // These tests document what the concepts require

  // edge requires:
  // - source_id(g, e), source(g, e), target_id(g, e), target(g, e) are valid
  using Graph    = std::vector<std::vector<int>>;
  using EdgeDesc = edge_t<Graph>;

  STATIC_REQUIRE(edge<Graph, EdgeDesc>);

  // Verify the requirement
  Graph g   = {{1, 2}};
  auto  v   = *vertices(g).begin();
  auto  e   = *edges(g, v).begin();
  auto  tid = target_id(g, e);

  STATIC_REQUIRE(std::integral<decltype(tid)>);
  REQUIRE(tid == 1);
}

TEST_CASE("Edge range concepts - range requirements documented", "[adjacency_list][concepts][range][documentation]") {
  // out_edge_range requires:
  // - R is a forward_range
  // - range_value_t<R> satisfies edge

  using Graph        = std::vector<std::vector<int>>;
  using EdgeDescView = decltype(edges(std::declval<Graph&>(), std::declval<vertex_t<Graph>>()));

  // The view itself should be a forward range
  STATIC_REQUIRE(std::ranges::forward_range<EdgeDescView>);

  // And each element should be an edge
  using EdgeDesc = std::ranges::range_value_t<EdgeDescView>;
  STATIC_REQUIRE(edge<Graph, EdgeDesc>);

  // Verify with actual data
  Graph g          = {{1, 2}};
  auto  v          = *vertices(g).begin();
  auto  edge_range = edges(g, v);

  STATIC_REQUIRE(out_edge_range<decltype(edge_range), Graph>);
}
