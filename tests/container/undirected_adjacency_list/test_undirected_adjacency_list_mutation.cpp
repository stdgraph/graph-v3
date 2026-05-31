/**
 * @file test_undirected_adjacency_list_mutation.cpp
 * @brief Tests for the mutation API of undirected_adjacency_list.
 *
 * Covers the uniform mutation member functions:
 *   - add_vertex()        (default / move / copy value)
 *   - add_edge()          (by id and by iterator, with/without value)
 *   - remove_edge(pos)    (by edge_iterator)
 *   - remove_edge(uid,vid)(by endpoint ids)
 *   - remove_vertex(uid)  (with renumbering of higher ids)
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using graph::container::undirected_adjacency_list;

using graph::adj_list::num_edges;
using graph::adj_list::num_vertices;
using graph::adj_list::contains_edge;

// int edge value, int vertex value, int graph value
using IntGraph = undirected_adjacency_list<int, int, int>;
// double edge value (different edge value type)
using DblGraph = undirected_adjacency_list<double, int, int>;
// std::string vertex value
using StrVGraph = undirected_adjacency_list<int, std::string, int>;

// Collect the neighbor ids of vertex uid into a multiset (handles parallel edges/self-loops).
template <class G>
static std::multiset<typename G::vertex_id_type> neighbors_of(G& g, typename G::vertex_id_type uid) {
  std::multiset<typename G::vertex_id_type> result;
  auto& v = g.vertices()[uid];
  for (auto& e : v.edges(g, uid)) {
    auto owner  = e.list_owner_id();
    auto target = e.list_target_id();
    result.insert(owner == uid ? target : owner);
  }
  return result;
}

TEST_CASE("add_vertex variants", "[undirected_adjacency_list][mutation][add_vertex]") {
  SECTION("default value vertices") {
    IntGraph g(0);
    g.add_vertex();
    g.add_vertex();
    REQUIRE(num_vertices(g) == 2);
  }

  SECTION("copied value vertices") {
    IntGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_vertex(30);
    REQUIRE(num_vertices(g) == 3);
    REQUIRE(g.vertices()[0].value() == 10);
    REQUIRE(g.vertices()[2].value() == 30);
  }

  SECTION("moved value vertices") {
    StrVGraph g(0);
    std::string s = "alice";
    g.add_vertex(std::move(s));
    REQUIRE(num_vertices(g) == 1);
    REQUIRE(g.vertices()[0].value() == "alice");
  }
}

TEST_CASE("add_edge variants", "[undirected_adjacency_list][mutation][add_edge]") {
  SECTION("by id with value") {
    IntGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_vertex(30);
    g.add_edge(0, 1, 100);
    g.add_edge(1, 2, 200);
    REQUIRE(num_edges(g) == 2);
    REQUIRE(contains_edge(g, 0u, 1u));
    REQUIRE(contains_edge(g, 1u, 2u));
    REQUIRE_FALSE(contains_edge(g, 0u, 2u));
  }

  SECTION("undirected: edge visible from both endpoints") {
    IntGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_edge(0, 1, 100);
    REQUIRE(g.vertices()[0].num_edges() == 1);
    REQUIRE(g.vertices()[1].num_edges() == 1);
    REQUIRE(neighbors_of(g, 0u) == std::multiset<unsigned int>{1});
    REQUIRE(neighbors_of(g, 1u) == std::multiset<unsigned int>{0});
  }

  SECTION("double edge value graph") {
    DblGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_edge(0, 1, 2.5);
    REQUIRE(num_edges(g) == 1);
    REQUIRE(contains_edge(g, 0u, 1u));
  }
}

TEST_CASE("remove_edge by iterator", "[undirected_adjacency_list][mutation][remove_edge]") {
  IntGraph g(0);
  g.add_vertex(10);
  g.add_vertex(20);
  g.add_vertex(30);
  g.add_edge(0, 1, 100);
  g.add_edge(0, 2, 200);
  g.add_edge(1, 2, 300);
  REQUIRE(num_edges(g) == 3);

  // Remove the first physical edge encountered in the graph-wide edge list.
  auto pos = g.edges().begin();
  g.remove_edge(pos);
  REQUIRE(num_edges(g) == 2);
}

TEST_CASE("remove_edge by endpoint ids", "[undirected_adjacency_list][mutation][remove_edge]") {
  SECTION("removes the matching undirected edge from both endpoints") {
    IntGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_vertex(30);
    g.add_edge(0, 1, 100);
    g.add_edge(0, 2, 200);
    g.add_edge(1, 2, 300);
    REQUIRE(num_edges(g) == 3);

    auto removed = g.remove_edge(0, 1);
    REQUIRE(removed == 1);
    REQUIRE(num_edges(g) == 2);
    REQUIRE_FALSE(contains_edge(g, 0u, 1u));
    REQUIRE(contains_edge(g, 0u, 2u));
    REQUIRE(contains_edge(g, 1u, 2u));
    REQUIRE(g.vertices()[0].num_edges() == 1);
    REQUIRE(g.vertices()[1].num_edges() == 1);
  }

  SECTION("symmetric: order of endpoints does not matter") {
    IntGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_edge(0, 1, 100);
    auto removed = g.remove_edge(1, 0);
    REQUIRE(removed == 1);
    REQUIRE(num_edges(g) == 0);
  }

  SECTION("removing a non-existent edge returns 0") {
    IntGraph g(0);
    g.add_vertex(10);
    g.add_vertex(20);
    g.add_vertex(30);
    g.add_edge(0, 1, 100);
    auto removed = g.remove_edge(1, 2);
    REQUIRE(removed == 0);
    REQUIRE(num_edges(g) == 1);
  }

  SECTION("out-of-range endpoint throws") {
    IntGraph g(0);
    g.add_vertex(10);
    REQUIRE_THROWS_AS(g.remove_edge(0, 5), std::out_of_range);
  }
}

TEST_CASE("remove_vertex renumbers higher ids", "[undirected_adjacency_list][mutation][remove_vertex]") {
  SECTION("isolated vertex in the middle") {
    IntGraph g(0);
    g.add_vertex(10); // 0
    g.add_vertex(20); // 1 (isolated, to be removed)
    g.add_vertex(30); // 2
    g.add_edge(0, 2, 200);
    REQUIRE(num_vertices(g) == 3);
    REQUIRE(num_edges(g) == 1);

    g.remove_vertex(1);

    REQUIRE(num_vertices(g) == 2);
    REQUIRE(num_edges(g) == 1);
    // Old vertex 2 (value 30) is now vertex 1.
    REQUIRE(g.vertices()[0].value() == 10);
    REQUIRE(g.vertices()[1].value() == 30);
    // Edge 0--2 is now 0--1.
    REQUIRE(contains_edge(g, 0u, 1u));
    REQUIRE(neighbors_of(g, 0u) == std::multiset<unsigned int>{1});
    REQUIRE(neighbors_of(g, 1u) == std::multiset<unsigned int>{0});
  }

  SECTION("connected vertex: incident edges are removed") {
    IntGraph g(0);
    g.add_vertex(10); // 0
    g.add_vertex(20); // 1 (to be removed; has 2 edges)
    g.add_vertex(30); // 2
    g.add_vertex(40); // 3
    g.add_edge(0, 1, 100);
    g.add_edge(1, 2, 200);
    g.add_edge(2, 3, 300);
    REQUIRE(num_edges(g) == 3);

    g.remove_vertex(1);

    REQUIRE(num_vertices(g) == 3);
    // Edges 0-1 and 1-2 removed; only the old 2-3 edge remains.
    REQUIRE(num_edges(g) == 1);
    // Old vertices 2,3 (values 30,40) become ids 1,2.
    REQUIRE(g.vertices()[0].value() == 10);
    REQUIRE(g.vertices()[1].value() == 30);
    REQUIRE(g.vertices()[2].value() == 40);
    REQUIRE(contains_edge(g, 1u, 2u));
    REQUIRE_FALSE(contains_edge(g, 0u, 1u));
    REQUIRE(g.vertices()[0].num_edges() == 0);
  }

  SECTION("removing the last vertex needs no renumbering") {
    IntGraph g(0);
    g.add_vertex(10); // 0
    g.add_vertex(20); // 1
    g.add_vertex(30); // 2
    g.add_edge(0, 1, 100);
    g.add_edge(1, 2, 200);

    g.remove_vertex(2);

    REQUIRE(num_vertices(g) == 2);
    REQUIRE(num_edges(g) == 1);
    REQUIRE(contains_edge(g, 0u, 1u));
    REQUIRE(neighbors_of(g, 1u) == std::multiset<unsigned int>{0});
  }

  SECTION("removing vertex 0 shifts everything down") {
    IntGraph g(0);
    g.add_vertex(10); // 0 (removed)
    g.add_vertex(20); // 1
    g.add_vertex(30); // 2
    g.add_edge(1, 2, 200);

    g.remove_vertex(0);

    REQUIRE(num_vertices(g) == 2);
    REQUIRE(num_edges(g) == 1);
    REQUIRE(g.vertices()[0].value() == 20);
    REQUIRE(g.vertices()[1].value() == 30);
    REQUIRE(contains_edge(g, 0u, 1u));
  }

  SECTION("out-of-range id throws") {
    IntGraph g(0);
    g.add_vertex(10);
    REQUIRE_THROWS_AS(g.remove_vertex(5), std::out_of_range);
  }
}
