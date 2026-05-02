#include <catch2/catch_test_macros.hpp>
#include <graph/views/bfs.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/uol_graph_traits.hpp>
#include <vector>
#include <algorithm>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;
using namespace graph::container;

TEST_CASE("vertices_bfs - basic traversal", "[bfs][vertices]") {
  // Create simple tree: 0 -> [1, 2], 1 -> [3, 4]
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {},     // 2 (leaf)
        {},     // 3 (leaf)
        {}      // 4 (leaf)
  };

  std::vector<int> visited;
  for (auto [v] : vertices_bfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited.size() == 5);
  REQUIRE(visited[0] == 0); // Root
  // Level 1: 1, 2 (order may vary)
  REQUIRE(std::find(visited.begin() + 1, visited.begin() + 3, 1) != visited.begin() + 3);
  REQUIRE(std::find(visited.begin() + 1, visited.begin() + 3, 2) != visited.begin() + 3);
  // Level 2: 3, 4 (order may vary)
  REQUIRE(std::find(visited.begin() + 3, visited.end(), 3) != visited.end());
  REQUIRE(std::find(visited.begin() + 3, visited.end(), 4) != visited.end());
}

TEST_CASE("vertices_bfs - level order", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Create tree with distinct levels
  Graph g = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {5},    // 2 -> 5
        {},     // 3 (leaf)
        {},     // 4 (leaf)
        {}      // 5 (leaf)
  };

  std::vector<int> visited;
  for (auto [v] : vertices_bfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Verify level-order traversal
  REQUIRE(visited.size() == 6);
  REQUIRE(visited[0] == 0); // Level 0
  // Level 1 (1 and 2 in some order)
  REQUIRE(((visited[1] == 1 && visited[2] == 2) || (visited[1] == 2 && visited[2] == 1)));
  // Level 2 (3, 4, 5 in some order, but all after level 1)
  std::set<int> level2(visited.begin() + 3, visited.end());
  REQUIRE(level2 == std::set<int>{3, 4, 5});
}

TEST_CASE("vertices_bfs - structured bindings", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // 0 -> 1
        {}   // 1 (leaf)
  };

  int count = 0;
  for (auto [v] : vertices_bfs(g, 0)) {
    REQUIRE(vertex_id(g, v) < static_cast<int>(g.size()));
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("vertices_bfs - with value function", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {},     // 1 (leaf)
        {}      // 2 (leaf)
  };

  auto value_fn = [](const auto& g, auto v) { return static_cast<int>(vertex_id(g, v)) * 10; };

  std::vector<int> values;
  for (auto [v, val] : vertices_bfs(g, 0, value_fn)) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 0);                                                  // 0 * 10
  REQUIRE(std::find(values.begin() + 1, values.end(), 10) != values.end()); // 1 * 10
  REQUIRE(std::find(values.begin() + 1, values.end(), 20) != values.end()); // 2 * 10
}

TEST_CASE("vertices_bfs - depth tracking", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Create chain: 0 -> 1 -> 2 -> 3
  Graph g = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {3}, // 2 -> 3
        {}   // 3 (leaf)
  };

  auto bfs = vertices_bfs(g, 0);

  for (auto [v] : bfs) {
    // Just iterate through
  }

  REQUIRE(bfs.depth() == 3);       // Path length 0->1->2->3
  REQUIRE(bfs.num_visited() == 3); // 3 vertices visited after seed
}

TEST_CASE("vertices_bfs - size tracking", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {},     // 1 (leaf)
        {}      // 2 (leaf)
  };

  auto bfs = vertices_bfs(g, 0);

  int iterations = 0;
  for (auto [v] : bfs) {
    ++iterations;
  }

  REQUIRE(iterations == 3);
  REQUIRE(bfs.num_visited() == 2); // 2 vertices visited after seed
}

TEST_CASE("vertices_bfs - single vertex", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {} // 0 (leaf)
  };

  std::vector<int> visited;
  for (auto [v] : vertices_bfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(visited.size() == 1);
  REQUIRE(visited[0] == 0);
}

