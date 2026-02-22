/**
 * @file test_find_in_edge_cpo.cpp
 * @brief Comprehensive tests for find_in_edge(g,u,v), find_in_edge(g,u,vid), and find_in_edge(g,uid,vid) CPOs
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <vector>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Stub graph with ADL in_edges (reusable) — same pattern as test_in_edges_cpo.cpp
// =============================================================================

namespace test_find_in_edge {

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

} // namespace test_find_in_edge

// =============================================================================
// Tests: find_in_edge(g, u, v) — both vertex descriptors
// =============================================================================

TEST_CASE("find_in_edge(g, u, v) finds incoming edge by descriptor", "[find_in_edge][cpo][uu]") {
  // Build: 0->2, 1->2, 3->2.  Vertex 2 has in-edges from {0, 1, 3}.
  test_find_in_edge::ADLGraph graph(4);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 2);

  auto verts = vertices(graph);
  auto it    = verts.begin();
  auto v0    = *it++;
  auto v1    = *it++;
  auto v2    = *it++;
  auto v3    = *it;

  SECTION("Find existing incoming edge from v1 to v2") {
    auto e = find_in_edge(graph, v2, v1);
    // Returns outgoing edge from v1 to v2; source_id == v1's vertex id
    REQUIRE(source_id(graph, e) == 1);
  }

  SECTION("Find existing incoming edge from v0 to v2") {
    auto e = find_in_edge(graph, v2, v0);
    REQUIRE(source_id(graph, e) == 0);
  }

  SECTION("Find existing incoming edge from v3 to v2") {
    auto e = find_in_edge(graph, v2, v3);
    REQUIRE(source_id(graph, e) == 3);
  }
}

// =============================================================================
// Tests: find_in_edge(g, u, vid) — descriptor + source ID
// =============================================================================

TEST_CASE("find_in_edge(g, u, vid) finds incoming edge by source ID", "[find_in_edge][cpo][uid]") {
  test_find_in_edge::ADLGraph graph(4);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 2);

  auto verts = vertices(graph);
  auto it    = verts.begin();
  std::advance(it, 2);
  auto v2 = *it;

  SECTION("Find incoming edge from source ID 1") {
    auto e = find_in_edge(graph, v2, 1);
    REQUIRE(source_id(graph, e) == 1);
  }

  SECTION("Find incoming edge from source ID 0") {
    auto e = find_in_edge(graph, v2, 0);
    REQUIRE(source_id(graph, e) == 0);
  }

  SECTION("Find incoming edge from source ID 3") {
    auto e = find_in_edge(graph, v2, 3);
    REQUIRE(source_id(graph, e) == 3);
  }
}

// =============================================================================
// Tests: find_in_edge(g, uid, vid) — both IDs
// =============================================================================

TEST_CASE("find_in_edge(g, uid, vid) finds incoming edge by both IDs", "[find_in_edge][cpo][uidvid]") {
  test_find_in_edge::ADLGraph graph(4);
  graph.add_edge(0, 2);
  graph.add_edge(1, 2);
  graph.add_edge(3, 2);

  SECTION("Find incoming edge to vertex 2 from vertex 1") {
    auto e = find_in_edge(graph, size_t(2), size_t(1));
    REQUIRE(source_id(graph, e) == 1);
  }

  SECTION("Find incoming edge to vertex 2 from vertex 0") {
    auto e = find_in_edge(graph, size_t(2), size_t(0));
    REQUIRE(source_id(graph, e) == 0);
  }

  SECTION("Find incoming edge to vertex 2 from vertex 3") {
    auto e = find_in_edge(graph, size_t(2), size_t(3));
    REQUIRE(source_id(graph, e) == 3);
  }
}

// =============================================================================
// Tests: Vertex with no incoming edges
// =============================================================================

TEST_CASE("find_in_edge on vertex with single incoming edge", "[find_in_edge][cpo][single]") {
  test_find_in_edge::ADLGraph graph(3);
  graph.add_edge(0, 1); // Only edge: 0 -> 1

  auto verts = vertices(graph);
  auto it    = verts.begin();
  auto v0    = *it++;
  auto v1    = *it;

  auto e = find_in_edge(graph, v1, v0);
  REQUIRE(source_id(graph, e) == 0);
}
