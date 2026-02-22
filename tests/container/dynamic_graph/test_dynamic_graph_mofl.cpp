/**
 * @file test_dynamic_graph_mofl.cpp
 * @brief Tests for dynamic_graph with map vertices + forward_list edges
 * 
 * Phase 3.1: Map Vertex Containers
 * Tests mofl_graph_traits (map vertices + forward_list edges)
 * This is the first associative container combination for dynamic graphs.
 * 
 * Key differences from sequential containers (vector/deque):
 * 1. Key-based vertex identification - not index-based
 * 2. Descriptor iterators are forward-only (despite underlying container capabilities)
 * 3. Sparse vertex IDs by design - only referenced vertices are created
 * 4. Ordered iteration - vertices iterate in key order
 * 
 * Note: The vertices(g) CPO is tested separately in graph container interface tests.
 * This file focuses on mofl_graph_traits specific functionality.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/mofl_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <set>

using namespace graph::container;

// Type aliases for common test configurations with uint32_t vertex IDs
using mofl_void_void_void =
      dynamic_graph<void, void, void, uint32_t, false, false, mofl_graph_traits<void, void, void, uint32_t, false>>;
using mofl_int_void_void =
      dynamic_graph<int, void, void, uint32_t, false, false, mofl_graph_traits<int, void, void, uint32_t, false>>;
using mofl_void_int_void =
      dynamic_graph<void, int, void, uint32_t, false, false, mofl_graph_traits<void, int, void, uint32_t, false>>;
using mofl_int_int_void =
      dynamic_graph<int, int, void, uint32_t, false, false, mofl_graph_traits<int, int, void, uint32_t, false>>;
using mofl_void_void_int =
      dynamic_graph<void, void, int, uint32_t, false, false, mofl_graph_traits<void, void, int, uint32_t, false>>;
using mofl_int_int_int =
      dynamic_graph<int, int, int, uint32_t, false, false, mofl_graph_traits<int, int, int, uint32_t, false>>;

// Type aliases with string vertex IDs (the primary use case for map containers)
using mofl_str_void_void_void =
      dynamic_graph<void, void, void, std::string, false, false, mofl_graph_traits<void, void, void, std::string, false>>;
using mofl_str_int_void_void =
      dynamic_graph<int, void, void, std::string, false, false, mofl_graph_traits<int, void, void, std::string, false>>;
using mofl_str_void_int_void =
      dynamic_graph<void, int, void, std::string, false, false, mofl_graph_traits<void, int, void, std::string, false>>;
using mofl_str_int_int_int =
      dynamic_graph<int, int, int, std::string, false, false, mofl_graph_traits<int, int, int, std::string, false>>;

using mofl_sourced =
      dynamic_graph<void, void, void, uint32_t, true, false, mofl_graph_traits<void, void, void, uint32_t, true>>;
using mofl_int_sourced =
      dynamic_graph<int, void, void, uint32_t, true, false, mofl_graph_traits<int, void, void, uint32_t, true>>;

using mofl_str_sourced =
      dynamic_graph<void, void, void, std::string, true, false, mofl_graph_traits<void, void, void, std::string, true>>;

//==================================================================================================
// 1. Traits Verification Tests
//==================================================================================================

TEST_CASE("mofl traits verification", "[dynamic_graph][mofl][traits]") {
  SECTION("vertices_type is std::map") {
    using traits     = mofl_graph_traits<void, void, void, uint32_t, false>;
    using vertices_t = typename traits::vertices_type;
    // Verify it's a map by checking for map-specific members
    static_assert(requires { typename vertices_t::key_type; });
    static_assert(requires { typename vertices_t::mapped_type; });
    REQUIRE(true); // Compilation success means the test passes
  }

  SECTION("edges_type is std::forward_list") {
    using traits  = mofl_graph_traits<void, void, void, uint32_t, false>;
    using edges_t = typename traits::edges_type;
    // forward_list has before_begin() which is unique to it
    static_assert(requires(edges_t e) { e.before_begin(); });
    // forward_list also has push_front but not push_back
    static_assert(requires(edges_t e, typename edges_t::value_type v) { e.push_front(v); });
    REQUIRE(true);
  }

  SECTION("vertex_id_type can be string") {
    using traits = mofl_graph_traits<void, void, void, std::string, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
    REQUIRE(true);
  }

  SECTION("sourced flag is preserved") {
    using traits_unsourced = mofl_graph_traits<void, void, void, uint32_t, false>;
    using traits_sourced   = mofl_graph_traits<void, void, void, uint32_t, true>;
    static_assert(traits_unsourced::sourced == false);
    static_assert(traits_sourced::sourced == true);
    REQUIRE(true);
  }

  SECTION("vertex_id_type for uint32_t") {
    using traits = mofl_graph_traits<void, void, void, uint32_t, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, uint32_t>);
    REQUIRE(true);
  }

  SECTION("custom vertex_id_type") {
    using traits = mofl_graph_traits<void, void, void, int64_t, false>;
    static_assert(std::same_as<typename traits::vertex_id_type, int64_t>);
    REQUIRE(true);
  }
}

//==================================================================================================
// 2. Iterator Category Tests
//==================================================================================================

TEST_CASE("mofl iterator categories", "[dynamic_graph][mofl][iterators]") {
  SECTION("underlying map iterators are bidirectional") {
    // Note: The raw map iterator is bidirectional, but the graph library
    // wraps vertices with descriptor iterators which are forward-only.
    using G      = mofl_void_void_void;
    using iter_t = typename G::vertices_type::iterator;
    static_assert(std::bidirectional_iterator<iter_t>);
    static_assert(!std::random_access_iterator<iter_t>);
    REQUIRE(true);
  }

  SECTION("edge iterators are forward only (forward_list iterator)") {
    using traits      = mofl_graph_traits<void, void, void, uint32_t, false>;
    using edges_t     = typename traits::edges_type;
    using edge_iter_t = typename edges_t::iterator;
    // forward_list iterators are forward iterators
    static_assert(std::forward_iterator<edge_iter_t>);
    // but NOT bidirectional
    static_assert(!std::bidirectional_iterator<edge_iter_t>);
    REQUIRE(true);
  }

  SECTION("graph is a range") {
    static_assert(std::ranges::range<mofl_void_void_void>);
    static_assert(std::ranges::range<mofl_int_int_int>);
    static_assert(std::ranges::range<mofl_str_void_void_void>);
    REQUIRE(true);
  }
}

//==================================================================================================
// 3. Construction Tests
//==================================================================================================

TEST_CASE("mofl construction", "[dynamic_graph][mofl][construction]") {
  SECTION("default constructor creates empty graph") {
    mofl_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with void types") {
    mofl_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int edge values") {
    mofl_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int vertex values") {
    mofl_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int graph value") {
    mofl_void_void_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with all int values") {
    mofl_int_int_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("constructor with graph value - int GV") {
    mofl_void_void_int g(42);
    REQUIRE(g.size() == 0);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("copy constructor") {
    mofl_int_int_int g1;
    mofl_int_int_int g2(g1);
    REQUIRE(g2.size() == g1.size());
  }

  SECTION("move constructor") {
    mofl_int_int_int g1;
    mofl_int_int_int g2(std::move(g1));
    REQUIRE(g2.size() == 0);
  }

  SECTION("copy assignment") {
    mofl_int_int_int g1, g2;
    g2 = g1;
    REQUIRE(g2.size() == g1.size());
  }

  SECTION("move assignment") {
    mofl_int_int_int g1, g2;
    g2 = std::move(g1);
    REQUIRE(g2.size() == 0);
  }
}

TEST_CASE("mofl construction with string vertex IDs", "[dynamic_graph][mofl][construction][string]") {
  SECTION("default constructor creates empty graph") {
    mofl_str_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int edge values") {
    mofl_str_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with int vertex values") {
    mofl_str_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("default constructor with all int values") {
    mofl_str_int_int_int g;
    REQUIRE(g.size() == 0);
  }
}

TEST_CASE("mofl construction sourced", "[dynamic_graph][mofl][construction][sourced]") {
  SECTION("sourced edge construction with uint32_t IDs") {
    mofl_sourced g;
    REQUIRE(g.size() == 0);
  }

  SECTION("sourced with edge value construction") {
    mofl_int_sourced g;
    REQUIRE(g.size() == 0);
  }

  SECTION("sourced edge construction with string IDs") {
    mofl_str_sourced g;
    REQUIRE(g.size() == 0);
  }
}

//==================================================================================================
// 4. Basic Properties Tests
//==================================================================================================

TEST_CASE("mofl properties", "[dynamic_graph][mofl][properties]") {
  SECTION("size() on empty graph") {
    mofl_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("const graph methods") {
    const mofl_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("begin() == end() for empty graph") {
    mofl_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("const begin() == const end() for empty graph") {
    const mofl_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("cbegin() == cend() for empty graph") {
    mofl_void_void_void g;
    REQUIRE(g.cbegin() == g.cend());
  }
}

TEST_CASE("mofl properties with string IDs", "[dynamic_graph][mofl][properties][string]") {
  SECTION("size() on empty graph") {
    mofl_str_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("begin() == end() for empty graph") {
    mofl_str_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }
}

//==================================================================================================
// 5. Type Alias Tests
//==================================================================================================

TEST_CASE("mofl type aliases", "[dynamic_graph][mofl][types]") {
  SECTION("graph type aliases are correct") {
    using G = mofl_int_int_int;
    static_assert(std::same_as<typename G::value_type, int>); // GV
    static_assert(G::sourced == false);
    REQUIRE(true);
  }

  SECTION("sourced graph type aliases are correct") {
    using G = mofl_sourced;
    static_assert(G::sourced == true);
    REQUIRE(true);
  }

  SECTION("string key graph type aliases are correct") {
    using G      = mofl_str_int_int_int;
    using traits = typename G::graph_traits;
    static_assert(std::same_as<typename traits::vertex_id_type, std::string>);
    REQUIRE(true);
  }
}

//==================================================================================================
// 6. Initializer List Construction Tests (uint32_t vertex IDs)
//==================================================================================================

TEST_CASE("mofl initializer_list construction", "[dynamic_graph][mofl][initializer_list]") {
  SECTION("empty initializer list") {
    using G = mofl_void_void_void;
    G g({});
    REQUIRE(g.size() == 0);
  }

  SECTION("single edge without value") {
    using G = mofl_void_void_void;
    G g({{0, 1}});
    REQUIRE(g.size() == 2);
  }

  SECTION("single edge with value") {
    using G = mofl_int_void_void;
    G g({{0, 1, 42}});
    REQUIRE(g.size() == 2);
  }

  SECTION("multiple edges from same source") {
    using G = mofl_int_void_void;
    G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});
    REQUIRE(g.size() == 4);
  }

  SECTION("triangle graph") {
    using G = mofl_void_void_void;
    G g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(g.size() == 3);
  }

  SECTION("self-loop") {
    using G = mofl_void_void_void;
    G g({{0, 0}});
    REQUIRE(g.size() == 1);
  }

  SECTION("parallel edges") {
    using G = mofl_int_void_void;
    G g({{0, 1, 1}, {0, 1, 2}, {0, 1, 3}});
    REQUIRE(g.size() == 2);
  }

  SECTION("sparse vertex IDs - only referenced vertices created") {
    using G = mofl_void_void_void;
    G g({{100, 200}});
    REQUIRE(g.size() == 2);
    // With mofl, only vertices 100 and 200 are created
    // (unlike vector-based which would create 0..200)
  }

  SECTION("star graph (center with many spokes)") {
    using G = mofl_int_void_void;
    G g({{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4}, {0, 5, 5}});
    REQUIRE(g.size() == 6);
  }

  SECTION("complete graph K4") {
    using G = mofl_int_void_void;
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
    using G = mofl_void_void_int;
    G g(42, {{0, 1}, {1, 2}});
    REQUIRE(g.graph_value() == 42);
    REQUIRE(g.size() == 3);
  }
}

//==================================================================================================
// 7. Initializer List Construction Tests (string vertex IDs)
//==================================================================================================

TEST_CASE("mofl initializer_list construction string IDs", "[dynamic_graph][mofl][initializer_list][string]") {
  SECTION("single edge with string IDs") {
    using G = mofl_str_void_void_void;
    G g({{"alice", "bob"}});
    REQUIRE(g.size() == 2);
  }

  SECTION("string IDs with edge values") {
    using G = mofl_str_int_void_void;
    G g({{"alice", "bob", 10}, {"bob", "charlie", 20}});
    REQUIRE(g.size() == 3);
  }

  SECTION("social network graph") {
    using G = mofl_str_int_void_void;
    G g({{"alice", "bob", 5},
         {"alice", "charlie", 3},
         {"bob", "charlie", 4},
         {"bob", "dave", 2},
         {"charlie", "eve", 5}});
    REQUIRE(g.size() == 5);
  }

  SECTION("sourced edges with string IDs") {
    using G = mofl_str_sourced;
    G g({{"alice", "bob"}, {"bob", "charlie"}});
    REQUIRE(g.size() == 3);
  }
}

//==================================================================================================
// 8. Graph Value Tests
//==================================================================================================

TEST_CASE("mofl graph value access", "[dynamic_graph][mofl][graph_value]") {
  SECTION("graph_value() returns reference") {
    mofl_void_void_int g(100);
    REQUIRE(g.graph_value() == 100);
    g.graph_value() = 200;
    REQUIRE(g.graph_value() == 200);
  }

  SECTION("const graph_value()") {
    const mofl_void_void_int g(42);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("graph value preserved through copy") {
    mofl_void_void_int g1(100);
    mofl_void_void_int g2 = g1;
    REQUIRE(g2.graph_value() == 100);
    g2.graph_value() = 200;
    REQUIRE(g1.graph_value() == 100); // g1 unchanged
  }

  SECTION("graph value preserved through move") {
    mofl_void_void_int g1(100);
    mofl_void_void_int g2 = std::move(g1);
    REQUIRE(g2.graph_value() == 100);
  }
}

//==================================================================================================
// 9. Graph Iteration Tests (direct iteration, not via vertices() CPO)
//==================================================================================================

TEST_CASE("mofl graph iteration", "[dynamic_graph][mofl][iteration]") {
  SECTION("iterate over empty graph") {
    mofl_void_void_void g;
    size_t              count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("iterate over graph with vertices") {
    using G = mofl_void_void_void;
    G g({{0, 1}, {1, 2}});

    size_t count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("const iteration") {
    using G = mofl_void_void_void;
    const G g({{0, 1}, {1, 2}});

    size_t count = 0;
    for ([[maybe_unused]] const auto& v : g) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("iterate string key graph") {
    using G = mofl_str_void_void_void;
    G g({{"alice", "bob"}, {"bob", "charlie"}});

    size_t count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 3);
  }
}

//==================================================================================================
// 10. Edge Cases and Error Handling
//==================================================================================================

TEST_CASE("mofl edge cases", "[dynamic_graph][mofl][edge_cases]") {
  SECTION("graph with single vertex (self-loop)") {
    using G = mofl_void_void_void;
    G g({{0, 0}});
    REQUIRE(g.size() == 1);
  }

  SECTION("clear() empties the graph") {
    using G = mofl_int_void_void;
    G g({{0, 1, 10}, {1, 2, 20}});
    REQUIRE(g.size() == 3);
    g.clear();
    REQUIRE(g.size() == 0);
  }

  SECTION("multiple clears are safe") {
    using G = mofl_void_void_void;
    G g({{0, 1}});
    g.clear();
    g.clear();
    g.clear();
    REQUIRE(g.size() == 0);
  }

  SECTION("swap two graphs") {
    using G = mofl_void_void_int;
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
    using G = mofl_void_void_void;
    G g({{1000000, 2000000}});
    REQUIRE(g.size() == 2); // Only 2 vertices, not 2000001
  }
}

//==================================================================================================
// 11. Const Correctness Tests
//==================================================================================================

TEST_CASE("mofl const correctness", "[dynamic_graph][mofl][const]") {
  SECTION("const graph properties") {
    using G = mofl_int_void_void;
    const G g({{0, 1, 10}, {1, 2, 20}});

    REQUIRE(g.size() == 3);
    REQUIRE(g.begin() != g.end());
  }

  SECTION("const graph iteration") {
    using G = mofl_int_void_void;
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

TEST_CASE("mofl memory management", "[dynamic_graph][mofl][memory]") {
  SECTION("multiple independent graphs") {
    using G = mofl_void_void_int;
    G g1(100, {{0, 1}});
    G g2(200, {{1, 2}});
    G g3(300, {{2, 3}});

    REQUIRE(g1.graph_value() == 100);
    REQUIRE(g2.graph_value() == 200);
    REQUIRE(g3.graph_value() == 300);
  }

  SECTION("copy does not alias") {
    using G = mofl_void_void_int;
    G g1(100, {{0, 1}});
    G g2 = g1;

    g2.graph_value() = 200;
    REQUIRE(g1.graph_value() == 100);
    REQUIRE(g2.graph_value() == 200);
  }

  SECTION("clear allows reuse") {
    using G = mofl_int_void_void;
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

TEST_CASE("mofl template instantiation", "[dynamic_graph][mofl][compilation]") {
  [[maybe_unused]] mofl_void_void_void     g1;
  [[maybe_unused]] mofl_int_void_void      g2;
  [[maybe_unused]] mofl_void_int_void      g3;
  [[maybe_unused]] mofl_int_int_void       g4;
  [[maybe_unused]] mofl_void_void_int      g5;
  [[maybe_unused]] mofl_int_int_int        g6;
  [[maybe_unused]] mofl_sourced            g7;
  [[maybe_unused]] mofl_int_sourced        g8;
  [[maybe_unused]] mofl_str_void_void_void g9;
  [[maybe_unused]] mofl_str_int_void_void  g10;
  [[maybe_unused]] mofl_str_int_int_int    g11;
  [[maybe_unused]] mofl_str_sourced        g12;

  REQUIRE(true);
}

//==================================================================================================
// 14. Comparison with vofl Behavior
//==================================================================================================

TEST_CASE("mofl sparse vertex behavior", "[dynamic_graph][mofl][sparse]") {
  SECTION("only referenced vertices are created") {
    using G = mofl_void_void_void;

    // Edge from 10 to 20 - should only create 2 vertices
    G g({{10, 20}});
    REQUIRE(g.size() == 2);

    // Unlike vector-based graphs, no vertices 0-9 or 11-19 exist
  }

  SECTION("multiple sparse edges") {
    using G = mofl_void_void_void;
    G g({{100, 200}, {300, 400}, {500, 600}});
    REQUIRE(g.size() == 6);
  }

  SECTION("reverse order vertex creation") {
    using G = mofl_void_void_void;
    // Higher ID to lower ID
    G g({{100, 50}, {200, 25}});
    REQUIRE(g.size() == 4);
  }
}

//==================================================================================================
// 15. Vertex Accessor Methods Tests
//==================================================================================================

TEST_CASE("mofl contains_vertex", "[dynamic_graph][mofl][accessor][contains_vertex]") {
  SECTION("uint32_t vertex IDs") {
    using G = mofl_void_void_void;
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
    using G = mofl_str_void_void_void;
    G g({{"alice", "bob"}, {"bob", "charlie"}});

    REQUIRE(g.contains_vertex("alice"));
    REQUIRE(g.contains_vertex("bob"));
    REQUIRE(g.contains_vertex("charlie"));

    REQUIRE_FALSE(g.contains_vertex("david"));
    REQUIRE_FALSE(g.contains_vertex(""));
    REQUIRE_FALSE(g.contains_vertex("Alice")); // case sensitive
  }

  SECTION("empty graph") {
    using G = mofl_void_void_void;
    G g;

    REQUIRE_FALSE(g.contains_vertex(0));
    REQUIRE_FALSE(g.contains_vertex(1));
  }

  SECTION("const graph") {
    using G = mofl_void_void_void;
    const G g({{0, 1}, {2, 3}});

    REQUIRE(g.contains_vertex(0));
    REQUIRE(g.contains_vertex(1));
    REQUIRE_FALSE(g.contains_vertex(5));
  }
}

TEST_CASE("mofl try_find_vertex", "[dynamic_graph][mofl][accessor][try_find_vertex]") {
  SECTION("uint32_t vertex IDs - found") {
    using G = mofl_void_void_void;
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
    using G = mofl_void_void_void;
    G g({{0, 1}, {5, 10}});

    auto it3 = g.try_find_vertex(3);
    REQUIRE(it3 == g.end());

    auto it100 = g.try_find_vertex(100);
    REQUIRE(it100 == g.end());
  }

  SECTION("string vertex IDs") {
    using G = mofl_str_void_void_void;
    G g({{"alice", "bob"}, {"bob", "charlie"}});

    auto it_alice = g.try_find_vertex("alice");
    REQUIRE(it_alice != g.end());
    REQUIRE(it_alice->first == "alice");

    auto it_david = g.try_find_vertex("david");
    REQUIRE(it_david == g.end());
  }

  SECTION("does not modify container") {
    using G = mofl_void_void_void;
    G g({{0, 1}});
    REQUIRE(g.size() == 2);

    // Looking for non-existent vertex should NOT add it
    auto it = g.try_find_vertex(999);
    REQUIRE(it == g.end());
    REQUIRE(g.size() == 2); // Size unchanged - critical for safety
  }

  SECTION("const graph") {
    using G = mofl_void_void_void;
    const G g({{0, 1}, {2, 3}});

    auto it = g.try_find_vertex(0);
    REQUIRE(it != g.end());
    REQUIRE(it->first == 0);

    auto it_missing = g.try_find_vertex(99);
    REQUIRE(it_missing == g.end());
  }
}

TEST_CASE("mofl vertex_at", "[dynamic_graph][mofl][accessor][vertex_at]") {
  SECTION("uint32_t vertex IDs - found") {
    using G = mofl_void_void_void;
    G g({{0, 1}, {1, 2}});

    // Should not throw
    REQUIRE_NOTHROW(g.vertex_at(0));
    REQUIRE_NOTHROW(g.vertex_at(1));
    REQUIRE_NOTHROW(g.vertex_at(2));
  }

  SECTION("uint32_t vertex IDs - throws on not found") {
    using G = mofl_void_void_void;
    G g({{0, 1}});

    REQUIRE_THROWS_AS(g.vertex_at(5), std::out_of_range);
    REQUIRE_THROWS_AS(g.vertex_at(100), std::out_of_range);
  }

  SECTION("string vertex IDs") {
    using G = mofl_str_void_void_void;
    G g({{"alice", "bob"}});

    REQUIRE_NOTHROW(g.vertex_at("alice"));
    REQUIRE_NOTHROW(g.vertex_at("bob"));
    REQUIRE_THROWS_AS(g.vertex_at("charlie"), std::out_of_range);
  }

  SECTION("modify vertex through vertex_at") {
    using G = mofl_void_int_void; // has vertex value
    G g({{0, 1}});

    g.vertex_at(0).value() = 42;
    g.vertex_at(1).value() = 100;

    REQUIRE(g.vertex_at(0).value() == 42);
    REQUIRE(g.vertex_at(1).value() == 100);
  }

  SECTION("const graph") {
    using G = mofl_void_void_void;
    const G g({{0, 1}, {2, 3}});

    REQUIRE_NOTHROW(g.vertex_at(0));
    REQUIRE_THROWS_AS(g.vertex_at(99), std::out_of_range);
  }

  SECTION("does not modify container") {
    using G = mofl_void_void_void;
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

TEST_CASE("mofl load_vertices", "[dynamic_graph][mofl][load_vertices]") {
  SECTION("uint32_t IDs - basic load") {
    using G           = mofl_void_int_void; // vertex value = int
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
    using G           = mofl_void_int_void;
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
    using G           = mofl_str_void_int_void; // string keys, vertex value = int
    using vertex_data = copyable_vertex_t<std::string, int>;

    G g({{"alice", "bob"}});
    REQUIRE(g.size() == 2);

    std::vector<vertex_data> vv = {{"alice", 100}, {"bob", 200}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.vertex_at("alice").value() == 100);
    REQUIRE(g.vertex_at("bob").value() == 200);
  }

  SECTION("string IDs - load creates new vertices") {
    using G           = mofl_str_void_int_void;
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
    using G           = mofl_void_int_void;
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
    using G           = mofl_void_int_void;
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

TEST_CASE("mofl load_edges explicit", "[dynamic_graph][mofl][load_edges]") {
  SECTION("uint32_t IDs - basic load") {
    using G         = mofl_int_void_void; // edge value = int
    using edge_data = copyable_edge_t<uint32_t, int>;

    G g;
    REQUIRE(g.size() == 0);

    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4); // Vertices 0, 1, 2, 3 auto-created
  }

  SECTION("uint32_t IDs - sparse vertex creation") {
    using G         = mofl_void_void_void;
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
    using G         = mofl_str_int_void_void; // string keys, edge value = int
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
    using G         = mofl_int_void_void;
    using edge_data = copyable_edge_t<uint32_t, int>;

    G g({{0, 1, 10}});
    REQUIRE(g.size() == 2);

    // Add more edges
    std::vector<edge_data> ee = {{1, 2, 20}, {2, 3, 30}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 4); // New vertices created
  }

  SECTION("with projection function") {
    using G         = mofl_int_void_void;
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

TEST_CASE("mofl load_vertices and load_edges combined", "[dynamic_graph][mofl][load]") {
  SECTION("load edges then vertices") {
    using G           = mofl_int_int_void; // edge value = int, vertex value = int
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
    using G           = mofl_int_int_void;
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
    using G           = mofl_str_int_int_int; // all values are int
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
// Summary: Phase 3.1 mofl_graph_traits Tests
//
// This file tests mofl_graph_traits (map vertices + forward_list edges).
//
// Key behaviors being tested:
// - Map provides key-based storage (sparse vertex IDs)
// - Ordered iteration - vertices iterate in key order
// - String vertex IDs supported
// - Forward vertex iteration (descriptor iterators, despite map's bidirectional iterators)
// - Forward-only edge iterators (from forward_list)
// - Proper copy/move semantics
// - Graph value storage
// - load_vertices() with associative containers
// - load_edges() with associative containers
//
// Note: The vertices(g) CPO is tested separately.
//==================================================================================================
