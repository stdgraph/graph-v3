/**
 * @file test_dynamic_graph_cpo_random_access.cpp
 * @brief Consolidated CPO tests for random-access edge containers
 * 
 * This file consolidates CPO tests for containers with random-access edge iterators:
 * - vov_graph_traits (vector vertices + vector edges)
 * - vod_graph_traits (vector vertices + deque edges)
 * - dov_graph_traits (deque vertices + vector edges)
 * - dod_graph_traits (deque vertices + deque edges)
 * 
 * These containers support all CPOs including num_edges(g, u) and num_edges(g, uid)
 * because they have random_access iterators and are sized_range.
 * 
 * Container behavior: edges appear in insertion order (push_back)
 * 
 * NOTE: This is a PROTOTYPE to validate the consolidation approach.
 * Tests that differ between containers due to edge order or other
 * behavioral differences should remain in container-specific files.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vod_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <graph/container/traits/dod_graph_traits.hpp>
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
// Type trait to get container name for test output
//==================================================================================================
template <typename Traits>
constexpr const char* traits_name() {
    if constexpr (std::is_same_v<Traits, vov_graph_traits<void, void, void, uint64_t, false>>)
        return "vov";
    else if constexpr (std::is_same_v<Traits, vod_graph_traits<void, void, void, uint64_t, false>>)
        return "vod";
    else if constexpr (std::is_same_v<Traits, dov_graph_traits<void, void, void, uint64_t, false>>)
        return "dov";
    else if constexpr (std::is_same_v<Traits, dod_graph_traits<void, void, void, uint64_t, false>>)
        return "dod";
    else
        return "unknown";
}

//==================================================================================================
// 1. vertices(g) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO vertices(g)", "[dynamic_graph][cpo][vertices][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
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

TEMPLATE_TEST_CASE("random-access CPO num_vertices(g)", "[dynamic_graph][cpo][num_vertices][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
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

TEMPLATE_TEST_CASE("random-access CPO num_edges(g)", "[dynamic_graph][cpo][num_edges][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
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
// 4. num_edges(g, u) CPO Tests - Random-access specific (sized_range)
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO num_edges(g, u)", "[dynamic_graph][cpo][num_edges][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("vertex with no edges returns 0") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}});  // Only vertex 0 has an edge
        
        auto v2 = *find_vertex(g, uint64_t{2});
        REQUIRE(num_edges(g, v2) == 0);
    }
    
    SECTION("returns correct per-vertex edge count") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}, {1, 2}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        auto v1 = *find_vertex(g, uint64_t{1});
        auto v2 = *find_vertex(g, uint64_t{2});
        
        REQUIRE(num_edges(g, v0) == 2);
        REQUIRE(num_edges(g, v1) == 1);
        REQUIRE(num_edges(g, v2) == 0);
    }
    
    SECTION("counts self-loops correctly") {
        Graph g;
        g.resize_vertices(2);
        add_edges(g, {{0, 0}, {0, 1}});  // 0->0 is self-loop
        
        auto v0 = *find_vertex(g, uint64_t{0});
        REQUIRE(num_edges(g, v0) == 2);
    }
}

//==================================================================================================
// 5. num_edges(g, uid) CPO Tests - Random-access specific (sized_range)
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO num_edges(g, uid)", "[dynamic_graph][cpo][num_edges][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("vertex with no edges returns 0") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}});  // Only vertex 0 has an edge
        
        REQUIRE(num_edges(g, uint64_t{2}) == 0);
    }
    
    SECTION("returns correct per-vertex edge count by ID") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}, {1, 2}});
        
        REQUIRE(num_edges(g, uint64_t{0}) == 2);
        REQUIRE(num_edges(g, uint64_t{1}) == 1);
        REQUIRE(num_edges(g, uint64_t{2}) == 0);
    }
}

//==================================================================================================
// 6. has_edge(g) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO has_edge(g)", "[dynamic_graph][cpo][has_edge][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
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
// 7. degree(g, u) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO degree(g, u)", "[dynamic_graph][cpo][degree][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
    using Graph = dynamic_graph<void, void, void, uint64_t, false, TestType>;
    
    SECTION("vertex with no edges has degree 0") {
        Graph g;
        g.resize_vertices(3);
        
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 0);
    }
    
    SECTION("returns correct out-degree") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}});
        
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 2);
    }
    
    SECTION("self-loop counts as one edge") {
        Graph g;
        g.resize_vertices(2);
        add_edges(g, {{0, 0}});  // self-loop
        
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 1);
    }
}

//==================================================================================================
// 8. find_vertex(g, uid) CPO Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO find_vertex(g, uid)", "[dynamic_graph][cpo][find_vertex][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
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
// 9. Integration Tests - Container-agnostic
//==================================================================================================

TEMPLATE_TEST_CASE("random-access CPO integration", "[dynamic_graph][cpo][integration][random_access]",
                   (vov_graph_traits<void, void, void, uint64_t, false>),
                   (vod_graph_traits<void, void, void, uint64_t, false>),
                   (dov_graph_traits<void, void, void, uint64_t, false>),
                   (dod_graph_traits<void, void, void, uint64_t, false>)) {
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
    
    SECTION("degree equals num_edges for random-access containers") {
        Graph g;
        g.resize_vertices(3);
        add_edges(g, {{0, 1}, {0, 2}});
        
        auto v0 = *find_vertex(g, uint64_t{0});
        // For random-access containers, degree and num_edges(g,u) should be equal
        REQUIRE(degree(g, v0) == num_edges(g, v0));
    }
}
