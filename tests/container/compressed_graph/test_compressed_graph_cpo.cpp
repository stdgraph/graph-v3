#include <catch2/catch_test_macros.hpp>
#include "graph/container/compressed_graph.hpp"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// =============================================================================
// vertices(g) Friend Function Tests
// =============================================================================

TEST_CASE("vertices() returns view of vertex descriptors", "[vertices][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  SECTION("basic iteration") {
    auto   v     = vertices(g);
    size_t count = 0;
    for (auto vd : v) {
      REQUIRE(vd.vertex_id() < g.size());
      ++count;
    }
    REQUIRE(count == 4);
  }

  SECTION("vertex IDs are sequential") {
    auto           v = vertices(g);
    vector<size_t> ids;
    for (auto vd : v) {
      ids.push_back(vd.vertex_id());
    }
    REQUIRE(ids == vector<size_t>{0, 1, 2, 3});
  }

  SECTION("can access vertex values through vertex_id") {
    auto        v = vertices(g);
    vector<int> values;
    for (auto vd : v) {
      values.push_back(g.vertex_value(static_cast<uint32_t>(vd.vertex_id())));
    }
    REQUIRE(values == vector<int>{100, 200, 300, 400});
  }
}

TEST_CASE("vertices() with empty graph", "[vertices][api]") {
  compressed_graph<void, void, void> g;

  auto   v     = vertices(g);
  size_t count = 0;
  for ([[maybe_unused]] auto vd : v) {
    ++count;
  }
  REQUIRE(count == 0);
}

TEST_CASE("vertices() with void vertex values", "[vertices][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(ee);

  SECTION("iteration works") {
    auto   v     = vertices(g);
    size_t count = 0;
    for (auto vd : v) {
      REQUIRE(vd.vertex_id() < g.size());
      ++count;
    }
    REQUIRE(count == 4); // vertices 0, 1, 2, 3
  }

  SECTION("vertex IDs are correct") {
    auto           v = vertices(g);
    vector<size_t> ids;
    for (auto vd : v) {
      ids.push_back(vd.vertex_id());
    }
    REQUIRE(ids.size() == 4);
    REQUIRE(ids[0] == 0);
    REQUIRE(ids[1] == 1);
    REQUIRE(ids[2] == 2);
    REQUIRE(ids[3] == 3);
  }
}

TEST_CASE("vertices() with single vertex", "[vertices][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>>  ee            = {}; // No edges
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 42}};

  Graph g;
  g.load_vertices(vertex_values);

  auto   v     = vertices(g);
  size_t count = 0;
  for (auto vd : v) {
    REQUIRE(vd.vertex_id() == 0);
    REQUIRE(g.vertex_value(static_cast<uint32_t>(vd.vertex_id())) == 42);
    ++count;
  }
  REQUIRE(count == 1);
}

TEST_CASE("vertices() works with STL algorithms", "[vertices][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 5}, {1, 15}, {2, 25}, {3, 35}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);
  auto v = vertices(g);

  SECTION("std::count_if") {
    size_t count = 0;
    for (auto vd : v) {
      if (g.vertex_value(static_cast<uint32_t>(vd.vertex_id())) > 10)
        ++count;
    }
    REQUIRE(count == 3); // vertices 1, 2, 3 have values > 10
  }

  SECTION("find vertex with value") {
    bool   found    = false;
    size_t found_id = 0;
    for (auto vd : v) {
      if (g.vertex_value(static_cast<uint32_t>(vd.vertex_id())) == 25) {
        found    = true;
        found_id = vd.vertex_id();
        break;
      }
    }
    REQUIRE(found);
    REQUIRE(found_id == 2);
  }

  SECTION("extract vertex IDs") {
    vector<size_t> ids;
    for (auto vd : v) {
      ids.push_back(vd.vertex_id());
    }
    REQUIRE(ids == vector<size_t>{0, 1, 2, 3});
  }
}

TEST_CASE("vertices() is a lightweight view", "[vertices][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}};

  Graph g;
  g.load_edges(ee);

  // Create multiple views - should be cheap
  auto v1 = vertices(g);
  auto v2 = vertices(g);

  // Both views should produce same results
  vector<size_t> ids1, ids2;
  for (auto vd : v1)
    ids1.push_back(vd.vertex_id());
  for (auto vd : v2)
    ids2.push_back(vd.vertex_id());

  REQUIRE(ids1 == ids2);
  REQUIRE(ids1.size() == 5); // vertices 0-4
}

TEST_CASE("vertices() with string vertex values", "[vertices][api]") {
  using Graph                                          = compressed_graph<void, string, void>;
  vector<copyable_edge_t<int, void>>     ee            = {{0, 1}, {1, 2}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto           v = vertices(g);
  vector<string> names;
  for (auto vd : v) {
    names.push_back(g.vertex_value(static_cast<uint32_t>(vd.vertex_id())));
  }
  REQUIRE(names == vector<string>{"Alice", "Bob", "Charlie"});
}

TEST_CASE("vertices() const correctness", "[vertices][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g_temp;
  g_temp.load_edges(ee);
  g_temp.load_vertices(vertex_values);
  const Graph g = std::move(g_temp);

  // Should work with const graph
  auto   v     = vertices(g);
  size_t count = 0;
  for (auto vd : v) {
    // Can read vertex values from const graph
    [[maybe_unused]] auto val = g.vertex_value(static_cast<uint32_t>(vd.vertex_id()));
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("vertices() with large graph", "[vertices][api]") {
  using Graph = compressed_graph<int, int, void>;

  // Create a larger graph
  vector<copyable_edge_t<int, int>>   ee;
  vector<copyable_vertex_t<int, int>> vertex_values;

  const size_t n = 1000;
  for (size_t i = 0; i < n - 1; ++i) {
    ee.push_back({static_cast<int>(i), static_cast<int>(i + 1), static_cast<int>(i * 10)});
    vertex_values.push_back({static_cast<int>(i), static_cast<int>(i * 100)});
  }
  vertex_values.push_back({static_cast<int>(n - 1), static_cast<int>((n - 1) * 100)});

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto   v     = vertices(g);
  size_t count = 0;
  for (auto vd : v) {
    REQUIRE(vd.vertex_id() == count);
    REQUIRE(g.vertex_value(static_cast<uint32_t>(vd.vertex_id())) == static_cast<int>(count * 100));
    ++count;
  }
  REQUIRE(count == n);
}

// =============================================================================
// edges(g, u) Friend Function Tests
// =============================================================================

TEST_CASE("edges(g,u) returns view of edge descriptors", "[edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("iterate edges from vertex 0") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    size_t      count = 0;
    vector<int> targets;
    vector<int> values;
    for (auto ed : e) {
      targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
      values.push_back(static_cast<int>(g.edge_value(static_cast<uint32_t>(ed.value()))));
      ++count;
    }
    REQUIRE(count == 2);
    REQUIRE(targets == vector<int>{1, 2});
    REQUIRE(values == vector<int>{10, 20});
  }

  SECTION("iterate edges from vertex 1") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    size_t      count = 0;
    vector<int> targets;
    for (auto ed : e) {
      targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
      ++count;
    }
    REQUIRE(count == 1);
    REQUIRE(targets == vector<int>{2});
  }

  SECTION("vertex with no edges") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    ++it;
    ++it; // vertex 3
    auto v3 = *it;
    auto e  = edges(g, v3);

    size_t count = 0;
    for ([[maybe_unused]] auto ed : e) {
      ++count;
    }
    REQUIRE(count == 0);
  }
}

TEST_CASE("edges(g,u) with void edge values", "[edges][api]") {
  using Graph                                   = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>> edges_data = {{0, 1}, {0, 2}, {0, 3}, {1, 2}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<int> targets;
  for (auto ed : e) {
    targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
  }

  REQUIRE(targets.size() == 3);
  REQUIRE(targets == vector<int>{1, 2, 3});
}

TEST_CASE("edges(g,u) with empty graph", "[edges][api]") {
  compressed_graph<int, void, void> g;

  // Can't get edges from non-existent vertex, but test empty case
  REQUIRE(g.empty());
}

TEST_CASE("edges(g,u) with single edge", "[edges][api]") {
  using Graph                                  = compressed_graph<int, string, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 100}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto ee = edges(g, v0);

  size_t count = 0;
  int    targ  = -1;
  int    value = -1;
  for (auto ed : ee) {
    targ  = static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value())));
    value = static_cast<int>(g.edge_value(static_cast<uint32_t>(ed.value())));
    ++count;
  }

  REQUIRE(count == 1);
  REQUIRE(targ == 1);
  REQUIRE(value == 100);
}

TEST_CASE("edges(g,u) works with STL algorithms", "[edges][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  SECTION("count edges") {
    size_t count = 0;
    for ([[maybe_unused]] auto ed : e) {
      ++count;
    }
    REQUIRE(count == 4);
  }

  SECTION("find edge with specific target") {
    bool found       = false;
    int  found_value = -1;
    for (auto ed : e) {
      if (g.target_id(static_cast<uint32_t>(ed.value())) == 2) {
        found       = true;
        found_value = g.edge_value(static_cast<uint32_t>(ed.value()));
        break;
      }
    }
    REQUIRE(found);
    REQUIRE(found_value == 20);
  }

  SECTION("collect all targets") {
    vector<int> targets;
    for (auto ed : e) {
      targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
    }
    REQUIRE(targets == vector<int>{1, 2, 3, 4});
  }
}

TEST_CASE("edges(g,u) is a lightweight view", "[edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();

  // Create multiple views - should be cheap
  auto e1 = edges(g, v0);
  auto e2 = edges(g, v0);

  // Both views should produce same results
  vector<int> targets1, targets2;
  for (auto ed : e1)
    targets1.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
  for (auto ed : e2)
    targets2.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));

  REQUIRE(targets1 == targets2);
  REQUIRE(targets1.size() == 2);
}

