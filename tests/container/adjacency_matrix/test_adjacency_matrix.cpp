#include <catch2/catch_test_macros.hpp>

#include "graph/container/adjacency_matrix.hpp"

#include <algorithm>
#include <vector>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// =============================================================================
// Concept conformance
// =============================================================================

static_assert(std::ranges::random_access_range<adjacency_matrix<>>,
              "matrix must be a random-access range to drive the inner-value pattern");
static_assert(adjacency_list<adjacency_matrix<>>, "unweighted matrix must model adjacency_list");
static_assert(index_adjacency_list<adjacency_matrix<>>, "unweighted matrix must model index_adjacency_list");
static_assert(adjacency_list<adjacency_matrix<double>>, "weighted matrix must model adjacency_list");
static_assert(index_adjacency_list<adjacency_matrix<double>>, "weighted matrix must model index_adjacency_list");

// =============================================================================
// Unweighted, directed
// =============================================================================

TEST_CASE("adjacency_matrix: unweighted directed basics", "[container][adjacency_matrix]") {
  adjacency_matrix<> g(4);
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 2);
  g.add_edge(2, 3);

  REQUIRE(g.num_vertices() == 4);
  REQUIRE(g.num_edges() == 4);
  REQUIRE(g.has_edge(0, 1));
  REQUIRE_FALSE(g.has_edge(1, 0));

  SECTION("vertices() yields sequential integral ids") {
    std::vector<std::size_t> ids;
    for (auto u : vertices(g)) {
      ids.push_back(u.vertex_id());
    }
    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2, 3});
  }

  SECTION("out_edges yields only present targets") {
    auto collect = [&](auto u) {
      std::vector<std::size_t> targets;
      for (auto uv : out_edges(g, u)) {
        targets.push_back(static_cast<std::size_t>(target_id(g, uv)));
      }
      std::ranges::sort(targets);
      return targets;
    };

    auto vs = vertices(g);
    auto it = vs.begin();
    REQUIRE(collect(*it) == std::vector<std::size_t>{1, 2}); // vertex 0
    ++it;
    REQUIRE(collect(*it) == std::vector<std::size_t>{2}); // vertex 1
    ++it;
    REQUIRE(collect(*it) == std::vector<std::size_t>{3}); // vertex 2
    ++it;
    REQUIRE(collect(*it).empty()); // vertex 3 has no out-edges
  }
}

// =============================================================================
// Unweighted, undirected (symmetric)
// =============================================================================

TEST_CASE("adjacency_matrix: undirected adds reciprocal edge", "[container][adjacency_matrix]") {
  adjacency_matrix<void, std::uint32_t, /*Directed=*/false> g(3);
  g.add_edge(0, 1);
  g.add_edge(1, 2);

  REQUIRE(g.has_edge(0, 1));
  REQUIRE(g.has_edge(1, 0));
  REQUIRE(g.has_edge(1, 2));
  REQUIRE(g.has_edge(2, 1));
  REQUIRE(g.num_edges() == 4);
}

// =============================================================================
// Weighted
// =============================================================================

TEST_CASE("adjacency_matrix: weighted edges expose target + value", "[container][adjacency_matrix]") {
  adjacency_matrix<double> g(3);
  g.add_edge(0, 1, 1.5);
  g.add_edge(0, 2, 2.5);
  g.add_edge(1, 2, 3.5);

  REQUIRE(g.weight(0, 1) == 1.5);
  REQUIRE(g.weight(1, 2) == 3.5);

  auto vs = vertices(g);
  auto u0 = *vs.begin();

  // out_edges yields the present targets; the weight is recovered via the
  // edge_value CPO (the stored edge element is a {target, weight} pair).
  std::vector<std::pair<std::size_t, double>> edges;
  for (auto uv : out_edges(g, u0)) {
    edges.emplace_back(static_cast<std::size_t>(target_id(g, uv)), edge_value(g, uv));
  }
  std::ranges::sort(edges);

  REQUIRE(edges.size() == 2);
  REQUIRE(edges[0] == std::pair<std::size_t, double>{1, 1.5});
  REQUIRE(edges[1] == std::pair<std::size_t, double>{2, 2.5});
}