TEST_CASE("vertices_bfs - cycle handling", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Create cycle: 0 -> 1 -> 2 -> 0
  Graph g = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {0}  // 2 -> 0 (cycle)
  };

  std::vector<int> visited;
  for (auto [v] : vertices_bfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Should visit each vertex exactly once
  REQUIRE(visited.size() == 3);
  REQUIRE(std::count(visited.begin(), visited.end(), 0) == 1);
  REQUIRE(std::count(visited.begin(), visited.end(), 1) == 1);
  REQUIRE(std::count(visited.begin(), visited.end(), 2) == 1);
}

TEST_CASE("vertices_bfs - disconnected components", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Create two disconnected components
  Graph g = {
        {1}, // 0 -> 1 (Component 1)
        {},  // 1 (leaf)
        {3}, // 2 -> 3 (Component 2)
        {}   // 3 (leaf)
  };

  std::vector<int> visited;
  for (auto [v] : vertices_bfs(g, 0)) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Should only visit component containing vertex 0
  REQUIRE(visited.size() == 2);
  REQUIRE(std::find(visited.begin(), visited.end(), 0) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 1) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 2) == visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 3) == visited.end());
}

TEST_CASE("vertices_bfs - empty iteration", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {} // 0 (leaf)
  };

  auto bfs = vertices_bfs(g, 0);
  auto it  = bfs.begin();
  auto end = bfs.end();

  REQUIRE(it != end); // Has seed
  ++it;
  REQUIRE(it == end); // No more vertices
}

TEST_CASE("vertices_bfs - cancel_all", "[bfs][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3},    // 1 -> 3
        {},     // 2 (leaf)
        {}      // 3 (leaf)
  };

  std::vector<int> visited;
  auto             bfs = vertices_bfs(g, 0);

  for (auto [v] : bfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
    if (vertex_id(g, v) == 1) {
      bfs.cancel(cancel_search::cancel_all);
    }
  }

  // Should stop after processing vertex 1
  REQUIRE(visited.size() <= 3); // 0, 1 or 2 (depending on when cancel takes effect)
  REQUIRE(std::find(visited.begin(), visited.end(), 3) == visited.end()); // Should not reach 3
}

TEST_CASE("vertices_bfs - cancel_branch", "[bfs][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  // Create tree: 0 -> [1, 2], 1 -> [3, 4], 2 -> [5]
  Graph g = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {5},    // 2 -> 5
        {},     // 3 (leaf)
        {},     // 4 (leaf)
        {}      // 5 (leaf)
  };

  std::vector<int> visited;
  auto             bfs = vertices_bfs(g, 0);

  for (auto [v] : bfs) {
    visited.push_back(static_cast<int>(vertex_id(g, v)));
    if (vertex_id(g, v) == 1) {
      bfs.cancel(cancel_search::cancel_branch); // Skip exploring children of vertex 1
    }
  }

  // Should visit: 0, 1, 2, 5 but NOT 3, 4 (children of 1)
  REQUIRE(std::find(visited.begin(), visited.end(), 0) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 1) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 2) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 5) != visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 3) == visited.end());
  REQUIRE(std::find(visited.begin(), visited.end(), 4) == visited.end());
}

TEST_CASE("vertices_bfs - large tree", "[bfs][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Create binary tree of depth 4
  Graph g(15);

  // Connect as binary tree: node i has children 2i+1 and 2i+2
  for (int i = 0; i < 7; ++i) {
    g[static_cast<std::size_t>(i)].push_back(2 * i + 1);
    g[static_cast<std::size_t>(i)].push_back(2 * i + 2);
  }

  int count = 0;
  for (auto [v] : vertices_bfs(g, 0)) {
    ++count;
  }

  REQUIRE(count == 15);
}

//=============================================================================
// edges_bfs tests
//=============================================================================

TEST_CASE("edges_bfs - basic traversal", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {},     // 2 (leaf)
        {},     // 3 (leaf)
        {}      // 4 (leaf)
  };

  std::vector<int> targets;
  for (auto [edge] : edges_bfs(g, 0)) {
    auto target_v = target(g, edge);
    targets.push_back(static_cast<int>(vertex_id(g, target_v)));
  }

  // Should visit edges in BFS order: (0->1), (0->2), (1->3), (1->4)
  REQUIRE(targets.size() == 4);
  REQUIRE(targets[0] == 1); // First level
  REQUIRE(targets[1] == 2); // First level
  REQUIRE(targets[2] == 3); // Second level
  REQUIRE(targets[3] == 4); // Second level
}

