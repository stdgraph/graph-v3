/**
 * @file test_generators.cpp
 * @brief Tests for graph generators (include/graph/generators/).
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <graph/generators.hpp>

#include <algorithm>
#include <set>

using namespace graph::generators;

// ---------------------------------------------------------------------------
// Erdős–Rényi
// ---------------------------------------------------------------------------

TEST_CASE("erdos_renyi: basic properties", "[generators][erdos_renyi]") {
  constexpr uint32_t N = 100;
  constexpr double   p = 0.1;
  auto edges = erdos_renyi(N, p);

  SECTION("no self-loops") {
    for (const auto& e : edges) {
      REQUIRE(e.source_id != e.target_id);
    }
  }

  SECTION("all vertex ids in range [0, N)") {
    for (const auto& e : edges) {
      REQUIRE(e.source_id < N);
      REQUIRE(e.target_id < N);
    }
  }

  SECTION("sorted by source_id") {
    REQUIRE(std::is_sorted(edges.begin(), edges.end(),
                           [](const auto& a, const auto& b) { return a.source_id < b.source_id; }));
  }

  SECTION("edge count is roughly N*(N-1)*p") {
    // With N=100, p=0.1, expected ≈ 990 edges. Allow wide tolerance for randomness.
    REQUIRE(edges.size() > 500);
    REQUIRE(edges.size() < 1500);
  }

  SECTION("deterministic with same seed") {
    auto edges2 = erdos_renyi(N, p);
    REQUIRE(edges.size() == edges2.size());
    for (size_t i = 0; i < edges.size(); ++i) {
      REQUIRE(edges[i].source_id == edges2[i].source_id);
      REQUIRE(edges[i].target_id == edges2[i].target_id);
    }
  }

  SECTION("different seeds produce different graphs") {
    auto edges2 = erdos_renyi(N, p, 99);
    // It's astronomically unlikely they'd be identical with a different seed.
    bool any_different = (edges.size() != edges2.size());
    if (!any_different) {
      for (size_t i = 0; i < edges.size() && !any_different; ++i) {
        any_different = (edges[i].source_id != edges2[i].source_id) ||
                        (edges[i].target_id != edges2[i].target_id);
      }
    }
    REQUIRE(any_different);
  }
}

TEST_CASE("erdos_renyi: weight distributions", "[generators][erdos_renyi]") {
  constexpr uint32_t N = 50;
  constexpr double   p = 0.2;

  SECTION("constant_one weights") {
    auto edges = erdos_renyi(N, p, 42, weight_dist::constant_one);
    for (const auto& e : edges) {
      REQUIRE(e.value == 1.0);
    }
  }

  SECTION("uniform weights in [1, 100]") {
    auto edges = erdos_renyi(N, p, 42, weight_dist::uniform);
    for (const auto& e : edges) {
      REQUIRE(e.value >= 1.0);
      REQUIRE(e.value <= 100.0);
    }
  }

  SECTION("exponential weights >= 1") {
    auto edges = erdos_renyi(N, p, 42, weight_dist::exponential);
    for (const auto& e : edges) {
      REQUIRE(e.value >= 1.0);
    }
  }
}

// ---------------------------------------------------------------------------
// Grid 2D
// ---------------------------------------------------------------------------

TEST_CASE("grid_2d: basic properties", "[generators][grid]") {
  constexpr uint32_t rows = 10;
  constexpr uint32_t cols = 10;
  constexpr uint32_t N    = rows * cols;
  auto edges = grid_2d(rows, cols);

  SECTION("all vertex ids in range [0, N)") {
    for (const auto& e : edges) {
      REQUIRE(e.source_id < N);
      REQUIRE(e.target_id < N);
    }
  }

  SECTION("sorted by source_id") {
    REQUIRE(std::is_sorted(edges.begin(), edges.end(),
                           [](const auto& a, const auto& b) { return a.source_id < b.source_id; }));
  }

  SECTION("bidirectional: every (u,v) has matching (v,u)") {
    std::set<std::pair<uint32_t, uint32_t>> edge_set;
    for (const auto& e : edges) {
      edge_set.emplace(e.source_id, e.target_id);
    }
    for (const auto& e : edges) {
      REQUIRE(edge_set.count({e.target_id, e.source_id}) > 0);
    }
  }

  SECTION("no self-loops") {
    for (const auto& e : edges) {
      REQUIRE(e.source_id != e.target_id);
    }
  }

  SECTION("edge count matches 4-connected grid formula") {
    // Interior: 4 edges, border: 2-3 edges
    // Total directed edges = 2 * (rows*(cols-1) + cols*(rows-1))
    size_t expected = 2 * (rows * (cols - 1) + cols * (rows - 1));
    REQUIRE(edges.size() == expected);
  }
}

// ---------------------------------------------------------------------------
// Barabási–Albert
// ---------------------------------------------------------------------------

TEST_CASE("barabasi_albert: basic properties", "[generators][barabasi_albert]") {
  constexpr uint32_t N = 100;
  constexpr uint32_t m = 3;
  auto edges = barabasi_albert(N, m);

  SECTION("all vertex ids in range [0, N)") {
    for (const auto& e : edges) {
      REQUIRE(e.source_id < N);
      REQUIRE(e.target_id < N);
    }
  }

  SECTION("sorted by source_id") {
    REQUIRE(std::is_sorted(edges.begin(), edges.end(),
                           [](const auto& a, const auto& b) { return a.source_id < b.source_id; }));
  }

  SECTION("no self-loops") {
    for (const auto& e : edges) {
      REQUIRE(e.source_id != e.target_id);
    }
  }

  SECTION("bidirectional: every (u,v) has matching (v,u)") {
    std::set<std::pair<uint32_t, uint32_t>> edge_set;
    for (const auto& e : edges) {
      edge_set.emplace(e.source_id, e.target_id);
    }
    for (const auto& e : edges) {
      REQUIRE(edge_set.count({e.target_id, e.source_id}) > 0);
    }
  }

  SECTION("connected — all vertices reachable from vertex 0") {
    // Build adjacency list and do BFS
    std::vector<std::vector<uint32_t>> adj(N);
    for (const auto& e : edges) {
      adj[e.source_id].push_back(e.target_id);
    }
    std::vector<bool> visited(N, false);
    std::vector<uint32_t> queue = {0};
    visited[0] = true;
    for (size_t i = 0; i < queue.size(); ++i) {
      for (auto v : adj[queue[i]]) {
        if (!visited[v]) {
          visited[v] = true;
          queue.push_back(v);
        }
      }
    }
    REQUIRE(queue.size() == N);
  }
}

// ---------------------------------------------------------------------------
// Path graph
// ---------------------------------------------------------------------------

TEST_CASE("path_graph: basic properties", "[generators][path]") {
  constexpr uint32_t N = 50;
  auto edges = path_graph(N);

  SECTION("exactly N-1 edges") {
    REQUIRE(edges.size() == N - 1);
  }

  SECTION("each edge connects consecutive vertices") {
    for (uint32_t i = 0; i < edges.size(); ++i) {
      REQUIRE(edges[i].source_id == i);
      REQUIRE(edges[i].target_id == i + 1);
    }
  }

  SECTION("sorted by source_id") {
    REQUIRE(std::is_sorted(edges.begin(), edges.end(),
                           [](const auto& a, const auto& b) { return a.source_id < b.source_id; }));
  }
}

// ---------------------------------------------------------------------------
// Template parameter: custom VId type
// ---------------------------------------------------------------------------

TEST_CASE("generators work with uint64_t vertex ids", "[generators][template]") {
  auto er_edges   = erdos_renyi<uint64_t>(uint64_t{50}, 0.1);
  auto grid_edges = grid_2d<uint64_t>(uint64_t{5}, uint64_t{5});
  auto ba_edges   = barabasi_albert<uint64_t>(uint64_t{50}, uint64_t{2});
  auto path_edges = path_graph<uint64_t>(uint64_t{20});

  REQUIRE(er_edges.size() > 0);
  REQUIRE(grid_edges.size() > 0);
  REQUIRE(ba_edges.size() > 0);
  REQUIRE(path_edges.size() == 19);
}
