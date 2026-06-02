/**
 * @file rmat.hpp
 * @brief R-MAT (Recursive MATrix) graph generator.
 *
 * Generates a directed graph whose edges follow a recursive, self-similar
 * degree distribution controlled by four probabilities (a, b, c, d) that
 * partition the adjacency matrix into four quadrants:
 *
 *     +-------+-------+
 *     |   a   |   b   |
 *     +-------+-------+
 *     |   c   |   d   |
 *     +-------+-------+
 *
 * Each edge is placed by recursively descending into a quadrant for
 * log2(scale) levels. With a skewed (a, b, c, d) this produces the
 * power-law / community structure used by the Graph500 benchmark.
 *
 * The number of vertices is rounded **up** to the next power of two; the
 * returned list is sorted by source_id. Duplicate directed edges produced by
 * the recursive sampling are removed.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace graph::generators {

/// Generate an R-MAT directed graph (Graph500-style).
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param scale Graph has 2^scale vertices.
/// @param m     Number of (directed) edges to attempt to place.
/// @param a     Quadrant probability for the top-left block.
/// @param b     Quadrant probability for the top-right block.
/// @param c     Quadrant probability for the bottom-left block.
/// @param d     Quadrant probability for the bottom-right block (a+b+c+d ≈ 1).
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (ascending by source_id), self-loops removed.
template <class VId = uint32_t>
edge_list<VId> rmat(uint32_t scale, size_t m, double a = 0.57, double b = 0.19,
                    double c = 0.19, double d = 0.05, uint64_t seed = 42,
                    weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list<VId>  generated_edges;
  if (scale == 0 || m == 0) {
    return generated_edges;
  }

  const double sum = a + b + c + d;
  // Normalise so the four probabilities form a partition of unity.
  const double pa = a / sum;
  const double pb = pa + b / sum;
  const double pc = pb + c / sum;

  std::uniform_real_distribution<double> coin(0.0, 1.0);
  std::set<std::pair<VId, VId>>          present;
  generated_edges.reserve(m);

  for (size_t e = 0; e < m; ++e) {
    VId u = 0;
    VId v = 0;
    for (uint32_t level = 0; level < scale; ++level) {
      const VId    bit = static_cast<VId>(VId{1} << (scale - 1 - level));
      const double r   = coin(rng);
      if (r < pa) {
        // top-left: row bit 0, col bit 0
      } else if (r < pb) {
        v = static_cast<VId>(v | bit); // top-right: col bit 1
      } else if (r < pc) {
        u = static_cast<VId>(u | bit); // bottom-left: row bit 1
      } else {
        u = static_cast<VId>(u | bit); // bottom-right: both bits 1
        v = static_cast<VId>(v | bit);
      }
    }
    if (u == v) {
      continue; // skip self-loops
    }
    if (present.insert({u, v}).second) {
      generated_edges.push_back({u, v, sample_weight(rng, wdist)});
    }
  }

  std::stable_sort(generated_edges.begin(), generated_edges.end(),
                   [](const auto& lhs, const auto& rhs) { return lhs.source_id < rhs.source_id; });
  return generated_edges;
}

} // namespace graph::generators
