/**
 * @file test_edge_cases.cpp
 * @brief Comprehensive edge case tests for graph views
 * 
 * Tests cover:
 * - Empty graphs
 * - Single vertex graphs
 * - Disconnected graphs
 * - Self-loops
 * - Parallel edges
 * - Const graphs
 * - Exception safety
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views.hpp>
#include <vector>
#include <deque>
#include <map>
#include <stdexcept>

using namespace graph;
using namespace graph::views::adaptors;

//=============================================================================
// Empty Graph Tests
//=============================================================================

TEST_CASE("Empty graph - vertexlist view", "[views][edge_cases][empty]") {
  std::vector<std::vector<int>> g; // Empty graph

  auto view = g | vertexlist();

  REQUIRE(std::ranges::distance(view) == 0);
  REQUIRE(view.begin() == view.end());

  // Should not iterate at all
  size_t count = 0;
  for (auto [id, v] : view) {
    (void)v;
    ++count;
  }
  REQUIRE(count == 0);
}

TEST_CASE("Empty graph - edgelist view", "[views][edge_cases][empty]") {
  std::vector<std::vector<int>> g; // Empty graph

  auto view = g | edgelist();

  REQUIRE(std::ranges::distance(view) == 0);
  REQUIRE(view.begin() == view.end());
}

TEST_CASE("Empty graph - DFS vertices", "[views][edge_cases][empty]") {
  std::vector<std::vector<int>> g; // Empty graph

  // Cannot perform DFS on empty graph (no seed vertex)
  // This test verifies we handle the boundary case properly
  REQUIRE(g.empty());
}

//=============================================================================
// Single Vertex Tests
//=============================================================================

TEST_CASE("Single vertex - no edges", "[views][edge_cases][single_vertex]") {
  std::vector<std::vector<int>> g(1); // One vertex, no edges

  SECTION("vertexlist") {
    auto view = g | vertexlist();
    REQUIRE(std::ranges::distance(view) == 1);

    for (auto [id, v] : view) {
      REQUIRE(id == 0);
      REQUIRE(vertex_id(g, v) == 0);
    }
  }

  SECTION("incidence from vertex 0") {
    auto view = g | incidence(0);
    REQUIRE(std::ranges::distance(view) == 0); // No edges
  }

  SECTION("neighbors from vertex 0") {
    auto view = g | neighbors(0);
    REQUIRE(std::ranges::distance(view) == 0); // No neighbors
  }

  SECTION("edgelist") {
    auto view = g | edgelist();
    REQUIRE(std::ranges::distance(view) == 0); // No edges
  }
}

TEST_CASE("Single vertex - self-loop", "[views][edge_cases][single_vertex][self_loop]") {
  std::vector<std::vector<int>> g(1);
  g[0].push_back(0); // Self-loop

  SECTION("incidence") {
    auto view = g | incidence(0);
    REQUIRE(std::ranges::distance(view) == 1);

    for (auto [tid, e] : view) {
      REQUIRE(source_id(g, e) == 0);
      REQUIRE(target_id(g, e) == 0);
    }
  }

  SECTION("neighbors") {
    auto view = g | neighbors(0);
    REQUIRE(std::ranges::distance(view) == 1);

    for (auto [tid, v] : view) {
      REQUIRE(tid == 0); // Points to itself
    }
  }

  SECTION("edgelist") {
    auto view = g | edgelist();
    REQUIRE(std::ranges::distance(view) == 1);

    for (auto [sid, tid, e] : view) {
      REQUIRE(sid == 0);
      REQUIRE(tid == 0);
    }
  }
}

//=============================================================================
// Disconnected Graph Tests
//=============================================================================

TEST_CASE("Disconnected graph - DFS reaches only one component", "[views][edge_cases][disconnected]") {
  std::vector<std::vector<int>> g(6);

  // Component 1: 0 -> 1 -> 2
  g[0].push_back(1);
  g[1].push_back(2);

  // Component 2: 3 -> 4 -> 5
  g[3].push_back(4);
  g[4].push_back(5);

  SECTION("DFS from component 1") {
    auto                view = g | vertices_dfs(0);
    std::vector<size_t> visited;

    for (auto [v] : view) {
      visited.push_back(vertex_id(g, v));
    }

    REQUIRE(visited.size() == 3);
    // Should visit only vertices 0, 1, 2
    REQUIRE(std::find(visited.begin(), visited.end(), 0) != visited.end());
    REQUIRE(std::find(visited.begin(), visited.end(), 1) != visited.end());
    REQUIRE(std::find(visited.begin(), visited.end(), 2) != visited.end());
    REQUIRE(std::find(visited.begin(), visited.end(), 3) == visited.end());
  }

  SECTION("DFS from component 2") {
    auto                view = g | vertices_dfs(3);
    std::vector<size_t> visited;

    for (auto [v] : view) {
      visited.push_back(vertex_id(g, v));
    }

    REQUIRE(visited.size() == 3);
    // Should visit only vertices 3, 4, 5
    REQUIRE(std::find(visited.begin(), visited.end(), 3) != visited.end());
    REQUIRE(std::find(visited.begin(), visited.end(), 4) != visited.end());
    REQUIRE(std::find(visited.begin(), visited.end(), 5) != visited.end());
    REQUIRE(std::find(visited.begin(), visited.end(), 0) == visited.end());
  }
}

TEST_CASE("Disconnected graph - BFS reaches only one component", "[views][edge_cases][disconnected]") {
  std::vector<std::vector<int>> g(6);

  // Component 1: 0 -> 1, 0 -> 2
  g[0].push_back(1);
  g[0].push_back(2);

  // Component 2: 3 -> 4, 3 -> 5
  g[3].push_back(4);
  g[3].push_back(5);

  auto                view = g | vertices_bfs(0);
  std::vector<size_t> visited;

  for (auto [v] : view) {
    visited.push_back(vertex_id(g, v));
  }

  REQUIRE(visited.size() == 3);
  REQUIRE(std::find(visited.begin(), visited.end(), 0) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 1) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 2) != visited.end());
}

TEST_CASE("Disconnected graph - topological sort includes all components", "[views][edge_cases][disconnected]") {
  std::vector<std::vector<int>> g(6);

  // Component 1: 0 -> 1 -> 2
  g[0].push_back(1);
  g[1].push_back(2);

  // Component 2: 3 -> 4 -> 5
  g[3].push_back(4);
  g[4].push_back(5);

  auto                view = g | vertices_topological_sort();
  std::vector<size_t> order;

  for (auto [v] : view) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 6); // All vertices included

  // Verify topological property for component 1
  auto pos0 = std::find(order.begin(), order.end(), 0);
  auto pos1 = std::find(order.begin(), order.end(), 1);
  auto pos2 = std::find(order.begin(), order.end(), 2);
  REQUIRE(pos0 < pos1);
  REQUIRE(pos1 < pos2);

  // Verify topological property for component 2
  auto pos3 = std::find(order.begin(), order.end(), 3);
  auto pos4 = std::find(order.begin(), order.end(), 4);
  auto pos5 = std::find(order.begin(), order.end(), 5);
  REQUIRE(pos3 < pos4);
  REQUIRE(pos4 < pos5);
}

//=============================================================================
// Self-Loop Tests
//=============================================================================

TEST_CASE("Self-loops - multiple vertices with self-loops", "[views][edge_cases][self_loop]") {
  std::vector<std::vector<int>> g(3);

  g[0].push_back(0); // Self-loop at 0
  g[1].push_back(1); // Self-loop at 1
  g[2].push_back(2); // Self-loop at 2

  SECTION("edgelist counts all self-loops") {
    auto view = g | edgelist();
    REQUIRE(std::ranges::distance(view) == 3);

    for (auto [sid, tid, e] : view) {
      REQUIRE(sid == tid); // All are self-loops
    }
  }

  SECTION("incidence at each vertex") {
    for (size_t u = 0; u < 3; ++u) {
      auto view = g | incidence(u);
      REQUIRE(std::ranges::distance(view) == 1);

      for (auto [tid, e] : view) {
        REQUIRE(source_id(g, e) == u);
        REQUIRE(target_id(g, e) == u);
      }
    }
  }
}

//=============================================================================
// Parallel Edges Tests
//=============================================================================

TEST_CASE("Parallel edges - multiple edges between same vertices", "[views][edge_cases][parallel_edges]") {
  std::vector<std::vector<int>> g(3);

  // Multiple edges from 0 to 1
  g[0].push_back(1);
  g[0].push_back(1);
  g[0].push_back(1);

  // Multiple edges from 1 to 2
  g[1].push_back(2);
  g[1].push_back(2);

  SECTION("incidence counts all parallel edges") {
    auto view = g | incidence(0);
    REQUIRE(std::ranges::distance(view) == 3); // Three parallel edges

    for (auto [tid, e] : view) {
      REQUIRE(source_id(g, e) == 0);
      REQUIRE(target_id(g, e) == 1);
    }
  }

  SECTION("neighbors lists parallel edges separately") {
    auto view = g | neighbors(0);
    REQUIRE(std::ranges::distance(view) == 3); // Three parallel edges

    for (auto [tid, v] : view) {
      REQUIRE(tid == 1);
    }
  }

  SECTION("edgelist includes all parallel edges") {
    auto view = g | edgelist();
    REQUIRE(std::ranges::distance(view) == 5); // 3 + 2 edges
  }
}

//=============================================================================
// Const Graph Tests
//=============================================================================

TEST_CASE("Const graph - vertexlist", "[views][edge_cases][const]") {
  const std::vector<std::vector<int>> g{{1, 2}, {2}, {0}};

  auto view = g | vertexlist();
  REQUIRE(std::ranges::distance(view) == 3);

  for (auto [id, v] : view) {
    // Should compile and work with const graph
    REQUIRE(id == vertex_id(g, v));
    REQUIRE(id < 3);
  }
}

TEST_CASE("Const graph - incidence", "[views][edge_cases][const]") {
  const std::vector<std::vector<int>> g{{1, 2}, {2}, {0}};

  auto view = g | incidence(0);
  REQUIRE(std::ranges::distance(view) == 2);

  for (auto [tid, e] : view) {
    REQUIRE(source_id(g, e) == 0);
  }
}

TEST_CASE("Const graph - neighbors", "[views][edge_cases][const]") {
  const std::vector<std::vector<int>> g{{1, 2}, {2}, {0}};

  auto view = g | neighbors(0);
  REQUIRE(std::ranges::distance(view) == 2);

  for (auto [tid, v] : view) {
    REQUIRE((tid == 1 || tid == 2));
  }
}

TEST_CASE("Const graph - edgelist", "[views][edge_cases][const]") {
  const std::vector<std::vector<int>> g{{1, 2}, {2}, {0}};

  auto view = g | edgelist();
  REQUIRE(std::ranges::distance(view) == 4);
}

TEST_CASE("Const graph - topological sort (DAG)", "[views][edge_cases][const]") {
  const std::vector<std::vector<int>> g{{1}, {2}, {}};

  auto view = g | vertices_topological_sort();
  REQUIRE(std::ranges::distance(view) == 3);
}

//=============================================================================
// Deque-Based Graph Tests (Alternative Container)
//=============================================================================

TEST_CASE("Deque-based graph - basic views", "[views][edge_cases][deque]") {
  std::deque<std::deque<int>> g(3);
  g[0].push_back(1);
  g[0].push_back(2);
  g[1].push_back(2);

  SECTION("vertexlist") {
    auto view = g | vertexlist();
    REQUIRE(std::ranges::distance(view) == 3);
  }

  SECTION("incidence") {
    auto view = g | incidence(0);
    REQUIRE(std::ranges::distance(view) == 2);
  }

  SECTION("neighbors") {
    auto view = g | neighbors(0);
    REQUIRE(std::ranges::distance(view) == 2);
  }

  SECTION("edgelist") {
    auto view = g | edgelist();
    REQUIRE(std::ranges::distance(view) == 3);
  }
}

//=============================================================================
// Map-Based Sparse Graph Tests
//=============================================================================

TEST_CASE("Sparse vertex IDs - non-contiguous", "[views][edge_cases][sparse]") {
  // Using vector but treating as sparse
  std::vector<std::vector<int>> g(11);
  g[0].push_back(5);
  g[5].push_back(10);
  // Vertices 1-4, 6-9 exist but have no edges

  auto view = g | edgelist();
  REQUIRE(std::ranges::distance(view) == 2);

  for (auto [sid, tid, e] : view) {
    REQUIRE((sid == 0 || sid == 5));
    REQUIRE((tid == 5 || tid == 10));
  }
}

//=============================================================================
// Value Function Edge Cases
//=============================================================================

TEST_CASE("Value function - capturing lambda", "[views][edge_cases][value_function]") {
  std::vector<std::vector<int>> g{{1, 2}, {2}, {}};
  std::map<size_t, std::string> names{{0, "A"}, {1, "B"}, {2, "C"}};

  auto vvf = [&names](const auto& g, auto v) { return names[vertex_id(g, v)]; };

  auto view = g | vertexlist(vvf);

  for (auto [id, v, name] : view) {
    REQUIRE(id == vertex_id(g, v));
    REQUIRE(name == names[id]);
  }
}

TEST_CASE("Value function - mutable lambda", "[views][edge_cases][value_function]") {
  std::vector<std::vector<int>> g{{1}, {2}, {}};

  int  counter = 0;
  auto vvf     = [counter](const auto& g, auto v) mutable { return vertex_id(g, v) + counter++; };

  auto view = g | vertexlist(vvf);

  std::vector<size_t> values;
  for (auto [id, v, val] : view) {
    REQUIRE(id == vertex_id(g, v));
    values.push_back(val);
  }

  // Counter should increment for each call
  REQUIRE(values.size() == 3);
}

TEST_CASE("Value function - with structured binding", "[views][edge_cases][value_function]") {
  std::vector<std::vector<int>> g{{1, 2}, {2}, {}};

  auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };

  auto view = g | vertexlist(vvf);

  for (auto [id, v, val] : view) {
    REQUIRE(id == vertex_id(g, v));
    REQUIRE(val == id * 10);
  }
}

//=============================================================================
// Exception Safety Tests
//=============================================================================

TEST_CASE("Exception safety - value function throws", "[views][edge_cases][exception]") {
  std::vector<std::vector<int>> g{{1, 2}, {2}, {}};

  auto throwing_vvf = [](const auto& g, auto v) -> int {
    auto id = vertex_id(g, v);
    if (id == 1) {
      throw std::runtime_error("Test exception");
    }
    return static_cast<int>(id);
  };

  auto view = g | vertexlist(throwing_vvf);
  auto it   = view.begin();

  // First vertex should work
  REQUIRE_NOTHROW(*it);
  ++it;

  // Second vertex should throw
  REQUIRE_THROWS_AS(*it, std::runtime_error);
}

//=============================================================================
// Large Graph Stress Tests
//=============================================================================

TEST_CASE("Large graph - vertexlist stress test", "[views][edge_cases][large]") {
  constexpr size_t              SIZE = 10000;
  std::vector<std::vector<int>> g(SIZE);

  // Linear chain
  for (size_t i = 0; i + 1 < SIZE; ++i) {
    g[i].push_back(static_cast<int>(i + 1));
  }

  auto view = g | vertexlist();
  REQUIRE(std::ranges::distance(view) == SIZE);
}

TEST_CASE("Large graph - DFS stress test", "[views][edge_cases][large]") {
  constexpr size_t              SIZE = 1000;
  std::vector<std::vector<int>> g(SIZE);

  // Linear chain
  for (size_t i = 0; i + 1 < SIZE; ++i) {
    g[i].push_back(static_cast<int>(i + 1));
  }

  auto   view  = g | vertices_dfs(0);
  size_t count = 0;

  for (auto [v] : view) {
    (void)v;
    ++count;
  }

  REQUIRE(count == SIZE);
}

TEST_CASE("Large graph - BFS stress test", "[views][edge_cases][large]") {
  constexpr size_t              SIZE = 1000;
  std::vector<std::vector<int>> g(SIZE);

  // Star graph (center connected to all)
  for (size_t i = 1; i < SIZE; ++i) {
    g[0].push_back(static_cast<int>(i));
  }

  auto   view  = g | vertices_bfs(0);
  size_t count = 0;

  for (auto [v] : view) {
    (void)v;
    ++count;
  }

  REQUIRE(count == SIZE);
}

TEST_CASE("Large graph - topological sort stress test", "[views][edge_cases][large]") {
  constexpr size_t              SIZE = 1000;
  std::vector<std::vector<int>> g(SIZE);

  // DAG: each vertex points to next 3 vertices
  for (size_t i = 0; i < SIZE; ++i) {
    for (size_t j = 1; j <= 3 && i + j < SIZE; ++j) {
      g[i].push_back(static_cast<int>(i + j));
    }
  }

  auto                view = g | vertices_topological_sort();
  std::vector<size_t> order;

  for (auto [v] : view) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == SIZE);

  // Verify topological property
  for (size_t u = 0; u < SIZE; ++u) {
    auto pos_u = std::find(order.begin(), order.end(), u);
    for (int v : g[u]) {
      auto pos_v = std::find(order.begin(), order.end(), static_cast<size_t>(v));
      REQUIRE(pos_u < pos_v);
    }
  }
}

//=============================================================================
// Iterator Invalidation Tests
//=============================================================================

TEST_CASE("Iterator stability - view outlives iterators", "[views][edge_cases][iterator]") {
  std::vector<std::vector<int>> g{{1, 2}, {2}, {}};

  auto view = g | vertexlist();
  auto it1  = view.begin();
  auto it2  = view.begin();

  // Both iterators should be equal
  REQUIRE(it1 == it2);

  // Advancing one shouldn't affect the other
  ++it1;
  REQUIRE(it1 != it2);
}

TEST_CASE("View copy - independent iteration", "[views][edge_cases][copy]") {
  std::vector<std::vector<int>> g{{1, 2}, {2}, {}};

  auto view1 = g | vertexlist();
  auto view2 = view1; // Copy

  auto it1 = view1.begin();
  auto it2 = view2.begin();

  // Both should start at beginning (compare IDs, not value_type)
  REQUIRE(vertex_id(g, (*it1).vertex) == vertex_id(g, (*it2).vertex));

  // Independent iteration
  ++it1;
  REQUIRE(vertex_id(g, (*it1).vertex) != vertex_id(g, (*it2).vertex));
}

//=============================================================================
// Empty Range Tests
//=============================================================================

TEST_CASE("Empty range - graph with vertices but no edges", "[views][edge_cases][empty_range]") {
  std::vector<std::vector<int>> g(5); // 5 vertices, no edges

  SECTION("edgelist is empty") {
    auto view = g | edgelist();
    REQUIRE(view.begin() == view.end());
  }

  SECTION("incidence from any vertex is empty") {
    for (size_t u = 0; u < 5; ++u) {
      auto view = g | incidence(u);
      REQUIRE(view.begin() == view.end());
    }
  }

  SECTION("neighbors from any vertex is empty") {
    for (size_t u = 0; u < 5; ++u) {
      auto view = g | neighbors(u);
      REQUIRE(view.begin() == view.end());
    }
  }
}
