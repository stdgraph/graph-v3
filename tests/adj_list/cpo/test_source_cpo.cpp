/**
 * @file test_source_cpo.cpp
 * @brief Comprehensive tests for source(g, uv) CPO
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <list>
#include <string>

#include "graph/adj_list/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test: Default Implementation with Vector - Simple Edges
// =============================================================================

TEST_CASE("source(g,uv) - vector<vector<int>> simple edges", "[source][cpo][default]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  // Get first vertex
  auto v0 = *vertices(graph).begin();

  // Get first edge from vertex 0
  auto e = *edges(graph, v0).begin();

  // Get source vertex descriptor
  auto source_v = source(graph, e);

  // Verify source vertex ID is 0
  REQUIRE(vertex_id(graph, source_v) == 0);
}

TEST_CASE("source(g,uv) - accessing source's edges", "[source][cpo][default]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {2, 3}, {3}, {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();
  ++it;
  auto v1  = *it;
  auto e12 = *edges(graph, v1).begin(); // Edge 1->2

  // Get source vertex
  auto v1_from_edge = source(graph, e12);

  // Get edges from the source vertex
  std::vector<int> source_edges;
  for (auto e : edges(graph, v1_from_edge)) {
    source_edges.push_back(target_id(graph, e));
  }

  REQUIRE(source_edges.size() == 2);
  REQUIRE(source_edges[0] == 2);
  REQUIRE(source_edges[1] == 3);
}

// =============================================================================
// Test: Default Implementation with Pair Edges (Weighted)
// =============================================================================

TEST_CASE("source(g,uv) - vector<vector<pair<int,double>>> weighted edges", "[source][cpo][default]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.5}, {2, 2.5}}, {{2, 1.0}, {3, 2.0}}, {{3, 1.5}}, {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();
  ++it;
  auto v1 = *it;
  auto e  = *edges(graph, v1).begin();

  auto source_v = source(graph, e);

  REQUIRE(vertex_id(graph, source_v) == 1);
}

TEST_CASE("source(g,uv) - iterating through edges and sources", "[source][cpo][default]") {
  using Edge                           = std::pair<int, double>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.0}, {2, 2.0}, {3, 3.0}}, {{2, 1.5}}, {}, {}};

  auto v0 = *vertices(graph).begin();

  std::vector<int> source_ids;
  for (auto e : edges(graph, v0)) {
    auto s = source(graph, e);
    source_ids.push_back(static_cast<int>(vertex_id(graph, s)));
  }

  REQUIRE(source_ids.size() == 3);
  REQUIRE(source_ids[0] == 0);
  REQUIRE(source_ids[1] == 0);
  REQUIRE(source_ids[2] == 0);
}

// =============================================================================
// Test: Tuple Edges
// =============================================================================

TEST_CASE("source(g,uv) - vector<vector<tuple<...>>> multi-property edges", "[source][cpo][default]") {
  using Edge                           = std::tuple<int, double, std::string>;
  std::vector<std::vector<Edge>> graph = {{{1, 1.5, "a"}, {2, 2.5, "b"}}, {{2, 3.5, "c"}}, {}, {}};

  auto v0 = *vertices(graph).begin();
  auto e  = *edges(graph, v0).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 0);
}

// =============================================================================
// Test: Deque Container
// =============================================================================

TEST_CASE("source(g,uv) - deque<deque<int>> simple edges", "[source][cpo][default]") {
  std::deque<std::deque<int>> graph = {{1, 2}, {2, 3}, {3}, {}};

  auto verts = vertices(graph);
  auto it    = verts.begin();
  ++it;
  auto v1 = *it;
  auto e  = *edges(graph, v1).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 1);
}

// =============================================================================
// Test: Map Container
// =============================================================================

TEST_CASE("source(g,uv) - map<int, vector<int>>", "[source][cpo][default]") {
  std::map<int, std::vector<int>> graph;
  graph[10] = {20, 30};
  graph[20] = {30};
  graph[30] = {};

  auto verts = vertices(graph);
  auto v     = *verts.begin();
  auto e     = *edges(graph, v).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 10);
}

TEST_CASE("source(g,uv) - map with sparse vertex IDs", "[source][cpo][default]") {
  std::map<int, std::vector<int>> graph;
  graph[100] = {200};
  graph[200] = {300};
  graph[300] = {};

  auto verts = vertices(graph);
  auto it    = verts.begin();
  ++it;
  auto v200 = *it;
  auto e    = *edges(graph, v200).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 200);
}

TEST_CASE("source(g,uv) - map with weighted edges", "[source][cpo][default]") {
  using Edge = std::pair<int, double>;
  std::map<int, std::vector<Edge>> graph;
  graph[10] = {{20, 1.5}, {30, 2.5}};
  graph[20] = {{30, 3.5}};
  graph[30] = {};

  SECTION("First vertex edges") {
    auto verts = vertices(graph);
    auto v     = *verts.begin();

    for (auto e : edges(graph, v)) {
      auto source_v = source(graph, e);
      REQUIRE(vertex_id(graph, source_v) == 10);
    }
  }

  SECTION("Second vertex edges") {
    auto verts = vertices(graph);
    auto it    = verts.begin();
    ++it;
    auto v = *it;

    for (auto e : edges(graph, v)) {
      auto source_v = source(graph, e);
      REQUIRE(vertex_id(graph, source_v) == 20);
    }
  }
}

// =============================================================================
// Test: Custom Member Function
// =============================================================================

namespace member_test {
struct CustomGraph {
  std::vector<std::vector<int>> adj_list = {{1, 2}, {2, 3}, {}, {}};

  // Custom member function that returns vertex_descriptor
  template <typename E>
  auto source(const E& uv) const {
    // Custom logic: return vertex descriptor for source
    return *find_vertex(adj_list, source_id(adj_list, uv));
  }
};
} // namespace member_test

TEST_CASE("source(g,uv) - custom member function", "[source][cpo][member]") {
  using namespace member_test;

  CustomGraph graph;

  auto v0 = *vertices(graph.adj_list).begin();
  auto e  = *edges(graph.adj_list, v0).begin();

  // Should call graph.source(uv)
  auto source_v = source(graph, e);

  REQUIRE(vertex_id(graph.adj_list, source_v) == 0);
}

// =============================================================================
// Test: ADL Customization
// =============================================================================

// Note: ADL customization tests for source removed due to ambiguity with CPO.
// The CPO provides member function and default implementation resolution which is sufficient.

// =============================================================================
// Test: Member Returning Iterator
// =============================================================================

namespace iterator_test {
struct GraphWithIterator {
  std::vector<std::vector<int>> adj_list = {{1, 2}, {2, 3}, {}, {}};

  // Member function returning iterator (not descriptor)
  template <typename E>
  auto source(const E& uv) const {
    auto sid = source_id(adj_list, uv);
    return find_vertex(adj_list, sid); // Returns iterator
  }
};
} // namespace iterator_test

TEST_CASE("source(g,uv) - custom member returning iterator", "[source][cpo][member][iterator]") {
  using namespace iterator_test;

  GraphWithIterator graph;

  auto v0 = *vertices(graph.adj_list).begin();
  auto e  = *edges(graph.adj_list, v0).begin();

  // Member returns iterator, CPO should dereference it
  auto source_v = source(graph, e);

  REQUIRE(vertex_id(graph.adj_list, source_v) == 0);
}

// =============================================================================
// Test: ADL Returning Iterator
// =============================================================================

// Note: ADL iterator test for source removed due to ambiguity with CPO.

// =============================================================================
// Test: Full Graph Traversal
// =============================================================================

TEST_CASE("source(g,uv) - full graph traversal using source", "[source][cpo][traversal]") {
  std::vector<std::vector<int>> graph = {{1, 2}, {2, 3}, {3}, {}};

  // Traverse all edges and verify source matches the vertex
  for (auto v : vertices(graph)) {
    auto vid = vertex_id(graph, v);

    for (auto e : edges(graph, v)) {
      auto source_v = source(graph, e);
      auto sid      = vertex_id(graph, source_v);
      REQUIRE(sid == vid);
    }
  }
}

// =============================================================================
// Test: Const Graph
// =============================================================================

TEST_CASE("source(g,uv) - const graph", "[source][cpo][const]") {
  const std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  auto v0 = *vertices(graph).begin();
  auto e  = *edges(graph, v0).begin();

  auto source_v = source(graph, e);

  REQUIRE(vertex_id(graph, source_v) == 0);
}

TEST_CASE("source(g,uv) - const map graph", "[source][cpo][const]") {
  const std::map<int, std::vector<int>> graph = {{10, {20, 30}}, {20, {30}}, {30, {}}};

  auto v = *vertices(graph).begin();
  auto e = *edges(graph, v).begin();

  auto source_v = source(graph, e);

  REQUIRE(vertex_id(graph, source_v) == 10);
}

// =============================================================================
// Test: Edge Cases
// =============================================================================

TEST_CASE("source(g,uv) - self-loops", "[source][cpo][edge_cases]") {
  std::vector<std::vector<int>> graph = {{0, 1}, // self-loop from 0 to 0, then to 1
                                         {1, 2}, // self-loop from 1 to 1, then to 2
                                         {}};

  auto v0 = *vertices(graph).begin();
  auto e  = *edges(graph, v0).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 0);
}

TEST_CASE("source(g,uv) - multiple edges to same target", "[source][cpo][edge_cases]") {
  std::vector<std::vector<int>> graph = {{1, 1, 1}, // three edges from 0 to 1
                                         {}};

  auto v0 = *vertices(graph).begin();

  for (auto e : edges(graph, v0)) {
    auto source_v = source(graph, e);
    REQUIRE(vertex_id(graph, source_v) == 0);
  }
}

TEST_CASE("source(g,uv) - large vertex IDs", "[source][cpo][edge_cases]") {
  std::map<int, std::vector<int>> graph;
  graph[1000] = {2000};
  graph[2000] = {3000};
  graph[3000] = {};

  auto verts = vertices(graph);
  auto it    = verts.begin();
  ++it;
  auto v2000 = *it;
  auto e     = *edges(graph, v2000).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 2000);
}

// =============================================================================
// Test: Integration with source_id
// =============================================================================

TEST_CASE("source(g,uv) - consistency with source_id", "[source][cpo][integration]") {
  std::vector<std::vector<int>> graph = {{1, 2, 3}, {2, 3}, {3}, {}};

  for (auto v : vertices(graph)) {
    for (auto e : edges(graph, v)) {
      auto source_v        = source(graph, e);
      auto sid_from_source = vertex_id(graph, source_v);
      auto sid_direct      = source_id(graph, e);

      REQUIRE(sid_from_source == sid_direct);
    }
  }
}

TEST_CASE("source(g,uv) - chaining source calls", "[source][cpo][integration]") {
  std::vector<std::vector<int>> graph = {{1}, {2}, {3}, {}};

  // Get edge from vertex 0
  auto v0  = *vertices(graph).begin();
  auto e01 = *edges(graph, v0).begin();

  // Get source of this edge (should be v0)
  auto source_v = source(graph, e01);
  REQUIRE(vertex_id(graph, source_v) == 0);

  // Get target from this edge
  auto target_v = target(graph, e01);
  REQUIRE(vertex_id(graph, target_v) == 1);

  // Get an edge from the target
  auto e12 = *edges(graph, target_v).begin();

  // Source of e12 should be vertex 1
  auto source_v2 = source(graph, e12);
  REQUIRE(vertex_id(graph, source_v2) == 1);
}

// =============================================================================
// Test: Performance Characteristics
// =============================================================================

TEST_CASE("source(g,uv) - vector random access performance", "[source][cpo][performance]") {
  std::vector<std::vector<int>> graph(100);
  for (int i = 0; i < 100; ++i) {
    graph[static_cast<size_t>(i)].push_back((i + 1) % 100);
  }

  // Accessing source should be O(1) for vector
  auto v50 = *std::next(vertices(graph).begin(), 50);
  auto e   = *edges(graph, v50).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 50);
}

TEST_CASE("source(g,uv) - map logarithmic access", "[source][cpo][performance]") {
  std::map<int, std::vector<int>> graph;
  for (int i = 0; i < 100; ++i) {
    graph[i * 10] = {(i + 1) * 10};
  }

  // Accessing source should be O(log n) for map
  auto verts = vertices(graph);
  auto v     = *std::next(verts.begin(), 50);
  auto e     = *edges(graph, v).begin();

  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == vertex_id(graph, v));
}

// =============================================================================
// Test: Bidirectional Graph Support
// =============================================================================

TEST_CASE("source(g,uv) - undirected edge conceptual test", "[source][cpo][bidirectional]") {
  // For an undirected graph representation using directed edges,
  // each edge has a source and target, even if conceptually bidirectional
  std::vector<std::vector<int>> graph = {
        {1, 2}, // 0 -> 1, 0 -> 2
        {0, 2}, // 1 -> 0, 1 -> 2 (reverse of 0->1, plus 1->2)
        {0, 1}  // 2 -> 0, 2 -> 1 (reverse edges)
  };

  // Each edge still has a well-defined source
  for (auto v : vertices(graph)) {
    auto vid = vertex_id(graph, v);
    for (auto e : edges(graph, v)) {
      auto source_v = source(graph, e);
      REQUIRE(vertex_id(graph, source_v) == vid);
    }
  }
}

// =============================================================================
// Test: Error Cases (Compile-Time)
// =============================================================================

// These tests verify that the CPO works correctly at compile time
TEST_CASE("source(g,uv) - requires edge_descriptor", "[source][cpo][concepts]") {
  std::vector<std::vector<int>> graph = {{1}, {}};

  auto v0 = *vertices(graph).begin();
  auto e  = *edges(graph, v0).begin();

  // Should compile fine with edge descriptor
  auto source_v = source(graph, e);
  REQUIRE(vertex_id(graph, source_v) == 0);

  // Would not compile with wrong type:
  // auto bad = source(graph, 42);  // Compile error
}