TEST_CASE("edges_bfs - structured bindings", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {}   // 2 (leaf)
  };

  int count = 0;
  for (auto [edge] : edges_bfs(g, 0)) {
    auto target_v = target(g, edge);
    REQUIRE(vertex_id(g, target_v) < g.size());
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("edges_bfs - with value function", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {},     // 1 (leaf)
        {}      // 2 (leaf)
  };

  auto value_fn = [](const auto& g, auto edge) {
    auto target_v = target(g, edge);
    return static_cast<int>(vertex_id(g, target_v)) * 10;
  };

  std::vector<int> values;
  for (auto [edge, val] : edges_bfs(g, 0, value_fn)) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 2);
  REQUIRE(std::find(values.begin(), values.end(), 10) != values.end()); // edge to vertex 1
  REQUIRE(std::find(values.begin(), values.end(), 20) != values.end()); // edge to vertex 2
}

TEST_CASE("edges_bfs - single vertex (no edges)", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {} // 0 (no outgoing edges)
  };

  int count = 0;
  for (auto [edge] : edges_bfs(g, 0)) {
    ++count;
  }

  REQUIRE(count == 0); // No edges from isolated vertex
}

TEST_CASE("edges_bfs - cycle handling", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // Create cycle: 0 -> 1 -> 2 -> 0
  Graph g = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {0}  // 2 -> 0 (cycle back, should not visit this edge)
  };

  std::vector<int> targets;
  for (auto [edge] : edges_bfs(g, 0)) {
    auto target_v = target(g, edge);
    targets.push_back(static_cast<int>(vertex_id(g, target_v)));
  }

  // Should visit only tree edges: 0->1, 1->2 (not the back edge 2->0)
  REQUIRE(targets.size() == 2);
  REQUIRE(targets[0] == 1);
  REQUIRE(targets[1] == 2);
}

TEST_CASE("edges_bfs - disconnected components", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // Two components: 0-1 and 2-3
  Graph g = {
        {1}, // 0 -> 1 (Component 1)
        {},  // 1 (leaf)
        {3}, // 2 -> 3 (Component 2)
        {}   // 3 (leaf)
  };

  std::vector<int> targets;
  for (auto [edge] : edges_bfs(g, 0)) {
    auto target_v = target(g, edge);
    targets.push_back(static_cast<int>(vertex_id(g, target_v)));
  }

  // Should only visit edges in component containing vertex 0
  REQUIRE(targets.size() == 1);
  REQUIRE(targets[0] == 1);
}

TEST_CASE("edges_bfs - cancel_all", "[bfs][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3},    // 1 -> 3
        {},     // 2 (leaf)
        {}      // 3 (leaf)
  };

  std::vector<int> targets;
  auto             bfs = edges_bfs(g, 0);

  for (auto [edge] : bfs) {
    auto target_v  = target(g, edge);
    int  target_id = static_cast<int>(vertex_id(g, target_v));
    targets.push_back(target_id);
    if (target_id == 1) {
      bfs.cancel(cancel_search::cancel_all);
    }
  }

  // Should stop after visiting edge to vertex 1
  REQUIRE(targets.size() <= 2);                                           // May visit edge to 2 if already in queue
  REQUIRE(std::find(targets.begin(), targets.end(), 3) == targets.end()); // Should not reach 3
}

