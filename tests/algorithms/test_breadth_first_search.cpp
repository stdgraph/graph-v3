/**
 * @file test_breadth_first_search.cpp
 * @brief Comprehensive tests for breadth-first search algorithms from breadth_first_search.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/breadth_first_search.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <set>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// =============================================================================
// Helper Types and Utilities
// =============================================================================

// Visitor that tracks BFS traversal events
struct BFSTrackingVisitor {
  std::vector<int>                 initialized;
  std::vector<int>                 discovered;
  std::vector<int>                 examined;
  std::vector<int>                 finished;
  std::vector<std::pair<int, int>> edges_examined;

  template <typename G, typename T>
  void on_initialize_vertex(const G& g, const T& v) {
    initialized.push_back(static_cast<int>(vertex_id(g, v)));
  }

  template <typename G, typename T>
  void on_discover_vertex(const G& g, const T& v) {
    discovered.push_back(static_cast<int>(vertex_id(g, v)));
  }

  template <typename G, typename T>
  void on_examine_vertex(const G& g, const T& v) {
    examined.push_back(static_cast<int>(vertex_id(g, v)));
  }

  template <typename G, typename T>
  void on_finish_vertex(const G& g, const T& v) {
    finished.push_back(static_cast<int>(vertex_id(g, v)));
  }

  template <typename G, typename Edge>
  void on_examine_edge(const G& g, const Edge& e) {
    // Store edge endpoints for verification
    edges_examined.push_back({-1, -1}); // Placeholder, actual implementation would extract source/target
  }

  void reset() {
    initialized.clear();
    discovered.clear();
    examined.clear();
    finished.clear();
    edges_examined.clear();
  }
};

// Simple counting visitor
struct CountingVisitor {
  int vertices_discovered = 0;
  int vertices_examined   = 0;
  int vertices_finished   = 0;
  int edges_examined      = 0;

  template <typename G, typename T>
  void on_discover_vertex(const G&, const T&) {
    ++vertices_discovered;
  }
  template <typename G, typename T>
  void on_examine_vertex(const G&, const T&) {
    ++vertices_examined;
  }
  template <typename G, typename T>
  void on_finish_vertex(const G&, const T&) {
    ++vertices_finished;
  }
  template <typename G, typename T>
  void on_examine_edge(const G&, const T&) {
    ++edges_examined;
  }
};

// =============================================================================
// Single-Source BFS Tests
// =============================================================================

TEST_CASE("breadth_first_search - single vertex", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  auto            g = single_vertex<Graph>();
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 1);
  REQUIRE(visitor.vertices_examined == 1);
  REQUIRE(visitor.vertices_finished == 1);
  REQUIRE(visitor.edges_examined == 0);
}

TEST_CASE("breadth_first_search - single edge", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  auto            g = single_edge<Graph>();
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 2);
  REQUIRE(visitor.vertices_examined == 2);
  REQUIRE(visitor.vertices_finished == 2);
  REQUIRE(visitor.edges_examined >= 1); // At least one edge examined
}

TEST_CASE("breadth_first_search - path graph traversal", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Path: 0 -> 1 -> 2 -> 3
  auto               g = path_graph_4<Graph>();
  BFSTrackingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // All 4 vertices should be discovered
  REQUIRE(visitor.discovered.size() == 4);
  REQUIRE(visitor.examined.size() == 4);
  REQUIRE(visitor.finished.size() == 4);

  // Vertex 0 should be discovered first
  REQUIRE(visitor.discovered[0] == 0);
}

TEST_CASE("breadth_first_search - cycle graph (no infinite loop)", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Cycle: 0 -> 1 -> 2 -> 3 -> 4 -> 0
  auto            g = cycle_graph_5<Graph>();
  CountingVisitor visitor;

  // Critical test: should not loop infinitely due to visited tracking
  breadth_first_search(g, 0u, visitor);

  // Should visit each vertex exactly once
  REQUIRE(visitor.vertices_discovered == 5);
  REQUIRE(visitor.vertices_examined == 5);
  REQUIRE(visitor.vertices_finished == 5);
}

TEST_CASE("breadth_first_search - disconnected graph (single component)", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Two disconnected components: 0-1-2 and 3-4
  Graph           g({{0, 1}, {1, 2}, {3, 4}});
  CountingVisitor visitor;

  // Start from component 0-1-2
  breadth_first_search(g, 0u, visitor);

  // Should only visit vertices in the same component as source
  REQUIRE(visitor.vertices_discovered == 3); // 0, 1, 2
}

TEST_CASE("breadth_first_search - self-loop handling", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  auto            g = self_loop<Graph>();
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // Should visit vertex 0 once (visited flag prevents re-visiting)
  REQUIRE(visitor.vertices_discovered == 1);
  REQUIRE(visitor.vertices_examined == 1);
}

TEST_CASE("breadth_first_search - complete graph", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Complete graph K4: every vertex connected to every other
  Graph           g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // All 4 vertices reachable from any vertex
  REQUIRE(visitor.vertices_discovered == 4);
}

TEST_CASE("breadth_first_search - tree structure", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Binary tree:      0
  //                  / \\
    //                1     2
  //               / \\
    //              3   4
  Graph           g({{0, 1}, {0, 2}, {1, 3}, {1, 4}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 5);
  REQUIRE(visitor.vertices_examined == 5);
}

TEST_CASE("breadth_first_search - DAG (directed acyclic graph)", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // DAG: 0 -> 1 -> 3
  //      |         ^
  //      v         |
  //      2 ------->+
  Graph           g({{0, 1}, {0, 2}, {1, 3}, {2, 3}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // All 4 vertices reachable from 0
  REQUIRE(visitor.vertices_discovered == 4);
}

TEST_CASE("breadth_first_search - diamond graph", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Diamond: 0 -> 1,2 -> 3
  Graph           g({{0, 1}, {0, 2}, {1, 3}, {2, 3}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // All 4 vertices discovered
  REQUIRE(visitor.vertices_discovered == 4);
  // Vertex 3 should only be discovered once (not twice)
  REQUIRE(visitor.vertices_examined == 4);
}

TEST_CASE("breadth_first_search - isolated vertex as source", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Graph with isolated vertex: 0-1-2, 3 (isolated), 4-5
  Graph           g({{0, 1}, {1, 2}, {4, 5}});
  CountingVisitor visitor;

  // Start from isolated vertex
  breadth_first_search(g, 3u, visitor);

  // Should only visit the isolated vertex
  REQUIRE(visitor.vertices_discovered == 1);
}

TEST_CASE("breadth_first_search - long chain", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Long chain: 0->1->2->3->4->5->6->7->8->9
  Graph           g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 10);
}

TEST_CASE("breadth_first_search - star graph (hub and spokes)", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Star: center 0 connected to 1,2,3,4,5
  Graph           g({{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}});
  CountingVisitor visitor;

  // Start from center
  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 6);
}

TEST_CASE("breadth_first_search - bipartite graph", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Bipartite K_{2,3}: vertices 0,1 connected to vertices 2,3,4 (directed)
  Graph           g({{0, 2}, {0, 3}, {0, 4}, {1, 2}, {1, 3}, {1, 4}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // Starting from 0, can reach 0,2,3,4 (but not 1 in directed graph)
  REQUIRE(visitor.vertices_discovered == 4);
}

TEST_CASE("breadth_first_search - multiple paths to same vertex", "[algorithm][bfs][single_source]") {
  using Graph = vov_void;

  // Multiple paths from 0 to 4:
  // 0 -> 1 -> 4
  // 0 -> 2 -> 4
  // 0 -> 3 -> 4
  Graph           g({{0, 1}, {0, 2}, {0, 3}, {1, 4}, {2, 4}, {3, 4}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // Vertex 4 should be discovered exactly once
  REQUIRE(visitor.vertices_discovered == 5);
  REQUIRE(visitor.vertices_examined == 5);
}

// =============================================================================
// Multi-Source BFS Tests
// =============================================================================

TEST_CASE("breadth_first_search - multi-source with vector", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  // Graph: 0-1-2, 3-4
  Graph           g({{0, 1}, {1, 2}, {3, 4}});
  CountingVisitor visitor;

  std::vector<uint32_t> sources = {0, 3};
  breadth_first_search(g, sources, visitor);

  // Should visit all 5 vertices starting from both components
  REQUIRE(visitor.vertices_discovered == 5);
}

TEST_CASE("breadth_first_search - multi-source with array", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  auto            g = path_graph_4<Graph>();
  CountingVisitor visitor;

  std::array<uint32_t, 2> sources = {0, 3};
  breadth_first_search(g, sources, visitor);

  // All 4 vertices should be visited
  REQUIRE(visitor.vertices_discovered == 4);
}

TEST_CASE("breadth_first_search - multi-source empty sources", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  auto            g = path_graph_4<Graph>();
  CountingVisitor visitor;

  std::vector<uint32_t> sources;
  breadth_first_search(g, sources, visitor);

  // No vertices should be discovered with empty sources
  REQUIRE(visitor.vertices_discovered == 0);
}

TEST_CASE("breadth_first_search - multi-source single source", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  auto            g = path_graph_4<Graph>();
  CountingVisitor visitor;

  std::vector<uint32_t> sources = {0};
  breadth_first_search(g, sources, visitor);

  // Should behave same as single-source
  REQUIRE(visitor.vertices_discovered == 4);
}

TEST_CASE("breadth_first_search - multi-source duplicate sources", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  auto            g = path_graph_4<Graph>();
  CountingVisitor visitor;

  // Same source multiple times
  std::vector<uint32_t> sources = {0, 0, 0};
  breadth_first_search(g, sources, visitor);

  // Duplicate sources cause on_discover_vertex to be called for each source initialization
  // Vertex 0 discovered 3 times (once per duplicate), then 1,2,3 discovered once each
  // Total: 3 + 3 = 6 discover calls
  REQUIRE(visitor.vertices_discovered == 6);
}

TEST_CASE("breadth_first_search - multi-source adjacent vertices", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  auto            g = path_graph_4<Graph>();
  CountingVisitor visitor;

  // Start from adjacent vertices in directed path: 0->1->2->3
  std::vector<uint32_t> sources = {1, 2};
  breadth_first_search(g, sources, visitor);

  // From 1 can reach 1,2,3; from 2 can reach 2,3; combined: 1,2,3
  REQUIRE(visitor.vertices_discovered == 3);
}

TEST_CASE("breadth_first_search - multi-source disconnected components", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  // Three disconnected components: {0,1}, {2,3,4}, {5}
  Graph           g({{0, 1}, {2, 3}, {3, 4}, {5, 5}}); // Add isolated vertex 5 with self-loop
  CountingVisitor visitor;

  // Start from one vertex in each component
  std::vector<uint32_t> sources = {0, 2, 5};
  breadth_first_search(g, sources, visitor);

  // All 6 vertices should be visited
  REQUIRE(visitor.vertices_discovered == 6);
}

TEST_CASE("breadth_first_search - multi-source all vertices", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  auto            g = path_graph_4<Graph>();
  CountingVisitor visitor;

  // Start from all vertices
  std::vector<uint32_t> sources = {0, 1, 2, 3};
  breadth_first_search(g, sources, visitor);

  // All should be discovered/examined
  REQUIRE(visitor.vertices_discovered == 4);
  REQUIRE(visitor.vertices_examined == 4);
}

TEST_CASE("breadth_first_search - multi-source overlapping neighborhoods", "[algorithm][bfs][multi_source]") {
  using Graph = vov_void;

  // Star graph with two sources at edge
  Graph           g({{0, 2}, {1, 2}, {2, 3}, {2, 4}});
  CountingVisitor visitor;

  std::vector<uint32_t> sources = {0, 1};
  breadth_first_search(g, sources, visitor);

  // All 5 vertices reachable
  REQUIRE(visitor.vertices_discovered == 5);
}

// =============================================================================
// Visitor Integration Tests
// =============================================================================

TEST_CASE("breadth_first_search - visitor callback ordering", "[algorithm][bfs][visitor]") {
  using Graph = vov_void;

  // Simple path: 0 -> 1 -> 2
  Graph              g({{0, 1}, {1, 2}});
  BFSTrackingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // Check that vertex 0 is discovered before being examined
  REQUIRE(visitor.discovered.size() >= 1);
  REQUIRE(visitor.examined.size() >= 1);
  REQUIRE(visitor.discovered[0] == 0);

  // All discovered vertices should be examined
  REQUIRE(visitor.discovered.size() == visitor.examined.size());

  // All examined vertices should be finished
  REQUIRE(visitor.examined.size() == visitor.finished.size());
}

// Visitor with only some methods - defined at namespace scope
struct MinimalDiscoverVisitor {
  int discovered = 0;
  template <typename G, typename T>
  void on_discover_vertex(const G&, const T&) {
    ++discovered;
  }
};

TEST_CASE("breadth_first_search - visitor without optional methods", "[algorithm][bfs][visitor]") {
  using Graph = vov_void;

  auto g = path_graph_4<Graph>();

  MinimalDiscoverVisitor visitor;
  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.discovered == 4);
}

TEST_CASE("breadth_first_search - empty visitor", "[algorithm][bfs][visitor]") {
  using Graph = vov_void;

  auto g = path_graph_4<Graph>();

  // Should work with default empty visitor
  REQUIRE_NOTHROW(breadth_first_search(g, 0u));
}

// =============================================================================
// Edge Cases and Boundary Conditions
// =============================================================================

TEST_CASE("breadth_first_search - graph with parallel edges", "[algorithm][bfs][edge_cases]") {
  using Graph = vov_void;

  // Parallel edges: 0 -> 1 (twice)
  Graph           g({{0, 1}, {0, 1}, {1, 2}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // Should handle parallel edges correctly (visited tracking)
  REQUIRE(visitor.vertices_discovered == 3);
}

TEST_CASE("breadth_first_search - graph with multiple self-loops", "[algorithm][bfs][edge_cases]") {
  using Graph = vov_void;

  // Vertex with multiple self-loops
  Graph           g({{0, 0}, {0, 0}, {0, 1}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 2);
}

TEST_CASE("breadth_first_search - large vertex ID", "[algorithm][bfs][edge_cases]") {
  using Graph = vov_void;

  // Graph with larger vertex IDs
  Graph g({{0, 4}, {4, 3}});

  CountingVisitor visitor;
  breadth_first_search(g, 0u, visitor);

  REQUIRE(visitor.vertices_discovered == 3); // 0, 4, 3
}

TEST_CASE("breadth_first_search - strongly connected component", "[algorithm][bfs][edge_cases]") {
  using Graph = vov_void;

  // Strongly connected: 0 <-> 1 <-> 2 <-> 0
  Graph           g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 0}, {0, 2}});
  CountingVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // All 3 vertices reachable from any vertex
  REQUIRE(visitor.vertices_discovered == 3);
  REQUIRE(visitor.vertices_examined == 3);
}

// =============================================================================
// Consistency Tests - Single-source vs Multi-source
// =============================================================================

TEST_CASE("breadth_first_search - single vs multi-source equivalence", "[algorithm][bfs][consistency]") {
  using Graph = vov_void;

  auto g = path_graph_4<Graph>();

  CountingVisitor v1, v2;

  // Single-source
  breadth_first_search(g, 0u, v1);

  // Multi-source with single element
  std::vector<uint32_t> sources = {0};
  breadth_first_search(g, sources, v2);

  // Results should be identical
  REQUIRE(v1.vertices_discovered == v2.vertices_discovered);
  REQUIRE(v1.vertices_examined == v2.vertices_examined);
  REQUIRE(v1.vertices_finished == v2.vertices_finished);
}

// =============================================================================
// Vertex-ID Visitor Tests
// =============================================================================

// Visitor that accepts vertex ids instead of vertex descriptors
struct BFSIdVisitor {
  std::vector<size_t> initialized;
  std::vector<size_t> discovered;
  std::vector<size_t> examined;
  std::vector<size_t> finished;

  template <typename G>
  void on_initialize_vertex(const G&, const vertex_id_t<G>& uid) {
    initialized.push_back(static_cast<size_t>(uid));
  }
  template <typename G>
  void on_discover_vertex(const G&, const vertex_id_t<G>& uid) {
    discovered.push_back(static_cast<size_t>(uid));
  }
  template <typename G>
  void on_examine_vertex(const G&, const vertex_id_t<G>& uid) {
    examined.push_back(static_cast<size_t>(uid));
  }
  template <typename G>
  void on_finish_vertex(const G&, const vertex_id_t<G>& uid) {
    finished.push_back(static_cast<size_t>(uid));
  }
};

TEST_CASE("breadth_first_search - vertex id visitor", "[algorithm][bfs][visitor_id]") {
  using Graph = vov_void;

  // Path: 0 -> 1 -> 2 -> 3
  auto          g = path_graph_4<Graph>();
  BFSIdVisitor visitor;

  breadth_first_search(g, 0u, visitor);

  // All 4 vertices should be discovered, examined, and finished via id-based callbacks
  REQUIRE(visitor.discovered.size() == 4);
  REQUIRE(visitor.examined.size() == 4);
  REQUIRE(visitor.finished.size() == 4);

  // BFS from 0: discover order should be 0, 1, 2, 3
  REQUIRE(visitor.discovered == std::vector<size_t>{0, 1, 2, 3});
}

TEST_CASE("breadth_first_search - vertex id visitor matches descriptor visitor", "[algorithm][bfs][visitor_id]") {
  using Graph = vov_void;

  auto g = path_graph_4<Graph>();

  BFSTrackingVisitor desc_visitor;
  BFSIdVisitor       id_visitor;

  breadth_first_search(g, 0u, desc_visitor);
  breadth_first_search(g, 0u, id_visitor);

  // ID-based visitor should produce the same vertex ids as descriptor-based visitor
  REQUIRE(desc_visitor.discovered.size() == id_visitor.discovered.size());
  for (size_t i = 0; i < desc_visitor.discovered.size(); ++i) {
    REQUIRE(desc_visitor.discovered[i] == static_cast<int>(id_visitor.discovered[i]));
  }
}
