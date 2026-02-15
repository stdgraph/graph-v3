/**
 * @file test_dynamic_graph_cpo_map_vertices.cpp
 * @brief Consolidated CPO tests for map-based vertex containers (mol, mov, mod, mofl, mos, mous)
 * 
 * Map-based vertex containers have key differences from vector/deque containers:
 * - Vertices are created on-demand from edge endpoints (no resize_vertices)
 * - Vertex IDs can be sparse (non-contiguous, e.g., 100, 500, 1000)
 * - Vertices are iterated in sorted order by key
 * - String vertex IDs are a primary use case
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "../../common/graph_test_types.hpp"
#include "../../common/map_graph_test_data.hpp"
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <string>
#include <vector>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;
using namespace graph::test::map_data;

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO vertices(g)",
                   "[dynamic_graph][cpo][vertices][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("basic edges - contiguous IDs") {
    auto g = make_basic_graph_void<Graph_void>();

    auto   v_range = vertices(g);
    size_t count   = 0;
    for ([[maybe_unused]] auto v : v_range) {
      ++count;
    }
    REQUIRE(count == basic_expected::vertex_count);
  }

  SECTION("sparse vertex IDs - key feature of map containers") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v_range = vertices(g);

    // Collect vertex IDs
    std::vector<uint32_t> ids;
    for (auto v : v_range) {
      ids.push_back(vertex_id(g, v));
    }

    // Map containers iterate in sorted order
    REQUIRE(ids.size() == sparse_expected::vertex_count);
    for (size_t i = 0; i < ids.size(); ++i) {
      REQUIRE(ids[i] == sparse_expected::vertex_ids_sorted[i]);
    }
  }

  SECTION("very sparse IDs - large gaps") {
    auto g = make_very_sparse_graph<Graph_void>();

    std::vector<uint32_t> ids;
    for (auto v : vertices(g)) {
      ids.push_back(vertex_id(g, v));
    }

    REQUIRE(ids.size() == very_sparse_expected::vertex_count);
    // Should be in sorted order
    for (size_t i = 0; i < ids.size(); ++i) {
      REQUIRE(ids[i] == very_sparse_expected::vertex_ids_sorted[i]);
    }
  }

  SECTION("const correctness") {
    const auto g = make_basic_graph_void<Graph_void>();

    auto   v_range = vertices(g);
    size_t count   = 0;
    for ([[maybe_unused]] auto v : v_range) {
      ++count;
    }
    REQUIRE(count == basic_expected::vertex_count);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    size_t count = 0;
    for ([[maybe_unused]] auto v : vertices(g)) {
      ++count;
    }
    REQUIRE(count == sparse_expected::vertex_count);
  }

  SECTION("empty graph") {
    Graph_void g;

    size_t count = 0;
    for ([[maybe_unused]] auto v : vertices(g)) {
      ++count;
    }
    REQUIRE(count == 0);
  }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO num_vertices(g)",
                   "[dynamic_graph][cpo][num_vertices][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("basic edges") {
    auto g = make_basic_graph_void<Graph_void>();
    REQUIRE(num_vertices(g) == basic_expected::vertex_count);
  }

  SECTION("sparse IDs - same count as contiguous") {
    auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_vertices(g) == sparse_expected::vertex_count);
  }

  SECTION("very sparse IDs") {
    auto g = make_very_sparse_graph<Graph_void>();
    REQUIRE(num_vertices(g) == very_sparse_expected::vertex_count);
  }

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_vertices(g) == 0);
  }

  SECTION("self-loops create fewer vertices") {
    auto g = make_self_loop_graph<Graph_void>();
    REQUIRE(num_vertices(g) == self_loop_expected::vertex_count);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_vertices(g) == sparse_expected::vertex_count);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();
    REQUIRE(num_vertices(g) == sparse_expected::vertex_count);
  }

  SECTION("consistency with vertices range") {
    auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_vertices(g) == static_cast<size_t>(std::ranges::distance(vertices(g))));
  }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO find_vertex(g, uid)",
                   "[dynamic_graph][cpo][find_vertex][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("find existing vertex - contiguous") {
    auto g = make_basic_graph_void<Graph_void>();

    for (auto expected_id : basic_expected::vertex_ids) {
      auto it = find_vertex(g, expected_id);
      REQUIRE(it != vertices(g).end());
      REQUIRE(vertex_id(g, *it) == expected_id);
    }
  }

  SECTION("find existing vertex - sparse IDs") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto expected_id : sparse_expected::vertex_ids_sorted) {
      auto it = find_vertex(g, expected_id);
      REQUIRE(it != vertices(g).end());
      REQUIRE(vertex_id(g, *it) == expected_id);
    }
  }

  SECTION("find non-existent vertex - gap in sparse IDs") {
    auto g = make_sparse_graph_void<Graph_void>();

    // These IDs are in the gaps
    auto it = find_vertex(g, uint32_t(50));
    REQUIRE(it == vertices(g).end());

    it = find_vertex(g, uint32_t(200));
    REQUIRE(it == vertices(g).end());

    it = find_vertex(g, uint32_t(750));
    REQUIRE(it == vertices(g).end());
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto it = find_vertex(g, uint32_t(100));
    REQUIRE(it != vertices(g).end());
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto it = find_vertex(g, uint32_t(100));
    REQUIRE(it != vertices(g).end());
    REQUIRE(vertex_id(g, *it) == 100);
  }

  SECTION("empty graph") {
    Graph_void g;

    auto it = find_vertex(g, uint32_t(0));
    REQUIRE(it == vertices(g).end());
  }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO vertex_id(g, u)",
                   "[dynamic_graph][cpo][vertex_id][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("contiguous IDs") {
    auto g = make_basic_graph_void<Graph_void>();

    std::vector<uint32_t> ids;
    for (auto v : vertices(g)) {
      ids.push_back(vertex_id(g, v));
    }

    REQUIRE(ids.size() == basic_expected::vertex_count);
    // Map iterates in sorted order
    std::vector<uint32_t> expected(basic_expected::vertex_ids.begin(), basic_expected::vertex_ids.end());
    REQUIRE(ids == expected);
  }

  SECTION("sparse IDs - key feature") {
    auto g = make_sparse_graph_void<Graph_void>();

    std::vector<uint32_t> ids;
    for (auto v : vertices(g)) {
      ids.push_back(vertex_id(g, v));
    }

    REQUIRE(ids.size() == sparse_expected::vertex_count);
    std::vector<uint32_t> expected(sparse_expected::vertex_ids_sorted.begin(),
                                   sparse_expected::vertex_ids_sorted.end());
    REQUIRE(ids == expected);
  }

  SECTION("round-trip via find_vertex") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto expected_id : sparse_expected::vertex_ids_sorted) {
      auto it = find_vertex(g, expected_id);
      REQUIRE(vertex_id(g, *it) == expected_id);
    }
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    for (auto v : vertices(g)) {
      [[maybe_unused]] auto id = vertex_id(g, v);
    }
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    std::vector<uint32_t> ids;
    for (auto v : vertices(g)) {
      ids.push_back(vertex_id(g, v));
    }
    REQUIRE(ids.size() == sparse_expected::vertex_count);
  }

  SECTION("very sparse - IDs match expected") {
    auto g = make_very_sparse_graph<Graph_void>();

    std::vector<uint32_t> ids;
    for (auto v : vertices(g)) {
      ids.push_back(vertex_id(g, v));
    }

    std::vector<uint32_t> expected(very_sparse_expected::vertex_ids_sorted.begin(),
                                   very_sparse_expected::vertex_ids_sorted.end());
    REQUIRE(ids == expected);
  }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO num_edges(g)",
                   "[dynamic_graph][cpo][num_edges][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("basic edges") {
    auto g = make_basic_graph_void<Graph_void>();
    REQUIRE(num_edges(g) == basic_expected::edge_count);
  }

  SECTION("sparse IDs - same edge count") {
    auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_edges(g) == sparse_expected::edge_count);
  }

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_edges(g) == 0);
  }

  SECTION("self-loops count as edges") {
    auto g = make_self_loop_graph<Graph_void>();
    REQUIRE(num_edges(g) == self_loop_expected::edge_count);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_edges(g) == sparse_expected::edge_count);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();
    REQUIRE(num_edges(g) == sparse_expected::edge_count);
  }
}

//==================================================================================================
// 6. has_edge(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO has_edge(g)",
                   "[dynamic_graph][cpo][has_edge][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("graph with edges") {
    auto g = make_basic_graph_void<Graph_void>();
    REQUIRE(has_edge(g) == true);
  }

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(has_edge(g) == false);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(has_edge(g) == true);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();
    REQUIRE(has_edge(g) == true);
  }
}

//==================================================================================================
// 7. num_edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO num_edges(g, u)",
                   "[dynamic_graph][cpo][num_edges][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("vertex with edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(num_edges(g, v100) == 2); // 100->500, 100->1000
  }

  SECTION("vertex with single edge") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v500 = *find_vertex(g, uint32_t(500));
    REQUIRE(num_edges(g, v500) == 1); // 500->1000
  }

  SECTION("vertex with no edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v5000 = *find_vertex(g, uint32_t(5000));
    REQUIRE(num_edges(g, v5000) == 0);
  }

  SECTION("all vertices") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v100  = *find_vertex(g, uint32_t(100));
    auto v500  = *find_vertex(g, uint32_t(500));
    auto v1000 = *find_vertex(g, uint32_t(1000));
    auto v5000 = *find_vertex(g, uint32_t(5000));

    REQUIRE(num_edges(g, v100) == 2);
    REQUIRE(num_edges(g, v500) == 1);
    REQUIRE(num_edges(g, v1000) == 1);
    REQUIRE(num_edges(g, v5000) == 0);
  }

  SECTION("matches degree") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto u : vertices(g)) {
      REQUIRE(num_edges(g, u) == degree(g, u));
    }
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(num_edges(g, v100) == 2);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(num_edges(g, v100) == 2);
  }
}

//==================================================================================================
// 8. num_edges(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO num_edges(g, uid)",
                   "[dynamic_graph][cpo][num_edges][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("by vertex ID - with edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_edges(g, uint32_t(100)) == 2);
  }

  SECTION("by vertex ID - single edge") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_edges(g, uint32_t(500)) == 1);
  }

  SECTION("by vertex ID - no edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_edges(g, uint32_t(5000)) == 0);
  }

  SECTION("all vertices by ID") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_edges(g, uint32_t(100)) == 2);
    REQUIRE(num_edges(g, uint32_t(500)) == 1);
    REQUIRE(num_edges(g, uint32_t(1000)) == 1);
    REQUIRE(num_edges(g, uint32_t(5000)) == 0);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_edges(g, uint32_t(100)) == 2);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    REQUIRE(num_edges(g, uint32_t(100)) == 2);
  }

  SECTION("consistency with num_edges(g, u)") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);
      REQUIRE(num_edges(g, uid) == num_edges(g, u));
    }
  }
}

//==================================================================================================
// 9. edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO edges(g, u)",
                   "[dynamic_graph][cpo][edges][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edges from sparse vertex") {
    auto g = make_sparse_graph_void<Graph_void>();

    // Vertex 100 has edges to 500 and 1000
    auto v100       = *find_vertex(g, uint32_t(100));
    auto edge_range = edges(g, v100);

    std::vector<uint32_t> targets;
    for (auto uv : edge_range) {
      targets.push_back(target_id(g, uv));
    }

    REQUIRE(targets.size() == 2);
    std::ranges::sort(targets);
    REQUIRE(targets[0] == 500);
    REQUIRE(targets[1] == 1000);
  }

  SECTION("vertex with no outgoing edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    // Vertex 5000 has no outgoing edges
    auto v5000      = *find_vertex(g, uint32_t(5000));
    auto edge_range = edges(g, v5000);

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));

    int sum = 0;
    for (auto uv : edges(g, v100)) {
      sum += edge_value(g, uv);
    }
    // Edges from 100: {100,500,15} and {100,1000,25}
    REQUIRE(sum == 40);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edges(g, v100)) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("all vertices") {
    auto g = make_sparse_graph_void<Graph_void>();

    size_t total_edges = 0;
    for (auto u : vertices(g)) {
      for ([[maybe_unused]] auto uv : edges(g, u)) {
        ++total_edges;
      }
    }
    REQUIRE(total_edges == sparse_expected::edge_count);
  }
}

//==================================================================================================
// 10. edges(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO edges(g, uid)",
                   "[dynamic_graph][cpo][edges][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("with vertex ID") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto edge_range = edges(g, uint32_t(100));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("returns valid range") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto edge_range = edges(g, uint32_t(500));

    // Verify return type is a range
    static_assert(std::ranges::forward_range<decltype(edge_range)>);

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 1);
  }

  SECTION("vertex with no edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto edge_range = edges(g, uint32_t(5000));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto edge_range = edges(g, uint32_t(100));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto edge_range = edges(g, uint32_t(100));

    int sum = 0;
    for (auto uv : edge_range) {
      sum += edge_value(g, uv);
    }
    REQUIRE(sum == 40);
  }

  SECTION("consistency with edges(g, u)") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);

      size_t count_u = 0, count_uid = 0;
      for ([[maybe_unused]] auto uv : edges(g, u))
        ++count_u;
      for ([[maybe_unused]] auto uv : edges(g, uid))
        ++count_uid;

      REQUIRE(count_u == count_uid);
    }
  }
}

//==================================================================================================
// 11. degree(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO degree(g, u)",
                   "[dynamic_graph][cpo][degree][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("sparse vertices") {
    auto g = make_sparse_graph_void<Graph_void>();

    // Vertex 100 -> 500, 1000 (degree 2)
    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(degree(g, v100) == 2);

    // Vertex 500 -> 1000 (degree 1)
    auto v500 = *find_vertex(g, uint32_t(500));
    REQUIRE(degree(g, v500) == 1);

    // Vertex 5000 -> nothing (degree 0)
    auto v5000 = *find_vertex(g, uint32_t(5000));
    REQUIRE(degree(g, v5000) == 0);
  }

  SECTION("self-loop counts") {
    auto g = make_self_loop_graph<Graph_void>();

    // Vertex 100 has self-loop and edge to 200
    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(degree(g, v100) == 2);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(degree(g, v100) == 2);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(degree(g, v100) == 2);
  }

  SECTION("matches num_edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto u : vertices(g)) {
      REQUIRE(degree(g, u) == num_edges(g, u));
    }
  }
}

//==================================================================================================
// 12. target_id(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO target_id(g, uv)",
                   "[dynamic_graph][cpo][target_id][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("sparse targets") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));

    std::vector<uint32_t> targets;
    for (auto uv : edges(g, v100)) {
      targets.push_back(target_id(g, uv));
    }

    std::ranges::sort(targets);
    REQUIRE(targets.size() == 2);
    REQUIRE(targets[0] == 500);
    REQUIRE(targets[1] == 1000);
  }

  SECTION("all edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    std::vector<uint32_t> all_targets;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        all_targets.push_back(target_id(g, uv));
      }
    }

    REQUIRE(all_targets.size() == sparse_expected::edge_count);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      [[maybe_unused]] auto tid = target_id(g, uv);
    }
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));

    std::vector<uint32_t> targets;
    for (auto uv : edges(g, v100)) {
      targets.push_back(target_id(g, uv));
    }
    REQUIRE(targets.size() == 2);
  }

  SECTION("self-loop") {
    auto g = make_self_loop_graph<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));

    bool found_self_loop = false;
    for (auto uv : edges(g, v100)) {
      if (target_id(g, uv) == 100) {
        found_self_loop = true;
      }
    }
    REQUIRE(found_self_loop);
  }
}

//==================================================================================================
// 13. target(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO target(g, uv)",
                   "[dynamic_graph][cpo][target][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("returns valid vertex descriptor") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));

    for (auto uv : edges(g, v100)) {
      auto target_v = target(g, uv);
      auto tid      = vertex_id(g, target_v);
      REQUIRE((tid == 500 || tid == 1000));
    }
  }

  SECTION("consistency with target_id") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto t = target(g, uv);
        REQUIRE(vertex_id(g, t) == target_id(g, uv));
      }
    }
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      [[maybe_unused]] auto t = target(g, uv);
    }
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      auto t   = target(g, uv);
      auto tid = vertex_id(g, t);
      REQUIRE((tid == 500 || tid == 1000));
    }
  }
}

//==================================================================================================
// 14. find_vertex_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO find_vertex_edge(g, uid, vid)",
                   "[dynamic_graph][cpo][find_vertex_edge][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("find existing edge - sparse IDs") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto edge = find_vertex_edge(g, uint32_t(100), uint32_t(500));
    REQUIRE(target_id(g, edge) == 500);
  }

  SECTION("multiple edges from same source") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto e1 = find_vertex_edge(g, uint32_t(100), uint32_t(500));
    auto e2 = find_vertex_edge(g, uint32_t(100), uint32_t(1000));

    REQUIRE(target_id(g, e1) == 500);
    REQUIRE(target_id(g, e2) == 1000);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto edge = find_vertex_edge(g, uint32_t(100), uint32_t(500));
    REQUIRE(target_id(g, edge) == 500);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto edge = find_vertex_edge(g, uint32_t(100), uint32_t(500));
    REQUIRE(edge_value(g, edge) == 15);
  }
}

//==================================================================================================
// 15. find_vertex_edge(g, u, v) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO find_vertex_edge(g, u, v)",
                   "[dynamic_graph][cpo][find_vertex_edge][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("find existing edge with vertex descriptors") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto u = *find_vertex(g, uint32_t(100));
    auto v = *find_vertex(g, uint32_t(500));

    auto edge = find_vertex_edge(g, u, v);
    REQUIRE(target_id(g, edge) == 500);
  }

  SECTION("multiple edges") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto u100  = *find_vertex(g, uint32_t(100));
    auto v500  = *find_vertex(g, uint32_t(500));
    auto v1000 = *find_vertex(g, uint32_t(1000));

    auto e1 = find_vertex_edge(g, u100, v500);
    auto e2 = find_vertex_edge(g, u100, v1000);

    REQUIRE(target_id(g, e1) == 500);
    REQUIRE(target_id(g, e2) == 1000);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto u = *find_vertex(g, uint32_t(100));
    auto v = *find_vertex(g, uint32_t(500));

    auto edge = find_vertex_edge(g, u, v);
    REQUIRE(target_id(g, edge) == 500);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto u = *find_vertex(g, uint32_t(100));
    auto v = *find_vertex(g, uint32_t(500));

    auto edge = find_vertex_edge(g, u, v);
    REQUIRE(edge_value(g, edge) == 15);
  }
}

//==================================================================================================
// 16. contains_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO contains_edge(g, uid, vid)",
                   "[dynamic_graph][cpo][contains_edge][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edge exists - sparse IDs") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(500)) == true);
    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(1000)) == true);
    REQUIRE(contains_edge(g, uint32_t(1000), uint32_t(5000)) == true);
  }

  SECTION("edge does not exist") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(5000)) == false);
    REQUIRE(contains_edge(g, uint32_t(500), uint32_t(100)) == false); // reverse doesn't exist
  }

  SECTION("self-loop") {
    auto g = make_self_loop_graph<Graph_void>();

    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(100)) == true);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(500)) == true);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(500)) == true);
    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(5000)) == false);
  }

  SECTION("all edges in graph") {
    auto g = make_sparse_graph_void<Graph_void>();

    // Verify all expected edges exist
    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(500)));
    REQUIRE(contains_edge(g, uint32_t(100), uint32_t(1000)));
    REQUIRE(contains_edge(g, uint32_t(500), uint32_t(1000)));
    REQUIRE(contains_edge(g, uint32_t(1000), uint32_t(5000)));
  }
}

//==================================================================================================
// 17. contains_edge(g, u, v) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO contains_edge(g, u, v)",
                   "[dynamic_graph][cpo][contains_edge][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edge exists") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto u100  = *find_vertex(g, uint32_t(100));
    auto v500  = *find_vertex(g, uint32_t(500));
    auto v1000 = *find_vertex(g, uint32_t(1000));

    REQUIRE(contains_edge(g, u100, v500));
    REQUIRE(contains_edge(g, u100, v1000));
  }

  SECTION("edge does not exist") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto u100  = *find_vertex(g, uint32_t(100));
    auto u500  = *find_vertex(g, uint32_t(500));
    auto v5000 = *find_vertex(g, uint32_t(5000));

    REQUIRE_FALSE(contains_edge(g, u100, v5000));
    REQUIRE_FALSE(contains_edge(g, u500, u100)); // reverse doesn't exist
  }

  SECTION("self-loop") {
    auto g = make_self_loop_graph<Graph_void>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(contains_edge(g, v100, v100));
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto u100 = *find_vertex(g, uint32_t(100));
    auto v500 = *find_vertex(g, uint32_t(500));

    REQUIRE(contains_edge(g, u100, v500));
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto u100  = *find_vertex(g, uint32_t(100));
    auto v500  = *find_vertex(g, uint32_t(500));
    auto v5000 = *find_vertex(g, uint32_t(5000));

    REQUIRE(contains_edge(g, u100, v500));
    REQUIRE_FALSE(contains_edge(g, u100, v5000));
  }
}

//==================================================================================================
// 18. vertex_value(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO vertex_value(g, u)",
                   "[dynamic_graph][cpo][vertex_value][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_int_vv = typename Types::int_vv;

  SECTION("access and modify") {
    auto g = make_sparse_graph_void<Graph_int_vv>();

    auto v100             = *find_vertex(g, uint32_t(100));
    vertex_value(g, v100) = 42;
    REQUIRE(vertex_value(g, v100) == 42);

    auto v5000             = *find_vertex(g, uint32_t(5000));
    vertex_value(g, v5000) = 99;
    REQUIRE(vertex_value(g, v5000) == 99);
  }

  SECTION("default values") {
    auto g = make_sparse_graph_void<Graph_int_vv>();

    auto v100 = *find_vertex(g, uint32_t(100));
    REQUIRE(vertex_value(g, v100) == 0); // int default
  }

  SECTION("all vertices") {
    auto g = make_sparse_graph_void<Graph_int_vv>();

    int val = 10;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val;
      val += 10;
    }

    // Verify values
    val = 10;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == val);
      val += 10;
    }
  }

  SECTION("const access") {
    auto g = make_sparse_graph_void<Graph_int_vv>();

    auto v100             = *find_vertex(g, uint32_t(100));
    vertex_value(g, v100) = 42;

    const auto& cg    = g;
    auto        cv100 = *find_vertex(cg, uint32_t(100));
    REQUIRE(vertex_value(cg, cv100) == 42);
  }
}

//==================================================================================================
// 19. edge_value(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO edge_value(g, uv)",
                   "[dynamic_graph][cpo][edge_value][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag) { // Note: mos, mous use set - const edges
  using Types        = graph_test_types<TestType>;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("access edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));

    int sum = 0;
    for (auto uv : edges(g, v100)) {
      sum += edge_value(g, uv);
    }
    REQUIRE(sum == 40); // 15 + 25
  }

  SECTION("modify edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      edge_value(g, uv) = 100;
    }

    int sum = 0;
    for (auto uv : edges(g, v100)) {
      sum += edge_value(g, uv);
    }
    REQUIRE(sum == 200); // 100 + 100
  }

  SECTION("via find_vertex_edge") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto edge = find_vertex_edge(g, uint32_t(100), uint32_t(500));
    REQUIRE(edge_value(g, edge) == 15);

    edge_value(g, edge) = 150;
    REQUIRE(edge_value(g, edge) == 150);
  }

  SECTION("const access") {
    const auto g = make_sparse_graph_int<Graph_int_ev>();

    auto v100 = *find_vertex(g, uint32_t(100));

    int sum = 0;
    for (auto uv : edges(g, v100)) {
      sum += edge_value(g, uv);
    }
    REQUIRE(sum == 40);
  }

  SECTION("all edges") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    int total = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        total += edge_value(g, uv);
      }
    }
    REQUIRE(total == sparse_expected::edge_value_sum);
  }
}

//==================================================================================================
// 20. graph_value(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO graph_value(g)",
                   "[dynamic_graph][cpo][graph_value][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;

  SECTION("access and modify") {
    auto g = make_sparse_graph_int<Graph_all_int>();

    graph_value(g) = 42;
    REQUIRE(graph_value(g) == 42);
  }

  SECTION("default value") {
    auto g = make_sparse_graph_int<Graph_all_int>();
    REQUIRE(graph_value(g) == 0); // int default
  }

  SECTION("const access") {
    auto g         = make_sparse_graph_int<Graph_all_int>();
    graph_value(g) = 99;

    const auto& cg = g;
    REQUIRE(graph_value(cg) == 99);
  }
}

//==================================================================================================
// 21. source_id(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO source_id(g, uv)",
                   "[dynamic_graph][cpo][source_id][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types             = graph_test_types<TestType>;
  using Graph_sourced     = typename Types::sourced_void;
  using Graph_sourced_int = typename Types::sourced_int;

  SECTION("sparse source IDs") {
    auto g = make_sparse_graph_void<Graph_sourced>();

    auto v100 = *find_vertex(g, uint32_t(100));

    for (auto uv : edges(g, v100)) {
      REQUIRE(source_id(g, uv) == 100);
    }
  }

  SECTION("different sources") {
    auto g = make_sparse_graph_void<Graph_sourced>();

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);
      for (auto uv : edges(g, u)) {
        REQUIRE(source_id(g, uv) == uid);
      }
    }
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_sourced>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      REQUIRE(source_id(g, uv) == 100);
    }
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_sourced_int>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      REQUIRE(source_id(g, uv) == 100);
    }
  }

  SECTION("self-loop") {
    auto g = make_self_loop_graph<Graph_sourced>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      REQUIRE(source_id(g, uv) == 100);
    }
  }
}

//==================================================================================================
// 22. source(g, uv) CPO Tests (Sourced=true)
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO source(g, uv)",
                   "[dynamic_graph][cpo][source][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types             = graph_test_types<TestType>;
  using Graph_sourced     = typename Types::sourced_void;
  using Graph_sourced_int = typename Types::sourced_int;

  SECTION("basic usage") {
    auto g = make_sparse_graph_void<Graph_sourced>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 100);
    }
  }

  SECTION("consistency with source_id") {
    auto g = make_sparse_graph_void<Graph_sourced>();

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == source_id(g, uv));
      }
    }
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_sourced>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 100);
    }
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_sourced_int>();

    auto v100 = *find_vertex(g, uint32_t(100));
    for (auto uv : edges(g, v100)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 100);
    }
  }

  SECTION("different sources") {
    auto g = make_sparse_graph_void<Graph_sourced>();

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == uid);
      }
    }
  }
}

//==================================================================================================
// 23. partition_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO partition_id(g, u)",
                   "[dynamic_graph][cpo][partition_id][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("default partition") {
    auto g = make_sparse_graph_void<Graph_void>();

    for (auto v : vertices(g)) {
      REQUIRE(partition_id(g, v) == 0);
    }
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    for (auto v : vertices(g)) {
      REQUIRE(partition_id(g, v) == 0);
    }
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    for (auto v : vertices(g)) {
      REQUIRE(partition_id(g, v) == 0);
    }
  }
}

//==================================================================================================
// 24. num_partitions(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO num_partitions(g)",
                   "[dynamic_graph][cpo][num_partitions][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("default single partition") {
    auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();
    REQUIRE(num_partitions(g) == 1);
  }
}

//==================================================================================================
// 25. vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO vertices(g, pid)",
                   "[dynamic_graph][cpo][vertices][partition][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("partition 0 returns all vertices") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto verts_all = vertices(g);
    auto verts_p0  = vertices(g, 0);

    REQUIRE(std::ranges::distance(verts_all) == std::ranges::distance(verts_p0));
  }

  SECTION("non-zero partition returns empty") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto verts_p1 = vertices(g, 1);
    REQUIRE(std::ranges::distance(verts_p1) == 0);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    auto verts_p0 = vertices(g, 0);
    REQUIRE(std::ranges::distance(verts_p0) == static_cast<ptrdiff_t>(sparse_expected::vertex_count));
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    auto verts_p0 = vertices(g, 0);
    REQUIRE(std::ranges::distance(verts_p0) == static_cast<ptrdiff_t>(sparse_expected::vertex_count));
  }

  SECTION("iterate partition vertices") {
    auto g = make_sparse_graph_void<Graph_void>();

    size_t count = 0;
    for ([[maybe_unused]] auto u : vertices(g, 0)) {
      ++count;
    }
    REQUIRE(count == sparse_expected::vertex_count);
  }
}

//==================================================================================================
// 26. num_vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO num_vertices(g, pid)",
                   "[dynamic_graph][cpo][num_vertices][partition][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("partition 0 returns total count") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_vertices(g, 0) == num_vertices(g));
  }

  SECTION("non-zero partition returns zero") {
    auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_vertices(g, 1) == 0);
  }

  SECTION("const correctness") {
    const auto g = make_sparse_graph_void<Graph_void>();

    REQUIRE(num_vertices(g, 0) == sparse_expected::vertex_count);
  }

  SECTION("with edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    REQUIRE(num_vertices(g, 0) == sparse_expected::vertex_count);
  }

  SECTION("empty graph") {
    Graph_void g;

    REQUIRE(num_vertices(g, 0) == 0);
  }

  SECTION("consistency with vertices(g, pid)") {
    auto g = make_sparse_graph_void<Graph_void>();

    auto nv0      = num_vertices(g, 0);
    auto verts_p0 = vertices(g, 0);

    REQUIRE(nv0 == static_cast<size_t>(std::ranges::distance(verts_p0)));
  }
}

//==================================================================================================
// 27. Integration Tests - Sparse IDs
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO integration: sparse traversal",
                   "[dynamic_graph][cpo][integration][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag,
                   mos_tag,
                   mous_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("traverse all edges with sparse IDs") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    int    total      = 0;
    size_t edge_count = 0;

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        total += edge_value(g, uv);
        ++edge_count;
      }
    }

    REQUIRE(edge_count == sparse_expected::edge_count);
    REQUIRE(total == sparse_expected::edge_value_sum);
  }

  SECTION("find path through sparse vertices") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    // Path: 100 -> 500 -> 1000 -> 5000
    std::vector<uint32_t> path;
    uint32_t              current = 100;
    path.push_back(current);

    while (true) {
      auto v          = *find_vertex(g, current);
      auto edge_range = edges(g, v);
      if (edge_range.begin() == edge_range.end())
        break;

      // Take first edge (for simplicity)
      current = target_id(g, *edge_range.begin());
      path.push_back(current);
      if (path.size() > 10)
        break; // Safety limit
    }

    REQUIRE(path.size() >= 2); // At least start and one step
  }
}

//==================================================================================================
// 28. Integration Tests - Values
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO integration: values",
                   "[dynamic_graph][cpo][integration][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag) { // exclude set containers for edge modification
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;

  SECTION("access all value types") {
    auto g = make_sparse_graph_int<Graph_all_int>();

    // Set graph value
    graph_value(g) = 1000;

    // Set vertex values
    int vval = 10;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = vval;
      vval += 10;
    }

    // Verify
    REQUIRE(graph_value(g) == 1000);

    vval = 10;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == vval);
      vval += 10;
    }

    // Edge values already set by make_sparse_graph_int
    int ev_sum = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        ev_sum += edge_value(g, uv);
      }
    }
    REQUIRE(ev_sum == sparse_expected::edge_value_sum);
  }
}

//==================================================================================================
// 29. Integration Tests - Modify Values
//==================================================================================================

TEMPLATE_TEST_CASE("map CPO integration: modify vertex and edge values",
                   "[dynamic_graph][cpo][integration][map]",
                   mol_tag,
                   mov_tag,
                   mod_tag,
                   mofl_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_int_vv = typename Types::int_vv;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("modify vertex values") {
    auto g = make_sparse_graph_void<Graph_int_vv>();

    // Set values
    for (auto u : vertices(g)) {
      vertex_value(g, u) = static_cast<int>(vertex_id(g, u));
    }

    // Verify
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == static_cast<int>(vertex_id(g, u)));
    }
  }

  SECTION("modify edge values") {
    auto g = make_sparse_graph_int<Graph_int_ev>();

    // Double all edge values
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        edge_value(g, uv) *= 2;
      }
    }

    // Verify sum is doubled
    int total = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        total += edge_value(g, uv);
      }
    }
    REQUIRE(total == sparse_expected::edge_value_sum * 2);
  }
}