TEST_CASE("edges_bfs - cancel_branch", "[bfs][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  // Create tree: 0 -> [1, 2], 1 -> [3, 4], 2 -> [5]
  Graph g = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {5},    // 2 -> 5
        {},     // 3 (leaf)
        {},     // 4 (leaf)
        {}      // 5 (leaf)
  };

  std::vector<int> targets;
  auto             bfs = edges_bfs(g, 0);

  for (auto [edge] : bfs) {
    auto target_v  = target(g, edge);
    int  target_id = static_cast<int>(vertex_id(g, target_v));
    targets.push_back(target_id);
    if (target_id == 1) {
      bfs.cancel(cancel_search::cancel_branch); // Skip exploring children of vertex 1
    }
  }

  // Should visit: edges to 1, 2, 5 but NOT 3, 4 (children of 1)
  REQUIRE(std::find(targets.begin(), targets.end(), 1) != targets.end());
  REQUIRE(std::find(targets.begin(), targets.end(), 2) != targets.end());
  REQUIRE(std::find(targets.begin(), targets.end(), 5) != targets.end());
  REQUIRE(std::find(targets.begin(), targets.end(), 3) == targets.end());
  REQUIRE(std::find(targets.begin(), targets.end(), 4) == targets.end());
}

TEST_CASE("edges_bfs - large tree", "[bfs][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // Create binary tree of depth 4
  Graph g(15);

  // Connect as binary tree: node i has children 2i+1 and 2i+2
  for (int i = 0; i < 7; ++i) {
    g[static_cast<std::size_t>(i)].push_back(2 * i + 1);
    g[static_cast<std::size_t>(i)].push_back(2 * i + 2);
  }

  int count = 0;
  for (auto [edge] : edges_bfs(g, 0)) {
    ++count;
  }

  REQUIRE(count == 14); // 15 vertices means 14 edges in tree
}

//=============================================================================
// BFS depth/size accessor tests
//=============================================================================

TEST_CASE("vertices_bfs - depth increases by level", "[bfs][vertices][depth]") {
  using Graph = std::vector<std::vector<int>>;
  // Create multi-level tree: 0 -> [1, 2], 1 -> [3, 4], 2 -> [5, 6]
  Graph g = {
        {1, 2}, // 0 (level 0)
        {3, 4}, // 1 (level 1)
        {5, 6}, // 2 (level 1)
        {},     // 3 (level 2)
        {},     // 4 (level 2)
        {},     // 5 (level 2)
        {}      // 6 (level 2)
  };

  auto        bfs          = vertices_bfs(g, 0);
  std::size_t prev_depth   = 0;
  int         vertex_count = 0;

  for (auto [v] : bfs) {
    auto vid = static_cast<int>(vertex_id(g, v));
    ++vertex_count;

    // After processing seed, depth should be at least as high as before
    auto current_depth = bfs.depth();
    REQUIRE(current_depth >= prev_depth);

    // Depth should match expected level
    if (vid == 0)
      REQUIRE(current_depth == 0); // Seed at level 0
    else if (vid <= 2)
      REQUIRE(current_depth >= 1); // Level 1
    else
      REQUIRE(current_depth >= 2); // Level 2

    prev_depth = current_depth;
  }

  REQUIRE(bfs.depth() == 2);  // Final depth is 2
  REQUIRE(vertex_count == 7); // All 7 vertices visited
}

TEST_CASE("vertices_bfs - size accumulates correctly", "[bfs][vertices][size]") {
  using Graph = std::vector<std::vector<int>>;
  // Tree structure
  Graph g = {
        {1, 2, 3}, // 0 -> 1, 2, 3
        {4},       // 1 -> 4
        {5},       // 2 -> 5
        {},        // 3 (leaf)
        {},        // 4 (leaf)
        {}         // 5 (leaf)
  };

  auto        bfs          = vertices_bfs(g, 0);
  std::size_t prev_size    = 0;
  int         vertex_count = 0;

  for (auto [v] : bfs) {
    ++vertex_count;
    auto current_size = bfs.num_visited();

    // Size should never decrease (monotonically increasing)
    REQUIRE(current_size >= prev_size);

    prev_size = current_size;
  }

  // After iteration completes, size should be 5 (all non-seed vertices discovered)
  REQUIRE(bfs.num_visited() == 5); // 5 vertices visited after seed (excludes seed itself)
  REQUIRE(vertex_count == 6);      // Total vertices iterated including seed
}

