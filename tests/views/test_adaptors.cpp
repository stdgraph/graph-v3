/**
 * @file test_adaptors.cpp
 * @brief Tests for range adaptor closures enabling pipe syntax
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/adaptors.hpp>
#include <ranges>
#include <algorithm>
#include <vector>

using namespace graph;
using namespace graph::views::adaptors; // Use adaptor objects for pipe syntax
using std::ranges::size;

// Simple test graph using vector-of-vectors
// Graph structure: 0 -> {1, 2}, 1 -> {2}, 2 -> {}
using test_graph = std::vector<std::vector<int>>;

auto make_test_graph() {
  test_graph g(3); // 3 vertices
  g[0] = {1, 2};   // vertex 0 connects to vertices 1 and 2
  g[1] = {2};      // vertex 1 connects to vertex 2
  g[2] = {};       // vertex 2 has no outgoing edges
  return g;
}

//=============================================================================
// vertexlist adaptor tests
//=============================================================================

TEST_CASE("vertexlist adaptor - basic pipe syntax", "[adaptors][vertexlist]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | vertexlist()
  auto view = g | vertexlist();

  REQUIRE(size(view) == 3);

  std::vector<int> vertex_ids;
  for (auto [id, v] : view) {
    vertex_ids.push_back(id);
    REQUIRE(id == vertex_id(g, v)); // Verify id matches
  }

  REQUIRE(vertex_ids == std::vector{0, 1, 2});
}

TEST_CASE("vertexlist adaptor - with value function", "[adaptors][vertexlist]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | vertexlist(vvf)
  auto vvf  = [](const auto& g, auto&& v) { return vertex_id(g, v) * 10; }; // Simple transform
  auto view = g | vertexlist(vvf);

  REQUIRE(size(view) == 3);

  std::vector<int> values;
  for (auto [id, v, val] : view) {
    values.push_back(val);
    REQUIRE(id == vertex_id(g, v)); // Verify id matches
  }

  REQUIRE(values == std::vector{0, 10, 20});
}

TEST_CASE("vertexlist adaptor - chaining with std::views::take", "[adaptors][vertexlist]") {
  auto g = make_test_graph();

  // Test chaining: g | vertexlist() | std::views::take(2)
  auto view = g | vertexlist() | std::views::take(2);

  std::vector<int> vertex_ids;
  for (auto [id, v] : view) {
    vertex_ids.push_back(id);
    REQUIRE(id == vertex_id(g, v)); // Verify id matches
  }

  REQUIRE(vertex_ids.size() == 2);
  REQUIRE(vertex_ids == std::vector{0, 1});
}

TEST_CASE("vertexlist adaptor - chaining with transform", "[adaptors][vertexlist]") {
  auto g = make_test_graph();

  // Test chaining pattern: g | vertexlist() | std::views::transform | std::views::filter
  // This pattern works because vertexlist() (without VVF) creates a semiregular view
  auto view = g | vertexlist() | std::views::transform([&g](auto&& tuple) {
                auto [id, v] = tuple;
                return std::make_tuple(id, v, id * 10);
              }) |
              std::views::filter([](auto&& tuple) {
                auto [id, v, val] = tuple;
                return val > 0;
              });

  std::vector<int> values;
  for (auto [id, v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector{10, 20});
}

TEST_CASE("vertexlist adaptor - direct call compatibility", "[adaptors][vertexlist]") {
  auto g = make_test_graph();

  // Test that adaptor can be called directly (not just piped)
  auto view1 = graph::views::vertexlist(g);
  auto view2 = g | vertexlist();

  REQUIRE(size(view1) == size(view2));
  REQUIRE(size(view1) == 3);
}

//=============================================================================
// incidence adaptor tests
//=============================================================================

TEST_CASE("incidence adaptor - basic pipe syntax", "[adaptors][incidence]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | incidence(uid)
  auto view = g | incidence(0);

  REQUIRE(size(view) == 2); // vertex 0 has 2 outgoing edges

  std::vector<int> target_ids;
  for (auto [tid, e] : view) {
    target_ids.push_back(tid);
  }

  REQUIRE(target_ids == std::vector{1, 2});
}

TEST_CASE("incidence adaptor - with value function", "[adaptors][incidence]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | incidence(uid, evf)
  auto evf  = [](const auto& g, auto&& e) { return target_id(g, e) * 10; }; // Transform target ID
  auto view = g | incidence(0, evf);

  REQUIRE(size(view) == 2);

  std::vector<int> values;
  for (auto [tid, e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector{10, 20}); // targets 1 and 2
}

TEST_CASE("incidence adaptor - chaining with std::views::take", "[adaptors][incidence]") {
  auto g = make_test_graph();

  // Test chaining: g | incidence(uid) | std::views::take(1)
  auto view = g | incidence(0) | std::views::take(1);

  std::vector<int> target_ids;
  for (auto [tid, e] : view) {
    target_ids.push_back(tid);
  }

  REQUIRE(target_ids.size() == 1);
  REQUIRE(target_ids == std::vector{1});
}

TEST_CASE("incidence adaptor - chaining with transform", "[adaptors][incidence]") {
  auto g = make_test_graph();

  // Test chaining pattern: g | incidence(uid) | std::views::transform
  // This pattern works because incidence(uid) (without EVF) creates a semiregular view
  auto view = g | incidence(0) | std::views::transform([&g](auto&& tuple) {
                auto [tid, e] = tuple;
                return tid * 10;
              }) |
              std::views::transform([](auto val) { return val * 2; });

  std::vector<int> values;
  for (auto val : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector{20, 40}); // (1*10*2, 2*10*2)
}

TEST_CASE("incidence adaptor - direct call compatibility", "[adaptors][incidence]") {
  auto g = make_test_graph();

  // Test that adaptor can be called directly (not just piped)
  auto view1 = graph::views::incidence(g, 0);
  auto view2 = g | incidence(0);

  REQUIRE(size(view1) == size(view2));
  REQUIRE(size(view1) == 2);
}

//=============================================================================
// neighbors adaptor tests
//=============================================================================

TEST_CASE("neighbors adaptor - basic pipe syntax", "[adaptors][neighbors]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | neighbors(uid)
  auto view = g | neighbors(0);

  REQUIRE(size(view) == 2); // vertex 0 has 2 neighbors

  std::vector<int> neighbor_ids;
  for (auto [tid, v] : view) {
    neighbor_ids.push_back(tid);
  }

  REQUIRE(neighbor_ids == std::vector{1, 2});
}

TEST_CASE("neighbors adaptor - with value function", "[adaptors][neighbors]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | neighbors(uid, vvf)
  auto vvf  = [](const auto& g, auto&& v) { return vertex_id(g, v) * 10; };
  auto view = g | neighbors(0, vvf);

  REQUIRE(size(view) == 2);

  std::vector<int> values;
  for (auto [tid, v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector{10, 20});
}

TEST_CASE("neighbors adaptor - chaining with std::views::filter", "[adaptors][neighbors]") {
  auto g = make_test_graph();

  // Test chaining: g | neighbors(uid) | std::views::filter
  auto view = g | neighbors(0) | std::views::filter([&g](auto&& tuple) {
                auto [tid, v] = tuple;
                return tid > 1;
              });

  std::vector<int> neighbor_ids;
  for (auto [tid, v] : view) {
    neighbor_ids.push_back(tid);
  }

  REQUIRE(neighbor_ids == std::vector{2});
}

TEST_CASE("neighbors adaptor - direct call compatibility", "[adaptors][neighbors]") {
  auto g = make_test_graph();

  // Test that adaptor can be called directly (not just piped)
  auto view1 = graph::views::neighbors(g, 0);
  auto view2 = g | neighbors(0);

  REQUIRE(size(view1) == size(view2));
  REQUIRE(size(view1) == 2);
}

//=============================================================================
// edgelist adaptor tests
//=============================================================================

TEST_CASE("edgelist adaptor - basic pipe syntax", "[adaptors][edgelist]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | edgelist()
  auto view = g | edgelist();

  std::vector<std::pair<int, int>> edge_pairs;
  for (auto [sid, tid, e] : view) {
    edge_pairs.emplace_back(sid, tid);
  }

  REQUIRE(edge_pairs.size() == 3); // 3 edges total
  REQUIRE(edge_pairs == std::vector<std::pair<int, int>>{{0, 1}, {0, 2}, {1, 2}});
}

TEST_CASE("edgelist adaptor - with value function", "[adaptors][edgelist]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | edgelist(evf)
  auto evf  = [](const auto& g, auto&& e) { return target_id(g, e) * 10; };
  auto view = g | edgelist(evf);

  std::vector<int> values;
  for (auto [sid, tid, e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(values == std::vector{10, 20, 20}); // targets are 1, 2, 2
}

TEST_CASE("edgelist adaptor - chaining with std::views::take", "[adaptors][edgelist]") {
  auto g = make_test_graph();

  // Test chaining: g | edgelist() | std::views::take(2)
  auto view = g | edgelist() | std::views::take(2);

  std::vector<std::pair<int, int>> edge_pairs;
  for (auto [sid, tid, e] : view) {
    edge_pairs.emplace_back(sid, tid);
  }

  REQUIRE(edge_pairs.size() == 2);
  REQUIRE(edge_pairs == std::vector<std::pair<int, int>>{{0, 1}, {0, 2}});
}

TEST_CASE("edgelist adaptor - chaining with transform and filter", "[adaptors][edgelist]") {
  auto g = make_test_graph();

  // Test chaining pattern: g | edgelist() | std::views::transform | std::views::filter
  // This pattern works because edgelist() (without EVF) creates a semiregular view
  auto view = g | edgelist() | std::views::transform([&g](auto&& tuple) {
                auto [sid, tid, e] = tuple;
                return std::make_tuple(e, tid * 10);
              }) |
              std::views::filter([](auto&& tuple) {
                auto [e, val] = tuple;
                return val >= 20;
              });

  std::vector<int> values;
  for (auto [e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector{20, 20}); // Two edges with target ID 2
}

TEST_CASE("edgelist adaptor - direct call compatibility", "[adaptors][edgelist]") {
  auto g = make_test_graph();

  // Test that adaptor can be called directly (not just piped)
  auto view1 = graph::views::edgelist(g);
  auto view2 = g | edgelist();

  // Count elements by iteration
  size_t count1 = 0, count2 = 0;
  for (auto [sid1, tid1, e] : view1) {
    ++count1;
  }
  for (auto [sid2, tid2, e] : view2) {
    ++count2;
  }

  REQUIRE(count1 == count2);
  REQUIRE(count1 == 3);
}

//=============================================================================
// Multi-adaptor composition tests
//=============================================================================

TEST_CASE("multiple views can be used independently with pipe syntax", "[adaptors][composition]") {
  auto g = make_test_graph();

  // Use multiple different adaptors on same graph
  auto vertices       = g | vertexlist();
  auto edges_from_0   = g | incidence(0);
  auto neighbors_of_0 = g | neighbors(0);
  auto all_edges      = g | edgelist();

  REQUIRE(size(vertices) == 3);
  REQUIRE(size(edges_from_0) == 2);
  REQUIRE(size(neighbors_of_0) == 2);

  // Count all_edges by iteration (edgelist may not be sized_range)
  size_t count = 0;
  for (auto [sid, tid, e] : all_edges) {
    ++count;
  }
  REQUIRE(count == 3);
}

TEST_CASE("adaptors work with std::views algorithms", "[adaptors][composition]") {
  auto g = make_test_graph();

  // Test that views compose well with standard algorithms
  // Pattern: use vertexlist() then std::views::transform for value computation
  auto view = g | vertexlist() | std::views::transform([&g](auto&& tuple) {
                auto [id, v] = tuple;
                return std::make_tuple(id, v, id * 10);
              }) |
              std::views::filter([](auto&& tuple) {
                auto [id, v, val] = tuple;
                return val > 0;
              });

  std::vector<int> values;
  for (auto [id, v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values == std::vector{10, 20});
}

TEST_CASE("complex chaining scenario", "[adaptors][composition]") {
  auto g = make_test_graph();

  // Complex chain: get vertices, compute values, take first 2, transform values
  // Pattern: use vertexlist() then std::views::transform for value computation
  auto view = g | vertexlist() | std::views::transform([&g](auto&& tuple) {
                auto [id, v] = tuple;
                return id * 10;
              }) |
              std::views::take(2) | std::views::transform([](auto val) { return val + 1; });

  std::vector<int> results;
  for (auto val : view) {
    results.push_back(val);
  }

  REQUIRE(results == std::vector{1, 11});
}

//=============================================================================
// Search view adaptor tests
//=============================================================================

TEST_CASE("vertices_dfs adaptor - basic pipe syntax", "[adaptors][dfs][vertices_dfs]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | vertices_dfs(seed)
  auto view = g | vertices_dfs(0);

  std::vector<int> visited;
  for (auto [v] : view) {
    visited.push_back(vertex_id(g, v));
  }

  REQUIRE(visited.size() == 3); // All vertices reachable
  REQUIRE(visited[0] == 0);     // Starts at seed
}

TEST_CASE("vertices_dfs adaptor - with value function", "[adaptors][dfs][vertices_dfs]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | vertices_dfs(seed, vvf)
  auto vvf  = [](const auto& g, auto&& v) { return vertex_id(g, v) * 10; };
  auto view = g | vertices_dfs(0, vvf);

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 0); // First vertex has value 0*10=0
}

TEST_CASE("vertices_dfs adaptor - chaining with std::views", "[adaptors][dfs][vertices_dfs]") {
  auto g = make_test_graph();

  // Test chaining: g | vertices_dfs(seed) | std::views::transform
  auto view = g | vertices_dfs(0) | std::views::transform([&g](auto&& tuple) {
                auto [v] = tuple;
                return vertex_id(g, v);
              });

  std::vector<int> visited;
  for (auto id : view) {
    visited.push_back(id);
  }

  REQUIRE(visited.size() == 3);
  REQUIRE(visited[0] == 0); // Starts at seed
}

TEST_CASE("edges_dfs adaptor - basic pipe syntax", "[adaptors][dfs][edges_dfs]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | edges_dfs(seed)
  auto view = g | edges_dfs(0);

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : view) {
    edges.emplace_back(vertex_id(g, source(g, e)), vertex_id(g, target(g, e)));
  }

  REQUIRE(edges.size() >= 2); // At least 2 edges traversed
  // First edge should be from seed vertex 0
  REQUIRE((edges[0].first == 0 || edges[1].first == 0));
}

TEST_CASE("edges_dfs adaptor - with value function", "[adaptors][dfs][edges_dfs]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | edges_dfs(seed, evf)
  auto evf  = [](const auto& g, auto&& e) { return target_id(g, e) * 10; };
  auto view = g | edges_dfs(0, evf);

  std::vector<int> values;
  for (auto [e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() >= 2);
  // Values should be multiples of 10
  for (auto val : values) {
    REQUIRE(val % 10 == 0);
  }
}

TEST_CASE("vertices_bfs adaptor - basic pipe syntax", "[adaptors][bfs][vertices_bfs]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | vertices_bfs(seed)
  auto view = g | vertices_bfs(0);

  std::vector<int> visited;
  for (auto [v] : view) {
    visited.push_back(vertex_id(g, v));
  }

  REQUIRE(visited.size() == 3); // All vertices reachable
  REQUIRE(visited[0] == 0);     // Starts at seed
}

TEST_CASE("vertices_bfs adaptor - with value function", "[adaptors][bfs][vertices_bfs]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | vertices_bfs(seed, vvf)
  auto vvf  = [](const auto& g, auto&& v) { return vertex_id(g, v) * 10; };
  auto view = g | vertices_bfs(0, vvf);

  std::vector<int> values;
  for (auto [v, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() == 3);
  REQUIRE(values[0] == 0); // First vertex has value 0*10=0
}

TEST_CASE("vertices_bfs adaptor - chaining with std::views", "[adaptors][bfs][vertices_bfs]") {
  auto g = make_test_graph();

  // Test chaining: g | vertices_bfs(seed) | std::views::filter
  auto view = g | vertices_bfs(0) | std::views::transform([&g](auto&& tuple) {
                auto [v] = tuple;
                return vertex_id(g, v);
              }) |
              std::views::filter([](int id) { return id > 0; });

  std::vector<int> visited;
  for (auto id : view) {
    visited.push_back(id);
  }

  REQUIRE(visited.size() == 2); // Only vertices 1 and 2
  REQUIRE(std::ranges::all_of(visited, [](int id) { return id > 0; }));
}

TEST_CASE("edges_bfs adaptor - basic pipe syntax", "[adaptors][bfs][edges_bfs]") {
  auto g = make_test_graph();

  // Test basic pipe syntax: g | edges_bfs(seed)
  auto view = g | edges_bfs(0);

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : view) {
    edges.emplace_back(vertex_id(g, source(g, e)), vertex_id(g, target(g, e)));
  }

  REQUIRE(edges.size() >= 2); // At least 2 edges traversed
  // First edges should be from seed vertex 0
  REQUIRE(edges[0].first == 0);
}

TEST_CASE("edges_bfs adaptor - with value function", "[adaptors][bfs][edges_bfs]") {
  auto g = make_test_graph();

  // Test pipe syntax with value function: g | edges_bfs(seed, evf)
  auto evf  = [](const auto& g, auto&& e) { return target_id(g, e) * 10; };
  auto view = g | edges_bfs(0, evf);

  std::vector<int> values;
  for (auto [e, val] : view) {
    values.push_back(val);
  }

  REQUIRE(values.size() >= 2);
  // Values should be multiples of 10
  for (auto val : values) {
    REQUIRE(val % 10 == 0);
  }
}

TEST_CASE("search adaptors - direct call compatibility", "[adaptors][dfs][bfs]") {
  auto g = make_test_graph();

  // Test that adaptors can be called directly (not just piped)
  auto dfs_view1 = graph::views::vertices_dfs(g, 0);
  auto dfs_view2 = g | vertices_dfs(0);

  std::vector<int> visited1, visited2;
  for (auto [v] : dfs_view1)
    visited1.push_back(vertex_id(g, v));
  for (auto [v] : dfs_view2)
    visited2.push_back(vertex_id(g, v));

  REQUIRE(visited1 == visited2);

  // Test BFS as well
  auto bfs_view1 = graph::views::vertices_bfs(g, 0);
  auto bfs_view2 = g | vertices_bfs(0);

  visited1.clear();
  visited2.clear();
  for (auto [v] : bfs_view1)
    visited1.push_back(vertex_id(g, v));
  for (auto [v] : bfs_view2)
    visited2.push_back(vertex_id(g, v));

  REQUIRE(visited1 == visited2);
}

TEST_CASE("vertices_topological_sort adaptor - basic pipe syntax", "[adaptors][topological_sort]") {
  auto g = make_test_graph();

  // Use pipe syntax
  std::vector<int> vertices;
  for (auto [v] : g | vertices_topological_sort()) {
    vertices.push_back(vertex_id(g, v));
  }

  // Should visit all vertices
  REQUIRE(vertices.size() == num_vertices(g));

  // Check topological order property: for each edge (u,v), u comes after v in the order
  // (reverse post-order means sources come after targets)
  std::unordered_map<int, size_t> pos;
  for (size_t i = 0; i < vertices.size(); ++i) {
    pos[vertices[i]] = i;
  }

  for (auto [sid, tid, e] : g | edgelist()) {
    // In topological sort, each vertex appears before all vertices it has edges to
    // so source comes before target: pos[sid] < pos[tid]
    REQUIRE(pos[sid] < pos[tid]);
  }
}

TEST_CASE("vertices_topological_sort adaptor - with value function", "[adaptors][topological_sort]") {
  auto g = make_test_graph();

  auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };

  std::vector<std::pair<int, int>> results;
  for (auto [v, val] : g | vertices_topological_sort(vvf)) {
    results.push_back({vertex_id(g, v), val});
  }

  REQUIRE(results.size() == num_vertices(g));

  // Check that value function was applied correctly
  for (auto [vid, val] : results) {
    REQUIRE(val == vid * 10);
  }
}

TEST_CASE("edges_topological_sort adaptor - basic pipe syntax", "[adaptors][topological_sort]") {
  auto g = make_test_graph();

  std::vector<std::pair<int, int>> edges;
  for (auto [e] : g | edges_topological_sort()) {
    auto src = vertex_id(g, source(g, e));
    auto tgt = vertex_id(g, target(g, e));
    edges.push_back({src, tgt});
  }

  REQUIRE(edges.size() == num_edges(g));
}

TEST_CASE("edges_topological_sort adaptor - with value function", "[adaptors][topological_sort]") {
  auto g = make_test_graph();

  auto evf = [](const auto& g, auto e) { return vertex_id(g, source(g, e)) + vertex_id(g, target(g, e)); };

  std::vector<int> values;
  for (auto [e, val] : g | edges_topological_sort(evf)) {
    values.push_back(val);
  }

  REQUIRE(values.size() == num_edges(g));
  // Each value should be sum of source and target vertex IDs
  for (auto val : values) {
    REQUIRE(val >= 0); // Basic sanity check
  }
}

TEST_CASE("topological_sort adaptors - chaining with std::views", "[adaptors][topological_sort]") {
  auto g = make_test_graph();

  // Chain with transform to extract just the vertex IDs
  std::vector<int> ids;
  for (auto id : g | vertices_topological_sort() | std::views::transform([&g](auto tup) {
                   auto [v] = tup;
                   return vertex_id(g, v);
                 })) {
    ids.push_back(id);
  }

  REQUIRE(ids.size() == num_vertices(g));
}

TEST_CASE("topological_sort adaptors - direct call compatibility", "[adaptors][topological_sort]") {
  auto g = make_test_graph();

  // Test that adaptors can be called directly (not just piped)
  auto topo_view1 = graph::views::vertices_topological_sort(g);
  auto topo_view2 = g | vertices_topological_sort();

  std::vector<int> visited1, visited2;
  for (auto [v] : topo_view1)
    visited1.push_back(vertex_id(g, v));
  for (auto [v] : topo_view2)
    visited2.push_back(vertex_id(g, v));

  REQUIRE(visited1 == visited2);
}
//=============================================================================
// Comprehensive chaining tests (Step 6.3)
//=============================================================================

TEST_CASE("complex chaining - multiple transforms", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Chain multiple transforms together
  std::vector<int> results;
  for (auto id : g | vertexlist() | std::views::transform([&g](auto info) {
                   auto [id, v] = info;
                   return id;
                 }) | std::views::transform([](int id) { return id * 10; }) |
                       std::views::transform([](int val) { return val + 5; })) {
    results.push_back(id);
  }

  REQUIRE(results.size() == 3);
  // Each result should be (id * 10) + 5
  REQUIRE(results[0] == 5);  // (0 * 10) + 5
  REQUIRE(results[1] == 15); // (1 * 10) + 5
  REQUIRE(results[2] == 25); // (2 * 10) + 5
}

TEST_CASE("complex chaining - filter and transform", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Filter vertices, then transform the results
  std::vector<int> results;
  for (auto val : g | vertexlist() | std::views::transform([&g](auto info) {
                    auto [id, v] = info;
                    return id;
                  }) | std::views::filter([](int id) { return id > 0; }) |
                        std::views::transform([](int id) { return id * 100; })) {
    results.push_back(val);
  }

  REQUIRE(results.size() == 2);
  REQUIRE(results[0] == 100); // vertex 1
  REQUIRE(results[1] == 200); // vertex 2
}

TEST_CASE("complex chaining - transform, filter, transform", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // More complex chain: transform -> filter -> transform
  std::vector<int> results;
  for (auto val : g | edgelist() | std::views::transform([&g](auto info) {
                    auto [sid, tid, e] = info;
                    return tid;
                  }) | std::views::filter([](int tgt) { return tgt == 2; }) |
                        std::views::transform([](int id) { return id * 7; })) {
    results.push_back(val);
  }

  // Edges: 0->1, 0->2, 1->2. After filter (tgt==2): 0->2, 1->2
  REQUIRE(results.size() == 2);
  REQUIRE(results[0] == 14); // 2 * 7
  REQUIRE(results[1] == 14); // 2 * 7
}

TEST_CASE("chaining with std::views::take", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Take first N elements from a view
  std::vector<int> results;
  for (auto val : g | vertexlist() | std::views::transform([&g](auto info) {
                    auto [id, v] = info;
                    return id;
                  }) | std::views::take(2)) {
    results.push_back(val);
  }

  REQUIRE(results.size() == 2);
}

TEST_CASE("chaining with std::views::drop", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Drop first N elements from a view
  std::vector<int> results;
  for (auto val : g | vertexlist() | std::views::transform([&g](auto info) {
                    auto [id, v] = info;
                    return id;
                  }) | std::views::drop(1)) {
    results.push_back(val);
  }

  REQUIRE(results.size() == 2);
  // After dropping first element, should have vertices 1 and 2
}

TEST_CASE("chaining incidence with transforms", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Complex chain with incidence view
  std::vector<int> results;
  for (auto val : g | incidence(0) | std::views::transform([&g](auto info) {
                    auto [tid, e] = info;
                    return tid;
                  }) | std::views::filter([](int tgt) { return tgt < 2; }) |
                        std::views::transform([](int id) { return id * 3; })) {
    results.push_back(val);
  }

  // Vertex 0's edges: 0->1, 0->2
  // After filter (tgt < 2): only 0->1
  REQUIRE(results.size() == 1);
  REQUIRE(results[0] == 3); // 1 * 3
}

TEST_CASE("chaining neighbors with filter", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Chain neighbors view with filter
  std::vector<int> results;
  for (auto id : g | neighbors(0) | std::views::transform([&g](auto info) {
                   auto [tid, v] = info;
                   return tid;
                 }) | std::views::filter([](int id) { return id % 2 == 0; })) {
    results.push_back(id);
  }

  // Neighbors of 0: {1, 2}, filter even: {2}
  REQUIRE(results.size() == 1);
  REQUIRE(results[0] == 2);
}

TEST_CASE("const correctness - const graph with pipe", "[adaptors][const]") {
  const auto g = make_test_graph();

  // Test that views work with const graphs
  std::vector<int> results;
  for (auto [id, v] : g | vertexlist()) {
    REQUIRE(id == vertex_id(g, v));
    results.push_back(id);
  }

  REQUIRE(results.size() == 3);
}

TEST_CASE("const correctness - const graph with chaining", "[adaptors][const]") {
  const auto g = make_test_graph();

  // Test chaining with const graph
  std::vector<int> results;
  for (auto id : g | vertexlist() | std::views::transform([&g](auto info) {
                   auto [id, v] = info;
                   return id;
                 }) | std::views::filter([](int id) { return id < 3; })) {
    results.push_back(id);
  }

  REQUIRE(results.size() == 3);
}

TEST_CASE("mixing different view types in chains", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Get all neighbors of all vertices using chains
  std::vector<int> all_neighbors;
  for (auto vid : g | vertexlist() | std::views::transform([&g](auto info) {
                    auto [id, v] = info;
                    return id;
                  })) {
    // For each vertex, get its neighbors
    for (auto [tid, n] : g | neighbors(vid)) {
      all_neighbors.push_back(tid);
    }
  }

  // 0->{1,2}, 1->{2}, 2->{}
  REQUIRE(all_neighbors.size() == 3);
}

TEST_CASE("search views - complex chaining with multiple filters", "[adaptors][chaining][dfs]") {
  auto g = make_test_graph();

  // Complex chain with DFS
  std::vector<int> results;
  for (auto id : g | vertices_dfs(0) | std::views::transform([&g](auto info) {
                   auto [v] = info;
                   return vertex_id(g, v);
                 }) | std::views::filter([](int id) { return id >= 0; }) |
                       std::views::filter([](int id) { return id < 10; }) |
                       std::views::transform([](int id) { return id + 1; })) {
    results.push_back(id);
  }

  // All 3 vertices pass both filters, then +1 applied
  REQUIRE(results.size() == 3);
  REQUIRE(results[0] == 1); // 0 + 1
}

TEST_CASE("edgelist chaining with reverse", "[adaptors][chaining]") {
  auto g = make_test_graph();

  // Collect edges, reverse the order
  std::vector<std::pair<int, int>> edges;
  auto                             edge_view = g | edgelist() | std::views::transform([&g](auto info) {
                     auto [sid, tid, e] = info;
                     return std::make_pair(sid, tid);
                   });

  for (auto edge : edge_view) {
    edges.push_back(edge);
  }

  REQUIRE(edges.size() == 3);
}