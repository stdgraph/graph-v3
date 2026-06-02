#include <catch2/catch_test_macros.hpp>

#include "graph/container/adjacency_matrix.hpp"

#include <algorithm>
#include <csignal>
#include <vector>

#if !defined(NDEBUG) && (defined(__unix__) || defined(__APPLE__))
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

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
  REQUIRE(g.exists(0, 1));
  REQUIRE_FALSE(g.exists(3, 0));
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
  REQUIRE(g.exists(0, 1));
  REQUIRE(g.exists(1, 0));
  REQUIRE_FALSE(g.exists(0, 2));
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

  REQUIRE(g(0, 1) == 1.5);
  REQUIRE(g(1, 2) == 3.5);
  REQUIRE(g.exists(0, 1));
  REQUIRE_FALSE(g.exists(2, 0));

  const adjacency_matrix<double>& cg2 = g;
  REQUIRE(cg2(0, 1) == 1.5);

  auto vs = vertices(g);
  auto u0 = *vs.begin();

  // out_edges yields the present targets; the weight is recovered via the
  // edge_value CPO.
  std::vector<std::pair<std::size_t, double>> edges;
  for (auto uv : out_edges(g, u0)) {
    edges.emplace_back(static_cast<std::size_t>(target_id(g, uv)), edge_value(g, uv));
  }
  std::ranges::sort(edges);

  REQUIRE(edges.size() == 2);
  REQUIRE(edges[0] == std::pair<std::size_t, double>{1, 1.5});
  REQUIRE(edges[1] == std::pair<std::size_t, double>{2, 2.5});
}

#if !defined(NDEBUG) && (defined(__unix__) || defined(__APPLE__))
TEST_CASE("adjacency_matrix: operator()(u,v) asserts on missing edge in debug", "[container][adjacency_matrix]") {
  adjacency_matrix<double> g(3);
  g.add_edge(0, 1, 1.5);

  const pid_t pid = fork();
  REQUIRE(pid >= 0);

  if (pid == 0) {
    (void)g(2, 0); // Missing edge -> debug assert should fire.
    _exit(0);
  }

  int status = 0;
  REQUIRE(waitpid(pid, &status, 0) == pid);
  REQUIRE(WIFSIGNALED(status));
  REQUIRE(WTERMSIG(status) == SIGABRT);
}
#endif

// =============================================================================
// Edge-range + projection constructor
// =============================================================================

TEST_CASE("adjacency_matrix: construct from copyable_edge range (identity)", "[container][adjacency_matrix]") {
  std::vector<graph::copyable_edge_t<std::uint32_t>> ee = {{0, 1}, {0, 2}, {1, 2}, {2, 3}};

  adjacency_matrix<> g(4, ee);

  REQUIRE(g.num_vertices() == 4);
  REQUIRE(g.num_edges() == 4);
  REQUIRE(g.has_edge(0, 1));
  REQUIRE(g.exists(0, 1));
  REQUIRE(g.has_edge(2, 3));
  REQUIRE_FALSE(g.has_edge(3, 0));
  REQUIRE_FALSE(g.exists(3, 0));
}

TEST_CASE("adjacency_matrix: construct from edge range with projection", "[container][adjacency_matrix]") {
  struct raw_edge {
    int from;
    int to;
    double w;
  };
  std::vector<raw_edge> ee = {{0, 1, 1.5}, {0, 2, 2.5}, {1, 2, 3.5}};

  auto proj = [](const raw_edge& e) {
    return graph::copyable_edge_t<std::uint32_t, double>{static_cast<std::uint32_t>(e.from),
                                                         static_cast<std::uint32_t>(e.to), e.w};
  };

  adjacency_matrix<double> g(3, ee, proj);

  REQUIRE(g.num_edges() == 3);
  REQUIRE(g(0, 1) == 1.5);
  REQUIRE(g(0, 2) == 2.5);
  REQUIRE(g(1, 2) == 3.5);
}