TEST_CASE("vertices_bfs - depth on wide tree", "[bfs][vertices][depth]") {
  using Graph = std::vector<std::vector<int>>;
  // Wide tree: root has many children, all at same depth
  Graph g = {
        {1, 2, 3, 4, 5}, // 0 -> 1, 2, 3, 4, 5 (all at depth 1)
        {},              // 1 (leaf)
        {},              // 2 (leaf)
        {},              // 3 (leaf)
        {},              // 4 (leaf)
        {}               // 5 (leaf)
  };

  auto bfs = vertices_bfs(g, 0);

  for (auto [v] : bfs) {
    // Just iterate
  }

  REQUIRE(bfs.depth() == 1);       // Max depth is 1 (only two levels)
  REQUIRE(bfs.num_visited() == 5); // 5 children visited
}

TEST_CASE("vertices_bfs - depth on deep chain", "[bfs][vertices][depth]") {
  using Graph = std::vector<std::vector<int>>;
  // Long chain: 0 -> 1 -> 2 -> 3 -> 4 -> 5
  Graph g = {
        {1}, // 0
        {2}, // 1
        {3}, // 2
        {4}, // 3
        {5}, // 4
        {}   // 5 (leaf)
  };

  auto bfs = vertices_bfs(g, 0);

  for (auto [v] : bfs) {
    // Just iterate
  }

  REQUIRE(bfs.depth() == 5);       // Max depth is 5 (6 levels: 0 through 5)
  REQUIRE(bfs.num_visited() == 5); // 5 vertices after seed
}

TEST_CASE("vertices_bfs - size on disconnected graph", "[bfs][vertices][size]") {
  using Graph = std::vector<std::vector<int>>;
  // Two disconnected components
  Graph g = {
        {1, 2}, // 0 -> 1, 2 (Component 1)
        {},     // 1 (leaf)
        {},     // 2 (leaf)
        {4},    // 3 -> 4 (Component 2, unreachable from 0)
        {}      // 4 (leaf)
  };

  auto bfs = vertices_bfs(g, 0);

  for (auto [v] : bfs) {
    // Just iterate
  }

  REQUIRE(bfs.depth() == 1);       // Depth 1 within component
  REQUIRE(bfs.num_visited() == 2); // Only 2 vertices reachable from 0 (excludes seed)
}

TEST_CASE("edges_bfs - depth tracks edge depth", "[bfs][edges][depth]") {
  using Graph = std::vector<std::vector<int>>;
  // Tree: 0 -> [1, 2], 1 -> [3, 4]
  Graph g = {
        {1, 2}, // 0
        {3, 4}, // 1
        {},     // 2
        {},     // 3
        {}      // 4
  };

  auto bfs = edges_bfs(g, 0);

  for (auto [edge] : bfs) {
    // Just iterate
  }

  REQUIRE(bfs.depth() == 2);       // Deepest edge reaches depth 2
  REQUIRE(bfs.num_visited() == 4); // 4 edges total
}

TEST_CASE("edges_bfs - size counts edges", "[bfs][edges][size]") {
  using Graph = std::vector<std::vector<int>>;
  // Binary tree
  Graph g = {
        {1, 2}, // 0 -> 1, 2
        {3, 4}, // 1 -> 3, 4
        {5, 6}, // 2 -> 5, 6
        {},     // 3
        {},     // 4
        {},     // 5
        {}      // 6
  };

  auto bfs        = edges_bfs(g, 0);
  int  edge_count = 0;

  for (auto [edge] : bfs) {
    ++edge_count;
    // Size should equal edges discovered so far
    REQUIRE(bfs.num_visited() == static_cast<std::size_t>(edge_count));
  }

  REQUIRE(bfs.num_visited() == 6); // 6 edges total
  REQUIRE(edge_count == 6);
  REQUIRE(bfs.depth() == 2); // Max depth is 2
}

TEST_CASE("vertices_bfs - depth/size with value function", "[bfs][vertices][depth]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0
        {3},    // 1
        {},     // 2
        {}      // 3
  };

  auto value_fn = [](const auto& g, auto v) { return static_cast<int>(vertex_id(g, v)) * 10; };
  auto bfs      = vertices_bfs(g, 0, value_fn);

  for (auto [v, val] : bfs) {
    // Just iterate
  }

  REQUIRE(bfs.depth() == 2);       // Depth is 2 (0 -> 1 -> 3)
  REQUIRE(bfs.num_visited() == 3); // 3 vertices after seed
}

