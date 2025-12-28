/**
 * @file test_dynamic_graph_uous.cpp
 * @brief Tests for dynamic_graph with unordered_map vertices + unordered_set edges
 * 
 * Phase 4.2.5: Unordered Map Vertex + Unordered Set Edge Containers
 * Tests uous_graph_traits (unordered_map vertices + unordered_set edges)
 * 
 * Key characteristics:
 * - Vertices: std::unordered_map (hash-based; key-based lookup; forward iteration only)
 * - Edges: std::unordered_set (hash-based; automatic deduplication; unordered)
 * - Sparse vertex IDs - only referenced vertices are created
 * - Vertex IDs can be any hashable type (int, string, custom struct with std::hash)
 * - O(1) average vertex insertion, lookup, and deletion
 * - O(1) average edge insertion, lookup, and deletion
 * - Forward iterators only for both vertices and edges
 * - Edge deduplication - no parallel edges with same endpoints
 * - Both vertices and edges stored in unordered fashion (insertion order not preserved)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/uous_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <unordered_set>

using namespace graph::container;

// Type aliases for common test configurations with uint32_t vertex IDs
using uous_void_void_void = dynamic_graph<void, void, void, uint32_t, false, uous_graph_traits<void, void, void, uint32_t, false>>;
using uous_int_void_void = dynamic_graph<int, void, void, uint32_t, false, uous_graph_traits<int, void, void, uint32_t, false>>;
using uous_void_int_void = dynamic_graph<void, int, void, uint32_t, false, uous_graph_traits<void, int, void, uint32_t, false>>;
using uous_int_int_void = dynamic_graph<int, int, void, uint32_t, false, uous_graph_traits<int, int, void, uint32_t, false>>;
using uous_void_void_int = dynamic_graph<void, void, int, uint32_t, false, uous_graph_traits<void, void, int, uint32_t, false>>;
using uous_int_int_int = dynamic_graph<int, int, int, uint32_t, false, uous_graph_traits<int, int, int, uint32_t, false>>;

// Type aliases with string vertex IDs (the primary use case for map containers)
using uous_str_void_void_void = dynamic_graph<void, void, void, std::string, false, uous_graph_traits<void, void, void, std::string, false>>;
using uous_str_int_void_void = dynamic_graph<int, void, void, std::string, false, uous_graph_traits<int, void, void, std::string, false>>;
using uous_str_void_int_void = dynamic_graph<void, int, void, std::string, false, uous_graph_traits<void, int, void, std::string, false>>;
using uous_str_int_int_int = dynamic_graph<int, int, int, std::string, false, uous_graph_traits<int, int, int, std::string, false>>;

using uous_sourced = dynamic_graph<void, void, void, uint32_t, true, uous_graph_traits<void, void, void, uint32_t, true>>;
using uous_int_sourced = dynamic_graph<int, void, void, uint32_t, true, uous_graph_traits<int, void, void, uint32_t, true>>;

using uous_str_sourced = dynamic_graph<void, void, void, std::string, true, uous_graph_traits<void, void, void, std::string, true>>;

// Edge and vertex data types for loading
using edge_void = copyable_edge_t<uint32_t, void>;
using edge_int = copyable_edge_t<uint32_t, int>;
using vertex_int = copyable_vertex_t<uint32_t, int>;

// Helper function to count total edges in graph
template<typename G>
size_t count_all_edges(G& g) {
    size_t count = 0;
    for (auto& v : g) {
        count += static_cast<size_t>(std::ranges::distance(v.second.edges()));
    }
    return count;
}

//==================================================================================================
// 1. Traits Verification Tests
//==================================================================================================

TEST_CASE("uous traits verification", "[dynamic_graph][uous][traits]") {
    SECTION("vertices_type is std::unordered_map") {
        using traits = uous_graph_traits<void, void, void, uint32_t, false>;
        using vertices_t = typename traits::vertices_type;
        // Verify it's a map by checking for map-specific members
        static_assert(requires { typename vertices_t::key_type; });
        static_assert(requires { typename vertices_t::mapped_type; });
        REQUIRE(true);
    }

    SECTION("edges_type is std::set") {
        using traits = uous_graph_traits<void, void, void, uint32_t, false>;
        using edges_t = typename traits::edges_type;
        // unordered_set has key_type and doesn't have mapped_type (unlike map)
        static_assert(requires { typename edges_t::key_type; });
        REQUIRE(true);
    }

    SECTION("vertex_id_type can be string") {
        using traits = uous_graph_traits<void, void, void, std::string, false>;
        static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
        REQUIRE(true);
    }

    SECTION("sourced flag is preserved") {
        using traits_unsourced = uous_graph_traits<void, void, void, uint32_t, false>;
        using traits_sourced = uous_graph_traits<void, void, void, uint32_t, true>;
        static_assert(traits_unsourced::sourced == false);
        static_assert(traits_sourced::sourced == true);
        REQUIRE(true);
    }

    SECTION("vertex_id_type for uint32_t") {
        using traits = uous_graph_traits<void, void, void, uint32_t, false>;
        static_assert(std::same_as<typename traits::vertex_id_type, uint32_t>);
        REQUIRE(true);
    }

    SECTION("custom vertex_id_type") {
        using traits = uous_graph_traits<void, void, void, int64_t, false>;
        static_assert(std::same_as<typename traits::vertex_id_type, int64_t>);
        REQUIRE(true);
    }
}

//==================================================================================================
// 2. Iterator Category Tests
//==================================================================================================

TEST_CASE("uous iterator categories", "[dynamic_graph][uous][iterators]") {
    SECTION("underlying unordered_map iterators are forward only") {
        using G = uous_void_void_void;
        using iter_t = typename G::vertices_type::iterator;
        // unordered_map iterators are forward only (not bidirectional)
        static_assert(std::forward_iterator<iter_t>);
        static_assert(!std::bidirectional_iterator<iter_t>);
        static_assert(!std::random_access_iterator<iter_t>);
        REQUIRE(true);
    }

    SECTION("unordered_set edge iterators are forward only") {
        using traits = uous_graph_traits<void, void, void, uint32_t, false>;
        using edges_t = typename traits::edges_type;
        using edge_iter_t = typename edges_t::iterator;
        // unordered_set iterators are forward only (not bidirectional)
        static_assert(std::forward_iterator<edge_iter_t>);
        static_assert(!std::bidirectional_iterator<edge_iter_t>);
        // and NOT random access
        static_assert(!std::random_access_iterator<edge_iter_t>);
        REQUIRE(true);
    }

    SECTION("graph is a range") {
        static_assert(std::ranges::range<uous_void_void_void>);
        static_assert(std::ranges::range<uous_int_int_int>);
        static_assert(std::ranges::range<uous_str_void_void_void>);
        REQUIRE(true);
    }
}

//==================================================================================================
// 3. Construction Tests
//==================================================================================================

TEST_CASE("uous construction", "[dynamic_graph][uous][construction]") {
    SECTION("default constructor creates empty graph") {
        uous_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with void types") {
        uous_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int edge values") {
        uous_int_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int vertex values") {
        uous_void_int_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int graph value") {
        uous_void_void_int g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with all int values") {
        uous_int_int_int g;
        REQUIRE(g.size() == 0);
    }

    SECTION("constructor with graph value - int GV") {
        uous_void_void_int g(42);
        REQUIRE(g.size() == 0);
        REQUIRE(g.graph_value() == 42);
    }

    SECTION("copy constructor") {
        uous_int_int_int g1;
        uous_int_int_int g2(g1);
        REQUIRE(g2.size() == g1.size());
    }

    SECTION("move constructor") {
        uous_int_int_int g1;
        uous_int_int_int g2(std::move(g1));
        REQUIRE(g2.size() == 0);
    }

    SECTION("copy assignment") {
        uous_int_int_int g1, g2;
        g2 = g1;
        REQUIRE(g2.size() == g1.size());
    }

    SECTION("move assignment") {
        uous_int_int_int g1, g2;
        g2 = std::move(g1);
        REQUIRE(g2.size() == 0);
    }
}

TEST_CASE("uous construction with string vertex IDs", "[dynamic_graph][uous][construction][string]") {
    SECTION("default constructor creates empty graph") {
        uous_str_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int edge values") {
        uous_str_int_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int vertex values") {
        uous_str_void_int_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with all int values") {
        uous_str_int_int_int g;
        REQUIRE(g.size() == 0);
    }
}

TEST_CASE("uous construction sourced", "[dynamic_graph][uous][construction][sourced]") {
    SECTION("sourced edge construction with uint32_t IDs") {
        uous_sourced g;
        REQUIRE(g.size() == 0);
    }

    SECTION("sourced with edge value construction") {
        uous_int_sourced g;
        REQUIRE(g.size() == 0);
    }

    SECTION("sourced edge construction with string IDs") {
        uous_str_sourced g;
        REQUIRE(g.size() == 0);
    }
}

//==================================================================================================
// 4. Basic Properties Tests
//==================================================================================================

TEST_CASE("uous properties", "[dynamic_graph][uous][properties]") {
    SECTION("size() on empty graph") {
        uous_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("const graph methods") {
        const uous_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("begin() == end() for empty graph") {
        uous_void_void_void g;
        REQUIRE(g.begin() == g.end());
    }

    SECTION("const begin() == const end() for empty graph") {
        const uous_void_void_void g;
        REQUIRE(g.begin() == g.end());
    }

    SECTION("cbegin() == cend() for empty graph") {
        uous_void_void_void g;
        REQUIRE(g.cbegin() == g.cend());
    }
}

TEST_CASE("uous properties with string IDs", "[dynamic_graph][uous][properties][string]") {
    SECTION("size() on empty graph") {
        uous_str_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("begin() == end() for empty graph") {
        uous_str_void_void_void g;
        REQUIRE(g.begin() == g.end());
    }
}

//==================================================================================================
// 5. Initializer List Construction Tests (uint32_t vertex IDs)
//==================================================================================================

TEST_CASE("uous initializer_list construction", "[dynamic_graph][uous][initializer_list]") {
    SECTION("empty initializer list") {
        using G = uous_void_void_void;
        G g({});
        REQUIRE(g.size() == 0);
    }

    SECTION("single edge without value") {
        using G = uous_void_void_void;
        G g({{0, 1}});
        REQUIRE(g.size() == 2);
    }

    SECTION("single edge with value") {
        using G = uous_int_void_void;
        G g({{0, 1, 42}});
        REQUIRE(g.size() == 2);
    }

    SECTION("multiple edges from same source") {
        using G = uous_int_void_void;
        G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
        REQUIRE(g.size() == 4);
    }

    SECTION("triangle graph") {
        using G = uous_void_void_void;
        G g({{0, 1}, {1, 2}, {2, 0}});
        REQUIRE(g.size() == 3);
    }

    SECTION("self-loop") {
        using G = uous_void_void_void;
        G g({{0, 0}});
        REQUIRE(g.size() == 1);
    }

    SECTION("sparse vertex IDs - only referenced vertices created") {
        using G = uous_void_void_void;
        G g({{100, 200}});
        REQUIRE(g.size() == 2);
        // With uous, only vertices 100 and 200 are created
    }

    SECTION("star graph (center with many spokes)") {
        using G = uous_int_void_void;
        G g({{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4}, {0, 5, 5}});
        REQUIRE(g.size() == 6);
    }

    SECTION("construction with graph value") {
        using G = uous_void_void_int;
        G g(42, {{0, 1}, {1, 2}});
        REQUIRE(g.graph_value() == 42);
        REQUIRE(g.size() == 3);
    }
}

//==================================================================================================
// 6. Set-Specific Behavior: Deduplication Tests
//==================================================================================================

TEST_CASE("uous edge deduplication", "[dynamic_graph][uous][unordered_set][deduplication]") {
    SECTION("duplicate edges are ignored - unsourced") {
        uous_void_void_void g;
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
    }

    SECTION("duplicate edges with different values - first value wins") {
        uous_int_void_void g;
        std::vector<edge_int> ee = {
            {0, 1, 100}, {0, 1, 200}, {0, 1, 300}  // Same edge, different values
        };
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 2);
        REQUIRE(count_all_edges(g) == 1);  // Only one edge stored
        
        // First inserted value should be kept
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        REQUIRE(it->second.edges().begin()->value() == 100);
    }

    SECTION("parallel edges NOT allowed - unlike mofl") {
        uous_int_void_void g({{0, 1, 1}, {0, 1, 2}, {0, 1, 3}});
        REQUIRE(g.size() == 2);
        // Only one 0->1 edge should exist (deduplication)
        REQUIRE(count_all_edges(g) == 1);
    }

    SECTION("sourced edges - deduplication by (source_id, target_id)") {
        uous_sourced g;
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
// 7. Set-Specific Behavior: Sorted Order Tests
//==================================================================================================

TEST_CASE("uous edges are unordered by target_id", "[dynamic_graph][uous][unordered_set][unordered]") {
    SECTION("unsourced edges unordered by target_id") {
        // Insert edges in ununordered order
        uous_void_void_void g;
        std::vector<edge_void> ee = {
            {0, 5}, {0, 2}, {0, 8}, {0, 1}, {0, 3}
        };
        g.load_edges(ee, std::identity{});
        
        // Edges from vertex 0 should be in unordered order by target_id
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        std::vector<uint32_t> target_ids;
        for (const auto& edge : it->second.edges()) {
            target_ids.push_back(edge.target_id());
        }
        std::ranges::sort(target_ids);
        
        REQUIRE(target_ids == std::vector<uint32_t>{1, 2, 3, 5, 8});
    }

    SECTION("sourced edges unordered by target_id") {
        uous_sourced g;
        std::vector<edge_void> ee = {
            {0, 7}, {0, 3}, {0, 9}, {0, 1}
        };
        g.load_edges(ee, std::identity{});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        std::vector<uint32_t> target_ids;
        for (const auto& edge : it->second.edges()) {
            target_ids.push_back(edge.target_id());
        }
        std::ranges::sort(target_ids);
        
        REQUIRE(target_ids == std::vector<uint32_t>{1, 3, 7, 9});
    }
}

//==================================================================================================
// 8. Initializer List Construction Tests (string vertex IDs)
//==================================================================================================

TEST_CASE("uous initializer_list construction string IDs", "[dynamic_graph][uous][initializer_list][string]") {
    SECTION("single edge with string IDs") {
        using G = uous_str_void_void_void;
        G g({{"alice", "bob"}});
        REQUIRE(g.size() == 2);
    }

    SECTION("string IDs with edge values") {
        using G = uous_str_int_void_void;
        G g({{"alice", "bob", 10}, {"bob", "charlie", 20}});
        REQUIRE(g.size() == 3);
    }

    SECTION("social network graph") {
        using G = uous_str_int_void_void;
        G g({
            {"alice", "bob", 5},
            {"alice", "charlie", 3},
            {"bob", "charlie", 4},
            {"bob", "dave", 2},
            {"charlie", "eve", 5}
        });
        REQUIRE(g.size() == 5);
    }

    SECTION("sourced edges with string IDs") {
        using G = uous_str_sourced;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        REQUIRE(g.size() == 3);
    }

    SECTION("string deduplication") {
        using G = uous_str_void_void_void;
        G g({{"alice", "bob"}, {"alice", "bob"}, {"alice", "bob"}});
        REQUIRE(g.size() == 2);
        REQUIRE(count_all_edges(g) == 1);  // Only one edge due to deduplication
    }
}

//==================================================================================================
// 9. Graph Value Tests
//==================================================================================================

TEST_CASE("uous graph value access", "[dynamic_graph][uous][graph_value]") {
    SECTION("graph_value() returns reference") {
        uous_void_void_int g(100);
        REQUIRE(g.graph_value() == 100);
        g.graph_value() = 200;
        REQUIRE(g.graph_value() == 200);
    }

    SECTION("const graph_value()") {
        const uous_void_void_int g(42);
        REQUIRE(g.graph_value() == 42);
    }

    SECTION("graph value preserved through copy") {
        uous_void_void_int g1(100);
        uous_void_void_int g2 = g1;
        REQUIRE(g2.graph_value() == 100);
        g2.graph_value() = 200;
        REQUIRE(g1.graph_value() == 100); // g1 unchanged
    }

    SECTION("graph value preserved through move") {
        uous_void_void_int g1(100);
        uous_void_void_int g2 = std::move(g1);
        REQUIRE(g2.graph_value() == 100);
    }
}

//==================================================================================================
// 10. Graph Iteration Tests
//==================================================================================================

TEST_CASE("uous graph iteration", "[dynamic_graph][uous][iteration]") {
    SECTION("iterate over empty graph") {
        uous_void_void_void g;
        size_t count = 0;
        for ([[maybe_unused]] auto& v : g) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("iterate over graph with vertices") {
        using G = uous_void_void_void;
        G g({{0, 1}, {1, 2}});
        
        size_t count = 0;
        for ([[maybe_unused]] auto& v : g) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("const iteration") {
        using G = uous_void_void_void;
        const G g({{0, 1}, {1, 2}});
        
        size_t count = 0;
        for ([[maybe_unused]] const auto& v : g) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("iterate string key graph") {
        using G = uous_str_void_void_void;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        
        size_t count = 0;
        for ([[maybe_unused]] auto& v : g) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("vertices in unordered key order") {
        uous_void_void_void g({{5, 1}, {3, 2}, {7, 4}, {1, 6}});
        
        std::vector<uint32_t> vertex_ids;
        for (auto& v : g) {
            vertex_ids.push_back(v.first);
        }
        std::ranges::sort(vertex_ids);
        
        // unordered_map doesn't guarantee order - sort and verify contents
        REQUIRE(vertex_ids.size() == 7);  // All 7 vertices: 1,2,3,4,5,6,7
        REQUIRE(vertex_ids == std::vector<uint32_t>{1, 2, 3, 4, 5, 6, 7});
    }
}

//==================================================================================================
// 11. Vertex Accessor Methods Tests
//==================================================================================================

TEST_CASE("uous contains_vertex", "[dynamic_graph][uous][accessor][contains_vertex]") {
    SECTION("uint32_t vertex IDs") {
        using G = uous_void_void_void;
        G g({{0, 1}, {1, 2}, {5, 10}});
        
        // Vertices that exist
        REQUIRE(g.contains_vertex(0));
        REQUIRE(g.contains_vertex(1));
        REQUIRE(g.contains_vertex(2));
        REQUIRE(g.contains_vertex(5));
        REQUIRE(g.contains_vertex(10));
        
        // Vertices that don't exist (sparse - not like vector)
        REQUIRE_FALSE(g.contains_vertex(3));
        REQUIRE_FALSE(g.contains_vertex(4));
        REQUIRE_FALSE(g.contains_vertex(6));
        REQUIRE_FALSE(g.contains_vertex(100));
    }
    
    SECTION("string vertex IDs") {
        using G = uous_str_void_void_void;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        
        REQUIRE(g.contains_vertex("alice"));
        REQUIRE(g.contains_vertex("bob"));
        REQUIRE(g.contains_vertex("charlie"));
        
        REQUIRE_FALSE(g.contains_vertex("david"));
        REQUIRE_FALSE(g.contains_vertex(""));
        REQUIRE_FALSE(g.contains_vertex("Alice")); // case sensitive
    }
    
    SECTION("empty graph") {
        using G = uous_void_void_void;
        G g;
        
        REQUIRE_FALSE(g.contains_vertex(0));
        REQUIRE_FALSE(g.contains_vertex(1));
    }
    
    SECTION("const graph") {
        using G = uous_void_void_void;
        const G g({{0, 1}, {2, 3}});
        
        REQUIRE(g.contains_vertex(0));
        REQUIRE(g.contains_vertex(1));
        REQUIRE_FALSE(g.contains_vertex(5));
    }
}

TEST_CASE("uous try_find_vertex", "[dynamic_graph][uous][accessor][try_find_vertex]") {
    SECTION("uint32_t vertex IDs - found") {
        using G = uous_void_void_void;
        G g({{0, 1}, {1, 2}, {5, 10}});
        
        auto it0 = g.try_find_vertex(0);
        REQUIRE(it0 != g.end());
        REQUIRE(it0->first == 0);
        
        auto it5 = g.try_find_vertex(5);
        REQUIRE(it5 != g.end());
        REQUIRE(it5->first == 5);
        
        auto it10 = g.try_find_vertex(10);
        REQUIRE(it10 != g.end());
        REQUIRE(it10->first == 10);
    }
    
    SECTION("uint32_t vertex IDs - not found") {
        using G = uous_void_void_void;
        G g({{0, 1}, {5, 10}});
        
        auto it3 = g.try_find_vertex(3);
        REQUIRE(it3 == g.end());
        
        auto it100 = g.try_find_vertex(100);
        REQUIRE(it100 == g.end());
    }
    
    SECTION("string vertex IDs") {
        using G = uous_str_void_void_void;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto it_alice = g.try_find_vertex("alice");
        REQUIRE(it_alice != g.end());
        REQUIRE(it_alice->first == "alice");
        
        auto it_david = g.try_find_vertex("david");
        REQUIRE(it_david == g.end());
    }
    
    SECTION("does not modify container") {
        using G = uous_void_void_void;
        G g({{0, 1}});
        REQUIRE(g.size() == 2);
        
        auto it = g.try_find_vertex(999);
        REQUIRE(it == g.end());
        REQUIRE(g.size() == 2);  // Size unchanged
    }
    
    SECTION("const graph") {
        using G = uous_void_void_void;
        const G g({{0, 1}, {2, 3}});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        REQUIRE(it->first == 0);
        
        auto it_missing = g.try_find_vertex(99);
        REQUIRE(it_missing == g.end());
    }
}

TEST_CASE("uous vertex_at", "[dynamic_graph][uous][accessor][vertex_at]") {
    SECTION("uint32_t vertex IDs - found") {
        using G = uous_void_void_void;
        G g({{0, 1}, {1, 2}});
        
        REQUIRE_NOTHROW(g.vertex_at(0));
        REQUIRE_NOTHROW(g.vertex_at(1));
        REQUIRE_NOTHROW(g.vertex_at(2));
    }
    
    SECTION("uint32_t vertex IDs - throws on not found") {
        using G = uous_void_void_void;
        G g({{0, 1}});
        
        REQUIRE_THROWS_AS(g.vertex_at(5), std::out_of_range);
        REQUIRE_THROWS_AS(g.vertex_at(100), std::out_of_range);
    }
    
    SECTION("string vertex IDs") {
        using G = uous_str_void_void_void;
        G g({{"alice", "bob"}});
        
        REQUIRE_NOTHROW(g.vertex_at("alice"));
        REQUIRE_NOTHROW(g.vertex_at("bob"));
        REQUIRE_THROWS_AS(g.vertex_at("charlie"), std::out_of_range);
    }
    
    SECTION("modify vertex through vertex_at") {
        using G = uous_void_int_void;  // has vertex value
        G g({{0, 1}});
        
        g.vertex_at(0).value() = 42;
        g.vertex_at(1).value() = 100;
        
        REQUIRE(g.vertex_at(0).value() == 42);
        REQUIRE(g.vertex_at(1).value() == 100);
    }
    
    SECTION("const graph") {
        using G = uous_void_void_void;
        const G g({{0, 1}, {2, 3}});
        
        REQUIRE_NOTHROW(g.vertex_at(0));
        REQUIRE_THROWS_AS(g.vertex_at(99), std::out_of_range);
    }
    
    SECTION("does not modify container") {
        using G = uous_void_void_void;
        G g({{0, 1}});
        REQUIRE(g.size() == 2);
        
        REQUIRE_THROWS(g.vertex_at(999));
        REQUIRE(g.size() == 2);
    }
}

//==================================================================================================
// 12. load_vertices Tests
//==================================================================================================

TEST_CASE("uous load_vertices", "[dynamic_graph][uous][load_vertices]") {
    SECTION("uint32_t IDs - basic load") {
        using G = uous_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}, {1, 2}});
        REQUIRE(g.size() == 3);
        
        std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.vertex_at(0).value() == 100);
        REQUIRE(g.vertex_at(1).value() == 200);
        REQUIRE(g.vertex_at(2).value() == 300);
    }
    
    SECTION("uint32_t IDs - load creates new vertices") {
        using G = uous_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g;
        REQUIRE(g.size() == 0);
        
        std::vector<vertex_data> vv = {{10, 100}, {20, 200}, {30, 300}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.vertex_at(10).value() == 100);
        REQUIRE(g.vertex_at(20).value() == 200);
        REQUIRE(g.vertex_at(30).value() == 300);
    }
    
    SECTION("string IDs - basic load") {
        using G = uous_str_void_int_void;
        using vertex_data = copyable_vertex_t<std::string, int>;
        
        G g({{"alice", "bob"}});
        REQUIRE(g.size() == 2);
        
        std::vector<vertex_data> vv = {{"alice", 100}, {"bob", 200}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.vertex_at("alice").value() == 100);
        REQUIRE(g.vertex_at("bob").value() == 200);
    }
}

//==================================================================================================
// 13. load_edges Tests
//==================================================================================================

TEST_CASE("uous load_edges explicit", "[dynamic_graph][uous][load_edges]") {
    SECTION("uint32_t IDs - basic load") {
        using G = uous_int_void_void;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        G g;
        REQUIRE(g.size() == 0);
        
        std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 4);
    }
    
    SECTION("uint32_t IDs - sparse vertex creation") {
        using G = uous_void_void_void;
        using edge_data = copyable_edge_t<uint32_t, void>;
        
        G g;
        
        std::vector<edge_data> ee = {{100, 200}, {300, 400}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 4);  // Only 4 vertices, not 401
        REQUIRE(g.contains_vertex(100));
        REQUIRE(g.contains_vertex(200));
        REQUIRE(g.contains_vertex(300));
        REQUIRE(g.contains_vertex(400));
        REQUIRE_FALSE(g.contains_vertex(0));
        REQUIRE_FALSE(g.contains_vertex(150));
    }
    
    SECTION("deduplication during load") {
        using G = uous_int_void_void;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        G g;
        
        // Load with duplicates
        std::vector<edge_data> ee = {
            {0, 1, 100}, {0, 1, 200}, {0, 1, 300},  // Same edge, different values
            {0, 2, 400}
        };
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(count_all_edges(g) == 2);  // Only 2 unique edges
    }
}

//==================================================================================================
// 14. Edge Cases and Error Handling
//==================================================================================================

TEST_CASE("uous edge cases", "[dynamic_graph][uous][edge_cases]") {
    SECTION("graph with single vertex (self-loop)") {
        using G = uous_void_void_void;
        G g({{0, 0}});
        REQUIRE(g.size() == 1);
        REQUIRE(count_all_edges(g) == 1);
    }

    SECTION("self-loop deduplication") {
        using G = uous_void_void_void;
        G g({{0, 0}, {0, 0}, {0, 0}});
        REQUIRE(g.size() == 1);
        REQUIRE(count_all_edges(g) == 1);  // Only one self-loop
    }

    SECTION("clear() empties the graph") {
        using G = uous_int_void_void;
        G g({{0, 1, 10}, {1, 2, 20}});
        REQUIRE(g.size() == 3);
        g.clear();
        REQUIRE(g.size() == 0);
    }

    SECTION("multiple clears are safe") {
        using G = uous_void_void_void;
        G g({{0, 1}});
        g.clear();
        g.clear();
        g.clear();
        REQUIRE(g.size() == 0);
    }

    SECTION("swap two graphs") {
        using G = uous_void_void_int;
        G g1(100, {{0, 1}});
        G g2(200, {{1, 2}, {2, 3}});
        
        REQUIRE(g1.graph_value() == 100);
        REQUIRE(g1.size() == 2);
        REQUIRE(g2.graph_value() == 200);
        REQUIRE(g2.size() == 3);
        
        std::swap(g1, g2);
        
        REQUIRE(g1.graph_value() == 200);
        REQUIRE(g1.size() == 3);
        REQUIRE(g2.graph_value() == 100);
        REQUIRE(g2.size() == 2);
    }

    SECTION("large sparse vertex IDs") {
        using G = uous_void_void_void;
        G g({{1000000, 2000000}});
        REQUIRE(g.size() == 2);  // Only 2 vertices, not 2000001
    }
}

//==================================================================================================
// 15. Const Correctness Tests
//==================================================================================================

TEST_CASE("uous const correctness", "[dynamic_graph][uous][const]") {
    SECTION("const graph properties") {
        using G = uous_int_void_void;
        const G g({{0, 1, 10}, {1, 2, 20}});
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.begin() != g.end());
    }

    SECTION("const graph iteration") {
        using G = uous_int_void_void;
        const G g({{0, 1, 10}, {1, 2, 20}});
        
        size_t count = 0;
        for (auto it = g.cbegin(); it != g.cend(); ++it) {
            ++count;
        }
        REQUIRE(count == 3);
    }
}

//==================================================================================================
// 16. Memory and Resource Management Tests
//==================================================================================================

TEST_CASE("uous memory management", "[dynamic_graph][uous][memory]") {
    SECTION("multiple independent graphs") {
        using G = uous_void_void_int;
        G g1(100, {{0, 1}});
        G g2(200, {{1, 2}});
        G g3(300, {{2, 3}});
        
        REQUIRE(g1.graph_value() == 100);
        REQUIRE(g2.graph_value() == 200);
        REQUIRE(g3.graph_value() == 300);
    }

    SECTION("copy does not alias") {
        using G = uous_void_void_int;
        G g1(100, {{0, 1}});
        G g2 = g1;
        
        g2.graph_value() = 200;
        REQUIRE(g1.graph_value() == 100);
        REQUIRE(g2.graph_value() == 200);
    }

    SECTION("clear allows reuse") {
        using G = uous_int_void_void;
        G g({{0, 1, 10}, {1, 2, 20}, {2, 3, 30}});
        REQUIRE(g.size() == 4);
        g.clear();
        REQUIRE(g.size() == 0);
        REQUIRE(g.begin() == g.end());
    }
}

//==================================================================================================
// 17. Template Instantiation Tests
//==================================================================================================

TEST_CASE("uous template instantiation", "[dynamic_graph][uous][compilation]") {
    [[maybe_unused]] uous_void_void_void g1;
    [[maybe_unused]] uous_int_void_void g2;
    [[maybe_unused]] uous_void_int_void g3;
    [[maybe_unused]] uous_int_int_void g4;
    [[maybe_unused]] uous_void_void_int g5;
    [[maybe_unused]] uous_int_int_int g6;
    [[maybe_unused]] uous_sourced g7;
    [[maybe_unused]] uous_int_sourced g8;
    [[maybe_unused]] uous_str_void_void_void g9;
    [[maybe_unused]] uous_str_int_void_void g10;
    [[maybe_unused]] uous_str_int_int_int g11;
    [[maybe_unused]] uous_str_sourced g12;
    
    REQUIRE(true);
}

//==================================================================================================
// 18. Sparse Vertex Behavior Tests
//==================================================================================================

TEST_CASE("uous sparse vertex behavior", "[dynamic_graph][uous][sparse]") {
    SECTION("only referenced vertices are created") {
        using G = uous_void_void_void;
        
        G g({{10, 20}});
        REQUIRE(g.size() == 2);
        
        // Unlike vector-based graphs, no vertices 0-9 or 11-19 exist
    }

    SECTION("multiple sparse edges") {
        using G = uous_void_void_void;
        G g({{100, 200}, {300, 400}, {500, 600}});
        REQUIRE(g.size() == 6);
    }

    SECTION("reverse order vertex creation") {
        using G = uous_void_void_void;
        G g({{100, 50}, {200, 25}});
        REQUIRE(g.size() == 4);
    }
}

//==================================================================================================
// 19. Edge Bidirectional Iteration Tests
//==================================================================================================

TEST_CASE("uous edge forward iteration only", "[dynamic_graph][uous][unordered_set][iteration]") {
    SECTION("forward iteration") {
        uous_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        std::vector<uint32_t> targets;
        for (const auto& edge : it->second.edges()) {
            targets.push_back(edge.target_id());
        }
        std::ranges::sort(targets);
        
        REQUIRE(targets.size() == 3);
        REQUIRE(targets == std::vector<uint32_t>{1, 2, 3});  // After sorting
    }

    SECTION("forward iteration only - no reverse") {
        uous_void_void_void g({{0, 1}, {0, 2}, {0, 3}});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        auto& edge_unordered_set = it->second.edges();
        
        // unordered_set doesn't support reverse iteration (forward iterators only)
        std::vector<uint32_t> targets;
        for (auto eit = edge_unordered_set.begin(); eit != edge_unordered_set.end(); ++eit) {
            targets.push_back(eit->target_id());
        }
        std::ranges::sort(targets);
        
        REQUIRE(targets == std::vector<uint32_t>{1, 2, 3});  // After sorting
    }
}

//==================================================================================================
// 20. Sourced Edge Tests
//==================================================================================================

TEST_CASE("uous sourced edges", "[dynamic_graph][uous][sourced]") {
    SECTION("source_id access") {
        uous_sourced g({{0, 1}, {0, 2}, {1, 0}});
        
        auto it0 = g.try_find_vertex(0);
        REQUIRE(it0 != g.end());
        for (const auto& edge : it0->second.edges()) {
            REQUIRE(edge.source_id() == 0);
        }
        
        auto it1 = g.try_find_vertex(1);
        REQUIRE(it1 != g.end());
        for (const auto& edge : it1->second.edges()) {
            REQUIRE(edge.source_id() == 1);
        }
    }

    SECTION("sourced edge with values") {
        uous_int_sourced g;
        std::vector<edge_int> ee = {
            {0, 1, 100}, {1, 0, 200}
        };
        g.load_edges(ee, std::identity{});
        
        auto it0 = g.try_find_vertex(0);
        REQUIRE(it0 != g.end());
        auto e0 = it0->second.edges().begin();
        REQUIRE(e0->source_id() == 0);
        REQUIRE(e0->target_id() == 1);
        REQUIRE(e0->value() == 100);
        
        auto it1 = g.try_find_vertex(1);
        REQUIRE(it1 != g.end());
        auto e1 = it1->second.edges().begin();
        REQUIRE(e1->source_id() == 1);
        REQUIRE(e1->target_id() == 0);
        REQUIRE(e1->value() == 200);
    }
}
