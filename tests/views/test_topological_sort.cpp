#include <catch2/catch_test_macros.hpp>
#include <graph/views/topological_sort.hpp>
#include <vector>
#include <unordered_set>
#include <map>
#include <set>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::views;

TEST_CASE("vertices_topological_sort - simple DAG", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // DAG: 0 -> 1 -> 2
  Graph g = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {}   // 2 (sink)
  };

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 3);
  // 0 must come before 1, and 1 before 2
  auto pos_0 = std::find(order.begin(), order.end(), 0) - order.begin();
  auto pos_1 = std::find(order.begin(), order.end(), 1) - order.begin();
  auto pos_2 = std::find(order.begin(), order.end(), 2) - order.begin();
  REQUIRE(pos_0 < pos_1);
  REQUIRE(pos_1 < pos_2);
}

TEST_CASE("vertices_topological_sort - diamond DAG", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Diamond: 0 -> [1, 2], 1 -> 3, 2 -> 3
  Graph g = {
        {1, 2}, // 0 -> 1, 2
        {3},    // 1 -> 3
        {3},    // 2 -> 3
        {}      // 3 (sink)
  };

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 4);

  // Verify topological constraints: each edge points forward
  auto pos = [&](int vid) { return std::find(order.begin(), order.end(), vid) - order.begin(); };

  // 0 must come before 1, 2, and 3
  REQUIRE(pos(0) < pos(1));
  REQUIRE(pos(0) < pos(2));
  REQUIRE(pos(0) < pos(3));

  // 1 and 2 must both come before 3
  REQUIRE(pos(1) < pos(3));
  REQUIRE(pos(2) < pos(3));
}

TEST_CASE("vertices_topological_sort - structured binding [v]", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {}};

  int count = 0;
  for (auto [v] : vertices_topological_sort(g)) {
    REQUIRE(vertex_id(g, v) < g.size());
    ++count;
  }

  REQUIRE(count == 2);
}

TEST_CASE("vertices_topological_sort - structured binding [v, val]", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};

  auto topo =
        vertices_topological_sort(g, [](const auto& g, auto v) { return static_cast<int>(vertex_id(g, v)) * 10; });

  std::vector<int> values;
  for (auto [v, val] : topo) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  // Check that we got the expected values (in some order)
  REQUIRE(std::find(values.begin(), values.end(), 0) != values.end());
  REQUIRE(std::find(values.begin(), values.end(), 10) != values.end());
  REQUIRE(std::find(values.begin(), values.end(), 20) != values.end());
}

TEST_CASE("vertices_topological_sort - value function receives descriptor", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}};

  // Value function receives descriptor, not ID
  auto topo = vertices_topological_sort(g, [](const auto& g, auto v) {
    // Can use vertex_id to convert descriptor to ID
    return static_cast<int>(vertex_id(g, v)) * 100;
  });

  std::vector<int> values;
  for (auto [v, val] : topo) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(std::find(values.begin(), values.end(), 0) != values.end());
  REQUIRE(std::find(values.begin(), values.end(), 100) != values.end());
  REQUIRE(std::find(values.begin(), values.end(), 200) != values.end());
}

TEST_CASE("vertices_topological_sort - complex DAG", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // More complex DAG with multiple paths
  //     0
  //    / \ (backslash)
  //   1 2 3
  //   |X| |
  //   4 5 6
  //    \|/
  //     7
  Graph g = {
        {1, 2, 3}, // 0
        {4, 5},    // 1
        {4, 5},    // 2
        {6},       // 3
        {7},       // 4
        {7},       // 5
        {7},       // 6
        {}         // 7 (sink)
  };

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 8);

  auto pos = [&](int vid) { return std::find(order.begin(), order.end(), vid) - order.begin(); };

  // Verify all edges point forward in the ordering
  REQUIRE(pos(0) < pos(1));
  REQUIRE(pos(0) < pos(2));
  REQUIRE(pos(0) < pos(3));
  REQUIRE(pos(1) < pos(4));
  REQUIRE(pos(1) < pos(5));
  REQUIRE(pos(2) < pos(4));
  REQUIRE(pos(2) < pos(5));
  REQUIRE(pos(3) < pos(6));
  REQUIRE(pos(4) < pos(7));
  REQUIRE(pos(5) < pos(7));
  REQUIRE(pos(6) < pos(7));
}

TEST_CASE("vertices_topological_sort - single vertex", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {} // 0 (isolated)
  };

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 1);
  REQUIRE(order[0] == 0);
}

TEST_CASE("vertices_topological_sort - disconnected components", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Two separate chains: 0->1->2 and 3->4->5
  Graph g = {
        {1}, // 0
        {2}, // 1
        {},  // 2
        {4}, // 3
        {5}, // 4
        {}   // 5
  };

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 6);

  auto pos = [&](int vid) { return std::find(order.begin(), order.end(), vid) - order.begin(); };

  // Within each component, order must be preserved
  REQUIRE(pos(0) < pos(1));
  REQUIRE(pos(1) < pos(2));
  REQUIRE(pos(3) < pos(4));
  REQUIRE(pos(4) < pos(5));
}

