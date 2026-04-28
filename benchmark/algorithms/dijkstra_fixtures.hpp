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
 */

#pragma once

#include <graph/container/compressed_graph.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/graph_data.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace graph::benchmark {

// ---------------------------------------------------------------------------
// Common types
// ---------------------------------------------------------------------------

using vertex_id_t = uint32_t;
using weight_t    = double;
using edge_entry  = graph::copyable_edge_t<vertex_id_t, weight_t>;
using edge_list   = std::vector<edge_entry>;

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
// Weight distribution
// ---------------------------------------------------------------------------

enum class weight_dist {
  uniform,      ///< U[1, 100] — default, "average case"
  exponential,  ///< Exp(0.1) + 1 — heavy left tail, more decrease-key events
  constant_one, ///< Always 1 — BFS-equivalent floor, minimum variance
};

inline double sample_weight(std::mt19937_64& rng, weight_dist dist) {
  switch (dist) {
    case weight_dist::uniform: {
      std::uniform_real_distribution<double> d(1.0, 100.0);
      return d(rng);
    }
    case weight_dist::exponential: {
      std::exponential_distribution<double> d(0.1);
      return 1.0 + d(rng);
    }
    case weight_dist::constant_one:
    default:
      return 1.0;
  }
}

// ---------------------------------------------------------------------------
// Erdős–Rényi G(n, p)  — directed, self-loops excluded
//
// Uses the O(E) geometric-skip algorithm (Batagelj & Brandes, 2005) instead
// of the naive O(n²) coin-flip loop, so it scales to n = 10⁶.
//
// The n*(n−1) ordered (u,v) pairs with u≠v are enumerated as positions
//   pos ∈ [0, n*(n−1))
// where position pos maps to:
//   u      = pos / (n−1)
//   offset = pos % (n−1)
//   v      = offset < u ? offset : offset + 1   (skip self-loop)
//
// Set p = k / n for E/V ≈ k (sparse: k=2, moderate: k=8, dense: k=32).
// The resulting edge list is already sorted by source_id because positions
// are visited in ascending order and u is non-decreasing.
// ---------------------------------------------------------------------------

inline edge_list erdos_renyi(vertex_id_t n, double p, uint64_t seed = 42,
                             weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  const size_t    total = static_cast<size_t>(n) * (n - 1); // n*(n-1) directed pairs

  edge_list edges;
  const size_t expected = static_cast<size_t>(total * p * 1.1) + 16;
  edges.reserve(expected);

  // Geometric skip: sample the gap between consecutive selected positions.
  // std::geometric_distribution<size_t> gives the number of failures before
  // the first success, so adding 1 gives the gap to the *next* success.
  std::geometric_distribution<size_t> geom(p);

  size_t pos = geom(rng); // 0-indexed position of the first selected edge
  while (pos < total) {
    const vertex_id_t u      = static_cast<vertex_id_t>(pos / (n - 1));
    const vertex_id_t offset = static_cast<vertex_id_t>(pos % (n - 1));
    const vertex_id_t v      = (offset < u) ? offset : offset + 1;
    edges.push_back({u, v, sample_weight(rng, wdist)});
    pos += geom(rng) + 1;
  }
  // Edges are already sorted by source_id (u is non-decreasing).
  return edges;
}

// ---------------------------------------------------------------------------
// 2D grid graph (rows × cols)  — bidirectional 4-connected
//
// Vertex (r, c) has id r*cols + c.
// Horizontal and vertical neighbour pairs each get two directed edges
// (both directions), giving E/V ≈ 4 for interior vertices.
// The returned list is sorted by source_id.
// ---------------------------------------------------------------------------

