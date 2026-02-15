#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <list>
#include <set>
#include <string>

#include "graph/adj_list/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Default Implementation with Vector - Simple Edges
// =============================================================================

TEST_CASE("num_edges(g) - vector<vector<int>> simple edges", "[num_edges][cpo][default]") {
  std::vector<std::vector<int>> graph = {
        {1, 2, 3}, // vertex 0 has 3 edges
        {2, 3},    // vertex 1 has 2 edges
        {3},       // vertex 2 has 1 edge
        {}         // vertex 3 has 0 edges
  };

  auto total = num_edges(graph);
  REQUIRE(total == 6);
}

TEST_CASE("num_edges(g) - empty graph", "[num_edges][cpo][default]") {
  std::vector<std::vector<int>> graph;

  auto total = num_edges(graph);
  REQUIRE(total == 0);
}

TEST_CASE("num_edges(g) - single vertex with edges", "[num_edges][cpo][default]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3, 4}};

  auto total = num_edges(graph);
  REQUIRE(total == 4);
}

TEST_CASE("num_edges(g) - single vertex without edges", "[num_edges][cpo][default]") {
  std::vector<std::vector<int>> graph = {{}};

  auto total = num_edges(graph);
  REQUIRE(total == 0);
}

TEST_CASE("num_edges(g) - multiple vertices without edges", "[num_edges][cpo][default]") {
  std::vector<std::vector<int>> graph = {{}, {}, {}};

  auto total = num_edges(graph);
  REQUIRE(total == 0);
}

// =============================================================================
// Test: Default Implementation with Pair Edges (Weighted)
// =============================================================================

TEST_CASE("num_edges(g) - vector<vector<pair<int,double>>> weighted edges", "[num_edges][cpo][default]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}, {3, 3.5}}, // vertex 0: 3 edges
        {{2, 1.0}, {3, 2.0}},           // vertex 1: 2 edges
        {{3, 1.5}},                     // vertex 2: 1 edge
        {}                              // vertex 3: 0 edges
  };

  auto total = num_edges(graph);
  REQUIRE(total == 6);
}

TEST_CASE("num_edges(g) - weighted graph with self-loops", "[num_edges][cpo][default]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {
        {{0, 1.0}, {1, 2.0}}, // vertex 0: self-loop + edge to 1
        {{1, 3.0}, {2, 4.0}}, // vertex 1: self-loop + edge to 2
        {{0, 5.0}}            // vertex 2: edge to 0
  };

  auto total = num_edges(graph);
  REQUIRE(total == 5);
}

// =============================================================================
// Test: Default Implementation with Tuple Edges
// =============================================================================

TEST_CASE("num_edges(g) - vector<vector<tuple<...>>> multi-property edges", "[num_edges][cpo][default]") {
  using Edge                           = std::tuple<int, double, std::string>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.5, "a"}, {2, 2.5, "b"}}, {{3, 3.5, "c"}}, {}};

  auto total = num_edges(graph);
  REQUIRE(total == 3);
}

// =============================================================================
// Test: Default Implementation with Deque
// =============================================================================

TEST_CASE("num_edges(g) - deque<deque<int>> simple edges", "[num_edges][cpo][default]") {
  std::deque<std::deque<int>> graph = {{10, 20}, {30}, {}, {40, 50, 60}};

  auto total = num_edges(graph);
  REQUIRE(total == 6);
}

// =============================================================================
// Test: Default Implementation with Map
// =============================================================================

TEST_CASE("num_edges(g) - map<int, vector<int>>", "[num_edges][cpo][default]") {
  std::map<int, std::vector<int>> graph = {{0, {1, 2}}, {1, {2}}, {2, {}}, {3, {0, 1, 2, 3}}};

  auto total = num_edges(graph);
  REQUIRE(total == 7);
}

TEST_CASE("num_edges(g) - empty map", "[num_edges][cpo][default]") {
  std::map<int, std::vector<int>> graph;

  auto total = num_edges(graph);
  REQUIRE(total == 0);
}

TEST_CASE("num_edges(g) - map with sparse vertex IDs", "[num_edges][cpo][default]") {
  std::map<int, std::vector<int>> graph = {{10, {20, 30}}, {20, {30}}, {100, {200, 300, 400}}};

  auto total = num_edges(graph);
  REQUIRE(total == 6);
}