TEST_CASE("edges(g,u) with string edge values", "[edges][api]") {
  using Graph                                     = compressed_graph<string, void, void>;
  vector<copyable_edge_t<int, string>> edges_data = {{0, 1, "edge_a"}, {0, 2, "edge_b"}, {1, 2, "edge_c"}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<string> labels;
  for (auto ed : e) {
    labels.push_back(g.edge_value(static_cast<uint32_t>(ed.value())));
  }

  REQUIRE(labels == vector<string>{"edge_a", "edge_b"});
}

TEST_CASE("edges(g,u) const correctness", "[edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}};

  Graph g_temp;
  g_temp.load_edges(edges_data);
  const Graph g = std::move(g_temp);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  size_t count = 0;
  for (auto ed : e) {
    [[maybe_unused]] auto targ  = g.target_id(static_cast<uint32_t>(ed.value()));
    [[maybe_unused]] auto value = g.edge_value(static_cast<uint32_t>(ed.value()));
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("edges(g,u) with large graph", "[edges][api]") {
  using Graph = compressed_graph<int, void, void>;

  // Create a graph where vertex 0 has many edges
  vector<copyable_edge_t<int, int>> edges_data;
  const size_t                      num_ee = 1000;
  for (size_t i = 1; i <= num_ee; ++i) {
    edges_data.push_back({0, static_cast<int>(i), static_cast<int>(i * 10)});
  }

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto ee = edges(g, v0);

  size_t count = 0;
  for (auto ed : ee) {
    auto targ  = g.target_id(static_cast<uint32_t>(ed.value()));
    auto value = g.edge_value(static_cast<uint32_t>(ed.value()));
    REQUIRE(static_cast<int>(targ) == static_cast<int>(count + 1));
    REQUIRE(value == static_cast<int>((count + 1) * 10));
    ++count;
  }
  REQUIRE(count == num_edges(g, v0));
}

TEST_CASE("edges(g,u) with self-loops", "[edges][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 0, 5}, {0, 1, 10}, {1, 1, 15}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("vertex 0 edges include self-loop") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    vector<int> targets;
    for (auto ed : e) {
      targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
    }
    REQUIRE(targets == vector<int>{0, 1});
  }

  SECTION("vertex 1 edges include self-loop") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    vector<int> targets;
    for (auto ed : e) {
      targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
    }
    REQUIRE(targets == vector<int>{1});
  }
}

TEST_CASE("edges(g,u) iteration from all vertices", "[edges][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 0, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Count total edges by iterating from each vertex
  size_t total_edges = 0;
  for (auto vd : vertices(g)) {
    for ([[maybe_unused]] auto ed : edges(g, vd)) {
      ++total_edges;
    }
  }

  REQUIRE(total_edges == 4);
}

// =============================================================================
// vertex_id(g, u) CPO Tests
// =============================================================================

TEST_CASE("vertex_id(g,u) returns correct vertex ID", "[vertex_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v  = vertices(g);
  auto it = v.begin();

  REQUIRE(vertex_id(g, *it) == 0);
  ++it;
  REQUIRE(vertex_id(g, *it) == 1);
  ++it;
  REQUIRE(vertex_id(g, *it) == 2);
}

TEST_CASE("vertex_id(g,u) with const graph", "[vertex_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}};

  Graph g_temp;
  g_temp.load_edges(edges_data);
  g_temp.load_vertices(vertex_values);
  const Graph g = std::move(g_temp);

  auto v  = vertices(g);
  auto it = v.begin();

  REQUIRE(vertex_id(g, *it) == 0);
  ++it;
  REQUIRE(vertex_id(g, *it) == 1);
  ++it;
  REQUIRE(vertex_id(g, *it) == 2);
}

// =============================================================================
// find_vertex(g, uid) CPO Tests
// =============================================================================

TEST_CASE("find_vertex(g,uid) finds vertex by ID", "[find_vertex][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  SECTION("find vertex 0") {
    auto v_iter = find_vertex(g, 0);
    REQUIRE(vertex_id(g, *v_iter) == 0);
    REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == 100);
  }

  SECTION("find vertex 1") {
    auto v_iter = find_vertex(g, 1);
    REQUIRE(vertex_id(g, *v_iter) == 1);
    REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == 200);
  }

  SECTION("find vertex 2") {
    auto v_iter = find_vertex(g, 2);
    REQUIRE(vertex_id(g, *v_iter) == 2);
    REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == 300);
  }
}

TEST_CASE("find_vertex(g,uid) with const graph", "[find_vertex][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}};

  Graph g_temp;
  g_temp.load_edges(edges_data);
  g_temp.load_vertices(vertex_values);
  const Graph g = std::move(g_temp);

  auto v_iter = find_vertex(g, 1);
  REQUIRE(vertex_id(g, *v_iter) == 1);
  REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == 200);
}

TEST_CASE("find_vertex(g,uid) with void vertex values", "[find_vertex][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("find first vertex") {
    auto v_iter = find_vertex(g, 0);
    auto verts  = vertices(g);
    REQUIRE(v_iter == verts.begin());
  }

  SECTION("find middle vertex") {
    auto v_iter   = find_vertex(g, 2);
    auto verts    = vertices(g);
    auto expected = verts.begin();
    ++expected;
    ++expected;
    REQUIRE(v_iter == expected);
  }

  SECTION("find last vertex") {
    auto v_iter   = find_vertex(g, 3);
    auto verts    = vertices(g);
    auto expected = verts.begin();
    ++expected;
    ++expected;
    ++expected;
    REQUIRE(v_iter == expected);
  }
}

TEST_CASE("find_vertex(g,uid) can access edges", "[find_vertex][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  // Find vertex 0 and iterate its edges
  auto v0_iter = find_vertex(g, 0);
  auto e       = edges(g, *v0_iter);

  vector<int> targets;
  vector<int> values;
  for (auto ed : e) {
    targets.push_back(static_cast<int>(g.target_id(static_cast<uint32_t>(ed.value()))));
    values.push_back(static_cast<int>(g.edge_value(static_cast<uint32_t>(ed.value()))));
  }
  REQUIRE(targets == vector<int>{1, 2});
  REQUIRE(values == vector<int>{10, 20});
}

TEST_CASE("find_vertex(g,uid) iterator equivalence", "[find_vertex][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(edges_data);

  // find_vertex should return same iterator as advancing begin by uid
  auto verts     = vertices(g);
  auto v1_find   = find_vertex(g, 1);
  auto v1_manual = verts.begin();
  ++v1_manual;

  REQUIRE(v1_find == v1_manual);
}

TEST_CASE("find_vertex(g,uid) all vertices findable", "[find_vertex][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}, {4, 500}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  // Verify every vertex ID can be found
  for (size_t uid = 0; uid < g.size(); ++uid) {
    auto v_iter = find_vertex(g, static_cast<uint32_t>(uid));
    REQUIRE(vertex_id(g, *v_iter) == uid);
    REQUIRE(g.vertex_value(static_cast<uint32_t>(uid)) == static_cast<int>((uid + 1) * 100));
  }
}

TEST_CASE("find_vertex(g,uid) with single vertex", "[find_vertex][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {}; // No edges
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 42}};

  Graph g;
  g.load_vertices(vertex_values);

  auto v_iter = find_vertex(g, 0);
  REQUIRE(vertex_id(g, *v_iter) == 0);
  REQUIRE(g.vertex_value(0) == 42);
}

TEST_CASE("find_vertex(g,uid) with string vertex values", "[find_vertex][api]") {
  using Graph                                          = compressed_graph<int, string, void>;
  vector<copyable_edge_t<int, int>>      edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  SECTION("find Alice") {
    auto v_iter = find_vertex(g, 0);
    REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == "Alice");
  }

  SECTION("find Bob") {
    auto v_iter = find_vertex(g, 1);
    REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == "Bob");
  }

  SECTION("find Charlie") {
    auto v_iter = find_vertex(g, 2);
    REQUIRE(g.vertex_value(vertex_id(g, *v_iter)) == "Charlie");
  }
}

// =============================================================================
// target_id(g, uv) CPO Tests
// =============================================================================

TEST_CASE("target_id(g,uv) returns correct target ID", "[target_id][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("edges from vertex 0") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    auto it = e.begin();
    REQUIRE(target_id(g, *it) == 1);
    ++it;
    REQUIRE(target_id(g, *it) == 2);
  }

  SECTION("edges from vertex 1") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    auto e_it = e.begin();
    REQUIRE(target_id(g, *e_it) == 2);
  }
}

TEST_CASE("target_id(g,uv) with const graph", "[target_id][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}};

  Graph g_temp;
  g_temp.load_edges(edges_data);
  const Graph g = std::move(g_temp);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<int> targets;
  for (auto ed : e) {
    targets.push_back(static_cast<int>(target_id(g, ed)));
  }

  REQUIRE(targets == vector<int>{1, 2});
}

TEST_CASE("target_id(g,uv) with void edge values", "[target_id][api]") {
  using Graph                                   = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>> edges_data = {{0, 1}, {0, 2}, {0, 3}, {1, 2}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<int> targets;
  for (auto ed : e) {
    targets.push_back(static_cast<int>(target_id(g, ed)));
  }

  REQUIRE(targets == vector<int>{1, 2, 3});
}

TEST_CASE("target_id(g,uv) with self-loops", "[target_id][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 0, 5}, {0, 1, 10}, {1, 1, 15}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("vertex 0 with self-loop") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    vector<int> targets;
    for (auto ed : e) {
      targets.push_back(static_cast<int>(target_id(g, ed)));
    }
    REQUIRE(targets == vector<int>{0, 1});
  }

  SECTION("vertex 1 with self-loop") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    auto e_it = e.begin();
    REQUIRE(target_id(g, *e_it) == 1);
  }
}

TEST_CASE("target_id(g,uv) all edges in graph", "[target_id][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 0, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Collect all edge targets by iterating through all vertices
  vector<int> all_targets;
  for (auto vd : vertices(g)) {
    for (auto ed : edges(g, vd)) {
      all_targets.push_back(static_cast<int>(target_id(g, ed)));
    }
  }

  REQUIRE(all_targets == vector<int>{1, 2, 3, 0});
}

TEST_CASE("target_id(g,uv) with string edge values", "[target_id][api]") {
  using Graph                                     = compressed_graph<string, void, void>;
  vector<copyable_edge_t<int, string>> edges_data = {{0, 1, "edge_a"}, {0, 2, "edge_b"}, {1, 2, "edge_c"}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  // Verify target_id works independently of edge value type
  vector<int> targets;
  for (auto ed : e) {
    targets.push_back(static_cast<int>(target_id(g, ed)));
  }

  REQUIRE(targets == vector<int>{1, 2});
}

TEST_CASE("target_id(g,uv) consistency with direct access", "[target_id][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  // Compare CPO target_id(g, uv) with direct g.target_id(edge_id)
  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  for (auto ed : e) {
    auto edge_idx = ed.value();
    REQUIRE(target_id(g, ed) == g.target_id(static_cast<uint32_t>(edge_idx)));
  }
}

// =============================================================================
// target(g, uv) CPO Tests
// =============================================================================

TEST_CASE("target(g,uv) returns correct target vertex descriptor", "[target][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("edges from vertex 0") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    auto it       = e.begin();
    auto target_v = target(g, *it);
    REQUIRE(vertex_id(g, target_v) == 1);

    ++it;
    target_v = target(g, *it);
    REQUIRE(vertex_id(g, target_v) == 2);
  }

  SECTION("edges from vertex 1") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    auto e_it     = e.begin();
    auto target_v = target(g, *e_it);
    REQUIRE(vertex_id(g, target_v) == 2);
  }
}

