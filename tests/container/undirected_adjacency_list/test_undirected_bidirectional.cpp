/**
 * @file test_undirected_bidirectional.cpp
 * @brief Integration tests for incoming-edge support in undirected_adjacency_list
 *
 * Verifies that undirected_adjacency_list models bidirectional_adjacency_list
 * by providing in_edges() ADL friends that return the same edge ranges as
 * edges(). Tests in_edges, in_degree, find_in_edge, and contains_in_edge CPOs.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <graph/graph.hpp>
#include <algorithm>
#include <set>
#include <vector>

using graph::container::undirected_adjacency_list;

// Graph type aliases
using IntGraph = undirected_adjacency_list<int, int, int>;

// Bring CPOs into scope (use aliases where available per project convention)
using graph::adj_list::vertices;
using graph::adj_list::find_vertex;
using graph::adj_list::vertex_id;
using graph::adj_list::edges;
using graph::adj_list::degree;
using graph::adj_list::target_id;
using graph::adj_list::source_id;
using graph::adj_list::in_edges;
using graph::adj_list::in_degree;
using graph::adj_list::find_in_edge;
using graph::adj_list::contains_in_edge;
using graph::adj_list::find_vertex_edge;
using graph::adj_list::contains_edge;
using graph::adj_list::edge_value;

// =============================================================================
// Concept satisfaction
// =============================================================================

TEST_CASE("undirected_adjacency_list models bidirectional_adjacency_list",
          "[undirected_adjacency_list][bidirectional][concept]") {
  static_assert(graph::adj_list::bidirectional_adjacency_list<IntGraph>,
                "undirected_adjacency_list must model bidirectional_adjacency_list");
  static_assert(graph::adj_list::index_bidirectional_adjacency_list<IntGraph>,
                "undirected_adjacency_list must model index_bidirectional_adjacency_list");

  // Also verify via graph:: namespace re-exports
  static_assert(graph::bidirectional_adjacency_list<IntGraph>);
  static_assert(graph::index_bidirectional_adjacency_list<IntGraph>);
}

// =============================================================================
// in_edges CPO
// =============================================================================

TEST_CASE("in_edges returns same edges as edges for undirected graph",
          "[undirected_adjacency_list][bidirectional][in_edges]") {
  IntGraph g(0);
  g.create_vertex(10); // 0
  g.create_vertex(20); // 1
  g.create_vertex(30); // 2
  g.create_edge(0, 1, 100);
  g.create_edge(0, 2, 200);
  g.create_edge(1, 2, 300);

  SECTION("in_edges and edges produce identical target sets per vertex") {
    for (auto v : vertices(g)) {
      std::set<unsigned int> out_targets, in_targets;
      for (auto e : edges(g, v)) {
        out_targets.insert(target_id(g, e));
      }
      for (auto ie : in_edges(g, v)) {
        in_targets.insert(target_id(g, ie));
      }
      REQUIRE(out_targets == in_targets);
    }
  }

  SECTION("in_edges count matches edges count per vertex") {
    for (auto v : vertices(g)) {
      size_t out_count = 0, in_count = 0;
      for ([[maybe_unused]] auto e : edges(g, v))
        ++out_count;
      for ([[maybe_unused]] auto ie : in_edges(g, v))
        ++in_count;
      REQUIRE(out_count == in_count);
    }
  }
}

TEST_CASE("in_edges by vertex id",
          "[undirected_adjacency_list][bidirectional][in_edges]") {
  IntGraph g(0);
  g.create_vertex(10); // 0
  g.create_vertex(20); // 1
  g.create_vertex(30); // 2
  g.create_edge(0, 1, 100);
  g.create_edge(0, 2, 200);
  g.create_edge(1, 2, 300);

  // in_edges(g, uid) should work via the CPO default tier
  size_t count = 0;
  for ([[maybe_unused]] auto ie : in_edges(g, 0u)) {
    ++count;
  }
  REQUIRE(count == 2); // vertex 0 has edges to 1 and 2
}

TEST_CASE("in_edges on const graph",
          "[undirected_adjacency_list][bidirectional][in_edges]") {
  IntGraph g(0);
  g.create_vertex(10);
  g.create_vertex(20);
  g.create_edge(0, 1, 100);

  const IntGraph& cg = g;
  auto            v  = *vertices(cg).begin();

  size_t count = 0;
  for ([[maybe_unused]] auto ie : in_edges(cg, v)) {
    ++count;
  }
  REQUIRE(count == 1);
}

TEST_CASE("in_edges on vertex with no edges",
          "[undirected_adjacency_list][bidirectional][in_edges]") {
  IntGraph g(0);
  g.create_vertex(10); // isolated vertex

  auto v     = *vertices(g).begin();
  auto range = in_edges(g, v);

  size_t count = 0;
  for ([[maybe_unused]] auto ie : range) {
    ++count;
  }
  REQUIRE(count == 0);
}

// =============================================================================
// in_degree CPO
// =============================================================================

TEST_CASE("in_degree equals degree for undirected graph",
          "[undirected_adjacency_list][bidirectional][in_degree]") {
  IntGraph g(0);
  g.create_vertex(10); // 0
  g.create_vertex(20); // 1
  g.create_vertex(30); // 2
  g.create_vertex(40); // 3 (isolated)
  g.create_edge(0, 1, 100);
  g.create_edge(0, 2, 200);
  g.create_edge(1, 2, 300);

  SECTION("in_degree matches degree via vertex descriptor") {
    for (auto v : vertices(g)) {
      REQUIRE(in_degree(g, v) == degree(g, v));
    }
  }

  SECTION("in_degree matches degree via vertex id") {
    for (unsigned int uid = 0; uid < 4; ++uid) {
      REQUIRE(in_degree(g, uid) == degree(g, uid));
    }
  }

  SECTION("in_degree of isolated vertex is 0") {
    REQUIRE(in_degree(g, 3u) == 0);
  }

  SECTION("in_degree specific values") {
    REQUIRE(in_degree(g, 0u) == 2); // edges to 1 and 2
    REQUIRE(in_degree(g, 1u) == 2); // edges to 0 and 2
    REQUIRE(in_degree(g, 2u) == 2); // edges to 0 and 1
    REQUIRE(in_degree(g, 3u) == 0); // isolated
  }
}

// =============================================================================
// find_in_edge CPO
// =============================================================================

TEST_CASE("find_in_edge works on undirected graph",
          "[undirected_adjacency_list][bidirectional][find_in_edge]") {
  IntGraph g(0);
  g.create_vertex(10); // 0
  g.create_vertex(20); // 1
  g.create_vertex(30); // 2
  g.create_edge(0, 1, 100);
  g.create_edge(0, 2, 200);
  g.create_edge(1, 2, 300);

  SECTION("find_in_edge with two vertex ids - edge exists") {
    // find_in_edge(g, uid, vid) default: find_vertex_edge(g, vid, uid)
    // = search edges(g, vid) for target_id == uid
    // So find_in_edge(g, 0, 1) = find_vertex_edge(g, 1, 0)
    // = search edges from vertex 1 for target_id == 0 -> finds edge 1-0
    auto e = find_in_edge(g, 0u, 1u);
    // The edge is found from vertex 1's perspective: source_id == 1, target_id == 0
    REQUIRE(source_id(g, e) == 1);
    REQUIRE(target_id(g, e) == 0);
    REQUIRE(edge_value(g, *e.value()) == 100);
  }

  SECTION("find_in_edge with two vertex ids - other direction") {
    auto e = find_in_edge(g, 1u, 0u);
    // find_in_edge(g, 1, 0) = find_vertex_edge(g, 0, 1)
    // = search edges from vertex 0 for target_id == 1
    REQUIRE(source_id(g, e) == 0);
    REQUIRE(target_id(g, e) == 1);
    REQUIRE(edge_value(g, *e.value()) == 100);
  }

  SECTION("find_in_edge symmetry") {
    // For undirected graphs, find_in_edge(g, u, v) and find_in_edge(g, v, u)
    // should both find the same underlying edge (with different source/target perspectives)
    auto e1 = find_in_edge(g, 0u, 2u);
    auto e2 = find_in_edge(g, 2u, 0u);
    REQUIRE(edge_value(g, *e1.value()) == 200);
    REQUIRE(edge_value(g, *e2.value()) == 200);
  }

  SECTION("find_in_edge with descriptor and vid") {
    auto v0 = *vertices(g).begin();
    auto e  = find_in_edge(g, v0, 1u);
    REQUIRE(edge_value(g, *e.value()) == 100);
  }

  SECTION("find_in_edge with two descriptors") {
    auto it = vertices(g).begin();
    auto v0 = *it;
    ++it;
    auto v1 = *it;
    auto e  = find_in_edge(g, v0, v1);
    REQUIRE(edge_value(g, *e.value()) == 100);
  }

  SECTION("find_in_edge on const graph") {
    const IntGraph& cg = g;
    auto            e  = find_in_edge(cg, 1u, 2u);
    REQUIRE(edge_value(cg, *e.value()) == 300);
  }
}

// =============================================================================
// contains_in_edge CPO
// =============================================================================

TEST_CASE("contains_in_edge works on undirected graph",
          "[undirected_adjacency_list][bidirectional][contains_in_edge]") {
  IntGraph g(0);
  g.create_vertex(10); // 0
  g.create_vertex(20); // 1
  g.create_vertex(30); // 2
  g.create_edge(0, 1, 100);
  g.create_edge(0, 2, 200);
  // No edge between 1 and 2

  SECTION("contains_in_edge with two vertex ids - edge exists") {
    // contains_in_edge(g, uid, vid) default: search edges(g, vid) for target_id == uid
    // So contains_in_edge(g, 0, 1) searches edges from vertex 1 for target_id == 0
    // Vertex 1 has an edge to vertex 0, so this is true.
    REQUIRE(contains_in_edge(g, 0u, 1u) == true);
    REQUIRE(contains_in_edge(g, 1u, 0u) == true);
    REQUIRE(contains_in_edge(g, 0u, 2u) == true);
    REQUIRE(contains_in_edge(g, 2u, 0u) == true);
  }

  SECTION("contains_in_edge with two vertex ids - edge does not exist") {
    // No edge between 1 and 2
    REQUIRE(contains_in_edge(g, 1u, 2u) == false);
    REQUIRE(contains_in_edge(g, 2u, 1u) == false);
  }

  SECTION("contains_in_edge matches contains_edge for undirected") {
    // For undirected graphs, contains_in_edge should agree with contains_edge
    REQUIRE(contains_in_edge(g, 0u, 1u) == contains_edge(g, 0u, 1u));
    REQUIRE(contains_in_edge(g, 0u, 2u) == contains_edge(g, 0u, 2u));
    REQUIRE(contains_in_edge(g, 1u, 2u) == contains_edge(g, 1u, 2u));
  }

  SECTION("contains_in_edge with two vertex descriptors") {
    auto it = vertices(g).begin();
    auto v0 = *it;
    ++it;
    auto v1 = *it;
    ++it;
    auto v2 = *it;

    REQUIRE(contains_in_edge(g, v0, v1) == true);
    REQUIRE(contains_in_edge(g, v0, v2) == true);
    REQUIRE(contains_in_edge(g, v1, v2) == false);
  }

  SECTION("contains_in_edge on const graph") {
    const IntGraph& cg = g;
    REQUIRE(contains_in_edge(cg, 0u, 1u) == true);
    REQUIRE(contains_in_edge(cg, 1u, 2u) == false);
  }
}

// =============================================================================
// Integration: Edge symmetry in undirected graphs
// =============================================================================

TEST_CASE("undirected graph edge symmetry with in_edges",
          "[undirected_adjacency_list][bidirectional][integration]") {
  IntGraph g(0);
  // Triangle: 0--1, 1--2, 2--0
  g.create_vertex(10);
  g.create_vertex(20);
  g.create_vertex(30);
  g.create_edge(0, 1, 12);
  g.create_edge(1, 2, 23);
  g.create_edge(2, 0, 31);

  SECTION("total in_edges iteration matches total edges iteration") {
    size_t total_out = 0, total_in = 0;
    for (auto v : vertices(g)) {
      for ([[maybe_unused]] auto e : edges(g, v))
        ++total_out;
      for ([[maybe_unused]] auto ie : in_edges(g, v))
        ++total_in;
    }
    // Each edge counted twice from each endpoint
    REQUIRE(total_out == 6);
    REQUIRE(total_in == 6);
    REQUIRE(total_out == total_in);
  }

  SECTION("in_degree sum equals degree sum") {
    size_t deg_sum = 0, in_deg_sum = 0;
    for (auto v : vertices(g)) {
      deg_sum += degree(g, v);
      in_deg_sum += in_degree(g, v);
    }
    REQUIRE(deg_sum == in_deg_sum);
    REQUIRE(deg_sum == 6); // 2*|E| for undirected
  }
}

TEST_CASE("undirected graph star topology - in_edges correctness",
          "[undirected_adjacency_list][bidirectional][integration]") {
  IntGraph g(0);
  // Star graph: vertex 0 connected to 1..4
  for (int i = 0; i < 5; ++i)
    g.create_vertex(i * 10);
  g.create_edge(0, 1, 1);
  g.create_edge(0, 2, 2);
  g.create_edge(0, 3, 3);
  g.create_edge(0, 4, 4);

  SECTION("hub vertex has same in_degree and degree") {
    REQUIRE(in_degree(g, 0u) == 4);
    REQUIRE(degree(g, 0u) == 4);
  }

  SECTION("leaf vertices have in_degree 1") {
    for (unsigned int uid = 1; uid <= 4; ++uid) {
      REQUIRE(in_degree(g, uid) == 1);
      REQUIRE(degree(g, uid) == 1);
    }
  }

  SECTION("in_edges from hub has all neighbors") {
    std::set<unsigned int> neighbors;
    for (auto ie : in_edges(g, 0u)) {
      neighbors.insert(target_id(g, ie));
    }
    REQUIRE(neighbors == std::set<unsigned int>{1, 2, 3, 4});
  }

  SECTION("contains_in_edge hub to each leaf") {
    for (unsigned int uid = 1; uid <= 4; ++uid) {
      REQUIRE(contains_in_edge(g, 0u, uid) == true);
      REQUIRE(contains_in_edge(g, uid, 0u) == true);
    }
  }

  SECTION("contains_in_edge between non-adjacent leaves") {
    REQUIRE(contains_in_edge(g, 1u, 2u) == false);
    REQUIRE(contains_in_edge(g, 3u, 4u) == false);
  }
}
