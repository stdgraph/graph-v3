/**
 * @file test_dynamic_graph_mov.cpp
 * @brief Tests for dynamic_graph with map vertices + vector edges
 * 
 * Phase 3.1c: Map Vertex Containers
 * Tests mov_graph_traits (map vertices + vector edges)
 * This is the third associative container combination for dynamic graphs.
 * 
 * Key differences from sequential containers (vector/deque):
 * 1. Key-based vertex identification - not index-based
 * 2. Descriptor iterators are forward-only (despite underlying container capabilities)
 * 3. Sparse vertex IDs by design - only referenced vertices are created
 * 4. Ordered iteration - vertices iterate in key order
 * 
 * Key differences from mol (map + list):
 * 1. std::vector provides random access edge iteration (vs list's bidirectional)
 * 2. Both preserve edge order: first added appears first
 * 
 * Key differences from vov (vector + vector):
 * 1. Sparse vertex IDs - only referenced vertices are created
 * 2. String vertex IDs supported natively
 * 
 * Note: The vertices(g) CPO is tested separately in graph container interface tests.
 * This file focuses on mov_graph_traits specific functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/mov_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <set>

using namespace graph::container;

// Type aliases for common test configurations with uint32_t vertex IDs
using mov_void_void_void =
      dynamic_graph<void, void, void, uint32_t, false, mov_graph_traits<void, void, void, uint32_t, false>>;
using mov_int_void_void =
      dynamic_graph<int, void, void, uint32_t, false, mov_graph_traits<int, void, void, uint32_t, false>>;
using mov_void_int_void =
      dynamic_graph<void, int, void, uint32_t, false, mov_graph_traits<void, int, void, uint32_t, false>>;
using mov_int_int_void =
      dynamic_graph<int, int, void, uint32_t, false, mov_graph_traits<int, int, void, uint32_t, false>>;
using mov_void_void_int =
      dynamic_graph<void, void, int, uint32_t, false, mov_graph_traits<void, void, int, uint32_t, false>>;
using mov_int_int_int = dynamic_graph<int, int, int, uint32_t, false, mov_graph_traits<int, int, int, uint32_t, false>>;

// Type aliases with string vertex IDs (the primary use case for map containers)
using mov_str_void_void_void =
      dynamic_graph<void, void, void, std::string, false, mov_graph_traits<void, void, void, std::string, false>>;
using mov_str_int_void_void =
      dynamic_graph<int, void, void, std::string, false, mov_graph_traits<int, void, void, std::string, false>>;
using mov_str_void_int_void =
      dynamic_graph<void, int, void, std::string, false, mov_graph_traits<void, int, void, std::string, false>>;
using mov_str_int_int_int =
      dynamic_graph<int, int, int, std::string, false, mov_graph_traits<int, int, int, std::string, false>>;



//==================================================================================================
// 1. Traits Verification Tests
//==================================================================================================

TEST_CASE("mov traits verification", "[dynamic_graph][mov][traits]") {
  SECTION("vertices_type is std::map") {
    using traits     = mov_graph_traits<void, void, void, uint32_t, false>;
    using vertices_t = typename traits::vertices_type;
    // Verify it's a map by checking for map-specific members
    static_assert(requires { typename vertices_t::key_type; });
    static_assert(requires { typename vertices_t::mapped_type; });
    REQUIRE(true); // Compilation success means the test passes
  }

  SECTION("edges_type is std::vector") {
    using traits  = mov_graph_traits<void, void, void, uint32_t, false>;
    using edges_t = typename traits::edges_type;
    // std::vector has push_back and supports random access
    static_assert(requires(edges_t e, typename edges_t::value_type v) { e.push_back(v); });
    static_assert(requires(edges_t e, size_t i) { e[i]; }); // Random access
    // Verify it's vector by checking for vector-specific characteristics
    static_assert(std::contiguous_iterator<typename edges_t::iterator>);
    REQUIRE(true);
  }

  SECTION("vertex_id_type can be string") {
    using traits = mov_graph_traits<void, void, void, std::string, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
    REQUIRE(true);
  }


  SECTION("vertex_id_type for uint32_t") {
    using traits = mov_graph_traits<void, void, void, uint32_t, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, uint32_t>);
    REQUIRE(true);
  }

  SECTION("custom vertex_id_type") {
    using traits = mov_graph_traits<void, void, void, int64_t, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, int64_t>);
    REQUIRE(true);
  }
}

//==================================================================================================
// 2. Iterator Category Tests
//==================================================================================================

TEST_CASE("mov iterator categories", "[dynamic_graph][mov][iterators]") {
  SECTION("underlying map iterators are bidirectional") {
    // Note: The raw map iterator is bidirectional, but the graph library
    // wraps vertices with descriptor iterators which are forward-only.
    using G      = mov_void_void_void;
    using iter_t = typename G::vertices_type::iterator;
    static_assert(std::bidirectional_iterator<iter_t>);
    static_assert(!std::random_access_iterator<iter_t>);
    REQUIRE(true);
  }

  SECTION("underlying vector iterators are random access") {
    // Note: The raw vector iterator is random access, but the graph library
    // wraps edges with descriptor iterators which are forward-only.
    using traits      = mov_graph_traits<void, void, void, uint32_t, false>;
    using edges_t     = typename traits::edges_type;
    using edge_iter_t = typename edges_t::iterator;
    static_assert(std::random_access_iterator<edge_iter_t>);
    REQUIRE(true);
  }

  SECTION("graph is a range") {
    static_assert(std::ranges::range<mov_void_void_void>);
    static_assert(std::ranges::range<mov_int_int_int>);
    static_assert(std::ranges::range<mov_str_void_void_void>);
    REQUIRE(true);
  }

  SECTION("underlying edges container is random access range") {
    // Note: The raw edges container is a random access range, but the graph
    // library uses edge descriptor iterators which are forward-only.
    static_assert(std::ranges::random_access_range<mov_void_void_void::vertex_type::edges_type>);
    static_assert(std::ranges::random_access_range<mov_int_int_int::vertex_type::edges_type>);
    REQUIRE(true);
  }
}

//==================================================================================================
// 3. Construction Tests
//==================================================================================================

TEST_CASE("mov construction", "[dynamic_graph][mov][construction]") {
  SECTION("default constructor creates empty graph") {
    mov_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with void types") {
    mov_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int edge values") {
    mov_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int vertex values") {
    mov_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int graph value") {
    mov_void_void_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with all int values") {
    mov_int_int_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("constructor with graph value - int GV") {
    mov_void_void_int g(42);
    REQUIRE(g.size() == 0);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("copy constructor") {
    mov_int_int_int g1;
    mov_int_int_int g2(g1);
    REQUIRE(g2.size() == g1.size());
  }

  SECTION("move constructor") {
    mov_int_int_int g1;
    mov_int_int_int g2(std::move(g1));
    REQUIRE(g2.size() == 0);
  }

  SECTION("copy assignment") {
    mov_int_int_int g1, g2;
    g2 = g1;
    REQUIRE(g2.size() == g1.size());
  }

  SECTION("move assignment") {
    mov_int_int_int g1, g2;
    g2 = std::move(g1);
    REQUIRE(g2.size() == 0);
  }
}

TEST_CASE("mov construction with string vertex IDs", "[dynamic_graph][mov][construction][string]") {
  SECTION("default constructor creates empty graph") {
    mov_str_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int edge values") {
    mov_str_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int vertex values") {
    mov_str_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with all int values") {
    mov_str_int_int_int g;
    REQUIRE(g.size() == 0);
  }
}


//==================================================================================================
// 4. Basic Properties Tests
//==================================================================================================

TEST_CASE("mov properties", "[dynamic_graph][mov][properties]") {
  SECTION("size() on empty graph") {
    mov_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("const graph methods") {
    const mov_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("begin() == end() for empty graph") {
    mov_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("const begin() == const end() for empty graph") {
    const mov_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("cbegin() == cend() for empty graph") {
    mov_void_void_void g;
    REQUIRE(g.cbegin() == g.cend());
  }
}

TEST_CASE("mov properties with string IDs", "[dynamic_graph][mov][properties][string]") {
  SECTION("size() on empty graph") {
    mov_str_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("begin() == end() for empty graph") {
    mov_str_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }
}

//==================================================================================================
// 5. Type Alias Tests
//==================================================================================================

TEST_CASE("mov type aliases", "[dynamic_graph][mov][types]") {
  SECTION("graph type aliases are correct") {
    using G = mov_int_int_int;
    static_assert(std::same_as<typename G::value_type, int>); // GV
    REQUIRE(true);
  }


  SECTION("string key graph type aliases are correct") {
    using G      = mov_str_int_int_int;
    using traits = typename G::graph_traits;
    static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
    REQUIRE(true);
  }
}

//==================================================================================================
// 6. Initializer List Construction Tests (uint32_t vertex IDs)
//==================================================================================================

TEST_CASE("mov initializer_list construction", "[dynamic_graph][mov][initializer_list]") {
  SECTION("empty initializer list") {
    using G = mov_void_void_void;
    G g({});
    REQUIRE(g.size() == 0);
  }

  SECTION("single edge without value") {
    using G = mov_void_void_void;
    G g({{0, 1}});
    REQUIRE(g.size() == 2);
  }

  SECTION("single edge with value") {
    using G = mov_int_void_void;
    G g({{0, 1, 42}});
    REQUIRE(g.size() == 2);
  }

  SECTION("multiple edges from same source") {
    using G = mov_int_void_void;
    G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
    REQUIRE(g.size() == 4);
  }

  SECTION("triangle graph") {
    using G = mov_void_void_void;
    G g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(g.size() == 3);
  }

  SECTION("self-loop") {
    using G = mov_void_void_void;
    G g({{0, 0}});
    REQUIRE(g.size() == 1);
  }

  SECTION("parallel edges") {
    using G = mov_int_void_void;
    G g({{0, 1, 1}, {0, 1, 2}, {0, 1, 3}});
    REQUIRE(g.size() == 2);
  }

  SECTION("sparse vertex IDs - only referenced vertices created") {
    using G = mov_void_void_void;
    G g({{100, 200}});
    REQUIRE(g.size() == 2);
    // With mov, only vertices 100 and 200 are created
    // (unlike vector-based which would create 0..200)
  }

  SECTION("star graph (center with many spokes)") {
    using G = mov_int_void_void;
    G g({{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4}, {0, 5, 5}});
    REQUIRE(g.size() == 6);
  }

  SECTION("complete graph K4") {
    using G = mov_int_void_void;
    G g({{0, 1, 1},
         {0, 2, 2},
         {0, 3, 3},
         {1, 0, 4},
         {1, 2, 5},
         {1, 3, 6},
         {2, 0, 7},
         {2, 1, 8},
         {2, 3, 9},
         {3, 0, 10},
         {3, 1, 11},
         {3, 2, 12}});
    REQUIRE(g.size() == 4);
  }

  SECTION("construction with graph value") {
    using G = mov_void_void_int;
    G g(42, {{0, 1}, {1, 2}});
    REQUIRE(g.graph_value() == 42);
    REQUIRE(g.size() == 3);
  }
}

//==================================================================================================
// 7. Initializer List Construction Tests (string vertex IDs)
//==================================================================================================

TEST_CASE("mov initializer_list construction string IDs", "[dynamic_graph][mov][initializer_list][string]") {
  SECTION("single edge with string IDs") {
    using G = mov_str_void_void_void;
    G g({{"alice", "bob"}});
    REQUIRE(g.size() == 2);
  }

  SECTION("string IDs with edge values") {
    using G = mov_str_int_void_void;
    G g({{"alice", "bob", 10}, {"bob", "charlie", 20}});
    REQUIRE(g.size() == 3);
  }

  SECTION("social network graph") {
    using G = mov_str_int_void_void;
    G g({{"alice", "bob", 5},
         {"alice", "charlie", 3},
         {"bob", "charlie", 4},
         {"bob", "dave", 2},
         {"charlie", "eve", 5}});
    REQUIRE(g.size() == 5);
  }

}

//==================================================================================================
// 8. Graph Value Tests
//==================================================================================================

TEST_CASE("mov graph value access", "[dynamic_graph][mov][graph_value]") {
  SECTION("graph_value() returns reference") {
    mov_void_void_int g(100);
    REQUIRE(g.graph_value() == 100);
    g.graph_value() = 200;
    REQUIRE(g.graph_value() == 200);
  }

  SECTION("const graph_value()") {
    const mov_void_void_int g(42);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("graph value preserved through copy") {
    mov_void_void_int g1(100);
    mov_void_void_int g2 = g1;
    REQUIRE(g2.graph_value() == 100);
    g2.graph_value() = 200;
    REQUIRE(g1.graph_value() == 100); // g1 unchanged
  }

  SECTION("graph value preserved through move") {
    mov_void_void_int g1(100);
    mov_void_void_int g2 = std::move(g1);
    REQUIRE(g2.graph_value() == 100);
  }
}

//==================================================================================================
// 9. Graph Iteration Tests (direct iteration, not via vertices() CPO)
//==================================================================================================

TEST_CASE("mov graph iteration", "[dynamic_graph][mov][iteration]") {
  SECTION("iterate over empty graph") {
    mov_void_void_void g;
    size_t             count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("iterate over graph with vertices") {
    using G = mov_void_void_void;
    G g({{0, 1}, {1, 2}});

    size_t count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("const iteration") {
    using G = mov_void_void_void;
    const G g({{0, 1}, {1, 2}});

    size_t count = 0;
    for ([[maybe_unused]] const auto& v : g) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("iterate string key graph") {
    using G = mov_str_void_void_void;
    G g({{"alice", "bob"}, {"bob", "charlie"}});

    size_t count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("vertices iterate in key order (map property)") {
    using G = mov_void_void_void;
    G g({{5, 10}, {1, 2}, {3, 4}});

    std::vector<uint32_t> keys;
    for (auto& v : g) {
      keys.push_back(v.first); // Map iterator gives pair
    }

    // Should be sorted: 1, 2, 3, 4, 5, 10
    REQUIRE(std::is_sorted(keys.begin(), keys.end()));
  }
}

//==================================================================================================
// 10. Edge Cases and Error Handling
//==================================================================================================

TEST_CASE("mov edge cases", "[dynamic_graph][mov][edge_cases]") {
  SECTION("graph with single vertex (self-loop)") {
    using G = mov_void_void_void;
    G g({{0, 0}});
    REQUIRE(g.size() == 1);
  }

  SECTION("clear() empties the graph") {
    using G = mov_int_void_void;
    G g({{0, 1, 10}, {1, 2, 20}});
    REQUIRE(g.size() == 3);
    g.clear();
    REQUIRE(g.size() == 0);
  }

  SECTION("multiple clears are safe") {
    using G = mov_void_void_void;
    G g({{0, 1}});
    g.clear();
    g.clear();
    g.clear();
    REQUIRE(g.size() == 0);
  }

  SECTION("swap two graphs") {
    using G = mov_void_void_int;
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
    using G = mov_void_void_void;
    G g({{1000000, 2000000}});
    REQUIRE(g.size() == 2); // Only 2 vertices, not 2000001
  }
}

//==================================================================================================
// 11. Const Correctness Tests
//==================================================================================================

TEST_CASE("mov const correctness", "[dynamic_graph][mov][const]") {
  SECTION("const graph properties") {
    using G = mov_int_void_void;
    const G g({{0, 1, 10}, {1, 2, 20}});

    REQUIRE(g.size() == 3);
    REQUIRE(g.begin() != g.end());
  }

  SECTION("const graph iteration") {
    using G = mov_int_void_void;
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

TEST_CASE("mov memory management", "[dynamic_graph][mov][memory]") {
  SECTION("multiple independent graphs") {
    using G = mov_void_void_int;
    G g1(100, {{0, 1}});
    G g2(200, {{1, 2}});
    G g3(300, {{2, 3}});

    REQUIRE(g1.graph_value() == 100);
    REQUIRE(g2.graph_value() == 200);
    REQUIRE(g3.graph_value() == 300);
  }

  SECTION("copy does not alias") {
    using G = mov_void_void_int;
    G g1(100, {{0, 1}});
    G g2 = g1;

    g2.graph_value() = 200;
    REQUIRE(g1.graph_value() == 100);
    REQUIRE(g2.graph_value() == 200);
  }

  SECTION("clear allows reuse") {
    using G = mov_int_void_void;
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

TEST_CASE("mov template instantiation", "[dynamic_graph][mov][compilation]") {
  [[maybe_unused]] mov_void_void_void     g1;
  [[maybe_unused]] mov_int_void_void      g2;
  [[maybe_unused]] mov_void_int_void      g3;
  [[maybe_unused]] mov_int_int_void       g4;
  [[maybe_unused]] mov_void_void_int      g5;
  [[maybe_unused]] mov_int_int_int        g6;
  [[maybe_unused]] mov_str_void_void_void g9;
  [[maybe_unused]] mov_str_int_void_void  g10;
  [[maybe_unused]] mov_str_int_int_int    g11;

  REQUIRE(true);
}

//==================================================================================================
// 14. Sparse Vertex Behavior (associative container specific)
//==================================================================================================

TEST_CASE("mov sparse vertex behavior", "[dynamic_graph][mov][sparse]") {
  SECTION("only referenced vertices are created") {
    using G = mov_void_void_void;

    // Edge from 10 to 20 - should only create 2 vertices
    G g({{10, 20}});
    REQUIRE(g.size() == 2);

    // Unlike vector-based graphs, no vertices 0-9 or 11-19 exist
  }

  SECTION("multiple sparse edges") {
    using G = mov_void_void_void;
    G g({{100, 200}, {300, 400}, {500, 600}});
    REQUIRE(g.size() == 6);
  }

  SECTION("reverse order vertex creation") {
    using G = mov_void_void_void;
    // Higher ID to lower ID
    G g({{100, 50}, {200, 25}});
    REQUIRE(g.size() == 4);
  }
}

//==================================================================================================
// 15. Vertex Accessor Methods Tests
//==================================================================================================

TEST_CASE("mov contains_vertex", "[dynamic_graph][mov][accessor][contains_vertex]") {
  SECTION("uint32_t vertex IDs") {
    using G = mov_void_void_void;
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
    using G = mov_str_void_void_void;
    G g({{"alice", "bob"}, {"bob", "charlie"}});

    REQUIRE(g.contains_vertex("alice"));
    REQUIRE(g.contains_vertex("bob"));
    REQUIRE(g.contains_vertex("charlie"));

    REQUIRE_FALSE(g.contains_vertex("david"));
    REQUIRE_FALSE(g.contains_vertex(""));
    REQUIRE_FALSE(g.contains_vertex("Alice")); // case sensitive
  }

  SECTION("empty graph") {
    using G = mov_void_void_void;
    G g;

    REQUIRE_FALSE(g.contains_vertex(0));
    REQUIRE_FALSE(g.contains_vertex(1));
  }

  SECTION("const graph") {
    using G = mov_void_void_void;
    const G g({{0, 1}, {2, 3}});

    REQUIRE(g.contains_vertex(0));
    REQUIRE(g.contains_vertex(1));
    REQUIRE_FALSE(g.contains_vertex(5));
  }
}

TEST_CASE("mov try_find_vertex", "[dynamic_graph][mov][accessor][try_find_vertex]") {
  SECTION("uint32_t vertex IDs - found") {
    using G = mov_void_void_void;
    G g({{0, 1}, {1, 2}, {5, 10}});

    auto it0 = g.try_find_vertex(0);
    REQUIRE(it0 != g.end());
    REQUIRE(it0->first == 0); // map iterator gives key

    auto it5 = g.try_find_vertex(5);
    REQUIRE(it5 != g.end());
    REQUIRE(it5->first == 5);

    auto it10 = g.try_find_vertex(10);
    REQUIRE(it10 != g.end());
    REQUIRE(it10->first == 10);
  }

  SECTION("uint32_t vertex IDs - not found") {
    using G = mov_void_void_void;
    G g({{0, 1}, {5, 10}});

    auto it3 = g.try_find_vertex(3);
    REQUIRE(it3 == g.end());

    auto it100 = g.try_find_vertex(100);
    REQUIRE(it100 == g.end());
  }

  SECTION("string vertex IDs") {
    using G = mov_str_void_void_void;
    G g({{"alice", "bob"}, {"bob", "charlie"}});

    auto it_alice = g.try_find_vertex("alice");
    REQUIRE(it_alice != g.end());
    REQUIRE(it_alice->first == "alice");

    auto it_david = g.try_find_vertex("david");
    REQUIRE(it_david == g.end());
  }

  SECTION("does not modify container") {
    using G = mov_void_void_void;
    G g({{0, 1}});
    REQUIRE(g.size() == 2);

    // Looking for non-existent vertex should NOT add it
    auto it = g.try_find_vertex(999);
    REQUIRE(it == g.end());
    REQUIRE(g.size() == 2); // Size unchanged - critical for safety
  }

  SECTION("const graph") {
    using G = mov_void_void_void;
    const G g({{0, 1}, {2, 3}});

    auto it = g.try_find_vertex(0);
    REQUIRE(it != g.end());
    REQUIRE(it->first == 0);

    auto it_missing = g.try_find_vertex(99);
    REQUIRE(it_missing == g.end());
  }
}

TEST_CASE("mov vertex_at", "[dynamic_graph][mov][accessor][vertex_at]") {
  SECTION("uint32_t vertex IDs - found") {
    using G = mov_void_void_void;
    G g({{0, 1}, {1, 2}});

    // Should not throw
    REQUIRE_NOTHROW(g.vertex_at(0));
    REQUIRE_NOTHROW(g.vertex_at(1));
    REQUIRE_NOTHROW(g.vertex_at(2));
  }

  SECTION("uint32_t vertex IDs - throws on not found") {
    using G = mov_void_void_void;
    G g({{0, 1}});

    REQUIRE_THROWS_AS(g.vertex_at(5), std::out_of_range);
    REQUIRE_THROWS_AS(g.vertex_at(100), std::out_of_range);
  }

  SECTION("string vertex IDs") {
    using G = mov_str_void_void_void;
    G g({{"alice", "bob"}});

    REQUIRE_NOTHROW(g.vertex_at("alice"));
    REQUIRE_NOTHROW(g.vertex_at("bob"));
    REQUIRE_THROWS_AS(g.vertex_at("charlie"), std::out_of_range);
  }

  SECTION("modify vertex through vertex_at") {
    using G = mov_void_int_void; // has vertex value
    G g({{0, 1}});

    g.vertex_at(0).value() = 42;
    g.vertex_at(1).value() = 100;

    REQUIRE(g.vertex_at(0).value() == 42);
    REQUIRE(g.vertex_at(1).value() == 100);
  }

  SECTION("const graph") {
    using G = mov_void_void_void;
    const G g({{0, 1}, {2, 3}});

    REQUIRE_NOTHROW(g.vertex_at(0));
    REQUIRE_THROWS_AS(g.vertex_at(99), std::out_of_range);
  }

  SECTION("does not modify container") {
    using G = mov_void_void_void;
    G g({{0, 1}});
    REQUIRE(g.size() == 2);

    // Accessing non-existent vertex throws but doesn't add
    REQUIRE_THROWS(g.vertex_at(999));
    REQUIRE(g.size() == 2); // Size unchanged
  }
}

//==================================================================================================
// 16. load_vertices Tests
//==================================================================================================

TEST_CASE("mov load_vertices", "[dynamic_graph][mov][load_vertices]") {
  SECTION("uint32_t IDs - basic load") {
    using G           = mov_void_int_void; // vertex value = int
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    G g({{0, 1}, {1, 2}}); // Create graph with edges
    REQUIRE(g.size() == 3);

    // Load vertex values
    std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.vertex_at(0).value() == 100);
    REQUIRE(g.vertex_at(1).value() == 200);
    REQUIRE(g.vertex_at(2).value() == 300);
  }

  SECTION("uint32_t IDs - load creates new vertices") {
    using G           = mov_void_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    G g; // Empty graph
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
    using G           = mov_str_void_int_void; // string keys, vertex value = int
    using vertex_data = copyable_vertex_t<std::string, int>;

    G g({{"alice", "bob"}});
    REQUIRE(g.size() == 2);

    std::vector<vertex_data> vv = {{"alice", 100}, {"bob", 200}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.vertex_at("alice").value() == 100);
    REQUIRE(g.vertex_at("bob").value() == 200);
  }

  SECTION("string IDs - load creates new vertices") {
    using G           = mov_str_void_int_void;
    using vertex_data = copyable_vertex_t<std::string, int>;

    G g; // Empty graph
    REQUIRE(g.size() == 0);

    std::vector<vertex_data> vv = {{"alice", 100}, {"bob", 200}, {"charlie", 300}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == 3);
    REQUIRE(g.vertex_at("alice").value() == 100);
    REQUIRE(g.vertex_at("bob").value() == 200);
    REQUIRE(g.vertex_at("charlie").value() == 300);
  }

  SECTION("overwrite existing vertex values") {
    using G           = mov_void_int_void;
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
    using G           = mov_void_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    struct Person {
      uint32_t    id;
      std::string name;
      int         age;
    };

    G                   g;
    std::vector<Person> people = {{1, "Alice", 30}, {2, "Bob", 25}, {3, "Charlie", 35}};

    g.load_vertices(people, [](const Person& p) -> vertex_data { return {p.id, p.age}; });

    REQUIRE(g.size() == 3);
    REQUIRE(g.vertex_at(1).value() == 30);
    REQUIRE(g.vertex_at(2).value() == 25);
    REQUIRE(g.vertex_at(3).value() == 35);
  }
}

//==================================================================================================
// 17. load_edges Tests (explicit calls, not via initializer_list)
//==================================================================================================

TEST_CASE("mov load_edges explicit", "[dynamic_graph][mov][load_edges]") {
  SECTION("uint32_t IDs - basic load") {
    using G         = mov_int_void_void; // edge value = int
    using edge_data = copyable_edge_t<uint32_t, int>;

    G g;
    REQUIRE(g.size() == 0);

    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4); // Vertices 0, 1, 2, 3 auto-created
  }

  SECTION("uint32_t IDs - sparse vertex creation") {
    using G         = mov_void_void_void;
    using edge_data = copyable_edge_t<uint32_t, void>;

    G g;

    // Edges with sparse IDs
    std::vector<edge_data> ee = {{100, 200}, {300, 400}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4); // Only 4 vertices, not 401
    REQUIRE(g.contains_vertex(100));
    REQUIRE(g.contains_vertex(200));
    REQUIRE(g.contains_vertex(300));
    REQUIRE(g.contains_vertex(400));
    REQUIRE_FALSE(g.contains_vertex(0));
    REQUIRE_FALSE(g.contains_vertex(150));
  }

  SECTION("string IDs - basic load") {
    using G         = mov_str_int_void_void; // string keys, edge value = int
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
    using G         = mov_int_void_void;
    using edge_data = copyable_edge_t<uint32_t, int>;

    G g({{0, 1, 10}});
    REQUIRE(g.size() == 2);

    // Add more edges
    std::vector<edge_data> ee = {{1, 2, 20}, {2, 3, 30}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4); // New vertices created
  }

  SECTION("with projection function") {
    using G         = mov_int_void_void;
    using edge_data = copyable_edge_t<uint32_t, int>;

    struct Connection {
      uint32_t    from;
      uint32_t    to;
      std::string label;
      int         weight;
    };

    G                       g;
    std::vector<Connection> connections = {{1, 2, "friend", 5}, {2, 3, "colleague", 3}, {3, 1, "family", 10}};

    g.load_edges(connections, [](const Connection& c) -> edge_data { return {c.from, c.to, c.weight}; });

    REQUIRE(g.size() == 3);
  }
}

//==================================================================================================
// 18. Combined load_vertices and load_edges Tests
//==================================================================================================

TEST_CASE("mov load_vertices and load_edges combined", "[dynamic_graph][mov][load]") {
  SECTION("load edges then vertices") {
    using G           = mov_int_int_void; // edge value = int, vertex value = int
    using vertex_data = copyable_vertex_t<uint32_t, int>;
    using edge_data   = copyable_edge_t<uint32_t, int>;

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
    using G           = mov_int_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;
    using edge_data   = copyable_edge_t<uint32_t, int>;

    G g;

    // First load vertices
    std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 3);

    // Then load edges
    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);                 // No new vertices added
    REQUIRE(g.vertex_at(0).value() == 100); // Values preserved
  }

  SECTION("string IDs - combined loading") {
    using G           = mov_str_int_int_int; // all values are int
    using vertex_data = copyable_vertex_t<std::string, int>;
    using edge_data   = copyable_edge_t<std::string, int>;

    G g(42); // graph value = 42

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
// 19. Random Access Edge Iteration (std::vector specific)
//==================================================================================================

TEST_CASE("mov random access edge iteration", "[dynamic_graph][mov][edges][random_access]") {
  SECTION("edges preserve insertion order") {
    using G = mov_int_void_void;
    G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});

    // Get vertex 0's edges
    auto it = g.try_find_vertex(0);
    REQUIRE(it != g.end());

    auto& v          = it->second;
    auto  edge_range = v.edges();

    // With std::vector, edges appear in order added
    std::vector<int> values;
    for (auto& e : edge_range) {
      values.push_back(e.value());
    }

    REQUIRE(values.size() == 3);
    // Order is 10, 20, 30 (first added appears first with std::vector)
    REQUIRE(values[0] == 10);
    REQUIRE(values[1] == 20);
    REQUIRE(values[2] == 30);
  }

  SECTION("random access edge container allows indexed access") {
    using G = mov_int_void_void;
    G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});

    auto it = g.try_find_vertex(0);
    REQUIRE(it != g.end());

    auto& v          = it->second;
    auto& edge_range = v.edges();

    // std::vector allows random access via operator[]
    REQUIRE(edge_range[0].value() == 10);
    REQUIRE(edge_range[1].value() == 20);
    REQUIRE(edge_range[2].value() == 30);

    // Also allows .at() for bounds-checked access
    REQUIRE(edge_range.at(0).value() == 10);
    REQUIRE(edge_range.at(2).value() == 30);
  }

  SECTION("random access via iterator arithmetic") {
    using G = mov_int_void_void;
    G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});

    auto it = g.try_find_vertex(0);
    REQUIRE(it != g.end());

    auto& v          = it->second;
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
    using G = mov_int_void_void;
    G g({{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {1, 0, 4}, {1, 2, 5}});

    // Count edges from vertex 0 - vector has O(1) size()
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

TEST_CASE("mov std::ranges integration", "[dynamic_graph][mov][ranges]") {
  SECTION("ranges::count_if on vertices") {
    using G           = mov_void_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    G                        g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}});
    std::vector<vertex_data> vv = {{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};
    g.load_vertices(vv, std::identity{});

    // Count vertices with even values
    auto count = std::ranges::count_if(g, [](auto& pair) { return pair.second.value() % 2 == 0; });

    REQUIRE(count == 3); // 0, 2, 4
  }

  SECTION("ranges::find_if on vertices") {
    using G           = mov_void_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    G                        g({{0, 1}, {1, 2}, {2, 3}});
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};
    g.load_vertices(vv, std::identity{});

    auto it = std::ranges::find_if(g, [](auto& pair) { return pair.second.value() == 30; });

    REQUIRE(it != g.end());
    REQUIRE(it->second.value() == 30);
  }
}

//==================================================================================================
// 21. Algorithm Compatibility
//==================================================================================================

TEST_CASE("mov algorithm compatibility", "[dynamic_graph][mov][algorithms]") {
  SECTION("std::accumulate on vertex values") {
    using G           = mov_void_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    G                        g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
    g.load_vertices(vv, std::identity{});

    auto sum = std::accumulate(g.begin(), g.end(), 0, [](int acc, auto& pair) { return acc + pair.second.value(); });

    REQUIRE(sum == 15); // 1+2+3+4+5
  }

  SECTION("std::all_of on vertices") {
    using G           = mov_void_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;

    G                        g({{0, 1}, {1, 2}});
    std::vector<vertex_data> vv = {{0, 2}, {1, 4}, {2, 6}};
    g.load_vertices(vv, std::identity{});

    bool all_even = std::all_of(g.begin(), g.end(), [](auto& pair) { return pair.second.value() % 2 == 0; });

    REQUIRE(all_even);
  }
}

//==================================================================================================
// 22. Workflow Scenarios
//==================================================================================================

TEST_CASE("mov complete workflow scenarios", "[dynamic_graph][mov][workflow]") {
  SECTION("social network simulation with string IDs") {
    using G = mov_str_int_void_void;

    // Build social network with friendship strengths
    G g({{"alice", "bob", 5},
         {"alice", "charlie", 3},
         {"bob", "charlie", 4},
         {"bob", "dave", 2},
         {"charlie", "eve", 5}});

    REQUIRE(g.size() == 5);

    // Find person with most friends (highest out-degree)
    std::string most_social;
    size_t      max_friends = 0;

    for (auto& [key, vertex] : g) {
      size_t friend_count = vertex.edges().size(); // O(1) with vector
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
    using G           = mov_int_int_void;
    using vertex_data = copyable_vertex_t<uint32_t, int>;
    using edge_data   = copyable_edge_t<uint32_t, int>;

    // Step 1: Build initial graph
    G                        g;
    std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee, std::identity{});

    // Step 2: Query graph properties
    REQUIRE(g.size() == 3);

    size_t total_edges = 0;
    for (auto& [key, vertex] : g) {
      total_edges += vertex.edges().size(); // O(1) with vector
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
// Summary: Phase 3.1 mov_graph_traits Tests
//
// This file tests mov_graph_traits (map vertices + vector edges).
//
// Key behaviors being tested:
// - Map provides key-based storage (sparse vertex IDs)
// - Ordered iteration - vertices iterate in key order
// - String vertex IDs supported natively
// - Forward vertex iteration (descriptor iterators, despite map's bidirectional iterators)
// - Forward edge iteration (descriptor iterators, despite vector's random access iterators)
// - Proper copy/move semantics
// - Graph value storage
// - load_vertices() with associative containers
// - load_edges() with associative containers
// - Edge order preservation (first added appears first)
// - O(1) edge count via vector::size()
//
// Key differences from mol (map + list):
// - std::vector allows random access edge iteration (vs list's bidirectional)
// - Contiguous memory for better cache locality
// - Edge order is preserved: first added appears first (same as list)
//
// Key differences from vov (vector + vector):
// - Sparse vertex IDs - only referenced vertices are created
// - String vertex IDs natively supported
// - Vertices iterate in sorted key order (map property)
//
// Note: The vertices(g) CPO is tested separately.
//==================================================================================================