inline edge_list grid_2d(vertex_id_t rows, vertex_id_t cols, uint64_t seed = 42,
                         weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  const vertex_id_t n = rows * cols;

  edge_list edges;
  edges.reserve(4 * static_cast<size_t>(n)); // upper bound

  for (vertex_id_t r = 0; r < rows; ++r) {
    for (vertex_id_t c = 0; c < cols; ++c) {
      vertex_id_t u = r * cols + c;
      // Right neighbour
      if (c + 1 < cols) {
        vertex_id_t v = u + 1;
        edges.push_back({u, v, sample_weight(rng, wdist)});
        edges.push_back({v, u, sample_weight(rng, wdist)});
      }
      // Down neighbour
      if (r + 1 < rows) {
        vertex_id_t v = u + cols;
        edges.push_back({u, v, sample_weight(rng, wdist)});
        edges.push_back({v, u, sample_weight(rng, wdist)});
      }
    }
  }
  std::stable_sort(edges.begin(), edges.end(),
                   [](const edge_entry& a, const edge_entry& b) {
                     return a.source_id < b.source_id;
                   });
  return edges;
}

// ---------------------------------------------------------------------------
// Barabási–Albert preferential attachment  — scale-free / power-law
//
// Starts with a fully-connected seed of m0 = max(m, 2) vertices, then
// adds each subsequent vertex w by selecting m existing targets with
// probability proportional to their current degree ("urn" method).
// Both w→t and t→w directed edges are added so the graph is undirected
// in terms of reachability, which maximises relaxation traffic from hubs.
// The returned list is sorted by source_id.
// ---------------------------------------------------------------------------

inline edge_list barabasi_albert(vertex_id_t n, vertex_id_t m, uint64_t seed = 42,
                                 weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);

  // "Urn" stores one entry per endpoint per edge, giving degree-proportional
  // selection at O(1) per pick (trade memory for simplicity).
  std::vector<vertex_id_t> urn;
  urn.reserve(2 * static_cast<size_t>(n) * m);

  edge_list edges;
  edges.reserve(2 * static_cast<size_t>(n) * m);

  // Seed: fully-connected clique of m0 vertices
  const vertex_id_t m0 = std::max(m, vertex_id_t{2});
  for (vertex_id_t u = 0; u < m0; ++u) {
    for (vertex_id_t v = u + 1; v < m0; ++v) {
      edges.push_back({u, v, sample_weight(rng, wdist)});
      edges.push_back({v, u, sample_weight(rng, wdist)});
      urn.push_back(u);
      urn.push_back(v);
    }
  }

  for (vertex_id_t w = m0; w < n; ++w) {
    std::vector<vertex_id_t> chosen;
    chosen.reserve(m);

    while (chosen.size() < m) {
      std::uniform_int_distribution<size_t> pick(0, urn.size() - 1);
      vertex_id_t t = urn[pick(rng)];
      // chosen.size() ≤ m ≤ ~8 so linear scan is fine
      bool already = (t == w);
      for (auto x : chosen) already |= (x == t);
      if (!already) {
        chosen.push_back(t);
        edges.push_back({w, t, sample_weight(rng, wdist)});
        edges.push_back({t, w, sample_weight(rng, wdist)});
        urn.push_back(w);
        urn.push_back(t);
      }
    }
  }

  std::stable_sort(edges.begin(), edges.end(),
                   [](const edge_entry& a, const edge_entry& b) {
                     return a.source_id < b.source_id;
                   });
  return edges;
}

// ---------------------------------------------------------------------------
// Path graph: 0 → 1 → 2 → … → (n−1)
//
// Minimum decrease-key traffic: each vertex is relaxed at most once.
// Serves as a lower-bound sanity check.
// ---------------------------------------------------------------------------

inline edge_list path_graph(vertex_id_t n, uint64_t seed = 42,
                            weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list       edges;
  edges.reserve(n > 0 ? n - 1 : 0);

  for (vertex_id_t u = 0; u + 1 < n; ++u) {
    edges.push_back({u, u + 1, sample_weight(rng, wdist)});
  }
  // Already sorted.
  return edges;
}

// ---------------------------------------------------------------------------
// Container builders
// ---------------------------------------------------------------------------

/// Build a compressed_graph (CSR) from a pre-sorted edge list.
/// edges must be sorted ascending by source_id (enforced by assertion in
/// compressed_graph::load_edges).
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
