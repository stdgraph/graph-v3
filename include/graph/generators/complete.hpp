/**
 * @file complete.hpp
 * @brief Complete graph generator K(n): every ordered pair (u, v) with u ≠ v.
 *
 * Produces a fully-connected directed graph with n*(n−1) edges. Useful as a
 * dense-graph stress test and as a worst-case input for algorithms whose cost
 * scales with edge count.
 *
 * @warning The edge count grows as O(n²); generating K(n) for large n is
 *          memory-intensive (e.g. n = 10'000 yields ~100M edges).
 */

#pragma once

#include <graph/generators/common.hpp>

#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

/// Generate a complete directed graph K(n): all ordered pairs (u, v), u ≠ v.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices.
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (ascending by source_id, then target_id).
template <class VId = uint32_t>
edge_list<VId> complete_graph(VId n, uint64_t seed = 42,
                              weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list<VId>  generated_edges;
  const size_t    total = (n > 0) ? static_cast<size_t>(n) * (n - 1) : 0;
  generated_edges.reserve(total);

  for (VId u = 0; u < n; ++u) {
    for (VId v = 0; v < n; ++v) {
      if (v == u) {
        continue;
      }
      generated_edges.push_back({u, v, sample_weight(rng, wdist)});
    }
  }
  // Already sorted by source_id (u non-decreasing), then target_id.
  return generated_edges;
}

} // namespace graph::generators