TEST_CASE("num_edges(g) - map with weighted edges", "[num_edges][cpo][default]") {
  using Edge                             = std::pair<int, double>;
  std::map<int, std::vector<Edge>> graph = {{0, {{1, 1.5}, {2, 2.5}}}, {1, {{2, 3.5}}}, {2, {}}};

  auto total = num_edges(graph);
  REQUIRE(total == 3);
}

// =============================================================================
// Test: Default Implementation with List (Forward Iterator)
// =============================================================================

TEST_CASE("num_edges(g) - vector<list<int>> forward iterator edges", "[num_edges][cpo][default]") {
  std::vector<std::list<int>> graph = {{1, 2, 3}, {2}, {}, {0, 1}};

  auto total = num_edges(graph);
  REQUIRE(total == 6);
}

TEST_CASE("num_edges(g) - vector<set<int>> forward iterator edges", "[num_edges][cpo][default]") {
  std::vector<std::set<int>> graph = {{1, 2, 3}, {2, 3, 4}, {3}, {}};

  auto total = num_edges(graph);
  REQUIRE(total == 7);
}

// =============================================================================
// Test: Custom Member Function
// =============================================================================

namespace test_member {
struct CustomGraph {
  std::vector<std::vector<int>> adjacency_list;

  auto vertices() const { return vertex_descriptor_view(adjacency_list); }

  auto edges(const auto& u) const { return edge_descriptor_view(u.inner_value(adjacency_list), u); }

  // Custom num_edges member
  std::size_t num_edges() const {
    return 42; // Custom implementation returns a fixed value
  }
};
} // namespace test_member

TEST_CASE("num_edges(g) - custom member function", "[num_edges][cpo][member]") {
  test_member::CustomGraph graph{{{1, 2}, {3}, {}}};

  // Should use the custom member function, not default
  auto total = num_edges(graph);
  REQUIRE(total == 42);
}

// =============================================================================
// Test: ADL Customization
// =============================================================================

namespace test_adl {
struct CustomGraph {
  std::vector<std::vector<int>> adjacency_list;

  auto vertices() const { return vertex_descriptor_view(adjacency_list); }

  auto edges(const auto& u) const { return edge_descriptor_view(u.inner_value(adjacency_list), u); }
};

// ADL num_edges function
std::size_t num_edges([[maybe_unused]] const CustomGraph& g) {
  return 99; // Custom implementation
}
} // namespace test_adl

TEST_CASE("num_edges(g) - ADL customization", "[num_edges][cpo][adl]") {
  test_adl::CustomGraph graph{{{1, 2, 3}, {4}, {}}};

  // Should find ADL num_edges
  auto total = num_edges(graph);
  REQUIRE(total == 99);
}

// =============================================================================
// Test: Large Graph Performance
// =============================================================================

TEST_CASE("num_edges(g) - large graph", "[num_edges][cpo][performance]") {
  std::vector<std::vector<int>> graph(1000);

  // Each vertex has edges to next 5 vertices (wrapping)
  for (std::size_t i = 0; i < graph.size(); ++i) {
    for (std::size_t j = 1; j <= 5; ++j) {
      graph[i].push_back(static_cast<int>((i + j) % graph.size()));
    }
  }

  auto total = num_edges(graph);
  REQUIRE(total == 5000); // 1000 vertices * 5 edges each
}

// =============================================================================
// Test: Different Graph Patterns
// =============================================================================

TEST_CASE("num_edges(g) - complete graph K4", "[num_edges][cpo][patterns]") {
  // Complete graph with 4 vertices (each connects to all others)
  std::vector<std::vector<int>> graph = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};

  auto total = num_edges(graph);
  REQUIRE(total == 12); // 4 vertices * 3 edges each (directed)
}

TEST_CASE("num_edges(g) - linear chain", "[num_edges][cpo][patterns]") {
  // Linear chain: 0 -> 1 -> 2 -> 3
  std::vector<std::vector<int>> graph = {{1}, {2}, {3}, {}};

  auto total = num_edges(graph);
  REQUIRE(total == 3);
}

TEST_CASE("num_edges(g) - star graph", "[num_edges][cpo][patterns]") {
  // Star: center vertex connects to all others
  std::vector<std::vector<int>> graph = {{1, 2, 3, 4, 5}, // center
                                         {},
                                         {},
                                         {},
                                         {},
                                         {}};

  auto total = num_edges(graph);
  REQUIRE(total == 5);
}

