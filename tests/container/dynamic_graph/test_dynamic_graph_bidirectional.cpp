/**
 * @file test_dynamic_graph_bidirectional.cpp
 * @brief Comprehensive tests for dynamic_graph with Bidirectional=true
 *
 * Phase 8.5: Verifies that dynamic_graph<..., Bidirectional=true> satisfies
 * bidirectional_adjacency_list, populates in_edges during load_edges,
 * works with in_edges/in_degree CPOs, views (in_incidence, in_neighbors),
 * and that Bidirectional=false mode is completely unchanged.
 *
 * Tests use two trait types (vov and vol) as required by the plan.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vol_graph_traits.hpp>
#include <graph/graph.hpp>
#include <algorithm>
#include <map>
#include <numeric>
#include <set>
#include <vector>

using namespace graph::container;
using graph::copyable_vertex_t;

// ============================================================================
// Type aliases — Bidirectional graphs (Sourced=true, Bidirectional=true)
// ============================================================================

// vov: vector vertices + vector edges — EV=int for weighted edges
using bidir_vov_int =
      dynamic_graph<int, void, void, uint32_t, true, true,
                    vov_graph_traits<int, void, void, uint32_t, true, true>>;

// vov: void edge value (unweighted)
using bidir_vov_void =
      dynamic_graph<void, void, void, uint32_t, true, true,
                    vov_graph_traits<void, void, void, uint32_t, true, true>>;

// vol: vector vertices + list edges — EV=int for weighted edges
using bidir_vol_int =
      dynamic_graph<int, void, void, uint32_t, true, true,
                    vol_graph_traits<int, void, void, uint32_t, true, true>>;

// vol: void edge value (unweighted)
using bidir_vol_void =
      dynamic_graph<void, void, void, uint32_t, true, true,
                    vol_graph_traits<void, void, void, uint32_t, true, true>>;

// vov with vertex value
using bidir_vov_int_vv =
      dynamic_graph<int, int, void, uint32_t, true, true,
                    vov_graph_traits<int, int, void, uint32_t, true, true>>;

// ============================================================================
// Non-bidirectional baselines for regression comparison
// ============================================================================

using nonbidir_vov =
      dynamic_graph<int, void, void, uint32_t, true, false,
                    vov_graph_traits<int, void, void, uint32_t, true, false>>;

using nonbidir_vol =
      dynamic_graph<int, void, void, uint32_t, true, false,
                    vol_graph_traits<int, void, void, uint32_t, true, false>>;

// ============================================================================
// CPOs in scope
// ============================================================================
using graph::adj_list::vertices;
using graph::adj_list::num_vertices;
using graph::adj_list::find_vertex;
using graph::adj_list::vertex_id;
using graph::adj_list::edges;
using graph::adj_list::degree;
using graph::adj_list::target_id;
using graph::adj_list::source_id;
using graph::adj_list::edge_value;
using graph::adj_list::in_edges;
using graph::adj_list::in_degree;
using graph::adj_list::find_in_edge;
using graph::adj_list::contains_in_edge;
using graph::adj_list::find_vertex_edge;
using graph::adj_list::contains_edge;

// ============================================================================
// Helper: build a small directed graph for testing
//
//   0 --10--> 1 --20--> 2
//   |                    ^
//   +--------30----------+
//
// Edges: (0,1,10), (0,2,30), (1,2,20)
// Expected in_edges:
//   vertex 0: none
//   vertex 1: {from 0, weight 10}
//   vertex 2: {from 0, weight 30}, {from 1, weight 20}
// ============================================================================

struct test_edge {
  uint32_t source_id;
  uint32_t target_id;
  int      value;
};

static const std::vector<test_edge> triangle_edges = {
      {0, 1, 10},
      {0, 2, 30},
      {1, 2, 20},
};

template <typename G>
G make_triangle_graph() {
  G g;
  g.load_edges(triangle_edges, std::identity{});
  return g;
}

// ============================================================================
// 1. Concept satisfaction
// ============================================================================

TEST_CASE("bidir dynamic_graph models bidirectional_adjacency_list",
          "[dynamic_graph][bidirectional][concept]") {

  SECTION("vov Bidirectional=true satisfies concept") {
    static_assert(graph::adj_list::bidirectional_adjacency_list<bidir_vov_int>,
                  "bidir_vov_int must model bidirectional_adjacency_list");
    static_assert(graph::adj_list::index_bidirectional_adjacency_list<bidir_vov_int>,
                  "bidir_vov_int must model index_bidirectional_adjacency_list");
  }

  SECTION("vol Bidirectional=true satisfies concept") {
    static_assert(graph::adj_list::bidirectional_adjacency_list<bidir_vol_int>,
                  "bidir_vol_int must model bidirectional_adjacency_list");
    static_assert(graph::adj_list::index_bidirectional_adjacency_list<bidir_vol_int>,
                  "bidir_vol_int must model index_bidirectional_adjacency_list");
  }

  SECTION("void edge value Bidirectional=true satisfies concept") {
    static_assert(graph::adj_list::bidirectional_adjacency_list<bidir_vov_void>);
    static_assert(graph::adj_list::bidirectional_adjacency_list<bidir_vol_void>);
  }

  SECTION("graph namespace re-exports") {
    static_assert(graph::bidirectional_adjacency_list<bidir_vov_int>);
    static_assert(graph::index_bidirectional_adjacency_list<bidir_vov_int>);
  }
}

// ============================================================================
// 2. Non-bidirectional unchanged (no regressions)
// ============================================================================

TEST_CASE("non-bidir dynamic_graph does NOT model bidirectional_adjacency_list",
          "[dynamic_graph][bidirectional][concept][nonbidir]") {

  static_assert(!graph::adj_list::bidirectional_adjacency_list<nonbidir_vov>,
                "Bidirectional=false must not model bidirectional_adjacency_list");
  static_assert(!graph::adj_list::bidirectional_adjacency_list<nonbidir_vol>,
                "Bidirectional=false must not model bidirectional_adjacency_list");
}

TEST_CASE("non-bidir dynamic_graph works identically to before",
          "[dynamic_graph][bidirectional][nonbidir]") {
  nonbidir_vov g;
  g.load_edges(triangle_edges, std::identity{});

  REQUIRE(num_vertices(g) == 3);

  // Outgoing edges work as usual
  size_t edge_count = 0;
  for (auto v : vertices(g)) {
    for ([[maybe_unused]] auto e : edges(g, v)) {
      ++edge_count;
    }
  }
  REQUIRE(edge_count == 3);

  // source_id works (Sourced=true)
  for (auto v : vertices(g)) {
    auto uid = vertex_id(g, v);
    for (auto e : edges(g, v)) {
      REQUIRE(source_id(g, e) == uid);
    }
  }
}

// ============================================================================
// 3. Basic bidirectional graph construction
// ============================================================================

TEMPLATE_TEST_CASE("bidir basic construction and in_edges",
                   "[dynamic_graph][bidirectional][in_edges]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  REQUIRE(num_vertices(g) == 3);

  SECTION("vertex 0 has no incoming edges") {
    auto uid = uint32_t{0};
    auto u   = *find_vertex(g, uid);
    size_t count = 0;
    for ([[maybe_unused]] auto ie : in_edges(g, u)) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("vertex 1 has 1 incoming edge from vertex 0") {
    auto uid = uint32_t{1};
    auto u   = *find_vertex(g, uid);
    std::vector<uint32_t> sources;
    for (auto ie : in_edges(g, u)) {
      sources.push_back(source_id(g, ie));
    }
    REQUIRE(sources.size() == 1);
    REQUIRE(sources[0] == 0);
  }

  SECTION("vertex 2 has 2 incoming edges from vertices 0 and 1") {
    auto uid = uint32_t{2};
    auto u   = *find_vertex(g, uid);
    std::set<uint32_t> sources;
    for (auto ie : in_edges(g, u)) {
      sources.insert(source_id(g, ie));
    }
    REQUIRE(sources.size() == 2);
    REQUIRE(sources.count(0) == 1);
    REQUIRE(sources.count(1) == 1);
  }
}

// ============================================================================
// 4. in_edges by vertex id
// ============================================================================

TEMPLATE_TEST_CASE("bidir in_edges by vertex id",
                   "[dynamic_graph][bidirectional][in_edges][uid]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  auto uid = uint32_t{2};
  std::set<uint32_t> sources;
  for (auto ie : in_edges(g, uid)) {
    sources.insert(source_id(g, ie));
  }
  REQUIRE(sources.size() == 2);
  REQUIRE(sources.count(0) == 1);
  REQUIRE(sources.count(1) == 1);
}

// ============================================================================
// 5. in_degree matches expected
// ============================================================================

TEMPLATE_TEST_CASE("bidir in_degree matches expected",
                   "[dynamic_graph][bidirectional][in_degree]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  SECTION("in_degree by vertex descriptor") {
    std::vector<size_t> expected_in_deg = {0, 1, 2};
    for (auto v : vertices(g)) {
      auto uid = vertex_id(g, v);
      REQUIRE(in_degree(g, v) == expected_in_deg[uid]);
    }
  }

  SECTION("in_degree by vertex id") {
    REQUIRE(in_degree(g, uint32_t{0}) == 0);
    REQUIRE(in_degree(g, uint32_t{1}) == 1);
    REQUIRE(in_degree(g, uint32_t{2}) == 2);
  }
}

// ============================================================================
// 6. source_id and target_id on in_edges
// ============================================================================

TEMPLATE_TEST_CASE("bidir in_edges carry correct source_id and target_id",
                   "[dynamic_graph][bidirectional][source_id][target_id]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  // For each vertex, the in_edges should have target_id == vertex's own id
  // and source_id == the origin of the original forward edge
  for (auto v : vertices(g)) {
    auto uid = vertex_id(g, v);
    for (auto ie : in_edges(g, v)) {
      REQUIRE(target_id(g, ie) == uid);
      // source_id should be a valid vertex
      auto sid = source_id(g, ie);
      REQUIRE(sid < num_vertices(g));
      REQUIRE(sid != uid); // no self-loops in our test graph
    }
  }
}

// ============================================================================
// 7. edge_value on in_edges (weighted graph)
// ============================================================================

TEST_CASE("bidir in_edges carry correct edge values",
          "[dynamic_graph][bidirectional][edge_value]") {
  auto g = make_triangle_graph<bidir_vov_int>();

  // Build expected: for each (src, tgt) -> weight
  std::map<std::pair<uint32_t, uint32_t>, int> expected;
  for (auto& e : triangle_edges) {
    expected[{e.source_id, e.target_id}] = e.value;
  }

  // Check in_edges carry the same values
  for (auto v : vertices(g)) {
    auto uid = vertex_id(g, v);
    for (auto ie : in_edges(g, v)) {
      auto sid = source_id(g, ie);
      auto key = std::pair{sid, uid};
      REQUIRE(expected.count(key) == 1);
      REQUIRE(edge_value(g, ie) == expected[key]);
    }
  }
}

// ============================================================================
// 8. void edge value (unweighted bidir)
// ============================================================================

TEST_CASE("bidir void edge value works (unweighted)",
          "[dynamic_graph][bidirectional][void_ev]") {
  struct edge_data {
    uint32_t source_id;
    uint32_t target_id;
  };
  std::vector<edge_data> edge_list = {{0, 1}, {0, 2}, {1, 2}};

  bidir_vov_void g;
  g.load_edges(edge_list, std::identity{});

  REQUIRE(num_vertices(g) == 3);
  REQUIRE(in_degree(g, uint32_t{0}) == 0);
  REQUIRE(in_degree(g, uint32_t{1}) == 1);
  REQUIRE(in_degree(g, uint32_t{2}) == 2);

  // source_id still works on in_edges
  auto u2 = *find_vertex(g, uint32_t{2});
  std::set<uint32_t> sources;
  for (auto ie : in_edges(g, u2)) {
    sources.insert(source_id(g, ie));
  }
  REQUIRE(sources == std::set<uint32_t>{0, 1});
}

// ============================================================================
// 9. find_in_edge and contains_in_edge
// ============================================================================

TEMPLATE_TEST_CASE("bidir find_in_edge and contains_in_edge",
                   "[dynamic_graph][bidirectional][find_in_edge]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  SECTION("contains_in_edge for existing edges") {
    // Edge (0,1) exists => vertex 1 should contain in_edge from 0
    REQUIRE(contains_in_edge(g, uint32_t{1}, uint32_t{0}) == true);
    // Edge (0,2) exists => vertex 2 should contain in_edge from 0
    REQUIRE(contains_in_edge(g, uint32_t{2}, uint32_t{0}) == true);
    // Edge (1,2) exists => vertex 2 should contain in_edge from 1
    REQUIRE(contains_in_edge(g, uint32_t{2}, uint32_t{1}) == true);
  }

  SECTION("contains_in_edge for non-existing edges") {
    // No edge (1,0) => vertex 0 should NOT contain in_edge from 1
    REQUIRE(contains_in_edge(g, uint32_t{0}, uint32_t{1}) == false);
    // No edge (2,0) => vertex 0 should NOT contain in_edge from 2
    REQUIRE(contains_in_edge(g, uint32_t{0}, uint32_t{2}) == false);
    // No edge (2,1) => vertex 1 should NOT contain in_edge from 2
    REQUIRE(contains_in_edge(g, uint32_t{1}, uint32_t{2}) == false);
  }

  SECTION("find_in_edge returns valid edge for existing edge") {
    auto ie = find_in_edge(g, uint32_t{2}, uint32_t{0});
    REQUIRE(source_id(g, ie) == 0);
    REQUIRE(target_id(g, ie) == 2);
  }

  SECTION("find_in_edge for a different existing edge") {
    auto ie = find_in_edge(g, uint32_t{2}, uint32_t{1});
    REQUIRE(source_id(g, ie) == 1);
    REQUIRE(target_id(g, ie) == 2);
  }
}

// ============================================================================
// 10. Const graph access
// ============================================================================

TEST_CASE("bidir in_edges on const graph",
          "[dynamic_graph][bidirectional][const]") {
  auto g = make_triangle_graph<bidir_vov_int>();

  const auto& cg = g;

  // in_edges on const graph
  auto u2 = *find_vertex(cg, uint32_t{2});
  size_t count = 0;
  for ([[maybe_unused]] auto ie : in_edges(cg, u2)) {
    ++count;
  }
  REQUIRE(count == 2);

  // in_degree on const graph
  REQUIRE(in_degree(cg, u2) == 2);
}

// ============================================================================
// 11. Copy and move semantics
// ============================================================================

TEST_CASE("bidir copy preserves reverse adjacency",
          "[dynamic_graph][bidirectional][copy]") {
  auto g1 = make_triangle_graph<bidir_vov_int>();

  // Copy constructor
  bidir_vov_int g2 = g1;

  REQUIRE(num_vertices(g2) == 3);
  REQUIRE(in_degree(g2, uint32_t{0}) == 0);
  REQUIRE(in_degree(g2, uint32_t{1}) == 1);
  REQUIRE(in_degree(g2, uint32_t{2}) == 2);

  // Verify source_ids survive the copy
  auto u2 = *find_vertex(g2, uint32_t{2});
  std::set<uint32_t> sources;
  for (auto ie : in_edges(g2, u2)) {
    sources.insert(source_id(g2, ie));
  }
  REQUIRE(sources == std::set<uint32_t>{0, 1});
}

TEST_CASE("bidir move preserves reverse adjacency",
          "[dynamic_graph][bidirectional][move]") {
  auto g1 = make_triangle_graph<bidir_vov_int>();

  // Move constructor
  bidir_vov_int g2 = std::move(g1);

  REQUIRE(num_vertices(g2) == 3);
  REQUIRE(in_degree(g2, uint32_t{0}) == 0);
  REQUIRE(in_degree(g2, uint32_t{1}) == 1);
  REQUIRE(in_degree(g2, uint32_t{2}) == 2);

  auto u2 = *find_vertex(g2, uint32_t{2});
  std::set<uint32_t> sources;
  for (auto ie : in_edges(g2, u2)) {
    sources.insert(source_id(g2, ie));
  }
  REQUIRE(sources == std::set<uint32_t>{0, 1});
}

TEST_CASE("bidir copy assignment preserves reverse adjacency",
          "[dynamic_graph][bidirectional][copy_assign]") {
  auto g1 = make_triangle_graph<bidir_vov_int>();
  bidir_vov_int g2;

  g2 = g1;

  REQUIRE(num_vertices(g2) == 3);
  REQUIRE(in_degree(g2, uint32_t{2}) == 2);
}

TEST_CASE("bidir move assignment preserves reverse adjacency",
          "[dynamic_graph][bidirectional][move_assign]") {
  auto g1 = make_triangle_graph<bidir_vov_int>();
  bidir_vov_int g2;

  g2 = std::move(g1);

  REQUIRE(num_vertices(g2) == 3);
  REQUIRE(in_degree(g2, uint32_t{2}) == 2);
}

// ============================================================================
// 12. Initializer-list construction
// ============================================================================

TEST_CASE("bidir initializer_list construction",
          "[dynamic_graph][bidirectional][init_list]") {
  // Sourced + EV=int: initializer_list<copyable_edge<VId, EV>>
  bidir_vov_int g({{0, 1, 10}, {0, 2, 30}, {1, 2, 20}});

  REQUIRE(num_vertices(g) == 3);
  REQUIRE(in_degree(g, uint32_t{0}) == 0);
  REQUIRE(in_degree(g, uint32_t{1}) == 1);
  REQUIRE(in_degree(g, uint32_t{2}) == 2);

  // Verify source_ids
  auto u1 = *find_vertex(g, uint32_t{1});
  for (auto ie : in_edges(g, u1)) {
    REQUIRE(source_id(g, ie) == 0);
    REQUIRE(edge_value(g, ie) == 10);
  }
}

// ============================================================================
// 13. Larger graph — fan-in topology
// ============================================================================

TEST_CASE("bidir fan-in topology",
          "[dynamic_graph][bidirectional][fan_in]") {
  // All vertices 0..4 point to vertex 5
  std::vector<test_edge> fan_edges;
  for (uint32_t i = 0; i < 5; ++i) {
    fan_edges.push_back({i, 5, static_cast<int>(i * 10)});
  }

  bidir_vov_int g;
  g.load_edges(fan_edges, std::identity{});

  REQUIRE(num_vertices(g) == 6);

  // vertex 5 should have 5 incoming edges
  REQUIRE(in_degree(g, uint32_t{5}) == 5);

  // All other vertices have 0 incoming edges
  for (uint32_t i = 0; i < 5; ++i) {
    REQUIRE(in_degree(g, i) == 0);
  }

  // Check all source_ids for vertex 5
  auto u5 = *find_vertex(g, uint32_t{5});
  std::set<uint32_t> sources;
  for (auto ie : in_edges(g, u5)) {
    sources.insert(source_id(g, ie));
  }
  REQUIRE(sources == std::set<uint32_t>{0, 1, 2, 3, 4});
}

// ============================================================================
// 14. Self-loop handling
// ============================================================================

TEST_CASE("bidir self-loop appears in both edges and in_edges",
          "[dynamic_graph][bidirectional][self_loop]") {
  struct edge_data {
    uint32_t source_id;
    uint32_t target_id;
  };
  std::vector<edge_data> el = {{0, 0}, {0, 1}};

  bidir_vov_void g;
  g.load_edges(el, std::identity{});

  REQUIRE(num_vertices(g) == 2);

  // vertex 0: out-degree=2 (self-loop + edge to 1), in-degree=1 (self-loop)
  REQUIRE(degree(g, uint32_t{0}) == 2);
  REQUIRE(in_degree(g, uint32_t{0}) == 1);

  // The in-edge on vertex 0 should have source_id==0
  auto u0 = *find_vertex(g, uint32_t{0});
  for (auto ie : in_edges(g, u0)) {
    REQUIRE(source_id(g, ie) == 0);
  }

  // vertex 1: out-degree=0, in-degree=1 (from 0)
  REQUIRE(degree(g, uint32_t{1}) == 0);
  REQUIRE(in_degree(g, uint32_t{1}) == 1);
}

// ============================================================================
// 15. Forward-reverse consistency
// ============================================================================

TEMPLATE_TEST_CASE("bidir forward-reverse consistency",
                   "[dynamic_graph][bidirectional][consistency]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  // For every forward edge (u -> v), there must be an in_edge on vertex v
  // with source_id == u
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    for (auto e : edges(g, u)) {
      auto tid = target_id(g, e);
      // vertex tid should have an in_edge from uid
      REQUIRE(contains_in_edge(g, tid, uid));
    }
  }

  // For every in_edge on vertex v with source_id == u,
  // vertex u should have a forward edge to v
  for (auto v : vertices(g)) {
    auto vid = vertex_id(g, v);
    for (auto ie : in_edges(g, v)) {
      auto sid = source_id(g, ie);
      // vertex sid should have a forward edge to vid
      REQUIRE(contains_edge(g, sid, vid));
    }
  }
}

// ============================================================================
// 16. Total in_degree == total out_degree == edge count
// ============================================================================

TEMPLATE_TEST_CASE("bidir total in_degree equals total out_degree",
                   "[dynamic_graph][bidirectional][degree_sum]",
                   bidir_vov_int, bidir_vol_int) {
  auto g = make_triangle_graph<TestType>();

  size_t total_out = 0, total_in = 0;
  for (auto v : vertices(g)) {
    total_out += degree(g, v);
    total_in += in_degree(g, v);
  }
  REQUIRE(total_out == total_in);
  REQUIRE(total_out == 3); // 3 edges
}

// ============================================================================
// 17. Views integration — in_incidence
// ============================================================================

TEST_CASE("bidir in_incidence view",
          "[dynamic_graph][bidirectional][views][in_incidence]") {
  auto g = make_triangle_graph<bidir_vov_int>();

  SECTION("in_incidence by vertex id") {
    // vertex 2 has in-edges from 0 and 1
    std::set<uint32_t> sources;
    for (auto [sid, ie] : graph::views::in_incidence(g, uint32_t{2})) {
      sources.insert(static_cast<uint32_t>(sid));
    }
    REQUIRE(sources == std::set<uint32_t>{0, 1});
  }

  SECTION("in_incidence with edge value function") {
    std::vector<int> weights;
    for (auto [sid, ie] : graph::views::in_incidence(g, uint32_t{2})) {
      weights.push_back(edge_value(g, ie));
    }
    std::sort(weights.begin(), weights.end());
    REQUIRE(weights == std::vector<int>{20, 30});
  }

  SECTION("in_incidence on vertex with no in-edges") {
    size_t count = 0;
    for ([[maybe_unused]] auto entry : graph::views::in_incidence(g, uint32_t{0})) {
      ++count;
    }
    REQUIRE(count == 0);
  }
}

// ============================================================================
// 18. Views integration — in_neighbors
// ============================================================================

TEST_CASE("bidir in_neighbors view",
          "[dynamic_graph][bidirectional][views][in_neighbors]") {
  auto g = make_triangle_graph<bidir_vov_int>();

  SECTION("in_neighbors by vertex id") {
    // vertex 2 has in-neighbors 0 and 1
    std::set<uint32_t> nbrs;
    for (auto [nid, nv] : graph::views::in_neighbors(g, uint32_t{2})) {
      nbrs.insert(static_cast<uint32_t>(nid));
    }
    REQUIRE(nbrs == std::set<uint32_t>{0, 1});
  }

  SECTION("in_neighbors on vertex with no in-edges") {
    size_t count = 0;
    for ([[maybe_unused]] auto entry : graph::views::in_neighbors(g, uint32_t{0})) {
      ++count;
    }
    REQUIRE(count == 0);
  }
}

// ============================================================================
// 19. basic_in_incidence and basic_in_neighbors
// ============================================================================

TEST_CASE("bidir basic_in_incidence view",
          "[dynamic_graph][bidirectional][views][basic_in_incidence]") {
  auto g = make_triangle_graph<bidir_vov_int>();

  std::set<uint32_t> sources;
  for (auto [sid] : graph::views::basic_in_incidence(g, uint32_t{2})) {
    sources.insert(static_cast<uint32_t>(sid));
  }
  REQUIRE(sources == std::set<uint32_t>{0, 1});
}

TEST_CASE("bidir basic_in_neighbors view",
          "[dynamic_graph][bidirectional][views][basic_in_neighbors]") {
  auto g = make_triangle_graph<bidir_vov_int>();

  std::set<uint32_t> nbrs;
  for (auto [nid] : graph::views::basic_in_neighbors(g, uint32_t{2})) {
    nbrs.insert(static_cast<uint32_t>(nid));
  }
  REQUIRE(nbrs == std::set<uint32_t>{0, 1});
}

// ============================================================================
// 20. Vertex value type with bidirectional
// ============================================================================

TEST_CASE("bidir with vertex values",
          "[dynamic_graph][bidirectional][vertex_value]") {
  bidir_vov_int_vv g;

  // Load vertices with values, then edges
  using vertex_data = copyable_vertex_t<uint32_t, int>;
  std::vector<vertex_data> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv, std::identity{});
  g.load_edges(triangle_edges, std::identity{}, 3);

  REQUIRE(num_vertices(g) == 3);

  // Vertex values accessible
  using graph::adj_list::vertex_value;
  auto u0 = *find_vertex(g, uint32_t{0});
  REQUIRE(vertex_value(g, u0) == 100);

  // in_edges work alongside vertex values
  REQUIRE(in_degree(g, uint32_t{2}) == 2);
}

// ============================================================================
// 21. Empty graph
// ============================================================================

TEST_CASE("bidir empty graph",
          "[dynamic_graph][bidirectional][empty]") {
  bidir_vov_int g;

  REQUIRE(num_vertices(g) == 0);
  // No crashes, just an empty graph
}

// ============================================================================
// 22. Single vertex, no edges
// ============================================================================

TEST_CASE("bidir single vertex no edges",
          "[dynamic_graph][bidirectional][single_vertex]") {
  bidir_vov_int g;
  g.resize_vertices(1);

  REQUIRE(num_vertices(g) == 1);
  auto u0 = *find_vertex(g, uint32_t{0});
  REQUIRE(in_degree(g, u0) == 0);
  REQUIRE(degree(g, u0) == 0);
}

// ============================================================================
// 23. Clear resets everything
// ============================================================================

TEST_CASE("bidir clear resets in_edges",
          "[dynamic_graph][bidirectional][clear]") {
  auto g = make_triangle_graph<bidir_vov_int>();
  REQUIRE(num_vertices(g) == 3);
  REQUIRE(in_degree(g, uint32_t{2}) == 2);

  g.clear();
  REQUIRE(num_vertices(g) == 0);
}

// ============================================================================
// 24. Multiple load_edges calls accumulate correctly
// ============================================================================

TEST_CASE("bidir multiple load_edges accumulate",
          "[dynamic_graph][bidirectional][load_edges][accumulate]") {
  bidir_vov_int g;

  // First batch: (0,1,10)
  std::vector<test_edge> batch1 = {{0, 1, 10}};
  g.load_edges(batch1, std::identity{});

  REQUIRE(num_vertices(g) == 2);
  REQUIRE(in_degree(g, uint32_t{1}) == 1);

  // Second batch: (0,2,30), (1,2,20)
  std::vector<test_edge> batch2 = {{0, 2, 30}, {1, 2, 20}};
  g.load_edges(batch2, std::identity{});

  REQUIRE(num_vertices(g) == 3);
  REQUIRE(in_degree(g, uint32_t{2}) == 2);

  // Original in_edges still present
  REQUIRE(in_degree(g, uint32_t{1}) == 1);
}

// ============================================================================
// 25. Explicit vertex_count in load_edges
// ============================================================================

TEST_CASE("bidir load_edges with explicit vertex_count",
          "[dynamic_graph][bidirectional][load_edges][vertex_count]") {
  bidir_vov_int g;
  g.load_edges(triangle_edges, std::identity{}, /*vertex_count=*/5);

  // Should have 5 vertices (some with no edges)
  REQUIRE(num_vertices(g) == 5);
  REQUIRE(in_degree(g, uint32_t{0}) == 0);
  REQUIRE(in_degree(g, uint32_t{1}) == 1);
  REQUIRE(in_degree(g, uint32_t{2}) == 2);
  REQUIRE(in_degree(g, uint32_t{3}) == 0);
  REQUIRE(in_degree(g, uint32_t{4}) == 0);
}

