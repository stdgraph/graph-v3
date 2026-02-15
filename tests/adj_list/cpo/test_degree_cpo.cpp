/**
 * @file test_degree_cpo.cpp
 * @brief Comprehensive tests for degree(g, u) and degree(g, uid) CPOs
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <deque>
#include <utility>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test graphs with custom degree member
// =============================================================================

struct GraphWithDegree {
  std::vector<std::vector<int>> adj_list;

  GraphWithDegree(size_t n) : adj_list(n) {}

  void add_edge(size_t from, size_t to) { adj_list[from].push_back(static_cast<int>(to)); }

  // Custom member: return doubled degree for testing
  size_t degree(size_t u) const { return adj_list[u].size() * 2; }
};

// =============================================================================
// Test graphs with ADL degree
// =============================================================================

namespace test_adl {
struct GraphWithADLDegree {
  std::vector<std::vector<int>> adj_list;

  GraphWithADLDegree(size_t n) : adj_list(n) {}

  void add_edge(size_t from, size_t to) { adj_list[from].push_back(static_cast<int>(to)); }
};

// ADL degree function - return tripled degree for testing
inline size_t degree(const GraphWithADLDegree& g, size_t u) { return g.adj_list[u].size() * 3; }

inline size_t degree(GraphWithADLDegree& g, size_t u) { return g.adj_list[u].size() * 3; }
} // namespace test_adl

// =============================================================================
// Tests: Default implementation with degree(g, u) - vertex descriptor
// =============================================================================

TEST_CASE("degree(g, u) works with vector<vector<int>> graph", "[degree][cpo][vector]") {
  std::vector<std::vector<int>> graph = {
        {1, 2, 3}, // vertex 0 has 3 edges
        {2, 3},    // vertex 1 has 2 edges
        {3},       // vertex 2 has 1 edge
        {}         // vertex 3 has 0 edges
  };

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it++;
  auto v3 = *it;

  REQUIRE(degree(graph, v0) == 3);
  REQUIRE(degree(graph, v1) == 2);
  REQUIRE(degree(graph, v2) == 1);
  REQUIRE(degree(graph, v3) == 0);
}

TEST_CASE("degree(g, u) works with deque<deque<int>> graph", "[degree][cpo][deque]") {
  std::deque<std::deque<int>> graph = {
        {1, 2, 3, 4}, // vertex 0 has 4 edges
        {2, 3},       // vertex 1 has 2 edges
        {},           // vertex 2 has 0 edges
  };

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it;

  REQUIRE(degree(graph, v0) == 4);
  REQUIRE(degree(graph, v1) == 2);
  REQUIRE(degree(graph, v2) == 0);
}

TEST_CASE("degree(g, u) works with weighted edges (pair)", "[degree][cpo][weighted][pair]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}, {3, 3.5}}, // vertex 0 has 3 edges
        {{2, 1.2}, {3, 2.3}},           // vertex 1 has 2 edges
        {{3, 1.0}},                     // vertex 2 has 1 edge
        {}                              // vertex 3 has 0 edges
  };

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it++;
  auto v3 = *it;

  REQUIRE(degree(graph, v0) == 3);
  REQUIRE(degree(graph, v1) == 2);
  REQUIRE(degree(graph, v2) == 1);
  REQUIRE(degree(graph, v3) == 0);
}

TEST_CASE("degree(g, u) works with weighted edges (tuple)", "[degree][cpo][weighted][tuple]") {
  using Edge                           = std::tuple<int, double, std::string>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.5, "road"}, {2, 2.5, "rail"}, {3, 3.5, "air"}},
                                          {{2, 1.2, "road"}, {3, 2.3, "rail"}},
                                          {{3, 1.0, "road"}},
                                          {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it++;
  auto v3 = *it;

  REQUIRE(degree(graph, v0) == 3);
  REQUIRE(degree(graph, v1) == 2);
  REQUIRE(degree(graph, v2) == 1);
  REQUIRE(degree(graph, v3) == 0);
}

TEST_CASE("degree(g, u) returns 0 for vertices with no edges", "[degree][cpo][empty]") {
  std::vector<std::vector<int>> graph = {
        {}, // vertex 0 has no edges
        {}, // vertex 1 has no edges
        {}  // vertex 2 has no edges
  };

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it;

  REQUIRE(degree(graph, v0) == 0);
  REQUIRE(degree(graph, v1) == 0);
  REQUIRE(degree(graph, v2) == 0);
}

// =============================================================================
// Tests: Custom member implementation
// =============================================================================

TEST_CASE("degree(g, u) uses custom member function when available", "[degree][cpo][custom][member]") {
  GraphWithDegree graph(3);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);

  // Custom member returns double the actual count
  REQUIRE(degree(graph, 0) == 4); // 2 * 2
  REQUIRE(degree(graph, 1) == 2); // 1 * 2
  REQUIRE(degree(graph, 2) == 0); // 0 * 2
}

// =============================================================================
// Tests: ADL implementation
// =============================================================================

TEST_CASE("degree(g, u) uses ADL when available", "[degree][cpo][adl]") {
  test_adl::GraphWithADLDegree graph(3);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);

  // ADL function returns triple the actual count
  REQUIRE(degree(graph, 0) == 6); // 2 * 3
  REQUIRE(degree(graph, 1) == 3); // 1 * 3
  REQUIRE(degree(graph, 2) == 0); // 0 * 3
}

// =============================================================================
// Tests: Const correctness
// =============================================================================

TEST_CASE("degree(g, u) works with const graph", "[degree][cpo][const]") {
  const std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it;

  REQUIRE(degree(graph, v0) == 3);
  REQUIRE(degree(graph, v1) == 2);
}

// =============================================================================
// Tests: Return type verification
// =============================================================================

TEST_CASE("degree(g, u) returns integral type", "[degree][cpo][return_type]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {2}};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  auto deg = degree(graph, v0);
  REQUIRE(std::is_integral_v<decltype(deg)>);
  REQUIRE(deg == 2);
}

// =============================================================================
// Tests: Graph topologies
// =============================================================================

TEST_CASE("degree works with complete graph K4", "[degree][cpo][topology][complete]") {
  std::vector<std::vector<int>> graph(4);
  // Complete graph: each vertex connected to all others
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (i != j) {
        graph[static_cast<size_t>(i)].push_back(j);
      }
    }
  }

  // Every vertex has degree 3 in K4
  auto verts = vertices(graph);
  for (auto v : verts) {
    REQUIRE(degree(graph, v) == 3);
  }
}

TEST_CASE("degree works with star graph", "[degree][cpo][topology][star]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3, 4}, // Center (0) connected to all others
                                         {},           // Leaves have degree 0 (out-degree)
                                         {},
                                         {},
                                         {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it;

  REQUIRE(degree(graph, v0) == 4); // Center has degree 4
  REQUIRE(degree(graph, v1) == 0); // Leaves have degree 0 (out-degree)
  REQUIRE(degree(graph, v2) == 0);
}

TEST_CASE("degree works with path graph", "[degree][cpo][topology][path]") {
  std::vector<std::vector<int>> graph = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {3}, // 2 -> 3
        {4}, // 3 -> 4
        {}   // 4 (end)
  };

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it++;
  auto v3 = *it++;
  auto v4 = *it;

  REQUIRE(degree(graph, v0) == 1);
  REQUIRE(degree(graph, v1) == 1);
  REQUIRE(degree(graph, v2) == 1);
  REQUIRE(degree(graph, v3) == 1);
  REQUIRE(degree(graph, v4) == 0); // Last vertex has out-degree 0
}

TEST_CASE("degree works with DAG", "[degree][cpo][topology][dag]") {
  std::vector<std::vector<int>> graph = {
        {1, 2}, // 0 -> 1, 2
        {3},    // 1 -> 3
        {3},    // 2 -> 3
        {4, 5}, // 3 -> 4, 5
        {},     // 4 (sink)
        {}      // 5 (sink)
  };

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it++;
  auto v3 = *it++;
  auto v4 = *it++;
  auto v5 = *it;

  REQUIRE(degree(graph, v0) == 2);
  REQUIRE(degree(graph, v1) == 1);
  REQUIRE(degree(graph, v2) == 1);
  REQUIRE(degree(graph, v3) == 2);
  REQUIRE(degree(graph, v4) == 0);
  REQUIRE(degree(graph, v5) == 0);
}

// =============================================================================
// Tests: Self-loops
// =============================================================================

TEST_CASE("degree counts self-loops", "[degree][cpo][self_loop]") {
  std::vector<std::vector<int>> graph = {{0, 1}, // Self-loop + edge to 1
                                         {1},    // Self-loop
                                         {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();

  auto v0 = *it++;
  auto v1 = *it++;
  auto v2 = *it;

  REQUIRE(degree(graph, v0) == 2); // 1 self-loop + 1 regular edge
  REQUIRE(degree(graph, v1) == 1); // 1 self-loop
  REQUIRE(degree(graph, v2) == 0);
}

// =============================================================================
// Tests: Single vertex
// =============================================================================

TEST_CASE("degree works with single vertex graph", "[degree][cpo][single_vertex]") {
  std::vector<std::vector<int>> graph = {{}};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  REQUIRE(degree(graph, v0) == 0);
}

TEST_CASE("degree works with single vertex with self-loop", "[degree][cpo][single_vertex][self_loop]") {
  std::vector<std::vector<int>> graph = {{0}};

  auto verts = vertices(graph);
  auto v0    = *verts.begin();

  REQUIRE(degree(graph, v0) == 1);
}

// =============================================================================
// Tests: Integration with edges CPO
// =============================================================================

TEST_CASE("degree(g, u) equals std::ranges::size(edges(g, u))", "[degree][cpo][integration]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3, 4}, {}, {4}};

  auto verts = vertices(graph);
  for (auto v : verts) {
    REQUIRE(degree(graph, v) == std::ranges::size(edges(graph, v)));
  }
}
