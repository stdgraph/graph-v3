/**
 * @file test_dynamic_graph_stl_algorithms.cpp
 * @brief Integration tests for dynamic_graph - STL algorithm compatibility
 * 
 * Phase 6.2: STL Algorithm Integration
 * Tests that standard library algorithms work correctly with dynamic_graph ranges.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/dol_graph_traits.hpp>
#include <graph/graph_info.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <map>

using namespace graph::container;

//==================================================================================================
// Type Aliases
//==================================================================================================

// Sequential container graphs (integral VId) - void values
using vov_void = dynamic_graph<void, void, void, uint64_t, false, vov_graph_traits<void, void, void, uint64_t, false>>;
using dofl_void =
      dynamic_graph<void, void, void, uint64_t, false, dofl_graph_traits<void, void, void, uint64_t, false>>;
using dol_void = dynamic_graph<void, void, void, uint64_t, false, dol_graph_traits<void, void, void, uint64_t, false>>;

// Sequential container graphs (integral VId) - int edges
using vov_int_edges =
      dynamic_graph<int, void, void, uint64_t, false, vov_graph_traits<int, void, void, uint64_t, false>>;
using dofl_int_edges =
      dynamic_graph<int, void, void, uint64_t, false, dofl_graph_traits<int, void, void, uint64_t, false>>;

// Sequential container graphs (integral VId) - int vertices
using vov_int_verts =
      dynamic_graph<void, int, void, uint64_t, false, vov_graph_traits<void, int, void, uint64_t, false>>;
using dofl_int_verts =
      dynamic_graph<void, int, void, uint64_t, false, dofl_graph_traits<void, int, void, uint64_t, false>>;

// Sequential container graphs (integral VId) - int edges and vertices
using vov_int_both = dynamic_graph<int, int, void, uint64_t, false, vov_graph_traits<int, int, void, uint64_t, false>>;
using dofl_int_both =
      dynamic_graph<int, int, void, uint64_t, false, dofl_graph_traits<int, int, void, uint64_t, false>>;

// Map-based graphs (string VId) - void values
using mos_void =
      dynamic_graph<void, void, void, std::string, false, mos_graph_traits<void, void, void, std::string, false>>;

// Map-based graphs (string VId) - int edges
using mos_int_edges =
      dynamic_graph<int, void, void, std::string, false, mos_graph_traits<int, void, void, std::string, false>>;

// Map-based graphs (string VId) - int vertices
using mos_int_verts =
      dynamic_graph<void, int, void, std::string, false, mos_graph_traits<void, int, void, std::string, false>>;

//==================================================================================================
// Helper Functions
//==================================================================================================

template <typename G>
size_t count_all_edges(const G& g) {
  size_t count = 0;
  for (auto&& v : vertices(g)) {
    count += static_cast<size_t>(std::ranges::distance(edges(g, v)));
  }
  return count;
}

//==================================================================================================
// Phase 6.2.1: std::ranges::for_each
//==================================================================================================

TEST_CASE("for_each - count vertices in vov graph", "[stl][6.2.1][for_each]") {
  vov_void g({{0, 1}, {1, 2}, {2, 3}});

  size_t count = 0;
  std::ranges::for_each(vertices(g), [&count](auto&&) { ++count; });

  REQUIRE(count == 4);
}

TEST_CASE("for_each - count vertices in mos graph", "[stl][6.2.1][for_each]") {
  mos_void g({{"A", "B"}, {"B", "C"}, {"C", "D"}});

  size_t count = 0;
  std::ranges::for_each(vertices(g), [&count](auto&&) { ++count; });

  REQUIRE(count == 4);
}

TEST_CASE("for_each - count vertices in dofl graph", "[stl][6.2.1][for_each]") {
  dofl_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

  size_t count = 0;
  std::ranges::for_each(vertices(g), [&count](auto&&) { ++count; });

  REQUIRE(count == 5);
}

TEST_CASE("for_each - accumulate vertex IDs in vov graph", "[stl][6.2.1][for_each]") {
  vov_void g({{0, 1}, {1, 2}, {2, 3}});

  uint64_t sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) { sum += vertex_id(g, v); });

  REQUIRE(sum == 6); // 0 + 1 + 2 + 3
}

TEST_CASE("for_each - accumulate vertex values in vov graph", "[stl][6.2.1][for_each]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) * 10);
  }

  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) { sum += vertex_value(g, v); });

  REQUIRE(sum == 60); // 0 + 10 + 20 + 30
}

TEST_CASE("for_each - accumulate vertex values in dofl graph", "[stl][6.2.1][for_each]") {
  dofl_int_verts g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) + 5);
  }

  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) { sum += vertex_value(g, v); });

  REQUIRE(sum == 18); // 5 + 6 + 7
}

TEST_CASE("for_each - count edges in vov graph", "[stl][6.2.1][for_each]") {
  vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

  size_t edge_count = 0;
  std::ranges::for_each(vertices(g), [&edge_count, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&edge_count](auto&&) { ++edge_count; });
  });

  REQUIRE(edge_count == 4);
}

TEST_CASE("for_each - count edges in mos graph", "[stl][6.2.1][for_each]") {
  mos_void g({{"A", "B"}, {"B", "C"}, {"C", "A"}});

  size_t edge_count = 0;
  std::ranges::for_each(vertices(g), [&edge_count, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&edge_count](auto&&) { ++edge_count; });
  });

  REQUIRE(edge_count == 3);
}

TEST_CASE("for_each - count edges in dofl graph", "[stl][6.2.1][for_each]") {
  dofl_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}});

  size_t edge_count = 0;
  std::ranges::for_each(vertices(g), [&edge_count, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&edge_count](auto&&) { ++edge_count; });
  });

  REQUIRE(edge_count == 4);
}

TEST_CASE("for_each - sum edge values in vov graph", "[stl][6.2.1][for_each]") {
  vov_int_edges g({{0, 1, 10}, {1, 2, 20}, {2, 3, 30}});

  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&sum, &g](auto&& e) { sum += edge_value(g, e); });
  });

  REQUIRE(sum == 60);
}

TEST_CASE("for_each - sum edge values in dofl graph", "[stl][6.2.1][for_each]") {
  dofl_int_edges g({{0, 1, 5}, {1, 2, 15}, {2, 3, 25}, {3, 0, 35}});

  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&sum, &g](auto&& e) { sum += edge_value(g, e); });
  });

  REQUIRE(sum == 80);
}

TEST_CASE("for_each - sum edge values in mos graph", "[stl][6.2.1][for_each]") {
  mos_int_edges g({{"A", "B", 100}, {"B", "C", 200}, {"C", "A", 300}});

  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&sum, &g](auto&& e) { sum += edge_value(g, e); });
  });

  REQUIRE(sum == 600);
}

TEST_CASE("for_each - collect edge target IDs in vov graph", "[stl][6.2.1][for_each]") {
  vov_void g({{0, 1}, {0, 2}, {1, 3}});

  std::vector<uint64_t> targets;
  std::ranges::for_each(vertices(g), [&targets, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&targets, &g](auto&& e) { targets.push_back(target_id(g, e)); });
  });

  std::ranges::sort(targets);
  REQUIRE(targets == std::vector<uint64_t>{1, 2, 3});
}

TEST_CASE("for_each - modify vertex values in place", "[stl][6.2.1][for_each]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});

  // Initialize vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v));
  }

  // Double all vertex values
  std::ranges::for_each(vertices(g), [&g](auto&& v) { vertex_value(g, v) *= 2; });

  // Verify
  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) { sum += vertex_value(g, v); });

  REQUIRE(sum == 12); // (0*2) + (1*2) + (2*2) + (3*2) = 0 + 2 + 4 + 6 = 12
}

TEST_CASE("for_each - modify edge values in place", "[stl][6.2.1][for_each]") {
  vov_int_edges g({{0, 1, 1}, {1, 2, 2}, {2, 3, 3}});

  // Triple all edge values
  std::ranges::for_each(
        vertices(g), [&g](auto&& v) { std::ranges::for_each(edges(g, v), [&g](auto&& e) { edge_value(g, e) *= 3; }); });

  // Verify
  int sum = 0;
  std::ranges::for_each(vertices(g), [&sum, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&sum, &g](auto&& e) { sum += edge_value(g, e); });
  });

  REQUIRE(sum == 18); // (1*3) + (2*3) + (3*3) = 3 + 6 + 9 = 18
}

TEST_CASE("for_each - modify both vertex and edge values", "[stl][6.2.1][for_each]") {
  vov_int_both g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});

  // Initialize vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) + 1);
  }

  // Add 5 to all vertex values
  std::ranges::for_each(vertices(g), [&g](auto&& v) { vertex_value(g, v) += 5; });

  // Add 100 to all edge values
  std::ranges::for_each(vertices(g), [&g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&g](auto&& e) { edge_value(g, e) += 100; });
  });

  // Verify vertex values
  int vertex_sum = 0;
  std::ranges::for_each(vertices(g), [&vertex_sum, &g](auto&& v) { vertex_sum += vertex_value(g, v); });
  REQUIRE(vertex_sum == 21); // (1+5) + (2+5) + (3+5) = 6 + 7 + 8 = 21

  // Verify edge values
  int edge_sum = 0;
  std::ranges::for_each(vertices(g), [&edge_sum, &g](auto&& v) {
    std::ranges::for_each(edges(g, v), [&edge_sum, &g](auto&& e) { edge_sum += edge_value(g, e); });
  });
  REQUIRE(edge_sum == 360); // (10+100) + (20+100) + (30+100) = 110 + 120 + 130 = 360
}

TEST_CASE("for_each - empty graph vertices", "[stl][6.2.1][for_each]") {
  vov_void g;

  size_t count = 0;
  std::ranges::for_each(vertices(g), [&count](auto&&) { ++count; });

  REQUIRE(count == 0);
}

TEST_CASE("for_each - graph with no edges", "[stl][6.2.1][for_each]") {
  vov_void g;
  g.resize_vertices(4);

  size_t vertex_count = 0;
  size_t edge_count   = 0;

  std::ranges::for_each(vertices(g), [&vertex_count, &edge_count, &g](auto&& v) {
    ++vertex_count;
    std::ranges::for_each(edges(g, v), [&edge_count](auto&&) { ++edge_count; });
  });

  REQUIRE(vertex_count == 4);
  REQUIRE(edge_count == 0);
}

TEST_CASE("for_each - single vertex with self-loop", "[stl][6.2.1][for_each]") {
  vov_int_edges g({{0, 0, 42}});

  size_t vertex_count = 0;
  size_t edge_count   = 0;
  int    edge_sum     = 0;

  std::ranges::for_each(vertices(g), [&vertex_count, &edge_count, &edge_sum, &g](auto&& v) {
    ++vertex_count;
    std::ranges::for_each(edges(g, v), [&edge_count, &edge_sum, &g](auto&& e) {
      ++edge_count;
      edge_sum += edge_value(g, e);
    });
  });

  REQUIRE(vertex_count == 1);
  REQUIRE(edge_count == 1);
  REQUIRE(edge_sum == 42);
}

TEST_CASE("for_each - count degrees using for_each", "[stl][6.2.1][for_each]") {
  vov_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}});

  std::map<uint64_t, size_t> degree_map;

  std::ranges::for_each(vertices(g), [&degree_map, &g](auto&& v) {
    auto   vid      = vertex_id(g, v);
    size_t deg      = static_cast<size_t>(std::ranges::distance(edges(g, v)));
    degree_map[vid] = deg;
  });

  REQUIRE(degree_map[0] == 2);
  REQUIRE(degree_map[1] == 2);
  REQUIRE(degree_map[2] == 1);
  REQUIRE(degree_map[3] == 0);
}

TEST_CASE("for_each - complex accumulation pattern", "[stl][6.2.1][for_each]") {
  vov_int_both g({{0, 1, 5}, {1, 2, 15}, {2, 0, 25}});

  // Initialize vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>((vertex_id(g, v) + 1) * 10);
  }

  // Sum: (vertex_id * vertex_value) + sum_of_edge_values_from_vertex
  int total = 0;

  std::ranges::for_each(vertices(g), [&total, &g](auto&& v) {
    auto vid  = vertex_id(g, v);
    auto vval = vertex_value(g, v);
    total += static_cast<int>(vid) * vval;

    std::ranges::for_each(edges(g, v), [&total, &g](auto&& e) { total += edge_value(g, e); });
  });

  // Expected: (0*10 + 5) + (1*20 + 15) + (2*30 + 25) = 5 + 35 + 85 = 125
  REQUIRE(total == 125);
}

TEST_CASE("for_each - collect all vertex IDs in mos graph", "[stl][6.2.1][for_each]") {
  mos_void g({{"X", "Y"}, {"Y", "Z"}, {"Z", "X"}});

  std::vector<std::string> ids;
  std::ranges::for_each(vertices(g), [&ids, &g](auto&& v) { ids.push_back(vertex_id(g, v)); });

  std::ranges::sort(ids);
  REQUIRE(ids == std::vector<std::string>{"X", "Y", "Z"});
}

TEST_CASE("for_each - nested for_each with multiple graphs", "[stl][6.2.1][for_each]") {
  vov_void  g1({{0, 1}, {1, 2}});
  dofl_void g2({{0, 1}, {1, 2}, {2, 3}});

  size_t total_vertices = 0;
  size_t total_edges    = 0;

  std::ranges::for_each(vertices(g1), [&total_vertices, &total_edges, &g1](auto&& v) {
    ++total_vertices;
    std::ranges::for_each(edges(g1, v), [&total_edges](auto&&) { ++total_edges; });
  });

  std::ranges::for_each(vertices(g2), [&total_vertices, &total_edges, &g2](auto&& v) {
    ++total_vertices;
    std::ranges::for_each(edges(g2, v), [&total_edges](auto&&) { ++total_edges; });
  });

  REQUIRE(total_vertices == 7); // 3 from g1 + 4 from g2
  REQUIRE(total_edges == 5);    // 2 from g1 + 3 from g2
}

TEST_CASE("for_each - maximum degree vertex", "[stl][6.2.1][for_each]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 3}});

  uint64_t max_degree_vertex = 0;
  size_t   max_degree        = 0;

  std::ranges::for_each(vertices(g), [&max_degree_vertex, &max_degree, &g](auto&& v) {
    auto   vid = vertex_id(g, v);
    size_t deg = static_cast<size_t>(std::ranges::distance(edges(g, v)));
    if (deg > max_degree) {
      max_degree        = deg;
      max_degree_vertex = vid;
    }
  });

  REQUIRE(max_degree_vertex == 0);
  REQUIRE(max_degree == 3);
}

//==================================================================================================
// Phase 6.2.2: std::ranges::find_if and Search
//==================================================================================================

TEST_CASE("find_if - find vertex by ID in vov graph", "[stl][6.2.2][find_if]") {
  vov_void g({{0, 1}, {1, 2}, {2, 3}});

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return vertex_id(g, v) == 2; });

  REQUIRE(it != verts.end());
  REQUIRE(vertex_id(g, *it) == 2);
}

TEST_CASE("find_if - find vertex by ID in mos graph", "[stl][6.2.2][find_if]") {
  mos_void g({{"A", "B"}, {"B", "C"}, {"C", "D"}});

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return vertex_id(g, v) == "C"; });

  REQUIRE(it != verts.end());
  REQUIRE(vertex_id(g, *it) == "C");
}

TEST_CASE("find_if - find vertex by ID in dofl graph", "[stl][6.2.2][find_if]") {
  dofl_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return vertex_id(g, v) == 3; });

  REQUIRE(it != verts.end());
  REQUIRE(vertex_id(g, *it) == 3);
}

TEST_CASE("find_if - vertex not found returns end", "[stl][6.2.2][find_if]") {
  vov_void g({{0, 1}, {1, 2}});

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return vertex_id(g, v) == 99; });

  REQUIRE(it == verts.end());
}

TEST_CASE("find_if - find vertex by value predicate in vov graph", "[stl][6.2.2][find_if]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) * 10);
  }

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return vertex_value(g, v) == 20; });

  REQUIRE(it != verts.end());
  REQUIRE(vertex_id(g, *it) == 2);
  REQUIRE(vertex_value(g, *it) == 20);
}

TEST_CASE("find_if - find vertex by value predicate in mos graph", "[stl][6.2.2][find_if]") {
  mos_int_verts g({{"A", "B"}, {"B", "C"}});

  // Set vertex values
  vertex_value(g, *vertices(g).begin()) = 100;
  auto it                               = vertices(g).begin();
  ++it;
  vertex_value(g, *it) = 200;
  ++it;
  vertex_value(g, *it) = 300;

  auto verts = vertices(g);
  auto found = std::ranges::find_if(verts, [&g](auto&& v) { return vertex_value(g, v) > 150; });

  REQUIRE(found != verts.end());
  REQUIRE(vertex_value(g, *found) >= 200);
}

TEST_CASE("find_if - find vertex with specific degree", "[stl][6.2.2][find_if]") {
  vov_void g({{0, 1}, {0, 2}, {1, 3}, {2, 3}});

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return std::ranges::distance(edges(g, v)) == 2; });

  REQUIRE(it != verts.end());
  REQUIRE(vertex_id(g, *it) == 0);
}

TEST_CASE("find_if - find isolated vertex", "[stl][6.2.2][find_if]") {
  vov_void g({{0, 1}, {1, 2}});
  g.resize_vertices(5);

  auto verts = vertices(g);
  auto it    = std::ranges::find_if(verts, [&g](auto&& v) { return std::ranges::distance(edges(g, v)) == 0; });

  REQUIRE(it != verts.end());
  // Could find vertex 2, 3, or 4 (all have out-degree 0)
  REQUIRE(std::ranges::distance(edges(g, *it)) == 0);
}

TEST_CASE("find_if - find edge by target ID in vov graph", "[stl][6.2.2][find_if]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}});

  auto v          = *vertices(g).begin();
  auto edge_range = edges(g, v);
  auto it         = std::ranges::find_if(edge_range, [&g](auto&& e) { return target_id(g, e) == 2; });

  REQUIRE(it != edge_range.end());
  REQUIRE(target_id(g, *it) == 2);
}

TEST_CASE("find_if - find edge by target ID in mos graph", "[stl][6.2.2][find_if]") {
  mos_void g({{"A", "B"}, {"A", "C"}, {"A", "D"}});

  auto v          = *vertices(g).begin();
  auto edge_range = edges(g, v);
  auto it         = std::ranges::find_if(edge_range, [&g](auto&& e) { return target_id(g, e) == "C"; });

  REQUIRE(it != edge_range.end());
  REQUIRE(target_id(g, *it) == "C");
}

TEST_CASE("find_if - edge not found returns end", "[stl][6.2.2][find_if]") {
  vov_void g({{0, 1}, {0, 2}});

  auto v          = *vertices(g).begin();
  auto edge_range = edges(g, v);
  auto it         = std::ranges::find_if(edge_range, [&g](auto&& e) { return target_id(g, e) == 99; });

  REQUIRE(it == edge_range.end());
}

TEST_CASE("find_if - find edge by value predicate in vov graph", "[stl][6.2.2][find_if]") {
  vov_int_edges g({{0, 1, 0}, {0, 2, 0}, {0, 3, 0}});

  // Set edge values
  size_t idx = 0;
  auto   v   = *vertices(g).begin();
  for (auto&& e : edges(g, v)) {
    edge_value(g, e) = static_cast<int>(idx * 10);
    ++idx;
  }

  auto edge_range = edges(g, v);
  auto it         = std::ranges::find_if(edge_range, [&g](auto&& e) { return edge_value(g, e) == 10; });

  REQUIRE(it != edge_range.end());
  REQUIRE(edge_value(g, *it) == 10);
}

TEST_CASE("find_if - find edge by value predicate in dofl graph", "[stl][6.2.2][find_if]") {
  dofl_int_edges g({{0, 1, 0}, {0, 2, 0}, {0, 3, 0}});

  // Set edge values
  auto   v   = *vertices(g).begin();
  size_t idx = 0;
  for (auto&& e : edges(g, v)) {
    edge_value(g, e) = static_cast<int>(idx * 5);
    ++idx;
  }

  auto edge_range = edges(g, v);
  auto it         = std::ranges::find_if(edge_range, [&g](auto&& e) { return edge_value(g, e) > 7; });

  REQUIRE(it != edge_range.end());
  REQUIRE(edge_value(g, *it) >= 10);
}

TEST_CASE("find_if - find edge with specific target value", "[stl][6.2.2][find_if]") {
  vov_int_both g({{0, 1, 0}, {0, 2, 0}, {0, 3, 0}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) * 100);
  }

  // Set edge values
  auto   v0  = *vertices(g).begin();
  size_t idx = 0;
  for (auto&& e : edges(g, v0)) {
    edge_value(g, e) = static_cast<int>(idx);
    ++idx;
  }

  auto edge_range = edges(g, v0);
  auto it         = std::ranges::find_if(edge_range, [&g](auto&& e) {
    auto target = target_id(g, e);
    // Find edge pointing to vertex with value 200
    for (auto&& v : vertices(g)) {
      if (vertex_id(g, v) == target) {
        return vertex_value(g, v) == 200;
      }
    }
    return false;
  });

  REQUIRE(it != edge_range.end());
  REQUIRE(target_id(g, *it) == 2);
}

TEST_CASE("find - find specific vertex ID in vov graph", "[stl][6.2.2][find]") {
  vov_void g({{0, 1}, {1, 2}, {2, 3}});

  // Create a vector of vertex IDs
  std::vector<uint64_t> ids;
  for (auto&& v : vertices(g)) {
    ids.push_back(vertex_id(g, v));
  }

  auto it = std::ranges::find(ids, 2);

  REQUIRE(it != ids.end());
  REQUIRE(*it == 2);
}

TEST_CASE("find - find specific vertex ID in mos graph", "[stl][6.2.2][find]") {
  mos_void g({{"A", "B"}, {"B", "C"}});

  // Create a vector of vertex IDs
  std::vector<std::string> ids;
  for (auto&& v : vertices(g)) {
    ids.push_back(vertex_id(g, v));
  }

  auto it = std::ranges::find(ids, "B");

  REQUIRE(it != ids.end());
  REQUIRE(*it == "B");
}

TEST_CASE("find - find specific vertex value", "[stl][6.2.2][find]") {
  vov_int_verts g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) * 7);
  }

  // Create a vector of vertex values
  std::vector<int> values;
  for (auto&& v : vertices(g)) {
    values.push_back(vertex_value(g, v));
  }

  auto it = std::ranges::find(values, 7);

  REQUIRE(it != values.end());
  REQUIRE(*it == 7);
}

TEST_CASE("find - find specific edge value", "[stl][6.2.2][find]") {
  vov_int_edges g({{0, 1, 0}, {0, 2, 0}, {0, 3, 0}});

  // Set edge values
  auto   v   = *vertices(g).begin();
  size_t idx = 0;
  for (auto&& e : edges(g, v)) {
    edge_value(g, e) = static_cast<int>(idx * 3);
    ++idx;
  }

  // Create a vector of edge values
  std::vector<int> values;
  for (auto&& e : edges(g, v)) {
    values.push_back(edge_value(g, e));
  }

  auto it = std::ranges::find(values, 3);

  REQUIRE(it != values.end());
  REQUIRE(*it == 3);
}

TEST_CASE("find_if_not - find vertex without edges", "[stl][6.2.2][find_if_not]") {
  vov_void g({{0, 1}, {1, 2}});
  g.resize_vertices(4);

  auto verts = vertices(g);
  auto it    = std::ranges::find_if_not(verts, [&g](auto&& v) { return std::ranges::distance(edges(g, v)) > 0; });

  REQUIRE(it != verts.end());
  REQUIRE(std::ranges::distance(edges(g, *it)) == 0);
}

TEST_CASE("find_if_not - find vertex with value not matching", "[stl][6.2.2][find_if_not]") {
  vov_int_verts g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) * 2);
  }

  auto verts = vertices(g);
  auto it    = std::ranges::find_if_not(verts, [&g](auto&& v) { return vertex_value(g, v) % 4 == 0; });

  REQUIRE(it != verts.end());
  REQUIRE(vertex_value(g, *it) % 4 != 0);
}

TEST_CASE("any_of - check if any vertex has high degree", "[stl][6.2.2][any_of]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}});

  bool has_high_degree =
        std::ranges::any_of(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) >= 3; });

  REQUIRE(has_high_degree);
}

TEST_CASE("any_of - check if any vertex has specific value", "[stl][6.2.2][any_of]") {
  vov_int_verts g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) * 5);
  }

  bool has_value_10 = std::ranges::any_of(vertices(g), [&g](auto&& v) { return vertex_value(g, v) == 10; });

  REQUIRE(has_value_10);
}

TEST_CASE("all_of - check if all vertices have edges", "[stl][6.2.2][all_of]") {
  vov_void g({{0, 1}, {1, 2}, {2, 0}});

  bool all_have_edges =
        std::ranges::all_of(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) > 0; });

  REQUIRE(all_have_edges);
}

TEST_CASE("all_of - check if all vertices have positive values", "[stl][6.2.2][all_of]") {
  vov_int_verts g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) + 1);
  }

  bool all_positive = std::ranges::all_of(vertices(g), [&g](auto&& v) { return vertex_value(g, v) > 0; });

  REQUIRE(all_positive);
}

TEST_CASE("none_of - check if no vertex is isolated in connected graph", "[stl][6.2.2][none_of]") {
  vov_void g({{0, 1}, {1, 2}, {2, 0}});

  bool none_isolated =
        std::ranges::none_of(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) == 0; });

  REQUIRE(none_isolated);
}

TEST_CASE("none_of - check if no vertex has negative value", "[stl][6.2.2][none_of]") {
  vov_int_verts g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v) + 5);
  }

  bool none_negative = std::ranges::none_of(vertices(g), [&g](auto&& v) { return vertex_value(g, v) < 0; });

  REQUIRE(none_negative);
}

TEST_CASE("search - find sequence of target IDs in edge list", "[stl][6.2.2][search]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});

  auto v = *vertices(g).begin();

  // Collect target IDs
  std::vector<uint64_t> targets;
  for (auto&& e : edges(g, v)) {
    targets.push_back(target_id(g, e));
  }

  // Search for sequence [2, 3]
  std::vector<uint64_t> sequence = {2, 3};
  auto                  it       = std::ranges::search(targets, sequence);

  REQUIRE(it.begin() != targets.end());
}

TEST_CASE("search - sequence not found returns end", "[stl][6.2.2][search]") {
  vov_void g({{0, 1}, {0, 3}, {0, 5}});

  auto v = *vertices(g).begin();

  // Collect target IDs
  std::vector<uint64_t> targets;
  for (auto&& e : edges(g, v)) {
    targets.push_back(target_id(g, e));
  }

  // Search for sequence [2, 3] which doesn't exist
  std::vector<uint64_t> sequence = {2, 3};
  auto                  it       = std::ranges::search(targets, sequence);

  REQUIRE(it.begin() == targets.end());
}

// ============================================================================
// Phase 6.2.3: std::ranges::count_if and Filtering
// ============================================================================

TEST_CASE("count_if - count vertices with specific property (vov)", "[stl][6.2.3][count_if]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});
  g.resize_vertices(6); // Vertices 0, 1, 2, 3, 4, 5

  // Assign values: [10, 20, 30, 0, 0, 0]
  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 30;

  // Count vertices with value > 15
  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) { return vertex_value(g, v) > 15; });

  REQUIRE(count == 2); // Vertices with values 20 and 30
}

TEST_CASE("count_if - count vertices with out-degree > 0 (dofl)", "[stl][6.2.3][count_if]") {
  dofl_void g({{0, 1}, {0, 2}, {1, 3}});
  g.resize_vertices(5);

  // Count vertices that have outgoing edges
  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) > 0; });

  REQUIRE(count == 2); // Vertices 0 and 1 have edges
}

TEST_CASE("count_if - count vertices by degree threshold (vov)", "[stl][6.2.3][count_if]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}});

  // Count vertices with out-degree >= 2
  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) >= 2; });

  REQUIRE(count == 1); // Only vertex 0 has degree >= 2 (degree 3)
}

TEST_CASE("count_if - count vertices with ID in range (mos)", "[stl][6.2.3][count_if]") {
  mos_void g({{"A", "B"}, {"B", "C"}, {"C", "D"}});

  // Count vertices with ID between "A" and "C" (inclusive)
  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) {
    auto id = vertex_id(g, v);
    return id >= "A" && id <= "C";
  });

  REQUIRE(count == 3); // A, B, C
}

TEST_CASE("count_if - count edges with specific target (vov)", "[stl][6.2.3][count_if]") {
  vov_void g({{0, 1}, {0, 2}, {0, 1}, {1, 2}}); // Duplicate edge to 1

  auto v = *vertices(g).begin();

  // Count edges from vertex 0 to vertex 1
  auto count = std::ranges::count_if(edges(g, v), [&g](auto&& e) { return target_id(g, e) == 1; });

  REQUIRE(count == 2); // Two edges to vertex 1
}

TEST_CASE("count_if - count edges with value above threshold (vov)", "[stl][6.2.3][count_if]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 5}});

  auto v = *vertices(g).begin();

  // Count edges from vertex 0 with value >= 20
  auto count = std::ranges::count_if(edges(g, v), [&g](auto&& e) { return edge_value(g, e) >= 20; });

  REQUIRE(count == 2); // Edges with values 20 and 30
}

TEST_CASE("count_if - count edges by target range (dofl)", "[stl][6.2.3][count_if]") {
  dofl_int_edges g({{0, 1, 100}, {0, 5, 200}, {0, 10, 300}});

  auto v = *vertices(g).begin();

  // Count edges with target ID >= 5
  auto count = std::ranges::count_if(edges(g, v), [&g](auto&& e) { return target_id(g, e) >= 5; });

  REQUIRE(count == 2); // Edges to vertices 5 and 10
}

TEST_CASE("count_if - count self-loops in graph (vov)", "[stl][6.2.3][count_if]") {
  vov_void g({{0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}});

  size_t self_loops = 0;
  for (auto&& u : vertices(g)) {
    auto uid = vertex_id(g, u);
    self_loops += static_cast<size_t>(
          std::ranges::count_if(edges(g, u), [&g, uid](auto&& e) { return target_id(g, e) == uid; }));
  }

  REQUIRE(self_loops == 3); // Vertices 0, 1, and 2 each have a self-loop
}

TEST_CASE("count_if - count self-loops in specific vertex (mos)", "[stl][6.2.3][count_if]") {
  mos_void g({{"A", "A"}, {"A", "B"}, {"B", "B"}, {"B", "C"}});

  auto it  = vertices(g).begin();
  auto v   = *it;
  auto uid = vertex_id(g, v);

  auto count = std::ranges::count_if(edges(g, v), [&g, &uid](auto&& e) { return target_id(g, e) == uid; });

  REQUIRE(count == 1); // Vertex "A" has one self-loop
}

TEST_CASE("count_if - count edges with both conditions (vov)", "[stl][6.2.3][count_if]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 25}, {0, 3, 30}, {0, 4, 5}});

  auto v = *vertices(g).begin();

  // Count edges with target ID > 1 AND value >= 20
  auto count =
        std::ranges::count_if(edges(g, v), [&g](auto&& e) { return target_id(g, e) > 1 && edge_value(g, e) >= 20; });

  REQUIRE(count == 2); // Edges to 2 (value 25) and 3 (value 30)
}

TEST_CASE("count - count vertices with specific value (vov)", "[stl][6.2.3][count]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});
  g.resize_vertices(6);

  // Assign values: [10, 10, 20, 10, 0, 0]
  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 10;

  // Extract values and count
  std::vector<int> values;
  for (auto&& v : vertices(g)) {
    values.push_back(vertex_value(g, v));
  }

  auto count = std::ranges::count(values, 10);
  REQUIRE(count == 3);
}

TEST_CASE("count - count edges to specific target (vov)", "[stl][6.2.3][count]") {
  vov_void g({{0, 1}, {0, 2}, {0, 1}, {0, 3}, {0, 1}});

  auto v = *vertices(g).begin();

  // Extract target IDs
  std::vector<uint64_t> targets;
  for (auto&& e : edges(g, v)) {
    targets.push_back(target_id(g, e));
  }

  auto count = std::ranges::count(targets, 1);
  REQUIRE(count == 3); // Three edges to vertex 1
}

TEST_CASE("count_if - count isolated vertices (dofl)", "[stl][6.2.3][count_if]") {
  dofl_void g({{0, 1}, {1, 2}});
  g.resize_vertices(6); // Vertices 3, 4, 5 are isolated

  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) == 0; });

  REQUIRE(count == 4); // Vertices 2, 3, 4, 5 have no outgoing edges
}

TEST_CASE("count_if - count high-degree vertices (vov)", "[stl][6.2.3][count_if]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 2}, {1, 3}});

  // Count vertices with out-degree >= 3
  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) { return std::ranges::distance(edges(g, v)) >= 3; });

  REQUIRE(count == 1); // Only vertex 0 has degree >= 3 (degree 4)
}

TEST_CASE("count_if - empty graph returns zero (vov)", "[stl][6.2.3][count_if]") {
  vov_void g;

  auto count = std::ranges::count_if(vertices(g), [&g](auto&&) {
    return true; // Always true, but no vertices
  });

  REQUIRE(count == 0);
}

TEST_CASE("count_if - count with complex predicate (vov)", "[stl][6.2.3][count_if]") {
  vov_int_both g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});
  g.resize_vertices(5);

  // Set vertex values: vertex 0 = 5, vertex 1 = 15, vertex 2 = 25, others = 0
  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 5;  // Vertex 0: value 5, out-degree 2
  vertex_value(g, *it++) = 15; // Vertex 1: value 15, out-degree 1
  vertex_value(g, *it++) = 25; // Vertex 2: value 25, out-degree 0

  // Count vertices with value >= 10 AND out-degree > 0
  auto count = std::ranges::count_if(
        vertices(g), [&g](auto&& v) { return vertex_value(g, v) >= 10 && std::ranges::distance(edges(g, v)) > 0; });

  REQUIRE(count == 1); // Only vertex 1 (value 15, degree 1) matches both conditions
}

TEST_CASE("count_if - count edges with negative values (vov)", "[stl][6.2.3][count_if]") {
  vov_int_edges g({{0, 1, -5}, {0, 2, 10}, {0, 3, -3}, {1, 2, 20}});

  auto v = *vertices(g).begin();

  auto count = std::ranges::count_if(edges(g, v), [&g](auto&& e) { return edge_value(g, e) < 0; });

  REQUIRE(count == 2); // Edges with values -5 and -3
}

TEST_CASE("count_if - count vertices in mos with string predicate", "[stl][6.2.3][count_if]") {
  mos_int_verts g({{"apple", "banana"}, {"banana", "cherry"}, {"cherry", "date"}});

  // Set vertex values through iteration
  for (auto&& v : vertices(g)) {
    auto id = vertex_id(g, v);
    if (id == "apple")
      vertex_value(g, v) = 10;
    else if (id == "banana")
      vertex_value(g, v) = 20;
    else if (id == "cherry")
      vertex_value(g, v) = 30;
    else if (id == "date")
      vertex_value(g, v) = 40;
  }

  // Count vertices whose ID starts with 'b' or later
  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) {
    auto id = vertex_id(g, v);
    return !id.empty() && id[0] >= 'b';
  });

  REQUIRE(count == 3); // banana, cherry, date
}

TEST_CASE("count_if - filter vertices then count (vov)", "[stl][6.2.3][count_if][filter]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});
  g.resize_vertices(8);

  // Set values: [10, 20, 30, 40, 50, 60, 0, 0]
  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 30;
  vertex_value(g, *it++) = 40;
  vertex_value(g, *it++) = 50;
  vertex_value(g, *it++) = 60;

  // Filter vertices with value >= 25, then count those with value <= 45
  auto filtered = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) >= 25; });

  auto count = std::ranges::count_if(filtered, [&g](auto&& v) { return vertex_value(g, v) <= 45; });

  REQUIRE(count == 2); // Vertices with values 30 and 40
}

TEST_CASE("count_if - filter edges then count (dofl)", "[stl][6.2.3][count_if][filter]") {
  dofl_int_edges g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}});

  auto v = *vertices(g).begin();

  // Filter edges with value >= 15, then count those to targets >= 2
  auto filtered = edges(g, v) | std::views::filter([&g](auto&& e) { return edge_value(g, e) >= 15; });

  auto count = std::ranges::count_if(filtered, [&g](auto&& e) { return target_id(g, e) >= 2; });

  REQUIRE(count == 3); // Edges to vertices 2 (value 20), 3 (value 30), and 4 (value 40)
}

TEST_CASE("count_if - count using views::transform (vov)", "[stl][6.2.3][count_if][transform]") {
  vov_void g({{0, 1}, {0, 2}, {0, 5}, {0, 10}});

  auto v = *vertices(g).begin();

  // Transform edges to target IDs, then count those >= 5
  auto targets = edges(g, v) | std::views::transform([&g](auto&& e) { return target_id(g, e); });

  auto count = std::ranges::count_if(targets, [](auto tid) { return tid >= 5; });

  REQUIRE(count == 2); // Target IDs 5 and 10
}

TEST_CASE("count_if - count in both directions (vov)", "[stl][6.2.3][count_if]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 20}, {1, 0, 15}, {1, 2, 25}});

  // Count all edges in graph with value >= 15
  size_t total_count = 0;
  for (auto&& v : vertices(g)) {
    total_count +=
          static_cast<size_t>(std::ranges::count_if(edges(g, v), [&g](auto&& e) { return edge_value(g, e) >= 15; }));
  }

  REQUIRE(total_count == 3); // Edges with values 20, 15, and 25
}

TEST_CASE("count_if - count edges between specific vertex range (mos)", "[stl][6.2.3][count_if]") {
  mos_void g({{"A", "B"}, {"A", "E"}, {"B", "C"}, {"B", "D"}});

  // Count all edges from any vertex to targets in range ["B", "D"]
  size_t count = 0;
  for (auto&& v : vertices(g)) {
    count += static_cast<size_t>(std::ranges::count_if(edges(g, v), [&g](auto&& e) {
      auto tid = target_id(g, e);
      return tid >= "B" && tid <= "D";
    }));
  }

  REQUIRE(count == 3); // A->B, B->C, B->D
}

TEST_CASE("count_if - count vertices with even IDs (vov)", "[stl][6.2.3][count_if]") {
  vov_void g({{0, 1}, {1, 2}, {2, 3}});
  g.resize_vertices(10);

  auto count = std::ranges::count_if(vertices(g), [&g](auto&& v) { return vertex_id(g, v) % 2 == 0; });

  REQUIRE(count == 5); // Vertices 0, 2, 4, 6, 8
}

TEST_CASE("count_if - count edges with target divisible by 3 (dofl)", "[stl][6.2.3][count_if]") {
  dofl_void g({{0, 1}, {0, 3}, {0, 6}, {0, 7}, {0, 9}});

  auto v = *vertices(g).begin();

  auto count = std::ranges::count_if(edges(g, v), [&g](auto&& e) { return target_id(g, e) % 3 == 0; });

  REQUIRE(count == 3); // Targets 3, 6, and 9
}

// ============================================================================
// Phase 6.2.4: std::ranges::transform Tests
// ============================================================================

TEST_CASE("transform - extract vertex IDs to vector (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {0, 2}, {1, 2}});

  std::vector<size_t> ids;
  std::ranges::transform(vertices(g), std::back_inserter(ids), [&g](auto&& v) { return vertex_id(g, v); });

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == 0);
  REQUIRE(ids[1] == 1);
  REQUIRE(ids[2] == 2);
}

TEST_CASE("transform - extract edge target IDs to vector (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}});

  auto                v = *vertices(g).begin();
  std::vector<size_t> targets;
  std::ranges::transform(edges(g, v), std::back_inserter(targets), [&g](auto&& e) { return target_id(g, e); });

  REQUIRE(targets.size() == 3);
  REQUIRE(targets[0] == 1);
  REQUIRE(targets[1] == 2);
  REQUIRE(targets[2] == 3);
}

TEST_CASE("transform - double vertex values (vov)", "[stl][6.2.4][transform]") {
  vov_int_verts g({{0, 1}, {1, 2}});
  g.resize_vertices(3);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 30;

  std::vector<int> doubled;
  std::ranges::transform(vertices(g), std::back_inserter(doubled), [&g](auto&& v) { return vertex_value(g, v) * 2; });

  REQUIRE(doubled.size() == 3);
  REQUIRE(doubled[0] == 20);
  REQUIRE(doubled[1] == 40);
  REQUIRE(doubled[2] == 60);
}

TEST_CASE("transform - increment edge values (dofl)", "[stl][6.2.4][transform]") {
  dofl_int_edges g({{0, 1, 100}, {0, 2, 200}, {0, 3, 300}});

  auto             v = *vertices(g).begin();
  std::vector<int> incremented;
  std::ranges::transform(edges(g, v), std::back_inserter(incremented), [&g](auto&& e) { return edge_value(g, e) + 1; });

  std::ranges::sort(incremented);
  REQUIRE(incremented.size() == 3);
  REQUIRE(incremented[0] == 101);
  REQUIRE(incremented[1] == 201);
  REQUIRE(incremented[2] == 301);
}

TEST_CASE("transform - vertex IDs to strings (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {0, 2}, {1, 2}});

  std::vector<std::string> id_strings;
  std::ranges::transform(vertices(g), std::back_inserter(id_strings),
                         [&g](auto&& v) { return "v" + std::to_string(vertex_id(g, v)); });

  REQUIRE(id_strings.size() == 3);
  REQUIRE(id_strings[0] == "v0");
  REQUIRE(id_strings[1] == "v1");
  REQUIRE(id_strings[2] == "v2");
}

TEST_CASE("transform - compute edge weights as double (vov)", "[stl][6.2.4][transform]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}});

  auto                v = *vertices(g).begin();
  std::vector<double> weights;
  std::ranges::transform(edges(g, v), std::back_inserter(weights),
                         [&g](auto&& e) { return static_cast<double>(edge_value(g, e)) / 10.0; });

  REQUIRE(weights.size() == 3);
  REQUIRE(weights[0] == 1.0);
  REQUIRE(weights[1] == 2.0);
  REQUIRE(weights[2] == 3.0);
}

TEST_CASE("transform - filter then extract IDs (vov)", "[stl][6.2.4][transform][filter]") {
  vov_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}});

  auto v        = *vertices(g).begin();
  auto filtered = edges(g, v) | std::views::filter([&g](auto&& e) { return target_id(g, e) % 2 == 0; });

  std::vector<size_t> even_targets;
  std::ranges::transform(filtered, std::back_inserter(even_targets), [&g](auto&& e) { return target_id(g, e); });

  REQUIRE(even_targets.size() == 2);
  REQUIRE(even_targets[0] == 2);
  REQUIRE(even_targets[1] == 4);
}

TEST_CASE("transform - vertex degree to vector (dofl)", "[stl][6.2.4][transform]") {
  dofl_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}});

  std::vector<size_t> degrees;
  std::ranges::transform(vertices(g), std::back_inserter(degrees),
                         [&g](auto&& v) { return std::ranges::distance(edges(g, v)); });

  REQUIRE(degrees.size() == 4);
  REQUIRE(degrees[0] == 2); // Vertex 0 has 2 edges
  REQUIRE(degrees[1] == 2); // Vertex 1 has 2 edges
  REQUIRE(degrees[2] == 0); // Vertex 2 has 0 edges
  REQUIRE(degrees[3] == 0); // Vertex 3 has 0 edges
}

TEST_CASE("transform - combine vertex ID and value (vov)", "[stl][6.2.4][transform]") {
  vov_int_verts g({{0, 1}, {1, 2}});
  g.resize_vertices(3);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 100;
  vertex_value(g, *it++) = 200;
  vertex_value(g, *it++) = 300;

  std::vector<std::pair<size_t, int>> id_value_pairs;
  std::ranges::transform(vertices(g), std::back_inserter(id_value_pairs),
                         [&g](auto&& v) { return std::make_pair(vertex_id(g, v), vertex_value(g, v)); });

  REQUIRE(id_value_pairs.size() == 3);
  REQUIRE(id_value_pairs[0].first == 0);
  REQUIRE(id_value_pairs[0].second == 100);
  REQUIRE(id_value_pairs[1].first == 1);
  REQUIRE(id_value_pairs[1].second == 200);
  REQUIRE(id_value_pairs[2].first == 2);
  REQUIRE(id_value_pairs[2].second == 300);
}

TEST_CASE("transform - edge source and target pairs (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {0, 2}, {1, 2}});

  auto                                   v = *vertices(g).begin();
  std::vector<std::pair<size_t, size_t>> edge_pairs;
  std::ranges::transform(edges(g, v), std::back_inserter(edge_pairs),
                         [&g, v_id = vertex_id(g, v)](auto&& e) { return std::make_pair(v_id, target_id(g, e)); });

  REQUIRE(edge_pairs.size() == 2);
  REQUIRE(edge_pairs[0].first == 0);
  REQUIRE(edge_pairs[0].second == 1);
  REQUIRE(edge_pairs[1].first == 0);
  REQUIRE(edge_pairs[1].second == 2);
}

TEST_CASE("transform - square vertex values (vov)", "[stl][6.2.4][transform]") {
  vov_int_verts g({{0, 1}, {1, 2}});
  g.resize_vertices(4);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 2;
  vertex_value(g, *it++) = 3;
  vertex_value(g, *it++) = 4;
  vertex_value(g, *it++) = 5;

  std::vector<int> squared;
  std::ranges::transform(vertices(g), std::back_inserter(squared), [&g](auto&& v) {
    int val = vertex_value(g, v);
    return val * val;
  });

  REQUIRE(squared.size() == 4);
  REQUIRE(squared[0] == 4);
  REQUIRE(squared[1] == 9);
  REQUIRE(squared[2] == 16);
  REQUIRE(squared[3] == 25);
}

TEST_CASE("transform - negate edge values (dofl)", "[stl][6.2.4][transform]") {
  dofl_int_edges g({{0, 1, 10}, {0, 2, -20}, {0, 3, 30}});

  auto             v = *vertices(g).begin();
  std::vector<int> negated;
  std::ranges::transform(edges(g, v), std::back_inserter(negated), [&g](auto&& e) { return -edge_value(g, e); });

  std::ranges::sort(negated);
  REQUIRE(negated.size() == 3);
  REQUIRE(negated[0] == -30);
  REQUIRE(negated[1] == -10);
  REQUIRE(negated[2] == 20);
}

TEST_CASE("transform - vertices to booleans (even IDs) (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

  std::vector<bool> is_even;
  std::ranges::transform(vertices(g), std::back_inserter(is_even), [&g](auto&& v) { return vertex_id(g, v) % 2 == 0; });

  REQUIRE(is_even.size() == 5);
  REQUIRE(is_even[0] == true);  // ID 0
  REQUIRE(is_even[1] == false); // ID 1
  REQUIRE(is_even[2] == true);  // ID 2
  REQUIRE(is_even[3] == false); // ID 3
  REQUIRE(is_even[4] == true);  // ID 4
}

TEST_CASE("transform - filter vertices then transform values (vov)", "[stl][6.2.4][transform][filter]") {
  vov_int_verts g({{0, 1}, {1, 2}, {2, 3}});
  g.resize_vertices(5);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 15;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 25;
  vertex_value(g, *it++) = 30;

  auto filtered = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) >= 20; });

  std::vector<int> doubled;
  std::ranges::transform(filtered, std::back_inserter(doubled), [&g](auto&& v) { return vertex_value(g, v) * 2; });

  REQUIRE(doubled.size() == 3);
  REQUIRE(doubled[0] == 40); // 20 * 2
  REQUIRE(doubled[1] == 50); // 25 * 2
  REQUIRE(doubled[2] == 60); // 30 * 2
}

TEST_CASE("transform - empty graph vertices (vov)", "[stl][6.2.4][transform]") {
  vov_void g;

  std::vector<size_t> ids;
  std::ranges::transform(vertices(g), std::back_inserter(ids), [&g](auto&& v) { return vertex_id(g, v); });

  REQUIRE(ids.empty());
}

TEST_CASE("transform - empty edge list (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}});
  g.resize_vertices(3);

  auto verts = vertices(g);
  auto it    = verts.begin();
  ++it;
  ++it; // Move to vertex 2

  std::vector<size_t> targets;
  std::ranges::transform(edges(g, *it), std::back_inserter(targets), [&g](auto&& e) { return target_id(g, e); });

  REQUIRE(targets.empty());
}

TEST_CASE("transform - compute vertex ID differences (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 3}, {0, 5}, {0, 7}});

  auto   v         = *vertices(g).begin();
  size_t source_id = vertex_id(g, v);

  std::vector<size_t> differences;
  std::ranges::transform(edges(g, v), std::back_inserter(differences),
                         [&g, source_id](auto&& e) { return target_id(g, e) - source_id; });

  REQUIRE(differences.size() == 3);
  REQUIRE(differences[0] == 3);
  REQUIRE(differences[1] == 5);
  REQUIRE(differences[2] == 7);
}

TEST_CASE("transform - accumulate edge values per vertex (vov)", "[stl][6.2.4][transform]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});

  std::vector<int> sums;
  std::ranges::transform(vertices(g), std::back_inserter(sums), [&g](auto&& v) {
    int sum = 0;
    for (auto&& e : edges(g, v)) {
      sum += edge_value(g, e);
    }
    return sum;
  });

  REQUIRE(sums.size() == 3);
  REQUIRE(sums[0] == 30); // 10 + 20
  REQUIRE(sums[1] == 30); // 30
  REQUIRE(sums[2] == 0);  // No outgoing edges
}

TEST_CASE("transform - chain filter and transform with views (dofl)", "[stl][6.2.4][transform][filter]") {
  dofl_int_edges g({{0, 1, 5}, {0, 2, 10}, {0, 3, 15}, {0, 4, 20}, {0, 5, 25}});

  auto v      = *vertices(g).begin();
  auto result = edges(g, v) | std::views::filter([&g](auto&& e) { return edge_value(g, e) >= 10; }) |
                std::views::transform([&g](auto&& e) { return edge_value(g, e) / 5; });

  std::vector<int> transformed;
  std::ranges::copy(result, std::back_inserter(transformed));
  std::ranges::sort(transformed);

  REQUIRE(transformed.size() == 4);
  REQUIRE(transformed[0] == 2); // 10 / 5
  REQUIRE(transformed[1] == 3); // 15 / 5
  REQUIRE(transformed[2] == 4); // 20 / 5
  REQUIRE(transformed[3] == 5); // 25 / 5
}

TEST_CASE("transform - compute average edge value per vertex (vov)", "[stl][6.2.4][transform]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 40}});

  std::vector<double> averages;
  std::ranges::transform(vertices(g), std::back_inserter(averages), [&g](auto&& v) {
    size_t count = 0;
    int    sum   = 0;
    for (auto&& e : edges(g, v)) {
      sum += edge_value(g, e);
      ++count;
    }
    return count > 0 ? static_cast<double>(sum) / static_cast<double>(count) : 0.0;
  });

  REQUIRE(averages.size() == 4);
  REQUIRE(averages[0] == 20.0); // (10 + 20 + 30) / 3
  REQUIRE(averages[1] == 40.0); // 40 / 1
  REQUIRE(averages[2] == 0.0);  // No edges
  REQUIRE(averages[3] == 0.0);  // No edges
}

TEST_CASE("transform - create edge descriptors with values (dofl)", "[stl][6.2.4][transform]") {
  dofl_int_edges g({{0, 1, 100}, {0, 2, 200}});

  struct EdgeInfo {
    size_t target;
    int    value;
  };

  auto                  v = *vertices(g).begin();
  std::vector<EdgeInfo> edge_infos;
  std::ranges::transform(edges(g, v), std::back_inserter(edge_infos),
                         [&g](auto&& e) { return EdgeInfo{target_id(g, e), edge_value(g, e)}; });

  std::ranges::sort(edge_infos, {}, &EdgeInfo::target);
  REQUIRE(edge_infos.size() == 2);
  REQUIRE(edge_infos[0].target == 1);
  REQUIRE(edge_infos[0].value == 100);
  REQUIRE(edge_infos[1].target == 2);
  REQUIRE(edge_infos[1].value == 200);
}

TEST_CASE("transform - vertex values with conditional (vov)", "[stl][6.2.4][transform]") {
  vov_int_verts g({{0, 1}, {1, 2}});
  g.resize_vertices(5);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 5;
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 15;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 25;

  std::vector<int> transformed;
  std::ranges::transform(vertices(g), std::back_inserter(transformed), [&g](auto&& v) {
    int val = vertex_value(g, v);
    return val < 15 ? val * 2 : val / 5;
  });

  REQUIRE(transformed.size() == 5);
  REQUIRE(transformed[0] == 10); // 5 * 2
  REQUIRE(transformed[1] == 20); // 10 * 2
  REQUIRE(transformed[2] == 3);  // 15 / 5
  REQUIRE(transformed[3] == 4);  // 20 / 5
  REQUIRE(transformed[4] == 5);  // 25 / 5
}

TEST_CASE("transform - check if edges exist to specific targets (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {0, 3}, {0, 5}});

  auto              v = *vertices(g).begin();
  std::vector<bool> target_checks;

  for (size_t target : {1UL, 2UL, 3UL, 4UL, 5UL}) {
    auto found = std::ranges::any_of(edges(g, v),
                                     [&g, target](auto&& e) { return target_id(g, e) == static_cast<size_t>(target); });
    target_checks.push_back(found);
  }

  REQUIRE(target_checks.size() == 5);
  REQUIRE(target_checks[0] == true);  // Target 1 exists
  REQUIRE(target_checks[1] == false); // Target 2 doesn't exist
  REQUIRE(target_checks[2] == true);  // Target 3 exists
  REQUIRE(target_checks[3] == false); // Target 4 doesn't exist
  REQUIRE(target_checks[4] == true);  // Target 5 exists
}

TEST_CASE("transform - multiple transformations in sequence (vov)", "[stl][6.2.4][transform]") {
  vov_int_verts g({{0, 1}, {1, 2}});
  g.resize_vertices(3);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 10;
  vertex_value(g, *it++) = 20;
  vertex_value(g, *it++) = 30;

  // First transformation: double values
  std::vector<int> doubled;
  std::ranges::transform(vertices(g), std::back_inserter(doubled), [&g](auto&& v) { return vertex_value(g, v) * 2; });

  // Second transformation: add 5 to doubled values
  std::vector<int> final;
  std::ranges::transform(doubled, std::back_inserter(final), [](int val) { return val + 5; });

  REQUIRE(final.size() == 3);
  REQUIRE(final[0] == 25); // (10 * 2) + 5
  REQUIRE(final[1] == 45); // (20 * 2) + 5
  REQUIRE(final[2] == 65); // (30 * 2) + 5
}

TEST_CASE("transform - extract max edge value per vertex (vov)", "[stl][6.2.4][transform]") {
  vov_int_edges g({{0, 1, 10}, {0, 2, 50}, {0, 3, 30}, {1, 2, 100}});

  std::vector<int> max_values;
  std::ranges::transform(vertices(g), std::back_inserter(max_values), [&g](auto&& v) {
    int  max_val   = std::numeric_limits<int>::min();
    bool has_edges = false;
    for (auto&& e : edges(g, v)) {
      max_val   = std::max(max_val, edge_value(g, e));
      has_edges = true;
    }
    return has_edges ? max_val : 0;
  });

  REQUIRE(max_values.size() == 4);
  REQUIRE(max_values[0] == 50);  // Max of 10, 50, 30
  REQUIRE(max_values[1] == 100); // Only 100
  REQUIRE(max_values[2] == 0);   // No edges
  REQUIRE(max_values[3] == 0);   // No edges
}

TEST_CASE("transform - concatenate vertex ID with value string (vov)", "[stl][6.2.4][transform]") {
  vov_int_verts g({{0, 1}});
  g.resize_vertices(3);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 100;
  vertex_value(g, *it++) = 200;
  vertex_value(g, *it++) = 300;

  std::vector<std::string> labels;
  std::ranges::transform(vertices(g), std::back_inserter(labels), [&g](auto&& v) {
    return "V" + std::to_string(vertex_id(g, v)) + ":" + std::to_string(vertex_value(g, v));
  });

  REQUIRE(labels.size() == 3);
  REQUIRE(labels[0] == "V0:100");
  REQUIRE(labels[1] == "V1:200");
  REQUIRE(labels[2] == "V2:300");
}

TEST_CASE("transform - compute out-degree minus in-degree (vov)", "[stl][6.2.4][transform]") {
  vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});

  std::vector<int> degree_diff;
  std::ranges::transform(vertices(g), std::back_inserter(degree_diff), [&g](auto&& v) {
    size_t out_degree = static_cast<size_t>(std::ranges::distance(edges(g, v)));

    // Count in-degree by checking all edges
    size_t in_degree = 0;
    size_t v_id      = vertex_id(g, v);
    for (auto&& u : vertices(g)) {
      for (auto&& e : edges(g, u)) {
        if (target_id(g, e) == v_id) {
          ++in_degree;
        }
      }
    }

    return static_cast<int>(out_degree) - static_cast<int>(in_degree);
  });

  REQUIRE(degree_diff.size() == 3);
  REQUIRE(degree_diff[0] == 1);  // out=2, in=1 -> 2-1=1
  REQUIRE(degree_diff[1] == 0);  // out=1, in=1 -> 1-1=0
  REQUIRE(degree_diff[2] == -1); // out=1, in=2 -> 1-2=-1 (edges: 0->2, 1->2)
}

TEST_CASE("transform - filter edges by value range then transform (dofl)", "[stl][6.2.4][transform][filter]") {
  dofl_int_edges g({{0, 1, 5}, {0, 2, 15}, {0, 3, 25}, {0, 4, 35}});

  auto v        = *vertices(g).begin();
  auto filtered = edges(g, v) | std::views::filter([&g](auto&& e) {
                    int val = edge_value(g, e);
                    return val >= 10 && val <= 30;
                  });

  std::vector<std::pair<size_t, int>> results;
  std::ranges::transform(filtered, std::back_inserter(results),
                         [&g](auto&& e) { return std::make_pair(target_id(g, e), edge_value(g, e) + 100); });

  std::ranges::sort(results);
  REQUIRE(results.size() == 2);
  REQUIRE(results[0].first == 2);
  REQUIRE(results[0].second == 115); // 15 + 100
  REQUIRE(results[1].first == 3);
  REQUIRE(results[1].second == 125); // 25 + 100
}

TEST_CASE("transform - map vertices with complex struct (vov)", "[stl][6.2.4][transform]") {
  vov_int_both g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});
  g.resize_vertices(3);

  auto verts             = vertices(g);
  auto it                = verts.begin();
  vertex_value(g, *it++) = 100;
  vertex_value(g, *it++) = 200;
  vertex_value(g, *it++) = 300;

  struct VertexData {
    size_t id;
    int    value;
    size_t degree;
  };

  std::vector<VertexData> vertex_data;
  std::ranges::transform(vertices(g), std::back_inserter(vertex_data), [&g](auto&& v) {
    return VertexData{vertex_id(g, v), vertex_value(g, v), static_cast<size_t>(std::ranges::distance(edges(g, v)))};
  });

  REQUIRE(vertex_data.size() == 3);
  REQUIRE(vertex_data[0].id == 0);
  REQUIRE(vertex_data[0].value == 100);
  REQUIRE(vertex_data[0].degree == 2);
  REQUIRE(vertex_data[1].id == 1);
  REQUIRE(vertex_data[1].value == 200);
  REQUIRE(vertex_data[1].degree == 1);
  REQUIRE(vertex_data[2].id == 2);
  REQUIRE(vertex_data[2].value == 300);
  REQUIRE(vertex_data[2].degree == 0);
}
//=============================================================================
// Phase 6.2.5: std::ranges::sort (where applicable)
// Tests sorting of edges by target ID and vertices by value
// Note: Only works with random access containers (vov, vod, mov, mod)
// forward_list and list-based containers don't support random access
//=============================================================================

TEST_CASE("sort edge target IDs (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g({{0, 2}, {0, 1}, {0, 3}});

  auto                        edge_range = edges(g, 0);
  std::vector<vertex_id_t<G>> targets;
  std::ranges::transform(edge_range, std::back_inserter(targets), [&g](auto&& e) { return target_id(g, e); });

  std::ranges::sort(targets);

  REQUIRE(targets.size() == 3);
  REQUIRE(targets[0] == 1);
  REQUIRE(targets[1] == 2);
  REQUIRE(targets[2] == 3);
}

TEST_CASE("sort edge target IDs in descending order (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g({{0, 2}, {0, 1}, {0, 3}});

  auto                        edge_range = edges(g, 0);
  std::vector<vertex_id_t<G>> targets;
  std::ranges::transform(edge_range, std::back_inserter(targets), [&g](auto&& e) { return target_id(g, e); });

  std::ranges::sort(targets, std::greater<>());

  REQUIRE(targets.size() == 3);
  REQUIRE(targets[0] == 3);
  REQUIRE(targets[1] == 2);
  REQUIRE(targets[2] == 1);
}

TEST_CASE("sort edge values (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 30}, {0, 2, 10}, {0, 3, 20}});

  auto             edge_range = edges(g, 0);
  std::vector<int> values;
  std::ranges::transform(edge_range, std::back_inserter(values), [&g](auto&& e) { return edge_value(g, e); });

  std::ranges::sort(values);

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 10);
  REQUIRE(values[1] == 20);
  REQUIRE(values[2] == 30);
}

TEST_CASE("sort vertex IDs (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g({{2, 0}, {1, 2}, {0, 1}});

  std::vector<vertex_id_t<G>> ids;
  std::ranges::transform(vertices(g), std::back_inserter(ids), [&g](auto&& v) { return vertex_id(g, v); });

  std::ranges::sort(ids);

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == 0);
  REQUIRE(ids[1] == 1);
  REQUIRE(ids[2] == 2);
}

TEST_CASE("sort vertex values (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 300 : (id == 1) ? 100 : 200;
  }

  std::vector<int> values;
  std::ranges::transform(vertices(g), std::back_inserter(values), [&g](auto&& v) { return vertex_value(g, v); });

  std::ranges::sort(values);

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 100);
  REQUIRE(values[1] == 200);
  REQUIRE(values[2] == 300);
}

TEST_CASE("sort by vertex value with projection (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 300 : (id == 1) ? 100 : 200;
  }

  struct VertexInfo {
    vertex_id_t<G> id;
    int            value;
  };

  std::vector<VertexInfo> infos;
  std::ranges::transform(vertices(g), std::back_inserter(infos),
                         [&g](auto&& v) { return VertexInfo{vertex_id(g, v), vertex_value(g, v)}; });

  // Sort by value using projection
  std::ranges::sort(infos, {}, &VertexInfo::value);

  REQUIRE(infos.size() == 3);
  REQUIRE(infos[0].id == 1);
  REQUIRE(infos[0].value == 100);
  REQUIRE(infos[1].id == 2);
  REQUIRE(infos[1].value == 200);
  REQUIRE(infos[2].id == 0);
  REQUIRE(infos[2].value == 300);
}

TEST_CASE("sort edge infos by target then by value (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 50}, {0, 2, 30}, {0, 2, 10}, {0, 1, 20}});

  struct EdgeInfo {
    vertex_id_t<G> target;
    int            value;

    auto operator<=>(const EdgeInfo&) const = default;
  };

  auto                  edge_range = edges(g, 0);
  std::vector<EdgeInfo> infos;
  std::ranges::transform(edge_range, std::back_inserter(infos),
                         [&g](auto&& e) { return EdgeInfo{target_id(g, e), edge_value(g, e)}; });

  std::ranges::sort(infos);

  REQUIRE(infos.size() == 4);
  REQUIRE(infos[0].target == 1);
  REQUIRE(infos[0].value == 20);
  REQUIRE(infos[1].target == 1);
  REQUIRE(infos[1].value == 50);
  REQUIRE(infos[2].target == 2);
  REQUIRE(infos[2].value == 10);
  REQUIRE(infos[3].target == 2);
  REQUIRE(infos[3].value == 30);
}

TEST_CASE("sort degrees in ascending order (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}});

  std::vector<size_t> degrees;
  std::ranges::transform(vertices(g), std::back_inserter(degrees),
                         [&g](auto&& v) { return std::ranges::distance(edges(g, v)); });

  std::ranges::sort(degrees);

  REQUIRE(degrees.size() == 3);
  REQUIRE(degrees[0] == 0); // vertex 2 has out-degree 0
  REQUIRE(degrees[1] == 1); // vertex 1 has out-degree 1
  REQUIRE(degrees[2] == 2); // vertex 0 has out-degree 2
}

TEST_CASE("sort combined vertex ID and value pairs (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 300 : (id == 1) ? 100 : (id == 2) ? 200 : 100;
  }

  std::vector<std::pair<vertex_id_t<G>, int>> pairs;
  std::ranges::transform(vertices(g), std::back_inserter(pairs),
                         [&g](auto&& v) { return std::make_pair(vertex_id(g, v), vertex_value(g, v)); });

  // Sort by value first, then by ID (default pair comparison)
  std::ranges::sort(pairs, [](const auto& a, const auto& b) {
    return a.second < b.second || (a.second == b.second && a.first < b.first);
  });

  REQUIRE(pairs.size() == 4);
  REQUIRE(pairs[0].first == 1);
  REQUIRE(pairs[0].second == 100);
  REQUIRE(pairs[1].first == 3);
  REQUIRE(pairs[1].second == 100);
  REQUIRE(pairs[2].first == 2);
  REQUIRE(pairs[2].second == 200);
  REQUIRE(pairs[3].first == 0);
  REQUIRE(pairs[3].second == 300);
}

TEST_CASE("sort on empty edge list (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g({{0, 1}});

  auto                        edge_range = edges(g, 1); // vertex 1 has no outgoing edges
  std::vector<vertex_id_t<G>> targets;
  std::ranges::transform(edge_range, std::back_inserter(targets), [&g](auto&& e) { return target_id(g, e); });

  std::ranges::sort(targets);

  REQUIRE(targets.empty());
}

TEST_CASE("sort on empty graph (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g;

  std::vector<vertex_id_t<G>> ids;
  std::ranges::transform(vertices(g), std::back_inserter(ids), [&g](auto&& v) { return vertex_id(g, v); });

  std::ranges::sort(ids);

  REQUIRE(ids.empty());
}

TEST_CASE("stable_sort preserves relative order of equal elements (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 100}, {0, 2, 200}, {0, 3, 100}, {0, 4, 300}, {0, 5, 100}});

  struct EdgeData {
    size_t         insertion_order;
    vertex_id_t<G> target;
    int            value;
  };

  auto                  edge_range = edges(g, 0);
  std::vector<EdgeData> data;
  size_t                order = 0;
  std::ranges::transform(edge_range, std::back_inserter(data),
                         [&g, &order](auto&& e) { return EdgeData{order++, target_id(g, e), edge_value(g, e)}; });

  // Stable sort by value - edges with same value maintain insertion order
  std::ranges::stable_sort(data, {}, &EdgeData::value);

  REQUIRE(data.size() == 5);
  // All edges with value 100 should appear first, in original order
  REQUIRE(data[0].value == 100);
  REQUIRE(data[0].target == 1);
  REQUIRE(data[0].insertion_order == 0);
  REQUIRE(data[1].value == 100);
  REQUIRE(data[1].target == 3);
  REQUIRE(data[1].insertion_order == 2);
  REQUIRE(data[2].value == 100);
  REQUIRE(data[2].target == 5);
  REQUIRE(data[2].insertion_order == 4);
  REQUIRE(data[3].value == 200);
  REQUIRE(data[4].value == 300);
}

TEST_CASE("sort with custom comparator (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? -10 : (id == 1) ? 20 : -5;
  }

  std::vector<int> values;
  std::ranges::transform(vertices(g), std::back_inserter(values), [&g](auto&& v) { return vertex_value(g, v); });

  // Sort by absolute value
  std::ranges::sort(values, [](int a, int b) { return std::abs(a) < std::abs(b); });

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == -5);  // |−5| = 5
  REQUIRE(values[1] == -10); // |−10| = 10
  REQUIRE(values[2] == 20);  // |20| = 20
}

TEST_CASE("sort edge values then reverse (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 30}, {0, 2, 10}, {0, 3, 20}});

  auto             edge_range = edges(g, 0);
  std::vector<int> values;
  std::ranges::transform(edge_range, std::back_inserter(values), [&g](auto&& e) { return edge_value(g, e); });

  std::ranges::sort(values);
  std::ranges::reverse(values);

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 30);
  REQUIRE(values[1] == 20);
  REQUIRE(values[2] == 10);
}

TEST_CASE("partial_sort to get top N edge values (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 50}, {0, 2, 30}, {0, 3, 70}, {0, 4, 10}, {0, 5, 90}});

  auto             edge_range = edges(g, 0);
  std::vector<int> values;
  std::ranges::transform(edge_range, std::back_inserter(values), [&g](auto&& e) { return edge_value(g, e); });

  // Get top 3 largest values using partial_sort
  std::ranges::partial_sort(values, values.begin() + 3, std::greater<>());

  REQUIRE(values.size() == 5);
  REQUIRE(values[0] == 90);
  REQUIRE(values[1] == 70);
  REQUIRE(values[2] == 50);
  // Remaining elements are in unspecified order
}

TEST_CASE("nth_element to find median edge value (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 50}, {0, 2, 30}, {0, 3, 70}, {0, 4, 10}, {0, 5, 90}});

  auto             edge_range = edges(g, 0);
  std::vector<int> values;
  std::ranges::transform(edge_range, std::back_inserter(values), [&g](auto&& e) { return edge_value(g, e); });

  // Find median using nth_element (middle element when sorted)
  size_t mid = values.size() / 2;
  std::ranges::nth_element(values, values.begin() + static_cast<std::ptrdiff_t>(mid));

  REQUIRE(values.size() == 5);
  REQUIRE(values[mid] == 50); // median of {10, 30, 50, 70, 90}
}

TEST_CASE("is_sorted check on vertex IDs (vov)", "[stl][6.2.5][sort]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}, {2, 0}});

  std::vector<vertex_id_t<G>> ids;
  std::ranges::transform(vertices(g), std::back_inserter(ids), [&g](auto&& v) { return vertex_id(g, v); });

  REQUIRE(std::ranges::is_sorted(ids)); // Vertex IDs are naturally sorted
}

TEST_CASE("is_sorted check on unsorted edge values (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_edges;
  G g({{0, 1, 30}, {0, 2, 10}, {0, 3, 20}});

  auto             edge_range = edges(g, 0);
  std::vector<int> values;
  std::ranges::transform(edge_range, std::back_inserter(values), [&g](auto&& e) { return edge_value(g, e); });

  REQUIRE_FALSE(std::ranges::is_sorted(values)); // {30, 10, 20} not sorted

  std::ranges::sort(values);

  REQUIRE(std::ranges::is_sorted(values)); // {10, 20, 30} is sorted
}

TEST_CASE("sort with both vertex and edge values (vov)", "[stl][6.2.5][sort]") {
  using G = vov_int_both;
  G g({{0, 1, 50}, {0, 2, 30}, {1, 2, 40}});

  // Set vertex values
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = (vertex_id(g, v) == 0) ? 100 : (vertex_id(g, v) == 1) ? 200 : 150;
  }

  struct GraphData {
    uint64_t vid;
    int      vval;
    uint64_t target;
    int      eval;
  };

  std::vector<GraphData> data;
  for (auto&& v : vertices(g)) {
    for (auto&& e : edges(g, v)) {
      data.push_back(GraphData{vertex_id(g, v), vertex_value(g, v), target_id(g, e), edge_value(g, e)});
    }
  }

  // Sort by source vertex value, then by edge value
  std::ranges::sort(
        data, [](const auto& a, const auto& b) { return a.vval < b.vval || (a.vval == b.vval && a.eval < b.eval); });

  REQUIRE(data.size() == 3);
  REQUIRE(data[0].vid == 0);
  REQUIRE(data[0].vval == 100);
  REQUIRE(data[0].eval == 30);
  REQUIRE(data[1].vid == 0);
  REQUIRE(data[1].vval == 100);
  REQUIRE(data[1].eval == 50);
  REQUIRE(data[2].vid == 1);
  REQUIRE(data[2].vval == 200);
  REQUIRE(data[2].eval == 40);
}

//==================================================================================================
// Phase 6.2.6: Range Adaptors and Views
//==================================================================================================

TEST_CASE("views::filter on vertices by value (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 100 : (id == 1) ? 200 : 300;
  }

  auto filtered = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) >= 200; });

  size_t count = 0;
  for (auto&& v : filtered) {
    REQUIRE(vertex_value(g, v) >= 200);
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("views::filter on edges by value (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 50}, {1, 2, 30}});

  auto v0       = *find_vertex(g, 0);
  auto filtered = edges(g, v0) | std::views::filter([&g](auto&& e) { return edge_value(g, e) > 20; });

  size_t count = 0;
  for (auto&& e : filtered) {
    REQUIRE(edge_value(g, e) > 20);
    ++count;
  }
  REQUIRE(count == 1);
  REQUIRE(std::ranges::distance(filtered) == 1);
}

TEST_CASE("views::filter by degree (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {3, 4}});

  auto high_degree =
        vertices(g) | std::views::filter([&g](auto&& v) { return std::ranges::distance(edges(g, v)) >= 2; });

  std::vector<uint64_t> ids;
  for (auto&& v : high_degree) {
    ids.push_back(vertex_id(g, v));
  }

  REQUIRE(ids.size() == 1);
  REQUIRE(ids[0] == 0);
}

TEST_CASE("views::transform on vertices to extract IDs (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}, {2, 3}});

  auto ids = vertices(g) | std::views::transform([&g](auto&& v) { return vertex_id(g, v); });

  std::vector<uint64_t> id_vec(ids.begin(), ids.end());

  REQUIRE(id_vec.size() == 4);
  REQUIRE(id_vec[0] == 0);
  REQUIRE(id_vec[1] == 1);
  REQUIRE(id_vec[2] == 2);
  REQUIRE(id_vec[3] == 3);
}

TEST_CASE("views::transform on edges to extract target IDs (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {0, 3}});

  auto v0      = *find_vertex(g, 0);
  auto targets = edges(g, v0) | std::views::transform([&g](auto&& e) { return target_id(g, e); });

  std::vector<uint64_t> target_vec(targets.begin(), targets.end());
  std::ranges::sort(target_vec);

  REQUIRE(target_vec.size() == 3);
  REQUIRE(target_vec[0] == 1);
  REQUIRE(target_vec[1] == 2);
  REQUIRE(target_vec[2] == 3);
}

TEST_CASE("views::transform on vertex values (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = static_cast<int>(id) * 10;
  }

  auto doubled = vertices(g) | std::views::transform([&g](auto&& v) { return vertex_value(g, v) * 2; });

  std::vector<int> values(doubled.begin(), doubled.end());

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 0);
  REQUIRE(values[1] == 20);
  REQUIRE(values[2] == 40);
}

TEST_CASE("views::transform on edge values (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 20}});

  auto v0     = *find_vertex(g, 0);
  auto scaled = edges(g, v0) | std::views::transform([&g](auto&& e) { return edge_value(g, e) * 3; });

  std::vector<int> values(scaled.begin(), scaled.end());
  std::ranges::sort(values);

  REQUIRE(values.size() == 2);
  REQUIRE(values[0] == 30);
  REQUIRE(values[1] == 60);
}

TEST_CASE("views::take on vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}});

  auto first_three = vertices(g) | std::views::take(3);

  size_t count = 0;
  for (auto&& v : first_three) {
    REQUIRE(vertex_id(g, v) < 3);
    ++count;
  }
  REQUIRE(count == 3);
}

TEST_CASE("views::take on edges (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});

  auto v0        = *find_vertex(g, 0);
  auto first_two = edges(g, v0) | std::views::take(2);

  REQUIRE(std::ranges::distance(first_two) == 2);
}

TEST_CASE("views::take more than available (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}});

  auto many = vertices(g) | std::views::take(100);

  REQUIRE(std::ranges::distance(many) == 3);
}

TEST_CASE("views::drop on vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

  auto skip_first = vertices(g) | std::views::drop(2);

  std::vector<uint64_t> ids;
  for (auto&& v : skip_first) {
    ids.push_back(vertex_id(g, v));
  }

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == 2);
  REQUIRE(ids[1] == 3);
  REQUIRE(ids[2] == 4);
}

TEST_CASE("views::drop on edges (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {0, 3}});

  auto v0       = *find_vertex(g, 0);
  auto skip_one = edges(g, v0) | std::views::drop(1);

  REQUIRE(std::ranges::distance(skip_one) == 2);
}

TEST_CASE("views::drop more than available (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}});

  auto skip_many = vertices(g) | std::views::drop(10);

  REQUIRE(std::ranges::distance(skip_many) == 0);
}

TEST_CASE("views::reverse on vector (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}, {2, 3}});

  // Extract IDs to vector first, then reverse the vector
  std::vector<uint64_t> ids;
  for (auto&& v : vertices(g)) {
    ids.push_back(vertex_id(g, v));
  }

  auto                  reversed = ids | std::views::reverse;
  std::vector<uint64_t> reversed_ids(reversed.begin(), reversed.end());

  REQUIRE(reversed_ids.size() == 4);
  REQUIRE(reversed_ids[0] == 3);
  REQUIRE(reversed_ids[1] == 2);
  REQUIRE(reversed_ids[2] == 1);
  REQUIRE(reversed_ids[3] == 0);
}

TEST_CASE("pipeline: filter then transform vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 100 : (id == 1) ? 200 : (id == 2) ? 150 : 300;
  }

  auto result = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) >= 150; }) |
                std::views::transform([&g](auto&& v) { return vertex_id(g, v); });

  std::vector<uint64_t> ids(result.begin(), result.end());
  std::ranges::sort(ids);

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == 1);
  REQUIRE(ids[1] == 2);
  REQUIRE(ids[2] == 3);
}

TEST_CASE("pipeline: filter then transform edges (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 50}, {0, 3, 30}, {0, 4, 60}});

  auto v0     = *find_vertex(g, 0);
  auto result = edges(g, v0) | std::views::filter([&g](auto&& e) { return edge_value(g, e) > 25; }) |
                std::views::transform([&g](auto&& e) { return target_id(g, e); });

  std::vector<uint64_t> targets(result.begin(), result.end());
  std::ranges::sort(targets);

  REQUIRE(targets.size() == 3);
  REQUIRE(targets[0] == 2);
  REQUIRE(targets[1] == 3);
  REQUIRE(targets[2] == 4);
}

TEST_CASE("pipeline: transform then take vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) * 10;
  }

  auto result =
        vertices(g) | std::views::transform([&g](auto&& v) { return vertex_value(g, v); }) | std::views::take(3);

  std::vector<int> values;
  for (auto val : result) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 0);
  REQUIRE(values[1] == 10);
  REQUIRE(values[2] == 20);
}

TEST_CASE("pipeline: drop then transform edges (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}});

  auto v0     = *find_vertex(g, 0);
  auto result = edges(g, v0) | std::views::drop(1) | std::views::transform([&g](auto&& e) { return edge_value(g, e); });

  std::vector<int> values(result.begin(), result.end());
  std::ranges::sort(values);

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 20);
  REQUIRE(values[1] == 30);
  REQUIRE(values[2] == 40);
}

TEST_CASE("pipeline: filter then take vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) % 2 == 0 ? 100 : 200;
  }

  auto result =
        vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) == 100; }) | std::views::take(2);

  size_t count = static_cast<size_t>(std::ranges::distance(result));
  REQUIRE(count == 2);
}

TEST_CASE("pipeline: take then filter vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 50 : (id == 1) ? 150 : 250;
  }

  auto result =
        vertices(g) | std::views::take(3) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) > 100; });

  std::vector<int> values;
  for (auto&& v : result) {
    values.push_back(vertex_value(g, v));
  }

  REQUIRE(values.size() == 2);
  REQUIRE(values[0] == 150);
  REQUIRE(values[1] == 250);
}

TEST_CASE("pipeline: three-stage filter-transform-take (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) * 10;
  }

  auto result = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) >= 20; }) |
                std::views::transform([&g](auto&& v) { return vertex_value(g, v) / 10; }) | std::views::take(3);

  std::vector<int> values;
  for (auto val : result) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 2);
  REQUIRE(values[1] == 3);
  REQUIRE(values[2] == 4);
}

TEST_CASE("views::filter on empty vertex range (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g;

  auto filtered = vertices(g) | std::views::filter([](auto&&) { return true; });

  REQUIRE(std::ranges::distance(filtered) == 0);
}

TEST_CASE("views::filter on empty edge range (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}});

  auto v1       = *find_vertex(g, 1);
  auto filtered = edges(g, v1) | std::views::filter([](auto&&) { return true; });

  REQUIRE(std::ranges::distance(filtered) == 0);
}

TEST_CASE("views::transform with complex lambda (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 20}, {1, 2, 15}});

  struct EdgeInfo {
    uint64_t source;
    uint64_t target;
    int      value;
    int      doubled;
  };

  auto v0     = *find_vertex(g, 0);
  auto result = edges(g, v0) | std::views::transform([&g, v0](auto&& e) {
                  return EdgeInfo{vertex_id(g, v0), target_id(g, e), edge_value(g, e), edge_value(g, e) * 2};
                });

  std::vector<EdgeInfo> infos(result.begin(), result.end());
  std::ranges::sort(infos, [](const auto& a, const auto& b) { return a.target < b.target; });

  REQUIRE(infos.size() == 2);
  REQUIRE(infos[0].source == 0);
  REQUIRE(infos[0].target == 1);
  REQUIRE(infos[0].value == 10);
  REQUIRE(infos[0].doubled == 20);
  REQUIRE(infos[1].target == 2);
  REQUIRE(infos[1].value == 20);
  REQUIRE(infos[1].doubled == 40);
}

TEST_CASE("views with map-based graph (mos)", "[stl][6.2.6][views]") {
  mos_void g({{{"a", "b"}, {"b", "c"}, {"c", "d"}}});

  auto filtered = vertices(g) | std::views::filter([&g](auto&& v) {
                    auto id = vertex_id(g, v);
                    return id != "b";
                  });

  std::vector<std::string> ids;
  for (auto&& v : filtered) {
    ids.push_back(vertex_id(g, v));
  }
  std::ranges::sort(ids);

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == "a");
  REQUIRE(ids[1] == "c");
  REQUIRE(ids[2] == "d");
}

TEST_CASE("views with deque-based graph (dofl)", "[stl][6.2.6][views]") {
  using G = dofl_void;
  G g({{0, 1}, {1, 2}, {2, 3}});

  auto transformed = vertices(g) | std::views::transform([&g](auto&& v) { return vertex_id(g, v) * 2; });

  std::vector<uint64_t> doubled(transformed.begin(), transformed.end());

  REQUIRE(doubled.size() == 4);
  REQUIRE(doubled[0] == 0);
  REQUIRE(doubled[1] == 2);
  REQUIRE(doubled[2] == 4);
  REQUIRE(doubled[3] == 6);
}

TEST_CASE("complex pipeline: filter-drop-transform-take (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) * 5;
  }

  auto result = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) >= 10; }) |
                std::views::drop(1) | std::views::transform([&g](auto&& v) { return vertex_value(g, v); }) |
                std::views::take(2);

  std::vector<int> values;
  for (auto val : result) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 2);
  REQUIRE(values[0] == 15);
  REQUIRE(values[1] == 20);
}

TEST_CASE("views::filter with all elements rejected (vov)", "[stl][6.2.6][views]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = 50;
  }

  auto filtered = vertices(g) | std::views::filter([&g](auto&& v) { return vertex_value(g, v) > 100; });

  REQUIRE(std::ranges::distance(filtered) == 0);
}

TEST_CASE("views::take(0) on vertices (vov)", "[stl][6.2.6][views]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}});

  auto none = vertices(g) | std::views::take(0);

  REQUIRE(std::ranges::distance(none) == 0);
}

//==================================================================================================
// Phase 6.2.7: Accumulate and Fold Operations
//==================================================================================================

TEST_CASE("accumulate sum of vertex values (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = static_cast<int>(id) * 10;
  }

  int sum = 0;
  for (auto&& v : vertices(g)) {
    sum += vertex_value(g, v);
  }

  REQUIRE(sum == 60); // 0 + 10 + 20 + 30
}

TEST_CASE("accumulate sum of edge values (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 20}, {1, 2, 15}, {2, 3, 25}});

  int sum = 0;
  for (auto&& v : vertices(g)) {
    for (auto&& e : edges(g, v)) {
      sum += edge_value(g, e);
    }
  }

  REQUIRE(sum == 70); // 10 + 20 + 15 + 25
}

TEST_CASE("accumulate with std::accumulate on vertex values (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) * 5;
  }

  std::vector<int> values;
  for (auto&& v : vertices(g)) {
    values.push_back(vertex_value(g, v));
  }

  int sum = std::accumulate(values.begin(), values.end(), 0);

  REQUIRE(sum == 15); // 0 + 5 + 10
}

TEST_CASE("accumulate with std::accumulate on edge values (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_edges;
  G g({{0, 1, 100}, {1, 2, 200}, {2, 3, 300}});

  std::vector<int> values;
  for (auto&& v : vertices(g)) {
    for (auto&& e : edges(g, v)) {
      values.push_back(edge_value(g, e));
    }
  }

  int sum = std::accumulate(values.begin(), values.end(), 0);

  REQUIRE(sum == 600);
}

TEST_CASE("count total out-degree (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}});

  size_t total_degree = 0;
  for (auto&& v : vertices(g)) {
    total_degree += static_cast<size_t>(std::ranges::distance(edges(g, v)));
  }

  REQUIRE(total_degree == 5); // 2 + 2 + 1 + 0
}

TEST_CASE("sum of all degrees equals edge count (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 0}});

  size_t edge_count = 0;
  for (auto&& v : vertices(g)) {
    edge_count += static_cast<size_t>(std::ranges::distance(edges(g, v)));
  }

  size_t total_degree = 0;
  for (auto&& v : vertices(g)) {
    total_degree += static_cast<size_t>(std::ranges::distance(edges(g, v)));
  }

  REQUIRE(total_degree == edge_count);
  REQUIRE(edge_count == 5);
}

TEST_CASE("find max degree vertex (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 3}});

  uint64_t max_degree_id = 0;
  size_t   max_degree    = 0;

  for (auto&& v : vertices(g)) {
    size_t degree = static_cast<size_t>(std::ranges::distance(edges(g, v)));
    if (degree > max_degree) {
      max_degree    = degree;
      max_degree_id = vertex_id(g, v);
    }
  }

  REQUIRE(max_degree_id == 0);
  REQUIRE(max_degree == 3);
}

TEST_CASE("find min degree vertex (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

  uint64_t min_degree_id = 0;
  size_t   min_degree    = std::numeric_limits<size_t>::max();

  for (auto&& v : vertices(g)) {
    size_t degree = static_cast<size_t>(std::ranges::distance(edges(g, v)));
    if (degree < min_degree) {
      min_degree    = degree;
      min_degree_id = vertex_id(g, v);
    }
  }

  REQUIRE(min_degree_id == 3);
  REQUIRE(min_degree == 0);
}

TEST_CASE("compute average degree (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

  size_t total_degree = 0;
  size_t vertex_count = 0;

  for (auto&& v : vertices(g)) {
    total_degree += static_cast<size_t>(std::ranges::distance(edges(g, v)));
    ++vertex_count;
  }

  double avg_degree = static_cast<double>(total_degree) / static_cast<double>(vertex_count);

  REQUIRE(vertex_count == 4);
  REQUIRE(total_degree == 4);
  REQUIRE(avg_degree == 1.0);
}

TEST_CASE("find vertex with max value (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 50 : (id == 1) ? 150 : (id == 2) ? 100 : 25;
  }

  uint64_t max_value_id = 0;
  int      max_value    = std::numeric_limits<int>::min();

  for (auto&& v : vertices(g)) {
    int val = vertex_value(g, v);
    if (val > max_value) {
      max_value    = val;
      max_value_id = vertex_id(g, v);
    }
  }

  REQUIRE(max_value_id == 1);
  REQUIRE(max_value == 150);
}

TEST_CASE("find edge with max value (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 50}, {1, 2, 30}, {2, 3, 25}});

  struct EdgeRef {
    uint64_t source;
    uint64_t target;
    int      value;
  };

  EdgeRef max_edge{0, 0, std::numeric_limits<int>::min()};

  for (auto&& v : vertices(g)) {
    auto uid = vertex_id(g, v);
    for (auto&& e : edges(g, v)) {
      int val = edge_value(g, e);
      if (val > max_edge.value) {
        max_edge = {uid, target_id(g, e), val};
      }
    }
  }

  REQUIRE(max_edge.source == 0);
  REQUIRE(max_edge.target == 2);
  REQUIRE(max_edge.value == 50);
}

TEST_CASE("compute average vertex value (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) * 10;
  }

  int    sum   = 0;
  size_t count = 0;

  for (auto&& v : vertices(g)) {
    sum += vertex_value(g, v);
    ++count;
  }

  double avg = static_cast<double>(sum) / static_cast<double>(count);

  REQUIRE(count == 4);
  REQUIRE(sum == 60);
  REQUIRE(avg == 15.0);
}

TEST_CASE("compute average edge value (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_edges;
  G g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}});

  int    sum   = 0;
  size_t count = 0;

  for (auto&& v : vertices(g)) {
    for (auto&& e : edges(g, v)) {
      sum += edge_value(g, e);
      ++count;
    }
  }

  double avg = static_cast<double>(sum) / static_cast<double>(count);

  REQUIRE(count == 4);
  REQUIRE(sum == 100);
  REQUIRE(avg == 25.0);
}

TEST_CASE("product of vertex values (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}});
  for (auto&& v : vertices(g)) {
    auto id            = vertex_id(g, v);
    vertex_value(g, v) = (id == 0) ? 2 : (id == 1) ? 3 : 5;
  }

  std::vector<int> values;
  for (auto&& v : vertices(g)) {
    values.push_back(vertex_value(g, v));
  }

  int product = std::accumulate(values.begin(), values.end(), 1, std::multiplies<int>());

  REQUIRE(product == 30); // 2 * 3 * 5
}

TEST_CASE("count vertices with degree > threshold (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 3}});

  size_t count = 0;
  for (auto&& v : vertices(g)) {
    if (std::ranges::distance(edges(g, v)) >= 2) {
      ++count;
    }
  }

  REQUIRE(count == 1); // vertex 0 has degree 3
}

TEST_CASE("sum vertex values with filter (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) * 10;
  }

  int sum = 0;
  for (auto&& v : vertices(g)) {
    int val = vertex_value(g, v);
    if (val >= 20) {
      sum += val;
    }
  }

  REQUIRE(sum == 90); // 20 + 30 + 40
}

TEST_CASE("sum edge values with filter (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_edges;
  G g({{0, 1, 5}, {0, 2, 15}, {1, 2, 25}, {2, 3, 35}});

  int sum = 0;
  for (auto&& v : vertices(g)) {
    for (auto&& e : edges(g, v)) {
      int val = edge_value(g, e);
      if (val > 10) {
        sum += val;
      }
    }
  }

  REQUIRE(sum == 75); // 15 + 25 + 35
}

TEST_CASE("accumulate on empty graph (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_verts;
  G g;

  int sum = 0;
  for (auto&& v : vertices(g)) {
    sum += vertex_value(g, v);
  }

  REQUIRE(sum == 0);
}

TEST_CASE("accumulate on graph with no edges (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_edges;
  G g({}); // No edges

  int sum = 0;
  for (auto&& v : vertices(g)) {
    for (auto&& e : edges(g, v)) {
      sum += edge_value(g, e);
    }
  }

  REQUIRE(sum == 0);
}

TEST_CASE("fold with custom operation - concatenate IDs (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {1, 2}, {2, 3}});

  std::string result;
  for (auto&& v : vertices(g)) {
    if (!result.empty())
      result += ",";
    result += std::to_string(vertex_id(g, v));
  }

  REQUIRE(result == "0,1,2,3");
}

TEST_CASE("reduce - count self-loops (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}});

  size_t self_loop_count = 0;
  for (auto&& v : vertices(g)) {
    auto uid = vertex_id(g, v);
    for (auto&& e : edges(g, v)) {
      if (uid == target_id(g, e)) {
        ++self_loop_count;
      }
    }
  }

  REQUIRE(self_loop_count == 3);
}

TEST_CASE("accumulate degrees into vector (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

  std::vector<size_t> degrees;
  for (auto&& v : vertices(g)) {
    degrees.push_back(static_cast<size_t>(std::ranges::distance(edges(g, v))));
  }

  REQUIRE(degrees.size() == 4);
  REQUIRE(degrees[0] == 2); // vertex 0
  REQUIRE(degrees[1] == 1); // vertex 1
  REQUIRE(degrees[2] == 1); // vertex 2
  REQUIRE(degrees[3] == 0); // vertex 3
}

TEST_CASE("find vertices with specific degree (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_void;
  G g({{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}});

  std::vector<uint64_t> degree_two;
  for (auto&& v : vertices(g)) {
    if (std::ranges::distance(edges(g, v)) == 2) {
      degree_two.push_back(vertex_id(g, v));
    }
  }

  REQUIRE(degree_two.size() == 2);
  REQUIRE(degree_two[0] == 0);
  REQUIRE(degree_two[1] == 1);
}

TEST_CASE("accumulate with map-based graph (mos)", "[stl][6.2.7][accumulate]") {
  mos_void g({{{"a", "b"}, {"b", "c"}, {"c", "d"}}});

  size_t total_degree = 0;
  for (auto&& v : vertices(g)) {
    total_degree += static_cast<size_t>(std::ranges::distance(edges(g, v)));
  }

  REQUIRE(total_degree == 3);
}

TEST_CASE("weighted sum - vertex values as weights (vov)", "[stl][6.2.7][accumulate]") {
  using G = vov_int_both;
  G g({{0, 1, 10}, {0, 2, 20}, {1, 2, 15}});
  for (auto&& v : vertices(g)) {
    vertex_value(g, v) = static_cast<int>(vertex_id(g, v)) + 1;
  }

  // Weighted sum: for each edge, multiply edge_value by source vertex_value
  int weighted_sum = 0;
  for (auto&& v : vertices(g)) {
    int weight = vertex_value(g, v);
    for (auto&& e : edges(g, v)) {
      weighted_sum += edge_value(g, e) * weight;
    }
  }

  REQUIRE(weighted_sum == 60); // (10*1 + 20*1) + (15*2)
}