// ============================================================================
// 26. Dense graph — complete K4
// ============================================================================

TEST_CASE("bidir complete graph K4",
          "[dynamic_graph][bidirectional][complete]") {
  // Complete directed graph on 4 vertices
  std::vector<test_edge> k4_edges;
  int w = 1;
  for (uint32_t i = 0; i < 4; ++i) {
    for (uint32_t j = 0; j < 4; ++j) {
      if (i != j) {
        k4_edges.push_back({i, j, w++});
      }
    }
  }

  bidir_vov_int g;
  g.load_edges(k4_edges, std::identity{});

  REQUIRE(num_vertices(g) == 4);

  // Each vertex: out-degree=3, in-degree=3
  for (auto v : vertices(g)) {
    REQUIRE(degree(g, v) == 3);
    REQUIRE(in_degree(g, v) == 3);
  }

  // Total edges: 12 forward, 12 reverse
  size_t total_out = 0, total_in = 0;
  for (auto v : vertices(g)) {
    total_out += degree(g, v);
    total_in += in_degree(g, v);
  }
  REQUIRE(total_out == 12);
  REQUIRE(total_in == 12);
}

// ============================================================================
// 27. vol trait type works identically to vov
// ============================================================================

TEST_CASE("bidir vol trait type works",
          "[dynamic_graph][bidirectional][vol]") {
  bidir_vol_int g({{0, 1, 10}, {0, 2, 30}, {1, 2, 20}});

  REQUIRE(num_vertices(g) == 3);
  REQUIRE(in_degree(g, uint32_t{0}) == 0);
  REQUIRE(in_degree(g, uint32_t{1}) == 1);
  REQUIRE(in_degree(g, uint32_t{2}) == 2);

  // edge values preserved
  auto u2 = *find_vertex(g, uint32_t{2});
  std::set<int> weights;
  for (auto ie : in_edges(g, u2)) {
    weights.insert(edge_value(g, ie));
  }
  REQUIRE(weights == std::set<int>{20, 30});
}
