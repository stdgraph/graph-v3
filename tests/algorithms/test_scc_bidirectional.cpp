/**
 * @file test_scc_bidirectional.cpp
 * @brief Tests for kosaraju() bidirectional overload (single-graph SCC).
 *
 * Verifies:
 * - Correctness of SCC detection using in_edges (no separate transpose graph)
 * - Agreement with the two-graph kosaraju overload
 * - Works with both vov (random-access) and vol (forward-iterator) containers
 */

#include <catch2/catch_test_macros.hpp>
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

// Bidirectional graph types — all use Sourced=true, Bidirectional=true
using bidir_vov_void = dynamic_graph<void, void, void, uint32_t, true, true,
                                     vov_graph_traits<void, void, void, uint32_t, true, true>>;

using bidir_vol_void = dynamic_graph<void, void, void, uint32_t, true, true,
                                     vol_graph_traits<void, void, void, uint32_t, true, true>>;

using bidir_vov_int = dynamic_graph<int, void, void, uint32_t, true, true,
                                    vov_graph_traits<int, void, void, uint32_t, true, true>>;

// =============================================================================
// Helpers (mirrored from test_connected_components.cpp)
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
// Single vertex
// =============================================================================

TEST_CASE("kosaraju bidir - single vertex (vov)", "[algorithm][kosaraju][scc][bidirectional]") {
  auto g = single_vertex<bidir_vov_void>();

  std::vector<uint32_t> component(num_vertices(g));
  kosaraju(g, component);

  REQUIRE(component[0] == 0);
  REQUIRE(count_unique_components(component) == 1);
}

TEST_CASE("kosaraju bidir - single vertex (vol)", "[algorithm][kosaraju][scc][bidirectional]") {
  auto g = single_vertex<bidir_vol_void>();

  std::vector<uint32_t> component(num_vertices(g));
  kosaraju(g, component);

  REQUIRE(component[0] == 0);
  REQUIRE(count_unique_components(component) == 1);
}

// =============================================================================
// Simple cycle — all vertices in one SCC
// =============================================================================

