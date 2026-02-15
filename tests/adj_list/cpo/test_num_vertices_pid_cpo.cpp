/**
 * @file test_num_vertices_pid_cpo.cpp
 * @brief Tests for num_vertices(g, pid) CPO
 * 
 * Tests the num_vertices(g, pid) CPO with different graph representations.
 * This file focuses on the default implementation which returns:
 * - num_vertices(g) when pid == 0 (single partition)
 * - 0 when pid != 0 (no such partition exists)
 * 
 * Resolution order:
 * 1. g.num_vertices(pid) - member function (highest priority)
 * 2. num_vertices(g, pid) - ADL (medium priority)
 * 3. Default: returns num_vertices(g) if pid==0, 0 otherwise (lowest priority)
 * 
 * Verifies:
 * - Default returns total vertex count for partition 0
 * - Default returns 0 for non-zero partitions
 * - Works with different graph storage types
 * - Consistent with num_vertices(g)
 * - Correct noexcept specification
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <map>
#include <deque>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Default Implementation Tests - Single Partition
// =============================================================================

TEST_CASE("num_vertices(g,pid) - vector graph partition 0 returns total count", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2}, {2, 3}, {3}, {0, 1, 2}};

  SECTION("partition 0 returns all vertices") {
    auto count = num_vertices(graph, 0);
    REQUIRE(count == 4);
  }

  SECTION("partition 0 matches num_vertices(g)") {
    auto count_all = num_vertices(graph);
    auto count_p0  = num_vertices(graph, 0);
    REQUIRE(count_all == count_p0);
  }
}

TEST_CASE("num_vertices(g,pid) - vector graph non-zero partition returns 0", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2}, {2, 3}, {3}};

  SECTION("partition 1 returns 0") {
    auto count = num_vertices(graph, 1);
    REQUIRE(count == 0);
  }

  SECTION("partition 5 returns 0") {
    auto count = num_vertices(graph, 5);
    REQUIRE(count == 0);
  }
}

TEST_CASE("num_vertices(g,pid) - map graph partition 0", "[num_vertices_pid][cpo][default]") {
  using Graph = std::map<int, std::vector<int>>;
  Graph graph = {{0, {1, 2}}, {1, {2, 3}}, {2, {3}}, {3, {}}};

  SECTION("partition 0 returns all vertices") {
    auto count = num_vertices(graph, 0);
    REQUIRE(count == 4);
  }

  SECTION("partition 1 returns 0") {
    auto count = num_vertices(graph, 1);
    REQUIRE(count == 0);
  }
}

TEST_CASE("num_vertices(g,pid) - deque graph partition 0", "[num_vertices_pid][cpo][default]") {
  using Graph = std::deque<std::deque<int>>;
  Graph graph = {{1}, {2}, {3}};

  auto count = num_vertices(graph, 0);
  REQUIRE(count == 3);
}

TEST_CASE("num_vertices(g,pid) - empty graph", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph;

  SECTION("partition 0 returns 0") {
    auto count = num_vertices(graph, 0);
    REQUIRE(count == 0);
  }

  SECTION("partition 1 returns 0") {
    auto count = num_vertices(graph, 1);
    REQUIRE(count == 0);
  }
}

TEST_CASE("num_vertices(g,pid) - single vertex graph", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2, 3}};

  SECTION("partition 0 has one vertex") {
    auto count = num_vertices(graph, 0);
    REQUIRE(count == 1);
  }

  SECTION("partition 1 has zero vertices") {
    auto count = num_vertices(graph, 1);
    REQUIRE(count == 0);
  }
}

TEST_CASE("num_vertices(g,pid) - const graph", "[num_vertices_pid][cpo][default]") {
  using Graph       = std::vector<std::vector<int>>;
  const Graph graph = {{1, 2}, {2, 3}, {3}};

  auto count = num_vertices(graph, 0);
  REQUIRE(count == 3);
}

TEST_CASE("num_vertices(g,pid) - negative partition id", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1}, {2}};

  // Negative partition IDs return 0 (don't exist)
  auto count = num_vertices(graph, -1);
  REQUIRE(count == 0);
}

TEST_CASE("num_vertices(g,pid) - large partition id", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1}, {2}, {3}};

  // Large partition IDs return 0 (don't exist)
  auto count = num_vertices(graph, 999);
  REQUIRE(count == 0);
}

TEST_CASE("num_vertices(g,pid) - return type is integral", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2}};

  auto count = num_vertices(graph, 0);
  STATIC_REQUIRE(std::integral<decltype(count)>);
}

TEST_CASE("num_vertices(g,pid) - noexcept for default implementation", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2}};

  STATIC_REQUIRE(noexcept(num_vertices(graph, 0)));
}

TEST_CASE("num_vertices(g,pid) - works with different partition ID types", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1}, {2}};

  SECTION("int partition id") {
    auto count = num_vertices(graph, 0);
    REQUIRE(count == 2);
  }

  SECTION("size_t partition id") {
    auto count = num_vertices(graph, size_t(0));
    REQUIRE(count == 2);
  }

  SECTION("uint32_t partition id") {
    auto count = num_vertices(graph, uint32_t(0));
    REQUIRE(count == 2);
  }
}

TEST_CASE("num_vertices(g,pid) - multiple calls consistent", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2}, {2, 3}, {3}};

  auto count1 = num_vertices(graph, 0);
  auto count2 = num_vertices(graph, 0);

  REQUIRE(count1 == count2);
  REQUIRE(count1 == 3);
}

TEST_CASE("num_vertices(g,pid) - large graph", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph(1000);

  auto count = num_vertices(graph, 0);
  REQUIRE(count == 1000);

  auto count_p1 = num_vertices(graph, 1);
  REQUIRE(count_p1 == 0);
}

TEST_CASE("num_vertices(g,pid) - integration with partition_id", "[num_vertices_pid][cpo][default]") {
  using Graph = std::vector<std::vector<int>>;
  Graph graph = {{1, 2}, {2, 3}, {3}};

  // For single partition graphs, all vertices should be in partition 0
  auto num_parts = num_partitions(graph);
  REQUIRE(num_parts == 1);

  auto verts_p0  = num_vertices(graph, 0);
  auto verts_all = num_vertices(graph);
  REQUIRE(verts_p0 == verts_all);

  // Sum of vertices across all partitions should equal total
  size_t total = 0;
  for (size_t pid = 0; pid < static_cast<size_t>(num_parts); ++pid) {
    total += num_vertices(graph, static_cast<int>(pid));
  }
  REQUIRE(total == static_cast<size_t>(verts_all));
}
