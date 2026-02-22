/**
 * @file test_in_neighbors.cpp
 * @brief Tests for incoming / outgoing neighbors view factory functions.
 *
 * Verifies that the accessor-parameterized neighbors views compile and
 * iterate correctly:
 *   - out_neighbors(g, u)      — explicit outgoing (equivalent to neighbors(g, u))
 *   - in_neighbors(g, u)       — incoming via in_edge_accessor
 *   - basic_out_neighbors(g, uid)
 *   - basic_in_neighbors(g, uid)
 *
 * Uses undirected_adjacency_list as the only currently available
 * bidirectional_adjacency_list container.
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
//   |       /
//  200   300
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

TEST_CASE("in_neighbors - concept prerequisites", "[in_neighbors][concept]") {
  static_assert(adj::bidirectional_adjacency_list<Graph>);
  static_assert(adj::index_bidirectional_adjacency_list<Graph>);
}

// =============================================================================
// out_neighbors — must match neighbors()
// =============================================================================

TEST_CASE("out_neighbors matches neighbors", "[in_neighbors][out]") {
  auto g  = make_triangle();
  auto v0 = *adj::find_vertex(g, 0u);

  SECTION("no VVF — same count and target_ids") {
    auto ref  = view::neighbors(g, v0);
    auto test = view::out_neighbors(g, v0);

    REQUIRE(ref.size() == test.size());

    std::vector<unsigned> ref_ids, test_ids;
    for (auto [tid, v] : ref)
      ref_ids.push_back(tid);
    for (auto [tid, v] : test)
      test_ids.push_back(tid);

    REQUIRE(ref_ids == test_ids);
  }

  SECTION("with VVF") {
    auto vvf  = [](const auto&, auto v) { return v.vertex_id() * 10; };
    auto ref  = view::neighbors(g, v0, vvf);
    auto test = view::out_neighbors(g, v0, vvf);

    REQUIRE(ref.size() == test.size());

    std::vector<unsigned> ref_vals, test_vals;
    for (auto [tid, v, val] : ref)
      ref_vals.push_back(val);
    for (auto [tid, v, val] : test)
      test_vals.push_back(val);

    REQUIRE(ref_vals == test_vals);
  }

  SECTION("from vertex id") {
    auto ref  = view::neighbors(g, 0u);
    auto test = view::out_neighbors(g, 0u);
    REQUIRE(ref.size() == test.size());
  }

  SECTION("with VVF from vertex id") {
    auto vvf  = [](const auto&, auto v) { return v.vertex_id() * 10; };
    auto ref  = view::neighbors(g, 0u, vvf);
    auto test = view::out_neighbors(g, 0u, vvf);
    REQUIRE(ref.size() == test.size());
  }
}

// =============================================================================
// basic_out_neighbors — must match basic_neighbors()
// =============================================================================

TEST_CASE("basic_out_neighbors matches basic_neighbors", "[in_neighbors][basic_out]") {
  auto g = make_triangle();

  SECTION("no VVF") {
    auto ref  = view::basic_neighbors(g, 0u);
    auto test = view::basic_out_neighbors(g, 0u);

    REQUIRE(ref.size() == test.size());

    std::vector<unsigned> ref_ids, test_ids;
    for (auto ni : ref)
      ref_ids.push_back(ni.target_id);
    for (auto ni : test)
      test_ids.push_back(ni.target_id);

    REQUIRE(ref_ids == test_ids);
  }

  SECTION("with VVF") {
    auto vvf  = [](const auto&, auto v) { return v.vertex_id() * 10; };
    auto ref  = view::basic_neighbors(g, 0u, vvf);
    auto test = view::basic_out_neighbors(g, 0u, vvf);
    REQUIRE(ref.size() == test.size());
  }
}

// =============================================================================
// in_neighbors — incoming neighbors
// =============================================================================

TEST_CASE("in_neighbors iterates in_edges", "[in_neighbors][in]") {
  auto g  = make_triangle();
  auto v0 = *adj::find_vertex(g, 0u);

  SECTION("neighbor count matches in_degree") {
    auto nview = view::in_neighbors(g, v0);
    REQUIRE(nview.size() == adj::in_degree(g, v0));
  }

  SECTION("neighbor count matches degree for undirected graph") {
    for (auto v : adj::vertices(g)) {
      auto nview = view::in_neighbors(g, v);
      REQUIRE(nview.size() == adj::degree(g, v));
    }
  }

  SECTION("no VVF — structured binding") {
    auto      nview = view::in_neighbors(g, v0);
    std::size_t count = 0;
    for (auto [tid, v] : nview) {
      (void)tid;
      (void)v;
      ++count;
    }
    REQUIRE(count == 2); // vertex 0 has 2 edges
  }

  SECTION("with VVF") {
    auto vvf   = [](const auto&, auto v) { return static_cast<int>(v.vertex_id() * 100); };
    auto nview = view::in_neighbors(g, v0, vvf);
    REQUIRE(nview.size() == 2);

    std::size_t count = 0;
    for (auto [tid, v, val] : nview) {
      (void)tid;
      (void)val;
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("from vertex id") {
    auto nview = view::in_neighbors(g, 1u);
    REQUIRE(nview.size() == adj::in_degree(g, *adj::find_vertex(g, 1u)));
  }

  SECTION("with VVF from vertex id") {
    auto vvf   = [](const auto&, auto v) { return static_cast<int>(v.vertex_id()); };
    auto nview = view::in_neighbors(g, 1u, vvf);
    REQUIRE(nview.size() == 2); // vertex 1: edges to 0, 2
  }
}

// =============================================================================
// basic_in_neighbors
// =============================================================================

TEST_CASE("basic_in_neighbors", "[in_neighbors][basic_in]") {
  auto g = make_triangle();

  SECTION("no VVF — iteration count") {
    auto bview = view::basic_in_neighbors(g, 0u);
    REQUIRE(bview.size() == 2);
  }

  SECTION("with VVF") {
    auto vvf   = [](const auto&, auto v) { return static_cast<int>(v.vertex_id()); };
    auto bview = view::basic_in_neighbors(g, 0u, vvf);
    REQUIRE(bview.size() == 2);

    std::size_t count = 0;
    for (auto ni : bview) {
      (void)ni;
      ++count;
    }
    REQUIRE(count == 2);
  }
}

// =============================================================================
// in_neighbors — isolated vertex
// =============================================================================

TEST_CASE("in_neighbors - isolated vertex", "[in_neighbors][empty]") {
  Graph g(0);
  g.create_vertex(10);
  g.create_vertex(20);
  g.create_edge(1, 1, 99); // self-loop on 1

  auto v0    = *adj::find_vertex(g, 0u);
  auto nview = view::in_neighbors(g, v0);

  REQUIRE(nview.begin() == nview.end());
  REQUIRE(nview.size() == 0);
}

// =============================================================================
// in_neighbors on const graph
// =============================================================================

TEST_CASE("in_neighbors - const graph", "[in_neighbors][const]") {
  auto        g  = make_triangle();
  const auto& cg = g;

  auto v0    = *adj::find_vertex(cg, 0u);
  auto nview = view::in_neighbors(cg, v0);

  REQUIRE(nview.size() == 2);
}
