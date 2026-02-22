/**
 * @file test_dynamic_graph_cpo_edge_map.cpp
 * @brief Consolidated CPO tests for edge map containers (vom, mom, voum)
 * 
 * Edge map containers use std::map or std::unordered_map for edge storage (keyed by target_id):
 * - Edges are DEDUPLICATED (only one edge per target vertex - no parallel edges)
 * - vom: vector vertices (resize_vertices), map edges (sorted by target_id)
 * - mom: map vertices (sparse, on-demand), map edges (sorted by target_id)
 * - voum: vector vertices (resize_vertices), unordered_map edges (hash-based, unordered)
 * 
 * Tests are adapted to handle both vertex container semantics.
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

// Helper to check if a tag uses map-based vertices (sparse)
template <typename Tag>
constexpr bool is_map_based_v = std::is_same_v<Tag, mom_tag>;

// Helper to check if a tag uses unordered edge containers (no sorted order guarantee)
template <typename Tag>
constexpr bool is_unordered_edges_v = std::is_same_v<Tag, voum_tag>;

// Helper type for edges
using edge_void = copyable_edge_t<uint32_t, void>;
using edge_int  = copyable_edge_t<uint32_t, int>;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO vertices(g)", "[dynamic_graph][cpo][vertices][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("empty graph") {
    Graph_void g;
    auto       v_range = vertices(g);
    REQUIRE(std::ranges::distance(v_range) == 0);
  }

  SECTION("basic edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}, {2, 3}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(4);
      g.load_edges(edgelist);
    }

    auto v_range = vertices(g);
    REQUIRE(std::ranges::distance(v_range) == 4);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    const auto& cg      = g;
    auto        v_range = vertices(cg);
    REQUIRE(std::ranges::distance(v_range) == 3);
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {1, 2, 20}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    REQUIRE(std::ranges::distance(vertices(g)) == 3);
  }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO num_vertices(g)", "[dynamic_graph][cpo][num_vertices][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_vertices(g) == 0);
  }

  SECTION("with edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}, {2, 3}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(4);
      g.load_edges(edgelist);
    }

    REQUIRE(num_vertices(g) == 4);
  }

  SECTION("consistency with vertices range") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    REQUIRE(num_vertices(g) == static_cast<size_t>(std::ranges::distance(vertices(g))));
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    REQUIRE(num_vertices(cg) == 2);
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(num_vertices(g) == 2);
  }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO find_vertex(g, uid)",
                   "[dynamic_graph][cpo][find_vertex][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("find existing vertex") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = find_vertex(g, uint32_t(0));
    auto v1 = find_vertex(g, uint32_t(1));
    auto v2 = find_vertex(g, uint32_t(2));

    REQUIRE(v0 != vertices(g).end());
    REQUIRE(v1 != vertices(g).end());
    REQUIRE(v2 != vertices(g).end());
  }

  SECTION("find non-existing vertex") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v99 = find_vertex(g, uint32_t(99));
    REQUIRE(v99 == vertices(g).end());
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = find_vertex(cg, uint32_t(0));
    REQUIRE(v0 != vertices(cg).end());
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = find_vertex(g, uint32_t(0));
    REQUIRE(v0 != vertices(g).end());
  }

  SECTION("empty graph") {
    Graph_void g;
    auto       v0 = find_vertex(g, uint32_t(0));
    REQUIRE(v0 == vertices(g).end());
  }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO vertex_id(g, u)", "[dynamic_graph][cpo][vertex_id][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("get vertex IDs") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    std::vector<uint32_t> ids;
    for (auto v : vertices(g)) {
      ids.push_back(vertex_id(g, v));
    }

    std::ranges::sort(ids);
    REQUIRE(ids.size() == 3);
    REQUIRE(ids[0] == 0);
    REQUIRE(ids[1] == 1);
    REQUIRE(ids[2] == 2);
  }

  SECTION("round-trip via find_vertex") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    for (uint32_t expected_id = 0; expected_id < 3; ++expected_id) {
      auto it = find_vertex(g, expected_id);
      REQUIRE(vertex_id(g, *it) == expected_id);
    }
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    for (auto v : vertices(cg)) {
      [[maybe_unused]] auto id = vertex_id(cg, v);
    }
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(vertex_id(g, v0) == 0);
  }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO num_edges(g)", "[dynamic_graph][cpo][num_edges][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_edges(g) == 0);
  }

  SECTION("with edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}, {2, 3}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(4);
      g.load_edges(edgelist);
    }

    REQUIRE(num_edges(g) == 3);
  }

  SECTION("duplicate edges - num_edges counts attempted inserts") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 1}, {0, 1}}; // 3 edges to same target
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    // NOTE: num_edges(g) counts attempted insertions, not actual stored edges.
    // Map deduplicates by target_id, but the counter still tracks all attempts.
    REQUIRE(num_edges(g) == 3); // Counts attempted insertions

    // Verify actual unique edges via degree
    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(degree(g, v0) == 1); // Only 1 unique edge from vertex 0
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    REQUIRE(num_edges(cg) == 2);
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {1, 2, 20}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    REQUIRE(num_edges(g) == 2);
  }
}

//==================================================================================================
// 6. has_edge(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO has_edge(g)", "[dynamic_graph][cpo][has_edge][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(has_edge(g) == false);
  }

  SECTION("graph with edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(has_edge(g) == true);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    REQUIRE(has_edge(cg) == true);
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(has_edge(g) == true);
  }
}

//==================================================================================================
// 7. edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO edges(g, u)", "[dynamic_graph][cpo][edges][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edges from vertex") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0         = *find_vertex(g, uint32_t(0));
    auto edge_range = edges(g, v0);

    std::vector<uint32_t> targets;
    for (auto uv : edge_range) {
      targets.push_back(target_id(g, uv));
    }

    REQUIRE(targets.size() == 2);
    if constexpr (is_unordered_edges_v<TestType>) {
      std::ranges::sort(targets);
    }
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
  }

  SECTION("duplicate edges - map deduplicates") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 1}, {0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edges(g, v0)) {
      ++count;
    }
    REQUIRE(count == 1); // Map deduplicates by target_id
  }

  SECTION("vertex with no edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
      auto   v1    = *find_vertex(g, uint32_t(1));
      size_t count = 0;
      for ([[maybe_unused]] auto uv : edges(g, v1)) {
        ++count;
      }
      REQUIRE(count == 0);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
      auto   v2    = *find_vertex(g, uint32_t(2));
      size_t count = 0;
      for ([[maybe_unused]] auto uv : edges(g, v2)) {
        ++count;
      }
      REQUIRE(count == 0);
    }
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {0, 2, 20}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    int sum = 0;
    for (auto uv : edges(g, v0)) {
      sum += edge_value(g, uv);
    }
    REQUIRE(sum == 30);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edges(cg, v0)) {
      ++count;
    }
    REQUIRE(count == 2);
  }
}

//==================================================================================================
// 8. degree(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO degree(g, u)", "[dynamic_graph][cpo][degree][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("vertex with edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(degree(g, v0) == 2);
  }

  SECTION("duplicate edges - map deduplicates") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 1}, {0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(degree(g, v0) == 1); // Map deduplicates by target_id
  }

  SECTION("vertex with no edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
      auto v1 = *find_vertex(g, uint32_t(1));
      REQUIRE(degree(g, v1) == 0);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
      auto v2 = *find_vertex(g, uint32_t(2));
      REQUIRE(degree(g, v2) == 0);
    }
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));
    REQUIRE(degree(cg, v0) == 2);
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {0, 2, 20}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(degree(g, v0) == 2);
  }
}

//==================================================================================================
// 9. target_id(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO target_id(g, uv)", "[dynamic_graph][cpo][target_id][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("get target IDs") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    std::vector<uint32_t> targets;
    for (auto uv : edges(g, v0)) {
      targets.push_back(target_id(g, uv));
    }

    REQUIRE(targets.size() == 2);
    if constexpr (is_unordered_edges_v<TestType>) {
      std::ranges::sort(targets);
    }
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
  }

  SECTION("duplicate edges - only one edge per target") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 1}, {0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    // Map deduplicates - only one edge to target 1
    size_t count = 0;
    for (auto uv : edges(g, v0)) {
      REQUIRE(target_id(g, uv) == 1);
      ++count;
    }
    REQUIRE(count == 1);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));
    for (auto uv : edges(cg, v0)) {
      REQUIRE(target_id(cg, uv) == 1);
    }
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      REQUIRE(target_id(g, uv) == 1);
    }
  }
}

//==================================================================================================
// 10. target(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO target(g, uv)", "[dynamic_graph][cpo][target][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("returns valid vertex descriptor") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      auto t   = target(g, uv);
      auto tid = vertex_id(g, t);
      REQUIRE((tid == 1 || tid == 2));
    }
  }

  SECTION("consistency with target_id") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto t = target(g, uv);
        REQUIRE(vertex_id(g, t) == target_id(g, uv));
      }
    }
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));
    for (auto uv : edges(cg, v0)) {
      [[maybe_unused]] auto t = target(cg, uv);
    }
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      auto t = target(g, uv);
      REQUIRE(vertex_id(g, t) == 1);
    }
  }
}

//==================================================================================================
// 11. find_vertex_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO find_vertex_edge(g, uid, vid)",
                   "[dynamic_graph][cpo][find_vertex_edge][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("find existing edge") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto edge = find_vertex_edge(g, uint32_t(0), uint32_t(1));
    REQUIRE(target_id(g, edge) == 1);
  }

  SECTION("finds edge (only one per target)") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 1}, {0, 1}}; // Duplicate - only first kept
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto edge = find_vertex_edge(g, uint32_t(0), uint32_t(1));
    REQUIRE(target_id(g, edge) == 1);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg   = g;
    auto        edge = find_vertex_edge(cg, uint32_t(0), uint32_t(1));
    REQUIRE(target_id(cg, edge) == 1);
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto edge = find_vertex_edge(g, uint32_t(0), uint32_t(1));
    REQUIRE(edge_value(g, edge) == 10);
  }
}

//==================================================================================================
// 12. contains_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO contains_edge(g, uid, vid)",
                   "[dynamic_graph][cpo][contains_edge][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edge exists") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
    REQUIRE(contains_edge(g, uint32_t(1), uint32_t(2)));
  }

  SECTION("edge does not exist") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    REQUIRE_FALSE(contains_edge(g, uint32_t(1), uint32_t(0))); // reverse
    REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(2)));
  }

  SECTION("duplicate edges - only one stored") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 1}, {0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
    // NOTE: num_edges(g) counts attempted insertions (3), not stored edges (1)
    REQUIRE(num_edges(g) == 3);
    // Verify via degree that only 1 edge is stored
    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(degree(g, v0) == 1);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    REQUIRE(contains_edge(cg, uint32_t(0), uint32_t(1)));
  }

  SECTION("with edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
  }
}

//==================================================================================================
// 13. vertex_value(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO vertex_value(g, u)",
                   "[dynamic_graph][cpo][vertex_value][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_int_vv = typename Types::int_vv;

  SECTION("access and modify") {
    Graph_int_vv           g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0             = *find_vertex(g, uint32_t(0));
    vertex_value(g, v0) = 42;
    REQUIRE(vertex_value(g, v0) == 42);
  }

  SECTION("default values") {
    Graph_int_vv           g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(vertex_value(g, v0) == 0); // int default
  }

  SECTION("const access") {
    Graph_int_vv           g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0             = *find_vertex(g, uint32_t(0));
    vertex_value(g, v0) = 42;

    const auto& cg  = g;
    auto        cv0 = *find_vertex(cg, uint32_t(0));
    REQUIRE(vertex_value(cg, cv0) == 42);
  }
}

//==================================================================================================
// 14. edge_value(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO edge_value(g, uv)", "[dynamic_graph][cpo][edge_value][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("access edge values") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {0, 2, 20}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    int sum = 0;
    for (auto uv : edges(g, v0)) {
      sum += edge_value(g, uv);
    }
    REQUIRE(sum == 30);
  }

  SECTION("duplicate edges - last value wins (or first)") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    int    sum   = 0;
    size_t count = 0;
    for (auto uv : edges(g, v0)) {
      sum += edge_value(g, uv);
      ++count;
    }
    // Map deduplicates - only one edge stored (value depends on load_edges behavior)
    REQUIRE(count == 1);
    // Value is either 10 (first wins) or 30 (last wins) depending on implementation
    REQUIRE((sum == 10 || sum == 30));
  }

  SECTION("const access") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));

    for (auto uv : edges(cg, v0)) {
      REQUIRE(edge_value(cg, uv) == 10);
    }
  }
}

//==================================================================================================
// 15. graph_value(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO graph_value(g)", "[dynamic_graph][cpo][graph_value][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;

  SECTION("access and modify") {
    Graph_all_int         g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    graph_value(g) = 42;
    REQUIRE(graph_value(g) == 42);
  }

  SECTION("default value") {
    Graph_all_int         g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(graph_value(g) == 0);
  }

  SECTION("const access") {
    Graph_all_int         g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    graph_value(g) = 99;

    const auto& cg = g;
    REQUIRE(graph_value(cg) == 99);
  }
}

//==================================================================================================
// 16. source_id(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO source_id(g, uv)", "[dynamic_graph][cpo][source_id][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types             = graph_test_types<TestType>;
  using Graph_sourced     = typename Types::sourced_void;
  using Graph_sourced_int = typename Types::sourced_int;

  SECTION("basic source IDs") {
    Graph_sourced          g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("different sources") {
    Graph_sourced          g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);
      for (auto uv : edges(g, u)) {
        REQUIRE(source_id(g, uv) == uid);
      }
    }
  }

  SECTION("const correctness") {
    Graph_sourced          g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));
    for (auto uv : edges(cg, v0)) {
      REQUIRE(source_id(cg, uv) == 0);
    }
  }

  SECTION("with edge values") {
    Graph_sourced_int     g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }
}

//==================================================================================================
// 17. source(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO source(g, uv)", "[dynamic_graph][cpo][source][edge_map]", vom_tag, mom_tag, voum_tag) {
  using Types             = graph_test_types<TestType>;
  using Graph_sourced     = typename Types::sourced_void;
  using Graph_sourced_int = typename Types::sourced_int;

  SECTION("basic usage") {
    Graph_sourced          g;
    std::vector<edge_void> edgelist{{0, 1}, {0, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("consistency with source_id") {
    Graph_sourced          g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == source_id(g, uv));
      }
    }
  }

  SECTION("const correctness") {
    Graph_sourced          g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    auto        v0 = *find_vertex(cg, uint32_t(0));
    for (auto uv : edges(cg, v0)) {
      auto src = source(cg, uv);
      REQUIRE(vertex_id(cg, src) == 0);
    }
  }

  SECTION("with edge values") {
    Graph_sourced_int     g;
    std::vector<edge_int> edgelist{{0, 1, 10}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));
    for (auto uv : edges(g, v0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }
}

//==================================================================================================
// 18. partition_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO partition_id(g, u)",
                   "[dynamic_graph][cpo][partition_id][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("default partition") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    for (auto v : vertices(g)) {
      REQUIRE(partition_id(g, v) == 0);
    }
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    for (auto v : vertices(cg)) {
      REQUIRE(partition_id(cg, v) == 0);
    }
  }
}

//==================================================================================================
// 19. num_partitions(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO num_partitions(g)",
                   "[dynamic_graph][cpo][num_partitions][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("default single partition") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    REQUIRE(num_partitions(cg) == 1);
  }
}

//==================================================================================================
// 20. vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO vertices(g, pid)",
                   "[dynamic_graph][cpo][vertices][partition][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("partition 0 returns all vertices") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    auto verts_all = vertices(g);
    auto verts_p0  = vertices(g, 0);

    REQUIRE(std::ranges::distance(verts_all) == std::ranges::distance(verts_p0));
  }

  SECTION("non-zero partition returns empty") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto verts_p1 = vertices(g, 1);
    REQUIRE(std::ranges::distance(verts_p1) == 0);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg       = g;
    auto        verts_p0 = vertices(cg, 0);
    REQUIRE(std::ranges::distance(verts_p0) == 2);
  }
}

//==================================================================================================
// 21. num_vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO num_vertices(g, pid)",
                   "[dynamic_graph][cpo][num_vertices][partition][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("partition 0 returns total count") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    REQUIRE(num_vertices(g, 0) == num_vertices(g));
  }

  SECTION("non-zero partition returns zero") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    REQUIRE(num_vertices(g, 1) == 0);
  }

  SECTION("const correctness") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    const auto& cg = g;
    REQUIRE(num_vertices(cg, 0) == 2);
  }

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_vertices(g, 0) == 0);
  }
}

//==================================================================================================
// 22. Integration Tests - Duplicate Edges (Map Behavior)
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO integration: duplicate edges",
                   "[dynamic_graph][cpo][integration][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("traverse edges (duplicates removed)") {
    Graph_int_ev g;
    // Loading edges with same source->target but different values
    std::vector<edge_int> edgelist{{0, 1, 10}, {0, 1, 20}, {0, 1, 30}, {0, 2, 40}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    // NOTE: num_edges(g) counts attempted insertions (4), not stored edges (2)
    REQUIRE(num_edges(g) == 4);

    // Verify actual unique edges via degree (map deduplicates)
    auto v0 = *find_vertex(g, uint32_t(0));
    REQUIRE(degree(g, v0) == 2); // Only 2 unique edges: 0->1 and 0->2

    // Sum all edge values (one edge to 1 and one to 2)
    int total = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        total += edge_value(g, uv);
      }
    }
    // Either first or last value for edge 0->1 (10 or 30) plus 40
    REQUIRE((total == 50 || total == 70));
  }

  SECTION("find edge (only one per target)") {
    Graph_int_ev          g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(2);
      g.load_edges(edgelist);
    }

    auto edge = find_vertex_edge(g, uint32_t(0), uint32_t(1));
    REQUIRE(target_id(g, edge) == 1);
    // Value is either first or last depending on load_edges behavior
    auto val = edge_value(g, edge);
    REQUIRE((val == 10 || val == 30));
  }
}

//==================================================================================================
// 23. Integration Tests - Values
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO integration: values",
                   "[dynamic_graph][cpo][integration][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;

  SECTION("access all value types") {
    Graph_all_int         g;
    std::vector<edge_int> edgelist{{0, 1, 10}, {1, 2, 20}};
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(3);
      g.load_edges(edgelist);
    }

    // Set graph value
    graph_value(g) = 1000;
    REQUIRE(graph_value(g) == 1000);

    // Set vertex values
    int vval = 100;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = vval;
      vval += 100;
    }

    // Sum edge values
    int ev_sum = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        ev_sum += edge_value(g, uv);
      }
    }
    REQUIRE(ev_sum == 30);
  }
}

//==================================================================================================
// 24. Integration Tests - Traversal
//==================================================================================================

TEMPLATE_TEST_CASE("edge_map CPO integration: traversal",
                   "[dynamic_graph][cpo][integration][edge_map]",
                   vom_tag,
                   mom_tag,
                   voum_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("traverse all edges") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 1}, {1, 2}, {2, 3}, {0, 1}}; // duplicate 0->1 deduplicated
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(4);
      g.load_edges(edgelist);
    }

    size_t edge_count = 0;
    for (auto u : vertices(g)) {
      for ([[maybe_unused]] auto uv : edges(g, u)) {
        ++edge_count;
      }
    }

    REQUIRE(edge_count == 3); // Duplicate 0->1 removed by map
  }

  SECTION("sorted edge order") {
    Graph_void             g;
    std::vector<edge_void> edgelist{{0, 3}, {0, 1}, {0, 2}}; // Inserted out of order
    if constexpr (is_map_based_v<TestType>) {
      g.load_edges(edgelist);
    } else {
      g.resize_vertices(4);
      g.load_edges(edgelist);
    }

    auto v0 = *find_vertex(g, uint32_t(0));

    std::vector<uint32_t> targets;
    for (auto uv : edges(g, v0)) {
      targets.push_back(target_id(g, uv));
    }

    // Map sorts by target key; unordered_map does not, so sort for comparison
    REQUIRE(targets.size() == 3);
    if constexpr (is_unordered_edges_v<TestType>) {
      std::ranges::sort(targets);
    }
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
    REQUIRE(targets[2] == 3);
  }
}
