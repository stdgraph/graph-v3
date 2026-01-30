/**
 * @file test_dynamic_graph_vos.cpp
 * @brief Comprehensive tests for dynamic_graph with vector vertices + set edges
 * 
 * Phase 4.1.2: Set Edge Container Support
 * Tests voem_graph_traits (vector vertices + set edges)
 * 
 * Key characteristics of std::set edges:
 * - Automatic deduplication (no parallel edges with same endpoints)
 * - Edges stored in sorted order (by source_id if Sourced, then target_id)
 * - O(log n) edge insertion, lookup, and deletion
 * - Bidirectional iterators (no random access to edges)
 * - Edge values NOT considered in comparison (only structural IDs)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/voem_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <set>

using namespace graph::container;

// Type aliases for common test configurations
using vos_void_void_void = dynamic_graph<void, void, void, uint32_t, false, voem_graph_traits<void, void, void, uint32_t, false>>;
using vos_int_void_void = dynamic_graph<int, void, void, uint32_t, false, voem_graph_traits<int, void, void, uint32_t, false>>;
using vos_void_int_void = dynamic_graph<void, int, void, uint32_t, false, voem_graph_traits<void, int, void, uint32_t, false>>;
using vos_int_int_void = dynamic_graph<int, int, void, uint32_t, false, voem_graph_traits<int, int, void, uint32_t, false>>;
using vos_void_void_int = dynamic_graph<void, void, int, uint32_t, false, voem_graph_traits<void, void, int, uint32_t, false>>;
using vos_int_int_int = dynamic_graph<int, int, int, uint32_t, false, voem_graph_traits<int, int, int, uint32_t, false>>;

using vos_string_string_string = dynamic_graph<std::string, std::string, std::string, uint32_t, false, 
                                                  voem_graph_traits<std::string, std::string, std::string, uint32_t, false>>;

using vos_sourced = dynamic_graph<void, void, void, uint32_t, true, voem_graph_traits<void, void, void, uint32_t, true>>;
using vos_int_sourced = dynamic_graph<int, void, void, uint32_t, true, voem_graph_traits<int, void, void, uint32_t, true>>;

// Edge and vertex data types for loading
using edge_void = copyable_edge_t<uint32_t, void>;
using edge_int = copyable_edge_t<uint32_t, int>;
using vertex_int = copyable_vertex_t<uint32_t, int>;

// Helper function to count total edges in graph
template<typename G>
size_t count_all_edges(G& g) {
    size_t count = 0;
    for (auto& v : g) {
        count += static_cast<size_t>(std::ranges::distance(v.edges()));
    }
    return count;
}

//==================================================================================================
// 1. Construction Tests
//==================================================================================================

TEST_CASE("voem default construction", "[voem][construction]") {
    SECTION("creates empty graph") {
        vos_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("with void types") {
        vos_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("with int edge values") {
        vos_int_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("with int vertex values") {
        vos_void_int_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("with int graph value") {
        vos_void_void_int g;
        REQUIRE(g.size() == 0);
    }

    SECTION("with all int values") {
        vos_int_int_int g;
        REQUIRE(g.size() == 0);
    }

    SECTION("with string values") {
        vos_string_string_string g;
        REQUIRE(g.size() == 0);
    }
}

TEST_CASE("voem constructor with graph value", "[voem][construction]") {
    SECTION("void GV - no graph value can be passed") {
        vos_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("int GV") {
        vos_void_void_int g(42);
        REQUIRE(g.size() == 0);
        REQUIRE(g.graph_value() == 42);
    }
}

//==================================================================================================
// 2. Load Edges Tests
//==================================================================================================

TEST_CASE("voem load_edges", "[voem][load]") {
    SECTION("simple edges") {
        vos_void_void_void g;
        std::vector<edge_void> ee = {{0, 1}, {0, 2}, {1, 2}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(count_all_edges(g) == 3);
    }

    SECTION("edges with vertex count") {
        vos_void_void_void g;
        std::vector<edge_void> ee = {{0, 1}, {1, 2}};
        g.load_edges(ee, std::identity{}, 6);  // Request 6 vertices
        
        REQUIRE(g.size() == 6);  // 0 through 5
        REQUIRE(count_all_edges(g) == 2);
    }
    
    SECTION("edges with values") {
        vos_int_void_void g;
        std::vector<edge_int> ee = {{0, 1, 100}, {0, 2, 200}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(count_all_edges(g) == 2);
        
        auto& v0 = g[0];
        auto it = v0.edges().begin();
        // Edges are sorted by target_id
        REQUIRE(it->second.target_id() == 1);
        REQUIRE(it->second.value() == 100);
        ++it;
        REQUIRE(it->second.target_id() == 2);
        REQUIRE(it->second.value() == 200);
    }
}

//==================================================================================================
// 3. Initializer List Construction Tests
//==================================================================================================

TEST_CASE("voem initializer list construction", "[voem][construction][initializer_list]") {
    SECTION("simple initializer list") {
        vos_void_void_void g({{0, 1}, {0, 2}, {1, 2}});
        
        REQUIRE(g.size() == 3);
        REQUIRE(count_all_edges(g) == 3);
    }
}

//==================================================================================================
// 4. Set-Specific Behavior: Deduplication Tests
//==================================================================================================

TEST_CASE("voem edge deduplication", "[voem][set][deduplication]") {
    SECTION("duplicate edges are ignored - unsourced") {
        vos_void_void_void g;
        // Load edges with duplicates
        std::vector<edge_void> ee = {
            {0, 1}, {0, 1}, {0, 1},  // Three identical edges
            {0, 2}, {0, 2},          // Two identical edges
            {1, 2}                   // One unique edge
        };
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 3);
        // Set deduplicates: only 3 unique edges should exist
        REQUIRE(count_all_edges(g) == 3);
        
        // Verify each vertex has correct number of edges
        auto& v0 = g[0];
        auto& v1 = g[1];
        REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 2);  // 0->1, 0->2
        REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1);  // 1->2
    }

    SECTION("duplicate edges with different values - first value wins") {
        // In std::set, insert ignores duplicates, keeping original
        vos_int_void_void g;
        std::vector<edge_int> ee = {
            {0, 1, 100}, {0, 1, 200}, {0, 1, 300}  // Same edge, different values
        };
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 2);
        REQUIRE(count_all_edges(g) == 1);  // Only one edge stored
        
        // First inserted value should be kept
        auto& v0 = g[0];
        REQUIRE(v0.edges().begin()->second.value() == 100);
    }

    SECTION("sourced edges - deduplication by (source_id, target_id)") {
        vos_sourced g;
        std::vector<edge_void> ee = {
            {0, 1}, {0, 1},  // Duplicates
            {1, 0}, {1, 0}   // Different direction, also duplicates
        };
        g.load_edges(ee, std::identity{});
        
        // Should have exactly 2 unique edges (0->1 and 1->0)
        REQUIRE(count_all_edges(g) == 2);
    }
}

//==================================================================================================
// 5. Set-Specific Behavior: Sorted Order Tests
//==================================================================================================

TEST_CASE("voem edges are sorted by target_id", "[voem][set][sorted]") {
    SECTION("unsourced edges sorted by target_id") {
        // Insert edges in unsorted order
        vos_void_void_void g;
        std::vector<edge_void> ee = {
            {0, 5}, {0, 2}, {0, 8}, {0, 1}, {0, 3}
        };
        g.load_edges(ee, std::identity{});
        
        // Edges from vertex 0 should be in sorted order by target_id
        auto& v0 = g[0];
        std::vector<uint32_t> target_ids;
        for (const auto& edge : v0.edges()) {
            target_ids.push_back(edge.second.target_id());
        }
        
        REQUIRE(target_ids == std::vector<uint32_t>{1, 2, 3, 5, 8});
    }

    SECTION("sourced edges sorted by target_id (source is same per vertex)") {
        vos_sourced g;
        std::vector<edge_void> ee = {
            {0, 7}, {0, 3}, {0, 9}, {0, 1}
        };
        g.load_edges(ee, std::identity{});
        
        auto& v0 = g[0];
        std::vector<uint32_t> target_ids;
        for (const auto& edge : v0.edges()) {
            target_ids.push_back(edge.second.target_id());
        }
        
        // Note: For sourced edges, comparison is (source_id, target_id)
        // Since source_id is same (0) for all edges from v0, they sort by target_id
        REQUIRE(target_ids == std::vector<uint32_t>{1, 3, 7, 9});
    }
}

//==================================================================================================
// 6. Vertex Access Tests
//==================================================================================================

TEST_CASE("voem vertex access", "[voem][vertex][access]") {
    SECTION("operator[] access") {
        vos_void_void_void g({{0, 1}, {1, 2}, {2, 3}});
        
        REQUIRE(g.size() == 4);
        // Access each vertex
        auto& v0 = g[0];
        auto& v1 = g[1];
        auto& v2 = g[2];
        auto& v3 = g[3];
        
        // Verify edge counts
        REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
        REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1);
        REQUIRE(std::distance(v2.edges().begin(), v2.edges().end()) == 1);
        REQUIRE(std::distance(v3.edges().begin(), v3.edges().end()) == 0);
    }

    SECTION("const operator[] access") {
        const vos_void_void_void g({{0, 1}, {1, 2}});
        
        const auto& v0 = g[0];
        const auto& v1 = g[1];
        
        REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
        REQUIRE(std::distance(v1.edges().begin(), v1.edges().end()) == 1);
    }
}

TEST_CASE("voem vertex iteration", "[voem][vertex][iteration]") {
    SECTION("range-based for") {
        vos_void_void_void g({{0, 1}, {1, 2}, {2, 0}});
        
        size_t count = 0;
        for (const auto& vertex : g) {
            (void)vertex;
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("begin/end iteration") {
        vos_void_void_void g({{0, 1}, {1, 2}});
        
        auto it = g.begin();
        REQUIRE(it != g.end());
        ++it;
        REQUIRE(it != g.end());
        ++it;
        REQUIRE(it != g.end());
        ++it;
        REQUIRE(it == g.end());
    }
}

//==================================================================================================
// 7. Edge Access Tests
//==================================================================================================

TEST_CASE("voem edge access", "[voem][edge][access]") {
    SECTION("edges() returns set") {
        vos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto& v0 = g[0];
        auto& edge_set = v0.edges();
        
        REQUIRE(std::distance(edge_set.begin(), edge_set.end()) == 3);
    }

    SECTION("edge target_id access") {
        vos_void_void_void g({{0, 5}});
        
        auto& v0 = g[0];
        auto it = v0.edges().begin();
        REQUIRE(it->second.target_id() == 5);
    }

    SECTION("edge value access") {
        vos_int_void_void g;
        std::vector<edge_int> ee = {{0, 1, 42}};
        g.load_edges(ee, std::identity{});
        
        auto& v0 = g[0];
        auto it = v0.edges().begin();
        REQUIRE(it->second.value() == 42);
    }
}

TEST_CASE("voem edge bidirectional iteration", "[voem][edge][iteration]") {
    SECTION("forward iteration") {
        vos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto& v0 = g[0];
        std::vector<uint32_t> targets;
        for (const auto& edge : v0.edges()) {
            targets.push_back(edge.second.target_id());
        }
        
        REQUIRE(targets.size() == 3);
        REQUIRE(targets == std::vector<uint32_t>{1, 2, 3});  // Sorted
    }

    SECTION("reverse iteration") {
        vos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto& v0 = g[0];
        auto& edge_set = v0.edges();
        
        std::vector<uint32_t> targets;
        for (auto it = edge_set.rbegin(); it != edge_set.rend(); ++it) {
            targets.push_back(it->second.target_id());
        }
        
        REQUIRE(targets == std::vector<uint32_t>{3, 2, 1});  // Reverse sorted
    }
}

//==================================================================================================
// 8. Vertex and Edge Value Tests
//==================================================================================================

TEST_CASE("voem vertex values", "[voem][vertex][value]") {
    SECTION("vertex value access") {
        vos_void_int_void g;
        std::vector<vertex_int> vv = {{0, 100}, {1, 200}};
        g.load_vertices(vv, std::identity{});
        
        std::vector<edge_void> ee = {{0, 1}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g[0].value() == 100);
        REQUIRE(g[1].value() == 200);
    }
}

TEST_CASE("voem edge values", "[voem][edge][value]") {
    SECTION("edge values preserved after deduplication") {
        // First edge with value 100 should be kept
        vos_int_void_void g;
        std::vector<edge_int> ee = {
            {0, 1, 100},
            {0, 2, 200}
        };
        g.load_edges(ee, std::identity{});
        
        auto& v0 = g[0];
        auto it = v0.edges().begin();
        REQUIRE(it->second.value() == 100);  // First edge value
        ++it;
        REQUIRE(it->second.value() == 200);  // Second edge value
    }
}

//==================================================================================================
// 9. Sourced Edge Tests
//==================================================================================================

TEST_CASE("voem sourced edges", "[voem][sourced]") {
    SECTION("source_id access") {
        vos_sourced g({{0, 1}, {0, 2}, {1, 0}});
        
        auto& v0 = g[0];
        for (const auto& edge : v0.edges()) {
            REQUIRE(edge.second.source_id() == 0);
        }
        
        auto& v1 = g[1];
        for (const auto& edge : v1.edges()) {
            REQUIRE(edge.second.source_id() == 1);
        }
    }

    SECTION("sourced edge with values") {
        vos_int_sourced g;
        std::vector<edge_int> ee = {
            {0, 1, 100}, {1, 0, 200}
        };
        g.load_edges(ee, std::identity{});
        
        auto& v0 = g[0];
        auto it0 = v0.edges().begin();
        REQUIRE(it0->second.source_id() == 0);
        REQUIRE(it0->second.target_id() == 1);
        REQUIRE(it0->second.value() == 100);
        
        auto& v1 = g[1];
        auto it1 = v1.edges().begin();
        REQUIRE(it1->second.source_id() == 1);
        REQUIRE(it1->second.target_id() == 0);
        REQUIRE(it1->second.value() == 200);
    }
}

//==================================================================================================
// 10. Self-Loop Tests
//==================================================================================================

TEST_CASE("voem self-loops", "[voem][self-loop]") {
    SECTION("single self-loop") {
        vos_void_void_void g({{0, 0}});
        
        REQUIRE(g.size() == 1);
        REQUIRE(count_all_edges(g) == 1);
        
        auto& v0 = g[0];
        REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 1);
        REQUIRE(v0.edges().begin()->second.target_id() == 0);
    }

    SECTION("self-loop deduplication") {
        vos_void_void_void g({{0, 0}, {0, 0}, {0, 0}});
        
        // Only one self-loop should exist
        REQUIRE(count_all_edges(g) == 1);
    }

    SECTION("self-loop with outgoing edges") {
        vos_void_void_void g({{0, 0}, {0, 1}, {0, 2}});
        
        REQUIRE(count_all_edges(g) == 3);
        
        auto& v0 = g[0];
        std::vector<uint32_t> targets;
        for (const auto& edge : v0.edges()) {
            targets.push_back(edge.second.target_id());
        }
        
        // Self-loop (0) should be first in sorted order
        REQUIRE(targets == std::vector<uint32_t>{0, 1, 2});
    }
}

//==================================================================================================
// 11. Large Graph Tests
//==================================================================================================

TEST_CASE("voem large graph", "[voem][performance]") {
    SECTION("1000 vertices linear chain") {
        std::vector<edge_void> ee;
        for (uint32_t i = 0; i < 999; ++i) {
            ee.push_back({i, i + 1});
        }
        
        vos_void_void_void g;
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 1000);
        REQUIRE(count_all_edges(g) == 999);
    }

    SECTION("star graph with 100 spokes") {
        std::vector<edge_void> ee;
        for (uint32_t i = 1; i <= 100; ++i) {
            ee.push_back({0, i});
        }
        
        vos_void_void_void g;
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 101);
        REQUIRE(count_all_edges(g) == 100);
        
        // Vertex 0 should have all 100 edges
        auto& v0 = g[0];
        REQUIRE(std::distance(v0.edges().begin(), v0.edges().end()) == 100);
    }
}

//==================================================================================================
// 12. Iterator Stability Tests (std::set guarantees)
//==================================================================================================

TEST_CASE("voem set iterator stability", "[voem][set][iterator]") {
    SECTION("edge iterators are bidirectional") {
        vos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto& v0 = g[0];
        auto& edge_set = v0.edges();
        
        // Forward
        auto it = edge_set.begin();
        REQUIRE(it->second.target_id() == 1);
        ++it;
        REQUIRE(it->second.target_id() == 2);
        ++it;
        REQUIRE(it->second.target_id() == 3);
        
        // Backward
        --it;
        REQUIRE(it->second.target_id() == 2);
        --it;
        REQUIRE(it->second.target_id() == 1);
    }
}

//==================================================================================================
// 13. Algorithm Compatibility Tests
//==================================================================================================

TEST_CASE("voem algorithm compatibility", "[voem][algorithm]") {
    SECTION("std::ranges::for_each on vertices") {
        vos_void_void_void g({{0, 1}, {1, 2}, {2, 0}});
        
        size_t count = 0;
        std::ranges::for_each(g, [&count](const auto& v) {
            (void)v;
            ++count;
        });
        
        REQUIRE(count == 3);
    }

    SECTION("std::ranges::for_each on edges") {
        vos_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto& v0 = g[0];
        size_t count = 0;
        std::ranges::for_each(v0.edges(), [&count](const auto& e) {
            (void)e;
            ++count;
        });
        
        REQUIRE(count == 3);
    }

    SECTION("std::find_if on edges") {
        vos_int_void_void g;
        std::vector<edge_int> ee = {
            {0, 1, 100}, {0, 2, 200}, {0, 3, 300}
        };
        g.load_edges(ee, std::identity{});
        
        auto& v0 = g[0];
        auto it = std::ranges::find_if(v0.edges(), [](const auto& e) {
            return e.second.value() == 200;
        });
        
        REQUIRE(it != v0.edges().end());
        REQUIRE(it->second.target_id() == 2);
    }
}

//==================================================================================================
// 14. Edge Case Tests
//==================================================================================================

TEST_CASE("voem edge cases", "[voem][edge-cases]") {
    SECTION("empty graph operations") {
        vos_void_void_void g;
        
        REQUIRE(g.size() == 0);
        REQUIRE(count_all_edges(g) == 0);
        REQUIRE(g.begin() == g.end());
    }

    SECTION("single vertex no edges") {
        // Create a graph with a single vertex using load_edges with vertex_count
        vos_void_void_void g;
        std::vector<edge_void> empty_edges;
        g.load_edges(empty_edges, std::identity{}, 1);
        
        REQUIRE(g.size() == 1);
        REQUIRE(count_all_edges(g) == 0);
        
        auto& v0 = g[0];
        REQUIRE(v0.edges().empty());
    }

    SECTION("vertices with no outgoing edges") {
        // Load edges with more vertices using load_edges with vertex_count
        vos_void_void_void g;
        std::vector<edge_void> ee = {{0, 1}};
        g.load_edges(ee, std::identity{}, 6);
        
        REQUIRE(g.size() == 6);  // 0 through 5
        
        // Only vertex 0 has an edge
        REQUIRE(std::distance(g[0].edges().begin(), g[0].edges().end()) == 1);
        
        // Vertices 1-5 have no outgoing edges (1 has incoming from 0 but no outgoing)
        for (uint32_t i = 2; i <= 5; ++i) {
            REQUIRE(g[i].edges().empty());
        }
    }
}

//==================================================================================================
// 15. Type Trait Tests
//==================================================================================================

TEST_CASE("voem type traits", "[voem][traits]") {
    SECTION("edge_type is correct") {
        using traits = voem_graph_traits<int, void, void, uint32_t, false>;
        using edge_t = traits::edge_type;
        
        static_assert(std::is_same_v<edge_t::value_type, int>);
        static_assert(std::is_same_v<edge_t::vertex_id_type, uint32_t>);
    }

    SECTION("edges_type is std::set") {
        using traits = voem_graph_traits<void, void, void, uint32_t, false>;
        using edges_t = traits::edges_type;
        
        // Verify it's a set by checking it has set-specific types
        static_assert(requires { typename edges_t::key_type; });
    }

    SECTION("sourced trait") {
        using traits_unsourced = voem_graph_traits<void, void, void, uint32_t, false>;
        using traits_sourced = voem_graph_traits<void, void, void, uint32_t, true>;
        
        static_assert(traits_unsourced::sourced == false);
        static_assert(traits_sourced::sourced == true);
    }
}

//==================================================================================================
// 16. Complex Graph Structure Tests
//==================================================================================================

TEST_CASE("voem complex structures", "[voem][complex]") {
    SECTION("complete graph K4") {
        std::vector<edge_void> ee;
        for (uint32_t i = 0; i < 4; ++i) {
            for (uint32_t j = 0; j < 4; ++j) {
                if (i != j) {
                    ee.push_back({i, j});
                }
            }
        }
        
        vos_void_void_void g;
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 4);
        REQUIRE(count_all_edges(g) == 12);  // 4 * 3 directed edges
        
        // Each vertex should have 3 outgoing edges
        for (uint32_t i = 0; i < 4; ++i) {
            REQUIRE(std::distance(g[i].edges().begin(), g[i].edges().end()) == 3);
        }
    }

    SECTION("cycle graph C5") {
        vos_void_void_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});
        
        REQUIRE(g.size() == 5);
        REQUIRE(count_all_edges(g) == 5);
    }

    SECTION("binary tree depth 3") {
        vos_void_void_void g({
            {0, 1}, {0, 2},           // Level 1
            {1, 3}, {1, 4},           // Level 2 left
            {2, 5}, {2, 6}            // Level 2 right
        });
        
        REQUIRE(g.size() == 7);
        REQUIRE(count_all_edges(g) == 6);
        
        // Root has 2 children
        REQUIRE(std::distance(g[0].edges().begin(), g[0].edges().end()) == 2);
        
        // Internal nodes have 2 children each
        REQUIRE(std::distance(g[1].edges().begin(), g[1].edges().end()) == 2);
        REQUIRE(std::distance(g[2].edges().begin(), g[2].edges().end()) == 2);
        
        // Leaves have no children
        for (uint32_t i = 3; i <= 6; ++i) {
            REQUIRE(g[i].edges().empty());
        }
    }
}
