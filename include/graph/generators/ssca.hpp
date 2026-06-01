/**
 * @file ssca.hpp
 * @brief SSCA#2 (HPCS Scalable Synthetic Compact Applications #2) graph generator.
 *
 * Produces a graph built from randomly-sized cliques connected by sparse
 * inter-clique edges, modelling the SSCA#2 graph-analysis benchmark:
 *
 *   1. Vertices [0, n) are partitioned into consecutive cliques whose sizes are
 *      drawn uniformly from [1, max_clique_size].
 *   2. Every ordered pair within a clique is connected (dense local structure),
 *      with up to `max_parallel_edges` parallel edges per pair.
 *   3. Each vertex additionally emits inter-clique edges with probability
 *      `prob_inter_clique`, targeting a vertex in another clique chosen with a
 *      probability that decays with the inter-clique id distance (2^-distance).
 *
 * Self-loops are skipped; parallel edges within a clique are intentionally
 * permitted (a defining SSCA#2 characteristic). The returned list is sorted by
 * source_id.
 */

#pragma once

#include <graph/generators/common.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

namespace graph::generators {

/// Generate an SSCA#2 clique-based benchmark graph.
///
/// @tparam VId  Vertex id type (default: uint32_t).
/// @param n                 Number of vertices.
/// @param max_clique_size   Maximum clique size (sizes drawn from [1, this]).
/// @param prob_inter_clique Probability a vertex emits an inter-clique edge.
/// @param max_parallel_edges Maximum parallel edges per intra-clique pair.
/// @param seed              RNG seed for reproducibility.
/// @param wdist             Weight distribution family.
/// @return Sorted edge list (ascending by source_id); parallel intra-clique
///         edges retained, no self-loops.
template <class VId = uint32_t>
edge_list<VId> ssca(VId n, VId max_clique_size = 8, double prob_inter_clique = 0.2,
                    int max_parallel_edges = 2, uint64_t seed = 42,
                    weight_dist wdist = weight_dist::uniform) {
  std::mt19937_64 rng(seed);
  edge_list<VId>  generated_edges;
  if (n == 0) {
    return generated_edges;
  }
  if (max_clique_size < 1) {
    max_clique_size = 1;
  }
  if (max_parallel_edges < 1) {
    max_parallel_edges = 1;
  }

  // 1. Partition vertices into consecutive cliques of random size.
  //    clique_of[v] = clique index; clique_first/clique_last delimit each clique.
  std::vector<VId> clique_of(n, 0);
  std::vector<VId> clique_first; // first vertex id of each clique
  std::vector<VId> clique_last;  // one-past-last vertex id of each clique
  {
    std::uniform_int_distribution<VId> size_dist(1, max_clique_size);
    VId                                v = 0;
    VId                                cid = 0;
    while (v < n) {
      VId sz   = size_dist(rng);
      VId stop = (n - v < sz) ? n : static_cast<VId>(v + sz);
      clique_first.push_back(v);
      clique_last.push_back(stop);
      for (VId u = v; u < stop; ++u) {
        clique_of[u] = cid;
      }
      v = stop;
      ++cid;
    }
  }
  const VId num_cliques = static_cast<VId>(clique_first.size());

  std::uniform_real_distribution<double> coin(0.0, 1.0);
  std::uniform_int_distribution<int>     par_dist(1, max_parallel_edges);

  // 2. Intra-clique edges: all ordered pairs, with parallel multiplicity.
  for (VId c = 0; c < num_cliques; ++c) {
    for (VId u = clique_first[c]; u < clique_last[c]; ++u) {
      for (VId w = clique_first[c]; w < clique_last[c]; ++w) {
        if (u == w) {
          continue;
        }
        int parallel = par_dist(rng);
        for (int p = 0; p < parallel; ++p) {
          generated_edges.push_back({u, w, sample_weight(rng, wdist)});
        }
      }
    }
  }

  // 3. Inter-clique edges: probability decays with clique-id distance.
  if (num_cliques > 1) {
    std::uniform_int_distribution<VId> vertex_dist(0, n - 1);
    for (VId u = 0; u < n; ++u) {
      if (coin(rng) >= prob_inter_clique) {
        continue;
      }
      // Sample a candidate target; accept with probability 2^-(clique distance).
      VId  w        = vertex_dist(rng);
      int  attempts = 0;
      while (attempts++ < 16) {
        if (clique_of[w] != clique_of[u]) {
          const VId dist = (clique_of[w] > clique_of[u])
                               ? static_cast<VId>(clique_of[w] - clique_of[u])
                               : static_cast<VId>(clique_of[u] - clique_of[w]);
          const double accept = std::ldexp(1.0, -static_cast<int>(std::min<VId>(dist, 30)));
          if (coin(rng) < accept) {
            break;
          }
        }
        w = vertex_dist(rng);
      }
      if (clique_of[w] != clique_of[u]) {
        generated_edges.push_back({u, w, sample_weight(rng, wdist)});
      }
    }
  }

  std::stable_sort(generated_edges.begin(), generated_edges.end(),
                   [](const auto& lhs, const auto& rhs) { return lhs.source_id < rhs.source_id; });
  return generated_edges;
}

} // namespace graph::generators
