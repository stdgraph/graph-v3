/**
 * @file test_jaccard.cpp
 * @brief Tests for Jaccard coefficient algorithm from jaccard.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/jaccard.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_set>
#include <vector>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// =============================================================================
// Helper Functions
// =============================================================================

/// Collect all Jaccard callbacks into a map keyed by (uid, vid).
template <typename G>
using JaccardMap =
      std::map<std::pair<typename std::remove_cvref_t<G>::vertex_id_type,
                         typename std::remove_cvref_t<G>::vertex_id_type>,
               double>;

template <typename G>
JaccardMap<G> collect_jaccard(G&& g) {
  using Graph = std::remove_cvref_t<G>;
  std::map<std::pair<typename Graph::vertex_id_type, typename Graph::vertex_id_type>, double> result;
  jaccard_coefficient(g, [&](auto uid, auto vid, auto& /*uv*/, double val) {
    result[{uid, vid}] = val;
  });
  return result;
}

/// Brute-force Jaccard coefficient for a pair of vertices.
template <typename G>
double brute_jaccard(const G& g, typename G::vertex_id_type u, typename G::vertex_id_type v) {
  std::unordered_set<typename G::vertex_id_type> nu, nv;
  auto uu = *find_vertex(g, u);
  for (auto&& e : edges(g, uu)) {
    auto t = target_id(g, e);
    if (t != u)
      nu.insert(t);
  }
  auto vv = *find_vertex(g, v);
  for (auto&& e : edges(g, vv)) {
    auto t = target_id(g, e);
    if (t != v)
      nv.insert(t);
  }
  size_t isect = 0;
  for (auto x : nu)
    if (nv.count(x))
      ++isect;
  size_t uni = nu.size() + nv.size() - isect;
  return (uni == 0) ? 0.0 : static_cast<double>(isect) / static_cast<double>(uni);
}

/// Near-equal for floating-point.
bool approx_equal(double a, double b, double eps = 1e-9) {
  return std::abs(a - b) < eps;
}

// =============================================================================
// Basic Test Cases
// =============================================================================

TEST_CASE("jaccard_coefficient - empty graph", "[algorithm][jaccard]") {
  using Graph = vov_void;

  Graph g;
  size_t call_count = 0;
  jaccard_coefficient(g, [&](auto, auto, auto&, double) { ++call_count; });

  REQUIRE(call_count == 0);
}

TEST_CASE("jaccard_coefficient - single vertex no edges", "[algorithm][jaccard]") {
  using Graph = vov_void;

  Graph g;
  g.resize_vertices(1);
  size_t call_count = 0;
  jaccard_coefficient(g, [&](auto, auto, auto&, double) { ++call_count; });

  REQUIRE(call_count == 0);
}

TEST_CASE("jaccard_coefficient - single edge", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Bidirectional: 0 - 1
  Graph g({{0, 1}, {1, 0}});
  auto  result = collect_jaccard(g);

  // Two directed edges → two callbacks
  REQUIRE(result.size() == 2);

  // N(0)={1}, N(1)={0}, intersection=∅, union={0,1} → J=0
  REQUIRE(approx_equal(result[{0, 1}], 0.0));
  REQUIRE(approx_equal(result[{1, 0}], 0.0));
}

TEST_CASE("jaccard_coefficient - path 0-1-2", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Bidirectional path
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}});
  auto  result = collect_jaccard(g);

  // 4 directed edges
  REQUIRE(result.size() == 4);

  // N(0)={1}, N(1)={0,2}, N(2)={1}
  // J(0,1): intersect=∅, union={0,1,2} → 0/3 = 0
  // J(1,2): intersect=∅, union={0,1,2} → 0/3 = 0
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, 0.0));
  }
}

TEST_CASE("jaccard_coefficient - triangle 0-1-2", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Bidirectional triangle
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
  auto  result = collect_jaccard(g);

  // 6 directed edges
  REQUIRE(result.size() == 6);

  // N(0)={1,2}, N(1)={0,2}, N(2)={0,1}
  // For any pair: |intersect|=1, |union|=3 → J = 1/3
  double expected = 1.0 / 3.0;
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, expected));
  }

  // Cross-check with brute force
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, brute_jaccard(g, key.first, key.second)));
  }
}

TEST_CASE("jaccard_coefficient - complete graph K4", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // K4 bidirectional
  Graph g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
  auto  result = collect_jaccard(g);

  // 12 directed edges
  REQUIRE(result.size() == 12);

  // N(u) has 3 vertices for each u, any pair shares 2 → |union|=4 → J = 2/4 = 0.5
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, 0.5));
  }
}

TEST_CASE("jaccard_coefficient - star graph centre=0", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Centre 0 connected to 1,2,3,4 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}});
  auto  result = collect_jaccard(g);

  // 8 directed edges
  REQUIRE(result.size() == 8);

  // N(0)={1,2,3,4}, N(leaf)={0}
  // J(0,i): intersect=∅, union={0,1,2,3,4}\{one side} → |inter|=0 → J=0
  // J(i,0): same
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, 0.0));
  }
}

