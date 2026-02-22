/**
 * @file test_contains_in_edge_cpo.cpp
 * @brief Comprehensive tests for contains_in_edge(g,u,v) and contains_in_edge(g,uid,vid) CPOs
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <vector>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Stub graph with ADL in_edges — same pattern as test_in_edges_cpo.cpp
// =============================================================================

namespace test_contains_in_edge {

struct ADLGraph : std::vector<std::vector<int>> {
  std::vector<std::vector<int>> in_adj;

  explicit ADLGraph(size_t n) : std::vector<std::vector<int>>(n), in_adj(n) {}

  void add_edge(size_t from, size_t to) {
    (*this)[from].push_back(static_cast<int>(to));
    in_adj[to].push_back(static_cast<int>(from));
  }
};

// ADL in_edges for vertex descriptor
template <typename U>
  requires vertex_descriptor_type<U>
auto in_edges(ADLGraph& g, const U& u) -> const std::vector<int>& {
  return g.in_adj[u.vertex_id()];
}

} // namespace test_contains_in_edge

// =============================================================================
// Tests: contains_in_edge(g, u, v) — both vertex descriptors
// =============================================================================

TEST_CASE("contains_in_edge(g, u, v) detects existing incoming edges", "[contains_in_edge][cpo][uv]") {
  // Graph: 0->1, 0->2, 1->2, 3->0
  test_contains_in_edge::ADLGraph graph(4);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 0);

  auto verts = vertices(graph);
  auto it    = verts.begin();
  auto v0    = *it++;
  auto v1    = *it++;
  auto v2    = *it++;
  auto v3    = *it;

  SECTION("Existing incoming edges return true") {
    // v0 has incoming from v3
    REQUIRE(contains_in_edge(graph, v0, v3) == true);
    // v1 has incoming from v0
    REQUIRE(contains_in_edge(graph, v1, v0) == true);
    // v2 has incoming from v0 and v1
    REQUIRE(contains_in_edge(graph, v2, v0) == true);
    REQUIRE(contains_in_edge(graph, v2, v1) == true);
  }

  SECTION("Non-existing incoming edges return false") {
    // v0 has no incoming from v0, v1, v2
    REQUIRE(contains_in_edge(graph, v0, v0) == false);
    REQUIRE(contains_in_edge(graph, v0, v1) == false);
    REQUIRE(contains_in_edge(graph, v0, v2) == false);
    // v1 has no incoming from v1, v2, v3
    REQUIRE(contains_in_edge(graph, v1, v1) == false);
    REQUIRE(contains_in_edge(graph, v1, v2) == false);
    REQUIRE(contains_in_edge(graph, v1, v3) == false);
    // v3 has no incoming edges at all
    REQUIRE(contains_in_edge(graph, v3, v0) == false);
    REQUIRE(contains_in_edge(graph, v3, v1) == false);
    REQUIRE(contains_in_edge(graph, v3, v2) == false);
    REQUIRE(contains_in_edge(graph, v3, v3) == false);
  }
}

TEST_CASE("contains_in_edge(g, u, v) handles vertex with no incoming edges", "[contains_in_edge][cpo][empty][uv]") {
  // Graph: 0->1.  Vertex 0 has no incoming edges.
  test_contains_in_edge::ADLGraph graph(2);
  graph.add_edge(0, 1);

  auto verts = vertices(graph);
  auto it    = verts.begin();
  auto v0    = *it++;
  auto v1    = *it;

  REQUIRE(contains_in_edge(graph, v0, v0) == false);
  REQUIRE(contains_in_edge(graph, v0, v1) == false);
  // v1 has incoming from v0
  REQUIRE(contains_in_edge(graph, v1, v0) == true);
}

// =============================================================================
// Tests: contains_in_edge(g, uid, vid) — both IDs
// =============================================================================

TEST_CASE("contains_in_edge(g, uid, vid) detects existing incoming edges by ID",
          "[contains_in_edge][cpo][uidvid]") {
  // Graph: 0->1, 0->2, 1->2, 3->0
  test_contains_in_edge::ADLGraph graph(4);
  graph.add_edge(0, 1);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 0);

  SECTION("Existing incoming edges return true") {
    REQUIRE(contains_in_edge(graph, size_t(0), size_t(3)) == true);
    REQUIRE(contains_in_edge(graph, size_t(1), size_t(0)) == true);
    REQUIRE(contains_in_edge(graph, size_t(2), size_t(0)) == true);
    REQUIRE(contains_in_edge(graph, size_t(2), size_t(1)) == true);
  }

  SECTION("Non-existing incoming edges return false") {
    REQUIRE(contains_in_edge(graph, size_t(0), size_t(0)) == false);
    REQUIRE(contains_in_edge(graph, size_t(0), size_t(1)) == false);
    REQUIRE(contains_in_edge(graph, size_t(0), size_t(2)) == false);
    REQUIRE(contains_in_edge(graph, size_t(3), size_t(0)) == false);
    REQUIRE(contains_in_edge(graph, size_t(3), size_t(1)) == false);
    REQUIRE(contains_in_edge(graph, size_t(3), size_t(2)) == false);
    REQUIRE(contains_in_edge(graph, size_t(3), size_t(3)) == false);
  }
}

TEST_CASE("contains_in_edge(g, uid, vid) handles empty incoming edges by ID",
          "[contains_in_edge][cpo][empty][uidvid]") {
  test_contains_in_edge::ADLGraph graph(3);
  // No edges added - all should be false
  REQUIRE(contains_in_edge(graph, size_t(0), size_t(1)) == false);
  REQUIRE(contains_in_edge(graph, size_t(1), size_t(0)) == false);
  REQUIRE(contains_in_edge(graph, size_t(2), size_t(0)) == false);
}
