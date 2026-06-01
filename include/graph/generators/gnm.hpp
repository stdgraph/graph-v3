/**
 * @file gnm.hpp
 * @brief Erdős–Rényi G(n, m) random graph generator (fixed edge count).
 *
 * The G(n, m) model selects exactly m distinct edges uniformly at random from
 * the n*(n−1) ordered (u, v) pairs with u ≠ v. This is the fixed-edge-count
 * companion to G(n, p) in @ref erdos_renyi.hpp.
 *
 * Edges are addressed by the same position encoding used by G(n, p):
 *   pos ∈ [0, n*(n−1))
 *   u      = pos / (n−1)
 *   offset = pos % (n−1)
 *   v      = offset < u ? offset : offset + 1   (skip self-loop)
 *
 * Distinct positions are drawn by rejection sampling (efficient while
 * m ≪ n*(n−1)) and the resulting edge list is sorted by source_id.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <unordered_set>
#include <vector>

namespace graph::generators {

/// Generate an Erdős–Rényi G(n, m) directed random graph (no self-loops).
///
/// Selects exactly `m` distinct edges uniformly at random. If `m` exceeds the
/// maximum possible edge count n*(n−1), it is clamped to that maximum.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices.
/// @param m     Number of edges to generate.
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (ascending by source_id).
template <class VId = uint32_t>
edge_list<VId> erdos_renyi_gnm(VId n, size_t m, uint64_t seed = 42,
                               weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  const size_t    total = (n > 1) ? static_cast<size_t>(n) * (n - 1) : 0;
  if (m > total) {
    m = total;
  }

  edge_list<VId> generated_edges;
  generated_edges.reserve(m);
  if (m == 0) {
    return generated_edges;
  }

  std::unordered_set<size_t> chosen;
  chosen.reserve(m * 2);
  std::uniform_int_distribution<size_t> pick(0, total - 1);

  while (chosen.size() < m) {
    const size_t pos = pick(rng);
    if (chosen.insert(pos).second) {
      const VId u      = static_cast<VId>(pos / (n - 1));
      const VId offset = static_cast<VId>(pos % (n - 1));
      const VId v      = (offset < u) ? offset : offset + 1;
      generated_edges.push_back({u, v, sample_weight(rng, wdist)});
    }
  }

  std::sort(generated_edges.begin(), generated_edges.end(),
            [](const auto& a, const auto& b) {
              return (a.source_id != b.source_id) ? a.source_id < b.source_id
                                                  : a.target_id < b.target_id;
            });
  return generated_edges;
}

} // namespace graph::generators