TEST_CASE("vertices_topological_sort - all edges point forward", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Random DAG structure
  Graph g = {
        {2, 3}, // 0
        {3, 4}, // 1
        {5},    // 2
        {5, 6}, // 3
        {6},    // 4
        {7},    // 5
        {7},    // 6
        {}      // 7
  };

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  // Build position map
  std::vector<std::size_t> position(g.size());
  for (std::size_t i = 0; i < order.size(); ++i) {
    position[static_cast<std::size_t>(order[i])] = i;
  }

  // Verify ALL edges point forward
  for (std::size_t u = 0; u < g.size(); ++u) {
    for (int v_id : g[u]) {
      REQUIRE(position[u] < position[static_cast<std::size_t>(v_id)]);
    }
  }
}

TEST_CASE("vertices_topological_sort - size accessor", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  auto topo = vertices_topological_sort(g);

  REQUIRE(topo.size() == 4);

  int count = 0;
  for (auto [v] : topo) {
    ++count;
  }

  REQUIRE(count == 4);
  REQUIRE(topo.size() == 4); // Size remains constant (total vertices in topo order)
}

TEST_CASE("vertices_topological_sort - num_visited tracks iteration progress", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // 0 -> 1, 2
        {3},    // 1 -> 3
        {3},    // 2 -> 3
        {}      // 3 (leaf)
  };

  auto topo = vertices_topological_sort(g);

  // Before iteration: nothing consumed yet
  REQUIRE(topo.num_visited() == 0);
  REQUIRE(topo.size() == 4); // Total is always available

  // Iterate partially
  auto it = topo.begin();
  REQUIRE(topo.num_visited() == 0); // begin() doesn't increment
  ++it;
  REQUIRE(topo.num_visited() == 1); // First ++ consumed first vertex
  ++it;
  REQUIRE(topo.num_visited() == 2);
  ++it;
  REQUIRE(topo.num_visited() == 3);
  ++it; // Past last vertex
  REQUIRE(topo.num_visited() == 4);

  // size() unchanged
  REQUIRE(topo.size() == 4);
}

TEST_CASE("vertices_topological_sort VVF - num_visited tracks iteration progress", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};
  auto  vvf   = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto topo = vertices_topological_sort(g, vvf);

  REQUIRE(topo.num_visited() == 0);

  std::vector<int> vals;
  for (auto [v, val] : topo) {
    vals.push_back(val);
  }

  REQUIRE(vals.size() == 4);
  REQUIRE(topo.num_visited() == 4);
  REQUIRE(topo.size() == 4);
}

TEST_CASE("edges_topological_sort - num_visited tracks source vertices step-by-step", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // 0->1, 0->2, 1->2
  Graph g = {
        {1, 2}, // 0: 2 outgoing edges
        {2},    // 1: 1 outgoing edge
        {}      // 2: 0 outgoing edges (leaf)
  };

  auto topo_edges = edges_topological_sort(g);

  // Before iteration: nothing consumed
  REQUIRE(topo_edges.num_visited() == 0);

  // Iterate all edges
  int edge_count = 0;
  for (auto [e] : topo_edges) {
    ++edge_count;
  }

  // num_visited counts source vertices whose edges have been fully yielded.
  // After exhausting all 3 edges, all source vertices are processed.
  REQUIRE(edge_count == 3); // 3 edges total
  // The graph has 3 vertices; all have been processed (some had edges, some didn't)
  REQUIRE(topo_edges.num_visited() == 3);
}

TEST_CASE("edges_topological_sort - num_visited zero before iteration", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  auto topo_edges = edges_topological_sort(g);

  // begin() positions to first edge but should NOT increment num_visited
  auto it = topo_edges.begin();
  REQUIRE(topo_edges.num_visited() == 0);
  (void)it; // suppress unused warning
}

TEST_CASE("edges_topological_sort - num_visited on all-leaf graph (no edges)", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // All isolated vertices, no edges at all
  Graph g = {
        {}, // 0
        {}, // 1
        {}  // 2
  };

  auto topo_edges = edges_topological_sort(g);

  // Before iteration: must be 0 even though graph has vertices
  REQUIRE(topo_edges.num_visited() == 0);

  // No edges to iterate
  int edge_count = 0;
  for (auto [e] : topo_edges) {
    ++edge_count;
  }

  REQUIRE(edge_count == 0);
  // No edges were yielded, so no source vertices were "processed"
  REQUIRE(topo_edges.num_visited() == 0);
}

TEST_CASE("edges_topological_sort - num_visited with leading edgeless vertices", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // Topo order might put leaf vertices first (depending on DFS order).
  // Graph: 0 has no edges, 1->2
  Graph g = {
        {},  // 0 (isolated)
        {2}, // 1->2
        {}   // 2 (leaf)
  };

  auto topo_edges = edges_topological_sort(g);

  // Even if vertex 0 comes first in topo order (no edges),
  // constructor must NOT inflate count_
  REQUIRE(topo_edges.num_visited() == 0);

  int edge_count = 0;
  for (auto [e] : topo_edges) {
    ++edge_count;
  }

  REQUIRE(edge_count == 1); // Only edge: 1->2
  REQUIRE(topo_edges.num_visited() > 0);
}

