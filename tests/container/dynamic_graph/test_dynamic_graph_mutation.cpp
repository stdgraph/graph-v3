/**
 * @file test_dynamic_graph_mutation.cpp
 * @brief Comprehensive tests for the dynamic_graph mutation API.
 *
 * Exercises the BGL-style mutation members added to dynamic_graph_base:
 *   - add_vertex()            (sequential containers: append, return new id)
 *   - add_vertex(val)         (sequential containers, VV != void)
 *   - add_vertex(id)          (associative containers: keyed, returns bool)
 *   - add_vertex(id, val)     (associative containers, VV != void)
 *   - add_edge(u, v)          / add_edge(u, v, val)
 *   - remove_edge(u, v)
 *   - remove_vertex(u)
 *
 * Coverage spans:
 *   - Sequential vertex containers (vector / deque) vs. associative (map / unordered_map)
 *   - Edge containers: vector, list, forward_list, set, map (rekeying paths)
 *   - Integral and non-integral (std::string) vertex ids (sparse keys)
 *   - Edge value present (EV != void) and absent (EV == void)
 *   - Vertex value present (VV != void) and absent
 *   - Bidirectional (in_edges) maintenance
 *
 * Duplication is minimized via TEMPLATE_TEST_CASE and a couple of container-category
 * aware helpers (add_n_vertices / sequential_vertices concept).
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <graph/graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vol_graph_traits.hpp>
#include <graph/container/traits/vofl_graph_traits.hpp>
#include <graph/container/traits/vos_graph_traits.hpp>
#include <graph/container/traits/vom_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <graph/container/traits/mov_graph_traits.hpp>
#include <graph/container/traits/mol_graph_traits.hpp>
#include <graph/container/traits/uov_graph_traits.hpp>

#include <concepts>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using namespace graph::container;

using graph::adj_list::num_vertices;
using graph::adj_list::num_edges;
using graph::adj_list::find_vertex;
using graph::adj_list::vertex_id;
using graph::adj_list::vertex_value;
using graph::adj_list::edges;
using graph::adj_list::degree;
using graph::adj_list::target_id;
using graph::adj_list::edge_value;
using graph::adj_list::in_degree;

//==================================================================================================
// Container-category helpers
//==================================================================================================

// A graph has sequential vertices iff the no-argument add_vertex() overload is available.
template <class G>
concept sequential_vertices = requires(G g) {
  { g.add_vertex() } -> std::same_as<typename G::vertex_id_type>;
};

// Create n vertices with ids 0..n-1 and return them. Works for both container categories:
// sequential containers append and return the generated id; associative containers receive
// the explicit integral key.
template <class G>
std::vector<typename G::vertex_id_type> add_n_vertices(G& g, std::size_t n) {
  using vid_t = typename G::vertex_id_type;
  std::vector<vid_t> ids;
  ids.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    if constexpr (sequential_vertices<G>) {
      ids.push_back(g.add_vertex());
    } else {
      vid_t id = static_cast<vid_t>(i);
      g.add_vertex(id);
      ids.push_back(id);
    }
  }
  return ids;
}

// Collect the set of out-edge target ids for a given source vertex id.
template <class G>
std::multiset<typename G::vertex_id_type> out_targets(G& g, typename G::vertex_id_type uid) {
  std::multiset<typename G::vertex_id_type> t;
  for (auto e : edges(g, uid))
    t.insert(target_id(g, e));
  return t;
}

//==================================================================================================
// Type aliases
//==================================================================================================

// --- Sequential, weighted (EV=int), no vertex value ---
using seq_vov_w  = vov_graph<int>;
using seq_vol_w  = vol_graph<int>;
using seq_vofl_w = vofl_graph<int>;
using seq_vos_w  = vos_graph<int>;  // set edges: rekey path
using seq_vom_w  = vom_graph<int>;  // map edges: rekey path
using seq_dov_w  = dov_graph<int>;  // deque vertices

// --- Sequential, unweighted (EV=void) ---
using seq_vov_u  = vov_graph<void>;
using seq_vol_u  = vol_graph<void>;
using seq_dov_u  = dov_graph<void>;

// --- Sequential, with vertex value (VV=int) ---
using seq_vov_vv = vov_graph<void, int>;
using seq_vol_vv = vol_graph<void, int>;
using seq_dov_vv = dov_graph<void, int>;

// --- Associative, weighted (EV=int), integral keys ---
using assoc_mov_w = mov_graph<int>;
using assoc_mol_w = mol_graph<int>;
using assoc_uov_w = uov_graph<int>;

// --- Associative, with vertex value (VV=int) ---
using assoc_mov_vv = mov_graph<void, int>;
using assoc_uov_vv = uov_graph<void, int>;

// --- Associative, non-integral (std::string) keys ---
using assoc_str_w = mol_graph<int, void, void, std::string>;

//==================================================================================================
// Non-uniform bidirectional traits (so in-edges store source_id and in_degree works)
//==================================================================================================

template <class EV = void, class VV = void, class GV = void, class VId = uint32_t>
struct vov_bidir_traits {
  using edge_value_type   = EV;
  using vertex_value_type = VV;
  using graph_value_type  = GV;
  using vertex_id_type    = VId;
  static constexpr bool bidirectional = true;

  using edge_type    = dynamic_out_edge<EV, VV, GV, VId, true, vov_bidir_traits>;
  using in_edge_type = dynamic_in_edge<EV, VV, GV, VId, true, vov_bidir_traits>;
  using vertex_type  = dynamic_vertex<EV, VV, GV, VId, true, vov_bidir_traits>;
  using graph_type   = dynamic_graph<EV, VV, GV, VId, true, vov_bidir_traits>;

  using edges_type    = std::vector<edge_type>;
  using in_edges_type = std::vector<in_edge_type>;
  using vertices_type = std::vector<vertex_type>;
};

template <class EV = void, class VV = void, class GV = void, class VId = uint32_t>
struct vol_bidir_traits {
  using edge_value_type   = EV;
  using vertex_value_type = VV;
  using graph_value_type  = GV;
  using vertex_id_type    = VId;
  static constexpr bool bidirectional = true;

  using edge_type    = dynamic_out_edge<EV, VV, GV, VId, true, vol_bidir_traits>;
  using in_edge_type = dynamic_in_edge<EV, VV, GV, VId, true, vol_bidir_traits>;
  using vertex_type  = dynamic_vertex<EV, VV, GV, VId, true, vol_bidir_traits>;
  using graph_type   = dynamic_graph<EV, VV, GV, VId, true, vol_bidir_traits>;

  using edges_type    = std::list<edge_type>;
  using in_edges_type = std::list<in_edge_type>;
  using vertices_type = std::vector<vertex_type>;
};

using bidir_vov = dynamic_graph<int, void, void, uint32_t, true, vov_bidir_traits<int, void, void, uint32_t>>;
using bidir_vol = dynamic_graph<int, void, void, uint32_t, true, vol_bidir_traits<int, void, void, uint32_t>>;

//==================================================================================================
// 1. add_vertex — sequential containers
//==================================================================================================

TEMPLATE_TEST_CASE("add_vertex appends and returns sequential ids", "[mutation][add_vertex][sequential]",
                   seq_vov_w, seq_vol_w, seq_vofl_w, seq_vos_w, seq_vom_w, seq_dov_w,
                   seq_vov_u, seq_dov_u) {
  TestType g;
  REQUIRE(num_vertices(g) == 0);

  auto id0 = g.add_vertex();
  auto id1 = g.add_vertex();
  auto id2 = g.add_vertex();

  REQUIRE(id0 == 0);
  REQUIRE(id1 == 1);
  REQUIRE(id2 == 2);
  REQUIRE(num_vertices(g) == 3);
  REQUIRE(num_edges(g) == 0);
  REQUIRE(g.contains_vertex(0));
  REQUIRE(g.contains_vertex(2));
  REQUIRE_FALSE(g.contains_vertex(3));
}

//==================================================================================================
// 2. add_vertex(val) — sequential containers with vertex value
//==================================================================================================

TEMPLATE_TEST_CASE("add_vertex with value stores the vertex value", "[mutation][add_vertex][value][sequential]",
                   seq_vov_vv, seq_vol_vv, seq_dov_vv) {
  TestType g;

  auto id0 = g.add_vertex(100);
  auto id1 = g.add_vertex(200);

  REQUIRE(id0 == 0);
  REQUIRE(id1 == 1);
  REQUIRE(num_vertices(g) == 2);

  REQUIRE(vertex_value(g, *find_vertex(g, id0)) == 100);
  REQUIRE(vertex_value(g, *find_vertex(g, id1)) == 200);
}

//==================================================================================================
// 3. add_vertex(id) — associative containers (sparse / idempotent)
//==================================================================================================

TEMPLATE_TEST_CASE("add_vertex with key inserts sparse vertices", "[mutation][add_vertex][associative]",
                   assoc_mov_w, assoc_mol_w, assoc_uov_w) {
  TestType g;
  using vid_t = typename TestType::vertex_id_type;

  REQUIRE(g.add_vertex(vid_t{5}) == true);
  REQUIRE(g.add_vertex(vid_t{100}) == true);
  REQUIRE(g.add_vertex(vid_t{7}) == true);
  REQUIRE(num_vertices(g) == 3);
  REQUIRE(g.contains_vertex(vid_t{5}));
  REQUIRE(g.contains_vertex(vid_t{100}));
  REQUIRE(g.contains_vertex(vid_t{7}));
  REQUIRE_FALSE(g.contains_vertex(vid_t{6}));

  SECTION("re-adding an existing key is a no-op returning false") {
    REQUIRE(g.add_vertex(vid_t{5}) == false);
    REQUIRE(num_vertices(g) == 3);
  }
}

TEST_CASE("add_vertex preserves edges of an existing key", "[mutation][add_vertex][associative]") {
  assoc_mov_w g;
  REQUIRE(g.add_vertex(1u));
  REQUIRE(g.add_vertex(2u));
  g.add_edge(1u, 2u, 42);
  REQUIRE(num_edges(g) == 1);

  // Re-adding key 1 must not disturb its existing edge.
  REQUIRE(g.add_vertex(1u) == false);
  REQUIRE(num_edges(g) == 1);
  REQUIRE(degree(g, 1u) == 1);
}

//==================================================================================================
// 4. add_vertex(id, val) — associative containers with vertex value
//==================================================================================================

TEMPLATE_TEST_CASE("add_vertex with key and value", "[mutation][add_vertex][value][associative]",
                   assoc_mov_vv, assoc_uov_vv) {
  TestType g;
  using vid_t = typename TestType::vertex_id_type;

  REQUIRE(g.add_vertex(vid_t{10}, 111) == true);
  REQUIRE(g.add_vertex(vid_t{20}, 222) == true);
  REQUIRE(num_vertices(g) == 2);
  REQUIRE(vertex_value(g, *find_vertex(g, vid_t{10})) == 111);
  REQUIRE(vertex_value(g, *find_vertex(g, vid_t{20})) == 222);

  SECTION("re-adding an existing key updates the value and returns false") {
    REQUIRE(g.add_vertex(vid_t{10}, 999) == false);
    REQUIRE(num_vertices(g) == 2);
    REQUIRE(vertex_value(g, *find_vertex(g, vid_t{10})) == 999);
  }
}

//==================================================================================================
// 5. Non-integral (std::string) vertex ids
//==================================================================================================

TEST_CASE("add_vertex / add_edge with non-integral string ids", "[mutation][add_vertex][nonintegral]") {
  assoc_str_w g;

  REQUIRE(g.add_vertex(std::string{"alice"}) == true);
  REQUIRE(g.add_vertex(std::string{"bob"}) == true);
  REQUIRE(g.add_vertex(std::string{"carol"}) == true);
  REQUIRE(g.add_vertex(std::string{"alice"}) == false); // duplicate
  REQUIRE(num_vertices(g) == 3);

  g.add_edge(std::string{"alice"}, std::string{"bob"}, 7);
  g.add_edge(std::string{"alice"}, std::string{"carol"}, 9);
  REQUIRE(num_edges(g) == 2);
  REQUIRE(degree(g, std::string{"alice"}) == 2);

  auto t = out_targets(g, std::string{"alice"});
  REQUIRE(t.count("bob") == 1);
  REQUIRE(t.count("carol") == 1);

  SECTION("remove_vertex keeps remaining string keys stable") {
    g.remove_vertex(std::string{"bob"});
    REQUIRE_FALSE(g.contains_vertex(std::string{"bob"}));
    REQUIRE(g.contains_vertex(std::string{"alice"}));
    REQUIRE(g.contains_vertex(std::string{"carol"}));
    REQUIRE(num_edges(g) == 1); // alice->bob removed, alice->carol remains
    REQUIRE(degree(g, std::string{"alice"}) == 1);
  }
}

//==================================================================================================
// 6. add_edge — weighted, all container categories
//==================================================================================================

TEMPLATE_TEST_CASE("add_edge with value builds a weighted graph", "[mutation][add_edge][weighted]",
                   seq_vov_w, seq_vol_w, seq_vofl_w, seq_vos_w, seq_vom_w, seq_dov_w,
                   assoc_mov_w, assoc_mol_w, assoc_uov_w) {
  TestType g;
  auto ids = add_n_vertices(g, 3); // 0, 1, 2

  g.add_edge(ids[0], ids[1], 10);
  g.add_edge(ids[0], ids[2], 30);
  g.add_edge(ids[1], ids[2], 20);

  REQUIRE(num_edges(g) == 3);
  REQUIRE(degree(g, ids[0]) == 2);
  REQUIRE(degree(g, ids[1]) == 1);
  REQUIRE(degree(g, ids[2]) == 0);

  REQUIRE(out_targets(g, ids[0]) == std::multiset<typename TestType::vertex_id_type>{ids[1], ids[2]});

  SECTION("edge values are stored") {
    int sum = 0;
    for (auto e : edges(g, ids[0]))
      sum += edge_value(g, e);
    REQUIRE(sum == 40);
  }
}

//==================================================================================================
// 7. add_edge — unweighted
//==================================================================================================

TEMPLATE_TEST_CASE("add_edge without value builds an unweighted graph", "[mutation][add_edge][unweighted]",
                   seq_vov_u, seq_vol_u, seq_dov_u) {
  TestType g;
  auto ids = add_n_vertices(g, 3);

  g.add_edge(ids[0], ids[1]);
  g.add_edge(ids[0], ids[2]);

  REQUIRE(num_edges(g) == 2);
  REQUIRE(degree(g, ids[0]) == 2);
  REQUIRE(degree(g, ids[1]) == 0);
}

//==================================================================================================
// 8. add_edge throws when a vertex is missing
//==================================================================================================

TEST_CASE("add_edge throws std::out_of_range for a missing vertex", "[mutation][add_edge][error]") {
  SECTION("sequential container") {
    seq_vov_w g;
    add_n_vertices(g, 2); // ids 0, 1
    REQUIRE_THROWS_AS(g.add_edge(0u, 5u, 1), std::out_of_range);
    REQUIRE_THROWS_AS(g.add_edge(5u, 0u, 1), std::out_of_range);
  }
  SECTION("associative container") {
    assoc_mov_w g;
    g.add_vertex(0u);
    REQUIRE_THROWS_AS(g.add_edge(0u, 99u, 1), std::out_of_range);
    REQUIRE_THROWS_AS(g.add_edge(99u, 0u, 1), std::out_of_range);
  }
}

//==================================================================================================
// 9. remove_edge
//==================================================================================================

TEMPLATE_TEST_CASE("remove_edge removes edges and reports the count", "[mutation][remove_edge]",
                   seq_vov_w, seq_vol_w, seq_vos_w, seq_vom_w, seq_dov_w,
                   assoc_mov_w, assoc_mol_w, assoc_uov_w) {
  TestType g;
  auto ids = add_n_vertices(g, 3);
  g.add_edge(ids[0], ids[1], 10);
  g.add_edge(ids[0], ids[2], 30);
  g.add_edge(ids[1], ids[2], 20);
  REQUIRE(num_edges(g) == 3);

  SECTION("removing an existing edge") {
    REQUIRE(g.remove_edge(ids[0], ids[1]) == 1);
    REQUIRE(num_edges(g) == 2);
    REQUIRE(degree(g, ids[0]) == 1);
    REQUIRE(out_targets(g, ids[0]) == std::multiset<typename TestType::vertex_id_type>{ids[2]});
  }

  SECTION("removing a non-existent edge returns 0") {
    REQUIRE(g.remove_edge(ids[2], ids[0]) == 0);
    REQUIRE(num_edges(g) == 3);
  }

  SECTION("removing an edge from a missing source returns 0") {
    REQUIRE(g.remove_edge(typename TestType::vertex_id_type{99}, ids[0]) == 0);
    REQUIRE(num_edges(g) == 3);
  }
}

//==================================================================================================
// 10. remove_vertex — sequential containers (id renumbering)
//==================================================================================================

TEMPLATE_TEST_CASE("remove_vertex renumbers ids on sequential containers", "[mutation][remove_vertex][sequential]",
                   seq_vov_w, seq_vol_w, seq_vos_w, seq_vom_w, seq_dov_w) {
  TestType g;
  add_n_vertices(g, 4); // ids 0,1,2,3
  // path 0 -> 1 -> 2 -> 3
  g.add_edge(0u, 1u, 1);
  g.add_edge(1u, 2u, 2);
  g.add_edge(2u, 3u, 3);
  REQUIRE(num_edges(g) == 3);

  g.remove_vertex(1u);

  // Vertex 1 gone: its out-edge (1->2) and the in-edge (0->1) disappear.
  // The surviving edge (2->3) is renumbered to (1->2).
  REQUIRE(num_vertices(g) == 3);
  REQUIRE(num_edges(g) == 1);
  REQUIRE(degree(g, 0u) == 0);
  REQUIRE(degree(g, 1u) == 1); // old vertex 2
  REQUIRE(degree(g, 2u) == 0); // old vertex 3
  REQUIRE(out_targets(g, 1u) == std::multiset<typename TestType::vertex_id_type>{2u});
}

//==================================================================================================
// 11. remove_vertex — associative containers (stable ids)
//==================================================================================================

TEMPLATE_TEST_CASE("remove_vertex keeps ids stable on associative containers",
                   "[mutation][remove_vertex][associative]",
                   assoc_mov_w, assoc_mol_w, assoc_uov_w) {
  TestType g;
  using vid_t = typename TestType::vertex_id_type;
  g.add_vertex(vid_t{10});
  g.add_vertex(vid_t{20});
  g.add_vertex(vid_t{30});
  g.add_edge(vid_t{10}, vid_t{20}, 1);
  g.add_edge(vid_t{10}, vid_t{30}, 2);
  g.add_edge(vid_t{20}, vid_t{30}, 3);
  REQUIRE(num_edges(g) == 3);

  g.remove_vertex(vid_t{20});

  REQUIRE(num_vertices(g) == 2);
  REQUIRE_FALSE(g.contains_vertex(vid_t{20}));
  REQUIRE(g.contains_vertex(vid_t{10}));
  REQUIRE(g.contains_vertex(vid_t{30}));
  // (10->20) removed and (20->30) gone with the vertex; (10->30) remains.
  REQUIRE(num_edges(g) == 1);
  REQUIRE(degree(g, vid_t{10}) == 1);
  REQUIRE(out_targets(g, vid_t{10}) == std::multiset<vid_t>{vid_t{30}});
}

TEST_CASE("remove_vertex on a missing id is a no-op", "[mutation][remove_vertex]") {
  seq_vov_w g;
  add_n_vertices(g, 2);
  g.add_edge(0u, 1u, 5);
  g.remove_vertex(99u);
  REQUIRE(num_vertices(g) == 2);
  REQUIRE(num_edges(g) == 1);
}

//==================================================================================================
// 12. Bidirectional in_edge maintenance
//==================================================================================================

TEMPLATE_TEST_CASE("bidirectional add_edge maintains in_edges", "[mutation][bidirectional][add_edge]",
                   bidir_vov, bidir_vol) {
  TestType g;
  add_n_vertices(g, 3);
  g.add_edge(0u, 1u, 10);
  g.add_edge(0u, 2u, 30);
  g.add_edge(1u, 2u, 20);

  REQUIRE(num_edges(g) == 3);
  REQUIRE(in_degree(g, 0u) == 0);
  REQUIRE(in_degree(g, 1u) == 1);
  REQUIRE(in_degree(g, 2u) == 2);
}

TEMPLATE_TEST_CASE("bidirectional remove_edge unlinks in_edges", "[mutation][bidirectional][remove_edge]",
                   bidir_vov, bidir_vol) {
  TestType g;
  add_n_vertices(g, 3);
  g.add_edge(0u, 2u, 30);
  g.add_edge(1u, 2u, 20);
  REQUIRE(in_degree(g, 2u) == 2);

  REQUIRE(g.remove_edge(0u, 2u) == 1);
  REQUIRE(num_edges(g) == 1);
  REQUIRE(in_degree(g, 2u) == 1);
  REQUIRE(degree(g, 0u) == 0);
}

TEMPLATE_TEST_CASE("bidirectional remove_vertex unlinks neighbor in/out edges",
                   "[mutation][bidirectional][remove_vertex]",
                   bidir_vov, bidir_vol) {
  TestType g;
  add_n_vertices(g, 4); // 0,1,2,3
  g.add_edge(0u, 1u, 1);
  g.add_edge(1u, 2u, 2);
  g.add_edge(2u, 3u, 3);
  REQUIRE(num_edges(g) == 3);

  g.remove_vertex(1u);

  // Surviving edge (2->3) renumbers to (1->2).
  REQUIRE(num_vertices(g) == 3);
  REQUIRE(num_edges(g) == 1);
  REQUIRE(in_degree(g, 0u) == 0);
  REQUIRE(in_degree(g, 2u) == 1); // old vertex 3 keeps one incoming edge
  REQUIRE(degree(g, 1u) == 1);    // old vertex 2
}
