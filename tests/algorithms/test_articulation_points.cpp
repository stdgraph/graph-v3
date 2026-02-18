/**
 * @file test_articulation_points.cpp
 * @brief Tests for articulation points algorithm from articulation_points.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/articulation_points.hpp>
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

/// Sort both vectors and compare (order-independent equality).
template <typename T>
bool same_set(std::vector<T> a, std::vector<T> b) {
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());
  return a == b;
}

/// Brute-force: count weakly connected components using BFS, skipping vertex @p skip_uid.
template <typename G>
size_t count_components_without(const G& g, typename G::vertex_id_type skip_uid) {
  size_t N = num_vertices(g);
  if (N <= 1) {
    return (N == 0 || skip_uid == 0) ? 0 : 1;
  }

  std::vector<bool> visited(N, false);
  visited[skip_uid] = true; // pretend the removed vertex is already visited

  size_t components = 0;

  for (size_t i = 0; i < N; ++i) {
    auto vid = static_cast<typename G::vertex_id_type>(i);
    if (visited[vid])
      continue;

    // BFS from vid
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

/// Brute-force: count components of the full graph using BFS.
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

/// Brute-force check: returns true if removing uid increases the number of components.
template <typename G>
bool is_articulation_point_brute(const G& g, typename G::vertex_id_type uid) {
  size_t orig_components    = count_components(g);
  size_t without_components = count_components_without(g, uid);
  return without_components > orig_components;
}

// =============================================================================
// Basic Test Cases
// =============================================================================

TEST_CASE("articulation_points - empty graph", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  Graph                                       g;
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  REQUIRE(result.empty());
}

TEST_CASE("articulation_points - single vertex no edges", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  Graph g;
  g.resize_vertices(1);
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  REQUIRE(result.empty());
}

TEST_CASE("articulation_points - single edge", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Bidirectional: 0 - 1
  Graph g({{0, 1}, {1, 0}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Removing either vertex leaves a single-vertex graph — still connected
  REQUIRE(result.empty());
}

TEST_CASE("articulation_points - path graph 0-1-2-3", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Bidirectional path
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Interior vertices 1 and 2 are articulation points
  REQUIRE(same_set(result, {1u, 2u}));

  // Cross-check with brute force
  for (size_t i = 0; i < num_vertices(g); ++i) {
    auto vid = static_cast<typename Graph::vertex_id_type>(i);
    bool expected =
          std::find(result.begin(), result.end(), vid) != result.end();
    REQUIRE(is_articulation_point_brute(g, vid) == expected);
  }
}

TEST_CASE("articulation_points - cycle graph 5 vertices", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Bidirectional cycle: 0-1-2-3-4-0
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // No articulation points in a cycle
  REQUIRE(result.empty());
}

TEST_CASE("articulation_points - star graph centre=0", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Centre 0 connected to 1,2,3,4 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Centre vertex is the only articulation point
  REQUIRE(same_set(result, {0u}));
}

TEST_CASE("articulation_points - bridge graph two triangles", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Triangle 0-1-2 and triangle 3-4-5, connected by bridge 2-3
  // Bidirectional edges
  Graph g({
        {0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, // triangle 0-1-2
        {3, 4}, {4, 3}, {4, 5}, {5, 4}, {3, 5}, {5, 3}, // triangle 3-4-5
        {2, 3}, {3, 2}                                    // bridge 2-3
  });
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Endpoints of the bridge are articulation points
  REQUIRE(same_set(result, {2u, 3u}));

  // Cross-check with brute force
  for (size_t i = 0; i < num_vertices(g); ++i) {
    auto vid = static_cast<typename Graph::vertex_id_type>(i);
    bool expected =
          std::find(result.begin(), result.end(), vid) != result.end();
    REQUIRE(is_articulation_point_brute(g, vid) == expected);
  }
}

TEST_CASE("articulation_points - complete graph K4", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // K4: every pair bidirectional
  Graph g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // No articulation points in a complete graph
  REQUIRE(result.empty());
}

TEST_CASE("articulation_points - disconnected graph", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Component 1: path 0-1-2 (bidirectional)
  // Component 2: single edge 3-4 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {3, 4}, {4, 3}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Vertex 1 is an articulation point of component 1
  // Component 2 has no articulation points
  REQUIRE(same_set(result, {1u}));
}

TEST_CASE("articulation_points - barbell graph", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Two K3 triangles (0-1-2) and (4-5-6) joined by path 2-3-4
  // Bidirectional edges
  Graph g({
        {0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, // triangle 0-1-2
        {4, 5}, {5, 4}, {5, 6}, {6, 5}, {4, 6}, {6, 4}, // triangle 4-5-6
        {2, 3}, {3, 2},                                   // bridge 2-3
        {3, 4}, {4, 3}                                    // bridge 3-4
  });
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Vertices 2, 3, and 4 are articulation points
  REQUIRE(same_set(result, {2u, 3u, 4u}));

  // Cross-check with brute force
  for (size_t i = 0; i < num_vertices(g); ++i) {
    auto vid = static_cast<typename Graph::vertex_id_type>(i);
    bool expected =
          std::find(result.begin(), result.end(), vid) != result.end();
    REQUIRE(is_articulation_point_brute(g, vid) == expected);
  }
}

TEST_CASE("articulation_points - self-loop does not affect result", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  // Path 0-1-2 with self-loop on vertex 1 (bidirectional path edges)
  Graph g({{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  // Same as path 0-1-2 without self-loop: vertex 1 is the articulation point
  REQUIRE(same_set(result, {1u}));
}

TEST_CASE("articulation_points - multi-edges handled correctly", "[algorithm][articulation_points]") {
  using Graph = vov_void;

  SECTION("parallel edge creates a cycle - no articulation point") {
    // Path 0-1-2 with a duplicate edge 0-1 (i.e., two parallel edges between 0 and 1).
    // The parallel edge acts like a cycle on {0,1}, so vertex 1 is no longer a
    // cut vertex for the 0-1 pair.  Only vertex 1 is needed to reach vertex 2,
    // so vertex 1 is still an articulation point.
    // However, if the *bridge* edge 1-2 also has a parallel, then no cut vertices remain.
    Graph g_bridge({
          {0, 1}, {1, 0},
          {0, 1}, {1, 0}, // parallel edge 0-1
          {1, 2}, {2, 1}
    });
    std::vector<typename Graph::vertex_id_type> result;
    articulation_points(g_bridge, std::back_inserter(result));
    // Vertex 1 is still an articulation point (removing it disconnects 0 from 2)
    REQUIRE(same_set(result, {1u}));

    // Brute-force cross-check
    for (size_t i = 0; i < num_vertices(g_bridge); ++i) {
      auto vid = static_cast<typename Graph::vertex_id_type>(i);
      bool expected = std::find(result.begin(), result.end(), vid) != result.end();
      REQUIRE(is_articulation_point_brute(g_bridge, vid) == expected);
    }
  }

  SECTION("all bridges doubled - articulation point unchanged") {
    // Path 0-1-2 with every edge doubled.  Parallel edges do NOT remove
    // articulation points because removing the vertex deletes ALL its
    // incident edges.  Vertex 1 still disconnects 0 from 2.
    Graph g_all({
          {0, 1}, {1, 0},
          {0, 1}, {1, 0}, // parallel 0-1
          {1, 2}, {2, 1},
          {1, 2}, {2, 1}  // parallel 1-2
    });
    std::vector<typename Graph::vertex_id_type> result;
    articulation_points(g_all, std::back_inserter(result));
    // Vertex 1 is still an articulation point
    REQUIRE(same_set(result, {1u}));

    // Brute-force cross-check
    for (size_t i = 0; i < num_vertices(g_all); ++i) {
      auto vid = static_cast<typename Graph::vertex_id_type>(i);
      bool expected = std::find(result.begin(), result.end(), vid) != result.end();
      REQUIRE(is_articulation_point_brute(g_all, vid) == expected);
    }
  }

  SECTION("triangle with one doubled edge") {
    // Triangle 0-1-2 with edge 0-1 doubled.
    // Already biconnected; doubling an edge changes nothing.
    Graph g_tri({
          {0, 1}, {1, 0},
          {0, 1}, {1, 0}, // parallel 0-1
          {1, 2}, {2, 1},
          {2, 0}, {0, 2}
    });
    std::vector<typename Graph::vertex_id_type> result;
    articulation_points(g_tri, std::back_inserter(result));
    REQUIRE(result.empty());
  }
}

// =============================================================================
// Parameterized Tests — container independence
// =============================================================================

TEMPLATE_TEST_CASE("articulation_points - path graph (typed)", "[algorithm][articulation_points]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  REQUIRE(same_set(result, {1u, 2u}));
}

TEMPLATE_TEST_CASE("articulation_points - cycle graph (typed)", "[algorithm][articulation_points]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  REQUIRE(result.empty());
}

TEMPLATE_TEST_CASE("articulation_points - star graph (typed)", "[algorithm][articulation_points]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}});
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  REQUIRE(same_set(result, {0u}));
}

TEMPLATE_TEST_CASE("articulation_points - bridge graph (typed)", "[algorithm][articulation_points]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({
        {0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0},
        {3, 4}, {4, 3}, {4, 5}, {5, 4}, {3, 5}, {5, 3},
        {2, 3}, {3, 2}
  });
  std::vector<typename Graph::vertex_id_type> result;

  articulation_points(g, std::back_inserter(result));

  REQUIRE(same_set(result, {2u, 3u}));
}