TEST_CASE("kosaraju bidir - simple cycle (vov)", "[algorithm][kosaraju][scc][bidirectional]") {
  // 0 → 1 → 2 → 0
  bidir_vov_void        g({{0, 1}, {1, 2}, {2, 0}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(count_unique_components(component) == 1);
}

TEST_CASE("kosaraju bidir - simple cycle (vol)", "[algorithm][kosaraju][scc][bidirectional]") {
  bidir_vol_void        g({{0, 1}, {1, 2}, {2, 0}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(count_unique_components(component) == 1);
}

// =============================================================================
// Two SCCs
// =============================================================================

TEST_CASE("kosaraju bidir - two SCCs (vov)", "[algorithm][kosaraju][scc][bidirectional]") {
  // SCC1: {0,1}  (0 ↔ 1)
  // SCC2: {2,3}  (2 ↔ 3)
  // Cross: 1 → 2  (one-way, so no merge)
  bidir_vov_void        g({{0, 1}, {1, 0}, {1, 2}, {2, 3}, {3, 2}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(all_same_component(component, {0, 1}));
  REQUIRE(all_same_component(component, {2, 3}));
  REQUIRE(different_components(component, 0, 2));
  REQUIRE(count_unique_components(component) == 2);
}

TEST_CASE("kosaraju bidir - two SCCs (vol)", "[algorithm][kosaraju][scc][bidirectional]") {
  bidir_vol_void        g({{0, 1}, {1, 0}, {1, 2}, {2, 3}, {3, 2}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(all_same_component(component, {0, 1}));
  REQUIRE(all_same_component(component, {2, 3}));
  REQUIRE(different_components(component, 0, 2));
  REQUIRE(count_unique_components(component) == 2);
}

// =============================================================================
// DAG — every vertex is its own SCC
// =============================================================================

TEST_CASE("kosaraju bidir - DAG (vov)", "[algorithm][kosaraju][scc][bidirectional]") {
  // 0 → 1 → 2 → 3
  bidir_vov_void        g({{0, 1}, {1, 2}, {2, 3}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(count_unique_components(component) == 4);
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = i + 1; j < 4; ++j)
      REQUIRE(different_components(component, i, j));
}

TEST_CASE("kosaraju bidir - DAG (vol)", "[algorithm][kosaraju][scc][bidirectional]") {
  bidir_vol_void        g({{0, 1}, {1, 2}, {2, 3}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(count_unique_components(component) == 4);
  for (size_t i = 0; i < 4; ++i)
    for (size_t j = i + 1; j < 4; ++j)
      REQUIRE(different_components(component, i, j));
}

// =============================================================================
// Complex SCC structure (3 SCCs)
// =============================================================================

TEST_CASE("kosaraju bidir - complex SCCs (vov)", "[algorithm][kosaraju][scc][bidirectional]") {
  // SCC1: {0,1,2}  cycle 0→1→2→0
  // SCC2: {3,4}    cycle 3→4→3
  // SCC3: {5}      singleton
  // Cross: 2→3, 4→5
  bidir_vov_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});

  std::vector<uint32_t> component(num_vertices(g));
  kosaraju(g, component);

  REQUIRE(count_unique_components(component) == 3);
  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(all_same_component(component, {3, 4}));
  REQUIRE(different_components(component, 0, 3));
  REQUIRE(different_components(component, 0, 5));
  REQUIRE(different_components(component, 3, 5));
}

TEST_CASE("kosaraju bidir - complex SCCs (vol)", "[algorithm][kosaraju][scc][bidirectional]") {
  bidir_vol_void g({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});

  std::vector<uint32_t> component(num_vertices(g));
  kosaraju(g, component);

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

TEST_CASE("kosaraju bidir - self loops", "[algorithm][kosaraju][scc][bidirectional]") {
  // 0 self-loop, 1 self-loop, 0→1 (one-way)
  bidir_vov_void        g({{0, 0}, {1, 1}, {0, 1}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  // Each vertex is its own SCC (self-loop doesn't merge with others)
  REQUIRE(count_unique_components(component) == 2);
  REQUIRE(different_components(component, 0, 1));
}

// =============================================================================
// Agreement with two-graph overload
// =============================================================================

TEST_CASE("kosaraju bidir - agrees with two-graph overload", "[algorithm][kosaraju][scc][bidirectional]") {
  // Build same graph for both overloads
  // Graph: 0→1→2→0 (SCC), 2→3→4→3 (SCC), 4→5
  bidir_vov_void g_bidir({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});

  // Non-bidir version + transpose
  using vov_dir = dynamic_graph<void, void, void, uint32_t, true, false,
                                vov_graph_traits<void, void, void, uint32_t, true, false>>;
  vov_dir g_fwd({{0, 1}, {1, 2}, {2, 0}, {2, 3}, {3, 4}, {4, 3}, {4, 5}});
  vov_dir g_rev({{1, 0}, {2, 1}, {0, 2}, {3, 2}, {4, 3}, {3, 4}, {5, 4}});

  std::vector<uint32_t> comp_bidir(num_vertices(g_bidir));
  std::vector<uint32_t> comp_twog(num_vertices(g_fwd));

  kosaraju(g_bidir, comp_bidir);
  kosaraju(g_fwd, g_rev, comp_twog);

  // Both should find the same number of SCCs
  REQUIRE(count_unique_components(comp_bidir) == count_unique_components(comp_twog));

  // Both should agree on which vertices are in the same SCC
  for (size_t i = 0; i < comp_bidir.size(); ++i)
    for (size_t j = i + 1; j < comp_bidir.size(); ++j) {
      INFO("vertices " << i << " and " << j);
      REQUIRE((comp_bidir[i] == comp_bidir[j]) == (comp_twog[i] == comp_twog[j]));
    }
}

// =============================================================================
// Weighted bidirectional graph
// =============================================================================

TEST_CASE("kosaraju bidir - weighted edges ignored", "[algorithm][kosaraju][scc][bidirectional]") {
  // Same structure as simple cycle but with weights
  bidir_vov_int         g({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
  std::vector<uint32_t> component(num_vertices(g));

  kosaraju(g, component);

  REQUIRE(all_same_component(component, {0, 1, 2}));
  REQUIRE(count_unique_components(component) == 1);
}

// =============================================================================
// Disconnected graph
// =============================================================================

TEST_CASE("kosaraju bidir - disconnected graph", "[algorithm][kosaraju][scc][bidirectional]") {
  // 0→1→0 (SCC), 2 isolated, 3→4→3 (SCC)
  bidir_vov_void g({{0, 1}, {1, 0}, {3, 4}, {4, 3}});

  // Need 5 vertices (0-4). Graph constructor infers from max vertex id in edge list.
  std::vector<uint32_t> component(num_vertices(g));
  kosaraju(g, component);

  REQUIRE(count_unique_components(component) == 3);
  REQUIRE(all_same_component(component, {0, 1}));
  REQUIRE(all_same_component(component, {3, 4}));
  REQUIRE(different_components(component, 0, 2));
  REQUIRE(different_components(component, 0, 3));
  REQUIRE(different_components(component, 2, 3));
}