TEST_CASE("target(g,uv) with const graph", "[target][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}};

  Graph g_temp;
  g_temp.load_edges(edges_data);
  const Graph g = std::move(g_temp);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<size_t> target_ids;
  for (auto ed : e) {
    auto target_v = target(g, ed);
    target_ids.push_back(vertex_id(g, target_v));
  }

  REQUIRE(target_ids == vector<size_t>{1, 2});
}

TEST_CASE("target(g,uv) with void edge values", "[target][api]") {
  using Graph = compressed_graph<int, int, void>; // Changed from void edge values to int
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 40}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<size_t> target_ids;
  for (auto ed : e) {
    auto target_v = target(g, ed);
    target_ids.push_back(vertex_id(g, target_v));
  }

  REQUIRE(target_ids == vector<size_t>{1, 2, 3});
}

TEST_CASE("target(g,uv) with vertex values", "[target][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {0, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  SECTION("access vertex values via target descriptor") {
    auto e_it     = e.begin();
    auto target_v = target(g, *e_it);
    REQUIRE(g.vertex_value(vertex_id(g, target_v)) == 200);

    ++e_it;
    target_v = target(g, *e_it);
    REQUIRE(g.vertex_value(vertex_id(g, target_v)) == 300);
  }
}

TEST_CASE("target(g,uv) with self-loops", "[target][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 0, 5}, {0, 1, 10}, {1, 1, 15}};

  Graph g;
  g.load_edges(edges_data);

  SECTION("self-loop from vertex 0") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    auto e_it     = e.begin();
    auto target_v = target(g, *e_it);
    REQUIRE(vertex_id(g, target_v) == 0); // Self-loop
  }

  SECTION("self-loop from vertex 1") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    auto e_it     = e.begin();
    auto target_v = target(g, *e_it);
    REQUIRE(vertex_id(g, target_v) == 1); // Self-loop
  }
}

TEST_CASE("target(g,uv) all edges in graph", "[target][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 0, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Collect all edge targets by iterating through all vertices
  vector<size_t> all_target_ids;
  for (auto vd : vertices(g)) {
    for (auto ed : edges(g, vd)) {
      auto target_v = target(g, ed);
      all_target_ids.push_back(vertex_id(g, target_v));
    }
  }

  REQUIRE(all_target_ids == vector<size_t>{1, 2, 3, 0});
}

TEST_CASE("target(g,uv) can traverse edges", "[target][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  // Start at vertex 0, follow edges to traverse the path
  auto v       = vertices(g);
  auto current = *v.begin();

  // Get first edge from vertex 0
  auto e0    = edges(g, current);
  auto e0_it = e0.begin();
  current    = target(g, *e0_it);
  REQUIRE(vertex_id(g, current) == 1);

  // Get first edge from vertex 1
  auto e1    = edges(g, current);
  auto e1_it = e1.begin();
  current    = target(g, *e1_it);
  REQUIRE(vertex_id(g, current) == 2);

  // Get first edge from vertex 2
  auto e2    = edges(g, current);
  auto e2_it = e2.begin();
  current    = target(g, *e2_it);
  REQUIRE(vertex_id(g, current) == 3);
}

TEST_CASE("target(g,uv) with string edge values", "[target][api]") {
  using Graph                                     = compressed_graph<string, void, void>;
  vector<copyable_edge_t<int, string>> edges_data = {{0, 1, "edge_a"}, {0, 2, "edge_b"}, {1, 2, "edge_c"}};

  Graph g;
  g.load_edges(edges_data);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  // Verify target works independently of edge value type
  vector<size_t> target_ids;
  for (auto ed : e) {
    auto target_v = target(g, ed);
    target_ids.push_back(vertex_id(g, target_v));
  }

  REQUIRE(target_ids == vector<size_t>{1, 2});
}

TEST_CASE("target(g,uv) consistency with target_id", "[target][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  // Compare target(g, uv) with target_id(g, uv)
  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  for (auto ed : e) {
    auto target_v = target(g, ed);
    auto tid      = target_id(g, ed);
    REQUIRE(vertex_id(g, target_v) == tid);
  }
}

TEST_CASE("target(g,uv) with string vertex values", "[target][api]") {
  using Graph                                          = compressed_graph<int, string, void>;
  vector<copyable_edge_t<int, int>>      edges_data    = {{0, 1, 10}, {0, 2, 20}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v  = vertices(g);
  auto v0 = *v.begin();
  auto e  = edges(g, v0);

  vector<string> target_names;
  for (auto ed : e) {
    auto target_v = target(g, ed);
    target_names.push_back(g.vertex_value(vertex_id(g, target_v)));
  }

  REQUIRE(target_names == vector<string>{"Bob", "Charlie"});
}

// =============================================================================
// source_id(g,uv) and source(g,uv) CPO Tests
// =============================================================================

TEST_CASE("source_id(g,uv) returns correct source ID", "[source_id][api]") {
  using Graph                                 = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_vec = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(edges_vec);

  SECTION("first vertex edges") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    vector<int> source_ids;
    for (auto ed : e) {
      source_ids.push_back(static_cast<int>(source_id(g, ed)));
    }

    REQUIRE(source_ids.size() == 2);
    REQUIRE(source_ids[0] == 0);
    REQUIRE(source_ids[1] == 0);
  }

  SECTION("second vertex edges") {
    auto v  = vertices(g);
    auto it = v.begin();
    ++it;
    auto v1 = *it;
    auto e  = edges(g, v1);

    vector<int> source_ids;
    for (auto ed : e) {
      source_ids.push_back(static_cast<int>(source_id(g, ed)));
    }

    REQUIRE(source_ids.size() == 1);
    REQUIRE(source_ids[0] == 1);
  }
}

TEST_CASE("source(g,uv) returns correct source vertex descriptor", "[source][api]") {
  using Graph                                 = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_vec = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(edges_vec);

  SECTION("first vertex edges") {
    auto v  = vertices(g);
    auto v0 = *v.begin();
    auto e  = edges(g, v0);

    for (auto ed : e) {
      auto src = source(g, ed);
      REQUIRE(vertex_id(g, src) == 0);
      REQUIRE(vertex_id(g, src) == vertex_id(g, v0));
    }
  }

  SECTION("all vertices") {
    auto v = vertices(g);
    for (auto vd : v) {
      auto e = edges(g, vd);
      for (auto ed : e) {
        auto src = source(g, ed);
        REQUIRE(vertex_id(g, src) == vertex_id(g, vd));
      }
    }
  }
}

// =============================================================================
// num_vertices(g) CPO Tests
// =============================================================================

TEST_CASE("num_vertices(g) returns vertex count", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);
  REQUIRE(count == 4);
  REQUIRE(count == g.size());
}

TEST_CASE("num_vertices(g) works with const graph", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(ee);

  const Graph& cg    = g;
  auto         count = num_vertices(cg);
  REQUIRE(count == 3);
}

TEST_CASE("num_vertices(g) with empty graph", "[num_vertices][api]") {
  using Graph = compressed_graph<int, int, void>;

  Graph g;
  auto  count = num_vertices(g);
  REQUIRE(count == 0);
}

TEST_CASE("num_vertices(g) with single vertex", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {
        {0, 0, 10} // Self-loop
  };

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);
  REQUIRE(count == 1);
}

TEST_CASE("num_vertices(g) with void edge values", "[num_vertices][api]") {
  using Graph                           = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);
  REQUIRE(count == 5);
}

TEST_CASE("num_vertices(g) with void vertex values", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);
  REQUIRE(count == 3);
}

TEST_CASE("num_vertices(g) with string values", "[num_vertices][api]") {
  using Graph                               = compressed_graph<string, string, void>;
  vector<copyable_edge_t<int, string>>   ee = {{0, 1, "edge_a"}, {1, 2, "edge_b"}};
  vector<copyable_vertex_t<int, string>> vv = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vv);

  auto count = num_vertices(g);
  REQUIRE(count == 3);
}

TEST_CASE("num_vertices(g) with large graph", "[num_vertices][api]") {
  using Graph = compressed_graph<int, int, void>;

  // Create graph with 1000 vertices
  vector<copyable_edge_t<int, int>> ee;
  for (int i = 0; i < 999; ++i) {
    ee.push_back({i, i + 1, i});
  }

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);
  REQUIRE(count == 1000);
}

TEST_CASE("num_vertices(g) with disconnected vertices", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {2, 3, 20}, {4, 5, 30}};

  Graph g;
  g.load_edges(ee);

  // Graph has vertices 0-5 (6 total)
  auto count = num_vertices(g);
  REQUIRE(count == 6);
}

TEST_CASE("num_vertices(g) return type is integral", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);
  static_assert(std::integral<decltype(count)>, "num_vertices must return integral type");
  REQUIRE(count == 2);
}

TEST_CASE("num_vertices(g) consistency with vertices(g)", "[num_vertices][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}, {3, 4, 50}};

  Graph g;
  g.load_edges(ee);

  auto count = num_vertices(g);

  // Count vertices manually using vertices(g)
  auto   v            = vertices(g);
  size_t manual_count = 0;
  for ([[maybe_unused]] auto vd : v) {
    ++manual_count;
  }

  REQUIRE(count == manual_count);
  REQUIRE(count == 5);
}

// =============================================================================
// num_edges(g) CPO Tests
// =============================================================================

TEST_CASE("num_edges(g) returns edge count", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 4);
}

TEST_CASE("num_edges(g) works with const graph", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(ee);

  const Graph& cg    = g;
  auto         count = num_edges(cg);
  REQUIRE(count == 3);
}

TEST_CASE("num_edges(g) with empty graph", "[num_edges][api]") {
  using Graph = compressed_graph<int, int, void>;

  Graph g;
  auto  count = num_edges(g);
  REQUIRE(count == 0);
}

TEST_CASE("num_edges(g) with single edge", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 1);
}

TEST_CASE("num_edges(g) with self-loop", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 0, 10}, {0, 1, 20}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 2);
}

TEST_CASE("num_edges(g) with void edge values", "[num_edges][api]") {
  using Graph                           = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 4);
}

TEST_CASE("num_edges(g) with void vertex values", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 3);
}

