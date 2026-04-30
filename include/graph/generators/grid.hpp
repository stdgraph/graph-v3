/**
 * @file grid.hpp
 * @brief 2D grid graph generator — bidirectional 4-connected.
 *
 * Vertex (r, c) has id r*cols + c.
 * Horizontal and vertical neighbour pairs each get two directed edges
 * (both directions), giving E/V ≈ 4 for interior vertices.
 * The returned list is sorted by source_id.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

/// Generate a 2D grid graph with bidirectional 4-connectivity.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param rows  Number of rows.
/// @param cols  Number of columns.
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (sorted ascending by source_id).
template <class VId = uint32_t>
edge_list<VId> grid_2d(VId rows, VId cols, uint64_t seed = 42,
                       weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  const VId       n = rows * cols;

  edge_list<VId> edges;
  edges.reserve(4 * static_cast<size_t>(n)); // upper bound

  for (VId r = 0; r < rows; ++r) {
    for (VId c = 0; c < cols; ++c) {
      VId u = r * cols + c;
      // Right neighbour
      if (c + 1 < cols) {
        VId v = u + 1;
        edges.push_back({u, v, sample_weight(rng, wdist)});
        edges.push_back({v, u, sample_weight(rng, wdist)});
      }
      // Down neighbour
      if (r + 1 < rows) {
        VId v = u + cols;
        edges.push_back({u, v, sample_weight(rng, wdist)});
        edges.push_back({v, u, sample_weight(rng, wdist)});
      }
    }
  }
  std::stable_sort(edges.begin(), edges.end(),
                   [](const auto& a, const auto& b) { return a.source_id < b.source_id; });
  return edges;
}

} // namespace graph::generators
