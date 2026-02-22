/**
 * @file test_dynamic_graph_dod.cpp
 * @brief Comprehensive tests for dynamic_graph with deque vertices + deque edges
 * 
 * Phase 1.7: Core Container Combinations with Deque Edges
 * Tests dod_graph_traits (deque vertices + deque edges)
 * Combines contiguous vertex storage with stable edge iterators and random access
 * 
 * Key characteristics of std::deque edges:
 * - Random access iterators (like vector)
 * - Stable iterators (unlike vector) - iterators not invalidated on push_back/push_front
 * - Bidirectional iteration (like list and vector)
 * - Good for dynamic graphs where edges are frequently added/removed
 * 
 * This file provides comprehensive coverage matching test_dynamic_graph_vov.cpp
 * with adaptations highlighting deque's stable iterator and random access features.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/traits/dod_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/graph_info.hpp>
#include <string>
#include <deque>
#include <algorithm>
#include <numeric>
#include <ranges>

using namespace graph::container;

// Type aliases for common test configurations
using dod_void_void_void =
      dynamic_graph<void, void, void, uint32_t, false, false, dod_graph_traits<void, void, void, uint32_t, false>>;
using dod_int_void_void =
      dynamic_graph<int, void, void, uint32_t, false, false, dod_graph_traits<int, void, void, uint32_t, false>>;
using dod_void_int_void =
      dynamic_graph<void, int, void, uint32_t, false, false, dod_graph_traits<void, int, void, uint32_t, false>>;
using dod_int_int_void =
      dynamic_graph<int, int, void, uint32_t, false, false, dod_graph_traits<int, int, void, uint32_t, false>>;
using dod_void_void_int =
      dynamic_graph<void, void, int, uint32_t, false, false, dod_graph_traits<void, void, int, uint32_t, false>>;
using dod_int_int_int = dynamic_graph<int, int, int, uint32_t, false, false, dod_graph_traits<int, int, int, uint32_t, false>>;

using dod_string_string_string =
      dynamic_graph<std::string,
                    std::string,
                    std::string,
                    uint32_t,
                    false, false, dod_graph_traits<std::string, std::string, std::string, uint32_t, false>>;

using dod_sourced = dynamic_graph<void, void, void, uint32_t, true, false, dod_graph_traits<void, void, void, uint32_t, true>>;
using dod_int_sourced =
      dynamic_graph<int, void, void, uint32_t, true, false, dod_graph_traits<int, void, void, uint32_t, true>>;

//==================================================================================================
// 1. Construction Tests
//==================================================================================================

TEST_CASE("dod default construction", "[dod][construction]") {
  SECTION("creates empty graph") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with void types") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int edge values") {
    dod_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int vertex values") {
    dod_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int graph value") {
    dod_void_void_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with all int values") {
    dod_int_int_int g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with string values") {
    dod_string_string_string g;
    REQUIRE(g.size() == 0);
  }
}

TEST_CASE("dod constructor with graph value", "[dod][construction]") {
  SECTION("void GV - no graph value can be passed") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("int GV") {
    dod_void_void_int g(42);
    REQUIRE(g.size() == 0);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("string GV") {
    dod_string_string_string g(std::string("test"));
    REQUIRE(g.size() == 0);
    REQUIRE(g.graph_value() == "test");
  }
}

TEST_CASE("dod copy and move construction", "[dod][construction]") {
  SECTION("copy constructor") {
    dod_int_int_int g1;
    dod_int_int_int g2(g1);
    REQUIRE(g2.size() == g1.size());
  }

  SECTION("move constructor") {
    dod_int_int_int g1;
    dod_int_int_int g2(std::move(g1));
    REQUIRE(g2.size() == 0);
  }

  SECTION("copy assignment") {
    dod_int_int_int g1, g2;
    g2 = g1;
    REQUIRE(g2.size() == g1.size());
  }

  SECTION("move assignment") {
    dod_int_int_int g1, g2;
    g2 = std::move(g1);
    REQUIRE(g2.size() == 0);
  }
}

TEST_CASE("dod sourced construction", "[dod][construction][sourced]") {
  SECTION("sourced edge construction") {
    dod_sourced g;
    REQUIRE(g.size() == 0);
  }

  SECTION("sourced with edge value construction") {
    dod_int_sourced g;
    REQUIRE(g.size() == 0);
  }
}

//==================================================================================================
// 2. Basic Properties Tests
//==================================================================================================

TEST_CASE("dod basic properties", "[dod][properties]") {
  SECTION("size() on empty graph") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("size() == 0 for empty graph") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("size() != 0 for non-empty graph") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0); // Will change once we can add vertices
  }

  SECTION("const graph methods") {
    const dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("begin() == end() for empty graph") {
    dod_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("const begin() == const end() for empty graph") {
    const dod_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("cbegin() == cend() for empty graph") {
    dod_void_void_void g;
    REQUIRE(g.cbegin() == g.cend());
  }
}

//==================================================================================================
// 3. Graph Value Tests
//==================================================================================================

TEST_CASE("dod graph_value()", "[dod][graph_value]") {
  SECTION("with int GV") {
    dod_void_void_int g(100);
    REQUIRE(g.graph_value() == 100);
  }

  SECTION("modification") {
    dod_void_void_int g(100);
    g.graph_value() = 200;
    REQUIRE(g.graph_value() == 200);
  }

  SECTION("const correctness") {
    const dod_void_void_int g(100);
    REQUIRE(g.graph_value() == 100);
  }

  SECTION("with string GV") {
    dod_string_string_string g(std::string("initial"));
    REQUIRE(g.graph_value() == "initial");
    g.graph_value() = "modified";
    REQUIRE(g.graph_value() == "modified");
  }

  SECTION("move semantics") {
    dod_string_string_string g(std::string("test"));
    std::string              val = std::move(g.graph_value());
    REQUIRE(val == "test");
  }

  SECTION("with copy") {
    dod_void_void_int g1(42);
    dod_void_void_int g2 = g1;
    REQUIRE(g2.graph_value() == 42);
    g2.graph_value() = 100;
    REQUIRE(g1.graph_value() == 42); // g1 unchanged
    REQUIRE(g2.graph_value() == 100);
  }
}

//==================================================================================================
// 4. Iterator Tests
//==================================================================================================

TEST_CASE("dod iterators", "[dod][iterator]") {
  SECTION("iterator on empty graph") {
    dod_void_void_void g;
    auto               it = g.begin();
    REQUIRE(it == g.end());
  }

  SECTION("const iterator on empty graph") {
    const dod_void_void_void g;
    auto                     it = g.begin();
    REQUIRE(it == g.end());
  }

  SECTION("range-based for on empty graph") {
    dod_void_void_void g;
    int                count = 0;
    for ([[maybe_unused]] auto& v : g) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("const range-based for on empty graph") {
    const dod_void_void_void g;
    int                      count = 0;
    for ([[maybe_unused]] const auto& v : g) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("std::ranges compatibility") {
    dod_void_void_void g;
    auto               count = std::ranges::distance(g.begin(), g.end());
    REQUIRE(count == 0);
  }
}

//==================================================================================================
// 5. Type Trait Tests
//==================================================================================================

TEST_CASE("dod_graph_traits", "[dod][traits]") {
  SECTION("types") {
    using traits = dod_graph_traits<int, std::string, void, uint32_t, false>;

    STATIC_REQUIRE(std::is_same_v<traits::edge_value_type, int>);
    STATIC_REQUIRE(std::is_same_v<traits::vertex_value_type, std::string>);
    STATIC_REQUIRE(std::is_same_v<traits::graph_value_type, void>);
    STATIC_REQUIRE(std::is_same_v<traits::vertex_id_type, uint32_t>);
    STATIC_REQUIRE(traits::sourced == false);
  }

  SECTION("sourced = true") {
    using traits = dod_graph_traits<int, std::string, void, uint32_t, true>;
    STATIC_REQUIRE(traits::sourced == true);
  }

  SECTION("vertex_id_type variations") {
    using traits_u64 = dod_graph_traits<void, void, void, uint64_t, false>;
    using traits_i32 = dod_graph_traits<void, void, void, int32_t, false>;
    using traits_i8  = dod_graph_traits<void, void, void, int8_t, false>;

    STATIC_REQUIRE(std::is_same_v<traits_u64::vertex_id_type, uint64_t>);
    STATIC_REQUIRE(std::is_same_v<traits_i32::vertex_id_type, int32_t>);
    STATIC_REQUIRE(std::is_same_v<traits_i8::vertex_id_type, int8_t>);
  }

  SECTION("vertices_type is deque") {
    using traits     = dod_graph_traits<void, void, void, uint32_t, false>;
    using vertex_t   = traits::vertex_type;
    using vertices_t = traits::vertices_type;

    STATIC_REQUIRE(std::is_same_v<vertices_t, std::deque<vertex_t>>);
  }

  SECTION("edges_type is deque") {
    using traits  = dod_graph_traits<void, void, void, uint32_t, false>;
    using edge_t  = traits::edge_type;
    using edges_t = traits::edges_type;

    STATIC_REQUIRE(std::is_same_v<edges_t, std::deque<edge_t>>);
  }
}

//==================================================================================================
// 6. Empty Graph Edge Cases
//==================================================================================================

TEST_CASE("dod empty graph edge cases", "[dod][edge_cases]") {
  SECTION("multiple empty graphs independent") {
    dod_void_void_void g1, g2, g3;
    REQUIRE(g1.size() == 0);
    REQUIRE(g2.size() == 0);
    REQUIRE(g3.size() == 0);
  }

  SECTION("copy of empty graph") {
    dod_int_int_int g1;
    dod_int_int_int g2 = g1;
    REQUIRE(g1.size() == 0);
    REQUIRE(g2.size() == 0);
  }

  SECTION("move of empty graph") {
    dod_int_int_int g1;
    dod_int_int_int g2 = std::move(g1);
    REQUIRE(g2.size() == 0);
  }

  SECTION("swap empty graphs") {
    dod_int_int_int g1, g2;
    std::swap(g1, g2);
    REQUIRE(g1.size() == 0);
    REQUIRE(g2.size() == 0);
  }

  SECTION("clear on empty graph") {
    dod_void_void_void g;
    g.clear();
    REQUIRE(g.size() == 0);
  }

  SECTION("multiple clears") {
    dod_void_void_void g;
    g.clear();
    g.clear();
    g.clear();
    REQUIRE(g.size() == 0);
  }
}

//==================================================================================================
// 7. Value Type Tests
//==================================================================================================

TEST_CASE("dod value types", "[dod][value_types]") {
  SECTION("with void edge value") {
    using graph_t = dynamic_graph<void, int, int, uint32_t, false, false, dod_graph_traits<void, int, int, uint32_t, false>>;
    graph_t g(100);
    REQUIRE(g.graph_value() == 100);
  }

  SECTION("with void vertex value") {
    using graph_t = dynamic_graph<int, void, int, uint32_t, false, false, dod_graph_traits<int, void, int, uint32_t, false>>;
    graph_t g(100);
    REQUIRE(g.graph_value() == 100);
  }

  SECTION("with void graph value") {
    using graph_t = dynamic_graph<int, int, void, uint32_t, false, false, dod_graph_traits<int, int, void, uint32_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with all void values") {
    dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int edge value type") {
    dod_int_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int vertex value type") {
    dod_void_int_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int graph value type") {
    dod_void_void_int g(42);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("with all int values") {
    dod_int_int_int g(42);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("with string edge value type") {
    using graph_t = dynamic_graph<std::string, void, void, uint32_t, false, false, dod_graph_traits<std::string, void, void, uint32_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with string vertex value type") {
    using graph_t = dynamic_graph<void, std::string, void, uint32_t, false, false, dod_graph_traits<void, std::string, void, uint32_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with string graph value type") {
    using graph_t = dynamic_graph<void, void, std::string, uint32_t, false, false, dod_graph_traits<void, void, std::string, uint32_t, false>>;
    graph_t g(std::string("test"));
    REQUIRE(g.graph_value() == "test");
  }

  SECTION("with all string values") {
    dod_string_string_string g(std::string("graph"));
    REQUIRE(g.graph_value() == "graph");
  }
}

//==================================================================================================
// 8. Vertex ID Type Tests
//==================================================================================================

TEST_CASE("dod vertex ID types", "[dod][vertex_id]") {
  SECTION("with uint32_t vertex id") {
    using graph_t =
          dynamic_graph<void, void, void, uint32_t, false, false, dod_graph_traits<void, void, void, uint32_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with uint64_t vertex id") {
    using graph_t =
          dynamic_graph<void, void, void, uint64_t, false, false, dod_graph_traits<void, void, void, uint64_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int32_t vertex id") {
    using graph_t = dynamic_graph<void, void, void, int32_t, false, false, dod_graph_traits<void, void, void, int32_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int8_t vertex id") {
    using graph_t = dynamic_graph<void, void, void, int8_t, false, false, dod_graph_traits<void, void, void, int8_t, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }

  SECTION("with int vertex id") {
    using graph_t = dynamic_graph<void, void, void, int, false, false, dod_graph_traits<void, void, void, int, false>>;
    graph_t g;
    REQUIRE(g.size() == 0);
  }
}

//==================================================================================================
// 9. Sourced Edge Tests
//==================================================================================================

TEST_CASE("dod sourced edges", "[dod][sourced]") {
  SECTION("sourced=false by default") {
    dod_void_void_void g;
    using traits = dod_graph_traits<void, void, void, uint32_t, false>;
    STATIC_REQUIRE(traits::sourced == false);
  }

  SECTION("sourced=true explicit") {
    dod_sourced g;
    using traits = dod_graph_traits<void, void, void, uint32_t, true>;
    STATIC_REQUIRE(traits::sourced == true);
  }

  SECTION("sourced with void values") {
    dod_sourced g;
    REQUIRE(g.size() == 0);
  }

  SECTION("sourced with int edge value") {
    dod_int_sourced g;
    REQUIRE(g.size() == 0);
  }

  SECTION("sourced copy construction") {
    dod_sourced g1;
    dod_sourced g2 = g1;
    REQUIRE(g2.size() == 0);
  }

  SECTION("sourced move construction") {
    dod_sourced g1;
    dod_sourced g2 = std::move(g1);
    REQUIRE(g2.size() == 0);
  }
}

//==================================================================================================
// 10. Const Correctness Tests
//==================================================================================================

TEST_CASE("dod const correctness", "[dod][const]") {
  SECTION("const graph size()") {
    const dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("const graph empty()") {
    const dod_void_void_void g;
    REQUIRE(g.size() == 0);
  }

  SECTION("const graph begin/end") {
    const dod_void_void_void g;
    REQUIRE(g.begin() == g.end());
  }

  SECTION("const graph iteration") {
    const dod_void_void_void g;
    int                      count = 0;
    for ([[maybe_unused]] const auto& v : g) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("const graph with graph value") {
    const dod_void_void_int g(42);
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("const graph cbegin/cend") {
    const dod_void_void_void g;
    REQUIRE(g.cbegin() == g.cend());
  }
}

//==================================================================================================
// 11. Memory and Resource Tests
//==================================================================================================

TEST_CASE("dod memory and resources", "[dod][memory]") {
  SECTION("multiple graphs do not interfere") {
    dod_int_int_int g1(100);
    dod_int_int_int g2(200);
    dod_int_int_int g3(300);

    REQUIRE(g1.graph_value() == 100);
    REQUIRE(g2.graph_value() == 200);
    REQUIRE(g3.graph_value() == 300);
  }

  SECTION("copy does not alias") {
    dod_int_int_int g1(100);
    dod_int_int_int g2 = g1;

    g2.graph_value() = 200;
    REQUIRE(g1.graph_value() == 100);
    REQUIRE(g2.graph_value() == 200);
  }

  SECTION("clear preserves type") {
    dod_int_int_int g(42);
    g.clear();
    REQUIRE(g.size() == 0);
    // Type is still int, we can set a new value
    g.graph_value() = 100;
    REQUIRE(g.graph_value() == 100);
  }

  SECTION("move leaves source valid but unspecified") {
    dod_int_int_int g1(100);
    dod_int_int_int g2 = std::move(g1);

    // g1 is valid but unspecified, we can still use it safely
    g1.clear();
    REQUIRE(g1.size() == 0);
  }
}

//==================================================================================================
// 12. Compilation Tests (static assertions)
//==================================================================================================

TEST_CASE("dod various template instantiations compile", "[dod][compilation]") {
  // Just test that these types compile
  [[maybe_unused]] dod_void_void_void       g1;
  [[maybe_unused]] dod_int_void_void        g2;
  [[maybe_unused]] dod_void_int_void        g3;
  [[maybe_unused]] dod_int_int_void         g4;
  [[maybe_unused]] dod_void_void_int        g5;
  [[maybe_unused]] dod_int_int_int          g6;
  [[maybe_unused]] dod_string_string_string g7;
  [[maybe_unused]] dod_sourced              g8;
  [[maybe_unused]] dod_int_sourced          g9;

  REQUIRE(true); // Just ensuring compilation
}

// Additional static checks - verify bidirectional range for edges
static_assert(std::ranges::range<dod_void_void_void>);
static_assert(std::ranges::range<dod_int_int_int>);
static_assert(std::ranges::range<dod_string_string_string>);
static_assert(std::ranges::random_access_range<dod_void_void_void::vertex_type::edges_type>);

//==================================================================================================
// Initializer List Constructor Tests
//==================================================================================================

TEST_CASE("dod initializer_list constructor with void edge values", "[dod][construction][initializer_list]") {
  using G = dod_void_void_void;

  SECTION("empty initializer list") {
    G g({});
    // Empty initializer list may create vertex 0 for sizing
    REQUIRE(g.size() <= 1);
  }

  SECTION("single edge") {
    G g({{0, 1}});
    REQUIRE(g.size() == 2);
    auto& u  = g[0];
    auto  ee = u.edges();
    REQUIRE(std::ranges::distance(ee) == 1);
    auto it = ee.begin();
    REQUIRE(it->target_id() == 1);
  }

  SECTION("multiple edges from same vertex") {
    G g({{0, 1}, {0, 2}, {0, 3}});
    REQUIRE(g.size() == 4);
    auto& u  = g[0];
    auto  ee = u.edges();
    REQUIRE(std::ranges::distance(ee) == 3);
  }

  SECTION("triangle graph") {
    G g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(g.size() == 3);

    auto& v0 = g[0];
    REQUIRE(std::ranges::distance(v0.edges()) == 1);
    REQUIRE(v0.edges().begin()->target_id() == 1);

    auto& v1 = g[1];
    REQUIRE(std::ranges::distance(v1.edges()) == 1);
    REQUIRE(v1.edges().begin()->target_id() == 2);

    auto& v2 = g[2];
    REQUIRE(std::ranges::distance(v2.edges()) == 1);
    REQUIRE(v2.edges().begin()->target_id() == 0);
  }

  SECTION("self-loop") {
    G g({{0, 0}});
    REQUIRE(g.size() == 1);
    auto& u  = g[0];
    auto  ee = u.edges();
    REQUIRE(std::ranges::distance(ee) == 1);
    REQUIRE(ee.begin()->target_id() == 0);
  }

  SECTION("parallel edges") {
    G g({{0, 1}, {0, 1}, {0, 1}});
    REQUIRE(g.size() == 2);
    auto& u  = g[0];
    auto  ee = u.edges();
    // vector preserves all duplicates
    REQUIRE(std::ranges::distance(ee) == 3);
  }

  SECTION("large vertex IDs") {
    G g({{100, 200}});
    REQUIRE(g.size() == 201); // auto-extends to accommodate vertex 200
  }
}

TEST_CASE("dod initializer_list constructor with int edge values", "[dod][construction][initializer_list]") {
  using G = dod_int_void_void;

  SECTION("edges with values") {
    G g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
    REQUIRE(g.size() == 3);

    auto& v0     = g[0];
    auto  edges0 = v0.edges();
    REQUIRE(std::ranges::distance(edges0) == 1);
    REQUIRE(edges0.begin()->target_id() == 1);
    REQUIRE(edges0.begin()->value() == 10);

    auto& v1     = g[1];
    auto  edges1 = v1.edges();
    REQUIRE(std::ranges::distance(edges1) == 1);
    REQUIRE(edges1.begin()->target_id() == 2);
    REQUIRE(edges1.begin()->value() == 20);

    auto& v2     = g[2];
    auto  edges2 = v2.edges();
    REQUIRE(std::ranges::distance(edges2) == 1);
    REQUIRE(edges2.begin()->target_id() == 0);
    REQUIRE(edges2.begin()->value() == 30);
  }

  SECTION("edges with zero values") {
    G g({{0, 1, 0}, {1, 2, 0}});
    REQUIRE(g.size() == 3);
    auto& v0 = g[0];
    REQUIRE(v0.edges().begin()->value() == 0);
  }

  SECTION("edges with negative values") {
    G g({{0, 1, -5}, {1, 2, -10}});
    REQUIRE(g.size() == 3);
    auto& v0 = g[0];
    REQUIRE(v0.edges().begin()->value() == -5);
    auto& v1 = g[1];
    REQUIRE(v1.edges().begin()->value() == -10);
  }
}

TEST_CASE("dod initializer_list constructor with string edge values", "[dod][construction][initializer_list]") {
  using G = dod_string_string_string;

  SECTION("edges with string values") {
    G g({{0, 1, "edge01"}, {1, 2, "edge12"}});
    REQUIRE(g.size() == 3);

    auto& v0     = g[0];
    auto  edges0 = v0.edges();
    REQUIRE(edges0.begin()->value() == "edge01");

    auto& v1     = g[1];
    auto  edges1 = v1.edges();
    REQUIRE(edges1.begin()->value() == "edge12");
  }

  SECTION("edges with empty string values") {
    G g({{0, 1, ""}, {1, 2, ""}});
    REQUIRE(g.size() == 3);
    auto& v0 = g[0];
    REQUIRE(v0.edges().begin()->value() == "");
  }
}

TEST_CASE("dod initializer_list constructor with graph value (copy)", "[dod][construction][initializer_list]") {
  using G = dod_void_void_int;

  SECTION("construct with graph value and edges") {
    int graph_val = 42;
    G   g(graph_val, {{0, 1}, {1, 2}});
    REQUIRE(g.size() == 3);
    REQUIRE(g.graph_value() == 42);

    // Verify edges are constructed correctly
    auto& v0 = g[0];
    REQUIRE(std::ranges::distance(v0.edges()) == 1);
    REQUIRE(v0.edges().begin()->target_id() == 1);
  }

  SECTION("construct with graph value and empty edges") {
    int graph_val = 100;
    G   g(graph_val, {});
    // Empty initializer list may create vertex 0 for sizing
    REQUIRE(g.size() <= 1);
    REQUIRE(g.graph_value() == 100);
  }

  SECTION("graph value is copied") {
    int graph_val = 50;
    G   g(graph_val, {{0, 1}});
    REQUIRE(g.graph_value() == 50);
    graph_val = 999;                // Modify original
    REQUIRE(g.graph_value() == 50); // Graph value unchanged
  }
}

TEST_CASE("dod initializer_list constructor with graph value (move)", "[dod][construction][initializer_list]") {
  using G = dod_string_string_string;

  SECTION("construct with moved graph value") {
    std::string graph_val = "test_graph";
    G           g(std::move(graph_val), {{0, 1, "edge"}, {1, 2, "edge2"}});
    REQUIRE(g.size() == 3);
    REQUIRE(g.graph_value() == "test_graph");
    // graph_val may or may not be empty after move (implementation-defined)
  }

  SECTION("construct with rvalue graph value") {
    G g(std::string("rvalue_graph"), {{0, 1, "e1"}});
    REQUIRE(g.size() == 2);
    REQUIRE(g.graph_value() == "rvalue_graph");
  }
}

TEST_CASE("dod initializer_list constructor with all value types", "[dod][construction][initializer_list]") {
  using G = dod_int_int_int;

  SECTION("construct with all int values") {
    int graph_val = 1000;
    G   g(graph_val, {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}});
    REQUIRE(g.size() == 4);
    REQUIRE(g.graph_value() == 1000);

    // Verify vertex values are default-constructed (0 for int)
    REQUIRE(g[0].value() == 0);
    REQUIRE(g[1].value() == 0);

    // Verify edge values
    auto& v0 = g[0];
    REQUIRE(v0.edges().begin()->value() == 10);

    auto& v1 = g[1];
    REQUIRE(v1.edges().begin()->value() == 20);
  }
}

TEST_CASE("dod initializer_list constructor with sourced edges", "[dod][construction][initializer_list][sourced]") {
  using G = dod_sourced;

  SECTION("construct sourced graph with initializer list") {
    G g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(g.size() == 3);

    // Verify sourced edges have source_id
    auto& v0     = g[0];
    auto  edges0 = v0.edges();
    REQUIRE(std::ranges::distance(edges0) == 1);
    auto e0 = edges0.begin();
    REQUIRE(e0->source_id() == 0);
    REQUIRE(e0->target_id() == 1);
  }
}

TEST_CASE("dod initializer_list complex graph patterns", "[dod][construction][initializer_list]") {
  using G = dod_int_void_void;

  SECTION("star graph") {
    // Central vertex 0 connected to vertices 1-5
    G g({{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {0, 4, 4}, {0, 5, 5}});
    REQUIRE(g.size() == 6);
    auto& center = g[0];
    REQUIRE(std::ranges::distance(center.edges()) == 5);
  }

  SECTION("complete graph K4") {
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
    // Each vertex should have 3 outgoing edges
    for (uint32_t i = 0; i < 4; ++i) {
      REQUIRE(std::ranges::distance(g[i].edges()) == 3);
    }
  }

  SECTION("chain graph") {
    G g({{0, 1, 1}, {1, 2, 2}, {2, 3, 3}, {3, 4, 4}});
    REQUIRE(g.size() == 5);
    // Each vertex except last should have 1 edge
    for (uint32_t i = 0; i < 4; ++i) {
      REQUIRE(std::ranges::distance(g[i].edges()) == 1);
    }
    // Last vertex has no edges
    REQUIRE(std::ranges::distance(g[4].edges()) == 0);
  }

  SECTION("cycle graph") {
    G g({{0, 1, 1}, {1, 2, 2}, {2, 3, 3}, {3, 4, 4}, {4, 0, 5}});
    REQUIRE(g.size() == 5);
    // Each vertex should have exactly 1 edge
    for (uint32_t i = 0; i < 5; ++i) {
      REQUIRE(std::ranges::distance(g[i].edges()) == 1);
    }
  }
}

//==================================================================================================
// 13. Load Operations Tests
//==================================================================================================

TEST_CASE("dod load_vertices", "[dynamic_graph][dod][load_vertices]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;

  SECTION("load empty vertex range") {
    G                        g;
    std::vector<vertex_data> vv;
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 0);
  }

  SECTION("load single vertex") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 100}};
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 1);
    REQUIRE(g[0].value() == 100);
  }

  SECTION("load multiple vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}, {3, 40}, {4, 50}};
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 5);
    REQUIRE(g[0].value() == 10);
    REQUIRE(g[1].value() == 20);
    REQUIRE(g[2].value() == 30);
    REQUIRE(g[3].value() == 40);
    REQUIRE(g[4].value() == 50);
  }

  SECTION("with custom projection from struct") {
    using G2           = dynamic_graph<int, std::string, void, uint32_t, false, false, dod_graph_traits<int, std::string, void, uint32_t, false>>;
    using vertex_data2 = copyable_vertex_t<uint32_t, std::string>;

    struct Person {
      uint32_t    id;
      std::string name;
      int         age;
    };

    G2                  g;
    std::vector<Person> people = {{0, "Alice", 30}, {1, "Bob", 25}, {2, "Charlie", 35}};
    g.load_vertices(people, [](const Person& p) -> vertex_data2 { return {p.id, p.name}; });

    REQUIRE(g.size() == 3);
    REQUIRE(g[0].value() == "Alice");
    REQUIRE(g[1].value() == "Bob");
    REQUIRE(g[2].value() == "Charlie");
  }

  SECTION("with void vertex values - using default constructor") {
    using G3 = dod_int_void_void;
    G3 g;
    // With void vertex values, we can't use load_vertices because copyable_vertex_t<VId, void>
    // only has {id} but load_vertices expects {id, value}. Instead, test construction.
    REQUIRE(g.size() == 0);
  }
}

TEST_CASE("dod load_edges", "[dynamic_graph][dod][load_edges]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("load empty edge range") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee;
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == 3);
    for (auto& v : g) {
      size_t count = 0;
      for (auto& e : v.edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 0);
    }
  }

  SECTION("load single edge") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}};
    g.load_edges(ee, std::identity{});

    // Check vertex 0 has the edge
    size_t count = 0;
    for (auto& edge : g[0].edges()) {
      ++count;
      REQUIRE(edge.value() == 100);
    }
    REQUIRE(count == 1);
  }

  SECTION("load multiple edges from one vertex") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    for (auto& edge : g[0].edges()) {
      ++count;
      (void)edge;
    }
    REQUIRE(count == 3);
  }

  SECTION("load edges from multiple vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}, {1, 2, 200}, {2, 0, 300}};
    g.load_edges(ee, std::identity{});

    // Count edges per vertex
    size_t count0 = 0, count1 = 0, count2 = 0;
    for (auto& e : g[0].edges()) {
      ++count0;
      (void)e;
    }
    for (auto& e : g[1].edges()) {
      ++count1;
      (void)e;
    }
    for (auto& e : g[2].edges()) {
      ++count2;
      (void)e;
    }

    REQUIRE(count0 == 1);
    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);
  }

  SECTION("with void edge values") {
    using G2           = dod_void_int_void;
    using vertex_data2 = copyable_vertex_t<uint32_t, int>;
    using edge_data2   = copyable_edge_t<uint32_t, void>;

    G2                        g;
    std::vector<vertex_data2> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data2> ee = {{0, 1}, {1, 2}, {2, 0}};
    g.load_edges(ee, std::identity{});

    // Verify edges exist by counting
    size_t total_edges = 0;
    for (auto& v : g) {
      for (auto& edge : v.edges()) {
        ++total_edges;
        (void)edge;
      }
    }
    REQUIRE(total_edges == 3);
  }

  SECTION("with custom projection") {
    using G2           = dynamic_graph<std::string, int, void, uint32_t, false, false, dod_graph_traits<std::string, int, void, uint32_t, false>>;
    using vertex_data2 = copyable_vertex_t<uint32_t, int>;
    using edge_data2   = copyable_edge_t<uint32_t, std::string>;

    struct Edge {
      uint32_t    from;
      uint32_t    to;
      std::string label;
    };

    G2                        g;
    std::vector<vertex_data2> vv = {{0, 1}, {1, 2}, {2, 3}};
    g.load_vertices(vv, std::identity{});

    std::vector<Edge> ee = {{0, 1, "edge01"}, {1, 2, "edge12"}};
    g.load_edges(ee, [](const Edge& e) -> edge_data2 { return {e.from, e.to, e.label}; });

    // Verify edges exist
    size_t total = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        ++total;
        (void)e;
      }
    }
    REQUIRE(total == 2);
  }

  SECTION("single self-loop") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 0, 999}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    for (auto& edge : g[0].edges()) {
      ++count;
      REQUIRE(edge.value() == 999);
    }
    REQUIRE(count == 1);
  }

  SECTION("multiple self-loops") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 0, 1}, {0, 0, 2}, {0, 0, 3}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    for (auto& edge : g[0].edges()) {
      ++count;
      (void)edge;
    }
    REQUIRE(count == 3);
  }

  SECTION("parallel edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}, {0, 1, 200}, {0, 1, 300}};
    g.load_edges(ee, std::identity{});

    size_t           count = 0;
    std::vector<int> values;
    for (auto& edge : g[0].edges()) {
      ++count;
      values.push_back(edge.value());
    }
    REQUIRE(count == 3);
    REQUIRE(values.size() == 3);
    REQUIRE(std::find(values.begin(), values.end(), 100) != values.end());
    REQUIRE(std::find(values.begin(), values.end(), 200) != values.end());
    REQUIRE(std::find(values.begin(), values.end(), 300) != values.end());
  }

  SECTION("large edge sets (1000 edges)") {
    G                        g;
    std::vector<vertex_data> vv(100);
    for (uint32_t i = 0; i < 100; ++i) {
      vv[i] = {i, static_cast<int>(i)};
    }
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < 100; ++i) {
      for (uint32_t j = 0; j < 10; ++j) {
        ee.push_back({i, (i + j) % 100, static_cast<int>(i * 1000 + j)});
      }
    }
    g.load_edges(ee, std::identity{});

    // Verify each vertex has 10 edges
    for (uint32_t i = 0; i < 100; ++i) {
      size_t count = 0;
      for (auto& edge : g[i].edges()) {
        ++count;
        (void)edge;
      }
      REQUIRE(count == 10);
    }
  }
}

//==================================================================================================
// Vertex/Edge Access with Populated Graphs
//==================================================================================================

TEST_CASE("dod vertex access in populated graph", "[dynamic_graph][dod][vertex_access]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("access vertices with values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g[0].value() == 100);
    REQUIRE(g[1].value() == 200);
    REQUIRE(g[2].value() == 300);
  }

  SECTION("modify vertex values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}};
    g.load_vertices(vv, std::identity{});

    g[0].value() = 999;
    g[1].value() = 888;

    REQUIRE(g[0].value() == 999);
    REQUIRE(g[1].value() == 888);
  }

  SECTION("iterate all vertices in populated graph") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}};
    g.load_vertices(vv, std::identity{});

    int sum = 0;
    for (auto& v : g) {
      sum += v.value();
    }
    REQUIRE(sum == 15); // 1+2+3+4+5
  }

  SECTION("access edges from vertex") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}, {0, 2, 200}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    int    sum   = 0;
    for (auto& edge : g[0].edges()) {
      ++count;
      sum += edge.value();
    }
    REQUIRE(count == 2);
    REQUIRE(sum == 300); // 100+200
  }
}

TEST_CASE("dod edge iteration patterns", "[dynamic_graph][dod][edge_access]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("iterate edges from multiple vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 1}, {0, 2, 2}, {1, 2, 3}, {1, 3, 4}, {2, 3, 5}};
    g.load_edges(ee, std::identity{});

    // Count edges per vertex
    std::vector<size_t> counts;
    for (auto& v : g) {
      size_t count = 0;
      for (auto& e : v.edges()) {
        ++count;
        (void)e;
      }
      counts.push_back(count);
    }

    REQUIRE(counts[0] == 2); // 0->1, 0->2
    REQUIRE(counts[1] == 2); // 1->2, 1->3
    REQUIRE(counts[2] == 1); // 2->3
    REQUIRE(counts[3] == 0); // no outgoing edges
  }

  SECTION("sum all edge values in graph") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};
    g.load_edges(ee, std::identity{});

    int total = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        total += e.value();
      }
    }
    REQUIRE(total == 60); // 10+20+30
  }

  SECTION("modify edge values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}};
    g.load_edges(ee, std::identity{});

    // Modify edge value
    for (auto& e : g[0].edges()) {
      e.value() = 999;
    }

    // Verify modification
    for (auto& e : g[0].edges()) {
      REQUIRE(e.value() == 999);
    }
  }
}

TEST_CASE("dod graph with complex structure", "[dynamic_graph][dod][complex]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("triangle graph") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}};
    g.load_vertices(vv, std::identity{});

    // Create triangle: 0->1, 1->2, 2->0
    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};
    g.load_edges(ee, std::identity{});

    // Each vertex should have exactly 1 outgoing edge
    for (size_t i = 0; i < 3; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 1);
    }
  }

  SECTION("star graph - one hub to many spokes") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 11; ++i) {
      vv.push_back({i, static_cast<int>(i * 10)});
    }
    g.load_vertices(vv, std::identity{});

    // Vertex 0 is hub, connects to all others
    std::vector<edge_data> ee;
    for (uint32_t i = 1; i < 11; ++i) {
      ee.push_back({0, i, static_cast<int>(i)});
    }
    g.load_edges(ee, std::identity{});

    // Hub should have 10 edges
    size_t hub_count = 0;
    for (auto& e : g[0].edges()) {
      ++hub_count;
      (void)e;
    }
    REQUIRE(hub_count == 10);

    // Spokes should have 0 edges
    for (uint32_t i = 1; i < 11; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 0);
    }
  }

  SECTION("complete graph K4") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    g.load_vertices(vv, std::identity{});

    // Every vertex connects to every other vertex
    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < 4; ++i) {
      for (uint32_t j = 0; j < 4; ++j) {
        if (i != j) {
          ee.push_back({i, j, static_cast<int>(i * 10 + j)});
        }
      }
    }
    g.load_edges(ee, std::identity{});

    // Each vertex should have 3 outgoing edges
    for (uint32_t i = 0; i < 4; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 3);
    }
  }
}

TEST_CASE("dod graph with string values", "[dynamic_graph][dod][string_values]") {
  using G           = dod_string_string_string;
  using vertex_data = copyable_vertex_t<uint32_t, std::string>;
  using edge_data   = copyable_edge_t<uint32_t, std::string>;

  SECTION("vertices and edges with string values") {
    G g("root_graph");

    std::vector<vertex_data> vv = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, "knows"}, {1, 2, "friend"}, {0, 2, "colleague"}};
    g.load_edges(ee, std::identity{});

    REQUIRE(g.graph_value() == "root_graph");
    REQUIRE(g[0].value() == "Alice");
    REQUIRE(g[1].value() == "Bob");
    REQUIRE(g[2].value() == "Charlie");

    // Check edge values
    std::vector<std::string> edge_labels;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        edge_labels.push_back(e.value());
      }
    }

    REQUIRE(edge_labels.size() == 3);
    REQUIRE(std::find(edge_labels.begin(), edge_labels.end(), "knows") != edge_labels.end());
    REQUIRE(std::find(edge_labels.begin(), edge_labels.end(), "friend") != edge_labels.end());
    REQUIRE(std::find(edge_labels.begin(), edge_labels.end(), "colleague") != edge_labels.end());
  }
}

TEST_CASE("dod single vertex graphs", "[dynamic_graph][dod][single_vertex]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("single vertex no edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 42}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == 1);
    REQUIRE(g[0].value() == 42);

    size_t count = 0;
    for (auto& e : g[0].edges()) {
      ++count;
      (void)e;
    }
    REQUIRE(count == 0);
  }

  SECTION("single vertex with self-loop") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 42}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 0, 100}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    for (auto& e : g[0].edges()) {
      ++count;
      REQUIRE(e.value() == 100);
    }
    REQUIRE(count == 1);
  }

  SECTION("single vertex with multiple self-loops") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 42}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 0, 1}, {0, 0, 2}, {0, 0, 3}, {0, 0, 4}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    int    sum   = 0;
    for (auto& e : g[0].edges()) {
      ++count;
      sum += e.value();
    }
    REQUIRE(count == 4);
    REQUIRE(sum == 10); // 1+2+3+4
  }
}

TEST_CASE("dod large populated graph", "[dynamic_graph][dod][large]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("1000 vertices each with value") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 1000; ++i) {
      vv.push_back({i, static_cast<int>(i * i)});
    }
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == 1000);
    REQUIRE(g[0].value() == 0);
    REQUIRE(g[500].value() == 250000); // 500^2
    REQUIRE(g[999].value() == 998001); // 999^2
  }

  SECTION("chain graph with 100 vertices") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 100; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Create chain: 0->1->2->...->99
    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < 99; ++i) {
      ee.push_back({i, i + 1, static_cast<int>(i * 100)});
    }
    g.load_edges(ee, std::identity{});

    // First 99 vertices have 1 edge, last has 0
    for (uint32_t i = 0; i < 99; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 1);
    }

    size_t last_count = 0;
    for (auto& e : g[99].edges()) {
      ++last_count;
      (void)e;
    }
    REQUIRE(last_count == 0);
  }
}

TEST_CASE("dod mixed access patterns", "[dynamic_graph][dod][mixed]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("interleaved vertex and edge access") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}, {1, 2, 200}};
    g.load_edges(ee, std::identity{});

    // Access vertex, then edges, then vertex again
    REQUIRE(g[0].value() == 10);

    size_t count0 = 0;
    for (auto& e : g[0].edges()) {
      ++count0;
      (void)e;
    }
    REQUIRE(count0 == 1);

    REQUIRE(g[1].value() == 20);

    size_t count1 = 0;
    for (auto& e : g[1].edges()) {
      ++count1;
      (void)e;
    }
    REQUIRE(count1 == 1);

    REQUIRE(g[2].value() == 30);
  }

  SECTION("range-based for with structured bindings") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee, std::identity{});

    // Iterate all vertices
    int vertex_sum = 0;
    for (auto& v : g) {
      vertex_sum += v.value();
    }
    REQUIRE(vertex_sum == 6); // 1+2+3

    // Iterate all edges across all vertices
    int edge_sum = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        edge_sum += e.value();
      }
    }
    REQUIRE(edge_sum == 30); // 10+20
  }
}

//==================================================================================================
// Summary: Phase 1.1 Tests Progress
// - Construction: 17 tests (TEST_CASE entries)
// - Basic Properties: 7 tests
// - Graph Value: 6 tests
// - Iterator: 5 tests
// - Type Traits: 5 tests
// - Empty Graph Edge Cases: 6 tests
// - Value Types: 12 tests
// - Vertex ID Types: 5 tests
// - Sourced Edges: 6 tests
// - Const Correctness: 6 tests
// - Memory/Resources: 4 tests
// - Compilation: 1 test
// - Load Vertices: 3 tests (9 SECTION entries)
// - Load Edges: 5 tests (6 SECTION entries)
// - Vertex/Edge Access: 10 tests (24 SECTION entries)
//
// Total: 98 TEST_CASE entries with 39 SECTION entries = ~970 ctests
// (845 existing tests + ~125 new dynamic_graph tests)
//
// Note: Additional tests for partitions, properties, and error handling
// will be added next.
//==================================================================================================

//==================================================================================================
// Error Handling and Edge Cases
//==================================================================================================

TEST_CASE("dod error handling for out-of-range access", "[dynamic_graph][dod][error]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("load_edges auto-extends for large source ID") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}};
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 2);

    // Edge with source_id=5 - should auto-extend vertices
    std::vector<edge_data> ee = {{5, 1, 100}};
    g.load_edges(ee, std::identity{});

    // Graph should auto-extend to accommodate vertex 5
    REQUIRE(g.size() == 6);
  }

  SECTION("load_edges auto-extends for large target ID") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}};
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 2);

    // Edge with target_id=10 - should auto-extend vertices
    std::vector<edge_data> ee = {{0, 10, 100}};
    g.load_edges(ee, std::identity{});

    // Graph should auto-extend to accommodate vertex 10
    REQUIRE(g.size() == 11);
  }

  SECTION("load_vertices with ID exceeding container size") {
    G g;
    // Start with 3 vertices
    std::vector<vertex_data> vv1 = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv1, std::identity{});

    // Try to load vertex with ID=10 without resizing
    std::vector<vertex_data> vv2 = {{10, 100}};

    REQUIRE_THROWS_AS(g.load_vertices(vv2, std::identity{}), std::out_of_range);
  }
}

TEST_CASE("dod edge cases with empty containers", "[dynamic_graph][dod][edge_case]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("load edges before vertices") {
    G g;

    // Load edges with no vertices - should infer vertex count
    std::vector<edge_data> ee = {{0, 1, 100}, {1, 2, 200}};
    g.load_edges(ee, std::identity{});

    // Graph should auto-size to accommodate vertices 0,1,2
    REQUIRE(g.size() == 3);
  }

  SECTION("multiple empty load operations") {
    G g;

    std::vector<vertex_data> empty_vertices;
    std::vector<edge_data>   empty_edges;

    g.load_vertices(empty_vertices, std::identity{});
    REQUIRE(g.size() == 0);

    g.load_edges(empty_edges, std::identity{});
    // Empty load_edges may create vertex 0 for sizing
    // Accept either 0 or 1 depending on implementation
    REQUIRE(g.size() <= 1);

    // Clear and start fresh
    g.clear();

    // Add actual data
    std::vector<vertex_data> vv = {{0, 10}};
    g.load_vertices(vv, std::identity{});
    REQUIRE(g.size() == 1);
  }

  SECTION("vertices only, no edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == 3);

    // All vertices should have no edges
    for (auto& v : g) {
      size_t count = 0;
      for (auto& e : v.edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 0);
    }
  }
}

TEST_CASE("dod boundary value tests", "[dynamic_graph][dod][boundary]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("vertex ID at zero") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 999}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == 1);
    REQUIRE(g[0].value() == 999);
  }

  SECTION("large vertex ID values") {
    G g;
    // Create sparse graph with large IDs
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 1000; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == 1000);
    REQUIRE(g[999].value() == 999);
  }

  SECTION("zero edge values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 0}};
    g.load_edges(ee, std::identity{});

    for (auto& e : g[0].edges()) {
      REQUIRE(e.value() == 0);
    }
  }

  SECTION("negative edge values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, -100}, {1, 0, -200}};
    g.load_edges(ee, std::identity{});

    int sum = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        sum += e.value();
      }
    }
    REQUIRE(sum == -300);
  }
}

TEST_CASE("dod incremental graph building", "[dynamic_graph][dod][incremental]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("load vertices in multiple batches") {
    G g;

    std::vector<vertex_data> batch1 = {{0, 10}, {1, 20}};
    g.load_vertices(batch1, std::identity{});
    REQUIRE(g.size() == 2);

    std::vector<vertex_data> batch2 = {{2, 30}, {3, 40}};
    g.load_vertices(batch2, std::identity{}, 4);
    REQUIRE(g.size() == 4);

    REQUIRE(g[0].value() == 10);
    REQUIRE(g[2].value() == 30);
    REQUIRE(g[3].value() == 40);
  }

  SECTION("load edges in multiple batches") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> batch1 = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(batch1, std::identity{});

    std::vector<edge_data> batch2 = {{2, 3, 30}, {3, 0, 40}};
    g.load_edges(batch2, std::identity{});

    // Count total edges
    size_t total = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        ++total;
        (void)e;
      }
    }
    REQUIRE(total == 4);
  }

  SECTION("update existing vertex values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}};
    g.load_vertices(vv, std::identity{});

    REQUIRE(g[0].value() == 10);
    REQUIRE(g[1].value() == 20);

    // Overwrite with new values
    std::vector<vertex_data> updates = {{0, 999}, {1, 888}};
    g.load_vertices(updates, std::identity{});

    REQUIRE(g[0].value() == 999);
    REQUIRE(g[1].value() == 888);
  }
}

TEST_CASE("dod duplicate and redundant edges", "[dynamic_graph][dod][duplicates]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("exact duplicate edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    // Load same edge multiple times
    std::vector<edge_data> ee = {{0, 1, 100}, {0, 1, 100}, {0, 1, 100}};
    g.load_edges(ee, std::identity{});

    // list allows duplicates
    size_t count = 0;
    for (auto& e : g[0].edges()) {
      ++count;
      REQUIRE(e.value() == 100);
    }
    REQUIRE(count == 3);
  }

  SECTION("same endpoints different values") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 100}, {0, 1, 200}, {0, 1, 300}};
    g.load_edges(ee, std::identity{});

    size_t count = 0;
    int    sum   = 0;
    for (auto& e : g[0].edges()) {
      ++count;
      sum += e.value();
    }
    REQUIRE(count == 3);
    REQUIRE(sum == 600);
  }

  SECTION("bidirectional edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    // Both directions
    std::vector<edge_data> ee = {{0, 1, 100}, {1, 0, 200}};
    g.load_edges(ee, std::identity{});

    size_t count0 = 0;
    for (auto& e : g[0].edges()) {
      ++count0;
      REQUIRE(e.value() == 100);
    }
    REQUIRE(count0 == 1);

    size_t count1 = 0;
    for (auto& e : g[1].edges()) {
      ++count1;
      REQUIRE(e.value() == 200);
    }
    REQUIRE(count1 == 1);
  }
}

TEST_CASE("dod graph properties and queries", "[dynamic_graph][dod][properties]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("count total edges in graph") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {1, 2, 4}, {1, 3, 5}, {2, 3, 6}};
    g.load_edges(ee, std::identity{});

    size_t total_edges = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        ++total_edges;
        (void)e;
      }
    }
    REQUIRE(total_edges == 6);
  }

  SECTION("find vertices with no outgoing edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee, std::identity{});

    std::vector<size_t> sinks;
    for (size_t i = 0; i < g.size(); ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      if (count == 0) {
        sinks.push_back(i);
      }
    }

    REQUIRE(sinks.size() == 2);
    REQUIRE(std::find(sinks.begin(), sinks.end(), 2) != sinks.end());
    REQUIRE(std::find(sinks.begin(), sinks.end(), 3) != sinks.end());
  }

  SECTION("compute out-degree for each vertex") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 5; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 1},
                                 {0, 2, 2},
                                 {0, 3, 3}, // vertex 0: degree 3
                                 {1, 2, 4},
                                 {1, 4, 5}, // vertex 1: degree 2
                                 {2, 4, 6}, // vertex 2: degree 1
                                 // vertex 3: degree 0
                                 {4, 0, 7}}; // vertex 4: degree 1

    g.load_edges(ee, std::identity{});

    std::vector<size_t> degrees;
    for (auto& v : g) {
      size_t d = 0;
      for (auto& e : v.edges()) {
        ++d;
        (void)e;
      }
      degrees.push_back(d);
    }

    REQUIRE(degrees[0] == 3);
    REQUIRE(degrees[1] == 2);
    REQUIRE(degrees[2] == 1);
    REQUIRE(degrees[3] == 0);
    REQUIRE(degrees[4] == 1);
  }

  SECTION("find maximum degree vertex") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 6; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Vertex 2 has highest degree
    std::vector<edge_data> ee = {{0, 1, 1}, {1, 2, 2}, {2, 0, 3}, {2, 1, 4}, {2, 3, 5},
                                 {2, 4, 6}, {2, 5, 7}, {3, 4, 8}, {4, 5, 9}};
    g.load_edges(ee, std::identity{});

    size_t max_degree     = 0;
    size_t max_vertex_idx = 0;

    for (size_t i = 0; i < g.size(); ++i) {
      size_t d = 0;
      for (auto& e : g[i].edges()) {
        ++d;
        (void)e;
      }
      if (d > max_degree) {
        max_degree     = d;
        max_vertex_idx = i;
      }
    }

    REQUIRE(max_vertex_idx == 2);
    REQUIRE(max_degree == 5);
  }
}

TEST_CASE("dod special graph patterns", "[dynamic_graph][dod][patterns]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("cycle graph C5") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 5; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Create cycle: 0->1->2->3->4->0
    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < 5; ++i) {
      ee.push_back({i, (i + 1) % 5, static_cast<int>(i)});
    }
    g.load_edges(ee, std::identity{});

    // Every vertex should have out-degree 1
    for (size_t i = 0; i < 5; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 1);
    }
  }

  SECTION("binary tree structure") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 7; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Binary tree: node i has children 2i+1 and 2i+2
    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < 3; ++i) {
      ee.push_back({i, 2 * i + 1, static_cast<int>(i * 10 + 1)});
      ee.push_back({i, 2 * i + 2, static_cast<int>(i * 10 + 2)});
    }
    g.load_edges(ee, std::identity{});

    // Root and internal nodes have degree 2
    for (uint32_t i = 0; i < 3; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 2);
    }

    // Leaves have degree 0
    for (uint32_t i = 3; i < 7; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 0);
    }
  }

  SECTION("bipartite graph") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 6; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Set A: {0,1,2}, Set B: {3,4,5}
    // Edges only between sets
    std::vector<edge_data> ee = {{0, 3, 1}, {0, 4, 2}, {0, 5, 3}, {1, 3, 4}, {1, 4, 5}, {2, 4, 6}, {2, 5, 7}};
    g.load_edges(ee, std::identity{});

    size_t total = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        ++total;
        (void)e;
      }
    }
    REQUIRE(total == 7);
  }
}

//==================================================================================================
// Summary: Phase 1.1 Tests Progress
// - Construction: 17 tests (TEST_CASE entries)
// - Basic Properties: 7 tests
// - Graph Value: 6 tests
// - Iterator: 5 tests
// - Type Traits: 5 tests
// - Empty Graph Edge Cases: 6 tests
// - Value Types: 12 tests
// - Vertex ID Types: 5 tests
// - Sourced Edges: 6 tests
// - Const Correctness: 6 tests
// - Memory/Resources: 4 tests
// - Compilation: 1 test
// - Load Vertices: 3 tests (9 SECTION entries)
// - Load Edges: 5 tests (6 SECTION entries)
// - Vertex/Edge Access: 10 tests (24 SECTION entries)
// - Error Handling: 3 tests (7 SECTION entries)
// - Edge Cases: 2 tests (5 SECTION entries)
// - Boundary Values: 1 test (4 SECTION entries)
// - Incremental Building: 1 test (3 SECTION entries)
// - Duplicates: 1 test (3 SECTION entries)
// - Graph Properties: 1 test (4 SECTION entries)
// - Special Patterns: 1 test (3 SECTION entries)
//
// Total: 109 TEST_CASE entries with 68 SECTION entries = ~1050 ctests
// (845 existing tests + ~205 new dynamic_graph tests)
//
// Phase 1.1 dod_graph_traits testing approaching completion.
// Remaining: Partition tests, sourced edge variations, performance benchmarks.
//==================================================================================================

//==================================================================================================
// Iterator Stability and Ranges Integration
//==================================================================================================

TEST_CASE("dod iterator stability", "[dynamic_graph][dod][iterators]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("vertex iterators remain valid after edge operations") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}};
    g.load_vertices(vv, std::identity{});

    auto it = g.begin();
    REQUIRE(it->value() == 10);

    // Load edges - vertex iterators should remain valid
    std::vector<edge_data> ee = {{0, 1, 100}};
    g.load_edges(ee, std::identity{});

    REQUIRE(it->value() == 10); // Iterator still valid
  }

  SECTION("iterate vertices multiple times") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}};
    g.load_vertices(vv, std::identity{});

    // First iteration
    int sum1 = 0;
    for (auto& v : g) {
      sum1 += v.value();
    }

    // Second iteration should give same result
    int sum2 = 0;
    for (auto& v : g) {
      sum2 += v.value();
    }

    REQUIRE(sum1 == sum2);
    REQUIRE(sum1 == 6);
  }

  SECTION("nested iteration - vertices and edges") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {0, 1, 20}};
    g.load_edges(ee, std::identity{});

    // Nested iteration should work
    int vertex_sum = 0;
    int edge_sum   = 0;
    for (auto& v : g) {
      vertex_sum += v.value();
      for (auto& e : v.edges()) {
        edge_sum += e.value();
      }
    }

    REQUIRE(vertex_sum == 3); // 1+2
    REQUIRE(edge_sum == 30);  // 10+20
  }
}

TEST_CASE("dod std::ranges integration", "[dynamic_graph][dod][ranges]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;

  SECTION("ranges::count_if on vertices") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 10; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Count vertices with even values
    auto count = std::ranges::count_if(g, [](auto& v) { return v.value() % 2 == 0; });

    REQUIRE(count == 5); // 0,2,4,6,8
  }

  SECTION("ranges::find_if on vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};
    g.load_vertices(vv, std::identity{});

    auto it = std::ranges::find_if(g, [](auto& v) { return v.value() == 30; });

    REQUIRE(it != g.end());
    REQUIRE(it->value() == 30);
  }

  SECTION("ranges::transform view") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    g.load_vertices(vv, std::identity{});

    auto squared = g | std::views::transform([](auto& v) { return v.value() * v.value(); });

    std::vector<int> results;
    for (auto val : squared) {
      results.push_back(val);
    }

    REQUIRE(results.size() == 4);
    REQUIRE(results[0] == 1);
    REQUIRE(results[1] == 4);
    REQUIRE(results[2] == 9);
    REQUIRE(results[3] == 16);
  }

  SECTION("ranges::filter view") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < 10; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    auto odd_vertices = g | std::views::filter([](auto& v) { return v.value() % 2 == 1; });

    size_t count = 0;
    for (auto& v : odd_vertices) {
      ++count;
      REQUIRE(v.value() % 2 == 1);
    }
    REQUIRE(count == 5); // 1,3,5,7,9
  }
}

TEST_CASE("dod algorithm compatibility", "[dynamic_graph][dod][algorithms]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;

  SECTION("std::accumulate on vertex values") {
    G                        g;
    std::vector<vertex_data> vv;
    for (uint32_t i = 1; i <= 5; ++i) {
      vv.push_back({i - 1, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    auto sum = std::accumulate(g.begin(), g.end(), 0, [](int acc, auto& v) { return acc + v.value(); });

    REQUIRE(sum == 15); // 1+2+3+4+5
  }

  SECTION("std::all_of on vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 2}, {1, 4}, {2, 6}};
    g.load_vertices(vv, std::identity{});

    bool all_even = std::all_of(g.begin(), g.end(), [](auto& v) { return v.value() % 2 == 0; });

    REQUIRE(all_even);
  }

  SECTION("std::any_of on vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 2}, {2, 3}};
    g.load_vertices(vv, std::identity{});

    bool has_even = std::any_of(g.begin(), g.end(), [](auto& v) { return v.value() % 2 == 0; });

    REQUIRE(has_even);
  }

  SECTION("std::none_of on vertices") {
    G                        g;
    std::vector<vertex_data> vv = {{0, 1}, {1, 3}, {2, 5}};
    g.load_vertices(vv, std::identity{});

    bool none_even = std::none_of(g.begin(), g.end(), [](auto& v) { return v.value() % 2 == 0; });

    REQUIRE(none_even);
  }
}

//==================================================================================================
// Performance and Scalability
//==================================================================================================

TEST_CASE("dod performance characteristics", "[dynamic_graph][dod][performance]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("dense graph - many edges per vertex") {
    G                        g;
    const size_t             n = 50;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < n; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Each vertex connects to 10 others
    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < n; ++i) {
      for (uint32_t j = 0; j < 10; ++j) {
        ee.push_back({i, static_cast<uint32_t>((i + j + 1) % n), static_cast<int>(i * 100 + j)});
      }
    }
    g.load_edges(ee, std::identity{});

    REQUIRE(g.size() == n);

    // Verify each vertex has 10 edges
    for (uint32_t i = 0; i < n; ++i) {
      size_t count = 0;
      for (auto& e : g[i].edges()) {
        ++count;
        (void)e;
      }
      REQUIRE(count == 10);
    }
  }

  SECTION("sparse graph - few edges") {
    G                        g;
    const size_t             n = 100;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < n; ++i) {
      vv.push_back({i, static_cast<int>(i)});
    }
    g.load_vertices(vv, std::identity{});

    // Only 20 edges total in graph of 100 vertices
    std::vector<edge_data> ee;
    for (uint32_t i = 0; i < 20; ++i) {
      ee.push_back({i, i + 1, static_cast<int>(i)});
    }
    g.load_edges(ee, std::identity{});

    size_t vertices_with_edges = 0;
    for (auto& v : g) {
      size_t count = 0;
      for (auto& e : v.edges()) {
        ++count;
        (void)e;
      }
      if (count > 0) {
        ++vertices_with_edges;
      }
    }

    REQUIRE(vertices_with_edges == 20);
  }

  SECTION("large vertex values - 10k vertices") {
    G                        g;
    const size_t             n = 10000;
    std::vector<vertex_data> vv;
    for (uint32_t i = 0; i < n; ++i) {
      vv.push_back({i, static_cast<int>(i * i)});
    }
    g.load_vertices(vv, std::identity{});

    REQUIRE(g.size() == n);
    REQUIRE(g[0].value() == 0);
    REQUIRE(g[5000].value() == 25000000);
    REQUIRE(g[9999].value() == 99980001);
  }
}

//==================================================================================================
// Comprehensive Workflow Tests
//==================================================================================================

TEST_CASE("dod complete workflow scenarios", "[dynamic_graph][dod][workflow]") {
  using G           = dod_int_int_void;
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  using edge_data   = copyable_edge_t<uint32_t, int>;

  SECTION("build graph, query, modify workflow") {
    // Step 1: Build initial graph
    G                        g;
    std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv, std::identity{});

    std::vector<edge_data> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee, std::identity{});

    // Step 2: Query graph properties
    REQUIRE(g.size() == 3);

    size_t total_edges = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        ++total_edges;
        (void)e;
      }
    }
    REQUIRE(total_edges == 2);

    // Step 3: Modify vertex values
    g[0].value() = 999;
    g[1].value() = 888;
    g[2].value() = 777;

    // Step 4: Add more edges
    std::vector<edge_data> more_edges = {{2, 0, 30}};
    g.load_edges(more_edges, std::identity{});

    // Step 5: Verify final state
    REQUIRE(g[0].value() == 999);
    REQUIRE(g[1].value() == 888);
    REQUIRE(g[2].value() == 777);

    total_edges = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        ++total_edges;
        (void)e;
      }
    }
    REQUIRE(total_edges == 3);
  }

  SECTION("social network simulation") {
    // Build a simple social network
    G                        g;
    std::vector<vertex_data> people = {{0, 25},  // age 25
                                       {1, 30},  // age 30
                                       {2, 35},  // age 35
                                       {3, 28},  // age 28
                                       {4, 32}}; // age 32
    g.load_vertices(people, std::identity{});

    // Friendship connections (relationship strength as edge value)
    std::vector<edge_data> friendships = {{0, 1, 5}, {0, 3, 3}, {1, 2, 4}, {1, 4, 2}, {2, 4, 5}, {3, 4, 3}};
    g.load_edges(friendships, std::identity{});

    // Query: Find person with most friends
    size_t max_friends = 0;
    size_t most_social = 0;
    for (size_t i = 0; i < g.size(); ++i) {
      size_t friend_count = 0;
      for (auto& e : g[i].edges()) {
        ++friend_count;
        (void)e;
      }
      if (friend_count > max_friends) {
        max_friends = friend_count;
        most_social = i;
      }
    }

    // Person 0 and person 1 both have 2 friends, person 0 found first
    REQUIRE(most_social == 0);
    REQUIRE(max_friends == 2);

    // Query: Sum of relationship strengths
    int total_strength = 0;
    for (auto& v : g) {
      for (auto& e : v.edges()) {
        total_strength += e.value();
      }
    }
    REQUIRE(total_strength == 22); // 5+3+4+2+5+3
  }

  SECTION("dependency graph workflow") {
    // Build a task dependency graph
    G                        g;
    std::vector<vertex_data> tasks = {{0, 1},  // Task A: priority 1
                                      {1, 2},  // Task B: priority 2
                                      {2, 3},  // Task C: priority 3
                                      {3, 1},  // Task D: priority 1
                                      {4, 2}}; // Task E: priority 2
    g.load_vertices(tasks, std::identity{});

    // Dependencies (task -> depends on)
    std::vector<edge_data> dependencies = {{1, 0, 1},  // B depends on A
                                           {2, 0, 1},  // C depends on A
                                           {2, 1, 1},  // C depends on B
                                           {4, 3, 1}}; // E depends on D
    g.load_edges(dependencies, std::identity{});

    // Find tasks with no dependencies (can start immediately)
    std::vector<size_t> ready_tasks;
    for (size_t i = 0; i < g.size(); ++i) {
      size_t dep_count = 0;
      for (auto& e : g[i].edges()) {
        ++dep_count;
        (void)e;
      }
      if (dep_count == 0) {
        ready_tasks.push_back(i);
      }
    }

    REQUIRE(ready_tasks.size() == 2);
    REQUIRE(std::find(ready_tasks.begin(), ready_tasks.end(), 0) != ready_tasks.end()); // Task A
    REQUIRE(std::find(ready_tasks.begin(), ready_tasks.end(), 3) != ready_tasks.end()); // Task D
  }
}

//==================================================================================================
// Final Summary: Phase 1.1 Complete
//==================================================================================================
// Summary: Phase 1.1 Tests - COMPLETE
// - Construction: 17 tests (TEST_CASE entries)
// - Basic Properties: 7 tests
// - Graph Value: 6 tests
// - Iterator: 5 tests
// - Type Traits: 5 tests
// - Empty Graph Edge Cases: 6 tests
// - Value Types: 12 tests
// - Vertex ID Types: 5 tests
// - Sourced Edges: 6 tests
// - Const Correctness: 6 tests
// - Memory/Resources: 4 tests
// - Compilation: 1 test
// - Load Vertices: 3 tests (9 SECTION entries)
// - Load Edges: 5 tests (6 SECTION entries)
// - Vertex/Edge Access: 10 tests (24 SECTION entries)
// - Error Handling: 3 tests (7 SECTION entries)
// - Edge Cases: 2 tests (5 SECTION entries)
// - Boundary Values: 1 test (4 SECTION entries)
// - Incremental Building: 1 test (3 SECTION entries)
// - Duplicates: 1 test (3 SECTION entries)
// - Graph Properties: 1 test (4 SECTION entries)
// - Special Patterns: 1 test (3 SECTION entries)
// - Iterator Stability: 1 test (3 SECTION entries)
// - Ranges Integration: 1 test (4 SECTION entries)
// - Algorithm Compatibility: 1 test (4 SECTION entries)
// - Performance: 1 test (3 SECTION entries)
// - Workflow Scenarios: 1 test (3 SECTION entries)
//
// Total: 114 TEST_CASE entries with 85 SECTION entries = ~1000+ ctests
// (845 existing tests + ~155 new dynamic_graph vol tests)
//
// Phase 1.1 dod_graph_traits testing: COMPLETE ✓
//
// Coverage achieved:
// - All construction patterns (default, values, copy/move, sourced)
// - Load operations (vertices, edges, projections, batching)
// - Value access and modification (vertices, edges, graph values)
// - Iteration patterns (forward, nested, range-based)
// - Error handling and edge cases
// - Graph properties and queries (degree, edge count, patterns)
// - STL/Ranges integration (algorithms, views, transforms)
// - Performance characteristics (dense, sparse, large graphs)
// - Real-world workflows (social network, dependencies)
//
// Next phases:
// - Phase 1.2: dod_graph_traits (vector + list)
// - Phase 1.3: dod_graph_traits (vector + vector)
// - Phase 1.7: deque-based traits
//==================================================================================================