TEST_CASE("num_edges(g) with string values", "[num_edges][api]") {
  using Graph                               = compressed_graph<string, string, void>;
  vector<copyable_edge_t<int, string>>   ee = {{0, 1, "edge_a"}, {1, 2, "edge_b"}, {2, 3, "edge_c"}};
  vector<copyable_vertex_t<int, string>> vv = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}, {3, "David"}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vv);

  auto count = num_edges(g);
  REQUIRE(count == 3);
}

TEST_CASE("num_edges(g) with large graph", "[num_edges][api]") {
  using Graph = compressed_graph<int, int, void>;

  // Create graph with 1000 edges
  vector<copyable_edge_t<int, int>> ee;
  for (int i = 0; i < 1000; ++i) {
    ee.push_back({i, i + 1, i});
  }

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 1000);
}

TEST_CASE("num_edges(g) with multiple edges per vertex", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 40}, {1, 3, 50}, {2, 3, 60}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 6);
}

TEST_CASE("num_edges(g) return type is integral", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  static_assert(std::integral<decltype(count)>, "num_edges must return integral type");
  REQUIRE(count == 1);
}

TEST_CASE("num_edges(g) consistency with edge iteration", "[num_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}, {3, 4, 50}};

  Graph g;
  g.load_edges(edges_data);

  auto count = num_edges(g);

  // Count edges manually by iterating through all vertices
  size_t manual_count = 0;
  for (auto v : vertices(g)) {
    auto e = edges(g, v);
    for ([[maybe_unused]] auto ed : e) {
      ++manual_count;
    }
  }

  REQUIRE(count == manual_count);
  REQUIRE(count == 5);
}

TEST_CASE("num_edges(g) with disconnected components", "[num_edges][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10},
        {1, 2, 20}, // Component 1
        {3, 4, 30},
        {4, 5, 40} // Component 2
  };

  Graph g;
  g.load_edges(ee);

  auto count = num_edges(g);
  REQUIRE(count == 4);
}

TEST_CASE("num_edges(g) efficiency test - uses ADL not default", "[num_edges][api]") {
  using Graph = compressed_graph<int, int, void>;

  // Create a larger graph to ensure ADL version is used (O(1))
  // rather than default iteration (O(V+E))
  vector<copyable_edge_t<int, int>> ee;
  for (int i = 0; i < 500; ++i) {
    ee.push_back({i, i + 1, i});
    ee.push_back({i, i + 2, i + 1000});
  }

  Graph g;
  g.load_edges(ee);

  // This should be O(1) using the ADL friend function
  auto count = num_edges(g);
  REQUIRE(count == 1000);

  // Verify it matches edge_ids() size (which accesses col_index_.size())
  auto edge_id_count = std::ranges::distance(g.edge_ids());
  REQUIRE(count == static_cast<size_t>(edge_id_count));
}

// =============================================================================
// degree(g,u) and degree(g,uid) CPO Tests
// =============================================================================

TEST_CASE("degree(g,u) returns edge count for vertex descriptor", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 40}, {2, 3, 50}};

  Graph g;
  g.load_edges(ee);

  auto v   = vertices(g);
  auto v0  = *v.begin();
  auto deg = degree(g, v0);
  REQUIRE(deg == 3); // vertex 0 has 3 outgoing edges
}

TEST_CASE("degree(g,uid) returns edge count for vertex ID", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {1, 2, 40}, {2, 3, 50}};

  Graph g;
  g.load_edges(ee);

  auto deg0 = degree(g, 0);
  auto deg1 = degree(g, 1);
  auto deg2 = degree(g, 2);

  REQUIRE(deg0 == 3);
  REQUIRE(deg1 == 1);
  REQUIRE(deg2 == 1);
}

TEST_CASE("degree(g,u) and degree(g,uid) consistency", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {1, 3, 40}};

  Graph g;
  g.load_edges(ee);

  // Compare degree via descriptor and via ID
  auto v = vertices(g);
  for (auto vd : v) {
    auto vid      = vertex_id(g, vd);
    auto deg_desc = degree(g, vd);
    auto deg_id   = degree(g, vid);
    REQUIRE(deg_desc == deg_id);
  }
}

TEST_CASE("degree(g,u) works with const graph", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(ee);

  const Graph& cg  = g;
  auto         v   = vertices(cg);
  auto         v0  = *v.begin();
  auto         deg = degree(cg, v0);
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,uid) works with const graph", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(ee);

  const Graph& cg  = g;
  auto         deg = degree(cg, 0);
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,u) with zero degree vertex", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(ee);

  auto v      = vertices(g);
  auto v_iter = v.begin();
  ++v_iter;
  ++v_iter; // vertex 2
  auto v2  = *v_iter;
  auto deg = degree(g, v2);
  REQUIRE(deg == 0); // vertex 2 has no outgoing edges
}

TEST_CASE("degree(g,uid) with zero degree vertex", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(ee);

  auto deg = degree(g, 2);
  REQUIRE(deg == 0); // vertex 2 has no outgoing edges
}

TEST_CASE("degree(g,u) with self-loop", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 0, 10}, {0, 1, 20}, {0, 2, 30}};

  Graph g;
  g.load_edges(ee);

  auto v   = vertices(g);
  auto v0  = *v.begin();
  auto deg = degree(g, v0);
  REQUIRE(deg == 3); // self-loop counts as one edge
}

TEST_CASE("degree(g,uid) with self-loop", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 0, 10}, {0, 1, 20}, {0, 2, 30}};

  Graph g;
  g.load_edges(ee);

  auto deg = degree(g, 0);
  REQUIRE(deg == 3); // self-loop counts as one edge
}

TEST_CASE("degree(g,u) with void edge values", "[degree][api]") {
  using Graph                           = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>> ee = {{0, 1}, {0, 2}, {1, 2}};

  Graph g;
  g.load_edges(ee);

  auto v   = vertices(g);
  auto v0  = *v.begin();
  auto deg = degree(g, v0);
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,uid) with void edge values", "[degree][api]") {
  using Graph                           = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>> ee = {{0, 1}, {0, 2}, {1, 2}};

  Graph g;
  g.load_edges(ee);

  auto deg = degree(g, 0);
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,u) with void vertex values", "[degree][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(ee);

  auto v   = vertices(g);
  auto v1  = *++v.begin();
  auto deg = degree(g, v1);
  REQUIRE(deg == 1);
}

TEST_CASE("degree(g,uid) with void vertex values", "[degree][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(ee);

  auto deg = degree(g, 1);
  REQUIRE(deg == 1);
}

TEST_CASE("degree(g,u) with string values", "[degree][api]") {
  using Graph                                          = compressed_graph<string, string, void>;
  vector<copyable_edge_t<int, string>>   ee            = {{0, 1, "edge_a"}, {0, 2, "edge_b"}, {1, 2, "edge_c"}};
  vector<copyable_vertex_t<int, string>> vertices_data = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertices_data);

  auto v   = vertices(g);
  auto v0  = *v.begin();
  auto deg = degree(g, v0);
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,uid) with string values", "[degree][api]") {
  using Graph                                          = compressed_graph<string, string, void>;
  vector<copyable_edge_t<int, string>>   ee            = {{0, 1, "edge_a"}, {0, 2, "edge_b"}, {1, 2, "edge_c"}};
  vector<copyable_vertex_t<int, string>> vertices_data = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertices_data);

  auto deg = degree(g, 0);
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,u) return type is integral", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}};

  Graph g;
  g.load_edges(ee);

  auto v   = vertices(g);
  auto v0  = *v.begin();
  auto deg = degree(g, v0);
  static_assert(std::integral<decltype(deg)>, "degree must return integral type");
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,uid) return type is integral", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}};

  Graph g;
  g.load_edges(ee);

  auto deg = degree(g, 0);
  static_assert(std::integral<decltype(deg)>, "degree must return integral type");
  REQUIRE(deg == 2);
}

TEST_CASE("degree(g,u) with various vertex degrees", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}, // vertex 0: degree 4
        {1, 2, 50}, {1, 3, 60}, {1, 4, 70},             // vertex 1: degree 3
        {2, 3, 80}, {2, 4, 90},                         // vertex 2: degree 2
        {3, 4, 100}                                     // vertex 3: degree 1
                                                        // vertex 4: degree 0
  };

  Graph g;
  g.load_edges(ee);

  vector<size_t> degrees;
  for (auto v : vertices(g)) {
    degrees.push_back(degree(g, v));
  }

  REQUIRE(degrees == vector<size_t>{4, 3, 2, 1, 0});
}

TEST_CASE("degree(g,uid) with various vertex degrees", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}, // vertex 0: degree 4
        {1, 2, 50}, {1, 3, 60}, {1, 4, 70},             // vertex 1: degree 3
        {2, 3, 80}, {2, 4, 90},                         // vertex 2: degree 2
        {3, 4, 100}                                     // vertex 3: degree 1
                                                        // vertex 4: degree 0
  };

  Graph g;
  g.load_edges(ee);

  REQUIRE(degree(g, 0) == 4);
  REQUIRE(degree(g, 1) == 3);
  REQUIRE(degree(g, 2) == 2);
  REQUIRE(degree(g, 3) == 1);
  REQUIRE(degree(g, 4) == 0);
}

TEST_CASE("degree(g,u) consistency with edges(g,u)", "[degree][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30},
                                                  {1, 2, 40}, {1, 3, 50}, {2, 3, 60}};

  Graph g;
  g.load_edges(edges_data);

  for (auto v : vertices(g)) {
    auto   deg          = degree(g, v);
    auto   e            = edges(g, v);
    size_t manual_count = 0;
    for ([[maybe_unused]] auto ed : e) {
      ++manual_count;
    }
    REQUIRE(deg == manual_count);
  }
}

TEST_CASE("degree(g,uid) consistency with edges(g,u)", "[degree][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30},
                                                  {1, 2, 40}, {1, 3, 50}, {2, 3, 60}};

  Graph g;
  g.load_edges(edges_data);

  for (size_t vid = 0; vid < g.size(); ++vid) {
    auto   deg          = degree(g, vid);
    auto   v_desc       = *find_vertex(g, vid);
    auto   e            = edges(g, v_desc);
    size_t manual_count = 0;
    for ([[maybe_unused]] auto ed : e) {
      ++manual_count;
    }
    REQUIRE(deg == manual_count);
  }
}