TEST_CASE("jaccard_coefficient - diamond graph (K4 minus edge 0-3)", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // K4 minus edge 0-3 (bidirectional)
  // Edges: 0-1, 0-2, 1-2, 1-3, 2-3
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});
  auto  result = collect_jaccard(g);

  // 10 directed edges
  REQUIRE(result.size() == 10);

  // N(0)={1,2}, N(1)={0,2,3}, N(2)={0,1,3}, N(3)={1,2}

  // J(1,2): intersect={0,3}, union={0,1,2,3} → 2/4 = 0.5
  REQUIRE(approx_equal(result[{1, 2}], 0.5));
  REQUIRE(approx_equal(result[{2, 1}], 0.5));

  // J(0,1): intersect={2}, union={0,1,2,3} → 1/4 = 0.25
  REQUIRE(approx_equal(result[{0, 1}], 0.25));
  REQUIRE(approx_equal(result[{1, 0}], 0.25));

  // J(0,2): intersect={1}, union={0,1,2,3} → 1/4 = 0.25
  REQUIRE(approx_equal(result[{0, 2}], 0.25));
  REQUIRE(approx_equal(result[{2, 0}], 0.25));

  // J(1,3): intersect={2}, union={0,1,2,3} → 1/4 = 0.25
  REQUIRE(approx_equal(result[{1, 3}], 0.25));
  REQUIRE(approx_equal(result[{3, 1}], 0.25));

  // J(2,3): intersect={1}, union={0,1,2,3} → 1/4 = 0.25
  REQUIRE(approx_equal(result[{2, 3}], 0.25));
  REQUIRE(approx_equal(result[{3, 2}], 0.25));

  // Cross-check all with brute force
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, brute_jaccard(g, key.first, key.second)));
  }
}

TEST_CASE("jaccard_coefficient - self-loop ignored", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Path 0-1-2 with self-loop on vertex 1
  Graph g({{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}});
  auto  result = collect_jaccard(g);

  // 4 directed edges (self-loop skipped)
  REQUIRE(result.size() == 4);

  // Same as path 0-1-2 without self-loop: all Jaccard = 0
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, 0.0));
  }
}

TEST_CASE("jaccard_coefficient - disconnected with isolated vertex", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Triangle 0-1-2, isolated vertex 3
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
  g.resize_vertices(4);
  auto result = collect_jaccard(g);

  // 6 directed edges (from the triangle only)
  REQUIRE(result.size() == 6);

  // Same Jaccard values as a pure triangle: 1/3
  double expected = 1.0 / 3.0;
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, expected));
  }

  // Vertex 3 should not appear in any callback
  for (auto& [key, val] : result) {
    REQUIRE(key.first != 3);
    REQUIRE(key.second != 3);
  }
}

TEST_CASE("jaccard_coefficient - callback count matches directed edges", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Triangle 0-1-2
  Graph  g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
  size_t call_count = 0;
  jaccard_coefficient(g, [&](auto, auto, auto&, double) { ++call_count; });

  // Exactly 6 callbacks (one per directed edge)
  REQUIRE(call_count == 6);
}

TEST_CASE("jaccard_coefficient - symmetry J(u,v) == J(v,u)", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // Diamond graph
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});
  auto  result = collect_jaccard(g);

  // Check symmetry for all (u,v)/(v,u) pairs
  for (auto& [key, val] : result) {
    auto reverse = std::make_pair(key.second, key.first);
    auto it      = result.find(reverse);
    REQUIRE(it != result.end());
    REQUIRE(approx_equal(val, it->second));
  }
}

TEST_CASE("jaccard_coefficient - values in [0, 1]", "[algorithm][jaccard]") {
  using Graph = vov_void;

  // K4 + star = a reasonable graph with varied Jaccard values
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2},
           {0, 4}, {4, 0}});
  auto  result = collect_jaccard(g);

  for (auto& [key, val] : result) {
    REQUIRE(val >= 0.0);
    REQUIRE(val <= 1.0);
  }
}

// =============================================================================
// Parameterized Tests — container independence
// =============================================================================

TEMPLATE_TEST_CASE("jaccard_coefficient - triangle (typed)", "[algorithm][jaccard]", vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
  auto  result = collect_jaccard(g);

  REQUIRE(result.size() == 6);
  double expected = 1.0 / 3.0;
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, expected));
  }
}

TEMPLATE_TEST_CASE("jaccard_coefficient - K4 (typed)", "[algorithm][jaccard]", vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
  auto  result = collect_jaccard(g);

  REQUIRE(result.size() == 12);
  for (auto& [key, val] : result) {
    REQUIRE(approx_equal(val, 0.5));
  }
}
