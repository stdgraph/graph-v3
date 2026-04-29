/**
 * @file erdos_renyi.hpp
 * @brief Erdős–Rényi G(n, p) random graph generator.
 *
 * Uses the O(E) geometric-skip algorithm (Batagelj & Brandes, 2005)
 * instead of the naive O(n²) coin-flip loop, so it scales to n = 10⁶.
 *
 * The n*(n−1) ordered (u,v) pairs with u≠v are enumerated as positions
 *   pos ∈ [0, n*(n−1))
 * where position pos maps to:
 *   u      = pos / (n−1)
 *   offset = pos % (n−1)
 *   v      = offset < u ? offset : offset + 1   (skip self-loop)
 *
 * Set p = k / n for E/V ≈ k (sparse: k=2, moderate: k=8, dense: k=32).
 * The resulting edge list is already sorted by source_id because positions
 * are visited in ascending order and u is non-decreasing.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

/// Generate an Erdős–Rényi G(n, p) directed random graph (no self-loops).
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices.
/// @param p     Edge probability. Use p = k/n for expected out-degree k.
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (sorted ascending by source_id).
template <class VId = uint32_t>
edge_list<VId> erdos_renyi(VId n, double p, uint64_t seed = 42,
                           weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  const size_t    total = static_cast<size_t>(n) * (n - 1); // n*(n-1) directed pairs

  edge_list<VId> edges;
  const size_t   expected = static_cast<size_t>(static_cast<double>(total) * p * 1.1) + 16;
  edges.reserve(expected);

  // Geometric skip: sample the gap between consecutive selected positions.
  std::geometric_distribution<size_t> geom(p);

  size_t pos = geom(rng); // 0-indexed position of the first selected edge
  while (pos < total) {
    const VId u      = static_cast<VId>(pos / (n - 1));
    const VId offset = static_cast<VId>(pos % (n - 1));
    const VId v      = (offset < u) ? offset : offset + 1;
    edges.push_back({u, v, sample_weight(rng, wdist)});
    pos += geom(rng) + 1;
  }
  // Edges are already sorted by source_id (u is non-decreasing).
  return edges;
}

} // namespace graph::generators