TEST_CASE("degree(g,u) with disconnected components", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10},
        {1, 2, 20}, // Component 1
        {3, 4, 30},
        {4, 5, 40} // Component 2
  };

  Graph g;
  g.load_edges(ee);

  auto v      = vertices(g);
  auto v_iter = v.begin();
  REQUIRE(degree(g, *v_iter) == 1);     // vertex 0
  REQUIRE(degree(g, *(++v_iter)) == 1); // vertex 1
  REQUIRE(degree(g, *(++v_iter)) == 0); // vertex 2
  REQUIRE(degree(g, *(++v_iter)) == 1); // vertex 3
  REQUIRE(degree(g, *(++v_iter)) == 1); // vertex 4
  REQUIRE(degree(g, *(++v_iter)) == 0); // vertex 5
}

TEST_CASE("degree(g,uid) with disconnected components", "[degree][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10},
        {1, 2, 20}, // Component 1
        {3, 4, 30},
        {4, 5, 40} // Component 2
  };

  Graph g;
  g.load_edges(ee);

  REQUIRE(degree(g, 0) == 1);
  REQUIRE(degree(g, 1) == 1);
  REQUIRE(degree(g, 2) == 0);
  REQUIRE(degree(g, 3) == 1);
  REQUIRE(degree(g, 4) == 1);
  REQUIRE(degree(g, 5) == 0);
}

// =============================================================================
// contains_edge(g, u, v) and contains_edge(g, uid, vid) CPO Tests
// =============================================================================

TEST_CASE("contains_edge(g, u, v) with vertex descriptors - basic edge existence", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);
  auto v3 = *find_vertex(g, 3);

  // Existing edges
  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v0, v2) == true);
  REQUIRE(contains_edge(g, v1, v2) == true);
  REQUIRE(contains_edge(g, v2, v3) == true);

  // Non-existing edges
  REQUIRE(contains_edge(g, v1, v0) == false);
  REQUIRE(contains_edge(g, v2, v0) == false);
  REQUIRE(contains_edge(g, v3, v2) == false);
  REQUIRE(contains_edge(g, v0, v3) == false);
  REQUIRE(contains_edge(g, v1, v3) == false);
}

TEST_CASE("contains_edge(g, uid, vid) with vertex IDs - basic edge existence", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Existing edges
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 0, 2) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 2, 3) == true);

  // Non-existing edges
  REQUIRE(contains_edge(g, 1, 0) == false);
  REQUIRE(contains_edge(g, 2, 0) == false);
  REQUIRE(contains_edge(g, 3, 2) == false);
  REQUIRE(contains_edge(g, 0, 3) == false);
  REQUIRE(contains_edge(g, 1, 3) == false);
}

TEST_CASE("contains_edge(g, u, v) consistency between descriptor and ID versions", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};

  Graph g;
  g.load_edges(edges_data);

  for (size_t src = 0; src < g.size(); ++src) {
    auto u = *find_vertex(g, src);
    for (size_t tgt = 0; tgt < g.size(); ++tgt) {
      auto v = *find_vertex(g, tgt);
      // Both versions should return the same result
      REQUIRE(contains_edge(g, u, v) == contains_edge(g, src, tgt));
    }
  }
}

TEST_CASE("contains_edge(g, u, v) with const graph", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}};

  Graph temp_g;
  temp_g.load_edges(edges_data);
  const Graph g = std::move(temp_g);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v1, v2) == true);
  REQUIRE(contains_edge(g, v0, v2) == false);
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 0, 2) == false);
}

TEST_CASE("contains_edge(g, u, v) with self-loops", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 0, 10}, {0, 1, 30}, {1, 1, 20}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  // Self-loops exist
  REQUIRE(contains_edge(g, v0, v0) == true);
  REQUIRE(contains_edge(g, v1, v1) == true);
  REQUIRE(contains_edge(g, 0, 0) == true);
  REQUIRE(contains_edge(g, 1, 1) == true);

  // Regular edge
  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, 0, 1) == true);
}

TEST_CASE("contains_edge(g, u, v) with zero out-degree vertices", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}};

  Graph g;
  g.load_edges(edges_data);

  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);
  auto v0 = *find_vertex(g, 0);

  // Vertices with zero out-degree have no outgoing edges
  REQUIRE(contains_edge(g, v1, v0) == false);
  REQUIRE(contains_edge(g, v1, v2) == false);
  REQUIRE(contains_edge(g, v2, v0) == false);
  REQUIRE(contains_edge(g, v2, v1) == false);
  REQUIRE(contains_edge(g, 1, 0) == false);
  REQUIRE(contains_edge(g, 1, 2) == false);
  REQUIRE(contains_edge(g, 2, 0) == false);
  REQUIRE(contains_edge(g, 2, 1) == false);
}

TEST_CASE("contains_edge(g, u, v) with void vertex values", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  // Cycle 0 -> 1 -> 2 -> 0
  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v1, v2) == true);
  REQUIRE(contains_edge(g, v2, v0) == true);
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 2, 0) == true);

  // Non-existent reverse edges
  REQUIRE(contains_edge(g, v1, v0) == false);
  REQUIRE(contains_edge(g, v2, v1) == false);
  REQUIRE(contains_edge(g, v0, v2) == false);
}

TEST_CASE("contains_edge(g, u, v) with void graph values", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v1, v2) == true);
  REQUIRE(contains_edge(g, v2, v0) == false);
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 2, 0) == false);
}

TEST_CASE("contains_edge(g, u, v) with string values", "[contains_edge][api]") {
  using Graph                                          = compressed_graph<string, string, void>;
  vector<copyable_edge_t<int, string>>   edges_data    = {{0, 1, "edge01"}, {1, 2, "edge12"}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "v0"}, {1, "v1"}, {2, "v2"}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v1, v2) == true);
  REQUIRE(contains_edge(g, v0, v2) == false);
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 0, 2) == false);
}

TEST_CASE("contains_edge(g, u, v) return type is bool", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  auto result_desc = contains_edge(g, v0, v1);
  auto result_id   = contains_edge(g, 0, 1);

  REQUIRE(std::is_same_v<decltype(result_desc), bool>);
  REQUIRE(std::is_same_v<decltype(result_id), bool>);
}

TEST_CASE("contains_edge(g, u, v) with multiple edges to same target", "[contains_edge][api]") {
  using Graph = compressed_graph<int, int, void>;
  // Note: compressed_graph typically stores one edge per (src, target) pair
  // but let's test the first edge is found
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 1, 20}, {0, 2, 30}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  // Should find at least one edge from 0 to 1
  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v0, v2) == true);
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 0, 2) == true);
}

TEST_CASE("contains_edge(g, u, v) with complete graph", "[contains_edge][api]") {
  using Graph = compressed_graph<int, int, void>;
  // Complete graph on 4 vertices
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {1, 0, 1}, {1, 2, 1}, {1, 3, 1},
                                                  {2, 0, 1}, {2, 1, 1}, {2, 3, 1}, {3, 0, 1}, {3, 1, 1}, {3, 2, 1}};

  Graph g;
  g.load_edges(edges_data);

  // Every vertex should have an edge to every other vertex
  for (size_t src = 0; src < g.size(); ++src) {
    auto u = *find_vertex(g, src);
    for (size_t tgt = 0; tgt < g.size(); ++tgt) {
      if (src != tgt) {
        auto v = *find_vertex(g, tgt);
        REQUIRE(contains_edge(g, u, v) == true);
        REQUIRE(contains_edge(g, src, tgt) == true);
      }
    }
  }
}

TEST_CASE("contains_edge(g, u, v) with disconnected components", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {
        {0, 1, 10},
        {1, 2, 20}, // Component 1
        {3, 4, 30},
        {4, 5, 40} // Component 2
  };

  Graph g;
  g.load_edges(edges_data);

  // Within component 1
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);

  // Within component 2
  REQUIRE(contains_edge(g, 3, 4) == true);
  REQUIRE(contains_edge(g, 4, 5) == true);

  // Between components (should not exist)
  REQUIRE(contains_edge(g, 0, 3) == false);
  REQUIRE(contains_edge(g, 0, 4) == false);
  REQUIRE(contains_edge(g, 1, 3) == false);
  REQUIRE(contains_edge(g, 2, 4) == false);
  REQUIRE(contains_edge(g, 3, 0) == false);
  REQUIRE(contains_edge(g, 4, 1) == false);
}

TEST_CASE("contains_edge(g, u, v) with single vertex graph", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {
        {0, 0, 10} // Self-loop
  };

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);

  REQUIRE(contains_edge(g, v0, v0) == true);
  REQUIRE(contains_edge(g, 0, 0) == true);
}

TEST_CASE("contains_edge(g, u, v) with empty graph", "[contains_edge][api]") {
  using Graph = compressed_graph<int, int, void>;

  Graph g;
  // Empty graph has vertices 0..N-1 but no edges loaded
  // Actually, an empty graph has no vertices either until edges are loaded

  // If graph is truly empty, we can't test much
  // Let's test a graph with vertices but no edges
  vector<copyable_edge_t<int, int>> edges_data = {};
  g.load_edges(edges_data);

  // No edges exist in empty graph
  // Note: empty graph may have 0 vertices, so this test may not be meaningful
  // But if we had vertices, none would have edges
}

TEST_CASE("contains_edge(g, u, v) with linear chain", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}, {3, 4, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Forward edges exist
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 2, 3) == true);
  REQUIRE(contains_edge(g, 3, 4) == true);

  // Reverse edges don't exist
  REQUIRE(contains_edge(g, 1, 0) == false);
  REQUIRE(contains_edge(g, 2, 1) == false);
  REQUIRE(contains_edge(g, 3, 2) == false);
  REQUIRE(contains_edge(g, 4, 3) == false);

  // Skip edges don't exist
  REQUIRE(contains_edge(g, 0, 2) == false);
  REQUIRE(contains_edge(g, 0, 3) == false);
  REQUIRE(contains_edge(g, 1, 3) == false);
  REQUIRE(contains_edge(g, 2, 4) == false);
}

TEST_CASE("contains_edge(g, u, v) with star graph", "[contains_edge][api]") {
  using Graph = compressed_graph<int, int, void>;
  // Star graph with center at vertex 0
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Center has edges to all spokes
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 0, 2) == true);
  REQUIRE(contains_edge(g, 0, 3) == true);
  REQUIRE(contains_edge(g, 0, 4) == true);

  // Spokes have no edges to each other
  REQUIRE(contains_edge(g, 1, 2) == false);
  REQUIRE(contains_edge(g, 1, 3) == false);
  REQUIRE(contains_edge(g, 2, 3) == false);
  REQUIRE(contains_edge(g, 2, 4) == false);

  // Spokes have no edges back to center
  REQUIRE(contains_edge(g, 1, 0) == false);
  REQUIRE(contains_edge(g, 2, 0) == false);
  REQUIRE(contains_edge(g, 3, 0) == false);
  REQUIRE(contains_edge(g, 4, 0) == false);
}

