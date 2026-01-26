/**
 * @file test_dynamic_graph_mod.cpp
 * @brief Phase 3.1 Tests for dynamic_graph with mod_graph_traits
 * 
 * Tests core functionality of dynamic_graph with map vertices + deque edges.
 * This mirrors test_dynamic_graph_mov.cpp but uses std::deque for edges instead of std::vector.
 * 
 * Container: map<VId, vertex> + deque<edge>
 * 
 * Key characteristics of mod_graph_traits:
 * - std::map for vertices: ordered by key, O(log n) lookup, sparse vertex IDs
 * - std::deque for edges: random access, efficient front/back insertion
 * - String vertex IDs are natural use case
 * - Forward iteration for both vertices and edges (via descriptor iterators)
 * - Edge order: first added appears first (like std::vector)
 *
 * Key differences from mov (map + vector):
 * - std::deque provides efficient front and back insertion (vs vector's back-only efficiency)
 * - std::deque may have slightly different memory layout (chunked vs contiguous)
 * - Both have random access iterators with similar edge ordering semantics
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/mod_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

//==================================================================================================
// 1. Type Aliases for Common Test Configurations (uint32_t vertex IDs)
//==================================================================================================

// Format: mod_EV_VV_GV (Edge Value, Vertex Value, Graph Value)
// "void" means no value (using graph::void_t)
using mod_void_void_void = dynamic_graph<void, void, void, uint32_t, false, mod_graph_traits<void, void, void, uint32_t, false>>;
using mod_int_void_void  = dynamic_graph<int, void, void, uint32_t, false, mod_graph_traits<int, void, void, uint32_t, false>>;
using mod_void_int_void  = dynamic_graph<void, int, void, uint32_t, false, mod_graph_traits<void, int, void, uint32_t, false>>;
using mod_int_int_void   = dynamic_graph<int, int, void, uint32_t, false, mod_graph_traits<int, int, void, uint32_t, false>>;
using mod_void_void_int  = dynamic_graph<void, void, int, uint32_t, false, mod_graph_traits<void, void, int, uint32_t, false>>;
using mod_int_int_int    = dynamic_graph<int, int, int, uint32_t, false, mod_graph_traits<int, int, int, uint32_t, false>>;

// String vertex ID variants
using mod_str_void_void_void = dynamic_graph<void, void, void, std::string, false, mod_graph_traits<void, void, void, std::string, false>>;
using mod_str_int_void_void  = dynamic_graph<int, void, void, std::string, false, mod_graph_traits<int, void, void, std::string, false>>;
using mod_str_void_int_void  = dynamic_graph<void, int, void, std::string, false, mod_graph_traits<void, int, void, std::string, false>>;
using mod_str_int_int_int    = dynamic_graph<int, int, int, std::string, false, mod_graph_traits<int, int, int, std::string, false>>;

// Sourced edge variants (store source vertex ID in edge)
using mod_sourced     = dynamic_graph<void, void, void, uint32_t, true, mod_graph_traits<void, void, void, uint32_t, true>>;
using mod_int_sourced = dynamic_graph<int, void, void, uint32_t, true, mod_graph_traits<int, void, void, uint32_t, true>>;
using mod_str_sourced = dynamic_graph<void, void, void, std::string, true, mod_graph_traits<void, void, void, std::string, true>>;

//==================================================================================================
// 2. Traits Verification Tests
//==================================================================================================

TEST_CASE("mod traits verification", "[dynamic_graph][mod][traits]") {
    SECTION("vertices container type is map") {
        using traits = mod_graph_traits<void, void, void, uint32_t, false>;
        using vertices_type = typename traits::vertices_type;
        
        // Should be std::map<VId, vertex_type>
        static_assert(std::same_as<typename vertices_type::key_type, uint32_t>);
        REQUIRE(true);
    }
    
    SECTION("edges container type is deque") {
        using traits = mod_graph_traits<void, void, void, uint32_t, false>;
        using edges_type = typename traits::edges_type;
        
        // Should be std::deque<edge_type>
        static_assert(std::same_as<edges_type, std::deque<typename traits::edge_type>>);
        REQUIRE(true);
    }
    
    SECTION("vertex iterator is bidirectional (map iterators)") {
        using G = mod_void_void_void;
        using vertices_type = typename G::graph_traits::vertices_type;
        using iterator = typename vertices_type::iterator;
        
        static_assert(std::bidirectional_iterator<iterator>);
        REQUIRE(true);
    }
    
    SECTION("edge iterator is random access (deque iterators)") {
        using traits = mod_graph_traits<void, void, void, uint32_t, false>;
        using edges_type = typename traits::edges_type;
        using iterator = typename edges_type::iterator;
        
        static_assert(std::random_access_iterator<iterator>);
        REQUIRE(true);
    }
    
    SECTION("string vertex ID type") {
        using traits = mod_graph_traits<void, void, void, std::string, false>;
        static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
        REQUIRE(true);
    }
}

//==================================================================================================
// 3. Construction Tests
//==================================================================================================

TEST_CASE("mod construction", "[dynamic_graph][mod][construction]") {
    SECTION("default constructor creates empty graph") {
        mod_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with void types") {
        mod_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int edge values") {
        mod_int_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int vertex values") {
        mod_void_int_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int graph value") {
        mod_void_void_int g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with all int values") {
        mod_int_int_int g;
        REQUIRE(g.size() == 0);
    }

    SECTION("constructor with graph value - int GV") {
        mod_void_void_int g(42);
        REQUIRE(g.size() == 0);
        REQUIRE(g.graph_value() == 42);
    }

    SECTION("copy constructor") {
        mod_int_int_int g1;
        mod_int_int_int g2(g1);
        REQUIRE(g2.size() == g1.size());
    }

    SECTION("move constructor") {
        mod_int_int_int g1;
        mod_int_int_int g2(std::move(g1));
        REQUIRE(g2.size() == 0);
    }

    SECTION("copy assignment") {
        mod_int_int_int g1, g2;
        g2 = g1;
        REQUIRE(g2.size() == g1.size());
    }

    SECTION("move assignment") {
        mod_int_int_int g1, g2;
        g2 = std::move(g1);
        REQUIRE(g2.size() == 0);
    }
}

TEST_CASE("mod construction with string vertex IDs", "[dynamic_graph][mod][construction][string]") {
    SECTION("default constructor creates empty graph") {
        mod_str_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int edge values") {
        mod_str_int_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with int vertex values") {
        mod_str_void_int_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("default constructor with all int values") {
        mod_str_int_int_int g;
        REQUIRE(g.size() == 0);
    }
}

TEST_CASE("mod construction sourced", "[dynamic_graph][mod][construction][sourced]") {
    SECTION("sourced edge construction with uint32_t IDs") {
        mod_sourced g;
        REQUIRE(g.size() == 0);
    }

    SECTION("sourced with edge value construction") {
        mod_int_sourced g;
        REQUIRE(g.size() == 0);
    }

    SECTION("sourced edge construction with string IDs") {
        mod_str_sourced g;
        REQUIRE(g.size() == 0);
    }
}

//==================================================================================================
// 4. Basic Properties Tests
//==================================================================================================

TEST_CASE("mod properties", "[dynamic_graph][mod][properties]") {
    SECTION("size() on empty graph") {
        mod_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("const graph methods") {
        const mod_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("begin() == end() for empty graph") {
        mod_void_void_void g;
        REQUIRE(g.begin() == g.end());
    }

    SECTION("const begin() == const end() for empty graph") {
        const mod_void_void_void g;
        REQUIRE(g.begin() == g.end());
    }

    SECTION("cbegin() == cend() for empty graph") {
        mod_void_void_void g;
        REQUIRE(g.cbegin() == g.cend());
    }
}

TEST_CASE("mod properties with string IDs", "[dynamic_graph][mod][properties][string]") {
    SECTION("size() on empty graph") {
        mod_str_void_void_void g;
        REQUIRE(g.size() == 0);
    }

    SECTION("begin() == end() for empty graph") {
        mod_str_void_void_void g;
        REQUIRE(g.begin() == g.end());
    }
}

//==================================================================================================
// 5. Type Alias Tests
//==================================================================================================

TEST_CASE("mod type aliases", "[dynamic_graph][mod][types]") {
    SECTION("graph type aliases are correct") {
        using G = mod_int_int_int;
        static_assert(std::same_as<typename G::value_type, int>); // GV
        static_assert(G::sourced == false);
        REQUIRE(true);
    }

    SECTION("sourced graph type aliases are correct") {
        using G = mod_sourced;
        static_assert(G::sourced == true);
        REQUIRE(true);
    }

    SECTION("string key graph type aliases are correct") {
        using G = mod_str_int_int_int;
        using traits = typename G::graph_traits;
        static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
        REQUIRE(true);
    }
}

//==================================================================================================
// 6. Initializer List Construction Tests (uint32_t vertex IDs)
//==================================================================================================

TEST_CASE("mod initializer_list construction", "[dynamic_graph][mod][initializer_list]") {
    SECTION("empty initializer list") {
        using G = mod_void_void_void;
        G g({});
        REQUIRE(g.size() == 0);
    }

    SECTION("single edge without value") {
        using G = mod_void_void_void;
        G g({{0, 1}});
        REQUIRE(g.size() == 2);
    }

    SECTION("single edge with value") {
        using G = mod_int_void_void;
        G g({{0, 1, 42}});
        REQUIRE(g.size() == 2);
    }

    SECTION("multiple edges from same source") {
        using G = mod_int_void_void;
        G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
        REQUIRE(g.size() == 4);
    }

    SECTION("triangle graph") {
        using G = mod_void_void_void;
        G g({{0, 1}, {1, 2}, {2, 0}});
        REQUIRE(g.size() == 3);
    }

    SECTION("self-loop") {
        using G = mod_void_void_void;
        G g({{0, 0}});
        REQUIRE(g.size() == 1);
    }

    SECTION("parallel edges") {
        using G = mod_int_void_void;
        G g({{0, 1, 1}, {0, 1, 2}, {0, 1, 3}});
        REQUIRE(g.size() == 2);
    }

    SECTION("sparse vertex IDs - only referenced vertices created") {
        using G = mod_void_void_void;
        G g({{100, 200}});
        REQUIRE(g.size() == 2);
        // With mod, only vertices 100 and 200 are created
        // (unlike vector-based which would create 0..200)
    }

    SECTION("star graph (center with many spokes)") {
        using G = mod_int_void_void;
        G g({{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4}, {0, 5, 5}});
        REQUIRE(g.size() == 6);
    }

    SECTION("complete graph K4") {
        using G = mod_int_void_void;
        G g({
            {0, 1, 1}, {0, 2, 2}, {0, 3, 3},
            {1, 0, 4}, {1, 2, 5}, {1, 3, 6},
            {2, 0, 7}, {2, 1, 8}, {2, 3, 9},
            {3, 0, 10}, {3, 1, 11}, {3, 2, 12}
        });
        REQUIRE(g.size() == 4);
    }

    SECTION("construction with graph value") {
        using G = mod_void_void_int;
        G g(42, {{0, 1}, {1, 2}});
        REQUIRE(g.graph_value() == 42);
        REQUIRE(g.size() == 3);
    }
}

//==================================================================================================
// 7. Initializer List Construction Tests (string vertex IDs)
//==================================================================================================

TEST_CASE("mod initializer_list construction string IDs", "[dynamic_graph][mod][initializer_list][string]") {
    SECTION("single edge with string IDs") {
        using G = mod_str_void_void_void;
        G g({{"alice", "bob"}});
        REQUIRE(g.size() == 2);
    }

    SECTION("string IDs with edge values") {
        using G = mod_str_int_void_void;
        G g({{"alice", "bob", 10}, {"bob", "charlie", 20}});
        REQUIRE(g.size() == 3);
    }

    SECTION("social network graph") {
        using G = mod_str_int_void_void;
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
        using G = mod_str_sourced;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        REQUIRE(g.size() == 3);
    }
}

//==================================================================================================
// 8. Graph Value Tests
//==================================================================================================

TEST_CASE("mod graph value access", "[dynamic_graph][mod][graph_value]") {
    SECTION("graph_value() returns reference") {
        mod_void_void_int g(100);
        REQUIRE(g.graph_value() == 100);
        g.graph_value() = 200;
        REQUIRE(g.graph_value() == 200);
    }

    SECTION("const graph_value()") {
        const mod_void_void_int g(42);
        REQUIRE(g.graph_value() == 42);
    }

    SECTION("graph value preserved through copy") {
        mod_void_void_int g1(100);
        mod_void_void_int g2 = g1;
        REQUIRE(g2.graph_value() == 100);
        g2.graph_value() = 200;
        REQUIRE(g1.graph_value() == 100); // g1 unchanged
    }

    SECTION("graph value preserved through move") {
        mod_void_void_int g1(100);
        mod_void_void_int g2 = std::move(g1);
        REQUIRE(g2.graph_value() == 100);
    }
}

//==================================================================================================
// 9. Graph Iteration Tests (direct iteration, not via vertices() CPO)
//==================================================================================================

TEST_CASE("mod graph iteration", "[dynamic_graph][mod][iteration]") {
    SECTION("iterate over empty graph") {
        mod_void_void_void g;
        size_t count = 0;
        for ([[maybe_unused]] auto& v : g) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("iterate over graph with vertices") {
        using G = mod_void_void_void;
        G g({{0, 1}, {1, 2}});
        
        size_t count = 0;
        for ([[maybe_unused]] auto& v : g) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("const iteration") {
        using G = mod_void_void_void;
        const G g({{0, 1}, {1, 2}});
        
        size_t count = 0;
        for ([[maybe_unused]] const auto& v : g) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("iterate string key graph") {
        using G = mod_str_void_void_void;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        
        size_t count = 0;
        for ([[maybe_unused]] auto& v : g) {
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("vertices iterate in key order (map property)") {
        using G = mod_void_void_void;
        G g({{5, 10}, {1, 2}, {3, 4}});
        
        std::vector<uint32_t> keys;
        for (auto& v : g) {
            keys.push_back(v.first);  // Map iterator gives pair
        }
        
        // Should be sorted: 1, 2, 3, 4, 5, 10
        REQUIRE(std::is_sorted(keys.begin(), keys.end()));
    }
}

//==================================================================================================
// 10. Edge Cases and Error Handling
//==================================================================================================

TEST_CASE("mod edge cases", "[dynamic_graph][mod][edge_cases]") {
    SECTION("graph with single vertex (self-loop)") {
        using G = mod_void_void_void;
        G g({{0, 0}});
        REQUIRE(g.size() == 1);
    }

    SECTION("clear() empties the graph") {
        using G = mod_int_void_void;
        G g({{0, 1, 10}, {1, 2, 20}});
        REQUIRE(g.size() == 3);
        g.clear();
        REQUIRE(g.size() == 0);
    }

    SECTION("multiple clears are safe") {
        using G = mod_void_void_void;
        G g({{0, 1}});
        g.clear();
        g.clear();
        g.clear();
        REQUIRE(g.size() == 0);
    }

    SECTION("swap two graphs") {
        using G = mod_void_void_int;
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
        using G = mod_void_void_void;
        G g({{1000000, 2000000}});
        REQUIRE(g.size() == 2); // Only 2 vertices, not 2000001
    }
}

//==================================================================================================
// 11. Const Correctness Tests
//==================================================================================================

TEST_CASE("mod const correctness", "[dynamic_graph][mod][const]") {
    SECTION("const graph properties") {
        using G = mod_int_void_void;
        const G g({{0, 1, 10}, {1, 2, 20}});
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.begin() != g.end());
    }

    SECTION("const graph iteration") {
        using G = mod_int_void_void;
        const G g({{0, 1, 10}, {1, 2, 20}});
        
        size_t count = 0;
        for (auto it = g.cbegin(); it != g.cend(); ++it) {
            ++count;
        }
        REQUIRE(count == 3);
    }
}

//==================================================================================================
// 12. Memory and Resource Management Tests
//==================================================================================================

TEST_CASE("mod memory management", "[dynamic_graph][mod][memory]") {
    SECTION("multiple independent graphs") {
        using G = mod_void_void_int;
        G g1(100, {{0, 1}});
        G g2(200, {{1, 2}});
        G g3(300, {{2, 3}});
        
        REQUIRE(g1.graph_value() == 100);
        REQUIRE(g2.graph_value() == 200);
        REQUIRE(g3.graph_value() == 300);
    }

    SECTION("copy does not alias") {
        using G = mod_void_void_int;
        G g1(100, {{0, 1}});
        G g2 = g1;
        
        g2.graph_value() = 200;
        REQUIRE(g1.graph_value() == 100);
        REQUIRE(g2.graph_value() == 200);
    }

    SECTION("clear allows reuse") {
        using G = mod_int_void_void;
        G g({{0, 1, 10}, {1, 2, 20}, {2, 3, 30}});
        REQUIRE(g.size() == 4);
        g.clear();
        REQUIRE(g.size() == 0);
        REQUIRE(g.begin() == g.end());
    }
}

//==================================================================================================
// 13. Template Instantiation Tests
//==================================================================================================

TEST_CASE("mod template instantiation", "[dynamic_graph][mod][compilation]") {
    [[maybe_unused]] mod_void_void_void g1;
    [[maybe_unused]] mod_int_void_void g2;
    [[maybe_unused]] mod_void_int_void g3;
    [[maybe_unused]] mod_int_int_void g4;
    [[maybe_unused]] mod_void_void_int g5;
    [[maybe_unused]] mod_int_int_int g6;
    [[maybe_unused]] mod_sourced g7;
    [[maybe_unused]] mod_int_sourced g8;
    [[maybe_unused]] mod_str_void_void_void g9;
    [[maybe_unused]] mod_str_int_void_void g10;
    [[maybe_unused]] mod_str_int_int_int g11;
    [[maybe_unused]] mod_str_sourced g12;
    
    REQUIRE(true);
}

//==================================================================================================
// 14. Sparse Vertex Behavior (associative container specific)
//==================================================================================================

TEST_CASE("mod sparse vertex behavior", "[dynamic_graph][mod][sparse]") {
    SECTION("only referenced vertices are created") {
        using G = mod_void_void_void;
        
        // Edge from 10 to 20 - should only create 2 vertices
        G g({{10, 20}});
        REQUIRE(g.size() == 2);
        
        // Unlike vector-based graphs, no vertices 0-9 or 11-19 exist
    }

    SECTION("multiple sparse edges") {
        using G = mod_void_void_void;
        G g({{100, 200}, {300, 400}, {500, 600}});
        REQUIRE(g.size() == 6);
    }

    SECTION("reverse order vertex creation") {
        using G = mod_void_void_void;
        // Higher ID to lower ID
        G g({{100, 50}, {200, 25}});
        REQUIRE(g.size() == 4);
    }
}

//==================================================================================================
// 15. Vertex Accessor Methods Tests
//==================================================================================================

TEST_CASE("mod contains_vertex", "[dynamic_graph][mod][accessor][contains_vertex]") {
    SECTION("uint32_t vertex IDs") {
        using G = mod_void_void_void;
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
        using G = mod_str_void_void_void;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        
        REQUIRE(g.contains_vertex("alice"));
        REQUIRE(g.contains_vertex("bob"));
        REQUIRE(g.contains_vertex("charlie"));
        
        REQUIRE_FALSE(g.contains_vertex("david"));
        REQUIRE_FALSE(g.contains_vertex(""));
        REQUIRE_FALSE(g.contains_vertex("Alice")); // case sensitive
    }
    
    SECTION("empty graph") {
        using G = mod_void_void_void;
        G g;
        
        REQUIRE_FALSE(g.contains_vertex(0));
        REQUIRE_FALSE(g.contains_vertex(1));
    }
    
    SECTION("const graph") {
        using G = mod_void_void_void;
        const G g({{0, 1}, {2, 3}});
        
        REQUIRE(g.contains_vertex(0));
        REQUIRE(g.contains_vertex(1));
        REQUIRE_FALSE(g.contains_vertex(5));
    }
}

TEST_CASE("mod try_find_vertex", "[dynamic_graph][mod][accessor][try_find_vertex]") {
    SECTION("uint32_t vertex IDs - found") {
        using G = mod_void_void_void;
        G g({{0, 1}, {1, 2}, {5, 10}});
        
        auto it0 = g.try_find_vertex(0);
        REQUIRE(it0 != g.end());
        REQUIRE(it0->first == 0);  // map iterator gives key
        
        auto it5 = g.try_find_vertex(5);
        REQUIRE(it5 != g.end());
        REQUIRE(it5->first == 5);
        
        auto it10 = g.try_find_vertex(10);
        REQUIRE(it10 != g.end());
        REQUIRE(it10->first == 10);
    }
    
    SECTION("uint32_t vertex IDs - not found") {
        using G = mod_void_void_void;
        G g({{0, 1}, {5, 10}});
        
        auto it3 = g.try_find_vertex(3);
        REQUIRE(it3 == g.end());
        
        auto it100 = g.try_find_vertex(100);
        REQUIRE(it100 == g.end());
    }
    
    SECTION("string vertex IDs") {
        using G = mod_str_void_void_void;
        G g({{"alice", "bob"}, {"bob", "charlie"}});
        
        auto it_alice = g.try_find_vertex("alice");
        REQUIRE(it_alice != g.end());
        REQUIRE(it_alice->first == "alice");
        
        auto it_david = g.try_find_vertex("david");
        REQUIRE(it_david == g.end());
    }
    
    SECTION("does not modify container") {
        using G = mod_void_void_void;
        G g({{0, 1}});
        REQUIRE(g.size() == 2);
        
        // Looking for non-existent vertex should NOT add it
        auto it = g.try_find_vertex(999);
        REQUIRE(it == g.end());
        REQUIRE(g.size() == 2);  // Size unchanged - critical for safety
    }
    
    SECTION("const graph") {
        using G = mod_void_void_void;
        const G g({{0, 1}, {2, 3}});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        REQUIRE(it->first == 0);
        
        auto it_missing = g.try_find_vertex(99);
        REQUIRE(it_missing == g.end());
    }
}

TEST_CASE("mod vertex_at", "[dynamic_graph][mod][accessor][vertex_at]") {
    SECTION("uint32_t vertex IDs - found") {
        using G = mod_void_void_void;
        G g({{0, 1}, {1, 2}});
        
        // Should not throw
        REQUIRE_NOTHROW(g.vertex_at(0));
        REQUIRE_NOTHROW(g.vertex_at(1));
        REQUIRE_NOTHROW(g.vertex_at(2));
    }
    
    SECTION("uint32_t vertex IDs - throws on not found") {
        using G = mod_void_void_void;
        G g({{0, 1}});
        
        REQUIRE_THROWS_AS(g.vertex_at(5), std::out_of_range);
        REQUIRE_THROWS_AS(g.vertex_at(100), std::out_of_range);
    }
    
    SECTION("string vertex IDs") {
        using G = mod_str_void_void_void;
        G g({{"alice", "bob"}});
        
        REQUIRE_NOTHROW(g.vertex_at("alice"));
        REQUIRE_NOTHROW(g.vertex_at("bob"));
        REQUIRE_THROWS_AS(g.vertex_at("charlie"), std::out_of_range);
    }
    
    SECTION("modify vertex through vertex_at") {
        using G = mod_void_int_void;  // has vertex value
        G g({{0, 1}});
        
        g.vertex_at(0).value() = 42;
        g.vertex_at(1).value() = 100;
        
        REQUIRE(g.vertex_at(0).value() == 42);
        REQUIRE(g.vertex_at(1).value() == 100);
    }
    
    SECTION("const graph") {
        using G = mod_void_void_void;
        const G g({{0, 1}, {2, 3}});
        
        REQUIRE_NOTHROW(g.vertex_at(0));
        REQUIRE_THROWS_AS(g.vertex_at(99), std::out_of_range);
    }
    
    SECTION("does not modify container") {
        using G = mod_void_void_void;
        G g({{0, 1}});
        REQUIRE(g.size() == 2);
        
        // Accessing non-existent vertex throws but doesn't add
        REQUIRE_THROWS(g.vertex_at(999));
        REQUIRE(g.size() == 2);  // Size unchanged
    }
}

//==================================================================================================
// 16. load_vertices Tests
//==================================================================================================

TEST_CASE("mod load_vertices", "[dynamic_graph][mod][load_vertices]") {
    SECTION("uint32_t IDs - basic load") {
        using G = mod_void_int_void;  // vertex value = int
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}, {1, 2}});  // Create graph with edges
        REQUIRE(g.size() == 3);
        
        // Load vertex values
        std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.vertex_at(0).value() == 100);
        REQUIRE(g.vertex_at(1).value() == 200);
        REQUIRE(g.vertex_at(2).value() == 300);
    }
    
    SECTION("uint32_t IDs - load creates new vertices") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g;  // Empty graph
        REQUIRE(g.size() == 0);
        
        // Load should auto-insert vertices for associative containers
        std::vector<vertex_data> vv = {{10, 100}, {20, 200}, {30, 300}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.vertex_at(10).value() == 100);
        REQUIRE(g.vertex_at(20).value() == 200);
        REQUIRE(g.vertex_at(30).value() == 300);
    }
    
    SECTION("string IDs - basic load") {
        using G = mod_str_void_int_void;  // string keys, vertex value = int
        using vertex_data = copyable_vertex_t<std::string, int>;
        
        G g({{"alice", "bob"}});
        REQUIRE(g.size() == 2);
        
        std::vector<vertex_data> vv = {{"alice", 100}, {"bob", 200}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.vertex_at("alice").value() == 100);
        REQUIRE(g.vertex_at("bob").value() == 200);
    }
    
    SECTION("string IDs - load creates new vertices") {
        using G = mod_str_void_int_void;
        using vertex_data = copyable_vertex_t<std::string, int>;
        
        G g;  // Empty graph
        REQUIRE(g.size() == 0);
        
        std::vector<vertex_data> vv = {{"alice", 100}, {"bob", 200}, {"charlie", 300}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.vertex_at("alice").value() == 100);
        REQUIRE(g.vertex_at("bob").value() == 200);
        REQUIRE(g.vertex_at("charlie").value() == 300);
    }
    
    SECTION("overwrite existing vertex values") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}});
        
        // First load
        std::vector<vertex_data> vv1 = {{0, 100}, {1, 200}};
        g.load_vertices(vv1, std::identity{});
        REQUIRE(g.vertex_at(0).value() == 100);
        REQUIRE(g.vertex_at(1).value() == 200);
        
        // Second load overwrites
        std::vector<vertex_data> vv2 = {{0, 999}, {1, 888}};
        g.load_vertices(vv2, std::identity{});
        REQUIRE(g.vertex_at(0).value() == 999);
        REQUIRE(g.vertex_at(1).value() == 888);
    }
    
    SECTION("with projection function") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        struct Person {
            uint32_t id;
            std::string name;
            int age;
        };
        
        G g;
        std::vector<Person> people = {{1, "Alice", 30}, {2, "Bob", 25}, {3, "Charlie", 35}};
        
        g.load_vertices(people, [](const Person& p) -> vertex_data {
            return {p.id, p.age};
        });
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.vertex_at(1).value() == 30);
        REQUIRE(g.vertex_at(2).value() == 25);
        REQUIRE(g.vertex_at(3).value() == 35);
    }
}

//==================================================================================================
// 17. load_edges Tests (explicit calls, not via initializer_list)
//==================================================================================================

TEST_CASE("mod load_edges explicit", "[dynamic_graph][mod][load_edges]") {
    SECTION("uint32_t IDs - basic load") {
        using G = mod_int_void_void;  // edge value = int
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        G g;
        REQUIRE(g.size() == 0);
        
        std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 4);  // Vertices 0, 1, 2, 3 auto-created
    }
    
    SECTION("uint32_t IDs - sparse vertex creation") {
        using G = mod_void_void_void;
        using edge_data = copyable_edge_t<uint32_t, void>;
        
        G g;
        
        // Edges with sparse IDs
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
    
    SECTION("string IDs - basic load") {
        using G = mod_str_int_void_void;  // string keys, edge value = int
        using edge_data = copyable_edge_t<std::string, int>;
        
        G g;
        
        std::vector<edge_data> ee = {{"alice", "bob", 10}, {"bob", "charlie", 20}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 3);
        REQUIRE(g.contains_vertex("alice"));
        REQUIRE(g.contains_vertex("bob"));
        REQUIRE(g.contains_vertex("charlie"));
    }
    
    SECTION("append edges to existing graph") {
        using G = mod_int_void_void;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        G g({{0, 1, 10}});
        REQUIRE(g.size() == 2);
        
        // Add more edges
        std::vector<edge_data> ee = {{1, 2, 20}, {2, 3, 30}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 4);  // New vertices created
    }
    
    SECTION("with projection function") {
        using G = mod_int_void_void;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        struct Connection {
            uint32_t from;
            uint32_t to;
            std::string label;
            int weight;
        };
        
        G g;
        std::vector<Connection> connections = {
            {1, 2, "friend", 5},
            {2, 3, "colleague", 3},
            {3, 1, "family", 10}
        };
        
        g.load_edges(connections, [](const Connection& c) -> edge_data {
            return {c.from, c.to, c.weight};
        });
        
        REQUIRE(g.size() == 3);
    }
}

//==================================================================================================
// 18. Combined load_vertices and load_edges Tests
//==================================================================================================

TEST_CASE("mod load_vertices and load_edges combined", "[dynamic_graph][mod][load]") {
    SECTION("load edges then vertices") {
        using G = mod_int_int_void;  // edge value = int, vertex value = int
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        G g;
        
        // First load edges (creates vertices with default values)
        std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
        g.load_edges(ee, std::identity{});
        REQUIRE(g.size() == 3);
        
        // Then load vertex values
        std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.vertex_at(0).value() == 100);
        REQUIRE(g.vertex_at(1).value() == 200);
        REQUIRE(g.vertex_at(2).value() == 300);
    }
    
    SECTION("load vertices then edges") {
        using G = mod_int_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        G g;
        
        // First load vertices
        std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
        g.load_vertices(vv, std::identity{});
        REQUIRE(g.size() == 3);
        
        // Then load edges
        std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
        g.load_edges(ee, std::identity{});
        
        REQUIRE(g.size() == 3);  // No new vertices added
        REQUIRE(g.vertex_at(0).value() == 100);  // Values preserved
    }
    
    SECTION("string IDs - combined loading") {
        using G = mod_str_int_int_int;  // all values are int
        using vertex_data = copyable_vertex_t<std::string, int>;
        using edge_data = copyable_edge_t<std::string, int>;
        
        G g(42);  // graph value = 42
        
        std::vector<edge_data> ee = {{"alice", "bob", 5}, {"bob", "charlie", 3}};
        g.load_edges(ee, std::identity{});
        
        std::vector<vertex_data> vv = {{"alice", 30}, {"bob", 25}, {"charlie", 35}};
        g.load_vertices(vv, std::identity{});
        
        REQUIRE(g.graph_value() == 42);
        REQUIRE(g.size() == 3);
        REQUIRE(g.vertex_at("alice").value() == 30);
        REQUIRE(g.vertex_at("bob").value() == 25);
        REQUIRE(g.vertex_at("charlie").value() == 35);
    }
}

//==================================================================================================
// 19. Random Access Edge Iteration (std::deque specific)
//==================================================================================================

TEST_CASE("mod random access edge iteration", "[dynamic_graph][mod][edges][random_access]") {
    SECTION("edges preserve insertion order") {
        using G = mod_int_void_void;
        G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
        
        // Get vertex 0's edges
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        auto& v = it->second;
        auto edge_range = v.edges();
        
        // With std::deque, edges appear in order added
        std::vector<int> values;
        for (auto& e : edge_range) {
            values.push_back(e.value());
        }
        
        REQUIRE(values.size() == 3);
        // Order is 10, 20, 30 (first added appears first with std::deque)
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }
    
    SECTION("random access edge container allows indexed access") {
        using G = mod_int_void_void;
        G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        auto& v = it->second;
        auto& edge_range = v.edges();
        
        // std::deque allows random access via operator[]
        REQUIRE(edge_range[0].value() == 10);
        REQUIRE(edge_range[1].value() == 20);
        REQUIRE(edge_range[2].value() == 30);
        
        // Also allows .at() for bounds-checked access
        REQUIRE(edge_range.at(0).value() == 10);
        REQUIRE(edge_range.at(2).value() == 30);
    }
    
    SECTION("random access via iterator arithmetic") {
        using G = mod_int_void_void;
        G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
        
        auto it = g.try_find_vertex(0);
        REQUIRE(it != g.end());
        
        auto& v = it->second;
        auto& edge_range = v.edges();
        
        // Random access iterators support + and - operators
        auto last_it = edge_range.end();
        --last_it;
        REQUIRE(last_it->value() == 30);
        
        // Jump directly to middle element
        auto middle_it = edge_range.begin() + 1;
        REQUIRE(middle_it->value() == 20);
        
        // Calculate distance
        REQUIRE(edge_range.end() - edge_range.begin() == 3);
    }
    
    SECTION("edge count using random access size") {
        using G = mod_int_void_void;
        G g({
            {0, 1, 1}, {0, 2, 2}, {0, 3, 3},
            {1, 0, 4}, {1, 2, 5}
        });
        
        // Count edges from vertex 0 - deque has O(1) size()
        auto it0 = g.try_find_vertex(0);
        REQUIRE(it0 != g.end());
        size_t count0 = it0->second.edges().size();
        REQUIRE(count0 == 3);
        
        // Count edges from vertex 1
        auto it1 = g.try_find_vertex(1);
        REQUIRE(it1 != g.end());
        size_t count1 = it1->second.edges().size();
        REQUIRE(count1 == 2);
    }
}

//==================================================================================================
// 20. std::ranges Integration
//==================================================================================================

TEST_CASE("mod std::ranges integration", "[dynamic_graph][mod][ranges]") {
    SECTION("ranges::count_if on vertices") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}});
        std::vector<vertex_data> vv = {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
        g.load_vertices(vv, std::identity{});
        
        // Count vertices with even values
        auto count = std::ranges::count_if(g, [](auto& pair) {
            return pair.second.value() % 2 == 0;
        });
        
        REQUIRE(count == 3); // 0, 2, 4
    }
    
    SECTION("ranges::find_if on vertices") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}, {1, 2}, {2, 3}});
        std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};
        g.load_vertices(vv, std::identity{});
        
        auto it = std::ranges::find_if(g, [](auto& pair) {
            return pair.second.value() == 30;
        });
        
        REQUIRE(it != g.end());
        REQUIRE(it->second.value() == 30);
    }
}

//==================================================================================================
// 21. Algorithm Compatibility
//==================================================================================================

TEST_CASE("mod algorithm compatibility", "[dynamic_graph][mod][algorithms]") {
    SECTION("std::accumulate on vertex values") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
        std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
        g.load_vertices(vv, std::identity{});
        
        auto sum = std::accumulate(g.begin(), g.end(), 0, [](int acc, auto& pair) {
            return acc + pair.second.value();
        });
        
        REQUIRE(sum == 15); // 1+2+3+4+5
    }
    
    SECTION("std::all_of on vertices") {
        using G = mod_void_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        
        G g({{0, 1}, {1, 2}});
        std::vector<vertex_data> vv = {{0, 2}, {1, 4}, {2, 6}};
        g.load_vertices(vv, std::identity{});
        
        bool all_even = std::all_of(g.begin(), g.end(), [](auto& pair) {
            return pair.second.value() % 2 == 0;
        });
        
        REQUIRE(all_even);
    }
}

//==================================================================================================
// 22. Workflow Scenarios
//==================================================================================================

TEST_CASE("mod complete workflow scenarios", "[dynamic_graph][mod][workflow]") {
    SECTION("social network simulation with string IDs") {
        using G = mod_str_int_void_void;
        
        // Build social network with friendship strengths
        G g({
            {"alice", "bob", 5},
            {"alice", "charlie", 3},
            {"bob", "charlie", 4},
            {"bob", "dave", 2},
            {"charlie", "eve", 5}
        });
        
        REQUIRE(g.size() == 5);
        
        // Find person with most friends (highest out-degree)
        std::string most_social;
        size_t max_friends = 0;
        
        for (auto& [key, vertex] : g) {
            size_t friend_count = vertex.edges().size();  // O(1) with deque
            if (friend_count > max_friends) {
                max_friends = friend_count;
                most_social = key;
            }
        }
        
        // alice and bob both have 2 friends, alice found first due to map ordering
        REQUIRE(most_social == "alice");
        REQUIRE(max_friends == 2);
    }
    
    SECTION("build graph, query, modify workflow") {
        using G = mod_int_int_void;
        using vertex_data = copyable_vertex_t<uint32_t, int>;
        using edge_data = copyable_edge_t<uint32_t, int>;
        
        // Step 1: Build initial graph
        G g;
        std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
        g.load_vertices(vv, std::identity{});
        
        std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
        g.load_edges(ee, std::identity{});
        
        // Step 2: Query graph properties
        REQUIRE(g.size() == 3);
        
        size_t total_edges = 0;
        for (auto& [key, vertex] : g) {
            total_edges += vertex.edges().size();  // O(1) with deque
        }
        REQUIRE(total_edges == 2);
        
        // Step 3: Modify vertex values
        g.vertex_at(0).value() = 999;
        g.vertex_at(1).value() = 888;
        g.vertex_at(2).value() = 777;
        
        // Step 4: Add more edges
        std::vector<edge_data> more_edges = {{2, 0, 30}};
        g.load_edges(more_edges, std::identity{});
        
        // Step 5: Verify final state
        REQUIRE(g.vertex_at(0).value() == 999);
        REQUIRE(g.vertex_at(1).value() == 888);
        REQUIRE(g.vertex_at(2).value() == 777);
        
        total_edges = 0;
        for (auto& [key, vertex] : g) {
            total_edges += vertex.edges().size();
        }
        REQUIRE(total_edges == 3);
    }
}

//==================================================================================================
// Summary: Phase 3.1 mod_graph_traits Tests
//
// This file tests mod_graph_traits (map vertices + deque edges).
// 
// Key behaviors being tested:
// - Map provides key-based storage (sparse vertex IDs)
// - Ordered iteration - vertices iterate in key order  
// - String vertex IDs supported
// - Forward vertex iteration (descriptor iterators, despite map's bidirectional iterators)
// - Forward edge iteration (descriptor iterators, despite deque's random access iterators)
// - Proper copy/move semantics
// - Graph value storage
// - load_vertices() with associative containers
// - load_edges() with associative containers
// - Edge order preservation (first added appears first)
// - O(1) edge count via deque::size()
//
// Key differences from mov (map + vector):
// - std::deque provides efficient front and back insertion
// - std::deque may have different memory layout (chunked)
// - Both have random access edge iteration
// - Edge order is preserved: first added appears first (same as vector)
//
// Key differences from mol (map + list):
// - std::deque allows random access edge iteration (vs list's bidirectional)
// - Contiguous-ish memory for better cache locality
//
// Key differences from vov (vector + vector):
// - Sparse vertex IDs - only referenced vertices are created
// - String vertex IDs natively supported
// - Vertices iterate in sorted key order (map property)
//
// Note: The vertices(g) CPO is tested separately.
//==================================================================================================
