/**
 * @file test_dynamic_graph_cpo_forward.cpp
 * @brief Consolidated CPO tests for forward-only edge containers
 * 
 * This file consolidates CPO tests for containers with forward-only edge iterators:
 * - vofl_graph_traits (vector vertices + forward_list edges)
 * - dofl_graph_traits (deque vertices + forward_list edges)
 * 
 * These containers have the following characteristics:
 * - Forward iterators only (not bidirectional or random_access)
 * - NOT a sized_range (forward_list deliberately omits size())
 * - num_edges(g, u) and num_edges(g, uid) are NOT supported
 * - Use degree(g, u) instead for per-vertex edge counts
 * - Edge insertion uses push_front(), so edges appear in REVERSE order
 * 
 * NOTE: This consolidation covers tests that are identical across vofl and dofl.
 * Tests that require specific edge order assertions account for reverse order.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/vofl_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// Helper to add edges for EV=void graphs (since create_edge() requires non-void EV)
template <typename Graph>
void add_edges(Graph& g, std::initializer_list<std::pair<uint64_t, uint64_t>> edges_list) {
    std::vector<copyable_edge_t<uint64_t, void>> edge_data;
    for (auto [src, tgt] : edges_list) {
        edge_data.push_back({src, tgt});
    }
    g.load_edges(edge_data);
}

//==================================================================================================
// 1. vertices(g) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO vertices(g)", "[dynamic_graph][cpo][vertices][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("returns vertex_descriptor_view") {
        Graph g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        
        // Should be a sized range
        REQUIRE(std::ranges::size(v_range) == 5);
        
        // Should be iterable
        size_t count = 0;
        for ([[maybe_unused]] auto&& u : v_range) {
            ++count;
        }
        REQUIRE(count == 5);
    }
    
    SECTION("empty graph returns empty range") {
        Graph g;
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 0);
        REQUIRE(v_range.begin() == v_range.end());
    }
    
    SECTION("vertex IDs are sequential from 0") {
        Graph g;
        g.resize_vertices(3);
        
        std::vector<uint64_t> ids;
        for (auto&& u : vertices(g)) {
            ids.push_back(vertex_id(g, u));
        }
        
        REQUIRE(ids == std::vector<uint64_t>{0, 1, 2});
    }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO num_vertices(g)", "[dynamic_graph][cpo][num_vertices][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("empty graph has zero vertices") {
        Graph g;
        REQUIRE(num_vertices(g) == 0);
    }
    
    SECTION("returns correct count after resize") {
        Graph g;
        g.resize_vertices(10);
        REQUIRE(num_vertices(g) == 10);
    }
    
    SECTION("returns count based on edges loaded") {
        Graph g;
        add_edges(g, {{0, 1}, {1, 2}});  // Should create 3 vertices (0, 1, 2)
        REQUIRE(num_vertices(g) == 3);
    }
}

//==================================================================================================
// 3. num_edges(g) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO num_edges(g)", "[dynamic_graph][cpo][num_edges][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("empty graph has zero edges") {
        Graph g;
        g.resize_vertices(5);
        REQUIRE(num_edges(g) == 0);
    }
    
    SECTION("returns correct edge count") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}, {1, 2}});
        REQUIRE(num_edges(g) == 3);
    }
    
    SECTION("counts self-loops") {
        Graph g;
        g.resize_vertices(2);
        add_edges(g, {{0, 0}, {0, 1}});  // 0->0 is self-loop
        REQUIRE(num_edges(g) == 2);
    }
}

//==================================================================================================
// NOTE: num_edges(g, u) and num_edges(g, uid) are NOT supported for forward containers
// because forward_list is not a sized_range. Use degree(g, u) instead.
//==================================================================================================

//==================================================================================================
// 4. has_edge(g) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO has_edge(g)", "[dynamic_graph][cpo][has_edge][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("empty graph has no edges") {
        Graph g;
        REQUIRE_FALSE(has_edge(g));
    }
    
    SECTION("graph with only vertices has no edges") {
        Graph g;
        g.resize_vertices(5);
        REQUIRE_FALSE(has_edge(g));
    }
    
    SECTION("graph with edges returns true") {
        Graph g;
        g.resize_vertices(2);
        add_edges(g, {{0, 1}});
        REQUIRE(has_edge(g));
    }
}

//==================================================================================================
// 5. degree(g, u) CPO Tests - Replaces num_edges(g, u) for forward containers
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO degree(g, u)", "[dynamic_graph][cpo][degree][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("vertex with no edges has degree 0") {
        Graph g;
        g.resize_vertices(3);
        
        auto v0 = *find_vertex(g, uint64_t{0});
        REQUIRE(degree(g, v0) == 0);
    }
    
    SECTION("returns correct out-degree") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        REQUIRE(degree(g, v0) == 2);
    }
    
    SECTION("self-loop counts as one edge") {
        Graph g;
        g.resize_vertices(2);
        add_edges(g, {{0, 0}});  // self-loop
        
        auto v0 = *find_vertex(g, uint64_t{0});
        REQUIRE(degree(g, v0) == 1);
    }
    
    SECTION("per-vertex degree counts") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}, {1, 2}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto v1 = *find_vertex(g, uint64_t{1});
        auto v2 = *find_vertex(g, uint64_t{2});
        
        REQUIRE(degree(g, v0) == 2);
        REQUIRE(degree(g, v1) == 1);
        REQUIRE(degree(g, v2) == 0);
    }
}

//==================================================================================================
// 6. find_vertex(g, uid) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO find_vertex(g, uid)", "[dynamic_graph][cpo][find_vertex][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("finds existing vertex") {
        Graph g;
        g.resize_vertices(5);
        
        auto it = find_vertex(g, uint64_t{2});
        REQUIRE(it != vertices(g).end());
        REQUIRE(vertex_id(g, *it) == 2);
    }
    
    SECTION("returns end for non-existent vertex") {
        Graph g;
        g.resize_vertices(3);
        
        auto it = find_vertex(g, uint64_t{10});
        REQUIRE(it == vertices(g).end());
    }
    
    SECTION("works on empty graph") {
        Graph g;
        
        auto it = find_vertex(g, uint64_t{0});
        REQUIRE(it == vertices(g).end());
    }
}

//==================================================================================================
// 7. edges(g, u) CPO Tests - Count only, order-specific tests below
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO edges(g, u) count", "[dynamic_graph][cpo][edges][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("empty vertex has no edges") {
        Graph g;
        g.resize_vertices(3);
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto edge_range = edges(g, v0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto&& e : edge_range) {
            ++count;
        }
        REQUIRE(count == 0);
    }
    
    SECTION("vertex with edges returns correct count") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto edge_range = edges(g, v0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto&& e : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }
    
    SECTION("multiple vertices with different edge counts") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}, {1, 2}});
        
        // Verify counts per vertex
        REQUIRE(degree(g, *find_vertex(g, uint64_t{0})) == 2);
        REQUIRE(degree(g, *find_vertex(g, uint64_t{1})) == 1);
        REQUIRE(degree(g, *find_vertex(g, uint64_t{2})) == 0);
    }
}

//==================================================================================================
// 8. edges(g, u) Order Tests - forward_list uses push_front (REVERSE order)
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO edges(g, u) order", "[dynamic_graph][cpo][edges][order][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("edges appear in reverse order (push_front)") {
        Graph g;
        g.resize_vertices(4);
        add_edges(g, {{0, 1}, {0, 2}, {0, 3}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto edge_range = edges(g, v0);
        
        std::vector<uint64_t> targets;
        for (auto&& e : edge_range) {
            targets.push_back(target_id(g, e));
        }
        
        // forward_list uses push_front, so last added appears first (REVERSE)
        REQUIRE(targets.size() == 3);
        REQUIRE(targets[0] == 3);  // Last added, first in list
        REQUIRE(targets[1] == 2);
        REQUIRE(targets[2] == 1);  // First added, last in list
    }
}

//==================================================================================================
// 9. target_id(g, uv) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO target_id(g, uv)", "[dynamic_graph][cpo][target_id][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("returns correct target ID") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 2}});  // Single edge from 0 to 2
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto edge_range = edges(g, v0);
        auto first_edge = *edge_range.begin();
        
        REQUIRE(target_id(g, first_edge) == 2);
    }
    
    SECTION("works with self-loop") {
        Graph g;
        g.resize_vertices(2);
        add_edges(g, {{0, 0}});  // Self-loop
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto first_edge = *edges(g, v0).begin();
        
        REQUIRE(target_id(g, first_edge) == 0);
    }
}

//==================================================================================================
// 10. Integration Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("forward CPO integration", "[dynamic_graph][cpo][integration][forward]",
                   (vofl_graph_traits<void, void, void, uint64_t, false>),
                   (dofl_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("traversal using CPOs") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}, {1, 2}});
        
        // Count total edges using CPO-based traversal
        size_t edge_count = 0;
        for (auto&& u : vertices(g)) {
            for (auto&& e : edges(g, u)) {
                (void)e;
                ++edge_count;
            }
        }
        
        REQUIRE(edge_count == num_edges(g));
    }
    
    SECTION("degree matches edge iteration count") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        
        // Count by iteration
        auto count = decltype(degree(g, v0)){0};
        for ([[maybe_unused]] auto&& e : edges(g, v0)) {
            ++count;
        }
        
        // degree should match
        REQUIRE(degree(g, v0) == count);
    }
}
