/**
 * @file test_dynamic_graph_cpo_random_access.cpp
 * @brief Consolidated CPO tests for all random-access vertex container types (vov, vod, dov, dod, vol, dol)
 * 
 * Uses template infrastructure from graph_test_types.hpp to test all 6 container
 * types with a single set of test cases.
 * 
 * Note: vol (vector of list) and dol (deque of list) have random-access vertex iteration
 * but bidirectional edge iteration. They're included here because the vertex container
 * uses random-access iterators.
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

//==================================================================================================
// 1. vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertices(g)",
                   "[dynamic_graph][cpo][vertices]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_vv = typename Types::int_vv;

  SECTION("returns vertex_descriptor_view") {
    Graph_void g;
    g.resize_vertices(5);

    auto v_range = vertices(g);

    REQUIRE(std::ranges::size(v_range) == 5);

    size_t count = 0;
    for ([[maybe_unused]] auto v : v_range) {
      ++count;
    }
    REQUIRE(count == 5);
  }

  SECTION("const correctness") {
    const Graph_void g;

    auto v_range = vertices(g);
    REQUIRE(std::ranges::size(v_range) == 0);
  }

  SECTION("with values") {
    Graph_int_vv g;
    g.resize_vertices(3);

    auto v_range = vertices(g);
    REQUIRE(std::ranges::size(v_range) == 3);
  }
}

//==================================================================================================
// 2. num_vertices(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_vertices(g)",
                   "[dynamic_graph][cpo][num_vertices]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_vv = typename Types::int_vv;

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_vertices(g) == 0);
  }

  SECTION("non-empty") {
    Graph_void g;
    g.resize_vertices(10);
    REQUIRE(num_vertices(g) == 10);
  }

  SECTION("matches vertices size") {
    Graph_int_vv g;
    g.resize_vertices(7);
    REQUIRE(num_vertices(g) == std::ranges::size(vertices(g)));
  }
}

//==================================================================================================
// 3. find_vertex(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO find_vertex(g, uid)",
                   "[dynamic_graph][cpo][find_vertex]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("with uint32_t") {
    Graph_void g;
    g.resize_vertices(5);

    auto v = find_vertex(g, uint32_t{2});
    REQUIRE(v != vertices(g).end());
  }

  SECTION("with int") {
    Graph_void g;
    g.resize_vertices(5);

    auto v = find_vertex(g, 3);
    REQUIRE(v != vertices(g).end());
  }

  SECTION("bounds check") {
    Graph_void g;
    g.resize_vertices(3);

    auto v0 = find_vertex(g, 0);
    auto v2 = find_vertex(g, 2);

    REQUIRE(v0 != vertices(g).end());
    REQUIRE(v2 != vertices(g).end());
  }
}

//==================================================================================================
// 4. vertex_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertex_id(g, u)",
                   "[dynamic_graph][cpo][vertex_id]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("basic access") {
    Graph_void g;
    g.resize_vertices(5);

    auto v_range = vertices(g);
    auto v_it    = v_range.begin();
    auto v_desc  = *v_it;

    auto id = vertex_id(g, v_desc);
    REQUIRE(id == 0);
  }

  SECTION("all vertices") {
    Graph_void g;
    g.resize_vertices(10);

    size_t expected_id = 0;
    for (auto v : vertices(g)) {
      REQUIRE(vertex_id(g, v) == expected_id);
      ++expected_id;
    }
  }

  SECTION("const correctness") {
    const Graph_void g;

    // Empty graph - should compile even though no vertices to iterate
    for (auto v : vertices(g)) {
      [[maybe_unused]] auto id = vertex_id(g, v);
    }
    REQUIRE(num_vertices(g) == 0);
  }

  SECTION("with vertex values") {
    using Graph_int_vv = typename Types::int_vv;
    Graph_int_vv g;
    g.resize_vertices(5);

    // Initialize vertex values to their IDs
    for (auto v : vertices(g)) {
      auto id            = vertex_id(g, v);
      vertex_value(g, v) = static_cast<int>(id) * 10;
    }

    // Verify IDs match expected values
    for (auto v : vertices(g)) {
      auto id = vertex_id(g, v);
      REQUIRE(vertex_value(g, v) == static_cast<int>(id) * 10);
    }
  }

  SECTION("with find_vertex") {
    Graph_void g;
    g.resize_vertices(8);

    for (uint32_t expected_id = 0; expected_id < 8; ++expected_id) {
      auto v_it = find_vertex(g, expected_id);
      REQUIRE(v_it != vertices(g).end());

      auto v_desc    = *v_it;
      auto actual_id = vertex_id(g, v_desc);
      REQUIRE(actual_id == expected_id);
    }
  }

  SECTION("sequential iteration") {
    Graph_void g;
    g.resize_vertices(100);

    // Verify IDs are sequential
    auto v_range = vertices(g);
    auto it      = v_range.begin();
    for (size_t expected = 0; expected < 100; ++expected) {
      REQUIRE(it != v_range.end());
      auto v = *it;
      REQUIRE(vertex_id(g, v) == expected);
      ++it;
    }
  }

  SECTION("consistency across calls") {
    Graph_void g;
    g.resize_vertices(5);

    auto v_range = vertices(g);
    auto v_it    = v_range.begin();
    auto v_desc  = *v_it;

    // Call vertex_id multiple times - should be stable
    auto id1 = vertex_id(g, v_desc);
    auto id2 = vertex_id(g, v_desc);
    auto id3 = vertex_id(g, v_desc);

    REQUIRE(id1 == id2);
    REQUIRE(id2 == id3);
  }
}

//==================================================================================================
// 5. num_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_edges(g)",
                   "[dynamic_graph][cpo][num_edges]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(num_edges(g) == 0);
  }

  SECTION("with edges") {
    Graph_void g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(num_edges(g) == 3);
  }

  SECTION("after multiple edge additions") {
    Graph_void g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, void>> ee = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2}};
    g.load_edges(ee, std::identity{}, 4, 0);

    REQUIRE(num_edges(g) == 5);
  }
}

//==================================================================================================
// 6. has_edges(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO has_edges(g)",
                   "[dynamic_graph][cpo][has_edges]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("empty graph") {
    Graph_void g;
    REQUIRE(!has_edges(g));
  }

  SECTION("with edges") {
    Graph_void g({{0, 1}});
    REQUIRE(has_edges(g));
  }

  SECTION("matches num_edges") {
    Graph_void g1;
    Graph_void g2({{0, 1}});

    REQUIRE(has_edges(g1) == (num_edges(g1) > 0));
    REQUIRE(has_edges(g2) == (num_edges(g2) > 0));
  }
}

//==================================================================================================
// 7. num_edges(g, u) CPO Tests - random_access containers support this
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_edges(g, u)",
                   "[dynamic_graph][cpo][num_edges]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("vertex with no edges") {
    Graph_void g;
    g.resize_vertices(3);

    auto u = *find_vertex(g, 0);
    REQUIRE(num_edges(g, u) == 0);
  }

  SECTION("vertex with single edge") {
    Graph_void g({{0, 1}});

    auto u = *find_vertex(g, 0);
    REQUIRE(num_edges(g, u) == 1);
  }

  SECTION("vertex with multiple edges") {
    Graph_void g({{0, 1}, {0, 2}, {0, 3}});

    auto u = *find_vertex(g, 0);
    REQUIRE(num_edges(g, u) == 3);
  }

  SECTION("all vertices") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    REQUIRE(num_edges(g, u0) == 2);
    REQUIRE(num_edges(g, u1) == 1);
    REQUIRE(num_edges(g, u2) == 1);
  }

  SECTION("matches degree") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    for (auto u : vertices(g)) {
      REQUIRE(num_edges(g, u) == degree(g, u));
    }
  }
}

//==================================================================================================
// 7b. num_edges(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_edges(g, uid)",
                   "[dynamic_graph][cpo][num_edges]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("by vertex ID - no edges") {
    Graph_void g;
    g.resize_vertices(3);

    REQUIRE(num_edges(g, 0u) == 0);
  }

  SECTION("by vertex ID - with edges") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    REQUIRE(num_edges(g, 0u) == 2);
    REQUIRE(num_edges(g, 1u) == 1);
    REQUIRE(num_edges(g, 2u) == 0);
  }

  SECTION("consistency with descriptor overload") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);
      REQUIRE(num_edges(g, u) == num_edges(g, uid));
    }
  }
}

//==================================================================================================
// 8. edges(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO edges(g, u)",
                   "[dynamic_graph][cpo][edges]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("returns edge range") {
    Graph_void g({{0, 1}, {0, 2}});

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    static_assert(std::ranges::forward_range<decltype(edge_range)>);

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("empty edge list") {
    Graph_void g;
    g.resize_vertices(3);

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    REQUIRE(edge_range.begin() == edge_range.end());
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 100}, {0, 2, 200}});

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    std::vector<int> values;
    for (auto uv : edge_range) {
      values.push_back(edge_value(g, uv));
    }

    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 100);
    REQUIRE(values[1] == 200);
  }

  SECTION("single edge") {
    Graph_void g({{0, 1}});

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    size_t count = 0;
    for (auto uv : edge_range) {
      REQUIRE(target_id(g, uv) == 1);
      ++count;
    }
    REQUIRE(count == 1);
  }

  SECTION("multiple edges") {
    Graph_void g({{0, 1}, {0, 2}, {0, 3}});

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    std::vector<uint32_t> targets;
    for (auto uv : edge_range) {
      targets.push_back(target_id(g, uv));
    }

    REQUIRE(targets.size() == 3);
    REQUIRE(targets[0] == 1);
    REQUIRE(targets[1] == 2);
    REQUIRE(targets[2] == 3);
  }

  SECTION("const correctness") {
    Graph_void  g({{0, 1}, {0, 2}});
    const auto& const_g = g;

    auto u0         = *find_vertex(const_g, 0);
    auto edge_range = edges(const_g, u0);

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("multiple iterations") {
    Graph_void g({{0, 1}, {0, 2}});

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    // First iteration
    size_t count1 = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count1;
    }

    // Second iteration should work the same
    size_t count2 = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count2;
    }

    REQUIRE(count1 == 2);
    REQUIRE(count2 == 2);
  }

  SECTION("all vertices") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});

    // Check each vertex's edges
    std::vector<size_t> edge_counts;
    for (auto u : vertices(g)) {
      size_t count = 0;
      for ([[maybe_unused]] auto uv : edges(g, u)) {
        ++count;
      }
      edge_counts.push_back(count);
    }

    REQUIRE(edge_counts.size() == 3);
    REQUIRE(edge_counts[0] == 2); // vertex 0 has 2 edges
    REQUIRE(edge_counts[1] == 1); // vertex 1 has 1 edge
    REQUIRE(edge_counts[2] == 1); // vertex 2 has 1 edge
  }

  SECTION("with self-loop") {
    Graph_void g({{0, 0}, {0, 1}});

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    std::vector<uint32_t> targets;
    for (auto uv : edge_range) {
      targets.push_back(target_id(g, uv));
    }

    REQUIRE(targets.size() == 2);
    // Should include self-loop
    REQUIRE((targets[0] == 0 || targets[1] == 0));
    REQUIRE((targets[0] == 1 || targets[1] == 1));
  }

  SECTION("with parallel edges") {
    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    Graph_int_ev                                g;
    g.resize_vertices(2);
    g.load_edges(edge_data);

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    size_t count = 0;
    for (auto uv : edge_range) {
      REQUIRE(target_id(g, uv) == 1);
      ++count;
    }

    // Should return all three parallel edges
    REQUIRE(count == 3);
  }

  SECTION("large graph") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 0; i < 20; ++i) {
      edge_data.push_back({0, i + 1});
    }

    Graph_void g;
    g.resize_vertices(21);
    g.load_edges(edge_data);

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }

    REQUIRE(count == 20);
  }

  SECTION("with string edge values") {
    using Graph_string = typename Types::string_type;
    Graph_string g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {{0, 1, "first"}, {0, 2, "second"}};
    g.load_edges(edge_data);

    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);

    std::vector<std::string> edge_vals;
    for (auto uv : edge_range) {
      edge_vals.push_back(edge_value(g, uv));
    }

    REQUIRE(edge_vals.size() == 2);
    REQUIRE(edge_vals[0] == "first");
    REQUIRE(edge_vals[1] == "second");
  }
}

//==================================================================================================
// 9. edges(g, uid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO edges(g, uid)",
                   "[dynamic_graph][cpo][edges]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("with vertex ID") {
    Graph_void g({{0, 1}, {0, 2}});

    auto edge_range = edges(g, uint32_t(0));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("returns edge_descriptor_view") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    auto edge_range = edges(g, uint32_t(1));

    // Verify return type is edge_descriptor_view
    static_assert(std::ranges::forward_range<decltype(edge_range)>);

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 1);
  }

  SECTION("with isolated vertex") {
    Graph_void g({{0, 1}, {0, 2}});
    g.resize_vertices(4); // Vertex 3 is isolated

    auto edge_range = edges(g, uint32_t(3));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("with different ID types") {
    Graph_void g({{0, 1}, {0, 2}});

    // Test with different integral types
    auto range1 = edges(g, uint32_t(0));
    auto range2 = edges(g, 0); // int literal
    auto range3 = edges(g, size_t(0));

    size_t count1 = 0, count2 = 0, count3 = 0;
    for ([[maybe_unused]] auto uv : range1)
      ++count1;
    for ([[maybe_unused]] auto uv : range2)
      ++count2;
    for ([[maybe_unused]] auto uv : range3)
      ++count3;

    REQUIRE(count1 == 2);
    REQUIRE(count2 == 2);
    REQUIRE(count3 == 2);
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    auto edge_range = edges(g, uint32_t(0));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("with edge values") {
    using Graph_int_ev = typename Types::int_ev;
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 2, 20}};
    g.load_edges(edge_data);

    auto edge_range = edges(g, uint32_t(0));

    std::vector<int> values;
    for (auto uv : edge_range) {
      values.push_back(edge_value(g, uv));
    }

    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 10);
    REQUIRE(values[1] == 20);
  }

  SECTION("multiple vertices") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}, {1, 0}});

    auto edges0 = edges(g, uint32_t(0));
    auto edges1 = edges(g, uint32_t(1));
    auto edges2 = edges(g, uint32_t(2));

    size_t count0 = 0, count1 = 0, count2 = 0;
    for ([[maybe_unused]] auto uv : edges0)
      ++count0;
    for ([[maybe_unused]] auto uv : edges1)
      ++count1;
    for ([[maybe_unused]] auto uv : edges2)
      ++count2;

    REQUIRE(count0 == 2);
    REQUIRE(count1 == 2);
    REQUIRE(count2 == 0);
  }

  SECTION("with parallel edges") {
    using Graph_int_ev = typename Types::int_ev;
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
          {0, 1, 10}, {0, 1, 20}, {0, 1, 30} // 3 parallel edges
    };
    g.load_edges(edge_data);

    auto edge_range = edges(g, uint32_t(0));

    std::vector<int> values;
    for (auto uv : edge_range) {
      values.push_back(edge_value(g, uv));
    }

    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 10);
    REQUIRE(values[1] == 20);
    REQUIRE(values[2] == 30);
  }

  SECTION("consistency with edges(g, u)") {
    using Graph_int_ev = typename Types::int_ev;
    Graph_int_ev g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);

    // Test edges(g, uid) and edges(g, u) give same results
    auto range_by_id   = edges(g, uint32_t(0));
    auto range_by_desc = edges(g, u0);

    std::vector<int> values_by_id;
    std::vector<int> values_by_desc;

    for (auto uv : range_by_id) {
      values_by_id.push_back(edge_value(g, uv));
    }

    for (auto uv : range_by_desc) {
      values_by_desc.push_back(edge_value(g, uv));
    }

    REQUIRE(values_by_id.size() == values_by_desc.size());
    REQUIRE(values_by_id == values_by_desc);
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(50);

    // Add 20 edges from vertex 0
    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i <= 20; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto edge_range = edges(g, uint32_t(0));

    size_t count = 0;
    for ([[maybe_unused]] auto uv : edge_range) {
      ++count;
    }

    REQUIRE(count == 20);
  }
}

//==================================================================================================
// 10. degree(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO degree(g, u)",
                   "[dynamic_graph][cpo][degree]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("isolated vertex") {
    Graph_void g;
    g.resize_vertices(3);

    // Vertices with no edges should have degree 0
    for (auto u : vertices(g)) {
      REQUIRE(degree(g, u) == 0);
    }
  }

  SECTION("single edge") {
    Graph_void g({{0, 1}});

    auto v_range = vertices(g);
    auto v0      = *v_range.begin();

    REQUIRE(degree(g, v0) == 1);
  }

  SECTION("multiple edges from vertex") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {0, 3}, {1, 2}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    // Vertex 0 has 3 outgoing edges
    auto v0 = *vertices(g).begin();
    REQUIRE(degree(g, v0) == 3);

    // Vertex 1 has 1 outgoing edge
    auto v1 = *std::next(vertices(g).begin(), 1);
    REQUIRE(degree(g, v1) == 1);

    // Vertices 2 and 3 have no outgoing edges
    auto v2 = *std::next(vertices(g).begin(), 2);
    auto v3 = *std::next(vertices(g).begin(), 3);
    REQUIRE(degree(g, v2) == 0);
    REQUIRE(degree(g, v3) == 0);
  }

  SECTION("all vertices") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}, {3, 0}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    // Expected degrees: v0=2, v1=2, v2=1, v3=1
    size_t expected_degrees[] = {2u, 2u, 1u, 1u};
    size_t idx                = 0;

    for (auto u : vertices(g)) {
      REQUIRE(degree(g, u) == expected_degrees[idx]);
      ++idx;
    }
  }

  SECTION("const correctness") {
    Graph_void g({{0, 1}, {0, 2}});

    const Graph_void& const_g = g;

    auto v0 = *vertices(const_g).begin();
    REQUIRE(degree(const_g, v0) == 2);
  }

  SECTION("by vertex ID") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {0, 3}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    // Access degree by vertex ID
    REQUIRE(degree(g, uint32_t{0}) == 3);
    REQUIRE(degree(g, uint32_t{1}) == 0);
    REQUIRE(degree(g, uint32_t{2}) == 0);
    REQUIRE(degree(g, uint32_t{3}) == 0);
  }

  SECTION("matches manual count") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {2, 1}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    for (auto u : vertices(g)) {
      auto deg = degree(g, u);

      // Manual count
      size_t manual_count = 0;
      for ([[maybe_unused]] auto e : edges(g, u)) {
        ++manual_count;
      }

      REQUIRE(static_cast<size_t>(deg) == manual_count);
    }
  }

  SECTION("with edge values") {
    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};
    Graph_int_ev                                g;
    g.resize_vertices(3);
    g.load_edges(edge_data);

    auto v0 = *vertices(g).begin();
    auto v1 = *std::next(vertices(g).begin(), 1);
    auto v2 = *std::next(vertices(g).begin(), 2);

    REQUIRE(degree(g, v0) == 2);
    REQUIRE(degree(g, v1) == 1);
    REQUIRE(degree(g, v2) == 0);
  }

  SECTION("self-loop") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {
          {0, 0}, {0, 1} // Self-loop plus normal edge
    };
    Graph_void g;
    g.resize_vertices(2);
    g.load_edges(edge_data);

    auto v0 = *vertices(g).begin();
    REQUIRE(degree(g, v0) == 2); // Self-loop counts as one edge
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    // Create a star graph: vertex 0 connects to all others
    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto v0 = *vertices(g).begin();
    REQUIRE(degree(g, v0) == 99);

    // All other vertices have degree 0
    size_t idx = 0;
    for (auto u : vertices(g)) {
      if (idx > 0) {
        REQUIRE(degree(g, u) == 0);
      }
      ++idx;
    }
  }
}

//==================================================================================================
// 11. target_id(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO target_id(g, uv)",
                   "[dynamic_graph][cpo][target_id]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("basic access") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);
    auto it        = edge_view.begin();

    REQUIRE(it != edge_view.end());
    auto uv0 = *it;
    REQUIRE(target_id(g, uv0) == 1);

    ++it;
    auto uv1 = *it;
    REQUIRE(target_id(g, uv1) == 2);
  }

  SECTION("all edges") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    // Collect all target IDs
    std::vector<uint32_t> targets;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        targets.push_back(target_id(g, uv));
      }
    }

    // Should have 5 edges total
    REQUIRE(targets.size() == 5);

    // Verify all target IDs are valid vertex IDs
    for (auto tid : targets) {
      REQUIRE(tid < num_vertices(g));
    }
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 100}, {0, 2, 200}, {1, 2, 300}});

    // Verify target_id works with edge values present
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto tid = target_id(g, uv);
        REQUIRE(tid < num_vertices(g));
      }
    }
  }

  SECTION("const correctness") {
    Graph_void  g({{0, 1}, {1, 2}});
    const auto& const_g = g;

    auto u0        = *find_vertex(const_g, 0);
    auto edge_view = edges(const_g, u0);

    auto uv  = *edge_view.begin();
    auto tid = target_id(const_g, uv);
    REQUIRE(tid == 1);
  }

  SECTION("self-loop") {
    Graph_void g({{0, 0}, {0, 1}}); // Self-loop and regular edge

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);
    auto it        = edge_view.begin();

    // First edge (0,0) is self-loop
    REQUIRE(target_id(g, *it) == 0);
    ++it;
    // Second edge (0,1)
    REQUIRE(target_id(g, *it) == 1);
  }

  SECTION("parallel edges") {
    // Multiple edges between same vertices
    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    Graph_int_ev                                g;
    g.resize_vertices(2);
    g.load_edges(edge_data);

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);

    // All parallel edges should have same target
    for (auto uv : edge_view) {
      REQUIRE(target_id(g, uv) == 1);
    }
  }

  SECTION("consistency with vertex_id") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto tid = target_id(g, uv);

        // Find target vertex and verify its ID matches
        auto target_vertex = *find_vertex(g, tid);
        REQUIRE(vertex_id(g, target_vertex) == tid);
      }
    }
  }

  SECTION("large graph") {
    // Create a graph with many edges
    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 0; i < 50; ++i) {
      edge_data.push_back({i, (i + 1) % 100});
      edge_data.push_back({i, (i + 2) % 100});
    }

    Graph_void g;
    g.resize_vertices(100);
    g.load_edges(edge_data);

    // Verify all target IDs are valid
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto tid = target_id(g, uv);
        REQUIRE(tid < 100);
      }
    }
  }

  SECTION("with string edge values") {
    using Graph_string = typename Types::string_type;

    std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
          {0, 1, "edge01"}, {0, 2, "edge02"}, {1, 2, "edge12"}};
    Graph_string g;
    g.resize_vertices(3);
    g.load_edges(edge_data);

    // target_id should work regardless of edge value type
    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto tid = target_id(g, uv);
      REQUIRE((tid == 1 || tid == 2));
    }
  }

  SECTION("iteration order") {
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {0, 3}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);

    // Edges appear in insertion order
    std::vector<uint32_t> expected_targets = {1, 2, 3};
    size_t                idx              = 0;

    for (auto uv : edge_view) {
      REQUIRE(target_id(g, uv) == expected_targets[idx]);
      ++idx;
    }
    REQUIRE(idx == 3);
  }
}

//==================================================================================================
// 12. target(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO target(g, uv)",
                   "[dynamic_graph][cpo][target]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;
  using Graph_int_vv = typename Types::int_vv;
  using Graph_string = typename Types::string_type;

  SECTION("basic access") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    // Get edge from vertex 0
    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);
    auto it        = edge_view.begin();

    REQUIRE(it != edge_view.end());
    auto uv = *it;

    // Get target vertex descriptor
    auto target_vertex = target(g, uv);

    // Verify it's the correct vertex
    REQUIRE(vertex_id(g, target_vertex) == 1);
  }

  SECTION("returns vertex descriptor") {
    Graph_void g({{0, 1}, {1, 2}});

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);

    auto uv            = *edge_view.begin();
    auto target_vertex = target(g, uv);

    // Can use it to get vertex_id
    auto tid = vertex_id(g, target_vertex);
    REQUIRE(tid == 1);
  }

  SECTION("consistency with target_id") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}});

    // For all edges, verify target(g,uv) matches find_vertex(g, target_id(g,uv))
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto target_desc   = target(g, uv);
        auto tid           = target_id(g, uv);
        auto expected_desc = *find_vertex(g, tid);

        REQUIRE(vertex_id(g, target_desc) == vertex_id(g, expected_desc));
      }
    }
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 100}, {0, 2, 200}, {1, 2, 300}});

    // target() should work regardless of edge value type
    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto target_vertex = target(g, uv);
      auto tid           = vertex_id(g, target_vertex);
      REQUIRE((tid == 1 || tid == 2));
    }
  }

  SECTION("const correctness") {
    Graph_void  g({{0, 1}, {1, 2}});
    const auto& const_g = g;

    auto u0        = *find_vertex(const_g, 0);
    auto edge_view = edges(const_g, u0);

    auto uv            = *edge_view.begin();
    auto target_vertex = target(const_g, uv);
    REQUIRE(vertex_id(const_g, target_vertex) == 1);
  }

  SECTION("self-loop") {
    Graph_void g({{0, 0}, {0, 1}}); // Self-loop and regular edge

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);
    auto it        = edge_view.begin();

    // First edge (0,0) - self-loop
    auto uv0     = *it;
    auto target0 = target(g, uv0);
    REQUIRE(vertex_id(g, target0) == 0); // Target is same as source

    ++it;
    // Second edge (0,1)
    auto uv1     = *it;
    auto target1 = target(g, uv1);
    REQUIRE(vertex_id(g, target1) == 1);
  }

  SECTION("access target properties") {
    Graph_int_vv g;
    g.resize_vertices(3);

    // Set vertex values
    for (auto u : vertices(g)) {
      vertex_value(g, u) = static_cast<int>(vertex_id(g, u)) * 10;
    }

    // Add edges
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}};
    g.load_edges(edge_data);

    // Access target vertex values through target()
    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto target_vertex = target(g, uv);
      auto target_val    = vertex_value(g, target_vertex);
      auto tid           = vertex_id(g, target_vertex);

      REQUIRE(target_val == static_cast<int>(tid) * 10);
    }
  }

  SECTION("with string vertex values") {
    // Use Graph_string which has string values for VV, EV, and GV
    Graph_string g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {{0, 1, "e01"}, {0, 2, "e02"}, {1, 3, "e13"}};
    g.load_edges(edge_data);

    // Set vertex values
    auto it                = vertices(g).begin();
    vertex_value(g, *it++) = "alpha";
    vertex_value(g, *it++) = "beta";
    vertex_value(g, *it++) = "gamma";
    vertex_value(g, *it++) = "delta";

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto target_vertex = target(g, uv);
      auto tid           = vertex_id(g, target_vertex);
      if (tid == 1) {
        REQUIRE(vertex_value(g, target_vertex) == "beta");
      } else if (tid == 2) {
        REQUIRE(vertex_value(g, target_vertex) == "gamma");
      }
    }
  }

  SECTION("parallel edges") {
    // Multiple edges to same target
    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    Graph_int_ev                                g;
    g.resize_vertices(2);
    g.load_edges(edge_data);

    auto u0        = *find_vertex(g, 0);
    auto edge_view = edges(g, u0);

    // All parallel edges should have same target
    for (auto uv : edge_view) {
      auto target_vertex = target(g, uv);
      REQUIRE(vertex_id(g, target_vertex) == 1);
    }
  }

  SECTION("iteration and navigation") {
    // Create a path graph: 0->1->2->3
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {1, 2}, {2, 3}};
    Graph_void                                   g;
    g.resize_vertices(4);
    g.load_edges(edge_data);

    // Navigate the path using target()
    auto                  current = *find_vertex(g, 0);
    std::vector<uint32_t> path;
    path.push_back(static_cast<uint32_t>(vertex_id(g, current)));

    // Follow edges to build path
    while (true) {
      auto edge_view = edges(g, current);
      auto it        = edge_view.begin();
      if (it == edge_view.end())
        break;

      auto uv = *it;
      current = target(g, uv);
      path.push_back(static_cast<uint32_t>(vertex_id(g, current)));

      if (path.size() >= 4)
        break; // Prevent infinite loop
    }

    // Should have followed path 0->1->2->3
    REQUIRE(path.size() == 4);
    REQUIRE(path[0] == 0);
    REQUIRE(path[1] == 1);
    REQUIRE(path[2] == 2);
    REQUIRE(path[3] == 3);
  }

  SECTION("large graph") {
    // Create a graph with many edges
    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 0; i < 50; ++i) {
      edge_data.push_back({i, (i + 1) % 100});
      edge_data.push_back({i, (i + 2) % 100});
    }

    Graph_void g;
    g.resize_vertices(100);
    g.load_edges(edge_data);

    // Verify target() works for all edges
    size_t edge_count = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto target_vertex = target(g, uv);
        auto tid           = vertex_id(g, target_vertex);
        REQUIRE(tid < 100);
        ++edge_count;
      }
    }

    REQUIRE(edge_count == 100);
  }
}

//==================================================================================================
// 13. find_vertex_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO find_vertex_edge(g, uid, vid)",
                   "[dynamic_graph][cpo][find_vertex_edge]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;
  using Graph_string = typename Types::string_type;

  SECTION("basic usage") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});

    auto e01 = find_vertex_edge(g, 0, 1);
    auto e02 = find_vertex_edge(g, 0, 2);
    auto e12 = find_vertex_edge(g, 1, 2);
    auto e23 = find_vertex_edge(g, 2, 3);

    REQUIRE(target_id(g, e01) == 1);
    REQUIRE(target_id(g, e02) == 2);
    REQUIRE(target_id(g, e12) == 2);
    REQUIRE(target_id(g, e23) == 3);
  }

  SECTION("with edge values") {
    Graph_int_ev g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
    g.load_edges(edge_data);

    auto e01 = find_vertex_edge(g, 0, 1);
    auto e02 = find_vertex_edge(g, 0, 2);
    auto e12 = find_vertex_edge(g, 1, 2);
    auto e23 = find_vertex_edge(g, 2, 3);

    REQUIRE(edge_value(g, e01) == 10);
    REQUIRE(edge_value(g, e02) == 20);
    REQUIRE(edge_value(g, e12) == 30);
    REQUIRE(edge_value(g, e23) == 40);
  }

  SECTION("with parallel edges") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 100}, {0, 1, 200}, {0, 1, 300}, {1, 2, 400}};
    g.load_edges(edge_data);

    auto e01 = find_vertex_edge(g, 0, 1);
    REQUIRE(target_id(g, e01) == 1);

    // The edge value should be one of the parallel edge values
    int val = edge_value(g, e01);
    REQUIRE((val == 100 || val == 200 || val == 300));
  }

  SECTION("with self-loop") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 0, 99}, {0, 1, 10}, {1, 1, 88}};
    g.load_edges(edge_data);

    auto e00 = find_vertex_edge(g, 0, 0);
    auto e11 = find_vertex_edge(g, 1, 1);

    REQUIRE(target_id(g, e00) == 0);
    REQUIRE(edge_value(g, e00) == 99);
    REQUIRE(target_id(g, e11) == 1);
    REQUIRE(edge_value(g, e11) == 88);
  }

  SECTION("const correctness") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 100}, {1, 2, 200}};
    g.load_edges(edge_data);

    const auto& cg = g;

    auto e01 = find_vertex_edge(cg, 0, 1);
    auto e12 = find_vertex_edge(cg, 1, 2);

    REQUIRE(target_id(cg, e01) == 1);
    REQUIRE(edge_value(cg, e01) == 100);
    REQUIRE(target_id(cg, e12) == 2);
    REQUIRE(edge_value(cg, e12) == 200);
  }

  SECTION("with different integral types") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    auto e01_uint32 = find_vertex_edge(g, uint32_t(0), uint32_t(1));
    auto e12_int    = find_vertex_edge(g, 1, 2);
    auto e23_size   = find_vertex_edge(g, size_t(2), size_t(3));

    REQUIRE(target_id(g, e01_uint32) == 1);
    REQUIRE(target_id(g, e12_int) == 2);
    REQUIRE(target_id(g, e23_size) == 3);
  }

  SECTION("with string edge values") {
    Graph_string g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
          {0, 1, "alpha"}, {0, 2, "beta"}, {1, 2, "gamma"}, {2, 3, "delta"}};
    g.load_edges(edge_data);

    auto e01 = find_vertex_edge(g, 0, 1);
    auto e02 = find_vertex_edge(g, 0, 2);
    auto e12 = find_vertex_edge(g, 1, 2);
    auto e23 = find_vertex_edge(g, 2, 3);

    REQUIRE(edge_value(g, e01) == "alpha");
    REQUIRE(edge_value(g, e02) == "beta");
    REQUIRE(edge_value(g, e12) == "gamma");
    REQUIRE(edge_value(g, e23) == "delta");
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto e01  = find_vertex_edge(g, 0, 1);
    auto e050 = find_vertex_edge(g, 0, 50);
    auto e099 = find_vertex_edge(g, 0, 99);

    REQUIRE(target_id(g, e01) == 1);
    REQUIRE(target_id(g, e050) == 50);
    REQUIRE(target_id(g, e099) == 99);
  }

  SECTION("chain of edges") {
    Graph_int_ev g;
    g.resize_vertices(6);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {
          {0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}, {4, 5, 50}};
    g.load_edges(edge_data);

    auto e01 = find_vertex_edge(g, 0, 1);
    REQUIRE(edge_value(g, e01) == 10);

    auto e12 = find_vertex_edge(g, 1, 2);
    REQUIRE(edge_value(g, e12) == 20);

    auto e23 = find_vertex_edge(g, 2, 3);
    REQUIRE(edge_value(g, e23) == 30);

    auto e34 = find_vertex_edge(g, 3, 4);
    REQUIRE(edge_value(g, e34) == 40);

    auto e45 = find_vertex_edge(g, 4, 5);
    REQUIRE(edge_value(g, e45) == 50);
  }

  SECTION("edge not found") {
    Graph_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u2 = *find_vertex(g, 2);

    // Edge from 0 to 2 doesn't exist (only 0->1->2)
    [[maybe_unused]] auto e02 = find_vertex_edge(g, u0, u2);

    // Verify by checking if there's any edge from 0 to 2
    bool found = false;
    for (auto uv : edges(g, u0)) {
      if (target_id(g, uv) == 2) {
        found = true;
        break;
      }
    }
    REQUIRE_FALSE(found);
  }

  SECTION("from isolated vertex") {
    Graph_void g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{1, 2}};
    g.load_edges(edge_data);

    // Vertex 0 is isolated - it has no outgoing edges
    auto u0         = *find_vertex(g, 0);
    auto edge_range = edges(g, u0);
    REQUIRE(std::ranges::empty(edge_range));
  }
}

//==================================================================================================
// 13b. find_vertex_edge(g, u, v) CPO Tests - descriptor overload
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO find_vertex_edge(g, u, v)",
                   "[dynamic_graph][cpo][find_vertex_edge]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;
  using Graph_string = typename Types::string_type;

  SECTION("basic edge found") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    auto e01 = find_vertex_edge(g, u0, u1);
    auto e02 = find_vertex_edge(g, u0, u2);
    auto e12 = find_vertex_edge(g, u1, u2);

    REQUIRE(target_id(g, e01) == 1);
    REQUIRE(target_id(g, e02) == 2);
    REQUIRE(target_id(g, e12) == 2);
  }

  SECTION("with edge values") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 100}, {0, 2, 200}, {1, 2, 300}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    auto e01 = find_vertex_edge(g, u0, u1);
    auto e02 = find_vertex_edge(g, u0, u2);
    auto e12 = find_vertex_edge(g, u1, u2);

    REQUIRE(edge_value(g, e01) == 100);
    REQUIRE(edge_value(g, e02) == 200);
    REQUIRE(edge_value(g, e12) == 300);
  }

  SECTION("with self-loop") {
    Graph_void g({{0, 0}, {0, 1}});

    auto u0  = *find_vertex(g, 0);
    auto e00 = find_vertex_edge(g, u0, u0);

    REQUIRE(target_id(g, e00) == 0);
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {0, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);

    auto e01 = find_vertex_edge(g, u0, u1);

    REQUIRE(target_id(g, e01) == 1);
  }

  SECTION("with parallel edges") {
    Graph_int_ev g;
    g.resize_vertices(2);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);

    auto e01 = find_vertex_edge(g, u0, u1);

    REQUIRE(target_id(g, e01) == 1);
    int val = edge_value(g, e01);
    REQUIRE((val == 10 || val == 20 || val == 30));
  }

  SECTION("with string edge values") {
    Graph_string g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {
          {0, 1, "edge_01"}, {0, 2, "edge_02"}, {1, 2, "edge_12"}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    auto e01 = find_vertex_edge(g, u0, u1);
    auto e02 = find_vertex_edge(g, u0, u2);
    auto e12 = find_vertex_edge(g, u1, u2);

    REQUIRE(edge_value(g, e01) == "edge_01");
    REQUIRE(edge_value(g, e02) == "edge_02");
    REQUIRE(edge_value(g, e12) == "edge_12");
  }

  SECTION("multiple source vertices") {
    Graph_void g({{0, 2}, {1, 2}, {2, 3}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);
    auto u3 = *find_vertex(g, 3);

    auto e02 = find_vertex_edge(g, u0, u2);
    auto e12 = find_vertex_edge(g, u1, u2);
    auto e23 = find_vertex_edge(g, u2, u3);

    REQUIRE(target_id(g, e02) == 2);
    REQUIRE(target_id(g, e12) == 2);
    REQUIRE(target_id(g, e23) == 3);
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto u0  = *find_vertex(g, 0);
    auto u50 = *find_vertex(g, 50);
    auto u99 = *find_vertex(g, 99);

    auto e0_50 = find_vertex_edge(g, u0, u50);
    auto e0_99 = find_vertex_edge(g, u0, u99);

    REQUIRE(target_id(g, e0_50) == 50);
    REQUIRE(target_id(g, e0_99) == 99);
  }

  SECTION("edge not found") {
    Graph_void g({{0, 1}, {1, 2}});

    auto                  u0 = *find_vertex(g, 0);
    [[maybe_unused]] auto u2 = *find_vertex(g, 2);

    // Edge 0->2 doesn't exist - verify by checking all edges from u0
    bool found = false;
    for (auto uv : edges(g, u0)) {
      if (target_id(g, uv) == 2) {
        found = true;
        break;
      }
    }
    REQUIRE_FALSE(found);
  }

  SECTION("with vertex ID") {
    Graph_int_ev g({{0, 1, 100}, {0, 2, 200}, {1, 2, 300}});

    // Using vertex ID overload
    auto e01 = find_vertex_edge(g, 0, 1);
    auto e02 = find_vertex_edge(g, 0, 2);

    REQUIRE(target_id(g, e01) == 1);
    REQUIRE(edge_value(g, e01) == 100);
    REQUIRE(target_id(g, e02) == 2);
    REQUIRE(edge_value(g, e02) == 200);
  }

  SECTION("with different integral types") {
    Graph_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);

    auto e01 = find_vertex_edge(g, u0, u1);
    REQUIRE(target_id(g, e01) == 1);
  }

  SECTION("isolated vertex") {
    Graph_void g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{1, 2}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);

    // Vertex 0 is isolated - it has no outgoing edges
    auto edge_range = edges(g, u0);
    REQUIRE(std::ranges::empty(edge_range));
  }
}

//==================================================================================================
// 14. contains_edge(g, uid, vid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO contains_edge(g, uid, vid)",
                   "[dynamic_graph][cpo][contains_edge]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edge exists") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 0, 2));
    REQUIRE(contains_edge(g, 1, 2));
  }

  SECTION("edge does not exist") {
    Graph_void g({{0, 1}, {1, 2}});

    REQUIRE_FALSE(contains_edge(g, 0, 2));
    REQUIRE_FALSE(contains_edge(g, 1, 0));
    REQUIRE_FALSE(contains_edge(g, 2, 1));
  }

  SECTION("self-loop") {
    Graph_void g({{0, 0}, {0, 1}});

    REQUIRE(contains_edge(g, 0, 0));
    REQUIRE(contains_edge(g, 0, 1));
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 0, 2));
    REQUIRE(contains_edge(g, 1, 2));
    REQUIRE_FALSE(contains_edge(g, 2, 0));
  }

  SECTION("with edge values") {
    Graph_int_ev g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 100}, {0, 2, 200}, {1, 2, 300}};
    g.load_edges(edge_data);

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 0, 2));
    REQUIRE(contains_edge(g, 1, 2));
    REQUIRE_FALSE(contains_edge(g, 0, 3));
    REQUIRE_FALSE(contains_edge(g, 2, 3));
  }

  SECTION("with parallel edges") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}, {1, 2, 40}};
    g.load_edges(edge_data);

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 1, 2));
    REQUIRE_FALSE(contains_edge(g, 0, 2));
  }

  SECTION("empty graph") {
    Graph_void g;
    g.resize_vertices(3);

    REQUIRE_FALSE(contains_edge(g, 0, 1));
    REQUIRE_FALSE(contains_edge(g, 1, 2));
    REQUIRE_FALSE(contains_edge(g, 0, 0));
  }

  SECTION("different integral types") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    REQUIRE(contains_edge(g, uint32_t(0), uint32_t(1)));
    REQUIRE(contains_edge(g, 1, 2));
    REQUIRE(contains_edge(g, size_t(2), size_t(3)));
    REQUIRE_FALSE(contains_edge(g, uint32_t(0), uint32_t(3)));
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 0, 50));
    REQUIRE(contains_edge(g, 0, 99));
    REQUIRE_FALSE(contains_edge(g, 1, 2));
    REQUIRE_FALSE(contains_edge(g, 50, 51));
  }

  SECTION("complete small graph") {
    Graph_void g;
    g.resize_vertices(4);

    // Create a complete graph on 4 vertices
    std::vector<copyable_edge_t<uint32_t, void>> edge_data = {{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3},
                                                              {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}};
    g.load_edges(edge_data);

    // Every pair should have an edge
    for (uint32_t i = 0; i < 4; ++i) {
      for (uint32_t j = 0; j < 4; ++j) {
        if (i != j) {
          REQUIRE(contains_edge(g, i, j));
        }
      }
    }
  }

  SECTION("bidirectional check") {
    Graph_void g({{0, 1}, {1, 0}});

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 1, 0));
  }

  SECTION("star graph") {
    Graph_void g;
    g.resize_vertices(6);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i <= 5; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    // Center to all spokes
    for (uint32_t i = 1; i <= 5; ++i) {
      REQUIRE(contains_edge(g, 0, i));
    }

    // No edges between spokes
    REQUIRE_FALSE(contains_edge(g, 1, 2));
    REQUIRE_FALSE(contains_edge(g, 2, 3));
  }

  SECTION("chain graph") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 1, 2));
    REQUIRE(contains_edge(g, 2, 3));
    REQUIRE(contains_edge(g, 3, 4));

    // Non-adjacent vertices
    REQUIRE_FALSE(contains_edge(g, 0, 2));
    REQUIRE_FALSE(contains_edge(g, 0, 3));
    REQUIRE_FALSE(contains_edge(g, 1, 4));
  }

  SECTION("cycle graph") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE(contains_edge(g, 1, 2));
    REQUIRE(contains_edge(g, 2, 3));
    REQUIRE(contains_edge(g, 3, 0));

    // Non-adjacent in cycle
    REQUIRE_FALSE(contains_edge(g, 0, 2));
    REQUIRE_FALSE(contains_edge(g, 1, 3));
  }

  SECTION("single vertex graph") {
    Graph_void g;
    g.resize_vertices(1);

    REQUIRE_FALSE(contains_edge(g, 0, 0));
  }

  SECTION("single edge graph") {
    Graph_void g({{0, 1}});

    REQUIRE(contains_edge(g, 0, 1));
    REQUIRE_FALSE(contains_edge(g, 1, 0));
  }

  SECTION("all edges not found") {
    Graph_void g({{0, 1}, {1, 2}});

    // Check all possible non-existent edges in opposite directions
    REQUIRE_FALSE(contains_edge(g, 0, 2)); // No transitive edge
    REQUIRE_FALSE(contains_edge(g, 1, 0)); // No reverse
    REQUIRE_FALSE(contains_edge(g, 2, 0)); // No reverse
    REQUIRE_FALSE(contains_edge(g, 2, 1)); // No reverse

    // Self-loops that don't exist
    REQUIRE_FALSE(contains_edge(g, 0, 0));
    REQUIRE_FALSE(contains_edge(g, 1, 1));
    REQUIRE_FALSE(contains_edge(g, 2, 2));
  }
}

//==================================================================================================
// 14b. contains_edge(g, u, v) CPO Tests - descriptor overload
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO contains_edge(g, u, v)",
                   "[dynamic_graph][cpo][contains_edge]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("edge exists") {
    Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    REQUIRE(contains_edge(g, u0, u1));
    REQUIRE(contains_edge(g, u0, u2));
    REQUIRE(contains_edge(g, u1, u2));
  }

  SECTION("edge does not exist") {
    Graph_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    REQUIRE_FALSE(contains_edge(g, u0, u2));
    REQUIRE_FALSE(contains_edge(g, u1, u0));
    REQUIRE_FALSE(contains_edge(g, u2, u1));
  }

  SECTION("with edge values") {
    Graph_int_ev g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 100}, {0, 2, 200}, {1, 2, 300}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);
    auto u3 = *find_vertex(g, 3);

    REQUIRE(contains_edge(g, u0, u1));
    REQUIRE(contains_edge(g, u0, u2));
    REQUIRE(contains_edge(g, u1, u2));
    REQUIRE_FALSE(contains_edge(g, u0, u3));
  }

  SECTION("self-loop") {
    Graph_void g({{0, 0}, {0, 1}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);

    REQUIRE(contains_edge(g, u0, u0));
    REQUIRE(contains_edge(g, u0, u1));
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {0, 2}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    REQUIRE(contains_edge(g, u0, u1));
    REQUIRE(contains_edge(g, u0, u2));
    REQUIRE(contains_edge(g, u1, u2));
  }

  SECTION("with parallel edges") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    auto u1 = *find_vertex(g, 1);
    auto u2 = *find_vertex(g, 2);

    REQUIRE(contains_edge(g, u0, u1));
    REQUIRE_FALSE(contains_edge(g, u0, u2));
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto u0  = *find_vertex(g, 0);
    auto u50 = *find_vertex(g, 50);
    auto u99 = *find_vertex(g, 99);

    REQUIRE(contains_edge(g, u0, u50));
    REQUIRE(contains_edge(g, u0, u99));
    REQUIRE_FALSE(contains_edge(g, u50, u99));
  }
}

//==================================================================================================
// 15. vertex_value(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertex_value(g, u)",
                   "[dynamic_graph][cpo][vertex_value]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_int_vv  = typename Types::int_vv;
  using Graph_string  = typename Types::string_type;
  using Graph_all_int = typename Types::all_int;

  SECTION("basic access") {
    Graph_int_vv g;
    g.resize_vertices(3);

    auto u             = *vertices(g).begin();
    vertex_value(g, u) = 42;
    REQUIRE(vertex_value(g, u) == 42);
  }

  SECTION("multiple vertices") {
    Graph_int_vv g;
    g.resize_vertices(5);

    int val = 0;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val;
      val += 100;
    }

    val = 0;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == val);
      val += 100;
    }
  }

  SECTION("with string values") {
    Graph_string g;
    g.resize_vertices(2);

    auto it              = vertices(g).begin();
    vertex_value(g, *it) = "first";
    ++it;
    vertex_value(g, *it) = "second";

    it = vertices(g).begin();
    REQUIRE(vertex_value(g, *it) == "first");
  }

  SECTION("const correctness") {
    Graph_int_vv g;
    g.resize_vertices(3);

    auto u0             = *vertices(g).begin();
    vertex_value(g, u0) = 99;

    const auto& cg  = g;
    auto        cu0 = *vertices(cg).begin();
    REQUIRE(vertex_value(cg, cu0) == 99);
  }

  SECTION("modification") {
    Graph_int_vv g;
    g.resize_vertices(3);

    auto u0             = *vertices(g).begin();
    vertex_value(g, u0) = 10;
    vertex_value(g, u0) += 5;
    vertex_value(g, u0) *= 2;
    REQUIRE(vertex_value(g, u0) == 30);
  }

  SECTION("default initialization") {
    Graph_int_vv g;
    g.resize_vertices(3);

    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == 0);
    }
  }

  SECTION("large graph") {
    Graph_int_vv g;
    g.resize_vertices(100);

    int val = 0;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val++;
    }

    val = 0;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == val++);
    }
  }

  SECTION("with edges and vertex values") {
    Graph_all_int g({{0, 1, 10}, {1, 2, 20}});

    int val = 100;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val;
      val += 100;
    }

    val = 100;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == val);
      val += 100;
    }
  }
}

//==================================================================================================
// 15b. edge_value(g, uv) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO edge_value(g, uv)",
                   "[dynamic_graph][cpo][edge_value]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_int_ev  = typename Types::int_ev;
  using Graph_all_int = typename Types::all_int;
  using Graph_string  = typename Types::string_type;

  SECTION("basic access") {
    Graph_int_ev g({{0, 1, 42}, {1, 2, 99}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(edge_value(g, uv) == 42);
    }
  }

  SECTION("multiple edges") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 100}, {0, 2, 200}, {1, 2, 300}};
    g.load_edges(edge_data);

    auto             u0 = *find_vertex(g, 0);
    std::vector<int> values;
    for (auto uv : edges(g, u0)) {
      values.push_back(edge_value(g, uv));
    }

    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 100);
    REQUIRE(values[1] == 200);
  }

  SECTION("modification") {
    Graph_all_int g({{0, 1, 50}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(edge_value(g, uv) == 50);
      edge_value(g, uv) = 75;
      REQUIRE(edge_value(g, uv) == 75);
    }
  }

  SECTION("with string values") {
    Graph_string g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, std::string>> edge_data = {{0, 1, "edge01"}, {1, 2, "edge12"}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(edge_value(g, uv) == "edge01");
    }
  }

  SECTION("const correctness") {
    const Graph_int_ev g({{0, 1, 42}, {1, 2, 99}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(edge_value(g, uv) == 42);
    }
  }

  SECTION("with parallel edges") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    g.load_edges(edge_data);

    auto             u0 = *find_vertex(g, 0);
    std::vector<int> values;
    for (auto uv : edges(g, u0)) {
      values.push_back(edge_value(g, uv));
    }

    REQUIRE(values.size() == 3);
    REQUIRE(values[0] == 10);
    REQUIRE(values[1] == 20);
    REQUIRE(values[2] == 30);
  }

  SECTION("with self-loop") {
    Graph_int_ev g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 0, 99}, {0, 1, 10}};
    g.load_edges(edge_data);

    auto             u0 = *find_vertex(g, 0);
    std::vector<int> values;
    for (auto uv : edges(g, u0)) {
      values.push_back(edge_value(g, uv));
    }

    REQUIRE(values.size() == 2);
    REQUIRE(values[0] == 99);
    REQUIRE(values[1] == 10);
  }

  SECTION("large graph") {
    Graph_int_ev g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i, static_cast<int>(i * 10)});
    }
    g.load_edges(edge_data);

    auto u0       = *find_vertex(g, 0);
    int  expected = 10;
    for (auto uv : edges(g, u0)) {
      REQUIRE(edge_value(g, uv) == expected);
      expected += 10;
    }
  }

  SECTION("iteration over all edges") {
    Graph_int_ev g;
    g.resize_vertices(4);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
    g.load_edges(edge_data);

    int sum = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        sum += edge_value(g, uv);
      }
    }

    REQUIRE(sum == 100); // 10 + 20 + 30 + 40
  }
}

//==================================================================================================
// 16. graph_value(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO graph_value(g)",
                   "[dynamic_graph][cpo][graph_value]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;
  using Graph_string  = typename Types::string_type;

  SECTION("basic access") {
    Graph_all_int g({{0, 1, 1}});

    graph_value(g) = 42;
    REQUIRE(graph_value(g) == 42);
  }

  SECTION("default initialization") {
    Graph_all_int g;
    REQUIRE(graph_value(g) == 0);
  }

  SECTION("modification") {
    Graph_all_int g({{0, 1, 1}});

    graph_value(g) = 10;
    graph_value(g) += 5;
    REQUIRE(graph_value(g) == 15);
  }

  SECTION("const correctness") {
    Graph_all_int g;
    graph_value(g) = 99;

    const auto& cg = g;
    REQUIRE(graph_value(cg) == 99);
  }

  SECTION("with vertices and edges") {
    Graph_all_int g({{0, 1, 10}, {1, 2, 20}, {2, 3, 30}});

    graph_value(g) = 999;
    REQUIRE(graph_value(g) == 999);
    REQUIRE(num_vertices(g) == 4);
    REQUIRE(num_edges(g) == 3);
  }

  SECTION("large value") {
    Graph_all_int g;
    graph_value(g) = std::numeric_limits<int>::max();
    REQUIRE(graph_value(g) == std::numeric_limits<int>::max());
  }

  SECTION("with string values") {
    // Use Graph_string which has string GV
    Graph_string g;
    graph_value(g) = "test_graph";
    REQUIRE(graph_value(g) == "test_graph");

    graph_value(g) = "updated";
    REQUIRE(graph_value(g) == "updated");
  }

  SECTION("independent of vertices/edges") {
    Graph_all_int g;

    graph_value(g) = 100;
    REQUIRE(graph_value(g) == 100);

    // Add vertices and edges
    g.resize_vertices(5);
    REQUIRE(graph_value(g) == 100);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(edge_data);
    REQUIRE(graph_value(g) == 100);

    // Modify graph value independently
    graph_value(g) = 200;
    REQUIRE(graph_value(g) == 200);
    REQUIRE(num_vertices(g) == 5);
    REQUIRE(num_edges(g) == 2);
  }
}

//==================================================================================================
// 17. partition_id(g, u) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO partition_id(g, u)",
                   "[dynamic_graph][cpo][partition_id]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("default single partition") {
    Graph_void g;
    g.resize_vertices(5);

    for (auto u : vertices(g)) {
      REQUIRE(partition_id(g, u) == 0);
    }
  }

  SECTION("with edges") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    for (auto u : vertices(g)) {
      REQUIRE(partition_id(g, u) == 0);
    }
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {1, 2}});

    for (auto u : vertices(g)) {
      REQUIRE(partition_id(g, u) == 0);
    }
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 10}, {1, 2, 20}});

    for (auto u : vertices(g)) {
      REQUIRE(partition_id(g, u) == 0);
    }
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    for (auto u : vertices(g)) {
      REQUIRE(partition_id(g, u) == 0);
    }
  }
}

//==================================================================================================
// 18. num_partitions(g) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_partitions(g)",
                   "[dynamic_graph][cpo][num_partitions]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("default value") {
    Graph_void g;
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("with vertices") {
    Graph_void g({{0, 1}, {1, 2}});
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}});
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 10}, {1, 2, 20}});
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);
    REQUIRE(num_partitions(g) == 1);
  }

  SECTION("with vertices and edges") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    REQUIRE(num_partitions(g) == 1);
    REQUIRE(num_vertices(g) == 4);
    REQUIRE(num_edges(g) == 3);
  }

  SECTION("consistency with partition_id") {
    Graph_void g({{0, 1}, {1, 2}});

    size_t np = num_partitions(g);
    REQUIRE(np == 1);

    for (auto u : vertices(g)) {
      auto pid = partition_id(g, u);
      REQUIRE(static_cast<size_t>(pid) < np);
    }
  }
}

//==================================================================================================
// 19. vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO vertices(g, pid)",
                   "[dynamic_graph][cpo][vertices][partition]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("partition 0 returns all vertices") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    auto verts_all = vertices(g);
    auto verts_p0  = vertices(g, 0);

    REQUIRE(std::ranges::distance(verts_all) == std::ranges::distance(verts_p0));
  }

  SECTION("non-zero partition returns empty") {
    Graph_void g({{0, 1}, {1, 2}});

    auto verts_p1 = vertices(g, 1);
    REQUIRE(std::ranges::distance(verts_p1) == 0);
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {1, 2}});

    auto verts_p0 = vertices(g, 0);
    REQUIRE(std::ranges::distance(verts_p0) == 3);
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 10}, {1, 2, 20}});

    auto verts_p0 = vertices(g, 0);
    REQUIRE(std::ranges::distance(verts_p0) == 3);
  }

  SECTION("iterate partition vertices") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

    size_t count = 0;
    for ([[maybe_unused]] auto u : vertices(g, 0)) {
      ++count;
    }
    REQUIRE(count == 5);
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    auto verts_p0 = vertices(g, 0);
    REQUIRE(std::ranges::distance(verts_p0) == 100);
  }
}

//==================================================================================================
// 20. num_vertices(g, pid) CPO Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO num_vertices(g, pid)",
                   "[dynamic_graph][cpo][num_vertices][partition]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types        = graph_test_types<TestType>;
  using Graph_void   = typename Types::void_type;
  using Graph_int_ev = typename Types::int_ev;

  SECTION("partition 0 returns total count") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    REQUIRE(num_vertices(g, 0) == num_vertices(g));
  }

  SECTION("non-zero partition returns zero") {
    Graph_void g({{0, 1}, {1, 2}});

    REQUIRE(num_vertices(g, 1) == 0);
  }

  SECTION("const correctness") {
    const Graph_void g({{0, 1}, {1, 2}});

    REQUIRE(num_vertices(g, 0) == 3);
  }

  SECTION("with edge values") {
    Graph_int_ev g({{0, 1, 10}, {1, 2, 20}});

    REQUIRE(num_vertices(g, 0) == 3);
  }

  SECTION("empty graph") {
    Graph_void g;

    REQUIRE(num_vertices(g, 0) == 0);
  }

  SECTION("large graph") {
    Graph_void g;
    g.resize_vertices(100);

    REQUIRE(num_vertices(g, 0) == 100);
    REQUIRE(num_vertices(g, 1) == 0);
  }

  SECTION("consistency with vertices(g, pid)") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    auto nv0      = num_vertices(g, 0);
    auto verts_p0 = vertices(g, 0);

    REQUIRE(nv0 == static_cast<size_t>(std::ranges::distance(verts_p0)));
  }
}

//==================================================================================================
// 21. source_id(g, uv) CPO Tests - Sourced graphs only
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO source_id(g, uv)",
                   "[dynamic_graph][cpo][source_id]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types              = graph_test_types<TestType>;
  using Graph_sourced_void = typename Types::sourced_void;
  using Graph_sourced_int  = typename Types::sourced_int;

  SECTION("basic usage") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {0, 2}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("different sources") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});

    for (size_t i = 0; i < 3; ++i) {
      auto u = *find_vertex(g, i);
      for (auto uv : edges(g, u)) {
        REQUIRE(source_id(g, uv) == i);
      }
    }
  }

  SECTION("const correctness") {
    const Graph_sourced_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("with edge values") {
    Graph_sourced_int g({{0, 1, 10}, {1, 2, 20}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
      REQUIRE(edge_value(g, uv) == 10);
    }
  }

  SECTION("with self-loop") {
    Graph_sourced_void g({{0, 0}, {0, 1}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("with parallel edges") {
    Graph_sourced_int g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("large graph") {
    Graph_sourced_void g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("multiple edges from same source") {
    Graph_sourced_void g({{0, 1}, {0, 2}, {0, 3}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("star graph") {
    Graph_sourced_void g;
    g.resize_vertices(6);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i <= 5; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
    }
  }

  SECTION("chain graph") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});

    for (size_t i = 0; i < 3; ++i) {
      auto u = *find_vertex(g, i);
      for (auto uv : edges(g, u)) {
        REQUIRE(source_id(g, uv) == i);
      }
    }
  }

  SECTION("cycle graph") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});

    for (size_t i = 0; i < 4; ++i) {
      auto u = *find_vertex(g, i);
      for (auto uv : edges(g, u)) {
        REQUIRE(source_id(g, uv) == i);
      }
    }
  }

  SECTION("consistency with source(g, uv)") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == source_id(g, uv));
      }
    }
  }

  SECTION("self-loops") {
    Graph_sourced_void g({{0, 0}, {1, 1}});

    // Self-loops: source and target are the same
    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      REQUIRE(source_id(g, uv) == 0);
      REQUIRE(target_id(g, uv) == 0);
    }

    auto u1 = *find_vertex(g, 1);
    for (auto uv : edges(g, u1)) {
      REQUIRE(source_id(g, uv) == 1);
      REQUIRE(target_id(g, uv) == 1);
    }
  }
}

//==================================================================================================
// 22. source(g, uv) CPO Tests - Sourced graphs only
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO source(g, uv)",
                   "[dynamic_graph][cpo][source]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types              = graph_test_types<TestType>;
  using Graph_sourced_void = typename Types::sourced_void;
  using Graph_sourced_int  = typename Types::sourced_int;

  SECTION("basic usage") {
    Graph_sourced_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("consistency with source_id") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == source_id(g, uv));
      }
    }
  }

  SECTION("const correctness") {
    const Graph_sourced_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("with edge values") {
    Graph_sourced_int g({{0, 1, 100}, {1, 2, 200}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
      REQUIRE(edge_value(g, uv) == 100);
    }
  }

  SECTION("with self-loop") {
    Graph_sourced_void g({{0, 0}, {0, 1}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("different sources") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});

    for (size_t i = 0; i < 4; ++i) {
      auto u = *find_vertex(g, i);
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == i);
      }
    }
  }

  SECTION("large graph") {
    Graph_sourced_void g;
    g.resize_vertices(100);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i < 100; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("returns valid descriptor") {
    Graph_sourced_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      auto sid = vertex_id(g, src);
      REQUIRE(sid < num_vertices(g));
    }
  }

  SECTION("with vertex values") {
    // Use sourced_all which has VV=int and Sourced=true
    using Graph_sourced_all = typename Types::sourced_all;

    Graph_sourced_all g({{0, 1, 10}, {1, 2, 20}});

    int val = 100;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val;
      val += 100;
    }

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_value(g, src) == 100);
    }
  }

  SECTION("parallel edges") {
    Graph_sourced_int g;
    g.resize_vertices(2);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("chain graph") {
    Graph_sourced_void g({{0, 1}, {1, 2}, {2, 3}});

    for (size_t i = 0; i < 3; ++i) {
      auto u = *find_vertex(g, i);
      for (auto uv : edges(g, u)) {
        auto src = source(g, uv);
        REQUIRE(vertex_id(g, src) == i);
      }
    }
  }

  SECTION("star graph") {
    Graph_sourced_void g;
    g.resize_vertices(6);

    std::vector<copyable_edge_t<uint32_t, void>> edge_data;
    for (uint32_t i = 1; i <= 5; ++i) {
      edge_data.push_back({0, i});
    }
    g.load_edges(edge_data);

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
    }
  }

  SECTION("can traverse from source to target") {
    Graph_sourced_void g({{0, 1}, {1, 2}});

    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      auto tgt = target(g, uv);

      REQUIRE(vertex_id(g, src) == 0);
      REQUIRE(vertex_id(g, tgt) == 1);
    }
  }

  SECTION("accumulate values from edges") {
    Graph_sourced_int g({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}});

    int sum = 0;
    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        sum += edge_value(g, uv);
      }
    }

    REQUIRE(sum == 60);
  }

  SECTION("self-loops") {
    Graph_sourced_void g({{0, 0}, {1, 1}});

    // For self-loops, source and target should be the same vertex
    auto u0 = *find_vertex(g, 0);
    for (auto uv : edges(g, u0)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 0);
      REQUIRE(vertex_id(g, src) == target_id(g, uv));
    }

    auto u1 = *find_vertex(g, 1);
    for (auto uv : edges(g, u1)) {
      auto src = source(g, uv);
      REQUIRE(vertex_id(g, src) == 1);
      REQUIRE(vertex_id(g, src) == target_id(g, uv));
    }
  }
}

//==================================================================================================
// 23. Integration Tests
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO integration",
                   "[dynamic_graph][cpo][integration]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types      = graph_test_types<TestType>;
  using Graph_void = typename Types::void_type;

  SECTION("graph construction and traversal") {
    Graph_void g({{0, 1}, {1, 2}});

    REQUIRE(num_vertices(g) == 3);
    REQUIRE(num_edges(g) == 2);
    REQUIRE(has_edges(g));
  }

  SECTION("empty graph properties") {
    Graph_void g;

    REQUIRE(num_vertices(g) == 0);
    REQUIRE(num_edges(g) == 0);
    REQUIRE(!has_edges(g));
  }

  SECTION("find vertex by id") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    for (size_t i = 0; i < num_vertices(g); ++i) {
      auto u = *find_vertex(g, i);
      REQUIRE(vertex_id(g, u) == i);
    }
  }

  SECTION("vertices and num_vertices consistency") {
    Graph_void g({{0, 1}, {1, 2}, {2, 3}});

    size_t count = 0;
    for ([[maybe_unused]] auto u : vertices(g)) {
      ++count;
    }

    REQUIRE(count == num_vertices(g));
  }

  SECTION("const graph access") {
    const Graph_void g({{0, 1}, {1, 2}});

    REQUIRE(num_vertices(g) == 3);
    REQUIRE(num_edges(g) == 2);

    for (auto u : vertices(g)) {
      auto uid = vertex_id(g, u);
      REQUIRE(uid < num_vertices(g));

      for (auto uv : edges(g, u)) {
        auto tid = target_id(g, uv);
        REQUIRE(tid < num_vertices(g));
      }
    }
  }
}

//==================================================================================================
// 24. Integration Tests - Values
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO integration: values",
                   "[dynamic_graph][cpo][integration]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;

  SECTION("vertex values only") {
    Graph_all_int g;
    g.resize_vertices(5);

    int val = 0;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val;
      val += 100;
    }

    val = 0;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == val);
      val += 100;
    }
  }

  SECTION("vertex and edge values") {
    Graph_all_int g;
    g.resize_vertices(3);

    std::vector<copyable_edge_t<uint32_t, int>> edge_data = {{0, 1, 5}, {1, 2, 10}};
    g.load_edges(edge_data);

    int val = 0;
    for (auto u : vertices(g)) {
      vertex_value(g, u) = val;
      val += 100;
    }

    val = 0;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == val);
      val += 100;
    }
  }
}

//==================================================================================================
// 25. Integration Tests - Modify Vertex and Edge Values
//==================================================================================================

TEMPLATE_TEST_CASE("random_access CPO integration: modify vertex and edge values",
                   "[dynamic_graph][cpo][integration]",
                   vov_tag,
                   vod_tag,
                   dov_tag,
                   dod_tag,
                   vol_tag,
                   dol_tag) {
  using Types         = graph_test_types<TestType>;
  using Graph_all_int = typename Types::all_int;

  SECTION("accumulate edge values into vertices") {
    Graph_all_int g({{0, 1, 1}, {1, 2, 2}});

    for (auto u : vertices(g)) {
      vertex_value(g, u) = 0;
    }

    for (auto u : vertices(g)) {
      for (auto uv : edges(g, u)) {
        vertex_value(g, u) += edge_value(g, uv);
      }
    }

    int expected_values[] = {1, 2, 0};
    int idx               = 0;
    for (auto u : vertices(g)) {
      REQUIRE(vertex_value(g, u) == expected_values[idx]);
      ++idx;
      if (idx >= 3)
        break;
    }
  }
}
