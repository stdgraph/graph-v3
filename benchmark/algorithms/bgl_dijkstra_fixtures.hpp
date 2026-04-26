/**
 * @file bgl_dijkstra_fixtures.hpp
 * @brief Boost.Graph (BGL) container builders for Dijkstra comparison benchmarks.
 *
 * Companion to dijkstra_fixtures.hpp. Builds BGL graphs from the *same*
 * edge_list produced by the synthetic generators, so graph-v3 and BGL
 * benchmarks operate on topologically identical graphs.
 *
 *   compressed_graph (CSR)        ↔ boost::compressed_sparse_row_graph
 *   dynamic_graph    (vov)        ↔ boost::adjacency_list<vecS, vecS, directedS, …>
 *
 * Only compiled when BENCH_BGL is defined (set by CMake when the BGL include
 * directory is available); see benchmark/algorithms/CMakeLists.txt.
 *
 * Phase 4.3 — sanity-check comparison only. BGL distance results must match
 * graph-v3 distance results bit-for-bit on the same source vertex; the
 * benchmarks assert this once at startup before timing starts.
 */

#pragma once

#include "dijkstra_fixtures.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/property_map/property_map.hpp>

#include <limits>
#include <utility>
#include <vector>

namespace graph::benchmark {

// ---------------------------------------------------------------------------
// Bundled edge property: a single double weight, mirroring graph-v3 layout.
// ---------------------------------------------------------------------------

struct bgl_edge_prop {
  double weight = 0.0;
};

// ---------------------------------------------------------------------------
// BGL container types
// ---------------------------------------------------------------------------

/// CSR equivalent. directedS, no vertex bundle, edge bundle = bgl_edge_prop.
/// vertex_index is implicit (vecS storage).
using bgl_csr_graph_t =
      boost::compressed_sparse_row_graph<boost::directedS,
                                         boost::no_property,
                                         bgl_edge_prop,
                                         boost::no_property,
                                         vertex_id_t,
                                         vertex_id_t>;

/// adjacency_list equivalent of dynamic_graph<vov, …>: vector of vector,
/// directed, edge bundle stores the weight.
using bgl_adj_graph_t =
      boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                            boost::no_property, bgl_edge_prop>;

// ---------------------------------------------------------------------------
// Container builders
// ---------------------------------------------------------------------------

/// Build a BGL CSR graph from a graph-v3 edge_list (already sorted by source).
inline bgl_csr_graph_t make_bgl_csr(const edge_list& edges, vertex_id_t num_vertices) {
  // BGL CSR ctor wants a range of (vertex_id_t, vertex_id_t) pairs and a
  // parallel range of edge bundles.
  std::vector<std::pair<vertex_id_t, vertex_id_t>> pairs;
  std::vector<bgl_edge_prop>                       props;
  pairs.reserve(edges.size());
  props.reserve(edges.size());
  for (const auto& e : edges) {
    pairs.emplace_back(e.source_id, e.target_id);
    props.push_back(bgl_edge_prop{static_cast<double>(e.value)});
  }
  return bgl_csr_graph_t(boost::edges_are_sorted,
                         pairs.begin(), pairs.end(),
                         props.begin(),
                         static_cast<std::size_t>(num_vertices));
}

/// Build a BGL adjacency_list from the same edge_list.
inline bgl_adj_graph_t make_bgl_adj(const edge_list& edges, vertex_id_t num_vertices) {
  bgl_adj_graph_t g(static_cast<std::size_t>(num_vertices));
  for (const auto& e : edges) {
    boost::add_edge(e.source_id, e.target_id,
                    bgl_edge_prop{static_cast<double>(e.value)}, g);
  }
  return g;
}

// ---------------------------------------------------------------------------
// Dijkstra wrapper
//
// Uses the no_color_map, no_init variant to match graph-v3 semantics:
// caller pre-initialises the distance vector; no per-call color-map
// allocation. Predecessor map is required by the BGL signature even when
// unused — wired to a dummy iterator_property_map.
// ---------------------------------------------------------------------------

template <class BglGraph>
inline void run_bgl_dijkstra(const BglGraph& g,
                             vertex_id_t source,
                             std::vector<double>& dist) {
  using vd_t = typename boost::graph_traits<BglGraph>::vertex_descriptor;

  // Predecessor scratch — required by the API but unused here.
  std::vector<vd_t> pred(boost::num_vertices(g));

  auto idx       = boost::get(boost::vertex_index, g);
  auto dist_pmap = boost::make_iterator_property_map(dist.begin(), idx);
  auto pred_pmap = boost::make_iterator_property_map(pred.begin(), idx);
  auto w_pmap    = boost::get(&bgl_edge_prop::weight, g);

  boost::dijkstra_shortest_paths_no_color_map_no_init(
        g, static_cast<vd_t>(source),
        pred_pmap,
        dist_pmap,
        w_pmap,
        idx,
        std::less<double>{},
        boost::closed_plus<double>(),
        std::numeric_limits<double>::max(),
        0.0,
        boost::default_dijkstra_visitor{});
}

} // namespace graph::benchmark