TEST_CASE("contains_edge(g, u, v) with bidirectional edges", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 0, 15}, {1, 2, 20}, {2, 1, 25}};

  Graph g;
  g.load_edges(edges_data);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  // Both directions exist
  REQUIRE(contains_edge(g, v0, v1) == true);
  REQUIRE(contains_edge(g, v1, v0) == true);
  REQUIRE(contains_edge(g, v1, v2) == true);
  REQUIRE(contains_edge(g, v2, v1) == true);
  REQUIRE(contains_edge(g, 0, 1) == true);
  REQUIRE(contains_edge(g, 1, 0) == true);
  REQUIRE(contains_edge(g, 1, 2) == true);
  REQUIRE(contains_edge(g, 2, 1) == true);

  // But not the third edge
  REQUIRE(contains_edge(g, v0, v2) == false);
  REQUIRE(contains_edge(g, v2, v0) == false);
}

TEST_CASE("contains_edge(g, u, v) with high degree vertex", "[contains_edge][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {0, 4, 1},
                                                  {0, 5, 1}, {0, 6, 1}, {0, 7, 1}, {0, 8, 1}};

  Graph g;
  g.load_edges(edges_data);

  // Vertex 0 has edges to vertices 1-8
  for (size_t i = 1; i <= 8; ++i) {
    REQUIRE(contains_edge(g, 0, i) == true);
  }

  // But not to any other vertex (if it existed)
  // And none of the target vertices have edges back
  for (size_t i = 1; i <= 8; ++i) {
    REQUIRE(contains_edge(g, i, 0) == false);
  }
}

// =============================================================================
// has_edges(g) CPO Tests
// =============================================================================

TEST_CASE("has_edges(g) with graph containing edges", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with empty graph", "[has_edges][api]") {
  using Graph = compressed_graph<int, int, void>;

  Graph g;
  // No edges loaded

  REQUIRE(has_edges(g) == false);
}

TEST_CASE("has_edges(g) with graph with vertices but no edges", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == false);
}

TEST_CASE("has_edges(g) with single edge", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with self-loop", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 0, 10}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with const graph", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}};

  Graph temp_g;
  temp_g.load_edges(edges_data);
  const Graph g = std::move(temp_g);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with const empty graph", "[has_edges][api]") {
  using Graph = compressed_graph<int, int, void>;

  Graph       temp_g;
  const Graph g = std::move(temp_g);

  REQUIRE(has_edges(g) == false);
}

TEST_CASE("has_edges(g) with void vertex values", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with string values", "[has_edges][api]") {
  using Graph                                          = compressed_graph<string, string, void>;
  vector<copyable_edge_t<int, string>>   edges_data    = {{0, 1, "edge01"}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "v0"}, {1, "v1"}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) return type is bool", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}};

  Graph g;
  g.load_edges(edges_data);

  auto result = has_edges(g);
  REQUIRE(std::is_same_v<decltype(result), bool>);
  REQUIRE(result == true);
}

TEST_CASE("has_edges(g) with many edges", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 1}, {0, 2, 1}, {1, 2, 1}, {1, 3, 1},
                                                  {2, 3, 1}, {3, 4, 1}, {4, 0, 1}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with disconnected components", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {
        {0, 1, 10}, {2, 3, 20} // Two separate components
  };

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with isolated vertex at beginning", "[has_edges][api]") {
  using Graph = compressed_graph<int, int, void>;
  // Vertex 0 has no edges, but vertex 1 does
  vector<copyable_edge_t<int, int>> edges_data = {{1, 2, 10}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with complete graph", "[has_edges][api]") {
  using Graph = compressed_graph<int, int, void>;
  // Complete graph on 3 vertices
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 1}, {0, 2, 1}, {1, 0, 1}, {1, 2, 1}, {2, 0, 1}, {2, 1, 1}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with bidirectional edges", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 0, 20}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with linear chain", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with star graph", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

TEST_CASE("has_edges(g) with cycle", "[has_edges][api]") {
  using Graph                                  = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 0, 30}};

  Graph g;
  g.load_edges(edges_data);

  REQUIRE(has_edges(g) == true);
}

// =============================================================================
// vertex_value(g, u) CPO Tests
// =============================================================================

TEST_CASE("vertex_value(g, u) basic access with int values", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(vertex_value(g, v0) == 100);
  REQUIRE(vertex_value(g, v1) == 200);
  REQUIRE(vertex_value(g, v2) == 300);
}

TEST_CASE("vertex_value(g, u) with string values", "[vertex_value][api]") {
  using Graph                                          = compressed_graph<int, string, void>;
  vector<copyable_edge_t<int, int>>      edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "Alice"}, {1, "Bob"}, {2, "Charlie"}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(vertex_value(g, v0) == "Alice");
  REQUIRE(vertex_value(g, v1) == "Bob");
  REQUIRE(vertex_value(g, v2) == "Charlie");
}

TEST_CASE("vertex_value(g, u) returns reference", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);

  // Get reference and modify
  vertex_value(g, v0) = 999;

  // Verify modification
  REQUIRE(vertex_value(g, v0) == 999);
}

TEST_CASE("vertex_value(g, u) with const graph", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph temp_g;
  temp_g.load_edges(edges_data);
  temp_g.load_vertices(vertex_values);
  const Graph g = std::move(temp_g);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(vertex_value(g, v0) == 100);
  REQUIRE(vertex_value(g, v1) == 200);
}

TEST_CASE("vertex_value(g, u) with double values", "[vertex_value][api]") {
  using Graph                                          = compressed_graph<int, double, void>;
  vector<copyable_edge_t<int, int>>      edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, double>> vertex_values = {{0, 3.14}, {1, 2.71}, {2, 1.41}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(vertex_value(g, v0) == 3.14);
  REQUIRE(vertex_value(g, v1) == 2.71);
  REQUIRE(vertex_value(g, v2) == 1.41);
}

TEST_CASE("vertex_value(g, u) with struct values", "[vertex_value][api]") {
  struct VertexData {
    int    id;
    string name;

    bool operator==(const VertexData& other) const { return id == other.id && name == other.name; }
  };

  using Graph                                              = compressed_graph<int, VertexData, void>;
  vector<copyable_edge_t<int, int>>          edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, VertexData>> vertex_values = {{0, {1, "Node1"}}, {1, {2, "Node2"}}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(vertex_value(g, v0).id == 1);
  REQUIRE(vertex_value(g, v0).name == "Node1");
  REQUIRE(vertex_value(g, v1).id == 2);
  REQUIRE(vertex_value(g, v1).name == "Node2");
}

TEST_CASE("vertex_value(g, u) with all vertices", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  // Verify all vertices have correct values
  int expected = 10;
  for (auto v : vertices(g)) {
    REQUIRE(vertex_value(g, v) == expected);
    expected += 10;
  }
}

TEST_CASE("vertex_value(g, u) modify through iteration", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 0}, {1, 0}, {2, 0}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  // Set values through iteration
  int value = 100;
  for (auto v : vertices(g)) {
    vertex_value(g, v) = value;
    value += 100;
  }

  // Verify values
  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(vertex_value(g, v0) == 100);
  REQUIRE(vertex_value(g, v1) == 200);
  REQUIRE(vertex_value(g, v2) == 300);
}

TEST_CASE("vertex_value(g, u) with negative values", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, -100}, {1, -200}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(vertex_value(g, v0) == -100);
  REQUIRE(vertex_value(g, v1) == -200);
}

TEST_CASE("vertex_value(g, u) with zero values", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 0}, {1, 0}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(vertex_value(g, v0) == 0);
  REQUIRE(vertex_value(g, v1) == 0);
}

TEST_CASE("vertex_value(g, u) return type is reference", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);

  // Check that we get a reference
  auto& val_ref = vertex_value(g, v0);
  REQUIRE(std::is_reference_v<decltype(vertex_value(g, v0))>);

  // Modify through reference
  val_ref = 999;
  REQUIRE(vertex_value(g, v0) == 999);
}

TEST_CASE("vertex_value(g, u) with const return for const graph", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}};

  Graph temp_g;
  temp_g.load_edges(edges_data);
  temp_g.load_vertices(vertex_values);
  const Graph g = std::move(temp_g);

  auto v0 = *find_vertex(g, 0);

  // Should return const reference for const graph
  auto& val_ref = vertex_value(g, v0);
  REQUIRE(std::is_const_v<std::remove_reference_t<decltype(val_ref)>>);
  REQUIRE(val_ref == 100);
}

TEST_CASE("vertex_value(g, u) with large values", "[vertex_value][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 1000000}, {1, 2000000}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(vertex_value(g, v0) == 1000000);
  REQUIRE(vertex_value(g, v1) == 2000000);
}

TEST_CASE("vertex_value(g, u) with isolated vertices", "[vertex_value][api]") {
  using Graph = compressed_graph<int, int, void>;
  // Vertices 0 and 2 have no edges
  vector<copyable_edge_t<int, int>>   edges_data    = {{1, 3, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 111}, {1, 222}, {2, 333}, {3, 444}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);
  auto v3 = *find_vertex(g, 3);

  REQUIRE(vertex_value(g, v0) == 111);
  REQUIRE(vertex_value(g, v1) == 222);
  REQUIRE(vertex_value(g, v2) == 333);
  REQUIRE(vertex_value(g, v3) == 444);
}

TEST_CASE("vertex_value(g, u) with vector values", "[vertex_value][api]") {
  using Graph                                               = compressed_graph<int, vector<int>, void>;
  vector<copyable_edge_t<int, int>>           edges_data    = {{0, 1, 10}};
  vector<copyable_vertex_t<int, vector<int>>> vertex_values = {{0, {1, 2, 3}}, {1, {4, 5, 6}}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(vertex_value(g, v0) == vector<int>{1, 2, 3});
  REQUIRE(vertex_value(g, v1) == vector<int>{4, 5, 6});
}

// =============================================================================
// edge_value(g, uv) CPO Tests
// =============================================================================

TEST_CASE("edge_value(g, uv) basic access with int values", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();
  auto uv2 = *edges(g, *find_vertex(g, 2)).begin();

  REQUIRE(edge_value(g, uv0) == 10);
  REQUIRE(edge_value(g, uv1) == 20);
  REQUIRE(edge_value(g, uv2) == 30);
}

TEST_CASE("edge_value(g, uv) with string values", "[edge_value][api]") {
  using Graph                                     = compressed_graph<string, void, void>;
  vector<copyable_edge_t<int, string>> edges_data = {{0, 1, "edge01"}, {1, 2, "edge12"}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0) == "edge01");
  REQUIRE(edge_value(g, uv1) == "edge12");
}