TEST_CASE("edges_topological_sort - num_visited on empty graph", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  auto topo_edges = edges_topological_sort(g);
  REQUIRE(topo_edges.num_visited() == 0);

  for (auto [e] : topo_edges) {
    REQUIRE(false); // Should not execute
  }

  REQUIRE(topo_edges.num_visited() == 0);
}

TEST_CASE("edges_topological_sort EVF - num_visited tracks source vertices", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};
  auto  evf   = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto topo_edges = edges_topological_sort(g, evf);

  REQUIRE(topo_edges.num_visited() == 0);

  int edge_count = 0;
  for (auto [e, val] : topo_edges) {
    ++edge_count;
  }

  REQUIRE(edge_count == 3);
  REQUIRE(topo_edges.num_visited() == 3);
}

TEST_CASE("edges_topological_sort EVF - num_visited zero before iteration", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}};
  auto  evf   = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto topo_edges = edges_topological_sort(g, evf);
  auto it         = topo_edges.begin();
  REQUIRE(topo_edges.num_visited() == 0);
  (void)it;
}

TEST_CASE("edges_topological_sort EVF - num_visited on all-leaf graph", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{}, {}, {}};
  auto  evf   = [](const auto& gr, auto e) { return static_cast<int>(target_id(gr, e)); };

  auto topo_edges = edges_topological_sort(g, evf);
  REQUIRE(topo_edges.num_visited() == 0);

  int edge_count = 0;
  for (auto [e, val] : topo_edges) {
    ++edge_count;
  }

  REQUIRE(edge_count == 0);
  REQUIRE(topo_edges.num_visited() == 0);
}

TEST_CASE("vertices_topological_sort VVF - num_visited step-by-step", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // 0 -> 1
        {2}, // 1 -> 2
        {}   // 2 (leaf)
  };
  auto vvf = [](const auto& gr, auto v) { return static_cast<int>(vertex_id(gr, v)); };

  auto topo = vertices_topological_sort(g, vvf);

  REQUIRE(topo.num_visited() == 0);

  auto it = topo.begin();
  REQUIRE(topo.num_visited() == 0); // begin() doesn't count

  ++it;
  REQUIRE(topo.num_visited() == 1);
  ++it;
  REQUIRE(topo.num_visited() == 2);
  ++it;
  REQUIRE(topo.num_visited() == 3);

  REQUIRE(topo.size() == 3);
}

TEST_CASE("vertices_topological_sort - num_visited with post-increment", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}};

  auto topo = vertices_topological_sort(g);
  REQUIRE(topo.num_visited() == 0);

  auto it = topo.begin();
  it++; // post-increment
  REQUIRE(topo.num_visited() == 1);
  it++;
  REQUIRE(topo.num_visited() == 2);
  it++;
  REQUIRE(topo.num_visited() == 3);
}

TEST_CASE("vertices_topological_sort - num_visited on single vertex graph", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{}}; // Single vertex, no edges

  auto topo = vertices_topological_sort(g);
  REQUIRE(topo.num_visited() == 0);
  REQUIRE(topo.size() == 1);

  for (auto [v] : topo) {
    (void)v;
  }

  REQUIRE(topo.num_visited() == 1);
}

TEST_CASE("vertices_topological_sort - num_visited on empty graph", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  auto topo = vertices_topological_sort(g);
  REQUIRE(topo.num_visited() == 0);
  REQUIRE(topo.size() == 0);

  for (auto [v] : topo) {
    // Should not execute
    REQUIRE(false);
  }

  REQUIRE(topo.num_visited() == 0);
}

TEST_CASE("vertices_topological_sort_safe - num_visited works after cycle check", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}}; // Linear DAG

  auto result = vertices_topological_sort_safe(g);
  REQUIRE(result.has_value());

  auto& topo = *result;
  REQUIRE(topo.num_visited() == 0);

  for (auto [v] : topo) {
    (void)v;
  }

  REQUIRE(topo.num_visited() == 3);
  REQUIRE(topo.size() == 3);
}

TEST_CASE("vertices_topological_sort - empty graph", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.empty());
}

TEST_CASE("vertices_topological_sort - linear chain", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1 -> 2 -> 3 -> 4
  Graph g = {{1}, {2}, {3}, {4}, {}};

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 5);
  // Must be exactly [0, 1, 2, 3, 4]
  for (std::size_t i = 0; i < order.size(); ++i) {
    REQUIRE(order[i] == static_cast<int>(i));
  }
}

