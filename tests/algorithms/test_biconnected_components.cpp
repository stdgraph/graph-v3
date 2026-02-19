/**
 * @file test_biconnected_components.cpp
 * @brief Tests for biconnected components algorithm from biconnected_components.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/biconnected_components.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <algorithm>
#include <queue>
#include <set>
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

/// Sort each inner vector, then sort the outer vector of vectors.
/// Used for order-independent comparison of component lists.
template <typename T>
std::vector<std::vector<T>> normalize_components(std::vector<std::vector<T>> comps) {
  for (auto& c : comps)
    std::sort(c.begin(), c.end());
  std::sort(comps.begin(), comps.end());
  return comps;
}

/// Count how many inner containers contain a given vertex ID.
template <typename T>
size_t count_occurrences(const std::vector<std::vector<T>>& comps, T vid) {
  size_t count = 0;
  for (auto& c : comps) {
    if (std::find(c.begin(), c.end(), vid) != c.end())
      ++count;
  }
  return count;
}

/// BFS-based component count, skipping vertex skip_uid.
template <typename G>
size_t count_components_without(const G& g, typename G::vertex_id_type skip_uid) {
  size_t N = num_vertices(g);
  if (N <= 1)
    return (N == 0 || skip_uid == 0) ? 0 : 1;

  std::vector<bool> visited(N, false);
  visited[skip_uid] = true;

  size_t components = 0;
  for (size_t i = 0; i < N; ++i) {
    auto vid = static_cast<typename G::vertex_id_type>(i);
    if (visited[vid])
      continue;
    ++components;
    std::queue<typename G::vertex_id_type> q;
    q.push(vid);
    visited[vid] = true;
    while (!q.empty()) {
      auto uid = q.front();
      q.pop();
      auto u = *find_vertex(g, uid);
      for (auto&& uv : edges(g, u)) {
        auto tid = target_id(g, uv);
        if (!visited[tid]) {
          visited[tid] = true;
          q.push(tid);
        }
      }
    }
  }
  return components;
}

/// BFS-based full component count.
template <typename G>
size_t count_components(const G& g) {
  size_t N = num_vertices(g);
  if (N == 0)
    return 0;

  std::vector<bool> visited(N, false);
  size_t            components = 0;
  for (size_t i = 0; i < N; ++i) {
    auto vid = static_cast<typename G::vertex_id_type>(i);
    if (visited[vid])
      continue;
    ++components;
    std::queue<typename G::vertex_id_type> q;
    q.push(vid);
    visited[vid] = true;
    while (!q.empty()) {
      auto uid = q.front();
      q.pop();
      auto u = *find_vertex(g, uid);
      for (auto&& uv : edges(g, u)) {
        auto tid = target_id(g, uv);
        if (!visited[tid]) {
          visited[tid] = true;
          q.push(tid);
        }
      }
    }
  }
  return components;
}

/// Returns true if removing uid increases the connected component count.
template <typename G>
bool is_articulation_point_brute(const G& g, typename G::vertex_id_type uid) {
  return count_components_without(g, uid) > count_components(g);
}

/// Check that every vertex appears in at least one component.
template <typename G>
bool all_vertices_covered(const G& g, const std::vector<std::vector<typename G::vertex_id_type>>& comps) {
  size_t N = num_vertices(g);
  std::vector<bool> seen(N, false);
  for (auto& c : comps)
    for (auto vid : c)
      seen[vid] = true;
  for (size_t i = 0; i < N; ++i) {
    if (!seen[i])
      return false;
  }
  return true;
}

/// Check that articulation points appear in >1 component, non-art-points in exactly 1.
template <typename G>
bool articulation_point_multiplicity_ok(const G&                                                       g,
                                        const std::vector<std::vector<typename G::vertex_id_type>>& comps) {
  using vid_t = typename G::vertex_id_type;
  size_t N    = num_vertices(g);
  for (size_t i = 0; i < N; ++i) {
    auto   vid        = static_cast<vid_t>(i);
    size_t occ        = count_occurrences(comps, vid);
    bool   is_art_pt  = is_articulation_point_brute(g, vid);
    if (is_art_pt && occ < 2)
      return false;
    if (!is_art_pt && occ != 1)
      return false;
  }
  return true;
}

// =============================================================================
// Basic Test Cases
// =============================================================================

TEST_CASE("biconnected_components - empty graph", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  Graph                                                    g;
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  REQUIRE(result.empty());
}

TEST_CASE("biconnected_components - single vertex no edges", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  Graph g;
  g.resize_vertices(1);
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0}});
  REQUIRE(normalize_components(result) == expected);
}

TEST_CASE("biconnected_components - single edge", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Bidirectional: 0 - 1
  Graph g({{0, 1}, {1, 0}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}});
  REQUIRE(normalize_components(result) == expected);
}

TEST_CASE("biconnected_components - path graph 0-1-2-3", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Bidirectional path
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  // Each bridge edge is its own biconnected component
  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {1, 2}, {2, 3}});
  REQUIRE(normalize_components(result) == expected);

  // Structural: art-point vertices 1 and 2 appear in 2 components each
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(1)) == 2);
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(2)) == 2);
  REQUIRE(all_vertices_covered(g, result));
  REQUIRE(articulation_point_multiplicity_ok(g, result));
}

TEST_CASE("biconnected_components - cycle graph 5 vertices", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Bidirectional cycle: 0-1-2-3-4-0
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  // One biconnected component containing all 5 vertices
  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2, 3, 4}});
  REQUIRE(normalize_components(result) == expected);
  REQUIRE(all_vertices_covered(g, result));
}

TEST_CASE("biconnected_components - star graph centre=0", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Centre 0 connected to 1,2,3,4 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  // Each spoke is its own biconnected component
  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {0, 2}, {0, 3}, {0, 4}});
  REQUIRE(normalize_components(result) == expected);

  // Centre vertex 0 appears in all 4 components
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(0)) == 4);
  REQUIRE(all_vertices_covered(g, result));
  REQUIRE(articulation_point_multiplicity_ok(g, result));
}

TEST_CASE("biconnected_components - bridge graph two triangles", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Triangle 0-1-2 and triangle 3-4-5, connected by bridge 2-3
  Graph g({
        {0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, // triangle 0-1-2
        {3, 4}, {4, 3}, {4, 5}, {5, 4}, {3, 5}, {5, 3}, // triangle 3-4-5
        {2, 3}, {3, 2}                                    // bridge 2-3
  });
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2}, {2, 3}, {3, 4, 5}});
  REQUIRE(normalize_components(result) == expected);

  // Vertices 2 and 3 are articulation points (appear in 2 components each)
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(2)) == 2);
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(3)) == 2);
  REQUIRE(all_vertices_covered(g, result));
  REQUIRE(articulation_point_multiplicity_ok(g, result));
}

TEST_CASE("biconnected_components - complete graph K4", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // K4: every pair bidirectional
  Graph g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2, 3}});
  REQUIRE(normalize_components(result) == expected);
  REQUIRE(all_vertices_covered(g, result));
}

TEST_CASE("biconnected_components - disconnected graph", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Component 1: path 0-1-2 (bidirectional)
  // Component 2: single edge 3-4 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {3, 4}, {4, 3}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {1, 2}, {3, 4}});
  REQUIRE(normalize_components(result) == expected);
  REQUIRE(all_vertices_covered(g, result));
  REQUIRE(articulation_point_multiplicity_ok(g, result));
}

TEST_CASE("biconnected_components - barbell graph", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Two K3 triangles (0-1-2) and (4-5-6) joined by path 2-3-4
  Graph g({
        {0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, // triangle 0-1-2
        {4, 5}, {5, 4}, {5, 6}, {6, 5}, {4, 6}, {6, 4}, // triangle 4-5-6
        {2, 3}, {3, 2},                                   // bridge 2-3
        {3, 4}, {4, 3}                                    // bridge 3-4
  });
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected =
        normalize_components<typename Graph::vertex_id_type>({{0, 1, 2}, {2, 3}, {3, 4}, {4, 5, 6}});
  REQUIRE(normalize_components(result) == expected);

  // Vertices 2, 3, 4 are articulation points
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(2)) == 2);
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(3)) == 2);
  REQUIRE(count_occurrences(result, static_cast<typename Graph::vertex_id_type>(4)) == 2);
  REQUIRE(all_vertices_covered(g, result));
  REQUIRE(articulation_point_multiplicity_ok(g, result));
}

TEST_CASE("biconnected_components - self-loop does not affect result", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Path 0-1-2 with self-loop on vertex 1
  Graph g({{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {1, 2}});
  REQUIRE(normalize_components(result) == expected);
}

TEST_CASE("biconnected_components - parallel edges", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  SECTION("parallel edge on a bridge - still same component") {
    // Path 0-1-2 with duplicate edge 0-1. Vertex 1 is still an articulation point.
    Graph g({{0, 1}, {1, 0}, {0, 1}, {1, 0}, {1, 2}, {2, 1}});
    std::vector<std::vector<typename Graph::vertex_id_type>> result;
    biconnected_components(g, result);

    // The parallel edge doesn't remove the articulation point (removing vertex 1
    // still disconnects 0 from 2), so we still get two components.
    auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {1, 2}});
    REQUIRE(normalize_components(result) == expected);
  }

  SECTION("triangle with doubled edge") {
    // Triangle 0-1-2 with edge 0-1 doubled. Already biconnected.
    Graph g({{0, 1}, {1, 0}, {0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 0}, {0, 2}});
    std::vector<std::vector<typename Graph::vertex_id_type>> result;
    biconnected_components(g, result);

    auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2}});
    REQUIRE(normalize_components(result) == expected);
  }
}

TEST_CASE("biconnected_components - multiple isolated vertices", "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // 3 isolated vertices
  Graph g;
  g.resize_vertices(3);
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0}, {1}, {2}});
  REQUIRE(normalize_components(result) == expected);
}

TEST_CASE("biconnected_components - disconnected with isolated vertex",
          "[algorithm][biconnected_components]") {
  using Graph = vov_void;

  // Edge 0-1, isolated vertex 2
  Graph g({{0, 1}, {1, 0}});
  g.resize_vertices(3);
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {2}});
  REQUIRE(normalize_components(result) == expected);
}

// =============================================================================
// Parameterized Tests — container independence
// =============================================================================

TEMPLATE_TEST_CASE("biconnected_components - path graph (typed)", "[algorithm][biconnected_components]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {1, 2}, {2, 3}});
  REQUIRE(normalize_components(result) == expected);
}

TEMPLATE_TEST_CASE("biconnected_components - cycle graph (typed)", "[algorithm][biconnected_components]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2, 3, 4}});
  REQUIRE(normalize_components(result) == expected);
}

TEMPLATE_TEST_CASE("biconnected_components - star graph (typed)", "[algorithm][biconnected_components]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1}, {0, 2}, {0, 3}, {0, 4}});
  REQUIRE(normalize_components(result) == expected);
}

TEMPLATE_TEST_CASE("biconnected_components - bridge graph (typed)", "[algorithm][biconnected_components]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({
        {0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0},
        {3, 4}, {4, 3}, {4, 5}, {5, 4}, {3, 5}, {5, 3},
        {2, 3}, {3, 2}
  });
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2}, {2, 3}, {3, 4, 5}});
  REQUIRE(normalize_components(result) == expected);
}

TEMPLATE_TEST_CASE("biconnected_components - K4 (typed)", "[algorithm][biconnected_components]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
  std::vector<std::vector<typename Graph::vertex_id_type>> result;

  biconnected_components(g, result);

  auto expected = normalize_components<typename Graph::vertex_id_type>({{0, 1, 2, 3}});
  REQUIRE(normalize_components(result) == expected);
}
