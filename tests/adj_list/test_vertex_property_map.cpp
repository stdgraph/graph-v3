/**
 * @file test_vertex_property_map.cpp
 * @brief Unit tests for vertex_property_map type alias and helper functions
 *
 * Tests vertex_property_map<G,T>, make_vertex_property_map (eager + lazy), vertex_property_map_contains,
 * and vertex_property_map_get on both index-based and map-based graph types.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/graph.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/adj_list/vertex_property_map.hpp>
#include <graph/algorithm/traversal_common.hpp>
#include "../common/algorithm_test_types.hpp"
#include "../common/graph_fixtures.hpp"
#include <type_traits>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test::algorithm;
using namespace graph::test::fixtures;

// =============================================================================
// Type Resolution Tests
// =============================================================================

TEST_CASE("vertex_property_map resolves to vector for index graphs", "[vertex_property_map][types]") {
  using Graph = vov_weighted;
  using Map   = vertex_property_map<Graph, int>;
  STATIC_REQUIRE(std::is_same_v<Map, std::vector<int>>);
}

TEST_CASE("vertex_property_map resolves to unordered_map for mapped graphs", "[vertex_property_map][types]") {
  using Graph = mov_weighted;
  using Map   = vertex_property_map<Graph, int>;
  STATIC_REQUIRE(std::is_same_v<Map, std::unordered_map<vertex_id_t<Graph>, int>>);
}

TEST_CASE("vertex_property_map resolves to vector for deque graphs", "[vertex_property_map][types]") {
  using Graph = dov_weighted;
  using Map   = vertex_property_map<Graph, double>;
  STATIC_REQUIRE(std::is_same_v<Map, std::vector<double>>);
}

TEST_CASE("vertex_property_map resolves to unordered_map for unordered_map graphs", "[vertex_property_map][types]") {
  using Graph = uov_weighted;
  using Map   = vertex_property_map<Graph, bool>;
  STATIC_REQUIRE(std::is_same_v<Map, std::unordered_map<vertex_id_t<Graph>, bool>>);
}

// =============================================================================
// make_vertex_property_map — Eager (with init value)
// =============================================================================

TEST_CASE("make_vertex_property_map eager - index graph", "[vertex_property_map][make][eager]") {
  using Graph = vov_weighted;
  auto g      = path_graph_4_weighted<Graph>();
  auto vmap   = make_vertex_property_map<Graph, int>(g, 42);

  REQUIRE(vmap.size() == num_vertices(g));
  for (size_t i = 0; i < vmap.size(); ++i) {
    REQUIRE(vmap[i] == 42);
  }
}

TEST_CASE("make_vertex_property_map eager - mapped graph", "[vertex_property_map][make][eager]") {
  using Graph = mov_weighted;
  auto g      = Graph({{10, 20, 1}, {20, 30, 2}, {30, 40, 3}});
  auto vmap   = make_vertex_property_map<Graph, int>(g, -1);

  REQUIRE(vmap.size() == num_vertices(g));
  // All 4 vertices should have entries with value -1
  for (auto&& [uid, u] : views::vertexlist(g)) {
    REQUIRE(vmap.contains(uid));
    REQUIRE(vmap.at(uid) == -1);
  }
}

// =============================================================================
// make_vertex_property_map — Lazy (capacity only)
// =============================================================================

TEST_CASE("make_vertex_property_map lazy - index graph", "[vertex_property_map][make][lazy]") {
  using Graph = vov_weighted;
  auto g      = path_graph_4_weighted<Graph>();
  auto vmap   = make_vertex_property_map<Graph, int>(g);

  // For index graphs, lazy still creates a sized vector (default-constructed values)
  REQUIRE(vmap.size() == num_vertices(g));
  for (size_t i = 0; i < vmap.size(); ++i) {
    REQUIRE(vmap[i] == 0); // int default = 0
  }
}

TEST_CASE("make_vertex_property_map lazy - mapped graph", "[vertex_property_map][make][lazy]") {
  using Graph = mov_weighted;
  auto g      = Graph({{10, 20, 1}, {20, 30, 2}, {30, 40, 3}});
  auto vmap   = make_vertex_property_map<Graph, int>(g);

  // For mapped graphs, lazy returns an empty map
  REQUIRE(vmap.empty());
  // But it should have reserved buckets (can't easily test, but insert should work)
  vmap[10] = 100;
  REQUIRE(vmap.size() == 1);
  REQUIRE(vmap[10] == 100);
}

// =============================================================================
// vertex_property_map_contains
// =============================================================================

TEST_CASE("vertex_property_map_contains - index graph always true", "[vertex_property_map][contains]") {
  using Graph = vov_weighted;
  auto g      = path_graph_4_weighted<Graph>();
  auto vmap   = make_vertex_property_map<Graph, int>(g, 0);

  for (auto&& [uid, u] : views::vertexlist(g)) {
    REQUIRE(vertex_property_map_contains(vmap, uid));
  }
}

TEST_CASE("vertex_property_map_contains - mapped graph", "[vertex_property_map][contains]") {
  using Graph = mov_weighted;
  auto g      = Graph({{10, 20, 1}, {20, 30, 2}});

  // Lazy: empty map — no entries
  auto vmap = make_vertex_property_map<Graph, int>(g);
  REQUIRE_FALSE(vertex_property_map_contains(vmap, vertex_id_t<Graph>(10)));

  // Add an entry
  vmap[10] = 42;
  REQUIRE(vertex_property_map_contains(vmap, vertex_id_t<Graph>(10)));
  REQUIRE_FALSE(vertex_property_map_contains(vmap, vertex_id_t<Graph>(20)));
}

// =============================================================================
// vertex_property_map_get (read with default, no insertion)
// =============================================================================

TEST_CASE("vertex_property_map_get - index graph returns stored value", "[vertex_property_map][get]") {
  using Graph = vov_weighted;
  auto g      = path_graph_4_weighted<Graph>();
  auto vmap   = make_vertex_property_map<Graph, int>(g, 99);

  vmap[1] = 42;
  REQUIRE(vertex_property_map_get(vmap, vertex_id_t<Graph>(0), -1) == 99);
  REQUIRE(vertex_property_map_get(vmap, vertex_id_t<Graph>(1), -1) == 42);
}

TEST_CASE("vertex_property_map_get - mapped graph returns default for absent keys", "[vertex_property_map][get]") {
  using Graph = mov_weighted;
  auto g      = Graph({{10, 20, 1}, {20, 30, 2}});
  auto vmap   = make_vertex_property_map<Graph, int>(g); // lazy: empty

  // Absent key → default
  REQUIRE(vertex_property_map_get(vmap, vertex_id_t<Graph>(10), -1) == -1);

  // Insert a value
  vmap[10] = 42;
  REQUIRE(vertex_property_map_get(vmap, vertex_id_t<Graph>(10), -1) == 42);

  // Absent key still returns default — and does NOT insert
  REQUIRE(vertex_property_map_get(vmap, vertex_id_t<Graph>(20), -1) == -1);
  REQUIRE(vmap.size() == 1); // no spurious insertion
}

// =============================================================================
// is_null_range_v trait
// =============================================================================

TEST_CASE("is_null_range_v trait", "[vertex_property_map][null_range]") {
  STATIC_REQUIRE(is_null_range_v<_null_range_type>);
  STATIC_REQUIRE(is_null_range_v<const _null_range_type>);
  STATIC_REQUIRE(is_null_range_v<_null_range_type&>);
  STATIC_REQUIRE(is_null_range_v<const _null_range_type&>);
  STATIC_REQUIRE_FALSE(is_null_range_v<std::vector<int>>);
  STATIC_REQUIRE_FALSE(is_null_range_v<std::vector<size_t>>);
  STATIC_REQUIRE_FALSE(is_null_range_v<int>);
}
