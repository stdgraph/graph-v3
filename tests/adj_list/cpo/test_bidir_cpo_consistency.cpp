/**
 * @file test_bidir_cpo_consistency.cpp
 * @brief Consistency tests for bidirectional dynamic_graph CPOs
 *
 * Verifies that CPO results for non-uniform bidirectional dynamic_graph are
 * internally consistent:
 *   - In-edge (source,target) pairs from in_edges() mirror out-edge pairs from edges()
 *   - in_degree(g,u) matches the counted number of in_edges(g,u)
 *   - source_id(g,ie) on in-edges is stable across multiple iterations
 *   - Results are identical between vov_bidir_graph_traits (vector) and
 *     vol_bidir_graph_traits (list) containers
 *
 * Terminology:
 *   - "out-edge pair": {source_vid, target_vid} from edges(g, u)
 *   - "in-edge pair":  {source_vid, target_vid} from in_edges(g, u)
 *   They should be equal sets for any bidirectional graph.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/graph.hpp>
#include "../../common/graph_test_types.hpp"
#include <algorithm>
#include <set>
#include <vector>

using namespace graph;
using namespace graph::adj_list;

using VovBiDirVoid =
      graph::container::dynamic_graph<void, void, void, uint32_t, true,
                                      graph::test::vov_bidir_graph_traits<>>;
using VovBiDirInt =
      graph::container::dynamic_graph<int, void, void, uint32_t, true,
                                      graph::test::vov_bidir_graph_traits<int>>;
using VolBiDirVoid =
      graph::container::dynamic_graph<void, void, void, uint32_t, true,
                                      graph::test::vol_bidir_graph_traits<>>;

// =============================================================================
// Helpers: collect (source, target) pairs via CPOs
// =============================================================================

template <typename G>
std::set<std::pair<uint32_t, uint32_t>>
collect_out_edge_pairs(const G& g) {
  std::set<std::pair<uint32_t, uint32_t>> result;
  for (auto u : vertices(g)) {
    auto uid = static_cast<uint32_t>(vertex_id(g, u));
    for (auto oe : edges(g, u)) {
      auto tid = static_cast<uint32_t>(target_id(g, oe));
      result.insert({uid, tid});
    }
  }
  return result;
}

template <typename G>
std::set<std::pair<uint32_t, uint32_t>>
collect_in_edge_pairs(const G& g) {
  std::set<std::pair<uint32_t, uint32_t>> result;
  for (auto u : vertices(g)) {
    auto uid = static_cast<uint32_t>(vertex_id(g, u));
    for (auto ie : in_edges(g, u)) {
      auto sid = static_cast<uint32_t>(source_id(g, ie));
      result.insert({sid, uid});
    }
  }
  return result;
}

// =============================================================================
// Symmetry: in-edge pairs == out-edge pairs
// =============================================================================

TEST_CASE("bidir CPO consistency: in-edges mirror out-edges (triangle)",
          "[bidir][cpo][consistency]") {
  // 0->1, 0->2, 1->2
  VovBiDirVoid g({{0, 1}, {0, 2}, {1, 2}});

  auto out_pairs = collect_out_edge_pairs(g);
  auto in_pairs  = collect_in_edge_pairs(g);

  REQUIRE(out_pairs == in_pairs);
}

TEST_CASE("bidir CPO consistency: in-edges mirror out-edges (star)",
          "[bidir][cpo][consistency][star]") {
  // Star: 0->1, 0->2, 0->3, 0->4
  VovBiDirVoid g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});

  auto out_pairs = collect_out_edge_pairs(g);
  auto in_pairs  = collect_in_edge_pairs(g);

  REQUIRE(out_pairs == in_pairs);
}

TEST_CASE("bidir CPO consistency: in-edges mirror out-edges (path)",
          "[bidir][cpo][consistency][path]") {
  // Path: 0->1->2->3
  VovBiDirVoid g({{0, 1}, {1, 2}, {2, 3}});

  auto out_pairs = collect_out_edge_pairs(g);
  auto in_pairs  = collect_in_edge_pairs(g);

  REQUIRE(out_pairs == in_pairs);
}

// =============================================================================
// in_degree matches manual count of incoming out-edges
// =============================================================================

TEST_CASE("bidir CPO consistency: in_degree matches incoming out-edge count",
          "[bidir][cpo][consistency][degree]") {
  // Has vertices with different in-degrees: 0->1, 0->2, 1->2, 2->3, 3->2
  VovBiDirVoid g({{0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 2}});

  for (auto u : vertices(g)) {
    auto  uid     = vertex_id(g, u);
    size_t indeg  = in_degree(g, u);

    // Count by walking all out-edges and checking target
    size_t manual = 0;
    for (auto v : vertices(g))
      for (auto oe : edges(g, v))
        if (target_id(g, oe) == uid)
          ++manual;

    REQUIRE(indeg == manual);
  }
}

TEST_CASE("bidir CPO consistency: in_degree via uid overload",
          "[bidir][cpo][consistency][degree][uid]") {
  // 0->1, 0->2, 1->2
  VovBiDirVoid g({{0, 1}, {0, 2}, {1, 2}});

  REQUIRE(in_degree(g, uint32_t(0)) == 0); // nothing points to 0
  REQUIRE(in_degree(g, uint32_t(1)) == 1); // only 0->1
  REQUIRE(in_degree(g, uint32_t(2)) == 2); // 0->2 and 1->2
}

// =============================================================================
// source_id stability across multiple iterations
// =============================================================================

TEST_CASE("bidir CPO consistency: source_id on in-edges is stable across iterations",
          "[bidir][cpo][consistency][stable]") {
  VovBiDirVoid g({{0, 1}, {0, 2}, {1, 2}});

  auto u2 = *find_vertex(g, uint32_t(2)); // vertex with 2 in-edges

  // Two iterations should yield the same source IDs in the same order
  std::vector<uint32_t> first_pass, second_pass;
  for (auto ie : in_edges(g, u2))
    first_pass.push_back(source_id(g, ie));
  for (auto ie : in_edges(g, u2))
    second_pass.push_back(source_id(g, ie));

  REQUIRE(first_pass == second_pass);
}

// =============================================================================
// Weighted graph: source_id still works
// =============================================================================

TEST_CASE("bidir CPO consistency: weighted graph in-edge pairs mirror out-edge pairs",
          "[bidir][cpo][consistency][weighted]") {
  // 0->1 (w:10), 0->2 (w:20), 1->2 (w:30)
  VovBiDirInt g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});

  auto out_pairs = collect_out_edge_pairs(g);
  auto in_pairs  = collect_in_edge_pairs(g);

  REQUIRE(out_pairs == in_pairs);
}

// =============================================================================
// Container type independence: vov and vol produce identical CPO results
// =============================================================================

TEST_CASE("bidir CPO consistency: vov and vol in-edge pairs are identical",
          "[bidir][cpo][consistency][vol]") {
  VovBiDirVoid g_vov({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
  VolBiDirVoid g_vol({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

  SECTION("in-edge pairs identical") {
    REQUIRE(collect_in_edge_pairs(g_vov) == collect_in_edge_pairs(g_vol));
  }

  SECTION("out-edge pairs identical") {
    REQUIRE(collect_out_edge_pairs(g_vov) == collect_out_edge_pairs(g_vol));
  }

  SECTION("in_degree values identical for all vertices") {
    REQUIRE(num_vertices(g_vov) == num_vertices(g_vol));
    for (uint32_t i = 0; i < 4; ++i)
      REQUIRE(in_degree(g_vov, i) == in_degree(g_vol, i));
  }
}

// =============================================================================
// Empty graph
// =============================================================================

TEST_CASE("bidir CPO consistency: empty graph has no in-edges",
          "[bidir][cpo][consistency][empty]") {
  VovBiDirVoid g;
  g.resize_vertices(4);

  SECTION("all in_degrees are zero") {
    for (auto u : vertices(g))
      REQUIRE(in_degree(g, u) == 0);
  }

  SECTION("all in_edges ranges are empty") {
    for (auto u : vertices(g))
      REQUIRE(std::ranges::empty(in_edges(g, u)));
  }

  SECTION("no in-edge or out-edge pairs") {
    REQUIRE(collect_out_edge_pairs(g).empty());
    REQUIRE(collect_in_edge_pairs(g).empty());
  }
}

// =============================================================================
// Single-vertex self-loop
// =============================================================================

TEST_CASE("bidir CPO consistency: self-loop edge",
          "[bidir][cpo][consistency][selfloop]") {
  // 0->0 (self-loop), 0->1
  VovBiDirVoid g({{0, 0}, {0, 1}});

  SECTION("in-edge and out-edge pairs mirror each other") {
    auto out_pairs = collect_out_edge_pairs(g);
    auto in_pairs  = collect_in_edge_pairs(g);
    REQUIRE(out_pairs == in_pairs);
  }

  SECTION("vertex 0 has self-loop in in-edges") {
    auto u0 = *find_vertex(g, uint32_t(0));
    bool found_self = false;
    for (auto ie : in_edges(g, u0)) {
      if (source_id(g, ie) == 0)
        found_self = true;
    }
    REQUIRE(found_self);
  }
}
