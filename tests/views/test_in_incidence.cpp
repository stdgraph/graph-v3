/**
 * @file test_in_incidence.cpp
 * @brief Tests for incoming / outgoing incidence view factory functions.
 *
 * Verifies that the accessor-parameterized incidence views compile and
 * iterate correctly:
 *   - out_incidence(g, u)      — explicit outgoing (equivalent to incidence(g, u))
 *   - in_incidence(g, u)       — incoming via in_edge_accessor
 *   - basic_out_incidence(g, uid)
 *   - basic_in_incidence(g, uid)
 *
 * Uses undirected_adjacency_list as the only currently available
 * bidirectional_adjacency_list container.  For undirected graphs
 * in_edges(g,u)==edges(g,u), so incoming views iterate the same edge
 * list but report source_id as the neighbor.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/graph.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <algorithm>
#include <vector>
#include <set>

using graph::container::undirected_adjacency_list;
using Graph = undirected_adjacency_list<int, int, int>;

namespace adj  = graph::adj_list;
namespace view = graph::views;

// ---------------------------------------------------------------------------
// Helper: build a small triangle graph
//   0 --100-- 1
//   |       / |
//  200  300   (no 1-2 direct; we do 0-2 and 1-2)
//   |  /
//   2
// Edges: (0,1,100), (0,2,200), (1,2,300)
// ---------------------------------------------------------------------------
static Graph make_triangle() {
  Graph g(0);
  g.create_vertex(10); // 0
  g.create_vertex(20); // 1
  g.create_vertex(30); // 2
  g.create_edge(0, 1, 100);
  g.create_edge(0, 2, 200);
  g.create_edge(1, 2, 300);
  return g;
}

// =============================================================================
// Concept checks
// =============================================================================

TEST_CASE("in_incidence - concept prerequisites", "[in_incidence][concept]") {
  static_assert(adj::bidirectional_adjacency_list<Graph>);
  static_assert(adj::index_bidirectional_adjacency_list<Graph>);
}

// =============================================================================
// out_incidence — must match incidence()
// =============================================================================

TEST_CASE("out_incidence matches incidence", "[in_incidence][out]") {
  auto g  = make_triangle();
  auto v0 = *adj::find_vertex(g, 0u);

  SECTION("no EVF — same edge count and target_ids") {
    auto ref  = view::incidence(g, v0);
    auto test = view::out_incidence(g, v0);

    REQUIRE(ref.size() == test.size());

    std::vector<unsigned> ref_ids, test_ids;
    for (auto [tid, e] : ref)
      ref_ids.push_back(tid);
    for (auto [tid, e] : test)
      test_ids.push_back(tid);

    REQUIRE(ref_ids == test_ids);
  }

  SECTION("with EVF") {
    auto evf  = [](const auto& g, auto e) { return adj::edge_value(g, e); };
    auto ref  = view::incidence(g, v0, evf);
    auto test = view::out_incidence(g, v0, evf);

    REQUIRE(ref.size() == test.size());

    std::vector<int> ref_vals, test_vals;
    for (auto [tid, e, val] : ref)
      ref_vals.push_back(val);
    for (auto [tid, e, val] : test)
      test_vals.push_back(val);

    REQUIRE(ref_vals == test_vals);
  }

  SECTION("from vertex id") {
    auto ref  = view::incidence(g, 0u);
    auto test = view::out_incidence(g, 0u);

    REQUIRE(ref.size() == test.size());
  }

  SECTION("with EVF from vertex id") {
    auto evf  = [](const auto& g, auto e) { return adj::edge_value(g, e); };
    auto ref  = view::incidence(g, 0u, evf);
    auto test = view::out_incidence(g, 0u, evf);

    REQUIRE(ref.size() == test.size());
  }
}

// =============================================================================
// basic_out_incidence — must match basic_incidence()
// =============================================================================

TEST_CASE("basic_out_incidence matches basic_incidence", "[in_incidence][basic_out]") {
  auto g = make_triangle();

  SECTION("no EVF") {
    auto ref  = view::basic_incidence(g, 0u);
    auto test = view::basic_out_incidence(g, 0u);

    REQUIRE(ref.size() == test.size());

    std::vector<unsigned> ref_ids, test_ids;
    for (auto ei : ref)
      ref_ids.push_back(ei.target_id);
    for (auto ei : test)
      test_ids.push_back(ei.target_id);

    REQUIRE(ref_ids == test_ids);
  }

  SECTION("with EVF") {
    auto evf  = [](const auto& g, auto e) { return adj::edge_value(g, e); };
    auto ref  = view::basic_incidence(g, 0u, evf);
    auto test = view::basic_out_incidence(g, 0u, evf);

    REQUIRE(ref.size() == test.size());
  }
}

// =============================================================================
// in_incidence — incoming edges
// =============================================================================

TEST_CASE("in_incidence iterates in_edges", "[in_incidence][in]") {
  auto g  = make_triangle();
  auto v0 = *adj::find_vertex(g, 0u);

  SECTION("edge count matches in_degree") {
    auto iview = view::in_incidence(g, v0);
    REQUIRE(iview.size() == adj::in_degree(g, v0));
  }

  SECTION("edge count matches degree for undirected graph") {
    // For undirected graphs, in_degree == degree
    for (auto v : adj::vertices(g)) {
      auto iview = view::in_incidence(g, v);
      REQUIRE(iview.size() == adj::degree(g, v));
    }
  }

  SECTION("no EVF — structured binding") {
    auto iview = view::in_incidence(g, v0);
    std::size_t count = 0;
    for (auto [tid, e] : iview) {
      // For undirected, source_id (used as neighbor_id) == iterating vertex id
      (void)tid;
      (void)e;
      ++count;
    }
    REQUIRE(count == 2); // vertex 0 has 2 edges
  }

  SECTION("with EVF") {
    auto evf   = [](const auto& g, auto e) { return adj::edge_value(g, e); };
    auto iview = view::in_incidence(g, v0, evf);
    REQUIRE(iview.size() == 2);

    std::set<int> values;
    for (auto [tid, e, val] : iview) {
      values.insert(val);
    }
    // Edge values from vertex 0: 100 (to 1) and 200 (to 2)
    REQUIRE(values == std::set<int>{100, 200});
  }

  SECTION("from vertex id") {
    auto iview = view::in_incidence(g, 1u);
    REQUIRE(iview.size() == adj::in_degree(g, *adj::find_vertex(g, 1u)));
  }

  SECTION("with EVF from vertex id") {
    auto evf   = [](const auto& g, auto e) { return adj::edge_value(g, e); };
    auto iview = view::in_incidence(g, 1u, evf);
    REQUIRE(iview.size() == 2); // vertex 1: edges to 0 and 2
  }
}

// =============================================================================
// basic_in_incidence
// =============================================================================

TEST_CASE("basic_in_incidence", "[in_incidence][basic_in]") {
  auto g = make_triangle();

  SECTION("no EVF — iteration count") {
    auto bview = view::basic_in_incidence(g, 0u);
    REQUIRE(bview.size() == 2);
  }

  SECTION("with EVF — values accessible") {
    auto evf   = [](const auto& g, auto e) { return adj::edge_value(g, e); };
    auto bview = view::basic_in_incidence(g, 0u, evf);
    REQUIRE(bview.size() == 2);

    std::set<int> values;
    for (auto ei : bview) {
      values.insert(ei.value);
    }
    REQUIRE(values == std::set<int>{100, 200});
  }
}

// =============================================================================
// in_incidence — empty vertex
// =============================================================================

TEST_CASE("in_incidence - isolated vertex", "[in_incidence][empty]") {
  Graph g(0);
  g.create_vertex(10); // 0  — no edges
  g.create_vertex(20); // 1
  g.create_edge(1, 1, 99); // self-loop on 1 (just so the graph isn't trivial)

  auto v0    = *adj::find_vertex(g, 0u);
  auto iview = view::in_incidence(g, v0);

  REQUIRE(iview.begin() == iview.end());
  REQUIRE(iview.size() == 0);
}

// =============================================================================
// in_incidence on const graph
// =============================================================================

TEST_CASE("in_incidence - const graph", "[in_incidence][const]") {
  auto        g  = make_triangle();
  const auto& cg = g;

  auto v0    = *adj::find_vertex(cg, 0u);
  auto iview = view::in_incidence(cg, v0);

  REQUIRE(iview.size() == 2);
}
