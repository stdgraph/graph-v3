/**
 * @file test_topological_sort.cpp
 * @brief Comprehensive tests for topological sort algorithm from topological_sort.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/topological_sort.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <set>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// =============================================================================
// Helper Functions
// =============================================================================

// Verify that the ordering is valid: for every edge (u,v), u appears before v
template<typename G, typename Order>
bool is_valid_topological_order(const G& g, const Order& order) {
    // Build position map
    std::unordered_map<uint32_t, size_t> position;
    for (size_t i = 0; i < order.size(); ++i) {
        position[order[i]] = i;
    }
    
    // Check every edge
    for (auto uid : order) {
        for (auto&& [uv] : views::incidence(g, uid)) {
            auto vid = target_id(g, uv);
            // If target is in the ordering, it must come after source
            if (position.count(vid) && position[uid] >= position[vid]) {
                return false;
            }
        }
    }
    return true;
}

// =============================================================================
// Full-Graph Topological Sort Tests
// =============================================================================

TEST_CASE("topological_sort full-graph - simple DAG", "[algorithm][topological_sort][full_graph]") {
    using Graph = vov_void;
    
    // DAG: 0 -> 1 -> 2
    Graph g({{0,1}, {1,2}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order == std::vector<uint32_t>{0, 1, 2});
}

TEST_CASE("topological_sort full-graph - diamond DAG", "[algorithm][topological_sort][full_graph]") {
    using Graph = vov_void;
    
    // Diamond: 0 -> {1,2} -> 3
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order.front() == 0);
    REQUIRE(order.back() == 3);
}

TEST_CASE("topological_sort full-graph - disconnected components", "[algorithm][topological_sort][full_graph]") {
    using Graph = vov_void;
    
    // Two components: 0->1, 2->3
    Graph g({{0,1}, {2,3}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
    
    // All vertices should be present
    std::set<uint32_t> vertices(order.begin(), order.end());
    REQUIRE(vertices.size() == 4);
}

TEST_CASE("topological_sort full-graph - cycle detection", "[algorithm][topological_sort][full_graph][cycle]") {
    using Graph = vov_void;
    
    // Cycle: 0 -> 1 -> 2 -> 0
    Graph g({{0,1}, {1,2}, {2,0}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE_FALSE(success);
}

TEST_CASE("topological_sort full-graph - single vertex", "[algorithm][topological_sort][full_graph]") {
    using Graph = vov_void;
    
    auto g = single_vertex<Graph>();
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 1);
    REQUIRE(order[0] == 0);
}

TEST_CASE("topological_sort full-graph - complex DAG multiple paths", "[algorithm][topological_sort][full_graph]") {
    using Graph = vov_void;
    
    // Complex DAG: 0->{1,2}, 1->3, 2->3
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
    
    // Verify all vertices present
    std::set<uint32_t> vertices(order.begin(), order.end());
    REQUIRE(vertices == std::set<uint32_t>{0, 1, 2, 3});
    REQUIRE(order.front() == 0);
    REQUIRE(order.back() == 3);
}

TEST_CASE("topological_sort full-graph - tree structure", "[algorithm][topological_sort][full_graph]") {
    using Graph = vov_void;
    
    // Binary tree: 0 -> {1, 2}, 1 -> {3, 4}
    Graph g({{0,1}, {0,2}, {1,3}, {1,4}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 5);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order.front() == 0);
}

// =============================================================================
// Single-Source Topological Sort Tests
// =============================================================================

TEST_CASE("topological_sort single-source - simple DAG", "[algorithm][topological_sort][single_source]") {
    using Graph = vov_void;
    
    // DAG: 0 -> 1 -> 2
    Graph g({{0,1}, {1,2}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, 0u, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order == std::vector<uint32_t>{0, 1, 2});
}

TEST_CASE("topological_sort single-source - diamond DAG", "[algorithm][topological_sort][single_source]") {
    using Graph = vov_void;
    
    // Diamond: 0 -> {1,2} -> 3
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, 0u, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order.front() == 0);
    REQUIRE(order.back() == 3);
}

TEST_CASE("topological_sort single-source - partial graph", "[algorithm][topological_sort][single_source]") {
    using Graph = vov_void;
    
    // Graph: 0->1->2, 3->4 (3,4 unreachable from 0)
    Graph g({{0,1}, {1,2}, {3,4}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, 0u, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);  // Only 0, 1, 2
    REQUIRE(std::find(order.begin(), order.end(), 3) == order.end());
    REQUIRE(std::find(order.begin(), order.end(), 4) == order.end());
}

TEST_CASE("topological_sort single-source - cycle detection", "[algorithm][topological_sort][single_source][cycle]") {
    using Graph = vov_void;
    
    // Cycle: 0 -> 1 -> 2 -> 0
    Graph g({{0,1}, {1,2}, {2,0}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, 0u, std::back_inserter(order));
    
    REQUIRE_FALSE(success);
}

TEST_CASE("topological_sort single-source - isolated vertex", "[algorithm][topological_sort][single_source]") {
    using Graph = vov_void;
    
    // Graph: 0->1, 2 (isolated), 3->4
    Graph g({{0,1}, {3,4}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, 2u, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 1);
    REQUIRE(order[0] == 2);
}

TEST_CASE("topological_sort single-source - tree structure", "[algorithm][topological_sort][single_source]") {
    using Graph = vov_void;
    
    // Binary tree: 0 -> {1, 2}, 1 -> {3, 4}
    Graph g({{0,1}, {0,2}, {1,3}, {1,4}});
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, 0u, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 5);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order.front() == 0);
}

TEST_CASE("topological_sort single-source - starting from middle vertex", "[algorithm][topological_sort][single_source]") {
    using Graph = vov_void;
    
    // Chain: 0->1->2->3->4
    Graph g({{0,1}, {1,2}, {2,3}, {3,4}});
    std::vector<uint32_t> order;
    
    // Start from vertex 2 (middle)
    bool success = topological_sort(g, 2u, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);  // Only 2, 3, 4
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order == std::vector<uint32_t>{2, 3, 4});
}

// =============================================================================
// Multi-Source Topological Sort Tests
// =============================================================================

TEST_CASE("topological_sort - single vertex", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    auto g = single_vertex<Graph>();
    std::vector<uint32_t> order;
    std::vector<uint32_t> sources = {0};
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 1);
    REQUIRE(order[0] == 0);
}

TEST_CASE("topological_sort - simple DAG", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // DAG: 0 -> 1 -> 2
    Graph g({{0,1}, {1,2}});
    std::vector<uint32_t> order;
    std::vector<uint32_t> sources = {0};
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);
    REQUIRE(is_valid_topological_order(g, order));
    
    // 0 must come before 1, 1 must come before 2
    auto pos_0 = std::find(order.begin(), order.end(), 0);
    auto pos_1 = std::find(order.begin(), order.end(), 1);
    auto pos_2 = std::find(order.begin(), order.end(), 2);
    REQUIRE(pos_0 < pos_1);
    REQUIRE(pos_1 < pos_2);
}

TEST_CASE("topological_sort - diamond DAG", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Diamond: 0 -> {1,2} -> 3
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    std::vector<uint32_t> order;
    std::vector<uint32_t> sources = {0};
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
    
    // 0 must be first, 3 must be last
    REQUIRE(order.front() == 0);
    REQUIRE(order.back() == 3);
}

TEST_CASE("topological_sort - multi-source same component", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Graph: 0->2, 1->2, 2->3
    Graph g({{0,2}, {1,2}, {2,3}});
    std::vector<uint32_t> sources = {0, 1};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
    
    // Both 0 and 1 must come before 2, 2 must come before 3
    auto pos_0 = std::find(order.begin(), order.end(), 0);
    auto pos_1 = std::find(order.begin(), order.end(), 1);
    auto pos_2 = std::find(order.begin(), order.end(), 2);
    auto pos_3 = std::find(order.begin(), order.end(), 3);
    REQUIRE(pos_0 < pos_2);
    REQUIRE(pos_1 < pos_2);
    REQUIRE(pos_2 < pos_3);
}

TEST_CASE("topological_sort - multi-source disconnected components", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Two components: 0->1, 2->3
    Graph g({{0,1}, {2,3}});
    std::vector<uint32_t> sources = {0, 2};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
}

TEST_CASE("topological_sort - partial graph (unreachable vertices)", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Graph: 0->1->2, 3->4 (3,4 unreachable from 0)
    Graph g({{0,1}, {1,2}, {3,4}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);  // Only 0, 1, 2
    REQUIRE(std::find(order.begin(), order.end(), 0) != order.end());
    REQUIRE(std::find(order.begin(), order.end(), 1) != order.end());
    REQUIRE(std::find(order.begin(), order.end(), 2) != order.end());
    // 3 and 4 should NOT be in the output
    REQUIRE(std::find(order.begin(), order.end(), 3) == order.end());
    REQUIRE(std::find(order.begin(), order.end(), 4) == order.end());
}

TEST_CASE("topological_sort - cycle detection simple", "[algorithm][topological_sort][cycle][multi_source]") {
    using Graph = vov_void;
    
    // Simple cycle: 0 -> 1 -> 2 -> 0
    Graph g({{0,1}, {1,2}, {2,0}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE_FALSE(success);  // Should detect cycle
}

TEST_CASE("topological_sort - cycle detection self-loop", "[algorithm][topological_sort][cycle][multi_source]") {
    using Graph = vov_void;
    
    // Self-loop at vertex 0
    auto g = self_loop<Graph>();
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE_FALSE(success);  // Self-loop is a cycle
}

TEST_CASE("topological_sort - cycle detection complex", "[algorithm][topological_sort][cycle][multi_source]") {
    using Graph = vov_void;
    
    // DAG with cycle: 0->1->2->3, 3->1 (cycle at 1-2-3)
    Graph g({{0,1}, {1,2}, {2,3}, {3,1}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE_FALSE(success);  // Should detect cycle
}

TEST_CASE("topological_sort - tree structure", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Binary tree: 0 -> {1, 2}, 1 -> {3, 4}
    Graph g({{0,1}, {0,2}, {1,3}, {1,4}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 5);
    REQUIRE(is_valid_topological_order(g, order));
    
    // Root must be first
    REQUIRE(order.front() == 0);
}

TEST_CASE("topological_sort - long chain", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Chain: 0->1->2->3->4->5
    Graph g({{0,1}, {1,2}, {2,3}, {3,4}, {4,5}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 6);
    REQUIRE(is_valid_topological_order(g, order));
    
    // Must be in exact order: 0,1,2,3,4,5
    REQUIRE(order == std::vector<uint32_t>{0, 1, 2, 3, 4, 5});
}

TEST_CASE("topological_sort - complex DAG", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Complex DAG with multiple paths
    // 0 -> {1, 2, 3}
    // 1 -> 4
    // 2 -> {4, 5}
    // 3 -> 5
    // 4 -> 6
    // 5 -> 6
    Graph g({{0,1}, {0,2}, {0,3}, {1,4}, {2,4}, {2,5}, {3,5}, {4,6}, {5,6}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 7);
    REQUIRE(is_valid_topological_order(g, order));
    
    // 0 must be first, 6 must be last
    REQUIRE(order.front() == 0);
    REQUIRE(order.back() == 6);
}

TEST_CASE("topological_sort - empty source list", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    Graph g({{0,1}, {1,2}});
    std::vector<uint32_t> sources;  // Empty
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.empty());  // No sources, no output
}

TEST_CASE("topological_sort - redundant sources", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Graph: 0->1->2
    Graph g({{0,1}, {1,2}});
    // Sources include both 0 and 1, but 1 is reachable from 0
    std::vector<uint32_t> sources = {0, 1};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);
    REQUIRE(is_valid_topological_order(g, order));
    
    // Each vertex should appear exactly once
    std::set<uint32_t> unique_vertices(order.begin(), order.end());
    REQUIRE(unique_vertices.size() == 3);
}

TEST_CASE("topological_sort - parallel edges", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Parallel edges: 0->1 (twice), 1->2
    Graph g({{0,1}, {0,1}, {1,2}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);
    REQUIRE(is_valid_topological_order(g, order));
}

TEST_CASE("topological_sort - isolated vertex as source", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Graph: 0->1, 2 (isolated), 3->4
    Graph g({{0,1}, {3,4}});
    std::vector<uint32_t> sources = {2};  // Isolated vertex
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 1);
    REQUIRE(order[0] == 2);
}

TEST_CASE("topological_sort - all vertices as sources", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Graph: 0->2, 1->2, 2->3
    Graph g({{0,2}, {1,2}, {2,3}});
    std::vector<uint32_t> sources = {0, 1, 2, 3};  // All vertices
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    REQUIRE(is_valid_topological_order(g, order));
}

TEST_CASE("topological_sort - verify unique vertices in output", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Diamond graph from multiple overlapping sources
    Graph g({{0,1}, {0,2}, {1,3}, {2,3}});
    std::vector<uint32_t> sources = {0, 1, 2};  // Overlapping sources
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 4);
    
    // Verify no duplicates
    std::set<uint32_t> unique_vertices(order.begin(), order.end());
    REQUIRE(unique_vertices.size() == 4);
}

TEST_CASE("topological_sort - strongly connected component", "[algorithm][topological_sort][cycle][multi_source]") {
    using Graph = vov_void;
    
    // Strongly connected: 0<->1<->2<->0
    Graph g({{0,1}, {1,0}, {1,2}, {2,1}, {2,0}, {0,2}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE_FALSE(success);  // Should detect cycles
}

TEST_CASE("topological_sort - DAG with forward edges", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // DAG with forward edge: 0->1->2, 0->2
    Graph g({{0,1}, {1,2}, {0,2}});
    std::vector<uint32_t> sources = {0};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 3);
    REQUIRE(is_valid_topological_order(g, order));
    REQUIRE(order == std::vector<uint32_t>{0, 1, 2});
}

TEST_CASE("topological_sort - verify output order property", "[algorithm][topological_sort][multi_source]") {
    using Graph = vov_void;
    
    // Create a graph where order matters
    Graph g({{0,3}, {1,3}, {2,3}, {3,4}});
    std::vector<uint32_t> sources = {0, 1, 2};
    std::vector<uint32_t> order;
    
    bool success = topological_sort(g, sources, std::back_inserter(order));
    
    REQUIRE(success);
    REQUIRE(order.size() == 5);
    
    // Verify that 3 comes after 0, 1, and 2
    auto pos_0 = std::distance(order.begin(), std::find(order.begin(), order.end(), 0));
    auto pos_1 = std::distance(order.begin(), std::find(order.begin(), order.end(), 1));
    auto pos_2 = std::distance(order.begin(), std::find(order.begin(), order.end(), 2));
    auto pos_3 = std::distance(order.begin(), std::find(order.begin(), order.end(), 3));
    auto pos_4 = std::distance(order.begin(), std::find(order.begin(), order.end(), 4));
    
    REQUIRE(pos_0 < pos_3);
    REQUIRE(pos_1 < pos_3);
    REQUIRE(pos_2 < pos_3);
    REQUIRE(pos_3 < pos_4);
    
    // Verify it's a valid topological order
    REQUIRE(is_valid_topological_order(g, order));
}