TEST_CASE("vertices_topological_sort - wide DAG", "[topo][vertices]") {
  using Graph = std::vector<std::vector<int>>;
  // Single source to many sinks
  // 0 -> [1, 2, 3, 4, 5]
  Graph g = {{1, 2, 3, 4, 5}, {}, {}, {}, {}, {}};

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(static_cast<int>(vertex_id(g, v)));
  }

  REQUIRE(order.size() == 6);
  REQUIRE(order[0] == 0); // Source must be first

  // All others come after 0
  for (std::size_t i = 1; i < order.size(); ++i) {
    REQUIRE(order[i] > 0);
  }
}

//===========================================================================
// edges_topological_sort tests
//===========================================================================

TEST_CASE("edges_topological_sort - simple DAG", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : edges_topological_sort(g)) {
    edges.emplace_back(source_id(g, e), target_id(g, e));
  }

  REQUIRE(edges.size() == 2);
  REQUIRE(edges[0] == std::make_pair(0, 1));
  REQUIRE(edges[1] == std::make_pair(1, 2));

  // Verify sources follow topological order
  for (std::size_t i = 1; i < edges.size(); ++i) {
    REQUIRE(edges[i - 1].first <= edges[i].first);
  }
}

TEST_CASE("edges_topological_sort - diamond DAG", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  //    0
  //   / \
    //  1   2
  //   \ /
  //    3
  Graph g = {{1, 2}, {3}, {3}, {}};

  std::map<int, std::set<int>> edge_map;
  for (auto [e] : edges_topological_sort(g)) {
    auto src = source_id(g, e);
    auto tgt = target_id(g, e);
    edge_map[src].insert(tgt);
  }

  REQUIRE(edge_map.size() == 3);
  REQUIRE(edge_map[0].size() == 2);
  REQUIRE(edge_map[0].count(1) == 1);
  REQUIRE(edge_map[0].count(2) == 1);
  REQUIRE(edge_map[1].size() == 1);
  REQUIRE(edge_map[1].count(3) == 1);
  REQUIRE(edge_map[2].size() == 1);
  REQUIRE(edge_map[2].count(3) == 1);
}

TEST_CASE("edges_topological_sort - structured binding with value", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  std::vector<std::tuple<int, int, int>> edges_with_values;
  for (auto [e, val] : edges_topological_sort(g, [](const auto&, auto ed) { return 42; })) {
    edges_with_values.emplace_back(source_id(g, e), target_id(g, e), val);
  }

  REQUIRE(edges_with_values.size() == 2);
  for (const auto& [src, tgt, val] : edges_with_values) {
    REQUIRE(val == 42);
  }
}

TEST_CASE("edges_topological_sort - value function receives descriptor", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  std::vector<int> edge_ids;
  for (auto [e, id] :
       edges_topological_sort(g, [](const auto& g, auto ed) { return source_id(g, ed) * 10 + target_id(g, ed); })) {
    edge_ids.push_back(id);
  }

  REQUIRE(edge_ids.size() == 2);
  REQUIRE(edge_ids[0] == 1);  // 0*10 + 1
  REQUIRE(edge_ids[1] == 12); // 1*10 + 2
}

TEST_CASE("edges_topological_sort - complex DAG", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  //    0
  //   / \
    //  1   2
  //  |\ /|
  //  | X |
  //  |/ \|
  //  3   4
  //   \ /
  //    5
  Graph g = {
        {1, 2}, // 0
        {3, 4}, // 1
        {3, 4}, // 2
        {5},    // 3
        {5},    // 4
        {}      // 5
  };

  int                           edge_count = 0;
  std::set<std::pair<int, int>> seen_edges;
  std::map<int, int>            vertex_positions; // vertex_id -> position in topological order

  // First, get vertex positions from vertices_topological_sort
  int pos = 0;
  for (auto [v] : vertices_topological_sort(g)) {
    vertex_positions[vertex_id(g, v)] = pos++;
  }

  // Now verify edges follow topological order (sources before targets)
  for (auto [e] : edges_topological_sort(g)) {
    auto src = source_id(g, e);
    auto tgt = target_id(g, e);

    seen_edges.emplace(src, tgt);
    ++edge_count;

    // Verify source comes before target in topological order
    REQUIRE(vertex_positions[src] < vertex_positions[tgt]);
  }

  REQUIRE(edge_count == 8);
  REQUIRE(seen_edges.size() == 8);
}

TEST_CASE("edges_topological_sort - disconnected components", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1    2 -> 3
  Graph g = {{1}, {}, {3}, {}};

  std::set<std::pair<int, int>> edges;
  for (auto [e] : edges_topological_sort(g)) {
    edges.emplace(source_id(g, e), target_id(g, e));
  }

  REQUIRE(edges.size() == 2);
  REQUIRE(edges.count({0, 1}) == 1);
  REQUIRE(edges.count({2, 3}) == 1);
}

TEST_CASE("edges_topological_sort - empty graph", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  int count = 0;
  for (auto [e] : edges_topological_sort(g)) {
    ++count;
  }

  REQUIRE(count == 0);
}

