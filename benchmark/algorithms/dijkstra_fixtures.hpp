/**
 * @file dijkstra_fixtures.hpp
 * @brief Synthetic graph generators for Dijkstra benchmarks.
 *
 * Provides graph generators that isolate three orthogonal axes:
 *   - Scale        : V ∈ {1K, 10K, 100K}
 *   - Topology     : Erdős–Rényi, 2D grid, Barabási–Albert, path
 *   - Weight dist  : uniform, exponential, constant-1
 *
 * Each generator returns a sorted edge_list (sorted by source_id, as
 * required by compressed_graph). Pass the list to make_csr() or make_vov()
 * to build the target container.
 *
 * Usage:
 *   auto edges = benchmark::erdos_renyi(10'000, 8.0 / 10'000);
 *   auto g     = benchmark::make_csr(edges, 10'000);
 *   // ... run Dijkstra on g
 *
 * NOTE: The generator algorithms now live in the public library at
 * <graph/generators.hpp>. This file provides benchmark-specific container
 * aliases and convenience wrappers in the graph::benchmark namespace.
 */

#pragma once

#include <graph/generators.hpp>
#include <graph/container/compressed_graph.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>

namespace graph::benchmark {

// ---------------------------------------------------------------------------
// Common types — aliases for the public generator types
// ---------------------------------------------------------------------------

using vertex_id_t = uint32_t;
using weight_t    = double;
using edge_entry  = graph::generators::edge_entry<vertex_id_t, weight_t>;
using edge_list   = graph::generators::edge_list<vertex_id_t, weight_t>;

// Re-export weight_dist into benchmark namespace for backward compatibility.
using graph::generators::weight_dist;

/// Primary container: CSR layout; minimises traversal overhead so that
/// heap cost is the dominant measurable term.
using csr_graph_t =
      graph::container::compressed_graph<weight_t, void, void, vertex_id_t, vertex_id_t>;

/// Secondary container: vov-backed dynamic_graph; representative of typical
/// user code and used as a regression baseline.
using vov_graph_t =
      graph::container::dynamic_graph<weight_t, void, void, vertex_id_t, false,
                                      graph::container::vov_graph_traits<weight_t, void, void,
                                                                         vertex_id_t, false>>;

// ---------------------------------------------------------------------------
// Generator wrappers — delegate to graph::generators with fixed VId
// ---------------------------------------------------------------------------

inline edge_list erdos_renyi(vertex_id_t n, double p, uint64_t seed = 42,
                             weight_dist wdist = weight_dist::uniform) {
  return graph::generators::erdos_renyi<vertex_id_t>(n, p, seed, wdist);
}

inline edge_list grid_2d(vertex_id_t rows, vertex_id_t cols, uint64_t seed = 42,
                         weight_dist wdist = weight_dist::uniform) {
  return graph::generators::grid_2d<vertex_id_t>(rows, cols, seed, wdist);
}

inline edge_list barabasi_albert(vertex_id_t n, vertex_id_t m, uint64_t seed = 42,
                                 weight_dist wdist = weight_dist::uniform) {
  return graph::generators::barabasi_albert<vertex_id_t>(n, m, seed, wdist);
}

inline edge_list path_graph(vertex_id_t n, uint64_t seed = 42,
                            weight_dist wdist = weight_dist::uniform) {
  return graph::generators::path_graph<vertex_id_t>(n, seed, wdist);
}

// ---------------------------------------------------------------------------
// Container builders
// ---------------------------------------------------------------------------

/// Build a compressed_graph (CSR) from a pre-sorted edge list.
inline csr_graph_t make_csr(const edge_list& edges, vertex_id_t num_vertices) {
  csr_graph_t g;
  g.load_edges(edges, std::identity{}, num_vertices);
  return g;
}

/// Build a vov dynamic_graph from an edge list (order does not matter).
inline vov_graph_t make_vov(const edge_list& edges, vertex_id_t num_vertices) {
  vov_graph_t g;
  g.load_edges(edges, std::identity{}, num_vertices);
  return g;
}

} // namespace graph::benchmark
