/**
 * @file test_dynamic_graph_cpo_unordered.cpp
 * @brief Consolidated CPO tests for unordered edge containers (vous, dous)
 * 
 * Uses template infrastructure from graph_test_types.hpp to test container
 * types with a single set of test cases.
 * 
 * NOTE: mous and uous (map-based vertex containers) are NOT included here
 * because they use different vertex creation semantics (on-demand vertex
 * creation from edges rather than resize_vertices).
 * 
 * IMPORTANT: Unordered_set containers use hash-based storage, so edge order is
 * unspecified. Tests that depend on edge ordering use sorted comparison rather
 * than positional assertions. Also, unordered_set does not allow duplicate keys,
 * so parallel edges with the same target are deduplicated.
 * 
 * Key differences from random_access containers:
 * - Edge iteration order is unspecified (hash-based)
 * - Parallel edges are deduplicated (same target_id not allowed)
 * - O(1) average lookup/insertion vs O(1) random access
 * - Forward iterators only (no bidirectional)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "../../common/graph_test_types.hpp"
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO vertices(g)", "[dynamic_graph][cpo][vertices]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_vv = typename Types::int_vv;

    SECTION("returns vertex_descriptor_view") {
        Graph_void g;
        g.resize_vertices(5);
        
        auto v_range = vertices(g);
        
        REQUIRE(std::ranges::size(v_range) == 5);
        
        size_t count = 0;
        for ([[maybe_unused]] auto v : v_range) {
            ++count;
        }
        REQUIRE(count == 5);
    }

    SECTION("const correctness") {
        const Graph_void g;
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 0);
    }

    SECTION("with values") {
        Graph_int_vv g;
        g.resize_vertices(3);
        
        auto v_range = vertices(g);
        REQUIRE(std::ranges::size(v_range) == 3);
    }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO num_vertices(g)", "[dynamic_graph][cpo][num_vertices]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_vv = typename Types::int_vv;

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(num_vertices(g) == 0);
    }

    SECTION("non-empty") {
        Graph_void g;
        g.resize_vertices(10);
        REQUIRE(num_vertices(g) == 10);
    }

    SECTION("matches vertices size") {
        Graph_int_vv g;
        g.resize_vertices(7);
        REQUIRE(num_vertices(g) == std::ranges::size(vertices(g)));
    }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO find_vertex(g, uid)", "[dynamic_graph][cpo][find_vertex]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("finds valid vertex") {
        Graph_void g;
        g.resize_vertices(5);
        
        auto u = find_vertex(g, 2u);
        REQUIRE(u != end(vertices(g)));
        REQUIRE(vertex_id(g, *u) == 2);
    }

    SECTION("returns end for invalid") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto u = find_vertex(g, 5u);
        REQUIRE(u == end(vertices(g)));
    }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO vertex_id(g, u)", "[dynamic_graph][cpo][vertex_id]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("returns correct id") {
        Graph_void g;
        g.resize_vertices(5);
        
        size_t idx = 0;
        for (auto u : vertices(g)) {
            REQUIRE(vertex_id(g, u) == idx);
            ++idx;
        }
    }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO num_edges(g)", "[dynamic_graph][cpo][num_edges]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(num_edges(g) == 0);
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}});
        REQUIRE(num_edges(g) == 3);
    }

    SECTION("counts all edges") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});
        REQUIRE(num_edges(g) == 4);
    }
}

//==================================================================================================
// 6. num_edges(g, u) CPO Tests - SUPPORTED (unordered_set has size())
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO num_edges(g, u)", "[dynamic_graph][cpo][num_edges]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("vertex with no edges") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 0);
    }

    SECTION("vertex with single edge") {
        Graph_void g({{0, 1}});
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 1);
    }

    SECTION("vertex with multiple edges") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u = *find_vertex(g, 0);
        REQUIRE(num_edges(g, u) == 3);
    }

    SECTION("matches degree") {
        Graph_void g({{0, 1}, {0, 2}, {1, 2}});
        
        for (auto u : vertices(g)) {
            REQUIRE(num_edges(g, u) == degree(g, u));
        }
    }
}

//==================================================================================================
// 7. edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO edges(g, u)", "[dynamic_graph][cpo][edges]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("empty vertex") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("with edge values") {
        Graph_int_ev g({{0, 1, 100}, {0, 2, 200}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<int> values;
        for (auto uv : edge_range) {
            values.push_back(edge_value(g, uv));
        }
        
        REQUIRE(values.size() == 2);
        // Unordered containers don't guarantee order - sort before checking
        std::sort(values.begin(), values.end());
        REQUIRE(values[0] == 100);
        REQUIRE(values[1] == 200);
    }

    SECTION("multiple edges") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u0 = *find_vertex(g, 0);
        auto edge_range = edges(g, u0);
        
        std::vector<uint32_t> targets;
        for (auto uv : edge_range) {
            targets.push_back(target_id(g, uv));
        }
        
        REQUIRE(targets.size() == 3);
        // Unordered containers don't guarantee order - sort before checking
        std::sort(targets.begin(), targets.end());
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
        REQUIRE(targets[2] == 3);
    }
}

//==================================================================================================
// 8. edges(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO edges(g, uid)", "[dynamic_graph][cpo][edges]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("by vertex ID") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto edge_range = edges(g, uint32_t(0));
        
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edge_range) {
            ++count;
        }
        REQUIRE(count == 2);
    }
}

//==================================================================================================
// 9. degree(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO degree(g, u)", "[dynamic_graph][cpo][degree]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("empty vertex") {
        Graph_void g;
        g.resize_vertices(3);
        
        auto u = *find_vertex(g, 0);
        REQUIRE(degree(g, u) == 0);
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u = *find_vertex(g, 0);
        REQUIRE(degree(g, u) == 3);
    }

    SECTION("by vertex ID") {
        Graph_void g({{0, 1}, {0, 2}, {1, 0}});
        
        REQUIRE(degree(g, 0u) == 2);
        REQUIRE(degree(g, 1u) == 1);
    }
}

//==================================================================================================
// 10. target_id(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO target_id(g, uv)", "[dynamic_graph][cpo][target_id]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("returns correct target") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        std::vector<uint32_t> targets;
        for (auto uv : edges(g, u0)) {
            targets.push_back(target_id(g, uv));
        }
        
        std::sort(targets.begin(), targets.end());
        REQUIRE(targets.size() == 2);
        REQUIRE(targets[0] == 1);
        REQUIRE(targets[1] == 2);
    }
}

//==================================================================================================
// 11. target(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO target(g, uv)", "[dynamic_graph][cpo][target]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("returns vertex descriptor") {
        Graph_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *begin(edges(g, u0));
        auto v = target(g, uv);
        
        REQUIRE(vertex_id(g, v) == 1);
    }
}

//==================================================================================================
// 12. find_vertex_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO find_vertex_edge(g, uid, vid)", "[dynamic_graph][cpo][find_vertex_edge]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("finds existing edge") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto uv = find_vertex_edge(g, 0u, 1u);
        REQUIRE(target_id(g, uv) == 1);
    }

    SECTION("by vertex IDs") {
        Graph_int_ev g({{0, 1, 100}});
        
        auto uv = find_vertex_edge(g, 0u, 1u);
        REQUIRE(edge_value(g, uv) == 100);
    }
}

//==================================================================================================
// 13. contains_edge(g, u, v) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO contains_edge(g, u, v)", "[dynamic_graph][cpo][contains_edge]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("existing edge") {
        Graph_void g({{0, 1}, {0, 2}});
        
        auto u0 = *find_vertex(g, 0);
        auto v1 = *find_vertex(g, 1);
        
        REQUIRE(contains_edge(g, u0, v1) == true);
    }

    SECTION("non-existent edge") {
        Graph_void g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto v2 = *find_vertex(g, 2);
        
        REQUIRE(contains_edge(g, u0, v2) == false);
    }

    SECTION("by vertex IDs") {
        Graph_void g({{0, 1}, {1, 2}});
        
        REQUIRE(contains_edge(g, 0u, 1u) == true);
        REQUIRE(contains_edge(g, 1u, 2u) == true);
        REQUIRE(contains_edge(g, 0u, 2u) == false);
    }
}

//==================================================================================================
// 14. vertex_value(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO vertex_value(g, u)", "[dynamic_graph][cpo][vertex_value]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_int_vv = typename Types::int_vv;

    SECTION("access and modify") {
        Graph_int_vv g;
        g.resize_vertices(3);
        
        auto u = *find_vertex(g, 1);
        vertex_value(g, u) = 42;
        
        REQUIRE(vertex_value(g, u) == 42);
    }
}

//==================================================================================================
// 15. edge_value(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO edge_value(g, uv)", "[dynamic_graph][cpo][edge_value]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_int_ev = typename Types::int_ev;

    SECTION("access") {
        Graph_int_ev g({{0, 1, 100}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *begin(edges(g, u0));
        
        REQUIRE(edge_value(g, uv) == 100);
    }
}

//==================================================================================================
// 16. graph_value(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO graph_value(g)", "[dynamic_graph][cpo][graph_value]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_all_int = typename Types::all_int;

    SECTION("access and modify") {
        Graph_all_int g;
        graph_value(g) = 42;
        
        REQUIRE(graph_value(g) == 42);
    }
}

//==================================================================================================
// 17. source_id(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO source_id(g, uv)", "[dynamic_graph][cpo][source_id]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_sourced = typename Types::sourced_void;

    SECTION("returns source vertex ID") {
        Graph_sourced g({{0, 1}, {2, 3}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *begin(edges(g, u0));
        
        REQUIRE(source_id(g, uv) == 0);
    }
}

//==================================================================================================
// 18. source(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO source(g, uv)", "[dynamic_graph][cpo][source]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_sourced = typename Types::sourced_void;

    SECTION("returns source vertex descriptor") {
        Graph_sourced g({{0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        auto uv = *begin(edges(g, u0));
        auto src = source(g, uv);
        
        REQUIRE(vertex_id(g, src) == 0);
    }
}

//==================================================================================================
// 19. partition_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO partition_id(g, u)", "[dynamic_graph][cpo][partition_id]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("always returns 0 (single partition)") {
        Graph_void g;
        g.resize_vertices(3);
        
        for (auto u : vertices(g)) {
            REQUIRE(partition_id(g, u) == 0);
        }
    }
}

//==================================================================================================
// 20. num_partitions(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO num_partitions(g)", "[dynamic_graph][cpo][num_partitions]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("always returns 1") {
        Graph_void g;
        REQUIRE(num_partitions(g) == 1);
    }
}

//==================================================================================================
// 21. has_edge(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("unordered CPO has_edge(g)", "[dynamic_graph][cpo][has_edge]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("empty graph") {
        Graph_void g;
        REQUIRE(has_edge(g) == false);
    }

    SECTION("with edges") {
        Graph_void g({{0, 1}});
        REQUIRE(has_edge(g) == true);
    }
}

//==================================================================================================
// 22. Edge Deduplication Tests (unordered_set specific)
//==================================================================================================

TEMPLATE_TEST_CASE("unordered edge deduplication", "[dynamic_graph][unordered][deduplication]",
                   vous_tag, dous_tag) {
    using Types = graph_test_types<TestType>;
    using Graph_void = typename Types::void_type;

    SECTION("duplicate edges are deduplicated") {
        // unordered_set doesn't allow duplicate keys (target_id)
        Graph_void g({{0, 1}, {0, 1}, {0, 1}});
        
        auto u0 = *find_vertex(g, 0);
        size_t count = 0;
        for ([[maybe_unused]] auto uv : edges(g, u0)) {
            ++count;
        }
        
        // Should only have 1 edge to target 1, not 3
        REQUIRE(count == 1);
    }

    SECTION("different targets are preserved") {
        Graph_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto u0 = *find_vertex(g, 0);
        REQUIRE(degree(g, u0) == 3);
    }
}