TEST_CASE("edges_bfs - depth/size with value function", "[bfs][edges][depth]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0
        {3},    // 1
        {},     // 2
        {}      // 3
  };

  auto value_fn = [](const auto& g, auto e) { return static_cast<int>(vertex_id(g, target(g, e))) * 10; };
  auto bfs      = edges_bfs(g, 0, value_fn);

  for (auto [e, val] : bfs) {
    // Just iterate
  }

  REQUIRE(bfs.depth() == 2);       // Depth is 2
  REQUIRE(bfs.num_visited() == 3); // 3 edges
}

// ===========================================================================
// Non-integral (string) vertex id helpers
// ===========================================================================

/// Directed graph with string vertex ids, no values.
using StrGraph = dynamic_adjacency_graph<uol_graph_traits<void, void, void, std::string, false>>;

/// Build a StrGraph from vertex names and an edge list.
static StrGraph make_graph(const std::vector<std::string>&                        vertex_names,
                           const std::vector<std::pair<std::string, std::string>>& edges) {
  using VD = copyable_vertex_t<std::string, void>;
  using ED = copyable_edge_t<std::string, void>;

  StrGraph g;
  g.load_vertices(vertex_names, [](const std::string& name) -> VD { return {name}; });
  g.load_edges(edges, [](const auto& e) -> ED { return {e.first, e.second}; });
  return g;
}

/// Collect vertex ids in traversal order.
template <typename Range>
static std::vector<std::string> collect_vertex_ids(StrGraph& g, Range&& rng) {
  std::vector<std::string> ids;
  for (auto [v] : rng)
    ids.push_back(std::string(vertex_id(g, v)));
  return ids;
}

/// Collect source/target id pairs in traversal order.
template <typename Range>
static std::vector<std::pair<std::string, std::string>> collect_edge_ids(StrGraph& g, Range&& rng) {
  std::vector<std::pair<std::string, std::string>> pairs;
  for (auto [uv] : rng)
    pairs.emplace_back(std::string(source_id(g, uv)), std::string(target_id(g, uv)));
  return pairs;
}

// ===========================================================================
// Non-integral vertex id tests
// ===========================================================================

TEST_CASE("vertices_bfs - string vertex ids", "[bfs][vertices][non_integral]") {
  //  a -> b -> d
  //  a -> c -> d
  auto g = make_graph({"a", "b", "c", "d"},
                      {{"a", "b"}, {"a", "c"}, {"b", "d"}, {"c", "d"}});

  SECTION("visits all reachable vertices exactly once") {
    auto ids = collect_vertex_ids(g, vertices_bfs(g, std::string{"a"}));
    REQUIRE(ids.size() == 4);
    std::unordered_set<std::string> id_set(ids.begin(), ids.end());
    REQUIRE(id_set == std::unordered_set<std::string>{"a", "b", "c", "d"});
  }

  SECTION("seed is visited first") {
    auto ids = collect_vertex_ids(g, vertices_bfs(g, std::string{"a"}));
    REQUIRE(ids[0] == "a");
  }

  SECTION("level-order: b and c before d") {
    auto ids   = collect_vertex_ids(g, vertices_bfs(g, std::string{"a"}));
    auto pos_b = std::find(ids.begin(), ids.end(), "b") - ids.begin();
    auto pos_c = std::find(ids.begin(), ids.end(), "c") - ids.begin();
    auto pos_d = std::find(ids.begin(), ids.end(), "d") - ids.begin();
    REQUIRE(pos_b < pos_d);
    REQUIRE(pos_c < pos_d);
  }

  SECTION("isolated leaf is only vertex when used as seed") {
    auto ids = collect_vertex_ids(g, vertices_bfs(g, std::string{"d"}));
    REQUIRE(ids == std::vector<std::string>{"d"});
  }

  SECTION("seed given as vertex descriptor") {
    auto seed = *find_vertex(g, std::string{"a"});
    auto ids  = collect_vertex_ids(g, vertices_bfs(g, seed));
    REQUIRE(ids.size() == 4);
    REQUIRE(ids[0] == "a");
  }
}

