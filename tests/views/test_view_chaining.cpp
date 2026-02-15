/**
 * @file test_view_chaining.cpp
 * @brief Tests demonstrating std::views chaining with graph views carrying value functions
 *
 * This is the key validation for the VVF/EVF parameter-based signature change.
 * Previously, value functions captured the graph by reference ([&g](auto v) {...}),
 * making lambdas non-semiregular and preventing std::views chaining. Now that value
 * functions receive the graph as a parameter ([](const auto& g, auto v) {...}),
 * stateless lambdas are default_initializable, copyable, and semiregular — enabling
 * full composition with std::views::take, filter, transform, etc.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/vertexlist.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/edgelist.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/bfs.hpp>
#include <graph/views/topological_sort.hpp>
#include <graph/views/adaptors.hpp>

#include <ranges>
#include <vector>
#include <set>
#include <concepts>
#include <algorithm>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Helper graphs
// =============================================================================

// Simple directed graph: 0->1, 0->2, 1->2
auto make_simple_graph() { return std::vector<std::vector<int>>{{1, 2}, {2}, {}}; }

// Larger graph: 0->1, 0->2, 1->3, 2->3, 3->4
auto make_chain_graph() { return std::vector<std::vector<int>>{{1, 2}, {3}, {3}, {4}, {}}; }

// DAG for topological sort: 0->1, 0->2, 1->3, 2->3
auto make_dag() { return std::vector<std::vector<int>>{{1, 2}, {3}, {3}, {}}; }

// =============================================================================
// Part 1: Semiregular Concept Verification
// =============================================================================

TEST_CASE("chaining concepts - vertexlist with VVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  using ViewType = decltype(vertexlist(g, vvf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - incidence with EVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  using ViewType = decltype(incidence(g, vertex_id_t<decltype(g)>{0}, evf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - neighbors with VVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto vvf = [](const auto&, auto v) { return static_cast<int>(v.vertex_id()); };

  using ViewType = decltype(neighbors(g, vertex_id_t<decltype(g)>{0}, vvf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - edgelist with EVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  using ViewType = decltype(edgelist(g, evf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - vertices_dfs with VVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  using ViewType = decltype(vertices_dfs(g, 0, vvf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - edges_dfs with EVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  using ViewType = decltype(edges_dfs(g, 0, evf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - vertices_bfs with VVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  using ViewType = decltype(vertices_bfs(g, 0, vvf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - edges_bfs with EVF is semiregular", "[chaining][concepts]") {
  auto g   = make_simple_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  using ViewType = decltype(edges_bfs(g, 0, evf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

TEST_CASE("chaining concepts - vertices_topological_sort with VVF is semiregular", "[chaining][concepts]") {
  auto g   = make_dag();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  using ViewType = decltype(vertices_topological_sort(g, vvf));

  STATIC_REQUIRE(std::ranges::range<ViewType>);
  STATIC_REQUIRE(std::ranges::view<ViewType>);
  STATIC_REQUIRE(std::semiregular<ViewType>);
  STATIC_REQUIRE(std::default_initializable<ViewType>);
  STATIC_REQUIRE(std::copyable<ViewType>);
}

// =============================================================================
// Part 2: Vertexlist Chaining
// =============================================================================

TEST_CASE("chaining - vertexlist VVF with std::views::take", "[chaining][vertexlist]") {
  auto g   = make_chain_graph(); // 5 vertices
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)) * 10; };

  auto view = vertexlist(g, vvf) | std::views::take(3);

  std::vector<int> values;
  for (auto [id, v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector<int>{0, 10, 20});
}

TEST_CASE("chaining - vertexlist VVF with std::views::filter", "[chaining][vertexlist]") {
  auto g   = make_chain_graph(); // 5 vertices
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertexlist(g, vvf) | std::views::filter([](auto info) {
                auto [id, v, val] = info;
                return val % 2 == 0; // even vertex IDs only
              });

  std::vector<int> values;
  for (auto [id, v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector<int>{0, 2, 4});
}

TEST_CASE("chaining - vertexlist VVF with std::views::transform", "[chaining][vertexlist]") {
  auto g   = make_simple_graph(); // 3 vertices
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertexlist(g, vvf) | std::views::transform([](auto info) {
                auto [id, v, val] = info;
                return val * 100;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result == std::vector<int>{0, 100, 200});
}

TEST_CASE("chaining - vertexlist VVF multi-stage pipeline", "[chaining][vertexlist]") {
  auto g   = make_chain_graph(); // 5 vertices: 0..4
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertexlist(g, vvf) | std::views::filter([](auto info) {
                auto [id, v, val] = info;
                return val > 0; // skip vertex 0
              }) |
              std::views::transform([](auto info) {
                auto [id, v, val] = info;
                return val * 10; // scale up
              }) |
              std::views::take(2); // first 2

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result == std::vector<int>{10, 20});
}

// =============================================================================
// Part 3: Incidence Chaining
// =============================================================================

TEST_CASE("chaining - incidence EVF with std::views::take", "[chaining][incidence]") {
  auto g   = make_chain_graph(); // vertex 0 has edges to 1, 2
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = incidence(g, vertex_id_t<decltype(g)>{0}, evf) | std::views::take(1);

  std::vector<int> values;
  for (auto [tid, e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 1);
  REQUIRE(values[0] == 1);
}

TEST_CASE("chaining - incidence EVF with std::views::transform", "[chaining][incidence]") {
  auto g   = make_chain_graph(); // vertex 0 has edges to 1, 2
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = incidence(g, vertex_id_t<decltype(g)>{0}, evf) | std::views::transform([](auto info) {
                auto [tid, e, val] = info;
                return val * 100;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result == std::vector<int>{100, 200});
}

// =============================================================================
// Part 4: Neighbors Chaining
// =============================================================================

TEST_CASE("chaining - neighbors VVF with std::views::filter", "[chaining][neighbors]") {
  auto g   = make_chain_graph(); // vertex 0 neighbors: 1, 2
  auto vvf = [](const auto&, auto v) { return static_cast<int>(v.vertex_id()); };

  auto view = neighbors(g, vertex_id_t<decltype(g)>{0}, vvf) | std::views::filter([](auto info) {
                auto [tid, v, val] = info;
                return val > 1; // only neighbor 2
              });

  std::vector<int> result;
  for (auto [tid, v, val] : view) {
    result.push_back(val);
  }

  REQUIRE(result == std::vector<int>{2});
}

TEST_CASE("chaining - neighbors VVF with std::views::transform and take", "[chaining][neighbors]") {
  auto g   = make_chain_graph();
  auto vvf = [](const auto&, auto v) { return static_cast<int>(v.vertex_id()) * 5; };

  auto view = neighbors(g, vertex_id_t<decltype(g)>{0}, vvf) | std::views::transform([](auto info) {
                auto [tid, v, val] = info;
                return val;
              }) |
              std::views::take(1);

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result == std::vector<int>{5});
}

// =============================================================================
// Part 5: Edgelist Chaining
// =============================================================================

TEST_CASE("chaining - edgelist EVF with std::views::take", "[chaining][edgelist]") {
  auto g   = make_simple_graph(); // edges: 0->1, 0->2, 1->2
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = edgelist(g, evf) | std::views::take(2);

  std::vector<int> values;
  for (auto [sid, tid, e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 2);
  REQUIRE(values[0] == 1);
  REQUIRE(values[1] == 2);
}

TEST_CASE("chaining - edgelist EVF with std::views::filter", "[chaining][edgelist]") {
  auto g   = make_chain_graph(); // 5 edges total
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = edgelist(g, evf) | std::views::filter([](auto info) {
                auto [sid, tid, e, val] = info;
                return val >= 3; // only edges targeting vertex 3 or 4
              });

  std::vector<int> values;
  for (auto [sid, tid, e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3); // 1->3, 2->3, 3->4
  std::set<int> value_set(values.begin(), values.end());
  REQUIRE(value_set == std::set<int>{3, 4});
}

// =============================================================================
// Part 6: DFS Chaining
// =============================================================================

TEST_CASE("chaining - vertices_dfs VVF with std::views::take", "[chaining][dfs]") {
  auto g   = make_chain_graph(); // 0->1->3->4, 0->2->3
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  // After renaming size() to num_visited(), DFS views no longer satisfy
  // sized_range, so std::views::take works correctly via sentinel comparison.
  auto view = vertices_dfs(g, 0, vvf) | std::views::take(3);

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
}

TEST_CASE("chaining - vertices_dfs VVF with std::views::filter", "[chaining][dfs]") {
  auto g   = make_chain_graph(); // 0->1->3->4, 0->2->3
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_dfs(g, 0, vvf) | std::views::filter([](auto info) {
                auto [v, val] = info;
                return val > 0; // skip source vertex 0
              });

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(!values.empty());
  for (auto val : values) {
    REQUIRE(val > 0);
  }
}

TEST_CASE("chaining - vertices_dfs VVF with std::views::transform", "[chaining][dfs]") {
  auto g   = make_simple_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_dfs(g, 0, vvf) | std::views::transform([](auto info) {
                auto [v, val] = info;
                return val * 10;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result.size() == 3);
  // All values should be multiples of 10
  for (auto val : result) {
    REQUIRE(val % 10 == 0);
  }
}

TEST_CASE("chaining - edges_dfs EVF with std::views::filter", "[chaining][dfs]") {
  auto g   = make_chain_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = edges_dfs(g, 0, evf) | std::views::filter([](auto info) {
                auto [e, val] = info;
                return val >= 3; // only edges to vertex 3+
              });

  std::vector<int> values;
  for (auto [e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(!values.empty());
  for (auto val : values) {
    REQUIRE(val >= 3);
  }
}

// =============================================================================
// Part 7: BFS Chaining
// =============================================================================

TEST_CASE("chaining - vertices_bfs VVF with std::views::take", "[chaining][bfs]") {
  auto g   = make_chain_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_bfs(g, 0, vvf) | std::views::take(3);

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
}

TEST_CASE("chaining - vertices_bfs VVF with std::views::filter", "[chaining][bfs]") {
  auto g   = make_chain_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_bfs(g, 0, vvf) | std::views::filter([](auto info) {
                auto [v, val] = info;
                return val > 0; // skip source vertex 0
              });

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(!values.empty());
  for (auto val : values) {
    REQUIRE(val > 0);
  }
}

TEST_CASE("chaining - vertices_bfs VVF with std::views::filter even IDs", "[chaining][bfs]") {
  auto g   = make_chain_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_bfs(g, 0, vvf) | std::views::filter([](auto info) {
                auto [v, val] = info;
                return val % 2 == 0; // even IDs only
              });

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(!values.empty());
  for (auto val : values) {
    REQUIRE(val % 2 == 0);
  }
}

TEST_CASE("chaining - edges_bfs EVF with std::views::transform", "[chaining][bfs]") {
  auto g   = make_simple_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = edges_bfs(g, 0, evf) | std::views::transform([](auto info) {
                auto [e, val] = info;
                return val * 100;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result.size() == 2); // 2 edges from BFS starting at 0
  for (auto val : result) {
    REQUIRE(val % 100 == 0);
  }
}

// =============================================================================
// Part 8: Topological Sort Chaining
// =============================================================================

TEST_CASE("chaining - vertices_topological_sort VVF with std::views::take", "[chaining][topological_sort]") {
  auto g   = make_dag(); // 4 vertices in DAG
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  // Topological sort views have a valid size() (eagerly computed), so take
  // works whether or not sized_range is satisfied.
  auto view = vertices_topological_sort(g, vvf) | std::views::take(2);

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 2);
}

TEST_CASE("chaining - vertices_topological_sort VVF with std::views::transform", "[chaining][topological_sort]") {
  auto g   = make_dag(); // 4 vertices in DAG
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_topological_sort(g, vvf) | std::views::transform([](auto info) {
                auto [v, val] = info;
                return val * 100;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result.size() == 4);
  for (auto val : result) {
    REQUIRE(val % 100 == 0);
  }
}

TEST_CASE("chaining - vertices_topological_sort VVF with std::views::filter", "[chaining][topological_sort]") {
  auto g   = make_dag();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_topological_sort(g, vvf) | std::views::filter([](auto info) {
                auto [v, val] = info;
                return val > 0;
              });

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  // All non-zero vertex IDs
  REQUIRE(!values.empty());
  for (auto val : values) {
    REQUIRE(val > 0);
  }
}

TEST_CASE("chaining - vertices_topological_sort VVF multi-stage", "[chaining][topological_sort]") {
  auto g   = make_dag();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = vertices_topological_sort(g, vvf) | std::views::filter([](auto info) {
                auto [v, val] = info;
                return val < 3; // exclude vertex 3
              }) |
              std::views::transform([](auto info) {
                auto [v, val] = info;
                return val * 10;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result.size() == 3); // vertices 0, 1, 2 (3 excluded)
  for (auto val : result) {
    REQUIRE(val % 10 == 0);
  }
}

// =============================================================================
// Part 9: Pipe Syntax Chaining (via adaptors)
// =============================================================================

TEST_CASE("chaining - pipe syntax vertexlist VVF with std::views::take", "[chaining][pipe]") {
  namespace adaptors = graph::views::adaptors;

  auto g   = make_chain_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)) * 10; };

  auto view = g | adaptors::vertexlist(vvf) | std::views::take(3);

  std::vector<int> values;
  for (auto [id, v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector<int>{0, 10, 20});
}

TEST_CASE("chaining - pipe syntax vertexlist VVF filter + transform", "[chaining][pipe]") {
  namespace adaptors = graph::views::adaptors;

  auto g   = make_chain_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view = g | adaptors::vertexlist(vvf) | std::views::filter([](auto info) {
                auto [id, v, val] = info;
                return val >= 2;
              }) |
              std::views::transform([](auto info) {
                auto [id, v, val] = info;
                return val * 100;
              });

  std::vector<int> result;
  for (auto val : view) {
    result.push_back(val);
  }

  REQUIRE(result == std::vector<int>{200, 300, 400});
}

TEST_CASE("chaining - pipe syntax incidence EVF with take", "[chaining][pipe]") {
  namespace adaptors = graph::views::adaptors;

  auto g   = make_chain_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view = g | adaptors::incidence(0, evf) | std::views::take(1);

  std::vector<int> values;
  for (auto [tid, e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 1);
  REQUIRE(values[0] == 1);
}

// =============================================================================
// Part 10: View Copy Semantics (semiregular proof)
// =============================================================================

TEST_CASE("chaining - VVF view is copyable and assignable", "[chaining][semiregular]") {
  auto g   = make_simple_graph();
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto view1 = vertexlist(g, vvf);

  // Copy construct
  auto view2 = view1;

  std::vector<int> vals1, vals2;
  for (auto [id, v, val] : view1)
    vals1.push_back(val);
  for (auto [id, v, val] : view2)
    vals2.push_back(val);

  REQUIRE(vals1 == vals2);

  // Copy assign
  auto view3 = vertexlist(g, vvf);
  view3      = view1;

  std::vector<int> vals3;
  for (auto [id, v, val] : view3)
    vals3.push_back(val);

  REQUIRE(vals1 == vals3);
}

TEST_CASE("chaining - EVF view is copyable and assignable", "[chaining][semiregular]") {
  auto g   = make_simple_graph();
  auto evf = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto view1 = edgelist(g, evf);
  auto view2 = view1; // copy

  std::vector<int> vals1, vals2;
  for (auto [sid, tid, e, val] : view1)
    vals1.push_back(val);
  for (auto [sid, tid, e, val] : view2)
    vals2.push_back(val);

  REQUIRE(vals1 == vals2);
}

// =============================================================================
// Part 11: Default Construction (semiregular proof)
// =============================================================================

TEST_CASE("chaining - VVF views are default constructible", "[chaining][default_init]") {
  using Graph = std::vector<std::vector<int>>;
  auto vvf    = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };
  auto evf    = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  // These must compile -- default construction is required for semiregular
  [[maybe_unused]] vertexlist_view<Graph, decltype(vvf)>   vl_default;
  [[maybe_unused]] neighbors_view<Graph, decltype(vvf)>    nb_default;
  [[maybe_unused]] edgelist_view<Graph, decltype(evf)>     el_default;
  [[maybe_unused]] vertices_dfs_view<Graph, decltype(vvf)> vdfs_default;
  [[maybe_unused]] vertices_bfs_view<Graph, decltype(vvf)> vbfs_default;

  // No crash, no UB -- just proves these are default_initializable
  REQUIRE(true);
}