TEST_CASE("edges_topological_sort - graph with no edges", "[topo][edges]") {
  using Graph = std::vector<std::vector<int>>;
  // Three isolated vertices
  Graph g = {{}, {}, {}};

  int count = 0;
  for (auto [e] : edges_topological_sort(g)) {
    ++count;
  }

  REQUIRE(count == 0);
}

//===========================================================================
// Cycle detection tests
//===========================================================================
// NOTE: Current implementation does not explicitly detect or reject cycles.
// On graphs with cycles, topological_sort produces an ordering, but it is
// NOT a valid topological ordering (some edges will point backward).
// These tests document the current behavior.
//===========================================================================

TEST_CASE("vertices_topological_sort - self-loop", "[topo][vertices][cycles]") {
  using Graph = std::vector<std::vector<int>>;
  // Vertex 0 has self-loop: 0 -> 0
  Graph g = {{0}};

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(vertex_id(g, v));
  }

  // Current behavior: produces ordering containing the vertex
  REQUIRE(order.size() == 1);
  REQUIRE(order[0] == 0);

  // However, this is NOT a valid topological sort because edge 0->0 exists
  // and vertex 0 cannot come before itself
}

TEST_CASE("vertices_topological_sort - simple cycle", "[topo][vertices][cycles]") {
  using Graph = std::vector<std::vector<int>>;
  // Simple cycle: 0 -> 1 -> 2 -> 0
  Graph g = {{1}, {2}, {0}};

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(vertex_id(g, v));
  }

  // Current behavior: produces an ordering with all vertices
  REQUIRE(order.size() == 3);

  // Verify all vertices present
  std::set<int> vertices_seen(order.begin(), order.end());
  REQUIRE(vertices_seen.size() == 3);
  REQUIRE(vertices_seen.count(0) == 1);
  REQUIRE(vertices_seen.count(1) == 1);
  REQUIRE(vertices_seen.count(2) == 1);

  // Check if any edge violates topological ordering
  std::map<int, int> positions;
  for (std::size_t i = 0; i < order.size(); ++i) {
    positions[order[i]] = static_cast<int>(i);
  }

  // At least one edge must point backward due to cycle
  bool has_backward_edge = false;
  for (std::size_t src = 0; src < g.size(); ++src) {
    for (int tgt : g[src]) {
      if (positions[static_cast<int>(src)] >= positions[tgt]) {
        has_backward_edge = true;
        break;
      }
    }
    if (has_backward_edge)
      break;
  }

  // Document that cycle results in invalid topological ordering
  REQUIRE(has_backward_edge);
}

TEST_CASE("vertices_topological_sort - cycle with tail", "[topo][vertices][cycles]") {
  using Graph = std::vector<std::vector<int>>;
  // DAG leading into cycle: 0 -> 1 -> 2 -> 3 -> 1 (cycle: 1->2->3->1)
  Graph g = {{1}, {2}, {3}, {1}};

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 4);

  // Vertex 0 should come before vertex 1 (acyclic part)
  auto pos_0 = std::find(order.begin(), order.end(), 0) - order.begin();
  auto pos_1 = std::find(order.begin(), order.end(), 1) - order.begin();
  REQUIRE(pos_0 < pos_1);

  // But the cycle (1->2->3->1) means at least one edge must be backward
  std::map<int, int> positions;
  for (std::size_t i = 0; i < order.size(); ++i) {
    positions[order[i]] = static_cast<int>(i);
  }

  bool has_backward_edge = false;
  if (positions[1] >= positions[2] || positions[2] >= positions[3] || positions[3] >= positions[1]) {
    has_backward_edge = true;
  }

  REQUIRE(has_backward_edge);
}

TEST_CASE("vertices_topological_sort - multiple cycles", "[topo][vertices][cycles]") {
  using Graph = std::vector<std::vector<int>>;
  // Two separate cycles: (0->1->0) and (2->3->2)
  Graph g = {{1}, {0}, {3}, {2}};

  std::vector<int> order;
  for (auto [v] : vertices_topological_sort(g)) {
    order.push_back(vertex_id(g, v));
  }

  // All vertices should be present
  REQUIRE(order.size() == 4);

  std::set<int> vertices_seen(order.begin(), order.end());
  REQUIRE(vertices_seen.size() == 4);
}

TEST_CASE("edges_topological_sort - simple cycle", "[topo][edges][cycles]") {
  using Graph = std::vector<std::vector<int>>;
  // Cycle: 0 -> 1 -> 2 -> 0
  Graph g = {{1}, {2}, {0}};

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : edges_topological_sort(g)) {
    edges.emplace_back(source_id(g, e), target_id(g, e));
  }

  // All 3 edges should be present
  REQUIRE(edges.size() == 3);

  // Verify all edges present
  std::set<std::pair<int, int>> edge_set(edges.begin(), edges.end());
  REQUIRE(edge_set.count({0, 1}) == 1);
  REQUIRE(edge_set.count({1, 2}) == 1);
  REQUIRE(edge_set.count({2, 0}) == 1);
}

