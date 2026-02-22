/**
 * @file test_triangle_count.cpp
 * @brief Tests for triangle counting algorithm
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/tc.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <graph/container/traits/vos_graph_traits.hpp>
#include <graph/container/traits/uos_graph_traits.hpp>
#include <graph/container/traits/dos_graph_traits.hpp>

using namespace graph;
using namespace graph::container;

// Graph type with sorted edges (required for triangle_count)
using vos_void = dynamic_graph<void, void, void, uint32_t, false, false, vos_graph_traits<void, void, void, uint32_t, false>>;

// Undirected adjacency list (automatically handles bidirectional edges)
using ual_int = undirected_adjacency_list<int, int>;

//=============================================================================
// Basic Triangle Tests
//=============================================================================

TEST_CASE("triangle_count - empty graph", "[algorithm][triangle_count]") {
  vos_void g;
  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - single vertex", "[algorithm][triangle_count]") {
  vos_void g;
  g.resize_vertices(1);
  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - two vertices no edge", "[algorithm][triangle_count]") {
  vos_void g;
  g.resize_vertices(2);
  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - two vertices with edge", "[algorithm][triangle_count]") {
  // Each undirected edge needs both directions
  vos_void g({{0, 1}, {1, 0}});
  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - single triangle", "[algorithm][triangle_count]") {
  // Triangle: 0-1-2-0 (each undirected edge stored as two directed edges)
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
  REQUIRE(triangle_count(g) == 1);
}

//=============================================================================
// Multiple Triangles
//=============================================================================

TEST_CASE("triangle_count - complete graph K4", "[algorithm][triangle_count]") {
  // Complete graph on 4 vertices: all pairs connected bidirectionally
  // Has C(4,3) = 4 triangles
  vos_void g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});

  REQUIRE(triangle_count(g) == 4);
}

TEST_CASE("triangle_count - two separate triangles", "[algorithm][triangle_count]") {
  // Triangle 1: 0-1-2, Triangle 2: 3-4-5
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, {3, 4}, {4, 3}, {4, 5}, {5, 4}, {3, 5}, {5, 3}});

  REQUIRE(triangle_count(g) == 2);
}

TEST_CASE("triangle_count - two triangles sharing edge", "[algorithm][triangle_count]") {
  // Triangle 1: 0-1-2, Triangle 2: 0-1-3 (share edge 0-1)
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, {1, 3}, {3, 1}, {0, 3}, {3, 0}});

  REQUIRE(triangle_count(g) == 2);
}

TEST_CASE("triangle_count - two triangles sharing vertex", "[algorithm][triangle_count]") {
  // Triangle 1: 0-1-2, Triangle 2: 0-3-4 (share vertex 0)
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {3, 4}, {4, 3}, {0, 4}, {4, 0}});

  REQUIRE(triangle_count(g) == 2);
}

//=============================================================================
// Graphs with No Triangles
//=============================================================================

TEST_CASE("triangle_count - path graph (no triangles)", "[algorithm][triangle_count]") {
  // Path: 0-1-2-3-4
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}});

  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - cycle graph (no triangles)", "[algorithm][triangle_count]") {
  // Cycle: 0-1-2-3-4-0
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});

  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - star graph (no triangles)", "[algorithm][triangle_count]") {
  // Star: center vertex 0 connected to all others (no triangles)
  vos_void g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}, {0, 5}, {5, 0}});

  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - bipartite graph (no triangles)", "[algorithm][triangle_count]") {
  // Complete bipartite K(3,3): {0,1,2} to {3,4,5} (no triangles)
  vos_void g({{0, 3},
              {3, 0},
              {0, 4},
              {4, 0},
              {0, 5},
              {5, 0},
              {1, 3},
              {3, 1},
              {1, 4},
              {4, 1},
              {1, 5},
              {5, 1},
              {2, 3},
              {3, 2},
              {2, 4},
              {4, 2},
              {2, 5},
              {5, 2}});

  REQUIRE(triangle_count(g) == 0);
}

//=============================================================================
// Complex Structures
//=============================================================================

TEST_CASE("triangle_count - diamond graph", "[algorithm][triangle_count]") {
  // Diamond: 0 at top, 1,2 in middle (connected), 3 at bottom
  vos_void g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});

  REQUIRE(triangle_count(g) == 2); // {0,1,2} and {1,2,3}
}

TEST_CASE("triangle_count - wheel graph", "[algorithm][triangle_count]") {
  // Wheel: center vertex 0, rim vertices 1-5 forming cycle
  vos_void g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {0, 4}, {4, 0}, {0, 5}, {5, 0},
              {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 5}, {5, 4}, {5, 1}, {1, 5}});

  REQUIRE(triangle_count(g) == 5); // One triangle for each rim edge with center
}

TEST_CASE("triangle_count - house graph", "[algorithm][triangle_count]") {
  // House: square base (0-1-2-3) + triangular roof (0-4-1)
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 0}, {0, 3}, {0, 4}, {4, 0}, {4, 1}, {1, 4}});

  REQUIRE(triangle_count(g) == 1); // Only {0,1,4}
}

//=============================================================================
// Graph Type Variations
//=============================================================================

TEST_CASE("triangle_count - single triangle vos", "[algorithm][triangle_count]") {
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});

  REQUIRE(triangle_count(g) == 1);
}

TEST_CASE("triangle_count - K4 vos", "[algorithm][triangle_count]") {
  vos_void g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {0, 3}, {3, 0}, {1, 2}, {2, 1}, {1, 3}, {3, 1}, {2, 3}, {3, 2}});

  REQUIRE(triangle_count(g) == 4);
}

//=============================================================================
// Edge Cases
//=============================================================================

TEST_CASE("triangle_count - graph with isolated vertices", "[algorithm][triangle_count]") {
  // Triangle 0-1-2 plus isolated vertices 3,4,5,6
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}});
  g.resize_vertices(7); // Add isolated vertices

  REQUIRE(triangle_count(g) == 1);
}

TEST_CASE("triangle_count - graph with self-loops ignored", "[algorithm][triangle_count]") {
  // Triangle with self-loops (self-loops don't contribute to triangles)
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {0, 2}, {2, 0}, {0, 0}, {1, 1}}); // Self-loops

  REQUIRE(triangle_count(g) == 1);
}

TEST_CASE("triangle_count - chordal cycle", "[algorithm][triangle_count]") {
  // 5-cycle with chord 0-2: creates one triangle {0,1,2}
  vos_void g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}, {0, 2}, {2, 0}});

  REQUIRE(triangle_count(g) == 1); // Only {0,1,2}
}

//=============================================================================
// undirected_adjacency_list Tests (no manual bidirectional edges needed)
//=============================================================================

TEST_CASE("triangle_count - UAL single triangle", "[algorithm][triangle_count][ual]") {
  // Triangle: 0-1-2-0 (only specify once, automatic bidirectional)
  // Using initializer list constructor like dynamic_graph
  ual_int g({{0, 1, 0}, {1, 2, 0}, {0, 2, 0}});

  REQUIRE(triangle_count(g) == 1);
}

TEST_CASE("triangle_count - UAL complete graph K4", "[algorithm][triangle_count][ual]") {
  // Complete graph on 4 vertices has C(4,3) = 4 triangles
  // Only need to specify each edge once (automatic bidirectional)
  ual_int g({{0, 1, 0}, {0, 2, 0}, {0, 3, 0}, {1, 2, 0}, {1, 3, 0}, {2, 3, 0}});

  REQUIRE(triangle_count(g) == 4);
}

TEST_CASE("triangle_count - UAL two separate triangles", "[algorithm][triangle_count][ual]") {
  // Triangle 1: 0-1-2, Triangle 2: 3-4-5
  ual_int g({{0, 1, 0}, {1, 2, 0}, {0, 2, 0}, {3, 4, 0}, {4, 5, 0}, {3, 5, 0}});

  REQUIRE(triangle_count(g) == 2);
}

TEST_CASE("triangle_count - UAL path graph (no triangles)", "[algorithm][triangle_count][ual]") {
  // Path: 0-1-2-3-4
  ual_int g({{0, 1, 0}, {1, 2, 0}, {2, 3, 0}, {3, 4, 0}});

  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - UAL star graph (no triangles)", "[algorithm][triangle_count][ual]") {
  // Star: center vertex 0 connected to all others
  ual_int g({{0, 1, 0}, {0, 2, 0}, {0, 3, 0}, {0, 4, 0}, {0, 5, 0}});

  REQUIRE(triangle_count(g) == 0);
}

TEST_CASE("triangle_count - UAL diamond graph", "[algorithm][triangle_count][ual]") {
  // Diamond: 0 at top, 1,2 in middle (connected), 3 at bottom
  ual_int g({{0, 1, 0}, {0, 2, 0}, {1, 2, 0}, {1, 3, 0}, {2, 3, 0}});

  REQUIRE(triangle_count(g) == 2); // {0,1,2} and {1,2,3}
}

TEST_CASE("triangle_count - UAL wheel graph", "[algorithm][triangle_count][ual]") {
  // Wheel: center vertex 0, rim vertices 1-5 forming cycle
  ual_int g(
        {{0, 1, 0}, {0, 2, 0}, {0, 3, 0}, {0, 4, 0}, {0, 5, 0}, {1, 2, 0}, {2, 3, 0}, {3, 4, 0}, {4, 5, 0}, {5, 1, 0}});

  REQUIRE(triangle_count(g) == 5); // One triangle for each rim edge with center
}

TEST_CASE("triangle_count - UAL graph with isolated vertices", "[algorithm][triangle_count][ual]") {
  // Triangle: 0-1-2, vertices 3-6 isolated
  ual_int g({{0, 1, 0}, {1, 2, 0}, {0, 2, 0}});
  // Note: Isolated vertices 3-6 don't need to be created explicitly
  // The initializer list constructor only creates vertices up to max edge vertex id

  REQUIRE(triangle_count(g) == 1);
}

TEST_CASE("triangle_count - UAL bipartite graph (no triangles)", "[algorithm][triangle_count][ual]") {
  // Complete bipartite K(3,3): {0,1,2} to {3,4,5}
  ual_int g({{0, 3, 0}, {0, 4, 0}, {0, 5, 0}, {1, 3, 0}, {1, 4, 0}, {1, 5, 0}, {2, 3, 0}, {2, 4, 0}, {2, 5, 0}});

  REQUIRE(triangle_count(g) == 0);
}
