/**
 * @file test_depth_first_search.cpp
 * @brief Comprehensive tests for depth-first search algorithms from depth_first_search.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/depth_first_search.hpp>
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

// Visitor that tracks DFS traversal events
struct DFSTrackingVisitor {
    std::vector<int> initialized;
    std::vector<int> started; 
    std::vector<int> discovered;
    std::vector<int> finished;
    std::vector<std::pair<int, int>> edges_examined;
    std::vector<std::pair<int, int>> tree_edges;
    std::vector<std::pair<int, int>> back_edges;
    std::vector<std::pair<int, int>> forward_or_cross_edges;
    std::vector<std::pair<int, int>> finished_edges;
    
    template<typename G, typename T>
    void on_initialize_vertex(const G& g, const T& v) {
        initialized.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    template<typename G, typename T>
    void on_start_vertex(const G& g, const T& v) {
        started.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    template<typename G, typename T>
    void on_discover_vertex(const G& g, const T& v) { 
        discovered.push_back(static_cast<int>(vertex_id(g, v))); 
    }
    
    template<typename G, typename T>
    void on_finish_vertex(const G& g, const T& v) { 
        finished.push_back(static_cast<int>(vertex_id(g, v))); 
    }
    
    template<typename G, typename Edge>
    void on_examine_edge(const G& g, const Edge& e) {
        edges_examined.push_back({-1, -1}); // Placeholder
    }
    
    template<typename G, typename Edge>
    void on_tree_edge(const G& g, const Edge& e) {
        tree_edges.push_back({-1, -1}); // Placeholder
    }
    
    template<typename G, typename Edge>
    void on_back_edge(const G& g, const Edge& e) {
        back_edges.push_back({-1, -1}); // Placeholder
    }
    
    template<typename G, typename Edge>
    void on_forward_or_cross_edge(const G& g, const Edge& e) {
        forward_or_cross_edges.push_back({-1, -1}); // Placeholder
    }
    
    template<typename G, typename Edge>
    void on_finish_edge(const G& g, const Edge& e) {
        finished_edges.push_back({-1, -1}); // Placeholder
    }
    
    void reset() {
        initialized.clear();
        started.clear();
        discovered.clear();
        finished.clear();
        edges_examined.clear();
        tree_edges.clear();
        back_edges.clear();
        forward_or_cross_edges.clear();
        finished_edges.clear();
    }
};

// Simple counting visitor
struct DFSCountingVisitor {
    int vertices_initialized = 0;
    int vertices_started = 0;
    int vertices_discovered = 0;
    int vertices_finished = 0;
    int edges_examined = 0;
    int tree_edges = 0;
    int back_edges = 0;
    int forward_or_cross_edges = 0;
    int finished_edges = 0;
    
    template<typename G, typename T> void on_initialize_vertex(const G&, const T&) { ++vertices_initialized; }
    template<typename G, typename T> void on_start_vertex(const G&, const T&) { ++vertices_started; }
    template<typename G, typename T> void on_discover_vertex(const G&, const T&) { ++vertices_discovered; }
    template<typename G, typename T> void on_finish_vertex(const G&, const T&) { ++vertices_finished; }
    template<typename G, typename T> void on_examine_edge(const G&, const T&) { ++edges_examined; }
    template<typename G, typename T> void on_tree_edge(const G&, const T&) { ++tree_edges; }
    template<typename G, typename T> void on_back_edge(const G&, const T&) { ++back_edges; }
    template<typename G, typename T> void on_forward_or_cross_edge(const G&, const T&) { ++forward_or_cross_edges; }
    template<typename G, typename T> void on_finish_edge(const G&, const T&) { ++finished_edges; }
};

// =============================================================================
// Single-Source DFS Tests
// =============================================================================

TEST_CASE("depth_first_search - single vertex", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    auto g = single_vertex<Graph>();
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_initialized == 1);
    REQUIRE(visitor.vertices_started == 1);
    REQUIRE(visitor.vertices_discovered == 1);
    REQUIRE(visitor.vertices_finished == 1);
    REQUIRE(visitor.edges_examined == 0);
    REQUIRE(visitor.tree_edges == 0);
    REQUIRE(visitor.back_edges == 0);
}

TEST_CASE("depth_first_search - single edge", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    auto g = single_edge<Graph>();
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 2);
    REQUIRE(visitor.vertices_finished == 2);
    REQUIRE(visitor.edges_examined >= 1); // At least one edge examined
    REQUIRE(visitor.tree_edges >= 1); // At least one tree edge
}

TEST_CASE("depth_first_search - path graph traversal", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Path: 0 -> 1 -> 2 -> 3
    auto g = path_graph_4<Graph>();
    DFSTrackingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // All 4 vertices should be discovered
    REQUIRE(visitor.discovered.size() == 4);
    REQUIRE(visitor.finished.size() == 4);
    
    // Vertex 0 should be discovered first
    REQUIRE(visitor.discovered[0] == 0);
    
    // Only source vertex is initialized (single-source DFS)
    REQUIRE(visitor.initialized.size() == 1);
    REQUIRE(visitor.initialized[0] == 0);
    
    // Source vertex started
    REQUIRE(visitor.started.size() == 1);
    REQUIRE(visitor.started[0] == 0);
}

TEST_CASE("depth_first_search - cycle detection with back edges", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Cycle: 0 -> 1 -> 2 -> 3 -> 4 -> 0
    auto g = cycle_graph_5<Graph>();
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Should visit each vertex exactly once
    REQUIRE(visitor.vertices_discovered == 5);
    REQUIRE(visitor.vertices_finished == 5);
    
    // Should detect at least one back edge (the cycle edge)
    REQUIRE(visitor.back_edges >= 1);
}

TEST_CASE("depth_first_search - disconnected graph (single component)", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Two disconnected components: 0-1-2 and 3-4
    Graph g({{0, 1}, {1, 2}, {3, 4}});
    DFSCountingVisitor visitor;
    
    // Start from component 0-1-2
    depth_first_search(g, 0u, visitor);
    
    // Should only visit vertices in the same component as source
    REQUIRE(visitor.vertices_discovered == 3); // 0, 1, 2
    
    // Only source vertex is initialized (single-source DFS)
    REQUIRE(visitor.vertices_initialized == 1);
}

TEST_CASE("depth_first_search - self-loop handling", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    auto g = self_loop<Graph>();
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Should visit vertex 0 once
    REQUIRE(visitor.vertices_discovered == 1);
    REQUIRE(visitor.vertices_finished == 1);
    
    // Self-loop should be detected as back edge
    REQUIRE(visitor.back_edges >= 1);
}

TEST_CASE("depth_first_search - tree structure", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Binary tree:      0
    //                  / \
    //                1     2
    //               / \
    //              3   4
    Graph g({{0,1}, {0,2}, {1,3}, {1,4}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 5);
    REQUIRE(visitor.vertices_finished == 5);
    
    // All edges in a tree should be tree edges
    REQUIRE(visitor.tree_edges == 4);
    REQUIRE(visitor.back_edges == 0);
}

TEST_CASE("depth_first_search - DAG (directed acyclic graph)", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // DAG: 0 -> 1 -> 3
    //      |         ^
    //      v         |
    //      2 ------->+
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // All 4 vertices reachable from 0
    REQUIRE(visitor.vertices_discovered == 4);
    REQUIRE(visitor.vertices_finished == 4);
    
    // DAG should have no back edges (acyclic)
    REQUIRE(visitor.back_edges == 0);
}

TEST_CASE("depth_first_search - diamond graph", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Diamond: 0 -> 1,2 -> 3
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // All 4 vertices discovered
    REQUIRE(visitor.vertices_discovered == 4);
    REQUIRE(visitor.vertices_finished == 4);
    
    // Should have forward or cross edge (second path to vertex 3)
    REQUIRE(visitor.forward_or_cross_edges >= 1);
}

TEST_CASE("depth_first_search - isolated vertex as source", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Graph with isolated vertex: 0-1-2, 3 (isolated), 4-5
    Graph g({{0,1}, {1,2}, {4,5}});
    DFSCountingVisitor visitor;
    
    // Start from isolated vertex
    depth_first_search(g, 3u, visitor);
    
    // Should only visit the isolated vertex
    REQUIRE(visitor.vertices_discovered == 1);
    REQUIRE(visitor.vertices_finished == 1);
}

TEST_CASE("depth_first_search - long chain", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Long chain: 0->1->2->3->4->5->6->7->8->9
    Graph g({{0,1}, {1,2}, {2,3}, {3,4}, {4,5}, {5,6}, {6,7}, {7,8}, {8,9}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 10);
    REQUIRE(visitor.vertices_finished == 10);
    
    // All edges in chain should be tree edges
    REQUIRE(visitor.tree_edges == 9);
}

TEST_CASE("depth_first_search - star graph (hub and spokes)", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Star: center 0 connected to 1,2,3,4,5
    Graph g({{0,1}, {0,2}, {0,3}, {0,4}, {0,5}});
    DFSCountingVisitor visitor;
    
    // Start from center
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 6);
    REQUIRE(visitor.vertices_finished == 6);
    
    // All edges should be tree edges
    REQUIRE(visitor.tree_edges == 5);
}

TEST_CASE("depth_first_search - bipartite graph", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Bipartite K_{2,3}: vertices 0,1 connected to vertices 2,3,4 (directed)
    Graph g({{0,2}, {0,3}, {0,4}, {1,2}, {1,3}, {1,4}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Starting from 0, can reach 0,2,3,4 (but not 1 in directed graph)
    REQUIRE(visitor.vertices_discovered == 4);
}

TEST_CASE("depth_first_search - multiple paths to same vertex", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Multiple paths from 0 to 4:
    // 0 -> 1 -> 4
    // 0 -> 2 -> 4
    // 0 -> 3 -> 4
    Graph g({{0,1}, {0,2}, {0,3}, {1,4}, {2,4}, {3,4}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Vertex 4 should be discovered exactly once
    REQUIRE(visitor.vertices_discovered == 5);
    REQUIRE(visitor.vertices_finished == 5);
    
    // Multiple edges to vertex 4 should create forward/cross edges
    REQUIRE(visitor.forward_or_cross_edges >= 2);
}

TEST_CASE("depth_first_search - strongly connected component", "[algorithm][dfs][single_source]") {
    using Graph = vov_void;
    
    // Strongly connected: 0 <-> 1 <-> 2 <-> 0
    Graph g({{0,1}, {1,0}, {1,2}, {2,1}, {2,0}, {0,2}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // All 3 vertices reachable from any vertex
    REQUIRE(visitor.vertices_discovered == 3);
    REQUIRE(visitor.vertices_finished == 3);
    
    // Strongly connected graph should have back edges
    REQUIRE(visitor.back_edges >= 1);
}

// =============================================================================
// Visitor Integration Tests
// =============================================================================

TEST_CASE("depth_first_search - visitor callback ordering", "[algorithm][dfs][visitor]") {
    using Graph = vov_void;
    
    // Simple path: 0 -> 1 -> 2
    Graph g({{0,1}, {1,2}});
    DFSTrackingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Only source vertex is initialized (single-source DFS)
    REQUIRE(visitor.initialized.size() == 1);
    REQUIRE(visitor.initialized[0] == 0);
    
    // Check start vertex
    REQUIRE(visitor.started.size() == 1);
    REQUIRE(visitor.started[0] == 0);
    
    // Check that vertex 0 is discovered first
    REQUIRE(visitor.discovered.size() >= 1);
    REQUIRE(visitor.discovered[0] == 0);
    
    // All discovered vertices should be finished
    REQUIRE(visitor.discovered.size() == visitor.finished.size());
    
    // Finish order should be reverse of discovery for linear path in DFS
    REQUIRE(visitor.finished[0] == 2); // Deepest vertex finishes first
    REQUIRE(visitor.finished[2] == 0); // Root finishes last
}

TEST_CASE("depth_first_search - tree edge vs back edge classification", "[algorithm][dfs][visitor]") {
    using Graph = vov_void;
    
    // Graph with both tree edges and a back edge
    // 0 -> 1 -> 2
    // |         |
    // +<--------+  (back edge)
    Graph g({{0,1}, {1,2}, {2,0}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 3);
    
    // Should have 2 tree edges (0->1, 1->2)
    REQUIRE(visitor.tree_edges == 2);
    
    // Should have 1 back edge (2->0)
    REQUIRE(visitor.back_edges == 1);
    
    // Total edges examined should equal sum of edge types
    REQUIRE(visitor.edges_examined == visitor.tree_edges + visitor.back_edges + visitor.forward_or_cross_edges);
    
    // All examined edges should be finished
    REQUIRE(visitor.edges_examined == visitor.finished_edges);
}

// Visitor with only some methods
struct MinimalDiscoverVisitor {
    int discovered = 0;
    template<typename G, typename T> void on_discover_vertex(const G&, const T&) { ++discovered; }
};

TEST_CASE("depth_first_search - visitor without optional methods", "[algorithm][dfs][visitor]") {
    using Graph = vov_void;
    
    auto g = path_graph_4<Graph>();
    
    MinimalDiscoverVisitor visitor;
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.discovered == 4);
}

TEST_CASE("depth_first_search - empty visitor", "[algorithm][dfs][visitor]") {
    using Graph = vov_void;
    
    auto g = path_graph_4<Graph>();
    
    // Should work with default empty visitor
    REQUIRE_NOTHROW(depth_first_search(g, 0u));
}

// =============================================================================
// Edge Cases and Boundary Conditions
// =============================================================================

TEST_CASE("depth_first_search - graph with parallel edges", "[algorithm][dfs][edge_cases]") {
    using Graph = vov_void;
    
    // Parallel edges: 0 -> 1 (twice)
    Graph g({{0,1}, {0,1}, {1,2}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Should handle parallel edges correctly
    REQUIRE(visitor.vertices_discovered == 3);
    
    // Second edge to vertex 1 should be a forward/cross edge 
    // (vertex 1 is Black/finished when the parallel edge is processed)
    REQUIRE(visitor.forward_or_cross_edges >= 1);
}

TEST_CASE("depth_first_search - graph with multiple self-loops", "[algorithm][dfs][edge_cases]") {
    using Graph = vov_void;
    
    // Vertex with multiple self-loops
    Graph g({{0,0}, {0,0}, {0,1}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 2);
    
    // Self-loops should be back edges
    REQUIRE(visitor.back_edges >= 2);
}

TEST_CASE("depth_first_search - large vertex ID", "[algorithm][dfs][edge_cases]") {
    using Graph = vov_void;
    
    // Graph with larger vertex IDs
    Graph g({{0,4}, {4,3}});
    
    DFSCountingVisitor visitor;
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 3); // 0, 4, 3
}

// =============================================================================
// Edge Classification Tests
// =============================================================================

TEST_CASE("depth_first_search - forward edge in DAG", "[algorithm][dfs][edge_classification]") {
    using Graph = vov_void;
    
    // DAG with forward edge: 0 -> 1 -> 2, 0 -> 2 (forward edge)
    Graph g({{0,1}, {1,2}, {0,2}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 3);
    
    // Should have tree edges and at least one forward/cross edge
    REQUIRE(visitor.tree_edges == 2);
    REQUIRE(visitor.forward_or_cross_edges >= 1);
}

TEST_CASE("depth_first_search - cross edge detection", "[algorithm][dfs][edge_classification]") {
    using Graph = vov_void;
    
    // Graph structure that creates a cross edge
    // 0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3 (cross edge from 2 to 3)
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 4);
    
    // Should detect forward or cross edge (2->3 after 1->3)
    REQUIRE(visitor.forward_or_cross_edges >= 1);
}

TEST_CASE("depth_first_search - cycle with multiple back edges", "[algorithm][dfs][edge_classification]") {
    using Graph = vov_void;
    
    // Complete graph K3: every vertex connected to every other
    Graph g({{0,1}, {0,2}, {1,0}, {1,2}, {2,0}, {2,1}});
    DFSCountingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    REQUIRE(visitor.vertices_discovered == 3);
    
    // Should have tree edges and multiple back edges (cycles)
    REQUIRE(visitor.tree_edges >= 2);
    REQUIRE(visitor.back_edges >= 2);
}

// =============================================================================
// Finish Order Tests
// =============================================================================

TEST_CASE("depth_first_search - finish order in tree", "[algorithm][dfs][finish_order]") {
    using Graph = vov_void;
    
    // Tree: 0 -> 1 -> 2
    Graph g({{0,1}, {1,2}});
    DFSTrackingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // Vertices should finish in reverse order of discovery for linear path
    REQUIRE(visitor.discovered[0] == 0);
    REQUIRE(visitor.discovered[1] == 1);
    REQUIRE(visitor.discovered[2] == 2);
    
    // Finish order: deepest first
    REQUIRE(visitor.finished[0] == 2);
    REQUIRE(visitor.finished[1] == 1);
    REQUIRE(visitor.finished[2] == 0);
}

TEST_CASE("depth_first_search - finish order in DAG for topological sort", "[algorithm][dfs][finish_order]") {
    using Graph = vov_void;
    
    // DAG: 0 -> 1 -> 3
    //      |         ^
    //      v         |
    //      2 ------->+
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    DFSTrackingVisitor visitor;
    
    depth_first_search(g, 0u, visitor);
    
    // All vertices discovered
    REQUIRE(visitor.discovered.size() == 4);
    
    // Vertex 0 (source) should finish last
    REQUIRE(visitor.finished.back() == 0);
    
    // Vertex 3 (sink) should finish before its predecessors
    auto finish_pos_3 = std::find(visitor.finished.begin(), visitor.finished.end(), 3);
    auto finish_pos_1 = std::find(visitor.finished.begin(), visitor.finished.end(), 1);
    auto finish_pos_2 = std::find(visitor.finished.begin(), visitor.finished.end(), 2);
    
    REQUIRE(finish_pos_3 < finish_pos_1);
    REQUIRE(finish_pos_3 < finish_pos_2);
}