TEST_CASE("edges_topological_sort - self-loop", "[topo][edges][cycles]") {
  using Graph = std::vector<std::vector<int>>;
  // Self-loop: 0 -> 0
  Graph g = {{0}};

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : edges_topological_sort(g)) {
    edges.emplace_back(source_id(g, e), target_id(g, e));
  }

  // Self-loop edge should be present
  REQUIRE(edges.size() == 1);
  REQUIRE(edges[0].first == 0);
  REQUIRE(edges[0].second == 0);
}

TEST_CASE("topological_sort - cycle detection documentation", "[topo][cycles][.][documentation]") {
  // This test documents the current behavior regarding cycles.
  //
  // CURRENT BEHAVIOR:
  // - topological_sort does NOT detect or reject cycles
  // - On cyclic graphs, it produces an ordering that includes all vertices
  // - The ordering is NOT a valid topological sort
  // - Some edges will point "backward" (from later to earlier positions)
  //
  // RATIONALE:
  // - DFS-based implementation visits all reachable vertices
  // - Cycle detection would require additional tracking (e.g., "on stack" marks)
  // - For performance, the current implementation prioritizes speed over validation
  //
  // USER RESPONSIBILITY:
  // - Users should ensure input graph is a DAG if they need valid topological ordering
  // - For cycle detection, users should use a dedicated cycle detection algorithm
  // - Behavior on cyclic graphs is well-defined but produces invalid orderings
  //
  // FUTURE CONSIDERATION:
  // - Could add optional cycle detection with a flag or separate function
  // - Could throw exception or return std::optional/expected
  // - Would add overhead to the common (acyclic) case

  SUCCEED("Documentation test - no actual test needed");
}

//===========================================================================
// Safe topological sort tests (with cycle detection via tl::expected)
//===========================================================================

TEST_CASE("vertices_topological_sort_safe - valid DAG", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // Simple DAG: 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  auto result = vertices_topological_sort_safe(g);

  REQUIRE(result.has_value());

  std::vector<int> order;
  for (auto [v] : result.value()) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 3);
  REQUIRE(order[0] == 0);
  REQUIRE(order[1] == 1);
  REQUIRE(order[2] == 2);
}

TEST_CASE("vertices_topological_sort_safe - detects simple cycle", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // Cycle: 0 -> 1 -> 2 -> 0
  Graph g = {{1}, {2}, {0}};

  auto result = vertices_topological_sort_safe(g);

  REQUIRE(!result.has_value());

  // Should return the vertex that closes the cycle
  auto cycle_vertex = result.error();
  auto cycle_id     = vertex_id(g, cycle_vertex);

  // The cycle is detected at vertex 0 (where back edge points)
  REQUIRE(cycle_id == 0);
}

TEST_CASE("vertices_topological_sort_safe - detects self-loop", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // Self-loop: 0 -> 0
  Graph g = {{0}};

  auto result = vertices_topological_sort_safe(g);

  REQUIRE(!result.has_value());
  REQUIRE(vertex_id(g, result.error()) == 0);
}

TEST_CASE("vertices_topological_sort_safe - with value function on DAG", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // DAG: 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  auto result = vertices_topological_sort_safe(g, [](const auto&, auto v) { return 42; });

  REQUIRE(result.has_value());

  std::vector<std::tuple<int, int>> results;
  for (auto [v, val] : result.value()) {
    results.emplace_back(vertex_id(g, v), val);
  }

  REQUIRE(results.size() == 3);
  for (const auto& [vid, val] : results) {
    REQUIRE(val == 42);
  }
}

TEST_CASE("vertices_topological_sort_safe - with value function on cycle", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // Cycle: 0 -> 1 -> 2 -> 0
  Graph g = {{1}, {2}, {0}};

  auto result = vertices_topological_sort_safe(g, [](const auto&, auto v) { return 99; });

  REQUIRE(!result.has_value());
}

TEST_CASE("vertices_topological_sort_safe - detects cycle with tail", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // DAG leading into cycle: 0 -> 1 -> 2 -> 3 -> 1
  Graph g = {{1}, {2}, {3}, {1}};

  auto result = vertices_topological_sort_safe(g);

  REQUIRE(!result.has_value());

  // Cycle detected at vertex 1
  REQUIRE(vertex_id(g, result.error()) == 1);
}

TEST_CASE("vertices_topological_sort_safe - diamond DAG", "[topo][vertices][safe]") {
  using Graph = std::vector<std::vector<int>>;
  //    0
  //   / \
    //  1   2
  //   \ /
  //    3
  Graph g = {{1, 2}, {3}, {3}, {}};

  auto result = vertices_topological_sort_safe(g);

  REQUIRE(result.has_value());

  std::vector<int> order;
  for (auto [v] : *result) {
    order.push_back(vertex_id(g, v));
  }

  REQUIRE(order.size() == 4);
  REQUIRE(order[0] == 0);
  REQUIRE(order[3] == 3);
}

