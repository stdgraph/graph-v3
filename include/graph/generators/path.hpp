/**
 * @file path.hpp
 * @brief Path graph generator: 0 → 1 → 2 → … → (n−1).
 *
 * Minimum decrease-key traffic: each vertex is relaxed at most once.
 * Serves as a lower-bound sanity check for shortest-path algorithms.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

/// Generate a directed path graph: 0 → 1 → 2 → … → (n−1).
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices.
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (already in order by source_id).
template <class VId = uint32_t>
edge_list<VId> path_graph(VId n, uint64_t seed = 42,
                          weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list<VId>  edges;
  edges.reserve(n > 0 ? n - 1 : 0);

  for (VId u = 0; u + 1 < n; ++u) {
    edges.push_back({u, static_cast<VId>(u + 1), sample_weight(rng, wdist)});
  }
  // Already sorted.
  return edges;
}

} // namespace graph::generators
