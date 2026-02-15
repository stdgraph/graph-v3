/**
 * @file test_dfs.cpp
 * @brief Comprehensive tests for DFS views
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/vertexlist.hpp>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <set>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Basic DFS Traversal Order
// =============================================================================

TEST_CASE("vertices_dfs - basic traversal order", "[dfs][vertices]") {
  //     0
  //    / \
    //   1   2
  //  / \
    // 3   4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {},     // 2 (leaf)
        {},     // 3 (leaf)
        {}      // 4 (leaf)
  };

  SECTION("DFS from vertex 0") {
    std::vector<int> visited_order;

    for (auto [v] : vertices_dfs(g, 0)) {
      visited_order.push_back(static_cast<int>(vertex_id(g, v)));
    }

    // DFS should visit in depth-first order
    // Starting at 0, go to 1, then 3, backtrack to 1, go to 4, backtrack to 0, go to 2
    REQUIRE(visited_order.size() == 5);
    REQUIRE(visited_order[0] == 0); // Start at seed

    // All vertices visited
    std::set<int> visited_set(visited_order.begin(), visited_order.end());
    REQUIRE(visited_set == std::set<int>{0, 1, 2, 3, 4});
  }

  SECTION("DFS from leaf vertex") {
    std::vector<int> visited_order;

    for (auto [v] : vertices_dfs(g, 3)) {
      visited_order.push_back(static_cast<int>(vertex_id(g, v)));
    }

    // Only vertex 3 should be visited (no outgoing edges)
    REQUIRE(visited_order == std::vector<int>{3});
  }
}

// =============================================================================
// Test 2: Structured Bindings
// =============================================================================

TEST_CASE("vertices_dfs - structured bindings", "[dfs][vertices][bindings]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  SECTION("structured binding [v]") {
    std::vector<int> ids;

    for (auto [v] : vertices_dfs(g, 0)) {
      ids.push_back(static_cast<int>(vertex_id(g, v)));
    }

    REQUIRE(ids.size() == 3);
  }

  SECTION("structured binding [v, val] with value function") {
    auto dfs = vertices_dfs(g, 0, [](const auto& g, auto v) { return vertex_id(g, v) * 10; });

    std::vector<std::pair<int, int>> results;
    for (auto [v, val] : dfs) {
      results.emplace_back(static_cast<int>(vertex_id(g, v)), val);
    }

    REQUIRE(results.size() == 3);
    for (auto& [id, val] : results) {
      REQUIRE(val == id * 10);
    }
  }
}

// =============================================================================
// Test 3: Visited Tracking (No Revisits)
// =============================================================================

TEST_CASE("vertices_dfs - visited tracking", "[dfs][vertices][visited]") {
  // Graph with cycle: 0 -> 1 -> 2 -> 0
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {0}  // 2 -> 0 (back edge)
  };

  std::vector<int> visited_order;

  for (auto [v] : vertices_dfs(g, 0)) {
    visited_order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Each vertex visited exactly once despite cycle
  REQUIRE(visited_order.size() == 3);
  std::set<int> visited_set(visited_order.begin(), visited_order.end());
  REQUIRE(visited_set == std::set<int>{0, 1, 2});
}

// =============================================================================
// Test 4: Value Function Types
// =============================================================================

TEST_CASE("vertices_dfs - value function types", "[dfs][vertices][vvf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};

  SECTION("returning int") {
    auto dfs = vertices_dfs(g, 0, [](const auto& g, auto v) { return static_cast<int>(vertex_id(g, v)); });

    int sum = 0;
    for (auto [v, val] : dfs) {
      sum += val;
    }
    REQUIRE(sum == 0 + 1 + 2);
  }

  SECTION("returning string") {
    auto dfs = vertices_dfs(g, 0, [](const auto& g, auto v) { return "v" + std::to_string(vertex_id(g, v)); });

    std::vector<std::string> names;
    for (auto [v, name] : dfs) {
      names.push_back(name);
    }

    REQUIRE(names.size() == 3);
    REQUIRE(std::find(names.begin(), names.end(), "v0") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "v1") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "v2") != names.end());
  }

  SECTION("capturing lambda") {
    int  multiplier = 5;
    auto dfs        = vertices_dfs(
          g, 0, [multiplier](const auto& g, auto v) { return static_cast<int>(vertex_id(g, v)) * multiplier; });

    std::vector<int> values;
    for (auto [v, val] : dfs) {
      values.push_back(val);
    }

    REQUIRE(values.size() == 3);
    // Values are id * 5
    std::set<int> value_set(values.begin(), values.end());
    REQUIRE(value_set == std::set<int>{0, 5, 10});
  }
}

// =============================================================================
// Test 5: Depth and Size Accessors
// =============================================================================

TEST_CASE("vertices_dfs - depth and size accessors", "[dfs][vertices][accessors]") {
  //     0
  //    /|\
    //   1 2 3
  //   |
  //   4
  //   |
  //   5
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2, 3}, // 0 -> 1, 2, 3
        {4},       // 1 -> 4
        {},        // 2 (leaf)
        {},        // 3 (leaf)
        {5},       // 4 -> 5
        {}         // 5 (leaf)
  };

  auto dfs = vertices_dfs(g, 0);

  // Before iteration
  REQUIRE(dfs.depth() == 1);       // seed is on stack
  REQUIRE(dfs.num_visited() == 0); // no vertices counted yet

  // Iterate
  std::vector<int> visited;
  for (auto [v] : dfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // After full iteration
  REQUIRE(visited.size() == 6);
  REQUIRE(dfs.num_visited() == 5); // All vertices except seed are counted by advance()
}

// =============================================================================
// Test 6: Graph Topologies
// =============================================================================

TEST_CASE("vertices_dfs - tree topology", "[dfs][vertices][topology]") {
  // Binary tree
  //       0
  //      / \
    //     1   2
  //    / \
    //   3   4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3, 4}, {}, {}, {}};

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited.size() == 5);
  REQUIRE(visited[0] == 0); // Start
                            // DFS goes deep first: 0 -> 1 -> 3, backtrack to 1 -> 4, backtrack to 0 -> 2
}

TEST_CASE("vertices_dfs - cycle topology", "[dfs][vertices][topology]") {
  // Ring: 0 -> 1 -> 2 -> 3 -> 0
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {3}, {0}};

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Should visit all 4 vertices exactly once
  REQUIRE(visited.size() == 4);
  std::set<int> visited_set(visited.begin(), visited.end());
  REQUIRE(visited_set == std::set<int>{0, 1, 2, 3});
}

TEST_CASE("vertices_dfs - DAG topology", "[dfs][vertices][topology]") {
  // Diamond DAG
  //     0
  //    / \
    //   1   2
  //    \ /
  //     3
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Vertex 3 visited only once even though reachable from both 1 and 2
  REQUIRE(visited.size() == 4);
  REQUIRE(std::count(visited.begin(), visited.end(), 3) == 1);
}

TEST_CASE("vertices_dfs - disconnected graph", "[dfs][vertices][topology]") {
  // Two components: 0-1-2 and 3-4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}, {4}, {}};

  SECTION("DFS from component 1") {
    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
      visited.push_back(static_cast<int>(vertex_id(g, v)));
    }

    // Only vertices 0, 1, 2 reachable from 0
    REQUIRE(visited.size() == 3);
    std::set<int> visited_set(visited.begin(), visited.end());
    REQUIRE(visited_set == std::set<int>{0, 1, 2});
  }

  SECTION("DFS from component 2") {
    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 3)) {
      visited.push_back(static_cast<int>(vertex_id(g, v)));
    }

    // Only vertices 3, 4 reachable from 3
    REQUIRE(visited.size() == 2);
    std::set<int> visited_set(visited.begin(), visited.end());
    REQUIRE(visited_set == std::set<int>{3, 4});
  }
}

// =============================================================================
// Test 7: Empty Graph and Single Vertex
// =============================================================================

TEST_CASE("vertices_dfs - single vertex graph", "[dfs][vertices][edge_cases]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {} // Single vertex with no edges
  };

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited == std::vector<int>{0});
}

TEST_CASE("vertices_dfs - vertex with no outgoing edges", "[dfs][vertices][edge_cases]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, {}, {0} // Has incoming but no reachable from it
  };

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 1)) { // Start from leaf
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited == std::vector<int>{1});
}

// =============================================================================
// Test 8: search_view Concept
// =============================================================================

TEST_CASE("vertices_dfs - search_view concept", "[dfs][vertices][concepts]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {}};

  auto dfs = vertices_dfs(g, 0);

  STATIC_REQUIRE(search_view<decltype(dfs)>);

  // Verify accessors exist and return correct types
  REQUIRE(dfs.cancel() == cancel_search::continue_search);
  REQUIRE(dfs.depth() >= 0);
  REQUIRE(dfs.num_visited() >= 0);
}

// =============================================================================
// Test 9: Range Concepts
// =============================================================================

TEST_CASE("vertices_dfs - range concepts", "[dfs][vertices][concepts]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {}};

  auto dfs = vertices_dfs(g, 0);

  STATIC_REQUIRE(std::ranges::input_range<decltype(dfs)>);
  STATIC_REQUIRE(std::ranges::view<decltype(dfs)>);

  // DFS is input range (not forward) due to shared state
  // Cannot iterate twice independently
}

// =============================================================================
// Test 10: vertex_info Type Verification
// =============================================================================

TEST_CASE("vertices_dfs - vertex_info type verification", "[dfs][vertices][types]") {
  using Graph      = std::vector<std::vector<int>>;
  using VertexType = vertex_t<Graph>;

  SECTION("no value function") {
    using ViewType = vertices_dfs_view<Graph, void, std::allocator<bool>>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<InfoType, vertex_info<void, VertexType, void>>);
  }

  SECTION("with value function") {
    using VVF      = int (*)(const Graph&, VertexType);
    using ViewType = vertices_dfs_view<Graph, VVF, std::allocator<bool>>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<InfoType, vertex_info<void, VertexType, int>>);
  }
}

// =============================================================================
// Test 11: Deque-based Graph
// =============================================================================

TEST_CASE("vertices_dfs - deque-based graph", "[dfs][vertices][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1, 2}, {}, {}};

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited.size() == 3);
}

// =============================================================================
// Test 12: Weighted Graph
// =============================================================================

TEST_CASE("vertices_dfs - weighted graph", "[dfs][vertices][weighted]") {
  using Graph = std::vector<std::vector<std::pair<int, double>>>;
  Graph g     = {{{1, 1.5}, {2, 2.5}}, {{2, 3.5}}, {}};

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited.size() == 3);
  std::set<int> visited_set(visited.begin(), visited.end());
  REQUIRE(visited_set == std::set<int>{0, 1, 2});
}

// =============================================================================
// Test 13: Large Graph Performance
// =============================================================================

TEST_CASE("vertices_dfs - large linear graph", "[dfs][vertices][performance]") {
  // Linear chain: 0 -> 1 -> 2 -> ... -> 999
  using Graph = std::vector<std::vector<int>>;
  Graph g(1000);
  for (int i = 0; i < 999; ++i) {
    g[i].push_back(i + 1);
  }

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited.size() == 1000);

  // Verify order: should be 0, 1, 2, ..., 999
  for (int i = 0; i < 1000; ++i) {
    REQUIRE(visited[i] == i);
  }
}

// =============================================================================
// Test 14: DFS Pre-order Property
// =============================================================================

TEST_CASE("vertices_dfs - pre-order property", "[dfs][vertices][order]") {
  // Verify parent is visited before children
  //     0
  //    / \
    //   1   2
  //  /|   |\
    // 3 4   5 6
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0
        {3, 4}, // 1
        {5, 6}, // 2
        {},     // 3
        {},     // 4
        {},     // 5
        {}      // 6
  };

  std::vector<int> visited;
  for (auto [v] : vertices_dfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Find positions
  auto pos = [&visited](int id) { return std::find(visited.begin(), visited.end(), id) - visited.begin(); };

  // Parent visited before children
  REQUIRE(pos(0) < pos(1));
  REQUIRE(pos(0) < pos(2));
  REQUIRE(pos(1) < pos(3));
  REQUIRE(pos(1) < pos(4));
  REQUIRE(pos(2) < pos(5));
  REQUIRE(pos(2) < pos(6));
}

// =============================================================================
// edges_dfs Tests
// =============================================================================

// =============================================================================
// Test 15: edges_dfs Basic Traversal
// =============================================================================

TEST_CASE("edges_dfs - basic traversal order", "[dfs][edges]") {
  //     0
  //    / \
    //   1   2
  //  / \
    // 3   4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {},     // 2 (leaf)
        {},     // 3 (leaf)
        {}      // 4 (leaf)
  };

  SECTION("edges from vertex 0") {
    std::vector<std::pair<int, int>> edges_visited;

    for (auto [e] : edges_dfs(g, 0)) {
      auto src = static_cast<int>(source_id(g, e));
      auto tgt = static_cast<int>(target_id(g, e));
      edges_visited.emplace_back(src, tgt);
    }

    // DFS tree edges: 0->1, 1->3, 1->4, 0->2
    // Note: Seed vertex 0 has no incoming edge, so we get 4 tree edges
    REQUIRE(edges_visited.size() == 4);

    // All edges are tree edges to unvisited vertices
    std::set<int> targets;
    for (auto [src, tgt] : edges_visited) {
      targets.insert(tgt);
    }
    REQUIRE(targets == std::set<int>{1, 2, 3, 4});
  }

  SECTION("edges from leaf vertex") {
    std::vector<std::pair<int, int>> edges_visited;

    for (auto [e] : edges_dfs(g, 3)) {
      auto src = static_cast<int>(source_id(g, e));
      auto tgt = static_cast<int>(target_id(g, e));
      edges_visited.emplace_back(src, tgt);
    }

    // Leaf vertex has no outgoing edges
    REQUIRE(edges_visited.empty());
  }
}

// =============================================================================
// Test 16: edges_dfs Structured Bindings
// =============================================================================

TEST_CASE("edges_dfs - structured bindings", "[dfs][edges][bindings]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  SECTION("structured binding [e]") {
    int count = 0;

    for (auto [e] : edges_dfs(g, 0)) {
      // Access edge through structured binding
      [[maybe_unused]] auto tgt = target_id(g, e);
      count++;
    }

    // Two edges: 0->1, 1->2 (note: 0->2 is skipped because 2 already visited via 1)
    REQUIRE(count == 2);
  }

  SECTION("structured binding [e, val] with value function") {
    auto dfs = edges_dfs(g, 0, [](const auto& g, auto e) { return target_id(g, e) * 10; });

    std::vector<std::pair<int, int>> results;
    for (auto [e, val] : dfs) {
      results.emplace_back(static_cast<int>(target_id(g, e)), val);
    }

    REQUIRE(results.size() == 2);
    for (auto& [tgt, val] : results) {
      REQUIRE(val == tgt * 10);
    }
  }
}

// =============================================================================
// Test 17: edges_dfs Value Function Types
// =============================================================================

TEST_CASE("edges_dfs - value function types", "[dfs][edges][evf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};

  SECTION("returning int") {
    auto dfs = edges_dfs(g, 0, [](const auto& g, auto e) { return static_cast<int>(target_id(g, e)); });

    int sum = 0;
    for (auto [e, val] : dfs) {
      sum += val;
    }
    REQUIRE(sum == 1 + 2);
  }

  SECTION("returning string") {
    auto dfs = edges_dfs(g, 0, [](const auto& g, auto e) {
      return "e" + std::to_string(source_id(g, e)) + "_" + std::to_string(target_id(g, e));
    });

    std::vector<std::string> names;
    for (auto [e, name] : dfs) {
      names.push_back(name);
    }

    REQUIRE(names.size() == 2);
    // Order depends on DFS, but should have both edges
    std::set<std::string> name_set(names.begin(), names.end());
    REQUIRE(name_set == std::set<std::string>{"e0_1", "e0_2"});
  }

  SECTION("capturing lambda") {
    int  multiplier = 5;
    auto dfs        = edges_dfs(
          g, 0, [multiplier](const auto& g, auto e) { return static_cast<int>(target_id(g, e)) * multiplier; });

    std::vector<int> values;
    for (auto [e, val] : dfs) {
      values.push_back(val);
    }

    REQUIRE(values.size() == 2);
    std::set<int> value_set(values.begin(), values.end());
    REQUIRE(value_set == std::set<int>{5, 10}); // 1*5, 2*5
  }
}

// =============================================================================
// Test 18: edges_dfs Cycle Handling
// =============================================================================

TEST_CASE("edges_dfs - cycle handling", "[dfs][edges][visited]") {
  // Graph with cycle: 0 -> 1 -> 2 -> 0
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {0}  // 2 -> 0 (back edge)
  };

  std::vector<std::pair<int, int>> edges_visited;

  for (auto [e] : edges_dfs(g, 0)) {
    auto src = static_cast<int>(source_id(g, e));
    auto tgt = static_cast<int>(target_id(g, e));
    edges_visited.emplace_back(src, tgt);
  }

  // Only tree edges: 0->1, 1->2
  // The back edge 2->0 is NOT yielded because vertex 0 is already visited
  REQUIRE(edges_visited.size() == 2);
  REQUIRE(edges_visited[0] == std::pair<int, int>{0, 1});
  REQUIRE(edges_visited[1] == std::pair<int, int>{1, 2});
}

// =============================================================================
// Test 19: edges_dfs Diamond DAG
// =============================================================================

TEST_CASE("edges_dfs - diamond DAG", "[dfs][edges][topology]") {
  // Diamond DAG
  //     0
  //    / \
    //   1   2
  //    \ /
  //     3
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  std::vector<std::pair<int, int>> edges_visited;
  for (auto [e] : edges_dfs(g, 0)) {
    auto src = static_cast<int>(source_id(g, e));
    auto tgt = static_cast<int>(target_id(g, e));
    edges_visited.emplace_back(src, tgt);
  }

  // Tree edges: 0->1, 1->3, 0->2 (2->3 skipped because 3 already visited)
  REQUIRE(edges_visited.size() == 3);

  // Verify all tree edges reach unique targets
  std::set<int> targets;
  for (auto [src, tgt] : edges_visited) {
    targets.insert(tgt);
  }
  REQUIRE(targets == std::set<int>{1, 2, 3});
}

// =============================================================================
// Test 20: edges_dfs Disconnected Graph
// =============================================================================

TEST_CASE("edges_dfs - disconnected graph", "[dfs][edges][topology]") {
  // Two components: 0-1-2 and 3-4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}, {4}, {}};

  SECTION("edges from component 1") {
    std::vector<std::pair<int, int>> edges_visited;
    for (auto [e] : edges_dfs(g, 0)) {
      auto src = static_cast<int>(source_id(g, e));
      auto tgt = static_cast<int>(target_id(g, e));
      edges_visited.emplace_back(src, tgt);
    }

    // Only edges in component 1
    REQUIRE(edges_visited.size() == 2);
    REQUIRE(edges_visited[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges_visited[1] == std::pair<int, int>{1, 2});
  }

  SECTION("edges from component 2") {
    std::vector<std::pair<int, int>> edges_visited;
    for (auto [e] : edges_dfs(g, 3)) {
      auto src = static_cast<int>(source_id(g, e));
      auto tgt = static_cast<int>(target_id(g, e));
      edges_visited.emplace_back(src, tgt);
    }

    // Only edge in component 2
    REQUIRE(edges_visited.size() == 1);
    REQUIRE(edges_visited[0] == std::pair<int, int>{3, 4});
  }
}

// =============================================================================
// Test 21: edges_dfs Single Vertex
// =============================================================================

TEST_CASE("edges_dfs - single vertex graph", "[dfs][edges][edge_cases]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {} // Single vertex with no edges
  };

  std::vector<int> edge_count;
  for (auto [e] : edges_dfs(g, 0)) {
    edge_count.push_back(1);
  }

  REQUIRE(edge_count.empty());
}

// =============================================================================
// Test 22: edges_dfs search_view Concept
// =============================================================================

TEST_CASE("edges_dfs - search_view concept", "[dfs][edges][concepts]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {}};

  auto dfs = edges_dfs(g, 0);

  STATIC_REQUIRE(search_view<decltype(dfs)>);

  // Verify accessors exist and return correct types
  REQUIRE(dfs.cancel() == cancel_search::continue_search);
  REQUIRE(dfs.depth() >= 0);
  REQUIRE(dfs.num_visited() >= 0);
}

// =============================================================================
// Test 23: edges_dfs Range Concepts
// =============================================================================

TEST_CASE("edges_dfs - range concepts", "[dfs][edges][concepts]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {}};

  auto dfs = edges_dfs(g, 0);

  STATIC_REQUIRE(std::ranges::input_range<decltype(dfs)>);
  STATIC_REQUIRE(std::ranges::view<decltype(dfs)>);
}

// =============================================================================
// Test 24: edges_dfs edge_info Type Verification
// =============================================================================

TEST_CASE("edges_dfs - edge_info type verification", "[dfs][edges][types]") {
  using Graph    = std::vector<std::vector<int>>;
  using EdgeType = edge_t<Graph>;

  SECTION("no value function") {
    using ViewType = edges_dfs_view<Graph, void, std::allocator<bool>>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<InfoType, edge_info<void, false, EdgeType, void>>);
  }

  SECTION("with value function") {
    using EVF      = int (*)(const Graph&, EdgeType);
    using ViewType = edges_dfs_view<Graph, EVF, std::allocator<bool>>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<InfoType, edge_info<void, false, EdgeType, int>>);
  }
}

// =============================================================================
// Test 25: edges_dfs Weighted Graph
// =============================================================================

TEST_CASE("edges_dfs - weighted graph", "[dfs][edges][weighted]") {
  using Graph = std::vector<std::vector<std::pair<int, double>>>;
  Graph g     = {{{1, 1.5}, {2, 2.5}}, {{2, 3.5}}, {}};

  std::vector<double> weights;
  for (auto [e] : edges_dfs(g, 0)) {
    // Edge value is the weight in the pair
    weights.push_back(edge_value(g, e));
  }

  // Two tree edges: 0->1 (1.5), 1->2 (3.5)
  // Note: 0->2 is skipped because 2 is already visited via 1
  REQUIRE(weights.size() == 2);
  REQUIRE(weights[0] == 1.5);
  REQUIRE(weights[1] == 3.5);
}

// =============================================================================
// Test 26: edges_dfs Large Graph
// =============================================================================

TEST_CASE("edges_dfs - large linear graph", "[dfs][edges][performance]") {
  // Linear chain: 0 -> 1 -> 2 -> ... -> 999
  using Graph = std::vector<std::vector<int>>;
  Graph g(1000);
  for (int i = 0; i < 999; ++i) {
    g[i].push_back(i + 1);
  }

  int edge_count = 0;
  for (auto [e] : edges_dfs(g, 0)) {
    ++edge_count;
  }

  // 999 tree edges
  REQUIRE(edge_count == 999);
}

// =============================================================================
// Test 27: edges_dfs Deque-based Graph
// =============================================================================

TEST_CASE("edges_dfs - deque-based graph", "[dfs][edges][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1, 2}, {}, {}};

  std::vector<int> targets;
  for (auto [e] : edges_dfs(g, 0)) {
    targets.push_back(static_cast<int>(target_id(g, e)));
  }

  REQUIRE(targets.size() == 2);
  std::set<int> target_set(targets.begin(), targets.end());
  REQUIRE(target_set == std::set<int>{1, 2});
}

// =============================================================================
// Test 28: edges_dfs Depth and Size Accessors
// =============================================================================

TEST_CASE("edges_dfs - depth and size accessors", "[dfs][edges][accessors]") {
  //     0
  //    /|\
    //   1 2 3
  //   |
  //   4
  //   |
  //   5
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2, 3}, // 0 -> 1, 2, 3
        {4},       // 1 -> 4
        {},        // 2 (leaf)
        {},        // 3 (leaf)
        {5},       // 4 -> 5
        {}         // 5 (leaf)
  };

  auto dfs = edges_dfs(g, 0);

  // Before iteration
  REQUIRE(dfs.depth() == 1);       // seed is on stack
  REQUIRE(dfs.num_visited() == 0); // no edges counted yet

  // Iterate
  int edge_count = 0;
  for ([[maybe_unused]] auto [e] : dfs) {
    edge_count++;
  }

  // 5 tree edges (one for each non-seed vertex)
  REQUIRE(edge_count == 5);
  REQUIRE(dfs.num_visited() == 5);
}

// =============================================================================
// DFS Cancel Functionality Tests (Step 3.3)
// =============================================================================

// =============================================================================
// Test 29: vertices_dfs cancel_all
// =============================================================================

TEST_CASE("vertices_dfs - cancel_all stops traversal", "[dfs][vertices][cancel]") {
  //     0
  //    / \
    //   1   2
  //  / \
    // 3   4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {},     // 2 (leaf)
        {},     // 3 (leaf)
        {}      // 4 (leaf)
  };

  std::vector<int> visited;
  auto             dfs = vertices_dfs(g, 0);

  for (auto [v] : dfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
    if (vertex_id(g, v) == 1) {
      dfs.cancel(cancel_search::cancel_all);
    }
  }

  // Should stop immediately after setting cancel_all
  // Visited: 0, 1 (cancel_all set after visiting 1)
  REQUIRE(visited.size() == 2);
  REQUIRE(visited[0] == 0);
  REQUIRE(visited[1] == 1);

  // Cancel state should be cancel_all
  REQUIRE(dfs.cancel() == cancel_search::cancel_all);
}

// =============================================================================
// Test 30: vertices_dfs cancel_branch skips subtree
// =============================================================================

TEST_CASE("vertices_dfs - cancel_branch skips subtree", "[dfs][vertices][cancel]") {
  //     0
  //    / \
    //   1   2
  //  / \
    // 3   4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {},     // 2 (leaf)
        {},     // 3 (leaf)
        {}      // 4 (leaf)
  };

  std::vector<int> visited;
  auto             dfs = vertices_dfs(g, 0);

  for (auto [v] : dfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
    if (vertex_id(g, v) == 1) {
      // Skip vertex 1's subtree (3, 4)
      dfs.cancel(cancel_search::cancel_branch);
    }
  }

  // Should visit 0, 1, then skip 3 and 4 (subtree of 1), then visit 2
  // Visited: 0, 1, 2
  REQUIRE(visited.size() == 3);
  std::set<int> visited_set(visited.begin(), visited.end());
  REQUIRE(visited_set == std::set<int>{0, 1, 2});

  // 3 and 4 should NOT be visited
  REQUIRE(std::find(visited.begin(), visited.end(), 3) == visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 4) == visited.end());

  // Cancel state should be reset to continue_search
  REQUIRE(dfs.cancel() == cancel_search::continue_search);
}

// =============================================================================
// Test 31: vertices_dfs continue_search normal behavior
// =============================================================================

TEST_CASE("vertices_dfs - continue_search normal behavior", "[dfs][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};

  auto dfs = vertices_dfs(g, 0);

  // Default is continue_search
  REQUIRE(dfs.cancel() == cancel_search::continue_search);

  std::vector<int> visited;
  for (auto [v] : dfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // All vertices visited
  REQUIRE(visited.size() == 3);
}

// =============================================================================
// Test 32: vertices_dfs cancel state propagates through iterator copies
// =============================================================================

TEST_CASE("vertices_dfs - cancel state propagates through shared state", "[dfs][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {}};

  auto dfs = vertices_dfs(g, 0);
  auto it  = dfs.begin();

  // Advance a bit
  ++it; // now at vertex 1

  // Cancel via view
  dfs.cancel(cancel_search::cancel_all);

  // Iterator should also see the cancel (shared state)
  ++it; // should stop

  REQUIRE(it.at_end());
}

// =============================================================================
// Test 33: edges_dfs cancel_all stops traversal
// =============================================================================

TEST_CASE("edges_dfs - cancel_all stops traversal", "[dfs][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3, 4}, {}, {}, {}};

  std::vector<std::pair<int, int>> edges_visited;
  auto                             dfs = edges_dfs(g, 0);

  for (auto [e] : dfs) {
    auto src = static_cast<int>(source_id(g, e));
    auto tgt = static_cast<int>(target_id(g, e));
    edges_visited.emplace_back(src, tgt);
    if (tgt == 3) {
      dfs.cancel(cancel_search::cancel_all);
    }
  }

  // Should stop after edge to 3
  // Edges: 0->1, 1->3 (cancel_all set after this)
  REQUIRE(edges_visited.size() == 2);
  REQUIRE(dfs.cancel() == cancel_search::cancel_all);
}

// =============================================================================
// Test 34: edges_dfs cancel_branch skips subtree
// =============================================================================

TEST_CASE("edges_dfs - cancel_branch skips subtree", "[dfs][edges][cancel]") {
  //     0
  //    / \
    //   1   2
  //  / \
    // 3   4
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3, 4}, {}, {}, {}};

  std::vector<std::pair<int, int>> edges_visited;
  auto                             dfs = edges_dfs(g, 0);

  for (auto [e] : dfs) {
    auto src = static_cast<int>(source_id(g, e));
    auto tgt = static_cast<int>(target_id(g, e));
    edges_visited.emplace_back(src, tgt);
    if (tgt == 1) {
      // Skip subtree rooted at 1 (edges 1->3, 1->4)
      dfs.cancel(cancel_search::cancel_branch);
    }
  }

  // Should see edge 0->1 (then skip subtree), then edge 0->2
  // Edges: 0->1, 0->2
  REQUIRE(edges_visited.size() == 2);

  std::set<int> targets;
  for (auto [src, tgt] : edges_visited) {
    targets.insert(tgt);
  }
  REQUIRE(targets == std::set<int>{1, 2});

  // 3 and 4 should NOT be reached
  REQUIRE(targets.find(3) == targets.end());
  REQUIRE(targets.find(4) == targets.end());
}

// =============================================================================
// Test 35: cancel_branch at root has no subtree to skip
// =============================================================================

TEST_CASE("vertices_dfs - cancel_branch at seed vertex", "[dfs][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};

  std::vector<int> visited;
  auto             dfs = vertices_dfs(g, 0);

  for (auto [v] : dfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
    if (vertex_id(g, v) == 0) {
      // Cancel at root - should skip entire traversal
      dfs.cancel(cancel_search::cancel_branch);
    }
  }

  // Only seed vertex visited, subtree (1, 2) skipped
  REQUIRE(visited.size() == 1);
  REQUIRE(visited[0] == 0);
}

// =============================================================================
// Test 36: Multiple cancel_branch calls
// =============================================================================

TEST_CASE("vertices_dfs - multiple cancel_branch calls", "[dfs][vertices][cancel]") {
  //       0
  //    /  |  \
    //   1   2   3
  //  /|   |   |\
    // 4 5   6   7 8
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2, 3}, // 0 -> 1, 2, 3
        {4, 5},    // 1 -> 4, 5
        {6},       // 2 -> 6
        {7, 8},    // 3 -> 7, 8
        {},        // 4
        {},        // 5
        {},        // 6
        {},        // 7
        {}         // 8
  };

  std::vector<int> visited;
  auto             dfs = vertices_dfs(g, 0);

  for (auto [v] : dfs) {
    int vid = static_cast<int>(vertex_id(g, v));
    visited.push_back(vid);
    // Skip subtrees of vertices 1 and 3
    if (vid == 1 || vid == 3) {
      dfs.cancel(cancel_search::cancel_branch);
    }
  }

  // Should visit: 0, 1 (skip 4,5), 2, 6, 3 (skip 7,8)
  // Note: exact order depends on DFS exploration
  std::set<int> visited_set(visited.begin(), visited.end());
  REQUIRE(visited_set == std::set<int>{0, 1, 2, 3, 6});

  // Vertices 4, 5, 7, 8 should NOT be visited
  REQUIRE(visited_set.find(4) == visited_set.end());
  REQUIRE(visited_set.find(5) == visited_set.end());
  REQUIRE(visited_set.find(7) == visited_set.end());
  REQUIRE(visited_set.find(8) == visited_set.end());
}

// =============================================================================
// Test 37: cancel with value function
// =============================================================================

TEST_CASE("vertices_dfs - cancel with value function", "[dfs][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3, 4}, {}, {}, {}};

  std::vector<std::pair<int, int>> results;
  auto dfs = vertices_dfs(g, 0, [](const auto& g, auto v) { return static_cast<int>(vertex_id(g, v)) * 10; });

  for (auto [v, val] : dfs) {
    int vid = static_cast<int>(vertex_id(g, v));
    results.emplace_back(vid, val);
    if (vid == 1) {
      dfs.cancel(cancel_search::cancel_branch);
    }
  }

  // Should visit 0, 1 (skip subtree), 2
  REQUIRE(results.size() == 3);

  // Verify value function was called correctly
  for (auto& [id, val] : results) {
    REQUIRE(val == id * 10);
  }
}