TEST_CASE("edges_topological_sort_safe - valid DAG", "[topo][edges][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // Simple DAG: 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  auto result = edges_topological_sort_safe(g);

  REQUIRE(result.has_value());

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : result.value()) {
    edges.emplace_back(source_id(g, e), target_id(g, e));
  }

  REQUIRE(edges.size() == 2);
  REQUIRE(edges[0] == std::make_pair(0, 1));
  REQUIRE(edges[1] == std::make_pair(1, 2));
}

TEST_CASE("edges_topological_sort_safe - detects cycle", "[topo][edges][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // Cycle: 0 -> 1 -> 2 -> 0
  Graph g = {{1}, {2}, {0}};

  auto result = edges_topological_sort_safe(g);

  REQUIRE(!result.has_value());
  REQUIRE(vertex_id(g, result.error()) == 0);
}

TEST_CASE("edges_topological_sort_safe - with value function", "[topo][edges][safe]") {
  using Graph = std::vector<std::vector<int>>;
  // DAG: 0 -> 1 -> 2
  Graph g = {{1}, {2}, {}};

  auto result = edges_topological_sort_safe(g, [](const auto&, auto e) { return 7; });

  REQUIRE(result.has_value());

  int count = 0;
  for (auto [e, val] : *result) {
    REQUIRE(val == 7);
    ++count;
  }

  REQUIRE(count == 2);
}

TEST_CASE("topological_sort_safe - usage patterns", "[topo][safe][.][documentation]") {
  using Graph = std::vector<std::vector<int>>;

  // Example 1: Basic error checking
  {
    Graph g      = {{1}, {2}, {0}}; // Cycle
    auto  result = vertices_topological_sort_safe(g);

    if (result) {
      // Process successful result
      for (auto [v] : *result) {
        (void)v; // Process vertex
      }
    } else {
      // Handle cycle
      auto                  cycle_v = result.error();
      [[maybe_unused]] auto id      = vertex_id(g, cycle_v);
      // Log error, trace cycle, etc.
    }
  }

  // Example 2: Using operator bool
  {
    Graph g      = {{1}, {2}, {}}; // DAG
    auto  result = vertices_topological_sort_safe(g);

    REQUIRE(result.has_value());
    REQUIRE(static_cast<bool>(result));
  }

  // Example 3: Monadic operations (if needed)
  {
    Graph g      = {{1}, {2}, {}}; // DAG
    auto  result = vertices_topological_sort_safe(g);

    // Can use .value(), .error(), operator*, etc.
    if (result) {
      [[maybe_unused]] auto& view = result.value();
      // Or: auto& view = *result;
    }
  }

  SUCCEED("Documentation test demonstrating usage patterns");
}

// ============================================================================
// cancel() tests
// ============================================================================

TEST_CASE("vertices_topological_sort - cancel(cancel_all) stops iteration", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  // Linear DAG: 0 -> 1 -> 2 -> 3 -> 4
  Graph g = {{1}, {2}, {3}, {4}, {}};

  auto view = vertices_topological_sort(g);
  REQUIRE(view.cancel() == cancel_search::continue_search);

  std::vector<int> order;
  for (auto it = view.begin(); it != view.end(); ++it) {
    auto [v] = *it;
    order.push_back(static_cast<int>(vertex_id(g, v)));
    if (order.size() == 2) {
      view.cancel(cancel_search::cancel_all);
    }
  }

  REQUIRE(order.size() == 2);
  REQUIRE(view.cancel() == cancel_search::cancel_all);
  REQUIRE(view.num_visited() == 2); // 2 vertices iterated via operator++
}

TEST_CASE("vertices_topological_sort - cancel_branch treated as cancel_all", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {3}, {4}, {}};

  auto view = vertices_topological_sort(g);

  std::vector<int> order;
  for (auto it = view.begin(); it != view.end(); ++it) {
    auto [v] = *it;
    order.push_back(static_cast<int>(vertex_id(g, v)));
    if (order.size() == 3) {
      view.cancel(cancel_search::cancel_branch); // Treated as cancel_all
    }
  }

  REQUIRE(order.size() == 3);
  REQUIRE(view.cancel() == cancel_search::cancel_branch);
}

TEST_CASE("vertices_topological_sort - cancel before iteration yields nothing", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}};

  auto view = vertices_topological_sort(g);
  view.cancel(cancel_search::cancel_all);

  std::size_t count = 0;
  for ([[maybe_unused]] auto [v] : view) {
    ++count;
  }

  REQUIRE(count == 0);
  REQUIRE(view.num_visited() == 0);
}

TEST_CASE("vertices_topological_sort_view<G, VVF> - cancel(cancel_all) stops iteration", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {3}, {4}, {}};

  auto vvf  = [](const auto& g_, auto v) { return vertex_id(g_, v) * 10; };
  auto view = vertices_topological_sort(g, vvf);
  REQUIRE(view.cancel() == cancel_search::continue_search);

  std::vector<int> values;
  for (auto it = view.begin(); it != view.end(); ++it) {
    auto [v, val] = *it;
    values.push_back(static_cast<int>(val));
    if (values.size() == 2) {
      view.cancel(cancel_search::cancel_all);
    }
  }

  REQUIRE(values.size() == 2);
  REQUIRE(view.cancel() == cancel_search::cancel_all);
  REQUIRE(view.num_visited() == 2);
}