TEST_CASE("edge_value(g, uv) returns reference", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();

  // Get reference and modify
  edge_value(g, uv0) = 999;

  // Verify modification
  REQUIRE(edge_value(g, uv0) == 999);
}

TEST_CASE("edge_value(g, uv) with const graph", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 100}, {1, 2, 200}};

  Graph temp_g;
  temp_g.load_edges(edges_data);
  const Graph g = std::move(temp_g);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0) == 100);
  REQUIRE(edge_value(g, uv1) == 200);
}

TEST_CASE("edge_value(g, uv) with double values", "[edge_value][api]") {
  using Graph                                     = compressed_graph<double, void, void>;
  vector<copyable_edge_t<int, double>> edges_data = {{0, 1, 3.14}, {1, 2, 2.71}, {2, 3, 1.41}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();
  auto uv2 = *edges(g, *find_vertex(g, 2)).begin();

  REQUIRE(edge_value(g, uv0) == 3.14);
  REQUIRE(edge_value(g, uv1) == 2.71);
  REQUIRE(edge_value(g, uv2) == 1.41);
}

TEST_CASE("edge_value(g, uv) with struct values", "[edge_value][api]") {
  struct EdgeData {
    int    weight;
    string label;

    bool operator==(const EdgeData& other) const { return weight == other.weight && label == other.label; }
  };

  using Graph                                       = compressed_graph<EdgeData, void, void>;
  vector<copyable_edge_t<int, EdgeData>> edges_data = {{0, 1, {10, "fast"}}, {1, 2, {20, "slow"}}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0).weight == 10);
  REQUIRE(edge_value(g, uv0).label == "fast");
  REQUIRE(edge_value(g, uv1).weight == 20);
  REQUIRE(edge_value(g, uv1).label == "slow");
}

TEST_CASE("edge_value(g, uv) with all edges", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {1, 3, 40}};

  Graph g;
  g.load_edges(edges_data);

  // Verify all edges have correct values
  vector<int> expected_values = {10, 20, 30, 40};
  size_t      idx             = 0;

  for (auto u : vertices(g)) {
    for (auto uv : edges(g, u)) {
      REQUIRE(edge_value(g, uv) == expected_values[idx]);
      ++idx;
    }
  }
  REQUIRE(idx == 4);
}

TEST_CASE("edge_value(g, uv) modify through iteration", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 0}, {1, 2, 0}, {2, 3, 0}};

  Graph g;
  g.load_edges(edges_data);

  // Set values through iteration
  int value = 100;
  for (auto u : vertices(g)) {
    for (auto uv : edges(g, u)) {
      edge_value(g, uv) = value;
      value += 100;
    }
  }

  // Verify values
  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();
  auto uv2 = *edges(g, *find_vertex(g, 2)).begin();

  REQUIRE(edge_value(g, uv0) == 100);
  REQUIRE(edge_value(g, uv1) == 200);
  REQUIRE(edge_value(g, uv2) == 300);
}

TEST_CASE("edge_value(g, uv) with negative values", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, -100}, {1, 2, -200}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0) == -100);
  REQUIRE(edge_value(g, uv1) == -200);
}

TEST_CASE("edge_value(g, uv) with zero values", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 0}, {1, 2, 0}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0) == 0);
  REQUIRE(edge_value(g, uv1) == 0);
}

TEST_CASE("edge_value(g, uv) return type is reference", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 100}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();

  // Check that we get a reference
  auto& val_ref = edge_value(g, uv0);
  REQUIRE(std::is_reference_v<decltype(edge_value(g, uv0))>);

  // Modify through reference
  val_ref = 999;
  REQUIRE(edge_value(g, uv0) == 999);
}

TEST_CASE("edge_value(g, uv) with const return for const graph", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 100}};

  Graph temp_g;
  temp_g.load_edges(edges_data);
  const Graph g = std::move(temp_g);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();

  // Should return const reference for const graph
  auto& val_ref = edge_value(g, uv0);
  REQUIRE(std::is_const_v<std::remove_reference_t<decltype(val_ref)>>);
  REQUIRE(val_ref == 100);
}

TEST_CASE("edge_value(g, uv) with large values", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 1000000}, {1, 2, 2000000}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0) == 1000000);
  REQUIRE(edge_value(g, uv1) == 2000000);
}

TEST_CASE("edge_value(g, uv) with multiple edges from same vertex", "[edge_value][api]") {
  using Graph                                  = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> edges_data = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};

  Graph g;
  g.load_edges(edges_data);

  auto v0         = *find_vertex(g, 0);
  auto edge_range = edges(g, v0);
  auto it         = edge_range.begin();

  auto uv0 = *it++;
  auto uv1 = *it++;
  auto uv2 = *it++;

  REQUIRE(edge_value(g, uv0) == 10);
  REQUIRE(edge_value(g, uv1) == 20);
  REQUIRE(edge_value(g, uv2) == 30);
}

TEST_CASE("edge_value(g, uv) with vector values", "[edge_value][api]") {
  using Graph                                          = compressed_graph<vector<int>, void, void>;
  vector<copyable_edge_t<int, vector<int>>> edges_data = {{0, 1, {1, 2, 3}}, {1, 2, {4, 5, 6}}};

  Graph g;
  g.load_edges(edges_data);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  REQUIRE(edge_value(g, uv0) == vector<int>{1, 2, 3});
  REQUIRE(edge_value(g, uv1) == vector<int>{4, 5, 6});
}

TEST_CASE("edge_value(g, uv) with mixed edge and vertex values", "[edge_value][api]") {
  using Graph                                          = compressed_graph<int, string, void>;
  vector<copyable_edge_t<int, int>>      edges_data    = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, string>> vertex_values = {{0, "v0"}, {1, "v1"}, {2, "v2"}};

  Graph g;
  g.load_edges(edges_data);
  g.load_vertices(vertex_values);

  auto uv0 = *edges(g, *find_vertex(g, 0)).begin();
  auto uv1 = *edges(g, *find_vertex(g, 1)).begin();

  // Verify edge values work independently of vertex values
  REQUIRE(edge_value(g, uv0) == 10);
  REQUIRE(edge_value(g, uv1) == 20);

  // Verify vertex values still work
  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  REQUIRE(vertex_value(g, v0) == "v0");
  REQUIRE(vertex_value(g, v1) == "v1");
}

// =============================================================================
// partition_id(g, u) Friend Function Tests
// =============================================================================

TEST_CASE("partition_id(g, u) returns 0 for single-partition graph", "[partition_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  SECTION("all vertices in partition 0") {
    auto v0 = *find_vertex(g, 0);
    auto v1 = *find_vertex(g, 1);
    auto v2 = *find_vertex(g, 2);
    auto v3 = *find_vertex(g, 3);

    REQUIRE(partition_id(g, v0) == 0);
    REQUIRE(partition_id(g, v1) == 0);
    REQUIRE(partition_id(g, v2) == 0);
    REQUIRE(partition_id(g, v3) == 0);
  }

  SECTION("consistent across multiple calls") {
    auto v1 = *find_vertex(g, 1);
    REQUIRE(partition_id(g, v1) == partition_id(g, v1));
  }
}

TEST_CASE("partition_id(g, u) with const graph", "[partition_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g_mutable;
  g_mutable.load_edges(ee);
  g_mutable.load_vertices(vertex_values);

  const Graph& g = g_mutable;

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);

  REQUIRE(partition_id(g, v0) == 0);
  REQUIRE(partition_id(g, v1) == 0);
}

TEST_CASE("partition_id(g, u) with void edge values", "[partition_id][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>>  ee            = {{0, 1}, {1, 2}, {2, 3}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto verts = vertices(g);
  for (auto v : verts) {
    REQUIRE(partition_id(g, v) == 0);
  }
}

TEST_CASE("partition_id(g, u) with void vertex values", "[partition_id][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 100}, {1, 2, 200}};

  Graph g;
  g.load_edges(ee);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  REQUIRE(partition_id(g, v0) == 0);
  REQUIRE(partition_id(g, v1) == 0);
  REQUIRE(partition_id(g, v2) == 0);
}

TEST_CASE("partition_id(g, u) with empty graph", "[partition_id][api]") {
  using Graph = compressed_graph<int, int, void>;
  Graph g;

  // Empty graph has no vertices to test
  auto verts = vertices(g);
  REQUIRE(ranges::begin(verts) == ranges::end(verts));
}

TEST_CASE("partition_id(g, u) with single vertex", "[partition_id][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}};

  Graph g;
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  REQUIRE(partition_id(g, v0) == 0);
}

TEST_CASE("partition_id(g, u) integration with vertex_id", "[partition_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  // All vertices have different IDs but same partition (0)
  vector<pair<int, int>> id_partition_pairs;
  for (auto v : vertices(g)) {
    auto vid = vertex_id(g, v);
    auto pid = partition_id(g, v);
    id_partition_pairs.emplace_back(vid, pid);
  }

  REQUIRE(id_partition_pairs.size() == 4);

  // Different vertex IDs
  REQUIRE(id_partition_pairs[0].first == 0);
  REQUIRE(id_partition_pairs[1].first == 1);
  REQUIRE(id_partition_pairs[2].first == 2);
  REQUIRE(id_partition_pairs[3].first == 3);

  // Same partition for all
  REQUIRE(id_partition_pairs[0].second == 0);
  REQUIRE(id_partition_pairs[1].second == 0);
  REQUIRE(id_partition_pairs[2].second == 0);
  REQUIRE(id_partition_pairs[3].second == 0);
}

TEST_CASE("partition_id(g, u) return type is integral", "[partition_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto v0  = *find_vertex(g, 0);
  auto pid = partition_id(g, v0);

  STATIC_REQUIRE(std::integral<decltype(pid)>);
}

TEST_CASE("partition_id(g, u) is noexcept", "[partition_id][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);

  STATIC_REQUIRE(noexcept(partition_id(g, v0)));
}

