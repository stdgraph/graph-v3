/**
 * @file test_transpose.cpp
 * @brief Tests for transpose_view graph adaptor.
 *
 * Verifies that transpose_view correctly swaps edge directions:
 * - edges(tv, v) returns in_edges of the underlying graph
 * - in_edges(tv, v) returns edges of the underlying graph
 * - target_id(tv, e) returns source_id of the underlying edge
 * - source_id(tv, e) returns target_id of the underlying edge
 * - Vertex-level CPOs (vertices, num_vertices, vertex_id) forward unchanged
 *
 * Uses bidirectional dynamic_graph containers (vov with Bidirectional=true).
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/graph.hpp>
#include <graph/views/transpose.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <algorithm>
#include <list>
#include <set>
#include <vector>

namespace adj  = graph::adj_list;
namespace view = graph::views;
using namespace graph::container;

// Non-uniform bidirectional traits: in_edge_type = dynamic_in_edge (has source_id())
// so that bidirectional_adjacency_list concept is satisfied.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t>
struct vov_bidir_graph_traits {
  using edge_value_type   = EV;
  using vertex_value_type = VV;
  using graph_value_type  = GV;
  using vertex_id_type    = VId;
  static constexpr bool bidirectional = true;

  using edge_type    = dynamic_out_edge<EV, VV, GV, VId, true, vov_bidir_graph_traits>;
  using in_edge_type = dynamic_in_edge<EV, VV, GV, VId, true, vov_bidir_graph_traits>;
  using vertex_type  = dynamic_vertex<EV, VV, GV, VId, true, vov_bidir_graph_traits>;
  using graph_type   = dynamic_graph<EV, VV, GV, VId, true, vov_bidir_graph_traits>;

  using edges_type    = std::vector<edge_type>;
  using in_edges_type = std::vector<in_edge_type>;
  using vertices_type = std::vector<vertex_type>;
};

// Bidirectional vov graph (random-access edges — full transpose_view support)
using bidir_vov = dynamic_graph<int, void, void, uint32_t, true,
                                vov_bidir_graph_traits<int, void, void, uint32_t>>;

// Bidirectional vov with void edge values
using bidir_vov_void = dynamic_graph<void, void, void, uint32_t, true,
                                     vov_bidir_graph_traits<void, void, void, uint32_t>>;

// =============================================================================
// Helper: build a small directed graph for transpose testing
//
//   0 --→ 1 --→ 2
//   |           ↑
//   +---→ 3 ---+
//
// Edges: (0,1), (0,3), (1,2), (3,2)
// Transpose edges: (1,0), (3,0), (2,1), (2,3)
// =============================================================================

static bidir_vov_void make_dag() {
  return bidir_vov_void({{0, 1}, {0, 3}, {1, 2}, {3, 2}});
}

// =============================================================================
// Vertex CPO forwarding
// =============================================================================

TEST_CASE("transpose_view - vertex CPOs forward unchanged", "[views][transpose]") {
  auto g  = make_dag();
  auto tv = view::transpose(g);

  CHECK(adj_list::num_vertices(tv) == adj_list::num_vertices(g));

  // Verify vertices range produces same vertex ids
  std::vector<uint32_t> g_ids, tv_ids;
  for (auto&& [uid, u] : view::vertexlist(g))
    g_ids.push_back(uid);
  for (auto&& [uid, u] : view::vertexlist(tv))
    tv_ids.push_back(uid);

  REQUIRE(g_ids == tv_ids);
}

// =============================================================================
// Edge direction swapping
// =============================================================================

TEST_CASE("transpose_view - edges returns in_edges of underlying", "[views][transpose]") {
  auto g  = make_dag();
  auto tv = view::transpose(g);

  // Vertex 2 has in-edges from 1 and 3 in the original graph.
  // So edges(tv, v2) should yield those two edges.
  auto v2 = *adj_list::find_vertex(tv, uint32_t(2));
  std::set<uint32_t> edge_sources;
  for (auto&& ie : adj_list::edges(tv, v2)) {
    // target_id on transpose edge = source_id on underlying in-edge
    edge_sources.insert(adj_list::target_id(tv, ie));
  }
  CHECK(edge_sources == std::set<uint32_t>{1, 3});
}

TEST_CASE("transpose_view - in_edges returns edges of underlying", "[views][transpose]") {
  auto g  = make_dag();
  auto tv = view::transpose(g);

  // Vertex 0 has out-edges to 1 and 3 in the original graph.
  // So in_edges(tv, v0) should yield those two edges.
  auto v0 = *adj_list::find_vertex(tv, uint32_t(0));
  std::set<uint32_t> edge_targets;
  for (auto&& ie : adj_list::in_edges(tv, v0)) {
    edge_targets.insert(adj_list::source_id(tv, ie));
  }
  // source_id on transpose's in-edges = target_id on underlying = 1, 3
  CHECK(edge_targets == std::set<uint32_t>{1, 3});
}

TEST_CASE("transpose_view - degree is swapped", "[views][transpose]") {
  auto g  = make_dag();
  auto tv = view::transpose(g);

  // Vertex 0: out-degree=2, in-degree=0 in original
  // Transpose: degree=0, in_degree=2
  auto v0 = *adj_list::find_vertex(tv, uint32_t(0));
  CHECK(adj_list::degree(tv, v0) == 0);
  CHECK(adj_list::in_degree(tv, v0) == 2);

  // Vertex 2: out-degree=0, in-degree=2 in original
  // Transpose: degree=2, in_degree=0
  auto v2 = *adj_list::find_vertex(tv, uint32_t(2));
  CHECK(adj_list::degree(tv, v2) == 2);
  CHECK(adj_list::in_degree(tv, v2) == 0);
}

// =============================================================================
// Double transpose is identity
// =============================================================================

TEST_CASE("transpose_view - double transpose is identity", "[views][transpose]") {
  auto g   = make_dag();
  auto tv  = view::transpose(g);
  auto ttv = view::transpose(tv);

  // edges(ttv, v0) should be same as edges(g, v0)
  auto v0 = *adj_list::find_vertex(g, uint32_t(0));

  std::set<uint32_t> g_targets, ttv_targets;
  for (auto&& e : adj_list::edges(g, v0)) {
    g_targets.insert(adj_list::target_id(g, e));
  }

  auto tv0 = *adj_list::find_vertex(ttv, uint32_t(0));
  for (auto&& e : adj_list::edges(ttv, tv0)) {
    ttv_targets.insert(adj_list::target_id(ttv, e));
  }

  CHECK(g_targets == ttv_targets);
}

// =============================================================================
// Whole-graph edge collection in transpose
// =============================================================================

TEST_CASE("transpose_view - all transposed edges correct", "[views][transpose]") {
  auto g  = make_dag();
  auto tv = view::transpose(g);

  // Original edges: (0,1), (0,3), (1,2), (3,2)
  // Transposed: edges(tv, v) should give: 1→0, 3→0, 2→1, 2→3
  // Collect as (source, target) pairs in traverse order
  std::set<std::pair<uint32_t, uint32_t>> transposed_edges;

  for (auto&& [uid, u] : view::vertexlist(tv)) {
    for (auto&& e : adj_list::edges(tv, u)) {
      auto tid = adj_list::target_id(tv, e);
      transposed_edges.emplace(static_cast<uint32_t>(uid), static_cast<uint32_t>(tid));
    }
  }

  std::set<std::pair<uint32_t, uint32_t>> expected{
        {1, 0}, {2, 1}, {2, 3}, {3, 0}};

  CHECK(transposed_edges == expected);
}

// =============================================================================
// Edge value forwarding
// =============================================================================

TEST_CASE("transpose_view - edge_value preserved", "[views][transpose]") {
  // Use weighted bidirectional graph
  bidir_vov g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
  auto      tv = view::transpose(g);

  // Collect edge values from vertex 0's edges in transpose
  // Vertex 0 has in-edges: (2→0, weight=30)
  // So edges(tv, v0) should have one edge with value 30
  auto v0 = *adj_list::find_vertex(tv, uint32_t(0));

  std::vector<int> values;
  for (auto&& e : adj_list::edges(tv, v0)) {
    values.push_back(adj_list::edge_value(tv, e));
  }

  REQUIRE(values.size() == 1);
  CHECK(values[0] == 30);
}

// =============================================================================
// Empty graph
// =============================================================================

TEST_CASE("transpose_view - empty graph", "[views][transpose]") {
  bidir_vov_void g;
  g.resize_vertices(0); // ensure graph is valid but empty
  auto tv = view::transpose(g);

  CHECK(adj_list::num_vertices(tv) == 0);
}

// =============================================================================
// Single vertex, no edges
// =============================================================================

TEST_CASE("transpose_view - single vertex no edges", "[views][transpose]") {
  bidir_vov_void g;
  g.resize_vertices(1);
  auto tv = view::transpose(g);

  CHECK(adj_list::num_vertices(tv) == 1);
}
