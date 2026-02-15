/**
 * @file test_mis.cpp
 * @brief Tests for maximal independent set algorithm from mis.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/algorithm/mis.hpp>
#include "../common/algorithm_test_types.hpp"
#include <vector>
#include <set>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test::algorithm;

// Helper to check if a set is independent (no two vertices are adjacent)
template <typename G>
bool is_independent_set(const G& g, const std::vector<typename G::vertex_id_type>& mis_vec) {
  std::set<typename G::vertex_id_type> mis_set(mis_vec.begin(), mis_vec.end());

  for (auto uid : mis_vec) {
    auto u = *find_vertex(g, uid);
    for (auto uv : edges(g, u)) {
      if (mis_set.count(target_id(g, uv))) {
        return false; // Two adjacent vertices in the set
      }
    }
  }
  return true;
}

// Helper to check if a set is maximal (no vertex can be added)
template <typename G>
bool is_maximal(const G& g, const std::vector<typename G::vertex_id_type>& mis_vec) {
  std::set<typename G::vertex_id_type> mis_set(mis_vec.begin(), mis_vec.end());

  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    if (mis_set.count(uid))
      continue; // Already in set

    // Check if uid is adjacent to any vertex in the MIS
    bool adjacent_to_mis = false;
    for (auto uv : edges(g, u)) {
      if (mis_set.count(target_id(g, uv))) {
        adjacent_to_mis = true;
        break;
      }
    }

    if (!adjacent_to_mis) {
      return false; // Could add uid to the set
    }
  }
  return true;
}

// =============================================================================
// Basic Test Cases
// =============================================================================

TEST_CASE("mis - empty graph", "[algorithm][mis]") {
  using Graph = vov_void;

  Graph                                       g;
  std::vector<typename Graph::vertex_id_type> mis_result;

  // Empty graph should produce empty MIS (seed=0 would be invalid)
  REQUIRE(num_vertices(g) == 0);
  // Cannot call maximal_independent_set on empty graph
}

TEST_CASE("mis - single vertex", "[algorithm][mis]") {
  using Graph = vov_void;

  // Graph with single vertex (no edges)
  Graph g({{0, 0}});    // Self-loop to force vertex 0 to exist
  g = Graph{};          // Reset to empty edge list but vertex 0 exists
  g.resize_vertices(1); // Ensure we have 1 vertex with no edges

  std::vector<typename Graph::vertex_id_type> mis_result;
  maximal_independent_set(g, std::back_inserter(mis_result), 0);

  REQUIRE(mis_result.size() == 1);
  REQUIRE(mis_result[0] == 0);
  REQUIRE(is_independent_set(g, mis_result));
  REQUIRE(is_maximal(g, mis_result));
}

TEST_CASE("mis - single edge", "[algorithm][mis]") {
  using Graph = vov_void;

  // Undirected edge requires both {0,1} and {1,0}
  Graph g({{0, 1}, {1, 0}});

  SECTION("seed = 0") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    REQUIRE(mis_result.size() == 1);
    REQUIRE(mis_result[0] == 0);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));
  }

  SECTION("seed = 1") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 1);

    REQUIRE(mis_result.size() == 1);
    REQUIRE(mis_result[0] == 1);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));
  }
}

TEST_CASE("mis - triangle", "[algorithm][mis]") {
  using Graph = vov_void;

  // Triangle: all vertices connected to each other
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 0}, {0, 2}});

  SECTION("seed = 0") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    REQUIRE(mis_result.size() == 1);
    REQUIRE(mis_result[0] == 0);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));
  }

  SECTION("seed = 1") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 1);

    REQUIRE(mis_result.size() == 1);
    REQUIRE(mis_result[0] == 1);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));
  }
}

TEST_CASE("mis - path graph", "[algorithm][mis]") {
  using Graph = vov_void;

  // Path: 0 - 1 - 2 - 3 - 4
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}});

  SECTION("seed = 0") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    // Should include 0, 2, 4 (alternating vertices)
    REQUIRE(mis_result.size() == 3);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(0) == 1); // seed must be included
  }

  SECTION("seed = 2") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 2);

    // Should include 2, 0, 4
    REQUIRE(mis_result.size() == 3);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(2) == 1); // seed must be included
  }
}

TEST_CASE("mis - cycle graph", "[algorithm][mis]") {
  using Graph = vov_void;

  // Cycle: 0 - 1 - 2 - 3 - 4 - 0
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});

  std::vector<typename Graph::vertex_id_type> mis_result;
  maximal_independent_set(g, std::back_inserter(mis_result), 0);

  // Should be able to select 2 vertices from a 5-cycle
  REQUIRE(mis_result.size() == 2);
  REQUIRE(is_independent_set(g, mis_result));
  REQUIRE(is_maximal(g, mis_result));

  std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
  REQUIRE(mis_set.count(0) == 1); // seed must be included
}

TEST_CASE("mis - star graph", "[algorithm][mis]") {
  using Graph = vov_void;

  // Star: center 0 connected to 1, 2, 3, 4
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}});

  SECTION("seed = 0 (center)") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    // Only the center should be in the MIS
    REQUIRE(mis_result.size() == 1);
    REQUIRE(mis_result[0] == 0);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));
  }

  SECTION("seed = 1 (leaf)") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 1);

    // Should include all leaves except the center: 1, 2, 3, 4
    REQUIRE(mis_result.size() == 4);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(1) == 1); // seed must be included
    REQUIRE(mis_set.count(0) == 0); // center should not be included
  }
}

TEST_CASE("mis - complete graph", "[algorithm][mis]") {
  using Graph = vov_void;

  // Complete graph K4: all vertices connected to each other
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});

  std::vector<typename Graph::vertex_id_type> mis_result;
  maximal_independent_set(g, std::back_inserter(mis_result), 0);

  // Complete graph: only one vertex can be in MIS
  REQUIRE(mis_result.size() == 1);
  REQUIRE(mis_result[0] == 0);
  REQUIRE(is_independent_set(g, mis_result));
  REQUIRE(is_maximal(g, mis_result));
}

// =============================================================================
// Disconnected Graphs
// =============================================================================

TEST_CASE("mis - disconnected graph", "[algorithm][mis][disconnected]") {
  using Graph = vov_void;

  // Two components: {0, 1} and {2, 3, 4}
  // Component 1: edge 0-1
  // Component 2: triangle 2-3-4
  Graph g({{0, 1}, {1, 0}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 2}, {2, 4}});

  SECTION("seed in first component") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    // Should include 0 from first component and one vertex from second
    REQUIRE(mis_result.size() == 2);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(0) == 1); // seed must be included
  }

  SECTION("seed in second component") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 2);

    // Should include 2 from second component and one vertex from first
    REQUIRE(mis_result.size() == 2);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(2) == 1); // seed must be included
  }
}

TEST_CASE("mis - multiple isolated vertices", "[algorithm][mis]") {
  using Graph = vov_void;

  // All vertices are independent (no edges)
  Graph g;
  g.resize_vertices(5);

  std::vector<typename Graph::vertex_id_type> mis_result;
  maximal_independent_set(g, std::back_inserter(mis_result), 0);

  // All vertices should be in the MIS
  REQUIRE(mis_result.size() == 5);
  REQUIRE(is_independent_set(g, mis_result));
  REQUIRE(is_maximal(g, mis_result));

  std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
  REQUIRE(mis_set.count(0) == 1); // seed must be included
  for (int i = 0; i < 5; ++i) {
    REQUIRE(mis_set.count(i) == 1);
  }
}

// =============================================================================
// Special Graph Structures
// =============================================================================

TEST_CASE("mis - bipartite graph", "[algorithm][mis][bipartite]") {
  using Graph = vov_void;

  // Complete bipartite graph K(2,3): partition {0,1} and {2,3,4}
  // Edges between partitions only
  Graph g({{0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {1, 4}, {4, 1}});

  SECTION("seed in first partition") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    // Should select all vertices from first partition: {0, 1}
    REQUIRE(mis_result.size() == 2);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(0) == 1);
    REQUIRE(mis_set.count(1) == 1);
  }

  SECTION("seed in second partition") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 2);

    // Should select all vertices from second partition: {2, 3, 4}
    REQUIRE(mis_result.size() == 3);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(2) == 1);
    REQUIRE(mis_set.count(3) == 1);
    REQUIRE(mis_set.count(4) == 1);
  }
}

TEST_CASE("mis - tree structure", "[algorithm][mis][tree]") {
  using Graph = vov_void;

  // Binary tree: root 0, children 1 and 2, grandchildren 3,4,5,6
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 3}, {3, 1}, {1, 4}, {4, 1}, {2, 5}, {5, 2}, {2, 6}, {6, 2}});

  SECTION("seed = 0 (root)") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    // Should include root and all grandchildren: {0, 3, 4, 5, 6}
    REQUIRE(mis_result.size() == 5);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(0) == 1); // root
    REQUIRE(mis_set.count(1) == 0); // child excluded
    REQUIRE(mis_set.count(2) == 0); // child excluded
  }

  SECTION("seed = 1 (child)") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 1);

    // Should include the child and non-adjacent vertices
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(1) == 1); // seed must be included
    REQUIRE(mis_set.count(0) == 0); // parent excluded
    REQUIRE(mis_set.count(3) == 0); // child of seed excluded
    REQUIRE(mis_set.count(4) == 0); // child of seed excluded
  }
}

TEST_CASE("mis - diamond graph", "[algorithm][mis]") {
  using Graph = vov_void;

  // Diamond: 0 -> {1, 2} -> 3
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});

  SECTION("seed = 0") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 0);

    // Should include 0 and 3 (opposite corners)
    REQUIRE(mis_result.size() == 2);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(0) == 1);
    REQUIRE(mis_set.count(3) == 1);
  }

  SECTION("seed = 1") {
    std::vector<typename Graph::vertex_id_type> mis_result;
    maximal_independent_set(g, std::back_inserter(mis_result), 1);

    // Should include 1 and 2 (middle vertices)
    REQUIRE(mis_result.size() == 2);
    REQUIRE(is_independent_set(g, mis_result));
    REQUIRE(is_maximal(g, mis_result));

    std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
    REQUIRE(mis_set.count(1) == 1);
    REQUIRE(mis_set.count(2) == 1);
  }
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_CASE("mis - self loop", "[algorithm][mis][edge_case]") {
  using Graph = vov_void;

  // Vertex 0 with self-loop and edge to vertex 1, vertex 2 isolated
  Graph g({{0, 0}, {0, 1}, {1, 0}});
  g.resize_vertices(3);

  std::vector<typename Graph::vertex_id_type> mis_result;
  // Start from vertex 1 (skip 0 since it has a self-loop)
  maximal_independent_set(g, std::back_inserter(mis_result), 1);

  // Should include vertex 1 and vertex 2 (which is isolated)
  REQUIRE(mis_result.size() == 2);
  REQUIRE(is_independent_set(g, mis_result));
  REQUIRE(is_maximal(g, mis_result));

  std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
  REQUIRE(mis_set.count(1) == 1); // seed vertex 1
  REQUIRE(mis_set.count(2) == 1); // isolated vertex 2
  REQUIRE(mis_set.count(0) == 0); // vertex 0 with self-loop excluded
}

TEST_CASE("mis - large path", "[algorithm][mis][large]") {
  using Graph = vov_void;

  // Long path: 0 - 1 - 2 - ... - 9
  Graph g({{0, 1},
           {1, 0},
           {1, 2},
           {2, 1},
           {2, 3},
           {3, 2},
           {3, 4},
           {4, 3},
           {4, 5},
           {5, 4},
           {5, 6},
           {6, 5},
           {6, 7},
           {7, 6},
           {7, 8},
           {8, 7},
           {8, 9},
           {9, 8}});

  std::vector<typename Graph::vertex_id_type> mis_result;
  maximal_independent_set(g, std::back_inserter(mis_result), 0);

  // Should select about half the vertices (alternating)
  REQUIRE(mis_result.size() == 5);
  REQUIRE(is_independent_set(g, mis_result));
  REQUIRE(is_maximal(g, mis_result));

  std::set<typename Graph::vertex_id_type> mis_set(mis_result.begin(), mis_result.end());
  REQUIRE(mis_set.count(0) == 1); // seed must be included
}