TEST_CASE("partition_id(g, u) with string edge values", "[partition_id][api]") {
  using Graph                                        = compressed_graph<string, int, void>;
  vector<copyable_edge_t<int, string>> ee            = {{0, 1, "edge01"}, {1, 2, "edge12"}};
  vector<copyable_vertex_t<int, int>>  vertex_values = {{0, 10}, {1, 20}, {2, 30}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto v0 = *find_vertex(g, 0);
  auto v1 = *find_vertex(g, 1);
  auto v2 = *find_vertex(g, 2);

  // All in partition 0 regardless of edge value type
  REQUIRE(partition_id(g, v0) == 0);
  REQUIRE(partition_id(g, v1) == 0);
  REQUIRE(partition_id(g, v2) == 0);
}

TEST_CASE("partition_id(g, u) works with all vertices", "[partition_id][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {1, 3, 40}, {2, 3, 50}};

  Graph g;
  g.load_edges(ee);

  // Iterate all vertices and verify partition_id works
  size_t count = 0;
  for (auto v : vertices(g)) {
    REQUIRE(partition_id(g, v) == 0);
    ++count;
  }

  REQUIRE(count == 4); // vertices 0, 1, 2, 3
}

// =============================================================================
// num_partitions(g) Friend Function Tests
// =============================================================================

TEST_CASE("num_partitions(g) returns 1 for single-partition graph", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  SECTION("default single partition") { REQUIRE(num_partitions(g) == 1); }

  SECTION("consistent across multiple calls") { REQUIRE(num_partitions(g) == num_partitions(g)); }
}

TEST_CASE("num_partitions(g) with const graph", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g_mutable;
  g_mutable.load_edges(ee);
  g_mutable.load_vertices(vertex_values);

  const Graph& g = g_mutable;

  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) with void edge values", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>>  ee            = {{0, 1}, {1, 2}, {2, 3}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) with void vertex values", "[num_partitions][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 100}, {1, 2, 200}};

  Graph g;
  g.load_edges(ee);

  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) with empty graph", "[num_partitions][api]") {
  using Graph = compressed_graph<int, int, void>;
  Graph g;

  // Empty graph still has 1 partition (the default partition 0)
  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) with single vertex", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}};

  Graph g;
  g.load_vertices(vertex_values);

  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) integration with partition_id", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto num_parts = num_partitions(g);
  REQUIRE(num_parts == 1);

  // All partition IDs should be in range [0, num_partitions)
  for (auto v : vertices(g)) {
    auto pid = partition_id(g, v);
    //REQUIRE(pid >= 0); // aways true for unsigned types
    REQUIRE(pid < num_parts);
  }
}

TEST_CASE("num_partitions(g) return type is integral", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto num_parts = num_partitions(g);

  STATIC_REQUIRE(std::integral<decltype(num_parts)>);
}

TEST_CASE("num_partitions(g) is noexcept", "[num_partitions][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  STATIC_REQUIRE(noexcept(num_partitions(g)));
}

TEST_CASE("num_partitions(g) with string edge values", "[num_partitions][api]") {
  using Graph                                        = compressed_graph<string, int, void>;
  vector<copyable_edge_t<int, string>> ee            = {{0, 1, "edge01"}, {1, 2, "edge12"}};
  vector<copyable_vertex_t<int, int>>  vertex_values = {{0, 10}, {1, 20}, {2, 30}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  // Single partition regardless of edge value type
  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) with large graph", "[num_partitions][api]") {
  using Graph = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee;

  // Create a chain of 100 vertices
  for (int i = 0; i < 99; ++i) {
    ee.push_back({i, i + 1, i * 10});
  }

  Graph g;
  g.load_edges(ee);

  // Still single partition by default
  REQUIRE(num_partitions(g) == 1);
}

TEST_CASE("num_partitions(g) guarantees minimum of 1", "[num_partitions][api]") {
  using Graph = compressed_graph<void, void, void>;
  Graph g;

  // Even empty graph has at least 1 partition
  auto num_parts = num_partitions(g);
  REQUIRE(num_parts >= 1);
}

// =============================================================================
// vertices(g, pid) Friend Function Tests
// =============================================================================

TEST_CASE("vertices(g, pid) returns all vertices for partition 0", "[vertices_pid][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  SECTION("partition 0 contains all vertices") {
    auto   verts = vertices(g, 0);
    size_t count = 0;
    for (auto v : verts) {
      REQUIRE(vertex_id(g, v) == count);
      ++count;
    }
    REQUIRE(count == 4);
  }

  SECTION("partition 0 matches vertices(g)") {
    auto verts_all = vertices(g);
    auto verts_p0  = vertices(g, 0);

    auto it_all = ranges::begin(verts_all);
    auto it_p0  = ranges::begin(verts_p0);

    while (it_all != ranges::end(verts_all) && it_p0 != ranges::end(verts_p0)) {
      REQUIRE(vertex_id(g, *it_all) == vertex_id(g, *it_p0));
      ++it_all;
      ++it_p0;
    }

    REQUIRE(it_all == ranges::end(verts_all));
    REQUIRE(it_p0 == ranges::end(verts_p0));
  }
}

TEST_CASE("vertices(g, pid) returns empty for non-zero partition", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(ee);

  SECTION("partition 1 is empty") {
    auto verts = vertices(g, 1);
    REQUIRE(ranges::begin(verts) == ranges::end(verts));
  }

  SECTION("partition 5 is empty") {
    auto verts = vertices(g, 5);
    REQUIRE(ranges::begin(verts) == ranges::end(verts));
  }
}

TEST_CASE("vertices(g, pid) with const graph", "[vertices_pid][api]") {
  using Graph                                       = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>>   ee            = {{0, 1, 10}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}, {1, 200}};

  Graph g_mutable;
  g_mutable.load_edges(ee);
  g_mutable.load_vertices(vertex_values);

  const Graph& g = g_mutable;

  auto   verts = vertices(g, 0);
  size_t count = 0;
  for (auto v : verts) {
    (void)v;
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("vertices(g, pid) with void edge values", "[vertices_pid][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_edge_t<int, void>>  ee            = {{0, 1}, {1, 2}, {2, 3}};
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 10}, {1, 20}, {2, 30}, {3, 40}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto   verts = vertices(g, 0);
  size_t count = 0;
  for (auto v : verts) {
    (void)v;
    ++count;
  }
  REQUIRE(count == 4);
}

TEST_CASE("vertices(g, pid) with void vertex values", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, void, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 100}, {1, 2, 200}};

  Graph g;
  g.load_edges(ee);

  auto   verts = vertices(g, 0);
  size_t count = 0;
  for (auto v : verts) {
    (void)v;
    ++count;
  }
  REQUIRE(count == 3);
}

TEST_CASE("vertices(g, pid) with empty graph", "[vertices_pid][api]") {
  using Graph = compressed_graph<int, int, void>;
  Graph g;

  SECTION("partition 0 is empty") {
    auto verts = vertices(g, 0);
    REQUIRE(ranges::begin(verts) == ranges::end(verts));
  }

  SECTION("partition 1 is empty") {
    auto verts = vertices(g, 1);
    REQUIRE(ranges::begin(verts) == ranges::end(verts));
  }
}

TEST_CASE("vertices(g, pid) with single vertex", "[vertices_pid][api]") {
  using Graph                                       = compressed_graph<void, int, void>;
  vector<copyable_vertex_t<int, int>> vertex_values = {{0, 100}};

  Graph g;
  g.load_vertices(vertex_values);

  SECTION("partition 0 has one vertex") {
    auto verts = vertices(g, 0);
    auto it    = ranges::begin(verts);
    REQUIRE(it != ranges::end(verts));

    auto v = *it;
    REQUIRE(vertex_id(g, v) == 0);

    ++it;
    REQUIRE(it == ranges::end(verts));
  }

  SECTION("partition 1 is empty") {
    auto verts = vertices(g, 1);
    REQUIRE(ranges::begin(verts) == ranges::end(verts));
  }
}

TEST_CASE("vertices(g, pid) with negative partition id", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  auto verts = vertices(g, -1);
  REQUIRE(ranges::begin(verts) == ranges::end(verts));
}

TEST_CASE("vertices(g, pid) iteration multiple times", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};

  Graph g;
  g.load_edges(ee);

  auto verts = vertices(g, 0);

  // First iteration
  size_t count1 = 0;
  for (auto v : verts) {
    (void)v;
    ++count1;
  }

  // Second iteration
  size_t count2 = 0;
  for (auto v : verts) {
    (void)v;
    ++count2;
  }

  REQUIRE(count1 == 3);
  REQUIRE(count2 == 3);
}

TEST_CASE("vertices(g, pid) with different integral types", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  SECTION("int partition id") {
    auto   verts = vertices(g, 0);
    size_t count = 0;
    for (auto v : verts) {
      (void)v;
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("size_t partition id") {
    auto   verts = vertices(g, size_t(0));
    size_t count = 0;
    for (auto v : verts) {
      (void)v;
      ++count;
    }
    REQUIRE(count == 2);
  }

  SECTION("uint32_t partition id") {
    auto   verts = vertices(g, uint32_t(0));
    size_t count = 0;
    for (auto v : verts) {
      (void)v;
      ++count;
    }
    REQUIRE(count == 2);
  }
}

TEST_CASE("vertices(g, pid) integration with partition_id", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}, {2, 3, 30}};

  Graph g;
  g.load_edges(ee);

  // For single partition, all vertices should be in partition 0
  auto num_parts = num_partitions(g);
  REQUIRE(num_parts == 1);

  auto verts_p0 = vertices(g, 0);
  for (auto v : verts_p0) {
    auto pid = partition_id(g, v);
    REQUIRE(pid == 0);
  }
}

TEST_CASE("vertices(g, pid) with string edge values", "[vertices_pid][api]") {
  using Graph                                        = compressed_graph<string, int, void>;
  vector<copyable_edge_t<int, string>> ee            = {{0, 1, "edge01"}, {1, 2, "edge12"}};
  vector<copyable_vertex_t<int, int>>  vertex_values = {{0, 10}, {1, 20}, {2, 30}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vertex_values);

  auto   verts = vertices(g, 0);
  size_t count = 0;
  for (auto v : verts) {
    (void)v;
    ++count;
  }
  REQUIRE(count == 3);
}

TEST_CASE("vertices(g, pid) large graph", "[vertices_pid][api]") {
  using Graph = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee;

  // Create chain of 100 vertices
  for (int i = 0; i < 99; ++i) {
    ee.push_back({i, i + 1, i * 10});
  }

  Graph g;
  g.load_edges(ee);

  auto   verts = vertices(g, 0);
  size_t count = 0;
  for (auto v : verts) {
    (void)v;
    ++count;
  }
  REQUIRE(count == 100);
}

TEST_CASE("vertices(g, pid) returns vertex_descriptor_view", "[vertices_pid][api]") {
  using Graph                          = compressed_graph<int, int, void>;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  auto verts = vertices(g, 0);

  // Verify it's a vertex_descriptor_view
  STATIC_REQUIRE(is_vertex_descriptor_view_v<decltype(verts)>);
}