TEST_CASE("vertices_bfs - string ids with value function", "[bfs][vertices][non_integral]") {
  auto g = make_graph({"a", "b", "c"}, {{"a", "b"}, {"b", "c"}});

  auto vvf = [](const StrGraph& g2, vertex_t<StrGraph> v) {
    return std::string(vertex_id(g2, v));
  };

  std::vector<std::pair<std::string, std::string>> result;
  for (auto [v, val] : vertices_bfs(g, std::string{"a"}, vvf)) {
    result.emplace_back(std::string(vertex_id(g, v)), val);
  }

  REQUIRE(result.size() == 3);
  for (auto& [id, val] : result)
    REQUIRE(id == val);
}

TEST_CASE("edges_bfs - string vertex ids", "[bfs][edges][non_integral]") {
  //  a -> b -> c
  //       b -> d
  auto g = make_graph({"a", "b", "c", "d"},
                      {{"a", "b"}, {"b", "c"}, {"b", "d"}});

  SECTION("yields exactly the tree edges") {
    auto pairs = collect_edge_ids(g, edges_bfs(g, std::string{"a"}));
    REQUIRE(pairs.size() == 3);
    std::set<std::pair<std::string, std::string>> edge_set(pairs.begin(), pairs.end());
    REQUIRE(edge_set.count({"a", "b"}) == 1);
    REQUIRE(edge_set.count({"b", "c"}) == 1);
    REQUIRE(edge_set.count({"b", "d"}) == 1);
  }

  SECTION("first tree edge has seed as source") {
    auto pairs = collect_edge_ids(g, edges_bfs(g, std::string{"a"}));
    REQUIRE(pairs[0].first == "a");
    REQUIRE(pairs[0].second == "b");
  }

  SECTION("seed given as descriptor") {
    auto seed  = *find_vertex(g, std::string{"a"});
    auto pairs = collect_edge_ids(g, edges_bfs(g, seed));
    REQUIRE(pairs.size() == 3);
  }

  SECTION("no edges from isolated seed") {
    auto pairs = collect_edge_ids(g, edges_bfs(g, std::string{"c"}));
    REQUIRE(pairs.empty());
  }
}

TEST_CASE("edges_bfs - string ids with value function", "[bfs][edges][non_integral]") {
  auto g = make_graph({"a", "b", "c"}, {{"a", "b"}, {"b", "c"}});

  auto evf = [](const StrGraph& g2, auto uv) {
    return std::string(target_id(g2, uv));
  };

  std::vector<std::string> targets;
  for (auto [uv, val] : edges_bfs(g, std::string{"a"}, evf)) {
    targets.push_back(std::string(target_id(g, uv)));
    REQUIRE(val == targets.back());
  }

  REQUIRE(targets.size() == 2);
}

TEST_CASE("vertices_bfs cancel_branch - string ids", "[bfs][cancel][non_integral]") {
  //  a -> b -> c
  //  a -> d
  auto g = make_graph({"a", "b", "c", "d"},
                      {{"a", "b"}, {"b", "c"}, {"a", "d"}});

  SECTION("cancel_branch at b prevents visiting c") {
    std::vector<std::string> ids;
    auto bfs_view = vertices_bfs(g, std::string{"a"});
    for (auto [v] : bfs_view) {
      auto id = std::string(vertex_id(g, v));
      ids.push_back(id);
      if (id == "b")
        bfs_view.cancel(cancel_search::cancel_branch);
    }
    REQUIRE(std::find(ids.begin(), ids.end(), "c") == ids.end());
    REQUIRE(std::find(ids.begin(), ids.end(), "d") != ids.end());
  }

  SECTION("cancel_all stops traversal immediately") {
    std::vector<std::string> ids;
    auto bfs_view = vertices_bfs(g, std::string{"a"});
    for (auto [v] : bfs_view) {
      auto id = std::string(vertex_id(g, v));
      ids.push_back(id);
      if (id == "a")
        bfs_view.cancel(cancel_search::cancel_all);
    }
    REQUIRE(ids == std::vector<std::string>{"a"});
  }
}
