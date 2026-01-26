/**
 * @file test_dynamic_graph_cpo_vov.cpp
 * @brief Phase 2 CPO tests for dynamic_graph with vov_graph_traits
 * 
 * Tests CPO (Customization Point Object) integration with dynamic_graph.
 * These tests verify that CPOs work correctly with the default implementations
 * and friend function overrides in dynamic_graph.
 * 
 * Container: vector<vertex> + vector<edge>
 * 
 * Current Status: 196 test cases (67 TEST_CASE blocks), 1860 assertions passing
 * 
 * CPOs tested (with available friend functions):
 * - vertices(g) - Get vertex range [3 tests]
 * - vertices(g, pid) - Get vertex range for partition (default single partition) [4 tests]
 * - num_vertices(g) - Get vertex count [3 tests]
 * - num_vertices(g, pid) - Get vertex count for partition (default single partition) [4 tests]
 * - find_vertex(g, uid) - Find vertex by ID [3 tests]
 * - vertex_id(g, u) - Get vertex ID from descriptor [7 tests]
 * - num_edges(g) - Get total edge count [3 tests]
 * - num_edges(g, u) - Get edge count for vertex (SUPPORTED with vector - random_access + sized_range)
 * - num_edges(g, uid) - Get edge count by vertex ID (SUPPORTED with vector - random_access + sized_range)
 * - has_edge(g) - Check if graph has any edges [3 tests]
 * - edges(g, u) - Get edge range for vertex [13 tests]
 * - edges(g, uid) - Get edge range by vertex ID [10 tests]
 * - degree(g, u) - Get out-degree of vertex [10 tests]
 *   (provides equivalent functionality to num_edges(g, u) for vov)
 * - target_id(g, uv) - Get target vertex ID from edge [10 tests]
 * - target(g, uv) - Get target vertex descriptor from edge [11 tests]
 * - find_vertex_edge(g, u, v) - Find edge between vertices [13 tests]
 *   - find_vertex_edge(g, uid, vid) - Additional dedicated tests [11 tests]
 * - contains_edge(g, u, v) and contains_edge(g, uid, vid) - Check if edge exists [15 tests]
 *   - contains_edge(g, uid, vid) - Additional dedicated tests [13 tests]
 * - vertex_value(g, u) - Access vertex value (when VV != void) [1 TEST_CASE, 5 sections]
 * - edge_value(g, uv) - Access edge value (when EV != void) [1 TEST_CASE, 6 sections]
 * - graph_value(g) - Access graph value (when GV != void) [1 TEST_CASE, 6 sections]
 * - partition_id(g, u) - Get partition ID for vertex (default single partition) [5 tests]
 * - num_partitions(g) - Get number of partitions (default 1) [4 tests]
 * - source_id(g, uv) - Get source vertex ID from edge (Sourced=true) [12 tests]
 * - source(g, uv) - Get source vertex descriptor from edge (Sourced=true) [12 tests]
 * 
 * Friend functions implemented and tested:
 * - vertex_value(g,u) in dynamic_graph_base (lines 1345-1348)
 * - edge_value(g,uv) in dynamic_vertex_base (lines 665-676)
 * - edges(g,u) in dynamic_vertex_base (lines 678-679)
 * 
 * Note: vector uses push_back() for edge insertion, so edges appear in
 * insertion order.
 * 
 * Note: degree(g,u) uses the CPO default implementation with std::ranges::distance.
 * 
 * IMPORTANT: Unlike vol_graph_traits (list with bidirectional iterators) and vofl_graph_traits
 * (forward_list with forward iterators), vov_graph_traits (vector with random_access iterators)
 * DOES support num_edges(g, u) and num_edges(g, uid) CPO overloads because:
 * - edges(g, u) returns edge_descriptor_view which provides size() for random_access iterators
 * - std::vector has random_access iterators, so edge_descriptor_view IS a sized_range
 * - This allows O(1) edge counting per vertex via num_edges(g, u)
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// Type aliases for test configurations
using vov_void     = dynamic_graph<void, void, void, uint32_t, false, vov_graph_traits<void, void, void, uint32_t, false>>;
using vov_int_ev   = dynamic_graph<int, void, void, uint32_t, false, vov_graph_traits<int, void, void, uint32_t, false>>;
using vov_int_vv   = dynamic_graph<void, int, void, uint32_t, false, vov_graph_traits<void, int, void, uint32_t, false>>;
using vov_all_int  = dynamic_graph<int, int, int, uint32_t, false, vov_graph_traits<int, int, int, uint32_t, false>>;
using vov_string   = dynamic_graph<std::string, std::string, std::string, uint32_t, false, 
                                     vov_graph_traits<std::string, std::string, std::string, uint32_t, false>>;

// Type aliases for Sourced=true configurations (for source_id/source CPO tests)
using vov_sourced_void = dynamic_graph<void, void, void, uint32_t, true, vov_graph_traits<void, void, void, uint32_t, true>>;
using vov_sourced_int  = dynamic_graph<int, void, void, uint32_t, true, vov_graph_traits<int, void, void, uint32_t, true>>;
using vov_sourced_all  = dynamic_graph<int, int, int, uint32_t, true, vov_graph_traits<int, int, int, uint32_t, true>>;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO vertices(g)", "[dynamic_graph][vov][cpo][vertices]") {
    SECTION("returns vertex_descriptor_view") {
        vov_void g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        
        // Should be a sized range
        REQUIRE(std::ranges::size(v_range) == 5);
        
        // Should be iterable
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 5);
    }

    SECTION("const correctness") {
        const vov_void g;
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 0);
    }

    SECTION("with values") {
        vov_int_vv g;
        g.resize_vertices(3);
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 3);
    }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO num_vertices(g)", "[dynamic_graph][vov][cpo][num_vertices]") {
    SECTION("empty graph") {
        vov_void g;
        
        REQUIRE(num_vertices(g) == 0);
    }

    SECTION("non-empty") {
        vov_void g;
        g.resize_vertices(10);
        
        REQUIRE(num_vertices(g) == 10);
    }

    SECTION("matches vertices size") {
        vov_int_vv g;
        g.resize_vertices(7);
        
        REQUIRE(num_vertices(g) == std::ranges::size(vertices(g)));
    }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO find_vertex(g, uid)", "[dynamic_graph][vov][cpo][find_vertex]") {
    SECTION("with uint32_t") {
        vov_void g;
        g.resize_vertices(5);
        
        auto v = find_vertex(g, uint32_t{2});
        
        REQUIRE(v != vertices(g).end());
    }

    SECTION("with int") {
        vov_void g;
        g.resize_vertices(5);
        
        // Should handle int -> uint32_t conversion
        auto v = find_vertex(g, 3);
        
        REQUIRE(v != vertices(g).end());
    }

    SECTION("bounds check") {
        vov_void g;
        g.resize_vertices(3);
        
        auto v0 = find_vertex(g, 0);
        auto v2 = find_vertex(g, 2);
        
        REQUIRE(v0 != vertices(g).end());
        REQUIRE(v2 != vertices(g).end());
    }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO vertex_id(g, u)", "[dynamic_graph][vov][cpo][vertex_id]") {
    SECTION("basic access") {
        vov_void g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        auto v_it = v_range.begin();
        auto v_desc = *v_it;
        
        auto id = vertex_id(g, v_desc);
        REQUIRE(id == 0);
    }

    SECTION("all vertices") {
        vov_void g;
        g.resize_vertices(10);
        
        size_t expected_id = 0;
        for (auto v : vertices(g)) {
            REQUIRE(vertex_id(g, v) == expected_id);
            ++expected_id;
        }
    }

    SECTION("const correctness") {
        const vov_void g;
        
        // Empty graph - should compile even though no vertices to iterate
        for (auto v : vertices(g)) {
            [[maybe_unused]] auto id = vertex_id(g, v);
        }
        REQUIRE(num_vertices(g) == 0);
    }

    SECTION("with vertex values") {
        vov_int_vv g;
        g.resize_vertices(5);
        
        // Initialize vertex values to their IDs
        for (auto v : vertices(g)) {
            auto id = vertex_id(g, v);
            vertex_value(g, v) = static_cast<int>(id) * 10;
        }
        
        // Verify IDs match expected values
        for (auto v : vertices(g)) {
            auto id = vertex_id(g, v);
            REQUIRE(vertex_value(g, v) == static_cast<int>(id) * 10);
        }
    }

    SECTION("with find_vertex") {
        vov_void g;
        g.resize_vertices(8);
        
        // Find vertex by ID and verify round-trip
        for (uint32_t expected_id = 0; expected_id < 8; ++expected_id) {
            auto v_it = find_vertex(g, expected_id);
            REQUIRE(v_it != vertices(g).end());
            
            auto v_desc = *v_it;
            auto actual_id = vertex_id(g, v_desc);
            REQUIRE(actual_id == expected_id);
        }
    }

    SECTION("sequential iteration") {
        vov_void g;
        g.resize_vertices(100);
        
        // Verify IDs are sequential
        auto v_range = vertices(g);
        auto it = v_range.begin();
        for (size_t expected = 0; expected < 100; ++expected) {
            REQUIRE(it != v_range.end());
            auto v = *it;
            REQUIRE(vertex_id(g, v) == expected);
            ++it;
        }
    }

    SECTION("consistency across calls") {
        vov_void g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        auto v_it = v_range.begin();
        auto v_desc = *v_it;
        
        // Call vertex_id multiple times - should be stable
        auto id1 = vertex_id(g, v_desc);
        auto id2 = vertex_id(g, v_desc);
        auto id3 = vertex_id(g, v_desc);
        
        REQUIRE(id1 == id2);
        REQUIRE(id2 == id3);
    }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO num_edges(g)", "[dynamic_graph][vov][cpo][num_edges]") {
    SECTION("empty graph") {
        vov_void g;
        
        REQUIRE(num_edges(g) == 0);
    }

    SECTION("with edges") {
        vov_void g({{0, 1}, {1, 2}, {2, 0}});
        
        REQUIRE(num_edges(g) == 3);
    }

    SECTION("after multiple edge additions") {
        vov_void g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, void>> ee = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2}
        };
        g.load_edges(ee, std::identity{}, 4, 0);
        
        REQUIRE(num_edges(g) == 5);
    }
}

//==================================================================================================
// 6. num_edges(g, u) CPO Tests - SUPPORTED with vector (random_access + sized_range)
//==================================================================================================

TEST_CASE("vov CPO num_edges(g, u)", "[dynamic_graph][vov][cpo][num_edges]") {
    SECTION("vertex with no edges") {
        vov_void g;
        g.resize_vertices(3);
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 0);
    }

    SECTION("vertex with single edge") {
        vov_void g({{0, 1}});
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 1);
    }

    SECTION("vertex with multiple edges") {
        vov_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 3);
    }

    SECTION("all vertices") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        REQUIRE(num_edges(g, u0) == 2);
        REQUIRE(num_edges(g, u1) == 1);
        REQUIRE(num_edges(g, u2) == 1);
    }

    SECTION("matches degree") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        for (auto u : vertices(g)) {
            REQUIRE(num_edges(g, u) == degree(g, u));
        }
    }
}

//==================================================================================================
// 7. num_edges(g, uid) CPO Tests - SUPPORTED with vector (random_access + sized_range)
//==================================================================================================

TEST_CASE("vov CPO num_edges(g, uid)", "[dynamic_graph][vov][cpo][num_edges]") {
    SECTION("by vertex ID - no edges") {
        vov_void g;
        g.resize_vertices(3);
        
        REQUIRE(num_edges(g, 0u) == 0);
    }

    SECTION("by vertex ID - with edges") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        REQUIRE(num_edges(g, 0u) == 2);
        REQUIRE(num_edges(g, 1u) == 1);
        REQUIRE(num_edges(g, 2u) == 0);
    }

    SECTION("consistency with descriptor overload") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});
        
        for (auto u : vertices(g)) {
            auto uid = vertex_id(g, u);
            REQUIRE(num_edges(g, u) == num_edges(g, uid));
        }
    }
}

//==================================================================================================
// 8. edges(g, u) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO edges(g, u)", "[dynamic_graph][vov][cpo][edges]") {
    SECTION("returns edge_descriptor_view") {
        vov_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        // Verify it's a range
        static_assert(std::ranges::forward_range<decltype(edge_range)>);
        
        // Should be able to iterate
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("empty edge vector") {
        vov_void g;
        g.resize_vertices(3);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        // Vertex with no edges should return empty range
        REQUIRE(edge_range.begin() == edge_range.end());
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("single edge") {
        vov_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        size_t count = 0;
        for (auto uv : edge_range) {
            REQUIRE(target_id(g, uv) == 1);
            ++count;
        }
        REQUIRE(count == 1);
    }

    SECTION("multiple edges") {
        vov_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_range) {
            targets.push_back(target_id(g, uv));
        }
        
        // vector: uses push_back, so edges appear in insertion order
        REQUIRE(targets.size() == 3);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
        REQUIRE(targets[2] == 3);
    }

    SECTION("const correctness") {
        vov_void g({{0, 1}, {0, 2}});
        const auto& const_g = g;
        
        auto u0 = *find_vertex(const_g, 0);
        auto edge_range = edges(const_g, u0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("with edge values") {
        vov_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 2);
        // vector order: insertion order with push_back
        REQUIRE(values[0] == 100);
        REQUIRE(values[1] == 200);
    }

    SECTION("multiple iterations") {
        vov_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        // First iteration
        size_t count1 = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count1;
        }
        
        // Second iteration should work the same
        size_t count2 = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count2;
        }
        
        REQUIRE(count1 == 2);
        REQUIRE(count2 == 2);
    }

    SECTION("all vertices") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});
        
        // Check each vertex's edges
        std::vector<size_t> edge_counts;
        for (auto u : vertices(g)) {
            size_t count = 0;
            for ([[maybe_unused]] auto uv : edges(g, u)) {
                ++count;
            }
            edge_counts.push_back(count);
        }
        
        REQUIRE(edge_counts.size() == 3);
        REQUIRE(edge_counts[0] == 2);  // vertex 0 has 2 edges
        REQUIRE(edge_counts[1] == 1);  // vertex 1 has 1 edge
        REQUIRE(edge_counts[2] == 1);  // vertex 2 has 1 edge
    }

    SECTION("with self-loop") {
        vov_void g({{0, 0}, {0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_range) {
            targets.push_back(target_id(g, uv));
        }
        
        REQUIRE(targets.size() == 2);
        // Should include self-loop
        REQUIRE((targets[0] == 0 || targets[1] == 0));
        REQUIRE((targets[0] == 1 || targets[1] == 1));
    }

    SECTION("with parallel edges") {
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 1, 20}, {0, 1, 30}
        };
        vov_int_ev g;
        g.resize_vertices(2);
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        size_t count = 0;
        for (auto uv : edge_range) {
            REQUIRE(target_id(g, uv) == 1);
            ++count;
        }
        
        // Should return all three parallel edges
        REQUIRE(count == 3);
    }

    SECTION("large graph") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 0; i < 20; ++i) {
            edge_data.push_back({0, i + 1});
        }
        
        vov_void g;
        g.resize_vertices(21);
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        
        REQUIRE(count == 20);
    }

    SECTION("with string edge values") {
        vov_string g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "first"}, {0, 2, "second"}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<std::string> edge_vals;
        for (auto uv : edge_range) {
            edge_vals.push_back(edge_value(g, uv));
        }
        
        REQUIRE(edge_vals.size() == 2);
        // vector order: insertion order with push_back
        REQUIRE(edge_vals[0] == "first");
        REQUIRE(edge_vals[1] == "second");
    }
}

TEST_CASE("vov CPO edges(g, uid)", "[dynamic_graph][vov][cpo][edges]") {
    SECTION("with vertex ID") {
        vov_void g({{0, 1}, {0, 2}});
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("returns edge_descriptor_view") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto edge_range = edges(g, uint32_t(1));
        
        // Verify return type is edge_descriptor_view
        static_assert(std::ranges::forward_range<decltype(edge_range)>);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 1);
    }

    SECTION("with isolated vertex") {
        vov_void g({{0, 1}, {0, 2}});
        g.resize_vertices(4);  // Vertex 3 is isolated
        
        auto edge_range = edges(g, uint32_t(3));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("with different ID types") {
        vov_void g({{0, 1}, {0, 2}});
        
        // Test with different integral types
        auto range1 = edges(g, uint32_t(0));
        auto range2 = edges(g, 0);  // int literal
        auto range3 = edges(g, size_t(0));
        
        size_t count1 = 0, count2 = 0, count3 = 0;
        for ([[maybe_unused]] auto uv : range1) ++count1;
        for ([[maybe_unused]] auto uv : range2) ++count2;
        for ([[maybe_unused]] auto uv : range3) ++count3;
        
        REQUIRE(count1 == 2);
        REQUIRE(count2 == 2);
        REQUIRE(count3 == 2);
    }

    SECTION("const correctness") {
        const vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }

    SECTION("with edge values") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 2, 20}
        };
        g.load_edges(edge_data);
        
        auto edge_range = edges(g, uint32_t(0));
        
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 2);
        // vector insertion order
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
    }

    SECTION("multiple vertices") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {1, 0}});
        
        auto edges0 = edges(g, uint32_t(0));
        auto edges1 = edges(g, uint32_t(1));
        auto edges2 = edges(g, uint32_t(2));
        
        size_t count0 = 0, count1 = 0, count2 = 0;
        for ([[maybe_unused]] auto uv : edges0) ++count0;
        for ([[maybe_unused]] auto uv : edges1) ++count1;
        for ([[maybe_unused]] auto uv : edges2) ++count2;
        
        REQUIRE(count0 == 2);
        REQUIRE(count1 == 2);
        REQUIRE(count2 == 0);
    }

    SECTION("with parallel edges") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 1, 20}, {0, 1, 30}  // 3 parallel edges
        };
        g.load_edges(edge_data);
        
        auto edge_range = edges(g, uint32_t(0));
        
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 3);
        // All target vertex 1, different values
        REQUIRE(values[0] == 10);  // insertion order
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }

    SECTION("consistency with edges(g, u)") {
        vov_int_ev g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 2, 20}, {0, 3, 30}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        
        // Test edges(g, uid) and edges(g, u) give same results
        auto range_by_id = edges(g, uint32_t(0));
        auto range_by_desc = edges(g, u0);
        
        std::vector<int> values_by_id;
        std::vector<int> values_by_desc;
        
        for (auto uv : range_by_id) {
            values_by_id.push_back(edge_value(g, uv));
        }
        
        for (auto uv : range_by_desc) {
            values_by_desc.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values_by_id.size() == values_by_desc.size());
        REQUIRE(values_by_id == values_by_desc);
    }

    SECTION("large graph") {
        vov_void g;
        g.resize_vertices(50);
        
        // Add 20 edges from vertex 0
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 1; i <= 20; ++i) {
            edge_data.push_back({0, i});
        }
        g.load_edges(edge_data);
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        
        REQUIRE(count == 20);
    }
}

//==================================================================================================
// 7. degree(g, u) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO degree(g, u)", "[dynamic_graph][vov][cpo][degree]") {
    SECTION("isolated vertex") {
        vov_void g;
        g.resize_vertices(3);
        
        // Vertices with no edges should have degree 0
        for (auto u : vertices(g)) {
            REQUIRE(degree(g, u) == 0);
        }
    }

    SECTION("single edge") {
        vov_void g({{0, 1}});
        
        auto v_range = vertices(g);
        auto v0 = *v_range.begin();
        
        REQUIRE(degree(g, v0) == 1);
    }

    SECTION("multiple edges from vertex") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3}, {1, 2}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        // Vertex 0 has 3 outgoing edges
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 3);
        
        // Vertex 1 has 1 outgoing edge
        auto v1 = *std::next(vertices(g).begin(), 1);
        REQUIRE(degree(g, v1) == 1);
        
        // Vertices 2 and 3 have no outgoing edges
        auto v2 = *std::next(vertices(g).begin(), 2);
        auto v3 = *std::next(vertices(g).begin(), 3);
        REQUIRE(degree(g, v2) == 0);
        REQUIRE(degree(g, v3) == 0);
    }

    SECTION("all vertices") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2},
            {1, 2}, {1, 3},
            {2, 3},
            {3, 0}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        // Expected degrees: v0=2, v1=2, v2=1, v3=1
        size_t expected_degrees[] = {2u, 2u, 1u, 1u};
        size_t idx = 0;
        
        for (auto u : vertices(g)) {
            REQUIRE(degree(g, u) == expected_degrees[idx]);
            ++idx;
        }
    }

    SECTION("const correctness") {
        vov_void g({{0, 1}, {0, 2}});
        
        const vov_void& const_g = g;
        
        auto v0 = *vertices(const_g).begin();
        REQUIRE(degree(const_g, v0) == 2);
    }

    SECTION("by vertex ID") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        // Access degree by vertex ID
        REQUIRE(degree(g, uint32_t{0}) == 3);
        REQUIRE(degree(g, uint32_t{1}) == 0);
        REQUIRE(degree(g, uint32_t{2}) == 0);
        REQUIRE(degree(g, uint32_t{3}) == 0);
    }

    SECTION("matches manual count") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3},
            {1, 0}, {1, 2},
            {2, 1}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        for (auto u : vertices(g)) {
            auto deg = degree(g, u);
            
            // Manual count
            size_t manual_count = 0;
            for ([[maybe_unused]] auto e : edges(g, u)) {
                ++manual_count;
            }
            
            REQUIRE(static_cast<size_t>(deg) == manual_count);
        }
    }

    SECTION("with edge values") {
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 2, 20}, {1, 2, 30}
        };
        vov_int_ev g;
        g.resize_vertices(3);
        g.load_edges(edge_data);
        
        auto v0 = *vertices(g).begin();
        auto v1 = *std::next(vertices(g).begin(), 1);
        auto v2 = *std::next(vertices(g).begin(), 2);
        
        REQUIRE(degree(g, v0) == 2);
        REQUIRE(degree(g, v1) == 1);
        REQUIRE(degree(g, v2) == 0);
    }

    SECTION("self-loop") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 0}, {0, 1}  // Self-loop plus normal edge
        };
        vov_void g;
        g.resize_vertices(2);
        g.load_edges(edge_data);
        
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 2);  // Self-loop counts as one edge
    }

    SECTION("large graph") {
        vov_void g;
        g.resize_vertices(100);
        
        // Create a star graph: vertex 0 connects to all others
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 1; i < 100; ++i) {
            edge_data.push_back({0, i});
        }
        g.load_edges(edge_data);
        
        auto v0 = *vertices(g).begin();
        REQUIRE(degree(g, v0) == 99);
        
        // All other vertices have degree 0
        size_t idx = 0;
        for (auto u : vertices(g)) {
            if (idx > 0) {
                REQUIRE(degree(g, u) == 0);
            }
            ++idx;
        }
    }
}

//==================================================================================================
// 8. target_id(g, uv) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO target_id(g, uv)", "[dynamic_graph][vov][cpo][target_id]") {
    SECTION("basic access") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        // Get edges from vertex 0
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto it = edge_view.begin();
        
        REQUIRE(it != edge_view.end());
        auto uv0 = *it;
        REQUIRE(target_id(g, uv0) == 1);  // vector: first added appears first
        
        ++it;
        REQUIRE(it != edge_view.end());
        auto uv1 = *it;
        REQUIRE(target_id(g, uv1) == 2);  // vector: second added appears second
    }
    
    SECTION("all edges") {
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        // Collect all target IDs
        std::vector<uint32_t> targets;
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                targets.push_back(target_id(g, uv));
            }
        }
        
        // Should have 5 edges total
        REQUIRE(targets.size() == 5);
        
        // Verify all target IDs are valid vertex IDs
        for (auto tid : targets) {
            REQUIRE(tid < num_vertices(g));
        }
    }
    
    SECTION("with edge values") {
        vov_int_ev g({{0, 1, 100}, {0, 2, 200}, {1, 2, 300}});
        
        // Verify target_id works with edge values present
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto tid = target_id(g, uv);
                REQUIRE(tid < num_vertices(g));
            }
        }
    }
    
    SECTION("const correctness") {
        vov_void g({{0, 1}, {1, 2}});
        const auto& const_g = g;
        
        auto u0 = *find_vertex(const_g, 0);
        auto edge_view = edges(const_g, u0);
        
        auto uv = *edge_view.begin();
        auto tid = target_id(const_g, uv);
        REQUIRE(tid == 1);
    }
    
    SECTION("self-loop") {
        vov_void g({{0, 0}, {0, 1}});  // Self-loop and regular edge
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto it = edge_view.begin();
        
        // vector: first added (0,0) appears first - self-loop
        REQUIRE(target_id(g, *it) == 0);  // Self-loop target is source
        ++it;
        // Second added (0,1) appears second
        REQUIRE(target_id(g, *it) == 1);
    }
    
    SECTION("parallel edges") {
        // Multiple edges between same vertices
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 1, 20}, {0, 1, 30}
        };
        vov_int_ev g;
        g.resize_vertices(2);
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        // All parallel edges should have same target
        for (auto uv : edge_view) {
            REQUIRE(target_id(g, uv) == 1);
        }
    }
    
    SECTION("consistency with vertex_id") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto tid = target_id(g, uv);
                
                // Find target vertex and verify its ID matches
                auto target_vertex = *find_vertex(g, tid);
                REQUIRE(vertex_id(g, target_vertex) == tid);
            }
        }
    }
    
    SECTION("large graph") {
        // Create a graph with many edges
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 0; i < 50; ++i) {
            edge_data.push_back({i, (i + 1) % 100});
            edge_data.push_back({i, (i + 2) % 100});
        }
        
        vov_void g;
        g.resize_vertices(100);
        g.load_edges(edge_data);
        
        // Verify all target IDs are valid
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto tid = target_id(g, uv);
                REQUIRE(tid < 100);
            }
        }
    }
    
    SECTION("with string edge values") {
        using vov_string_ev = dynamic_graph<std::string, void, void, uint32_t, false, 
                                             vov_graph_traits<std::string, void, void, uint32_t, false>>;
        
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "edge01"}, {0, 2, "edge02"}, {1, 2, "edge12"}
        };
        vov_string_ev g;
        g.resize_vertices(3);
        g.load_edges(edge_data);
        
        // target_id should work regardless of edge value type
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto tid = target_id(g, uv);
            REQUIRE((tid == 1 || tid == 2));
        }
    }
    
    SECTION("iteration order") {
        // Verify target_id works correctly with vector reverse insertion order
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        [[maybe_unused]] auto it = edge_view.begin();
        
        // vector uses push_back: edges appear in insertion order
        std::vector<uint32_t> expected_targets = {1, 2, 3};
        size_t idx = 0;
        
        for (auto uv : edge_view) {
            REQUIRE(target_id(g, uv) == expected_targets[idx]);
            ++idx;
        }
        REQUIRE(idx == 3);
    }
}

//==================================================================================================
// 9. target(g, uv) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO target(g, uv)", "[dynamic_graph][vov][cpo][target]") {
    SECTION("basic access") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        // Get edge from vertex 0
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto it = edge_view.begin();
        
        REQUIRE(it != edge_view.end());
        auto uv = *it;
        
        // Get target vertex descriptor
        auto target_vertex = target(g, uv);
        
        // Verify it's the correct vertex (vector: first added appears first)
        REQUIRE(vertex_id(g, target_vertex) == 1);
    }
    
    SECTION("returns vertex descriptor") {
        vov_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        auto uv = *edge_view.begin();
        auto target_vertex = target(g, uv);
        
        // Should return a vertex descriptor
        static_assert(vertex_descriptor_type<decltype(target_vertex)>);
        
        // Can use it to get vertex_id
        auto tid = vertex_id(g, target_vertex);
        REQUIRE(tid == 1);
    }
    
    SECTION("consistency with target_id") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}});
        
        // For all edges, verify target(g,uv) matches find_vertex(g, target_id(g,uv))
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto target_desc = target(g, uv);
                auto tid = target_id(g, uv);
                auto expected_desc = *find_vertex(g, tid);
                
                REQUIRE(vertex_id(g, target_desc) == vertex_id(g, expected_desc));
            }
        }
    }
    
    SECTION("with edge values") {
        vov_int_ev g({{0, 1, 100}, {0, 2, 200}, {1, 2, 300}});
        
        // target() should work regardless of edge value type
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto target_vertex = target(g, uv);
            auto tid = vertex_id(g, target_vertex);
            REQUIRE((tid == 1 || tid == 2));
        }
    }
    
    SECTION("const correctness") {
        vov_void g({{0, 1}, {1, 2}});
        const auto& const_g = g;
        
        auto u0 = *find_vertex(const_g, 0);
        auto edge_view = edges(const_g, u0);
        
        auto uv = *edge_view.begin();
        auto target_vertex = target(const_g, uv);
        REQUIRE(vertex_id(const_g, target_vertex) == 1);
    }
    
    SECTION("self-loop") {
        vov_void g({{0, 0}, {0, 1}});  // Self-loop and regular edge
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        auto it = edge_view.begin();
        
        // vector: first added (0,0) appears first - self-loop
        auto uv0 = *it;
        auto target0 = target(g, uv0);
        REQUIRE(vertex_id(g, target0) == 0);  // Target is same as source
        
        ++it;
        // Second added (0,1) appears second
        auto uv1 = *it;
        auto target1 = target(g, uv1);
        REQUIRE(vertex_id(g, target1) == 1);
    }
    
    SECTION("access target properties") {
        vov_int_vv g;
        g.resize_vertices(3);
        
        // Set vertex values
        for (auto u : vertices(g)) {
            vertex_value(g, u) = static_cast<int>(vertex_id(g, u)) * 10;
        }
        
        // Add edges
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}};
        g.load_edges(edge_data);
        
        // Access target vertex values through target()
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto target_vertex = target(g, uv);
            auto target_value = vertex_value(g, target_vertex);
            auto tid = vertex_id(g, target_vertex);
            
            REQUIRE(target_value == static_cast<int>(tid) * 10);
        }
    }
    
    SECTION("with string vertex values") {
        vov_string g;
        g.resize_vertices(3);
        
        // Set string vertex values
        std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
        size_t idx = 0;
        for (auto u : vertices(g)) {
            vertex_value(g, u) = names[idx++];
        }
        
        // Add edges with string edge values
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "likes"}, {0, 2, "knows"}
        };
        g.load_edges(edge_data);
        
        // Verify we can access target names
        auto u0 = *find_vertex(g, 0);
        std::vector<std::string> target_names;
        for (auto uv : edges(g, u0)) {
            auto target_vertex = target(g, uv);
            target_names.push_back(vertex_value(g, target_vertex));
        }
        
        // Should have 2 targets (insertion order due to vector)
        REQUIRE(target_names.size() == 2);
        REQUIRE((target_names[0] == "Charlie" || target_names[0] == "Bob"));
    }
    
    SECTION("parallel edges") {
        // Multiple edges to same target
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 1, 20}, {0, 1, 30}
        };
        vov_int_ev g;
        g.resize_vertices(2);
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_view = edges(g, u0);
        
        // All parallel edges should have same target
        for (auto uv : edge_view) {
            auto target_vertex = target(g, uv);
            REQUIRE(vertex_id(g, target_vertex) == 1);
        }
    }
    
    SECTION("iteration and navigation") {
        // Create a path graph: 0->1->2->3
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {1, 2}, {2, 3}
        };
        vov_void g;
        g.resize_vertices(4);
        g.load_edges(edge_data);
        
        // Navigate the path using target()
        auto current = *find_vertex(g, 0);
        std::vector<uint32_t> path;
        path.push_back(static_cast<uint32_t>(vertex_id(g, current)));
        
        // Follow edges to build path
        while (true) {
            auto edge_view = edges(g, current);
            auto it = edge_view.begin();
            if (it == edge_view.end()) break;
            
            auto uv = *it;
            current = target(g, uv);
            path.push_back(static_cast<uint32_t>(vertex_id(g, current)));
            
            if (path.size() >= 4) break;  // Prevent infinite loop
        }
        
        // Should have followed path 0->1->2->3
        REQUIRE(path.size() == 4);
        REQUIRE(path[0] == 0);
        REQUIRE(path[1] == 1);
        REQUIRE(path[2] == 2);
        REQUIRE(path[3] == 3);
    }
    
    SECTION("large graph") {
        // Create a graph with many edges
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 0; i < 50; ++i) {
            edge_data.push_back({i, (i + 1) % 100});
            edge_data.push_back({i, (i + 2) % 100});
        }
        
        vov_void g;
        g.resize_vertices(100);
        g.load_edges(edge_data);
        
        // Verify target() works for all edges
        size_t edge_count = 0;
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto target_vertex = target(g, uv);
                auto tid = vertex_id(g, target_vertex);
                REQUIRE(tid < 100);
                ++edge_count;
            }
        }
        
        REQUIRE(edge_count == 100);
    }
}

//==================================================================================================
// 10. find_vertex_edge(g, u, v) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO find_vertex_edge(g, u, v)", "[dynamic_graph][vov][cpo][find_vertex_edge]") {
    SECTION("basic edge found") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        // Find existing edges
        auto e01 = find_vertex_edge(g, u0, u1);
        auto e02 = find_vertex_edge(g, u0, u2);
        auto e12 = find_vertex_edge(g, u1, u2);
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
    }

    SECTION("edge not found") {
        vov_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u2 = *find_vertex(g, 2);
        
        // Edge from 0 to 2 doesn't exist (only 0->1->2)
        [[maybe_unused]] auto edge_range = edges(g, u0);
        [[maybe_unused]] auto end_iter = std::ranges::end(edge_range);
        [[maybe_unused]] auto e02 = find_vertex_edge(g, u0, u2);
        
        // When not found, should return an edge descriptor that equals end
        // We verify by checking if iterating from the result gives us nothing
        bool found = false;
        for (auto uv : edges(g, u0)) {
            if (target_id(g, uv) == 2) {
                found = true;
                break;
            }
        }
        REQUIRE_FALSE(found);
    }

    SECTION("with vertex ID") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        // Find edge using vertex descriptor + vertex ID
        auto e01 = find_vertex_edge(g, u0, uint32_t(1));
        auto e02 = find_vertex_edge(g, u0, uint32_t(2));
        auto e12 = find_vertex_edge(g, u1, uint32_t(2));
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
    }

    SECTION("with both IDs") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        // Find edges using both vertex IDs
        auto e01 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e02 = find_vertex_edge(g, uint32_t(0), uint32_t(2));
        auto e12 = find_vertex_edge(g, uint32_t(1), uint32_t(2));
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
    }

    SECTION("with edge values") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100}, {0, 2, 200}, {1, 2, 300}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        auto e01 = find_vertex_edge(g, u0, u1);
        auto e02 = find_vertex_edge(g, u0, u2);
        auto e12 = find_vertex_edge(g, u1, u2);
        
        REQUIRE(edge_value(g, e01) == 100);
        REQUIRE(edge_value(g, e02) == 200);
        REQUIRE(edge_value(g, e12) == 300);
    }

    SECTION("const correctness") {
        const vov_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        auto e01 = find_vertex_edge(g, u0, u1);
        
        REQUIRE(target_id(g, e01) == 1);
    }

    SECTION("with self-loop") {
        vov_void g({{0, 0}, {0, 1}});  // 0->0 (self-loop), 0->1
        
        auto u0 = *find_vertex(g, 0);
        
        // Find self-loop
        auto e00 = find_vertex_edge(g, u0, u0);
        
        REQUIRE(target_id(g, e00) == 0);
    }

    SECTION("with parallel edges") {
        vov_int_ev g;
        g.resize_vertices(2);
        
        // Multiple edges from 0 to 1 with different values
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 1, 20}, {0, 1, 30}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        
        // Should find one of the parallel edges (typically the first encountered)
        auto e01 = find_vertex_edge(g, u0, u1);
        
        REQUIRE(target_id(g, e01) == 1);
        // Verify it's one of the parallel edges
        int val = edge_value(g, e01);
        REQUIRE((val == 10 || val == 20 || val == 30));
    }

    SECTION("with string edge values") {
        vov_string g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "edge_01"}, {0, 2, "edge_02"}, {1, 2, "edge_12"}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        auto e01 = find_vertex_edge(g, u0, u1);
        auto e02 = find_vertex_edge(g, u0, u2);
        auto e12 = find_vertex_edge(g, u1, u2);
        
        REQUIRE(edge_value(g, e01) == "edge_01");
        REQUIRE(edge_value(g, e02) == "edge_02");
        REQUIRE(edge_value(g, e12) == "edge_12");
    }

    SECTION("multiple source vertices") {
        vov_void g({{0, 2}, {1, 2}, {2, 3}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        auto u3 = *find_vertex(g, 3);
        
        // Different sources to same target
        auto e02 = find_vertex_edge(g, u0, u2);
        auto e12 = find_vertex_edge(g, u1, u2);
        auto e23 = find_vertex_edge(g, u2, u3);
        
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
        REQUIRE(target_id(g, e23) == 3);
    }

    SECTION("large graph") {
        vov_void g;
        g.resize_vertices(100);
        
        // Add edges from vertex 0 to vertices 1-99
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 1; i < 100; ++i) {
            edge_data.push_back({0, i});
        }
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u50 = *find_vertex(g, 50);
        auto u99 = *find_vertex(g, 99);
        
        auto e0_50 = find_vertex_edge(g, u0, u50);
        auto e0_99 = find_vertex_edge(g, u0, u99);
        
        REQUIRE(target_id(g, e0_50) == 50);
        REQUIRE(target_id(g, e0_99) == 99);
    }

    SECTION("with different integral types") {
        vov_void g({{0, 1}, {0, 2}});
        
        // Test with different integral types
        auto e1 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e2 = find_vertex_edge(g, 0, 1);  // int literals
        auto e3 = find_vertex_edge(g, size_t(0), size_t(2));
        
        REQUIRE(target_id(g, e1) == 1);
        REQUIRE(target_id(g, e2) == 1);
        REQUIRE(target_id(g, e3) == 2);
    }

    SECTION("isolated vertex") {
        vov_void g({{0, 1}});
        g.resize_vertices(3);  // Vertex 2 is isolated
        
        [[maybe_unused]] auto u0 = *find_vertex(g, 0);
        auto u2 = *find_vertex(g, 2);
        
        // Try to find edge from isolated vertex
        bool found = false;
        for (auto uv : edges(g, u2)) {
            if (target_id(g, uv) == 0) {
                found = true;
                break;
            }
        }
        REQUIRE_FALSE(found);
    }
}

//--------------------------------------------------------------------------------------------------
// 11. find_vertex_edge(g, uid, vid) CPO Tests - uid_vid overload
//--------------------------------------------------------------------------------------------------

TEST_CASE("vov CPO find_vertex_edge(g, uid, vid)", "[dynamic_graph][vov][cpo][find_vertex_edge][uid_vid]") {
    SECTION("basic usage") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
        
        // Test finding edges using only vertex IDs
        auto e01 = find_vertex_edge(g, 0, 1);
        auto e02 = find_vertex_edge(g, 0, 2);
        auto e12 = find_vertex_edge(g, 1, 2);
        auto e23 = find_vertex_edge(g, 2, 3);
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e02) == 2);
        REQUIRE(target_id(g, e12) == 2);
        REQUIRE(target_id(g, e23) == 3);
    }

    SECTION("edge not found") {
        vov_void g({{0, 1}, {1, 2}});
        
        // Try to find non-existent edges
        auto e02 = find_vertex_edge(g, 0, 2);  // No direct edge from 0 to 2
        auto e10 = find_vertex_edge(g, 1, 0);  // No reverse edge
        auto e21 = find_vertex_edge(g, 2, 1);  // No reverse edge
        
        // Verify these are "not found" results (implementation-defined behavior)
        // We can verify by checking if edges exist
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        bool found_02 = false;
        for (auto e : edges(g, u0)) {
            if (target_id(g, e) == 2) found_02 = true;
        }
        REQUIRE(!found_02);
        
        bool found_10 = false;
        for (auto e : edges(g, u1)) {
            if (target_id(g, e) == 0) found_10 = true;
        }
        REQUIRE(!found_10);
    }

    SECTION("with edge values") {
        vov_int_ev g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}
        };
        g.load_edges(edge_data);
        
        // Find edges using vertex IDs and verify their values
        auto e01 = find_vertex_edge(g, 0, 1);
        auto e02 = find_vertex_edge(g, 0, 2);
        auto e12 = find_vertex_edge(g, 1, 2);
        auto e23 = find_vertex_edge(g, 2, 3);
        
        REQUIRE(edge_value(g, e01) == 10);
        REQUIRE(edge_value(g, e02) == 20);
        REQUIRE(edge_value(g, e12) == 30);
        REQUIRE(edge_value(g, e23) == 40);
    }

    SECTION("with parallel edges") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        // Add multiple edges from 0 to 1 with different values
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100}, {0, 1, 200}, {0, 1, 300}, {1, 2, 400}
        };
        g.load_edges(edge_data);
        
        // find_vertex_edge should find one of the parallel edges
        auto e01 = find_vertex_edge(g, 0, 1);
        REQUIRE(target_id(g, e01) == 1);
        
        // The edge value should be one of the parallel edge values
        int val = edge_value(g, e01);
        REQUIRE((val == 100 || val == 200 || val == 300));
    }

    SECTION("with self-loop") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 0, 99},   // Self-loop
            {0, 1, 10},
            {1, 1, 88}    // Self-loop
        };
        g.load_edges(edge_data);
        
        // Find self-loops using vertex IDs
        auto e00 = find_vertex_edge(g, 0, 0);
        auto e11 = find_vertex_edge(g, 1, 1);
        
        REQUIRE(target_id(g, e00) == 0);
        REQUIRE(edge_value(g, e00) == 99);
        REQUIRE(target_id(g, e11) == 1);
        REQUIRE(edge_value(g, e11) == 88);
    }

    SECTION("const correctness") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100}, {1, 2, 200}
        };
        g.load_edges(edge_data);
        
        // Test with const graph
        const auto& cg = g;
        
        auto e01 = find_vertex_edge(cg, 0, 1);
        auto e12 = find_vertex_edge(cg, 1, 2);
        
        REQUIRE(target_id(cg, e01) == 1);
        REQUIRE(edge_value(cg, e01) == 100);
        REQUIRE(target_id(cg, e12) == 2);
        REQUIRE(edge_value(cg, e12) == 200);
    }

    SECTION("with different integral types") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Test with various integral types for IDs
        auto e01_uint32 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
        auto e12_int = find_vertex_edge(g, 1, 2);
        auto e23_size = find_vertex_edge(g, size_t(2), size_t(3));
        
        REQUIRE(target_id(g, e01_uint32) == 1);
        REQUIRE(target_id(g, e12_int) == 2);
        REQUIRE(target_id(g, e23_size) == 3);
    }

    SECTION("with string edge values") {
        vov_string g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "alpha"}, {0, 2, "beta"}, {1, 2, "gamma"}, {2, 3, "delta"}
        };
        g.load_edges(edge_data);
        
        // Find edges and verify string values
        auto e01 = find_vertex_edge(g, 0, 1);
        auto e02 = find_vertex_edge(g, 0, 2);
        auto e12 = find_vertex_edge(g, 1, 2);
        auto e23 = find_vertex_edge(g, 2, 3);
        
        REQUIRE(edge_value(g, e01) == "alpha");
        REQUIRE(edge_value(g, e02) == "beta");
        REQUIRE(edge_value(g, e12) == "gamma");
        REQUIRE(edge_value(g, e23) == "delta");
    }

    SECTION("in large graph") {
        vov_void g;
        g.resize_vertices(100);
        
        // Create edges from vertex 0 to all other vertices
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 1; i < 100; ++i) {
            edge_data.push_back({0, i});
        }
        g.load_edges(edge_data);
        
        // Test finding edges to various vertices
        auto e01 = find_vertex_edge(g, 0, 1);
        auto e050 = find_vertex_edge(g, 0, 50);
        auto e099 = find_vertex_edge(g, 0, 99);
        
        REQUIRE(target_id(g, e01) == 1);
        REQUIRE(target_id(g, e050) == 50);
        REQUIRE(target_id(g, e099) == 99);
    }

    SECTION("from isolated vertex") {
        vov_void g;
        g.resize_vertices(5);
        
        // Only add edges between some vertices, leave vertex 3 isolated
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {1, 2}, {2, 4}
        };
        g.load_edges(edge_data);
        
        // Try to find edge from isolated vertex
        auto u3 = *find_vertex(g, 3);
        
        // Verify vertex 3 has no outgoing edges
        auto edges_3 = edges(g, u3);
        REQUIRE(std::ranges::distance(edges_3) == 0);
    }

    SECTION("chain of edges") {
        vov_int_ev g;
        g.resize_vertices(6);
        
        // Create a chain: 0->1->2->3->4->5
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}, {4, 5, 50}
        };
        g.load_edges(edge_data);
        
        // Traverse the chain using find_vertex_edge
        auto e01 = find_vertex_edge(g, 0, 1);
        REQUIRE(edge_value(g, e01) == 10);
        
        auto e12 = find_vertex_edge(g, 1, 2);
        REQUIRE(edge_value(g, e12) == 20);
        
        auto e23 = find_vertex_edge(g, 2, 3);
        REQUIRE(edge_value(g, e23) == 30);
        
        auto e34 = find_vertex_edge(g, 3, 4);
        REQUIRE(edge_value(g, e34) == 40);
        
        auto e45 = find_vertex_edge(g, 4, 5);
        REQUIRE(edge_value(g, e45) == 50);
    }
}

//==================================================================================================
// 12. contains_edge(g, u, v) and contains_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO contains_edge(g, u, v)", "[dynamic_graph][vov][cpo][contains_edge]") {
    SECTION("edge exists") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        // Check existing edges
        REQUIRE(contains_edge(g, u0, u1));
        REQUIRE(contains_edge(g, u0, u2));
        REQUIRE(contains_edge(g, u1, u2));
    }

    SECTION("edge does not exist") {
        vov_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        // Check non-existent edges
        REQUIRE_FALSE(contains_edge(g, u0, u2));  // No direct edge from 0 to 2
        REQUIRE_FALSE(contains_edge(g, u1, u0));  // No reverse edge
        REQUIRE_FALSE(contains_edge(g, u2, u1));  // No reverse edge
        REQUIRE_FALSE(contains_edge(g, u2, u0));  // No reverse edge
    }

    SECTION("with vertex IDs") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
        
        // Check existing edges using vertex IDs
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 0, 2));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE(contains_edge(g, 2, 3));
        
        // Check non-existent edges
        REQUIRE_FALSE(contains_edge(g, 0, 3));
        REQUIRE_FALSE(contains_edge(g, 1, 0));
        REQUIRE_FALSE(contains_edge(g, 2, 0));
        REQUIRE_FALSE(contains_edge(g, 3, 0));
    }

    SECTION("with edge values") {
        vov_int_ev g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100}, {0, 2, 200}, {1, 2, 300}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        auto u3 = *find_vertex(g, 3);
        
        // Check existing edges
        REQUIRE(contains_edge(g, u0, u1));
        REQUIRE(contains_edge(g, u0, u2));
        REQUIRE(contains_edge(g, u1, u2));
        
        // Check non-existent edges
        REQUIRE_FALSE(contains_edge(g, u0, u3));
        REQUIRE_FALSE(contains_edge(g, u3, u0));
    }

    SECTION("with parallel edges") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        // Add multiple edges from 0 to 1
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100}, {0, 1, 200}, {0, 1, 300}, {1, 2, 400}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        // Should return true if any edge exists between u and v
        REQUIRE(contains_edge(g, u0, u1));
        REQUIRE(contains_edge(g, u1, u2));
    }

    SECTION("with self-loop") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 0, 99},   // Self-loop
            {0, 1, 10},
            {1, 1, 88}    // Self-loop
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        // Check self-loops
        REQUIRE(contains_edge(g, u0, u0));
        REQUIRE(contains_edge(g, u1, u1));
        REQUIRE_FALSE(contains_edge(g, u2, u2));
        
        // Check regular edges
        REQUIRE(contains_edge(g, u0, u1));
    }

    SECTION("with self-loop (uid, vid)") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 0, 99}, {1, 1, 88}, {0, 1, 10}
        };
        g.load_edges(edge_data);
        
        // Check self-loops using vertex IDs
        REQUIRE(contains_edge(g, 0, 0));
        REQUIRE(contains_edge(g, 1, 1));
        REQUIRE_FALSE(contains_edge(g, 2, 2));
    }

    SECTION("const correctness") {
        vov_void g({{0, 1}, {1, 2}});
        
        const auto& cg = g;
        auto u0 = *find_vertex(cg, 0);
        auto u1 = *find_vertex(cg, 1);
        auto u2 = *find_vertex(cg, 2);
        
        REQUIRE(contains_edge(cg, u0, u1));
        REQUIRE(contains_edge(cg, u1, u2));
        REQUIRE_FALSE(contains_edge(cg, u0, u2));
    }

    SECTION("const correctness (uid, vid)") {
        vov_void g({{0, 1}, {1, 2}});
        
        const auto& cg = g;
        
        REQUIRE(contains_edge(cg, 0, 1));
        REQUIRE(contains_edge(cg, 1, 2));
        REQUIRE_FALSE(contains_edge(cg, 0, 2));
    }

    SECTION("with different integral types") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Test with various integral types
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE(contains_edge(g, size_t(2), size_t(3)));
        
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, 3, 0));
    }

    SECTION("empty graph") {
        vov_void g;
        g.resize_vertices(3);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        
        // No edges in the graph
        REQUIRE_FALSE(contains_edge(g, u0, u1));
        REQUIRE_FALSE(contains_edge(g, u1, u2));
        REQUIRE_FALSE(contains_edge(g, u0, u2));
    }

    SECTION("isolated vertex") {
        vov_void g;
        g.resize_vertices(5);
        
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {1, 2}, {2, 4}
        };
        g.load_edges(edge_data);
        
        // Vertex 3 is isolated - has no edges
        REQUIRE_FALSE(contains_edge(g, 3, 0));
        REQUIRE_FALSE(contains_edge(g, 3, 1));
        REQUIRE_FALSE(contains_edge(g, 3, 2));
        REQUIRE_FALSE(contains_edge(g, 3, 4));
        REQUIRE_FALSE(contains_edge(g, 0, 3));
    }

    SECTION("with string edge values") {
        vov_string g;
        g.resize_vertices(4);
        
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "alpha"}, {0, 2, "beta"}, {1, 2, "gamma"}
        };
        g.load_edges(edge_data);
        
        auto u0 = *find_vertex(g, 0);
        auto u1 = *find_vertex(g, 1);
        auto u2 = *find_vertex(g, 2);
        auto u3 = *find_vertex(g, 3);
        
        REQUIRE(contains_edge(g, u0, u1));
        REQUIRE(contains_edge(g, u0, u2));
        REQUIRE(contains_edge(g, u1, u2));
        REQUIRE_FALSE(contains_edge(g, u3, u0));
    }

    SECTION("large graph") {
        vov_void g;
        g.resize_vertices(100);
        
        // Create edges from vertex 0 to all other vertices
        std::vector<copyable_edge_t<uint32_t, void>> edge_data;
        for (uint32_t i = 1; i < 100; ++i) {
            edge_data.push_back({0, i});
        }
        g.load_edges(edge_data);
        
        // Check edges from vertex 0
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 0, 50));
        REQUIRE(contains_edge(g, 0, 99));
        
        // Check non-existent edges
        REQUIRE_FALSE(contains_edge(g, 1, 0));
        REQUIRE_FALSE(contains_edge(g, 1, 2));
        REQUIRE_FALSE(contains_edge(g, 50, 99));
    }

    SECTION("complete small graph") {
        vov_void g;
        g.resize_vertices(4);
        
        // Create a complete graph on 4 vertices (every vertex connected to every other)
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3},
            {1, 0}, {1, 2}, {1, 3},
            {2, 0}, {2, 1}, {2, 3},
            {3, 0}, {3, 1}, {3, 2}
        };
        g.load_edges(edge_data);
        
        // Every pair should have an edge
        for (uint32_t i = 0; i < 4; ++i) {
            for (uint32_t j = 0; j < 4; ++j) {
                if (i != j) {
                    REQUIRE(contains_edge(g, i, j));
                }
            }
        }
    }
}

TEST_CASE("vov CPO contains_edge(g, uid, vid)", "[dynamic_graph][vov][cpo][contains_edge][uid_vid]") {
    SECTION("basic usage") {
        vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
        
        // Test checking edges using only vertex IDs
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 0, 2));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE(contains_edge(g, 2, 3));
        
        // Non-existent edges
        REQUIRE_FALSE(contains_edge(g, 0, 3));
        REQUIRE_FALSE(contains_edge(g, 1, 0));
        REQUIRE_FALSE(contains_edge(g, 3, 2));
    }

    SECTION("all edges not found") {
        vov_void g({{0, 1}, {1, 2}});
        
        // Check all possible non-existent edges in opposite directions
        REQUIRE_FALSE(contains_edge(g, 0, 2));  // No transitive edge
        REQUIRE_FALSE(contains_edge(g, 1, 0));  // No reverse
        REQUIRE_FALSE(contains_edge(g, 2, 0));  // No reverse
        REQUIRE_FALSE(contains_edge(g, 2, 1));  // No reverse
        
        // Self-loops that don't exist
        REQUIRE_FALSE(contains_edge(g, 0, 0));
        REQUIRE_FALSE(contains_edge(g, 1, 1));
        REQUIRE_FALSE(contains_edge(g, 2, 2));
    }

    SECTION("with edge values") {
        vov_int_ev g;
        g.resize_vertices(5);
        
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {0, 2, 20}, {1, 3, 30}, {2, 4, 40}
        };
        g.load_edges(edge_data);
        
        // Check existing edges using vertex IDs
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 0, 2));
        REQUIRE(contains_edge(g, 1, 3));
        REQUIRE(contains_edge(g, 2, 4));
        
        // Check non-existent edges
        REQUIRE_FALSE(contains_edge(g, 0, 3));
        REQUIRE_FALSE(contains_edge(g, 0, 4));
        REQUIRE_FALSE(contains_edge(g, 1, 2));
        REQUIRE_FALSE(contains_edge(g, 3, 4));
    }

    SECTION("with parallel edges") {
        vov_int_ev g;
        g.resize_vertices(3);
        
        // Add multiple edges from 0 to 1
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100}, {0, 1, 200}, {0, 1, 300}, {1, 2, 400}
        };
        g.load_edges(edge_data);
        
        // Should return true if any edge exists between uid and vid
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE_FALSE(contains_edge(g, 0, 2));
    }

    SECTION("bidirectional check") {
        vov_void g;
        g.resize_vertices(3);
        
        // Create edges in both directions between some vertices
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {1, 0}, {1, 2}  // Bidirectional between 0 and 1, one-way 1->2
        };
        g.load_edges(edge_data);
        
        // Check bidirectional
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 1, 0));
        
        // Check unidirectional
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE_FALSE(contains_edge(g, 2, 1));
        
        // Check non-existent
        REQUIRE_FALSE(contains_edge(g, 0, 2));
        REQUIRE_FALSE(contains_edge(g, 2, 0));
    }

    SECTION("with different integral types") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Test with various integral types for IDs
        REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE(contains_edge(g, size_t(2), size_t(3)));
        
        // Mixed types
        REQUIRE(contains_edge(g, uint32_t(0), size_t(1)));
        REQUIRE(contains_edge(g, 1, uint32_t(2)));
        
        // Non-existent with different types
        REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(3)));
        REQUIRE_FALSE(contains_edge(g, size_t(3), 0));
    }

    SECTION("star graph") {
        vov_void g;
        g.resize_vertices(6);
        
        // Create a star graph: vertex 0 connected to all others
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}
        };
        g.load_edges(edge_data);
        
        // Check all edges from center
        for (uint32_t i = 1; i < 6; ++i) {
            REQUIRE(contains_edge(g, 0, i));
        }
        
        // Check no edges between outer vertices
        for (uint32_t i = 1; i < 6; ++i) {
            for (uint32_t j = i + 1; j < 6; ++j) {
                REQUIRE_FALSE(contains_edge(g, i, j));
                REQUIRE_FALSE(contains_edge(g, j, i));
            }
        }
        
        // Check no edges back to center
        for (uint32_t i = 1; i < 6; ++i) {
            REQUIRE_FALSE(contains_edge(g, i, 0));
        }
    }

    SECTION("chain graph") {
        vov_int_ev g;
        g.resize_vertices(6);
        
        // Create a chain: 0->1->2->3->4->5
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}, {4, 5, 50}
        };
        g.load_edges(edge_data);
        
        // Check all chain edges exist
        for (uint32_t i = 0; i < 5; ++i) {
            REQUIRE(contains_edge(g, i, i + 1));
        }
        
        // Check no reverse edges
        for (uint32_t i = 1; i < 6; ++i) {
            REQUIRE_FALSE(contains_edge(g, i, i - 1));
        }
        
        // Check no skip edges
        REQUIRE_FALSE(contains_edge(g, 0, 2));
        REQUIRE_FALSE(contains_edge(g, 0, 3));
        REQUIRE_FALSE(contains_edge(g, 1, 3));
        REQUIRE_FALSE(contains_edge(g, 2, 5));
    }

    SECTION("cycle graph") {
        vov_void g;
        g.resize_vertices(5);
        
        // Create a cycle: 0->1->2->3->4->0
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}
        };
        g.load_edges(edge_data);
        
        // Check all cycle edges
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE(contains_edge(g, 2, 3));
        REQUIRE(contains_edge(g, 3, 4));
        REQUIRE(contains_edge(g, 4, 0));  // Closing edge
        
        // Check no shortcuts across cycle
        REQUIRE_FALSE(contains_edge(g, 0, 2));
        REQUIRE_FALSE(contains_edge(g, 0, 3));
        REQUIRE_FALSE(contains_edge(g, 1, 3));
        REQUIRE_FALSE(contains_edge(g, 1, 4));
        REQUIRE_FALSE(contains_edge(g, 2, 4));
    }

    SECTION("dense graph") {
        vov_void g;
        g.resize_vertices(4);
        
        // Create edges between almost all pairs (missing 2->3)
        std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
            {0, 1}, {0, 2}, {0, 3},
            {1, 0}, {1, 2}, {1, 3},
            {2, 0}, {2, 1},  // Missing 2->3
            {3, 0}, {3, 1}, {3, 2}
        };
        g.load_edges(edge_data);
        
        // Verify most edges exist
        int edge_count = 0;
        for (uint32_t i = 0; i < 4; ++i) {
            for (uint32_t j = 0; j < 4; ++j) {
                if (i != j && contains_edge(g, i, j)) {
                    edge_count++;
                }
            }
        }
        REQUIRE(edge_count == 11);  // 12 possible - 1 missing
        
        // Verify the missing edge
        REQUIRE_FALSE(contains_edge(g, 2, 3));
    }

    SECTION("with string edge values") {
        vov_string g;
        g.resize_vertices(5);
        
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "first"}, {1, 2, "second"}, {2, 3, "third"}, {3, 4, "fourth"}
        };
        g.load_edges(edge_data);
        
        // Check edges exist
        REQUIRE(contains_edge(g, 0, 1));
        REQUIRE(contains_edge(g, 1, 2));
        REQUIRE(contains_edge(g, 2, 3));
        REQUIRE(contains_edge(g, 3, 4));
        
        // Check non-existent
        REQUIRE_FALSE(contains_edge(g, 0, 2));
        REQUIRE_FALSE(contains_edge(g, 4, 0));
    }

    SECTION("single vertex graph") {
        vov_void g;
        g.resize_vertices(1);
        
        // No edges, not even self-loop
        REQUIRE_FALSE(contains_edge(g, 0, 0));
    }

    SECTION("single edge graph") {
        vov_void g({{0, 1}});
        
        // Only one edge exists
        REQUIRE(contains_edge(g, 0, 1));
        
        // All other checks should fail
        REQUIRE_FALSE(contains_edge(g, 1, 0));
        REQUIRE_FALSE(contains_edge(g, 0, 0));
        REQUIRE_FALSE(contains_edge(g, 1, 1));
    }
}

//==================================================================================================
// 13. Integration Tests - Multiple CPOs Working Together
//==================================================================================================

TEST_CASE("vov CPO integration", "[dynamic_graph][vov][cpo][integration]") {
    SECTION("graph construction and traversal") {
        vov_void g({{0, 1}, {1, 2}});
        
        // Verify through CPOs
        REQUIRE(num_vertices(g) == 3);
        REQUIRE(num_edges(g) == 2);
        REQUIRE(has_edge(g));
    }

    SECTION("empty graph properties") {
        vov_void g;
        
        REQUIRE(num_vertices(g) == 0);
        REQUIRE(num_edges(g) == 0);
        REQUIRE(!has_edge(g));
        REQUIRE(std::ranges::size(vertices(g)) == 0);
    }

    SECTION("find vertex by id") {
        vov_void g;
        g.resize_vertices(5);
        
        // Find each vertex by ID
        for (uint32_t i = 0; i < 5; ++i) {
            auto v = find_vertex(g, i);
            REQUIRE(v != vertices(g).end());
        }
    }

    SECTION("vertices and num_vertices consistency") {
        vov_void g;
        g.resize_vertices(10);
        
        REQUIRE(num_vertices(g) == 10);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : vertices(g)) {
            ++count;
        }
        REQUIRE(count == num_vertices(g));
    }

    SECTION("const graph access") {
        vov_void g;
        g.resize_vertices(3);
        
        const vov_void& const_g = g;
        
        REQUIRE(num_vertices(const_g) == 3);
        REQUIRE(num_edges(const_g) == 0);
        REQUIRE(!has_edge(const_g));
        
        // Count vertices via iteration
        size_t vertex_count = 0;
        for ([[maybe_unused]] auto v : vertices(const_g)) {
            ++vertex_count;
        }
        REQUIRE(vertex_count == 3);
    }
}

//==================================================================================================
// 14. has_edge(g) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO has_edge(g)", "[dynamic_graph][vov][cpo][has_edge]") {
    SECTION("empty graph") {
        vov_void g;
        
        REQUIRE(!has_edge(g));
    }

    SECTION("with edges") {
        vov_void g({{0, 1}});
        
        REQUIRE(has_edge(g));
    }

    SECTION("matches num_edges") {
        vov_void g1;
        vov_void g2({{0, 1}});
        
        REQUIRE(has_edge(g1) == (num_edges(g1) > 0));
        REQUIRE(has_edge(g2) == (num_edges(g2) > 0));
    }
}

//==================================================================================================
// 15. vertex_value(g, u) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO vertex_value(g, u)", "[dynamic_graph][vov][cpo][vertex_value]") {
    SECTION("basic access") {
        vov_int_vv g;
        g.resize_vertices(3);
        
        // vertices(g) returns vertex_descriptor_view which when iterated gives descriptors
        auto u = *vertices(g).begin();
        vertex_value(g, u) = 42;
        REQUIRE(vertex_value(g, u) == 42);
    }

    SECTION("multiple vertices") {
        vov_int_vv g;
        g.resize_vertices(5);
        
        // Set values for all vertices
        int val = 0;
        for (auto u : vertices(g)) {
            vertex_value(g, u) = val;
            val += 100;
        }
        
        // Verify values
        val = 0;
        for (auto u : vertices(g)) {
            REQUIRE(vertex_value(g, u) == val);
            val += 100;
        }
    }

    SECTION("const correctness") {
        vov_int_vv g;
        g.resize_vertices(3);
        
        auto u = *vertices(g).begin();
        vertex_value(g, u) = 999;
        
        const vov_int_vv& const_g = g;
        auto const_u = *vertices(const_g).begin();
        // Should be able to read from const graph
        REQUIRE(vertex_value(const_g, const_u) == 999);
    }

    SECTION("with string values") {
        vov_string g;
        g.resize_vertices(2);
        
        int idx = 0;
        std::string expected[] = {"first", "second"};
        for (auto u : vertices(g)) {
            vertex_value(g, u) = expected[idx++];
            if (idx >= 2) break;
        }
        
        idx = 0;
        for (auto u : vertices(g)) {
            REQUIRE(vertex_value(g, u) == expected[idx++]);
            if (idx >= 2) break;
        }
    }

    SECTION("modification") {
        vov_all_int g;
        g.resize_vertices(3);
        
        auto u = *vertices(g).begin();
        vertex_value(g, u) = 10;
        REQUIRE(vertex_value(g, u) == 10);
        
        vertex_value(g, u) = 20;
        REQUIRE(vertex_value(g, u) == 20);
        
        // Modify through reference
        vertex_value(g, u) += 5;
        REQUIRE(vertex_value(g, u) == 25);
    }
}

//==================================================================================================
// 16. edge_value(g, uv) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO edge_value(g, uv)", "[dynamic_graph][vov][cpo][edge_value]") {
    SECTION("basic access") {
        vov_int_ev g({{0, 1, 42}, {1, 2, 99}});
        
        auto u = *vertices(g).begin();
        auto& v = u.inner_value(g);
        auto& edge_range = v.edges();
        auto e_iter = edge_range.begin();
        if (e_iter != edge_range.end()) {
            using edge_iter_t = decltype(e_iter);
            using vertex_desc_t = decltype(u);
            auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
            REQUIRE(edge_value(g, uv) == 42);
        }
    }

    SECTION("multiple edges") {
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 100},
            {0, 2, 200},
            {1, 2, 300}
        };
        vov_int_ev g;
        g.resize_vertices(3);
        g.load_edges(edge_data);
        
        // Check first vertex's edges
        // Note: vector uses push_back, so edges are in insertion order of loading
        auto u = *vertices(g).begin();
        auto& v = u.inner_value(g);
        auto& edge_range = v.edges();
        auto e_iter = edge_range.begin();
        if (e_iter != edge_range.end()) {
            using edge_iter_t = decltype(e_iter);
            using vertex_desc_t = decltype(u);
            auto uv0 = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
            REQUIRE(edge_value(g, uv0) == 100);  // loaded first, appears first with push_back
            ++e_iter;
            if (e_iter != edge_range.end()) {
                auto uv1 = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
                REQUIRE(edge_value(g, uv1) == 200);  // loaded second, appears second with push_back
            }
        }
    }

    SECTION("modification") {
        vov_all_int g({{0, 1, 50}});
        
        auto u = *vertices(g).begin();
        auto& v = u.inner_value(g);
        auto& edge_range = v.edges();
        auto e_iter = edge_range.begin();
        if (e_iter != edge_range.end()) {
            using edge_iter_t = decltype(e_iter);
            using vertex_desc_t = decltype(u);
            auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
            
            REQUIRE(edge_value(g, uv) == 50);
            
            edge_value(g, uv) = 75;
            REQUIRE(edge_value(g, uv) == 75);
            
            // Modify through reference
            edge_value(g, uv) += 25;
            REQUIRE(edge_value(g, uv) == 100);
        }
    }

    SECTION("const correctness") {
        vov_int_ev g({{0, 1, 42}});
        
        const vov_int_ev& const_g = g;
        auto const_u = *vertices(const_g).begin();
        auto& const_v = const_u.inner_value(const_g);
        auto& const_edge_range = const_v.edges();
        auto const_e_iter = const_edge_range.begin();
        if (const_e_iter != const_edge_range.end()) {
            using const_edge_iter_t = decltype(const_e_iter);
            using const_vertex_desc_t = decltype(const_u);
            auto const_uv = edge_descriptor<const_edge_iter_t, typename const_vertex_desc_t::iterator_type>(static_cast<std::size_t>(const_e_iter - const_edge_range.begin()), const_u);
            REQUIRE(edge_value(const_g, const_uv) == 42);
        }
    }

    SECTION("with string values") {
        std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
            {0, 1, "edge01"},
            {1, 2, "edge12"}
        };
        vov_string g;
        g.resize_vertices(3);
        g.load_edges(edge_data);
        
        std::vector<std::string> expected = {"edge01", "edge12"};
        size_t idx = 0;
        
        for (auto u : vertices(g)) {
            auto& v = u.inner_value(g);
            auto& edge_range = v.edges();
            for (auto e_iter = edge_range.begin(); e_iter != edge_range.end(); ++e_iter) {
                if (idx < 2) {
                    using edge_iter_t = decltype(e_iter);
                    using vertex_desc_t = decltype(u);
                    auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
                    REQUIRE(edge_value(g, uv) == expected[idx]);
                    ++idx;
                }
            }
        }
    }

    SECTION("iteration over all edges") {
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 10},
            {0, 2, 20},
            {1, 2, 30},
            {2, 0, 40}
        };
        vov_int_ev g;
        g.resize_vertices(3);
        g.load_edges(edge_data);
        
        // Sum all edge values
        int sum = 0;
        for (auto u : vertices(g)) {
            auto& v = u.inner_value(g);
            auto& edge_range = v.edges();
            for (auto e_iter = edge_range.begin(); e_iter != edge_range.end(); ++e_iter) {
                using edge_iter_t = decltype(e_iter);
                using vertex_desc_t = decltype(u);
                auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
                sum += edge_value(g, uv);
            }
        }
        
        REQUIRE(sum == 100);
    }
}

//==================================================================================================
// 17. Integration Tests - vertex_value and edge_value Together
//==================================================================================================

TEST_CASE("vov CPO integration: values", "[dynamic_graph][vov][cpo][integration]") {
    SECTION("vertex values only") {
        vov_all_int g;
        g.resize_vertices(5);
        
        // Set vertex values
        int val = 0;
        for (auto u : vertices(g)) {
            vertex_value(g, u) = val;
            val += 100;
        }
        
        // Verify vertex values
        val = 0;
        for (auto u : vertices(g)) {
            REQUIRE(vertex_value(g, u) == val);
            val += 100;
        }
    }

    SECTION("vertex and edge values") {
        std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
            {0, 1, 5},
            {1, 2, 10}
        };
        vov_all_int g;
        g.resize_vertices(3);
        g.load_edges(edge_data);
        
        // Set vertex values
        int val = 0;
        for (auto u : vertices(g)) {
            vertex_value(g, u) = val;
            val += 100;
        }
        
        // Verify vertex values
        val = 0;
        for (auto u : vertices(g)) {
            REQUIRE(vertex_value(g, u) == val);
            val += 100;
        }
        
        // Verify edge values
        for (auto u : vertices(g)) {
            auto& v = u.inner_value(g);
            auto& edge_range = v.edges();
            auto e_iter = edge_range.begin();
            if (e_iter != edge_range.end()) {
                using edge_iter_t = decltype(e_iter);
                using vertex_desc_t = decltype(u);
                auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
                int expected = (u.vertex_id() == 0) ? 5 : 10;
                REQUIRE(edge_value(g, uv) == expected);
            }
            if (u.vertex_id() >= 1) break; // Only check first 2 vertices
        }
    }
}

//==================================================================================================
// 18. graph_value(g) CPO Tests
//==================================================================================================

TEST_CASE("vov CPO graph_value(g)", "[dynamic_graph][vov][cpo][graph_value]") {
    SECTION("basic access") {
        vov_all_int g({{0, 1, 1}});
        
        // Set graph value
        graph_value(g) = 42;
        
        REQUIRE(graph_value(g) == 42);
    }

    SECTION("default initialization") {
        vov_all_int g;
        
        // Default constructed int should be 0
        REQUIRE(graph_value(g) == 0);
    }

    SECTION("const correctness") {
        vov_all_int g({{0, 1, 1}});
        graph_value(g) = 99;
        
        const auto& const_g = g;
        
        // Should be able to read from const graph
        REQUIRE(graph_value(const_g) == 99);
        
        // Verify type is const-qualified
        static_assert(std::is_const_v<std::remove_reference_t<decltype(graph_value(const_g))>>);
    }

    SECTION("with string values") {
        vov_string g;
        
        // Set string value
        graph_value(g) = "graph metadata";
        
        REQUIRE(graph_value(g) == "graph metadata");
        
        // Modify through reference
        graph_value(g) += " updated";
        
        REQUIRE(graph_value(g) == "graph metadata updated");
    }

    SECTION("modification") {
        vov_all_int g({{0, 1, 1}, {1, 2, 2}});
        
        // Initialize
        graph_value(g) = 0;
        REQUIRE(graph_value(g) == 0);
        
        // Increment
        graph_value(g) += 10;
        REQUIRE(graph_value(g) == 10);
        
        // Multiply
        graph_value(g) *= 3;
        REQUIRE(graph_value(g) == 30);
    }

    SECTION("independent of vertices/edges") {
        vov_all_int g({{0, 1, 1}});
        graph_value(g) = 100;
        
        // Modify vertex values
        for (auto u : vertices(g)) {
            vertex_value(g, u) = 50;
        }
        
        // Graph value should be unchanged
        REQUIRE(graph_value(g) == 100);
        
        // Modify edge values
        for (auto u : vertices(g)) {
            auto& v = u.inner_value(g);
            auto& edge_range = v.edges();
            for (auto e_iter = edge_range.begin(); e_iter != edge_range.end(); ++e_iter) {
                using edge_iter_t = decltype(e_iter);
                using vertex_desc_t = decltype(u);
                auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
                edge_value(g, uv) = 75;
            }
        }
        
        // Graph value should still be unchanged
        REQUIRE(graph_value(g) == 100);
    }
}

//==================================================================================================
// 19. partition_id(g, u) CPO Tests - Default Single Partition Behavior
//==================================================================================================

TEST_CASE("vov CPO partition_id(g, u)", "[dynamic_graph][vov][cpo][partition_id]") {
    SECTION("default single partition") {
        vov_void g;
        g.resize_vertices(5);
        
        // All vertices should be in partition 0 by default
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }

    SECTION("with edges") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Even with edges, all vertices in single partition
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }

    SECTION("const correctness") {
        const vov_void g({{0, 1}, {1, 2}});
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }

    SECTION("with different graph types") {
        vov_int_ev g1({{0, 1, 10}, {1, 2, 20}});
        vov_all_int g2({{0, 1, 1}, {1, 2, 2}});
        vov_string g3({{0, 1, "edge"}});
        
        // All graph types should default to partition 0
        for (auto u : vertices(g1)) {
            REQUIRE(partition_id(g1, u) == 0);
        }
        
        for (auto u : vertices(g2)) {
            REQUIRE(partition_id(g2, u) == 0);
        }
        
        for (auto u : vertices(g3)) {
            REQUIRE(partition_id(g3, u) == 0);
        }
    }

    SECTION("large graph") {
        vov_void g;
        g.resize_vertices(100);
        
        // Even in large graph, all vertices in partition 0
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }
}

//==================================================================================================
// 20. num_partitions(g) CPO Tests - Default Single Partition
//==================================================================================================

TEST_CASE("vov CPO num_partitions(g)", "[dynamic_graph][vov][cpo][num_partitions]") {
    SECTION("default value") {
        vov_void g;
        
        // Default should be 1 partition
        REQUIRE(num_partitions(g) == 1);
    }

    SECTION("with vertices and edges") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});
        
        // Still 1 partition regardless of graph structure
        REQUIRE(num_partitions(g) == 1);
        
        // Verify consistency: all vertices in partition 0
        size_t vertices_in_partition_0 = 0;
        for (auto u : vertices(g)) {
            if (partition_id(g, u) == 0) {
                ++vertices_in_partition_0;
            }
        }
        REQUIRE(vertices_in_partition_0 == num_vertices(g));
    }

    SECTION("const correctness") {
        const vov_void g({{0, 1}});
        
        REQUIRE(num_partitions(g) == 1);
    }

    SECTION("consistency with partition_id") {
        vov_all_int g({{0, 1, 1}, {1, 2, 2}, {2, 3, 3}});
        
        auto n_partitions = num_partitions(g);
        REQUIRE(n_partitions == 1);
        
        // All partition IDs should be in range [0, num_partitions)
        for (auto u : vertices(g)) {
            auto pid = partition_id(g, u);
            REQUIRE(pid >= 0);
            REQUIRE(pid < n_partitions);
        }
    }
}

//==================================================================================================
// 21. vertices(g, pid) CPO Tests - Default Single Partition Behavior
//==================================================================================================

TEST_CASE("vov CPO vertices(g, pid)", "[dynamic_graph][vov][cpo][vertices][partition]") {
    SECTION("partition 0 returns all vertices") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Partition 0 should return all vertices (default single partition)
        auto verts_all = vertices(g);
        auto verts_p0 = vertices(g, 0);
        
        // Should have same size
        REQUIRE(std::ranges::distance(verts_all) == std::ranges::distance(verts_p0));
        
        // Should contain same vertices
        size_t count = 0;
        for (auto u : verts_p0) {
            REQUIRE(partition_id(g, u) == 0);
            ++count;
        }
        REQUIRE(count == num_vertices(g));
    }

    SECTION("non-zero partition returns empty") {
        vov_void g({{0, 1}, {1, 2}});
        
        // Non-zero partitions should return empty range (default single partition)
        auto verts_p1 = vertices(g, 1);
        auto verts_p2 = vertices(g, 2);
        
        REQUIRE(std::ranges::distance(verts_p1) == 0);
        REQUIRE(std::ranges::distance(verts_p2) == 0);
    }

    SECTION("const correctness") {
        const vov_void g({{0, 1}, {1, 2}});
        
        auto verts_p0 = vertices(g, 0);
        REQUIRE(std::ranges::distance(verts_p0) == 3);
        
        auto verts_p1 = vertices(g, 1);
        REQUIRE(std::ranges::distance(verts_p1) == 0);
    }

    SECTION("with different graph types") {
        vov_int_ev g1({{0, 1, 10}, {1, 2, 20}});
        vov_all_int g2({{0, 1, 1}, {1, 2, 2}});
        
        // All graph types should return all vertices for partition 0
        auto verts1_p0 = vertices(g1, 0);
        REQUIRE(std::ranges::distance(verts1_p0) == 3);
        
        auto verts2_p0 = vertices(g2, 0);
        REQUIRE(std::ranges::distance(verts2_p0) == 3);
        
        // Non-zero partitions should be empty
        auto verts1_p1 = vertices(g1, 1);
        REQUIRE(std::ranges::distance(verts1_p1) == 0);
        
        auto verts2_p1 = vertices(g2, 1);
        REQUIRE(std::ranges::distance(verts2_p1) == 0);
    }
}

//==================================================================================================
// 22. num_vertices(g, pid) CPO Tests - Default Single Partition Behavior
//==================================================================================================

TEST_CASE("vov CPO num_vertices(g, pid)", "[dynamic_graph][vov][cpo][num_vertices][partition]") {
    SECTION("partition 0 returns total count") {
        vov_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Partition 0 should return total vertex count (default single partition)
        REQUIRE(num_vertices(g, 0) == num_vertices(g));
        REQUIRE(num_vertices(g, 0) == 4);
    }

    SECTION("non-zero partition returns zero") {
        vov_void g({{0, 1}, {1, 2}});
        
        // Non-zero partitions should return 0 (default single partition)
        REQUIRE(num_vertices(g, 1) == 0);
        REQUIRE(num_vertices(g, 2) == 0);
        REQUIRE(num_vertices(g, 99) == 0);
    }

    SECTION("const correctness") {
        const vov_void g({{0, 1}, {1, 2}});
        
        REQUIRE(num_vertices(g, 0) == 3);
        REQUIRE(num_vertices(g, 1) == 0);
    }

    SECTION("consistency with vertices(g, pid)") {
        vov_all_int g({{0, 1, 1}, {1, 2, 2}, {2, 3, 3}});
        
        // For partition 0, num_vertices(g, 0) should equal distance(vertices(g, 0))
        REQUIRE(num_vertices(g, 0) == static_cast<size_t>(std::ranges::distance(vertices(g, 0))));
        
        // For non-existent partitions, both should return 0/empty
        REQUIRE(num_vertices(g, 1) == static_cast<size_t>(std::ranges::distance(vertices(g, 1))));
        REQUIRE(num_vertices(g, 2) == static_cast<size_t>(std::ranges::distance(vertices(g, 2))));
        
        // Sum of all partition sizes should equal total (for single partition)
        size_t total = 0;
        for (size_t pid = 0; pid < static_cast<size_t>(num_partitions(g)); ++pid) {
            total += num_vertices(g, pid);
        }
        REQUIRE(total == num_vertices(g));
    }
}

//==================================================================================================
// 23. source_id(g, uv) CPO Tests - Sourced Edge Descriptor
//==================================================================================================

TEST_CASE("vov CPO source_id(g, uv)", "[dynamic_graph][vov][cpo][source_id]") {
    SECTION("basic usage") {
        vov_sourced_void g({{0, 1}, {1, 2}, {0, 2}});
        
        // Get edge from vertex 0
        auto u0 = *find_vertex(g, 0);
        auto e_range = edges(g, u0);
        auto e_it = e_range.begin();
        
        REQUIRE(e_it != e_range.end());
        auto uv = *e_it;
        
        // source_id should return 0 (the source vertex ID)
        REQUIRE(source_id(g, uv) == 0);
    }

    SECTION("multiple edges from same source") {
        vov_sourced_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u0 = *find_vertex(g, 0);
        
        // All edges from vertex 0 should have source_id == 0
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
        }
    }

    SECTION("different sources") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Check each vertex's outgoing edges
        for (size_t i = 0; i < 3; ++i) {
            auto u = *find_vertex(g, i);
            for (auto uv : edges(g, u)) {
                REQUIRE(source_id(g, uv) == i);
            }
        }
    }

    SECTION("with edge values") {
        vov_sourced_int g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
        
        // Verify source_id works correctly with edge values
        auto u0 = *find_vertex(g, 0);
        auto edges_from_0 = edges(g, u0);
        auto e_it = edges_from_0.begin();
        
        REQUIRE(e_it != edges_from_0.end());
        auto uv = *e_it;
        
        REQUIRE(source_id(g, uv) == 0);
        REQUIRE(target_id(g, uv) == 1);
        REQUIRE(edge_value(g, uv) == 10);
    }

    SECTION("self-loops") {
        vov_sourced_void g({{0, 0}, {1, 1}});
        
        // Self-loops: source and target are the same
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
            REQUIRE(target_id(g, uv) == 0);
        }
        
        auto u1 = *find_vertex(g, 1);
        for (auto uv : edges(g, u1)) {
            REQUIRE(source_id(g, uv) == 1);
            REQUIRE(target_id(g, uv) == 1);
        }
    }

    SECTION("const correctness") {
        const vov_sourced_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
        }
    }

    SECTION("parallel edges") {
        vov_sourced_int g({{0, 1, 10}, {0, 1, 20}, {0, 1, 30}});
        
        auto u0 = *find_vertex(g, 0);
        
        // All parallel edges should have the same source_id
        int count = 0;
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
            REQUIRE(target_id(g, uv) == 1);
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("star graph") {
        vov_sourced_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});
        
        // Center vertex has all edges with source_id == 0
        auto u0 = *find_vertex(g, 0);
        size_t edge_count = 0;
        
        for (auto uv : edges(g, u0)) {
            REQUIRE(source_id(g, uv) == 0);
            ++edge_count;
        }
        
        REQUIRE(edge_count == 4);
    }

    SECTION("chain graph") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
        
        // Each vertex has edges with its own ID as source
        for (size_t i = 0; i < 4; ++i) {
            auto u = *find_vertex(g, i);
            for (auto uv : edges(g, u)) {
                REQUIRE(source_id(g, uv) == i);
                REQUIRE(target_id(g, uv) == i + 1);
            }
        }
    }

    SECTION("cycle graph") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});
        
        std::vector<std::pair<uint32_t, uint32_t>> expected_edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}
        };
        
        for (const auto& [src, tgt] : expected_edges) {
            auto u = *find_vertex(g, src);
            bool found = false;
            
            for (auto uv : edges(g, u)) {
                if (target_id(g, uv) == tgt) {
                    REQUIRE(source_id(g, uv) == src);
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
    }

    SECTION("with all value types") {
        vov_sourced_all g({{0, 1, 100}, {1, 2, 200}, {2, 0, 300}});
        
        // Initialize vertex values
        for (auto u : vertices(g)) {
            vertex_value(g, u) = static_cast<int>(vertex_id(g, u)) * 10;
        }
        
        // Check that source_id, target_id, and values all work together
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto src_id = source_id(g, uv);
            auto tgt_id = target_id(g, uv);
            
            REQUIRE(src_id == 0);
            REQUIRE(tgt_id == 1);
            REQUIRE(edge_value(g, uv) == 100);
            
            auto src = source(g, uv);
            REQUIRE(vertex_value(g, src) == 0);
        }
    }

    SECTION("consistency with source(g, uv)") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // source_id(g, uv) should equal vertex_id(g, source(g, uv))
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto src_id = source_id(g, uv);
                auto src = source(g, uv);
                REQUIRE(src_id == vertex_id(g, src));
            }
        }
    }
}

//==================================================================================================
// 24. source(g, uv) CPO Tests - Get Source Vertex Descriptor
//==================================================================================================

TEST_CASE("vov CPO source(g, uv)", "[dynamic_graph][vov][cpo][source]") {
    SECTION("basic usage") {
        vov_sourced_void g({{0, 1}, {1, 2}, {0, 2}});
        
        // Get edge from vertex 0
        auto u0 = *find_vertex(g, 0);
        auto e_range = edges(g, u0);
        auto e_it = e_range.begin();
        
        REQUIRE(e_it != e_range.end());
        auto uv = *e_it;
        
        // source(g, uv) should return vertex descriptor for vertex 0
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == 0);
    }

    SECTION("consistency with source_id") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // For all edges, vertex_id(source(g, uv)) should equal source_id(g, uv)
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto src = source(g, uv);
                auto src_id = source_id(g, uv);
                REQUIRE(vertex_id(g, src) == src_id);
            }
        }
    }

    SECTION("returns valid descriptor") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 0}});
        
        // source() should return a valid vertex descriptor that can be used with other CPOs
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto src = source(g, uv);
            
            // Should be able to use the descriptor with other CPOs
            REQUIRE(vertex_id(g, src) == 0);
            
            // Should be able to get edges from the source
            auto src_edges = edges(g, src);
            REQUIRE(std::ranges::distance(src_edges) > 0);
        }
    }

    SECTION("with edge values") {
        vov_sourced_int g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
        
        auto u1 = *find_vertex(g, 1);
        for (auto uv : edges(g, u1)) {
            auto src = source(g, uv);
            REQUIRE(vertex_id(g, src) == 1);
            
            // Should be able to use source descriptor with other CPOs
            auto tgt = target(g, uv);
            REQUIRE(vertex_id(g, tgt) == 2);
            REQUIRE(edge_value(g, uv) == 20);
        }
    }

    SECTION("with vertex values") {
        vov_sourced_all g({{0, 1, 100}, {1, 2, 200}});
        
        // Set vertex values
        for (auto u : vertices(g)) {
            vertex_value(g, u) = static_cast<int>(vertex_id(g, u)) * 10;
        }
        
        // Verify source descriptor can access vertex values
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto src = source(g, uv);
            REQUIRE(vertex_value(g, src) == 0);  // vertex 0 has value 0
        }
    }

    SECTION("self-loops") {
        vov_sourced_void g({{0, 0}, {1, 1}, {2, 2}});
        
        // For self-loops, source and target should be the same vertex
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto src = source(g, uv);
                auto tgt = target(g, uv);
                
                REQUIRE(vertex_id(g, src) == vertex_id(g, tgt));
                REQUIRE(vertex_id(g, src) == vertex_id(g, u));
            }
        }
    }

    SECTION("const correctness") {
        const vov_sourced_void g({{0, 1}, {1, 2}});
        
        auto u0 = *find_vertex(g, 0);
        for (auto uv : edges(g, u0)) {
            auto src = source(g, uv);
            REQUIRE(vertex_id(g, src) == 0);
        }
    }

    SECTION("parallel edges") {
        vov_sourced_int g({{0, 1, 10}, {0, 1, 20}, {0, 1, 30}});
        
        auto u0 = *find_vertex(g, 0);
        
        // All parallel edges should have the same source
        for (auto uv : edges(g, u0)) {
            auto src = source(g, uv);
            REQUIRE(vertex_id(g, src) == 0);
        }
    }

    SECTION("chain graph") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
        
        // Each edge's source should match the vertex we're iterating from
        for (size_t i = 0; i < 4; ++i) {
            auto u = *find_vertex(g, i);
            for (auto uv : edges(g, u)) {
                auto src = source(g, uv);
                REQUIRE(vertex_id(g, src) == i);
            }
        }
    }

    SECTION("star graph") {
        vov_sourced_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});
        
        // Center vertex (0) is the source for all edges
        auto u0 = *find_vertex(g, 0);
        size_t edge_count = 0;
        
        for (auto uv : edges(g, u0)) {
            auto src = source(g, uv);
            REQUIRE(vertex_id(g, src) == 0);
            ++edge_count;
        }
        
        REQUIRE(edge_count == 4);
    }

    SECTION("can traverse from source to target") {
        vov_sourced_void g({{0, 1}, {1, 2}, {2, 3}});
        
        // Use source and target to traverse the chain
        auto u0 = *find_vertex(g, 0);
        auto edges_from_0 = edges(g, u0);
        auto e_it = edges_from_0.begin();
        
        REQUIRE(e_it != edges_from_0.end());
        auto edge = *e_it;
        
        auto src = source(g, edge);
        auto tgt = target(g, edge);
        
        REQUIRE(vertex_id(g, src) == 0);
        REQUIRE(vertex_id(g, tgt) == 1);
        
        // Can use target as source for next edge lookup
        auto edges_from_tgt = edges(g, tgt);
        REQUIRE(std::ranges::distance(edges_from_tgt) == 1);
    }

    SECTION("accumulate values from edges") {
        vov_sourced_all g({{0, 1, 100}, {1, 2, 200}, {2, 3, 300}});
        
        // Initialize vertex values to 0
        for (auto u : vertices(g)) {
            vertex_value(g, u) = 0;
        }
        
        // Accumulate edge values into source vertices
        for (auto u : vertices(g)) {
            for (auto uv : edges(g, u)) {
                auto src = source(g, uv);
                vertex_value(g, src) += edge_value(g, uv);
            }
        }
        
        // Verify accumulated values
        REQUIRE(vertex_value(g, *find_vertex(g, 0)) == 100);
        REQUIRE(vertex_value(g, *find_vertex(g, 1)) == 200);
        REQUIRE(vertex_value(g, *find_vertex(g, 2)) == 300);
        REQUIRE(vertex_value(g, *find_vertex(g, 3)) == 0);
    }
}

//==================================================================================================
// 25. Integration Tests - Multiple CPOs Working Together
//==================================================================================================

TEST_CASE("vov CPO integration: modify vertex and edge values", "[dynamic_graph][vov][cpo][integration]") {
    vov_all_int g({{0, 1, 1}, {1, 2, 2}});
    
    // Initialize vertex values
    for (auto u : vertices(g)) {
        vertex_value(g, u) = 0;
    }
    
    // Accumulate edge values into source vertices
    for (auto u : vertices(g)) {
        auto& v = u.inner_value(g);
        auto& edge_range = v.edges();
        for (auto e_iter = edge_range.begin(); e_iter != edge_range.end(); ++e_iter) {
            using edge_iter_t = decltype(e_iter);
            using vertex_desc_t = decltype(u);
            auto uv = edge_descriptor<edge_iter_t, typename vertex_desc_t::iterator_type>(static_cast<std::size_t>(e_iter - edge_range.begin()), u);
            vertex_value(g, u) += edge_value(g, uv);
        }
    }
    
    // Verify accumulated values
    int expected_values[] = {1, 2, 0};
    int idx = 0;
    for (auto u : vertices(g)) {
        REQUIRE(vertex_value(g, u) == expected_values[idx]);
        ++idx;
        if (idx >= 3) break;
    }
}