TEST_CASE("num_edges(g) - disconnected components", "[num_edges][cpo][patterns]") {
  // Two separate triangles
  std::vector<std::vector<int>> graph = {{1, 2},                 // component 1
                                         {0, 2}, {0, 1}, {4, 5}, // component 2
                                         {3, 5}, {3, 4}};

  auto total = num_edges(graph);
  REQUIRE(total == 12);
}

// =============================================================================
// Test: Consistency with Other CPOs
// =============================================================================

TEST_CASE("num_edges(g) - consistency with edges(g,u)", "[num_edges][cpo][consistency]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  // Count edges manually using edges(g, u)
  std::size_t manual_count = 0;
  for (auto v : vertices(graph)) {
    for (auto e : edges(graph, v)) {
      (void)e; // Suppress unused warning
      manual_count++;
    }
  }

  auto total = num_edges(graph);
  REQUIRE(total == manual_count);
}

TEST_CASE("num_edges(g) - consistency across different storage types", "[num_edges][cpo][consistency]") {
  // Same logical graph in different storage
  std::vector<std::vector<int>> vec_graph = {{1, 2}, {2}, {}};

  std::deque<std::deque<int>> deque_graph = {{1, 2}, {2}, {}};

  std::map<int, std::vector<int>> map_graph = {{0, {1, 2}}, {1, {2}}, {2, {}}};

  auto vec_count   = num_edges(vec_graph);
  auto deque_count = num_edges(deque_graph);
  auto map_count   = num_edges(map_graph);

  REQUIRE(vec_count == 3);
  REQUIRE(deque_count == 3);
  REQUIRE(map_count == 3);
}

// =============================================================================
// Test: Type Deduction
// =============================================================================

TEST_CASE("num_edges(g) - return type is integral", "[num_edges][cpo][types]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {2}, {}};

  auto result = num_edges(graph);

  static_assert(std::integral<decltype(result)>, "num_edges should return integral type");
  REQUIRE(result == 3);
}

// =============================================================================
// Test: Const Correctness
// =============================================================================

TEST_CASE("num_edges(g) - const graph", "[num_edges][cpo][const]") {
  const std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  auto total = num_edges(graph);
  REQUIRE(total == 6);
}

TEST_CASE("num_edges(g) - const map graph", "[num_edges][cpo][const]") {
  const std::map<int, std::vector<int>> graph = {{0, {1, 2}}, {1, {2}}, {2, {}}};

  auto total = num_edges(graph);
  REQUIRE(total == 3);
}

// =============================================================================
// Test: Edge Cases
// =============================================================================

TEST_CASE("num_edges(g) - all vertices have same number of edges", "[num_edges][cpo][edge_cases]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {0, 2}, {0, 1}, {4, 5}, {3, 5}, {3, 4}};

  auto total = num_edges(graph);
  REQUIRE(total == 12); // 6 vertices * 2 edges each
}

TEST_CASE("num_edges(g) - mixed edge counts", "[num_edges][cpo][edge_cases]") {
  std::vector<std::vector<int>> graph = {
        {},          // 0 edges
        {0},         // 1 edge
        {0, 1},      // 2 edges
        {0, 1, 2},   // 3 edges
        {0, 1, 2, 3} // 4 edges
  };

  auto total = num_edges(graph);
  REQUIRE(total == 10); // 0 + 1 + 2 + 3 + 4
}

// =============================================================================
// Test: Integration with vertices() and num_vertices()
// =============================================================================

TEST_CASE("num_edges(g) - integration with num_vertices", "[num_edges][cpo][integration]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  auto vertex_count = num_vertices(graph);
  auto edge_count   = num_edges(graph);

  REQUIRE(vertex_count == 4);
  REQUIRE(edge_count == 6);
}

TEST_CASE("num_edges(g) - average degree calculation", "[num_edges][cpo][integration]") {
  std::vector<std::vector<int>> graph = {
        {1, 2, 3}, // out-degree 3
        {2, 3},    // out-degree 2
        {3},       // out-degree 1
        {}         // out-degree 0
  };

  auto vertex_count = num_vertices(graph);
  auto edge_count   = num_edges(graph);

  // Average out-degree = total edges / num vertices
  double avg_degree = static_cast<double>(edge_count) / static_cast<double>(vertex_count);

  REQUIRE(vertex_count == 4);
  REQUIRE(edge_count == 6);
  REQUIRE(avg_degree == 1.5);
}
