/**
 * @file test_dijkstra_indexed_heap.cpp
 * @brief Phase 2 tests for the indexed d-ary heap path of dijkstra_shortest_paths.
 *
 * These tests:
 *   1. Re-run core Dijkstra scenarios with `use_indexed_dary_heap<>` and
 *      assert identical distances/predecessors as the default-heap path.
 *   2. Audit visitor call counts: examine, finish, edge-relaxed, and
 *      edge-not-relaxed events must match between the two heap paths
 *      (Phase 2.3 visitor-semantics audit).
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include "../common/map_graph_fixtures.hpp"
#include <graph/adj_list/vertex_property_map.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/mov_graph_traits.hpp>

#include <string>
#include <vector>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

namespace {

// Visitor that records exact call counts for every event Dijkstra fires.
struct CountingVisitor {
  int initialize = 0;
  int discover   = 0;
  int examine    = 0;
  int finish     = 0;
  int relaxed    = 0;
  int not_relaxed = 0;

  template <typename G, typename V> void on_initialize_vertex(const G&, const V&) { ++initialize; }
  template <typename G, typename V> void on_discover_vertex (const G&, const V&) { ++discover;  }
  template <typename G, typename V> void on_examine_vertex  (const G&, const V&) { ++examine;   }
  template <typename G, typename V> void on_finish_vertex   (const G&, const V&) { ++finish;    }
  template <typename G, typename E> void on_edge_relaxed    (const G&, const E&) { ++relaxed;   }
  template <typename G, typename E> void on_edge_not_relaxed(const G&, const E&) { ++not_relaxed; }
};

} // namespace

// ---------------------------------------------------------------------------
// Correctness: indexed heap produces the same distances as the default heap
// ---------------------------------------------------------------------------

TEST_CASE("dijkstra(indexed_heap) - CLRS example matches default heap",
          "[algorithm][dijkstra][indexed_heap]") {
  using Graph = vov_weighted;

  auto                            g = clrs_dijkstra_graph<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(g, distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
                          [](const auto& gr, const auto& uv) { return edge_value(gr, uv); },
                          empty_visitor{},
                          std::less<int>{},
                          std::plus<int>{},
                          std::allocator<std::byte>{},
                          use_indexed_dary_heap<4>{});

  for (size_t i = 0; i < clrs_dijkstra_results::distances_from_0.size(); ++i) {
    CHECK(distance[i] == clrs_dijkstra_results::distances_from_0[i]);
  }
}

TEST_CASE("dijkstra(indexed_heap) - path graph", "[algorithm][dijkstra][indexed_heap]") {
  using Graph = vov_weighted;

  auto                            g = path_graph_4_weighted<Graph>();
  std::vector<int>                distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(g, distance, predecessor);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(distance),
                          container_value_fn(predecessor),
                          [](const auto& gr, const auto& uv) { return edge_value(gr, uv); },
                          empty_visitor{},
                          std::less<int>{},
                          std::plus<int>{},
                          std::allocator<std::byte>{},
                          use_indexed_dary_heap<>{});

  for (size_t i = 0; i < path_graph_4_results::num_vertices; ++i) {
    CHECK(distance[i] == path_graph_4_results::distances[i]);
  }
}

TEST_CASE("dijkstra(indexed_heap) - multi-source CLRS", "[algorithm][dijkstra][indexed_heap]") {
  using Graph = vov_weighted;

  auto             g = clrs_dijkstra_graph<Graph>();
  std::vector<int> distance(num_vertices(g));
  std::vector<vertex_id_t<Graph>> predecessor(num_vertices(g));
  init_shortest_paths(g, distance, predecessor);

  std::vector<vertex_id_t<Graph>> sources = {0, 3};

  dijkstra_shortest_paths(g, sources,
                          container_value_fn(distance),
                          container_value_fn(predecessor),
                          [](const auto& gr, const auto& uv) { return edge_value(gr, uv); },
                          empty_visitor{},
                          std::less<int>{},
                          std::plus<int>{},
                          std::allocator<std::byte>{},
                          use_indexed_dary_heap<>{});

  // Both sources start at distance 0; every vertex is reachable.
  CHECK(distance[0] == 0);
  CHECK(distance[3] == 0);
  for (auto d : distance) CHECK(d != infinite_distance<int>());
}

TEST_CASE("dijkstra(indexed_heap) - distances-only overload",
          "[algorithm][dijkstra][indexed_heap]") {
  using Graph = vov_weighted;

  auto             g = clrs_dijkstra_graph<Graph>();
  std::vector<int> distance(num_vertices(g));
  init_shortest_paths(g, distance);

  dijkstra_shortest_distances(g, vertex_id_t<Graph>(0),
                              container_value_fn(distance),
                              [](const auto& gr, const auto& uv) { return edge_value(gr, uv); },
                              empty_visitor{},
                              std::less<int>{},
                              std::plus<int>{},
                              std::allocator<std::byte>{},
                              use_indexed_dary_heap<>{});

  for (size_t i = 0; i < clrs_dijkstra_results::distances_from_0.size(); ++i) {
    CHECK(distance[i] == clrs_dijkstra_results::distances_from_0[i]);
  }
}

TEST_CASE("dijkstra(indexed_heap) - arity 2 and arity 8 produce same distances",
          "[algorithm][dijkstra][indexed_heap]") {
  using Graph = vov_weighted;

  auto g = clrs_dijkstra_graph<Graph>();

  std::vector<int>                d2(num_vertices(g)), d8(num_vertices(g));
  std::vector<vertex_id_t<Graph>> p2(num_vertices(g)), p8(num_vertices(g));
  init_shortest_paths(g, d2, p2);
  init_shortest_paths(g, d8, p8);

  auto wt = [](const auto& gr, const auto& uv) { return edge_value(gr, uv); };

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(d2), container_value_fn(p2),
                          wt, empty_visitor{}, std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_indexed_dary_heap<2>{});
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(d8), container_value_fn(p8),
                          wt, empty_visitor{}, std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_indexed_dary_heap<8>{});

  CHECK(d2 == d8);
}

// ---------------------------------------------------------------------------
// Visitor call-count parity (Phase 2.3)
// ---------------------------------------------------------------------------

TEST_CASE("dijkstra(indexed_heap) - visitor call counts match default heap",
          "[algorithm][dijkstra][indexed_heap][visitor]") {
  using Graph = vov_weighted;

  auto g = clrs_dijkstra_graph<Graph>();
  auto wt = [](const auto& gr, const auto& uv) { return edge_value(gr, uv); };

  CountingVisitor v_default;
  CountingVisitor v_indexed;

  std::vector<int>                d_def(num_vertices(g)), d_idx(num_vertices(g));
  std::vector<vertex_id_t<Graph>> p_def(num_vertices(g)), p_idx(num_vertices(g));
  init_shortest_paths(g, d_def, p_def);
  init_shortest_paths(g, d_idx, p_idx);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(d_def), container_value_fn(p_def),
                          wt, v_default,
                          std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_default_heap{});

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(d_idx), container_value_fn(p_idx),
                          wt, v_indexed,
                          std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_indexed_dary_heap<>{});

  // Distances must agree.
  CHECK(d_def == d_idx);

  // Visitor call counts must agree exactly. Per Dijkstra invariants, every
  // reachable vertex is examined and finished once, every outgoing edge of
  // an examined vertex is either relaxed or not-relaxed exactly once, and
  // discover fires once per reachable vertex.
  CHECK(v_default.initialize  == v_indexed.initialize);
  CHECK(v_default.discover    == v_indexed.discover);
  CHECK(v_default.examine     == v_indexed.examine);
  CHECK(v_default.finish      == v_indexed.finish);
  CHECK(v_default.relaxed     == v_indexed.relaxed);
  CHECK(v_default.not_relaxed == v_indexed.not_relaxed);

  // Also assert the absolute invariant counts (5 reachable vertices in CLRS).
  CHECK(v_indexed.examine == 5);
  CHECK(v_indexed.finish  == 5);
  CHECK(v_indexed.discover == 5);
}

TEST_CASE("dijkstra(indexed_heap) - visitor parity on path graph",
          "[algorithm][dijkstra][indexed_heap][visitor]") {
  using Graph = vov_weighted;

  auto g  = path_graph_4_weighted<Graph>();
  auto wt = [](const auto& gr, const auto& uv) { return edge_value(gr, uv); };

  CountingVisitor v_default, v_indexed;

  std::vector<int>                d_def(num_vertices(g)), d_idx(num_vertices(g));
  std::vector<vertex_id_t<Graph>> p_def(num_vertices(g)), p_idx(num_vertices(g));
  init_shortest_paths(g, d_def, p_def);
  init_shortest_paths(g, d_idx, p_idx);

  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(d_def), container_value_fn(p_def),
                          wt, v_default,
                          std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_default_heap{});
  dijkstra_shortest_paths(g, vertex_id_t<Graph>(0),
                          container_value_fn(d_idx), container_value_fn(p_idx),
                          wt, v_indexed,
                          std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_indexed_dary_heap<>{});

  CHECK(d_def == d_idx);
  CHECK(v_default.discover    == v_indexed.discover);
  CHECK(v_default.examine     == v_indexed.examine);
  CHECK(v_default.finish      == v_indexed.finish);
  CHECK(v_default.relaxed     == v_indexed.relaxed);
  CHECK(v_default.not_relaxed == v_indexed.not_relaxed);
}

// ---------------------------------------------------------------------------
// Source-out-of-range still throws on the indexed-heap path
// ---------------------------------------------------------------------------

TEST_CASE("dijkstra(indexed_heap) - throws on out-of-range source",
          "[algorithm][dijkstra][indexed_heap]") {
  using Graph = vov_weighted;

  auto             g = clrs_dijkstra_graph<Graph>();
  std::vector<int> distance(num_vertices(g));
  init_shortest_paths(g, distance);

  CHECK_THROWS_AS(
        dijkstra_shortest_distances(
              g, vertex_id_t<Graph>(num_vertices(g) + 1),
              container_value_fn(distance),
              [](const auto& gr, const auto& uv) { return edge_value(gr, uv); },
              empty_visitor{}, std::less<int>{}, std::plus<int>{},
              std::allocator<std::byte>{}, use_indexed_dary_heap<>{}),
        std::out_of_range);
}

// ---------------------------------------------------------------------------
// Phase 3 - mapped-container support (assoc_position_map)
//
// SPARSE_VERTEX_TYPES are the map / unordered_map vertex containers
// (mov, mod, mol, uov, uod, uol). They do not satisfy index_vertex_range,
// so the indexed-heap path must select assoc_position_map automatically.
// ---------------------------------------------------------------------------

TEMPLATE_TEST_CASE("dijkstra(indexed_heap) - sparse CLRS matches default heap",
                   "[algorithm][dijkstra][indexed_heap][sparse]",
                   SPARSE_VERTEX_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;
  using namespace graph::test::map_fixtures;

  static_assert(!adj_list::index_vertex_range<Graph>,
                "SPARSE_VERTEX_TYPES must be mapped containers");

  const auto& exp = clrs_dijkstra_sparse_expected{};
  auto        g   = map_fixtures::clrs_dijkstra_graph<Graph>();

  auto d_def = make_vertex_property_map<Graph, int>(g, infinite_distance<int>());
  auto p_def = make_vertex_property_map<Graph, id_type>(g, id_type{});
  auto d_idx = make_vertex_property_map<Graph, int>(g, infinite_distance<int>());
  auto p_idx = make_vertex_property_map<Graph, id_type>(g, id_type{});
  for (auto&& [uid, u] : views::vertexlist(g)) {
    p_def[uid] = uid;
    p_idx[uid] = uid;
  }

  auto wt = [](const auto& gr, const auto& uv) { return edge_value(gr, uv); };

  dijkstra_shortest_paths(g, id_type(exp.s),
                          container_value_fn(d_def), container_value_fn(p_def),
                          wt, empty_visitor{}, std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_default_heap{});

  dijkstra_shortest_paths(g, id_type(exp.s),
                          container_value_fn(d_idx), container_value_fn(p_idx),
                          wt, empty_visitor{}, std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_indexed_dary_heap<>{});

  // Distances must agree with the textbook results and across heap paths.
  for (size_t i = 0; i < exp.num_vertices; ++i) {
    CHECK(d_idx[exp.vertex_ids[i]] == exp.distances[i]);
    CHECK(d_idx[exp.vertex_ids[i]] == d_def[exp.vertex_ids[i]]);
  }
  CHECK(p_idx[exp.s] == exp.s);
}

TEMPLATE_TEST_CASE("dijkstra(indexed_heap) - sparse visitor parity",
                   "[algorithm][dijkstra][indexed_heap][sparse][visitor]",
                   SPARSE_VERTEX_TYPES) {
  using Graph   = TestType;
  using id_type = vertex_id_t<Graph>;
  using namespace graph::test::map_fixtures;

  const auto& exp = clrs_dijkstra_sparse_expected{};
  auto        g   = map_fixtures::clrs_dijkstra_graph<Graph>();

  auto d_def = make_vertex_property_map<Graph, int>(g, infinite_distance<int>());
  auto d_idx = make_vertex_property_map<Graph, int>(g, infinite_distance<int>());

  CountingVisitor v_default, v_indexed;
  auto wt = [](const auto& gr, const auto& uv) { return edge_value(gr, uv); };

  dijkstra_shortest_distances(g, id_type(exp.s), container_value_fn(d_def),
                              wt, v_default, std::less<int>{}, std::plus<int>{},
                              std::allocator<std::byte>{}, use_default_heap{});
  dijkstra_shortest_distances(g, id_type(exp.s), container_value_fn(d_idx),
                              wt, v_indexed, std::less<int>{}, std::plus<int>{},
                              std::allocator<std::byte>{}, use_indexed_dary_heap<>{});

  CHECK(v_default.discover    == v_indexed.discover);
  CHECK(v_default.examine     == v_indexed.examine);
  CHECK(v_default.finish      == v_indexed.finish);
  CHECK(v_default.relaxed     == v_indexed.relaxed);
  CHECK(v_default.not_relaxed == v_indexed.not_relaxed);

  // 5 reachable vertices in the CLRS graph.
  CHECK(v_indexed.examine == 5);
  CHECK(v_indexed.finish  == 5);
}

// ---------------------------------------------------------------------------
// Phase 3 - non-integral vertex IDs (std::string keys)
//
// Exercises the assoc_position_map path with a hashable, non-integral key
// type. SPARSE_VERTEX_TYPES use uint32_t keys, so this test covers the
// remaining hashable_vertex_id branch of the if constexpr dispatch.
// ---------------------------------------------------------------------------

TEST_CASE("dijkstra(indexed_heap) - string vertex IDs (CLRS topology)",
          "[algorithm][dijkstra][indexed_heap][sparse][string_id]") {
  using VId    = std::string;
  using Traits = graph::container::mov_graph_traits<int, void, void, VId, false>;
  using Graph  = graph::container::dynamic_graph<int, void, void, VId, false, Traits>;

  static_assert(!adj_list::index_vertex_range<Graph>,
                "string-keyed graph must not satisfy index_vertex_range");
  static_assert(adj_list::hashable_vertex_id<Graph>,
                "std::string must satisfy hashable_vertex_id");

  // CLRS Figure 24.6 with string keys.
  Graph g({{"s", "t", 10}, {"s", "y", 5},
           {"t", "x", 1},  {"t", "y", 2},
           {"x", "z", 4},
           {"y", "t", 3},  {"y", "x", 9}, {"y", "z", 2},
           {"z", "s", 7},  {"z", "x", 6}});

  auto d_def = make_vertex_property_map<Graph, int>(g, infinite_distance<int>());
  auto p_def = make_vertex_property_map<Graph, VId>(g, VId{});
  auto d_idx = make_vertex_property_map<Graph, int>(g, infinite_distance<int>());
  auto p_idx = make_vertex_property_map<Graph, VId>(g, VId{});
  for (auto&& [uid, u] : views::vertexlist(g)) {
    p_def[uid] = uid;
    p_idx[uid] = uid;
  }

  auto wt = [](const auto& gr, const auto& uv) { return edge_value(gr, uv); };

  VId source{"s"};
  dijkstra_shortest_paths(g, source,
                          container_value_fn(d_def), container_value_fn(p_def),
                          wt, empty_visitor{}, std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_default_heap{});

  dijkstra_shortest_paths(g, source,
                          container_value_fn(d_idx), container_value_fn(p_idx),
                          wt, empty_visitor{}, std::less<int>{}, std::plus<int>{},
                          std::allocator<std::byte>{}, use_indexed_dary_heap<>{});

  // Textbook distances from CLRS Figure 24.6.
  CHECK(d_idx["s"] == 0);
  CHECK(d_idx["t"] == 8);
  CHECK(d_idx["x"] == 9);
  CHECK(d_idx["y"] == 5);
  CHECK(d_idx["z"] == 7);

  // Indexed heap must agree with the default heap on every vertex.
  for (auto&& [uid, u] : views::vertexlist(g)) {
    CHECK(d_idx[uid] == d_def[uid]);
  }
  CHECK(p_idx["s"] == "s");
}
