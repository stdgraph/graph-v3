/**
 * @file test_tarjan_scc.cpp
 * @brief Tests for tarjan_scc() strongly connected components algorithm.
 *
 * Verifies:
 * - Correctness of SCC detection using Tarjan's single-pass DFS
 * - Agreement with Kosaraju's algorithm (two-graph overload)
 * - Works with both vov (random-access) and vol (forward-iterator) containers
 * - Edge cases: empty graph, single vertex, self-loops, disconnected graphs
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/algorithm/tarjan_scc.hpp>
#include <graph/algorithm/connected_components.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vol_graph_traits.hpp>
#include "../common/graph_fixtures.hpp"
#include <algorithm>
#include <set>
#include <vector>

using namespace graph;
using namespace graph::container;
using namespace graph::test::fixtures;

// Graph types for testing
using vov_void = dynamic_graph<void, void, void, uint32_t, false,
                               vov_graph_traits<void, void, void, uint32_t, false>>;

using vol_void = dynamic_graph<void, void, void, uint32_t, false,
                               vol_graph_traits<void, void, void, uint32_t, false>>;

using vov_int = dynamic_graph<int, void, void, uint32_t, false,
                              vov_graph_traits<int, void, void, uint32_t, false>>;

// =============================================================================
// Helpers
// =============================================================================

template <typename Component>
bool all_same_component(const Component& component, const std::vector<size_t>& vertices) {
  if (vertices.empty())
    return true;
  auto first_comp = component[vertices[0]];
  return std::all_of(vertices.begin(), vertices.end(), [&](size_t v) { return component[v] == first_comp; });
}

template <typename Component>
bool different_components(const Component& component, size_t u, size_t v) {
  return component[u] != component[v];
}

template <typename Component>
size_t count_unique_components(const Component& component) {
  std::set<typename Component::value_type> unique(component.begin(), component.end());
  return unique.size();
}

// =============================================================================
// Empty graph
// =============================================================================

TEST_CASE("tarjan_scc - empty graph", "[algorithm][tarjan][scc]") {
  vov_void g;

  std::vector<uint32_t> component;
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 0);
}

// =============================================================================
// Single vertex
// =============================================================================

TEST_CASE("tarjan_scc - single vertex (vov)", "[algorithm][tarjan][scc]") {
  auto g = single_vertex<vov_void>();

  std::vector<uint32_t> component(num_vertices(g));
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 1);
  REQUIRE(component[0] == 0);
}

TEST_CASE("tarjan_scc - single vertex (vol)", "[algorithm][tarjan][scc]") {
  auto g = single_vertex<vol_void>();

  std::vector<uint32_t> component(num_vertices(g));
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 1);
  REQUIRE(component[0] == 0);
}

// =============================================================================
// Simple cycle — all vertices in one SCC
// =============================================================================

TEST_CASE("tarjan_scc - simple cycle (vov)", "[algorithm][tarjan][scc]") {
  // 0 -> 1 -> 2 -> 0
  vov_void              g({{0, 1}, {1, 2}, {2, 0}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 1);
  REQUIRE(all_same_component(component, {0, 1, 2}));
}

TEST_CASE("tarjan_scc - simple cycle (vol)", "[algorithm][tarjan][scc]") {
  vol_void              g({{0, 1}, {1, 2}, {2, 0}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 1);
  REQUIRE(all_same_component(component, {0, 1, 2}));
}

// =============================================================================
// Two SCCs
// =============================================================================

TEST_CASE("tarjan_scc - two SCCs (vov)", "[algorithm][tarjan][scc]") {
  // SCC1: {0,1}  (0 <-> 1)
  // SCC2: {2,3}  (2 <-> 3)
  // Cross: 1 -> 2  (one-way, so no merge)
  vov_void              g({{0, 1}, {1, 0}, {1, 2}, {2, 3}, {3, 2}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 2);
  REQUIRE(all_same_component(component, {0, 1}));
  REQUIRE(all_same_component(component, {2, 3}));
  REQUIRE(different_components(component, 0, 2));
}

TEST_CASE("tarjan_scc - two SCCs (vol)", "[algorithm][tarjan][scc]") {
  vol_void              g({{0, 1}, {1, 0}, {1, 2}, {2, 3}, {3, 2}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 2);
  REQUIRE(all_same_component(component, {0, 1}));
  REQUIRE(all_same_component(component, {2, 3}));
  REQUIRE(different_components(component, 0, 2));
}

// =============================================================================
// DAG — every vertex is its own SCC
// =============================================================================

TEST_CASE("tarjan_scc - DAG (vov)", "[algorithm][tarjan][scc]") {
  // 0 -> 1 -> 2 -> 3
  vov_void              g({{0, 1}, {1, 2}, {2, 3}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 4);
  REQUIRE(count_unique_components(component) == 4);
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = i + 1; j < 4; ++j)
      REQUIRE(different_components(component, i, j));
}

TEST_CASE("tarjan_scc - DAG (vol)", "[algorithm][tarjan][scc]") {
  vol_void              g({{0, 1}, {1, 2}, {2, 3}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 4);
  REQUIRE(count_unique_components(component) == 4);
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = i + 1; j < 4; ++j)
      REQUIRE(different_components(component, i, j));
}

// =============================================================================
// Complex SCC structure (3 SCCs)
// =============================================================================

TEST_CASE("tarjan_scc - complex SCCs (vov)", "[algorithm][tarjan][scc]") {
  // SCC1: {0,1,2}  cycle 0->1->2->0
  // SCC2: {3,4}    cycle 3->4->3
  // SCC3: {5}      singleton
  // Cross: 2->3, 4->5
  vov_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});

  std::vector<uint32_t> component(num_vertices(g));
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 3);
  REQUIRE(count_unique_components(component) == 3);
  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(all_same_component(component, {3, 4}));
  REQUIRE(different_components(component, 0, 3));
  REQUIRE(different_components(component, 0, 5));
  REQUIRE(different_components(component, 3, 5));
}

TEST_CASE("tarjan_scc - complex SCCs (vol)", "[algorithm][tarjan][scc]") {
  vol_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});

  std::vector<uint32_t> component(num_vertices(g));
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 3);
  REQUIRE(count_unique_components(component) == 3);
  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(all_same_component(component, {3, 4}));
  REQUIRE(different_components(component, 0, 3));
  REQUIRE(different_components(component, 0, 5));
  REQUIRE(different_components(component, 3, 5));
}

// =============================================================================
// Self-loops
// =============================================================================

TEST_CASE("tarjan_scc - self loops", "[algorithm][tarjan][scc]") {
  // 0 self-loop, 1 self-loop, 0->1 (one-way)
  vov_void              g({{0, 0}, {1, 1}, {0, 1}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  // Each vertex is its own SCC (self-loop doesn't merge with others)
  REQUIRE(num == 2);
  REQUIRE(count_unique_components(component) == 2);
  REQUIRE(different_components(component, 0, 1));
}

// =============================================================================
// Weighted edges (weights ignored)
// =============================================================================

TEST_CASE("tarjan_scc - weighted edges ignored", "[algorithm][tarjan][scc]") {
  // Same structure as simple cycle but with weights
  vov_int               g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 1);
  REQUIRE(all_same_component(component, {0, 1, 2}));
}

// =============================================================================
// Disconnected graph
// =============================================================================

TEST_CASE("tarjan_scc - disconnected graph", "[algorithm][tarjan][scc]") {
  // 0->1->0 (SCC), 2 isolated, 3->4->3 (SCC)
  vov_void g({{0, 1}, {1, 0}, {3, 4}, {4, 3}});

  std::vector<uint32_t> component(num_vertices(g));
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 3);
  REQUIRE(count_unique_components(component) == 3);
  REQUIRE(all_same_component(component, {0, 1}));
  REQUIRE(all_same_component(component, {3, 4}));
  REQUIRE(different_components(component, 0, 2));
  REQUIRE(different_components(component, 0, 3));
  REQUIRE(different_components(component, 2, 3));
}

// =============================================================================
// Agreement with Kosaraju (two-graph overload)
// =============================================================================

TEST_CASE("tarjan_scc - agrees with kosaraju", "[algorithm][tarjan][scc]") {
  // Graph: 0->1->2->0 (SCC), 2->3->4->3 (SCC), 4->5
  vov_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});
  vov_void g_t({{1, 0}, {2, 1}, {0, 2}, {3, 2}, {4, 3}, {3, 4}, {5, 4}});

  std::vector<uint32_t> comp_tarjan(num_vertices(g));
  std::vector<uint32_t> comp_kosaraju(num_vertices(g));

  size_t num_tarjan = tarjan_scc(g, container_value_fn(comp_tarjan));
  kosaraju(g, g_t, container_value_fn(comp_kosaraju));

  // Both should find the same number of SCCs
  REQUIRE(num_tarjan == count_unique_components(comp_kosaraju));

  // Both should agree on which vertices are in the same SCC
  for (size_t i = 0; i < comp_tarjan.size(); ++i)
    for (size_t j = i + 1; j < comp_tarjan.size(); ++j) {
      INFO("vertices " << i << " and " << j);
      REQUIRE((comp_tarjan[i] == comp_tarjan[j]) == (comp_kosaraju[i] == comp_kosaraju[j]));
    }
}

// =============================================================================
// Larger graph: Wikipedia Tarjan example
// =============================================================================

TEST_CASE("tarjan_scc - wikipedia example", "[algorithm][tarjan][scc]") {
  // Classic 8-vertex example:
  //   SCC1: {0,1,2}  (0->1, 1->2, 2->0)
  //   SCC2: {3,4,5}  (3->4, 4->5, 5->3)
  //   SCC3: {6,7}    (6->7, 7->6)
  //   Cross: 2->3, 5->6, 4->0 creates link back but 4->0 merges into SCC? No.
  //   Let's use the standard example:
  //   0->1, 1->2, 2->0, 2->3, 3->4, 4->5, 5->3, 5->6, 6->7, 7->6
  vov_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 5}, {5, 3}, {5, 6}, {6, 7}, {7, 6}});

  std::vector<uint32_t> component(num_vertices(g));
  size_t                num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 3);
  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(all_same_component(component, {3, 4, 5}));
  REQUIRE(all_same_component(component, {6, 7}));
  REQUIRE(different_components(component, 0, 3));
  REQUIRE(different_components(component, 0, 6));
  REQUIRE(different_components(component, 3, 6));
}

// =============================================================================
// Single large SCC (complete cycle)
// =============================================================================

TEST_CASE("tarjan_scc - complete cycle", "[algorithm][tarjan][scc]") {
  // 0 -> 1 -> 2 -> 3 -> 4 -> 0
  vov_void              g({{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}});
  std::vector<uint32_t> component(num_vertices(g));

  size_t num = tarjan_scc(g, container_value_fn(component));

  REQUIRE(num == 1);
  REQUIRE(all_same_component(component, {0, 1, 2, 3, 4}));
}
