/**
 * @file common.hpp
 * @brief Common types and utilities for graph generators.
 */

#pragma once

#include <graph/graph_data.hpp>

#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

// ---------------------------------------------------------------------------
// Weight distribution selector
// ---------------------------------------------------------------------------

/// Selects the random distribution used for edge weights.
enum class weight_dist {
  uniform,      ///< U[1, 100] — default, "average case"
  exponential,  ///< Exp(0.1) + 1 — heavy left tail
  constant_one, ///< Always 1 — BFS-equivalent floor
};

/// Sample a single weight value from the selected distribution.
/// @param rng  A uniform random bit generator (e.g. std::mt19937_64).
/// @param dist Which distribution family to draw from.
/// @return A positive weight value.
template <class URBG>
double sample_weight(URBG& rng, weight_dist dist) {
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
// Default type aliases for convenience
// ---------------------------------------------------------------------------

/// Default edge type returned by generators: (source_id, target_id, weight).
template <class VId = uint32_t, class EV = double>
using edge_entry = graph::copyable_edge_t<VId, EV>;

/// Default edge list type returned by generators.
template <class VId = uint32_t, class EV = double>
using edge_list = std::vector<edge_entry<VId, EV>>;

} // namespace graph::generators