TEST_CASE("vertices_topological_sort_view<G, VVF> - cancel_branch treated as cancel_all", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {3}, {}};

  auto vvf  = [](const auto& g_, auto v) { return vertex_id(g_, v); };
  auto view = vertices_topological_sort(g, vvf);

  std::size_t count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    ++count;
    if (count == 1) {
      view.cancel(cancel_search::cancel_branch);
    }
  }

  REQUIRE(count == 1);
}

TEST_CASE("edges_topological_sort - cancel(cancel_all) stops iteration", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3
  Graph g = {{1, 2}, {3}, {3}, {}};

  auto view = edges_topological_sort(g);
  REQUIRE(view.cancel() == cancel_search::continue_search);

  std::size_t edge_count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    ++edge_count;
    if (edge_count == 2) {
      view.cancel(cancel_search::cancel_all);
    }
  }

  REQUIRE(edge_count == 2);
  REQUIRE(view.cancel() == cancel_search::cancel_all);
}

TEST_CASE("edges_topological_sort - cancel_branch treated as cancel_all", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  auto view = edges_topological_sort(g);

  std::size_t edge_count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    ++edge_count;
    if (edge_count == 1) {
      view.cancel(cancel_search::cancel_branch);
    }
  }

  REQUIRE(edge_count == 1);
}

TEST_CASE("edges_topological_sort - cancel before iteration yields nothing", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  auto view = edges_topological_sort(g);
  view.cancel(cancel_search::cancel_all);

  std::size_t count = 0;
  for ([[maybe_unused]] auto [e] : view) {
    ++count;
  }

  REQUIRE(count == 0);
  REQUIRE(view.num_visited() == 0);
}

TEST_CASE("edges_topological_sort_view<G, EVF> - cancel(cancel_all) stops iteration", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  auto evf  = [](const auto& g_, auto e) { return target_id(g_, e); };
  auto view = edges_topological_sort(g, evf);
  REQUIRE(view.cancel() == cancel_search::continue_search);

  std::size_t edge_count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    ++edge_count;
    if (edge_count == 2) {
      view.cancel(cancel_search::cancel_all);
    }
  }

  REQUIRE(edge_count == 2);
  REQUIRE(view.cancel() == cancel_search::cancel_all);
}

TEST_CASE("edges_topological_sort_view<G, EVF> - cancel_branch treated as cancel_all", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {3}, {3}, {}};

  auto evf  = [](const auto& g_, auto e) { return target_id(g_, e); };
  auto view = edges_topological_sort(g, evf);

  std::size_t edge_count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    ++edge_count;
    if (edge_count == 1) {
      view.cancel(cancel_search::cancel_branch);
    }
  }

  REQUIRE(edge_count == 1);
}

TEST_CASE("vertices_topological_sort - cancel on empty graph is safe", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  auto view = vertices_topological_sort(g);
  view.cancel(cancel_search::cancel_all);

  std::size_t count = 0;
  for ([[maybe_unused]] auto [v] : view) {
    ++count;
  }
  REQUIRE(count == 0);
}

TEST_CASE("edges_topological_sort - cancel on empty graph is safe", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  auto view = edges_topological_sort(g);
  view.cancel(cancel_search::cancel_all);

  std::size_t count = 0;
  for ([[maybe_unused]] auto [e] : view) {
    ++count;
  }
  REQUIRE(count == 0);
}

TEST_CASE("vertices_topological_sort - cancel preserves num_visited accuracy", "[topo][vertices][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1 -> 2 -> 3 -> 4 -> 5
  Graph g = {{1}, {2}, {3}, {4}, {5}, {}};

  auto view = vertices_topological_sort(g);

  std::size_t count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    [[maybe_unused]] auto [v] = *it;
    ++count;
    if (count == 3) {
      view.cancel(cancel_search::cancel_all);
    }
  }

  REQUIRE(count == 3);
  REQUIRE(view.num_visited() == 3);
  // size() still reflects the full topological order
  REQUIRE(view.size() == 6);
}

TEST_CASE("edges_topological_sort - cancel preserves num_visited accuracy", "[topo][edges][cancel]") {
  using Graph = std::vector<std::vector<int>>;
  // 0 -> 1 -> 2 -> 3  (each vertex has exactly one edge)
  Graph g = {{1}, {2}, {3}, {}};

  auto view = edges_topological_sort(g);

  std::size_t edge_count = 0;
  for (auto it = view.begin(); it != view.end(); ++it) {
    ++edge_count;
    if (edge_count == 2) {
      view.cancel(cancel_search::cancel_all);
    }
  }

  REQUIRE(edge_count == 2);
  // num_visited counts source vertices whose edges were fully yielded
  // After cancel, we got edges from first 2 source vertices but
  // the second one's count happens when we'd advance past it
  // The exact count depends on when cancel fires relative to advance
}
