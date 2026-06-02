/**
 * @file plod.hpp
 * @brief PLOD (Power-Law Out-Degree) graph generator.
 *
 * Implements the Palmer–Steffan power-law out-degree model: each vertex i is
 * assigned a "credit" (target out-degree) of
 *
 *     credit(i) = floor(beta * x_i^(-alpha))
 *
 * where x_i is drawn uniformly from [1, n]. Edges are then placed by
 * repeatedly picking a source with remaining credit and a random target,
 * decrementing the source's credit. The resulting out-degree distribution
 * follows a power law with exponent controlled by `alpha`.
 *
 * Self-loops and duplicate directed edges are skipped; the returned list is
 * sorted by source_id.
 *
 * @note Barabási–Albert (`barabasi_albert.hpp`) is often a better choice for a
 *       scale-free graph; PLOD is provided for BGL parity.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <set>
#include <utility>
#include <vector>

namespace graph::generators {

/// Generate a PLOD (Power-Law Out-Degree) directed graph.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n     Number of vertices.
/// @param alpha Power-law exponent (larger ⇒ steeper degree decay).
/// @param beta  Degree scaling factor (larger ⇒ denser graph).
/// @param seed  RNG seed for reproducibility.
/// @param wdist Weight distribution family.
/// @return Sorted edge list (ascending by source_id), no self-loops/duplicates.
template <class VId = uint32_t>
edge_list<VId> plod(VId n, double alpha = 2.5, double beta = 10.0,
                    uint64_t seed = 42, weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list<VId>  generated_edges;
  if (n < 2) {
    return generated_edges;
  }

  // Assign a target out-degree (credit) to each vertex from the power law.
  std::vector<size_t> credit(n, 0);
  std::uniform_real_distribution<double> xdist(1.0, static_cast<double>(n));
  for (VId i = 0; i < n; ++i) {
    const double x = xdist(rng);
    const double c = beta * std::pow(x, -alpha) * static_cast<double>(n);
    credit[i]      = (c < 1.0) ? size_t{1} : static_cast<size_t>(c);
  }

  std::set<std::pair<VId, VId>>      present;
  std::uniform_int_distribution<VId> pick(0, n - 1);

  for (VId u = 0; u < n; ++u) {
    size_t guard = 0;
    const size_t max_attempts = credit[u] * 4 + 8;
    while (credit[u] > 0 && guard++ < max_attempts) {
      VId v = pick(rng);
      if (v == u) {
        continue;
      }
      if (present.insert({u, v}).second) {
        generated_edges.push_back({u, v, sample_weight(rng, wdist)});
        --credit[u];
      }
    }
  }

  std::stable_sort(generated_edges.begin(), generated_edges.end(),
                   [](const auto& a, const auto& b) { return a.source_id < b.source_id; });
  return generated_edges;
}

} // namespace graph::generators
