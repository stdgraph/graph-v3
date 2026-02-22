/**
 * @file edge_accessor.hpp
 * @brief Edge accessor policies for parameterizing graph views.
 *
 * Defines stateless policy objects that bundle the three operations needed
 * by view iterators: fetching the edge range from a vertex, extracting
 * the neighbor vertex id from an edge, and obtaining the neighbor vertex
 * descriptor.  By defaulting to @c out_edge_accessor, existing code is
 * source-compatible.  Passing @c in_edge_accessor flips views to iterate
 * over incoming edges.
 *
 * @section accessors Provided Accessors
 *
 * | Accessor              | edges()           | neighbor_id()   | neighbor()     |
 * |-----------------------|-------------------|-----------------|----------------|
 * | @c out_edge_accessor  | @c edges(g,u)     | @c target_id    | @c target      |
 * | @c in_edge_accessor   | @c in_edges(g,u)  | @c source_id    | @c source      |
 *
 * @section type_traits Type Traits
 *
 * Each accessor exposes two member alias templates:
 * - @c edge_range_t<G> — the range type returned by @c edges()
 * - @c edge_t<G> — the edge descriptor type (range value type)
 *
 * Views use these to derive their internal type aliases, ensuring that
 * @c incidence_view<G,void,in_edge_accessor> correctly uses
 * @c in_edge_range_t<G> and @c in_edge_t<G>.
 *
 * @section usage Usage
 *
 * @code
 *   // Forward (outgoing) incidence — default:
 *   for (auto [tid, e] : incidence(g, u)) { ... }
 *
 *   // Reverse (incoming) incidence:
 *   for (auto [sid, e] : in_incidence(g, u)) { ... }
 * @endcode
 *
 * @see incidence.hpp — incidence views parameterized by accessor
 * @see neighbors.hpp — neighbor views parameterized by accessor
 */

#pragma once

#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

namespace graph::views {

// =============================================================================
// out_edge_accessor — outgoing edge policy (default)
// =============================================================================

/**
 * @brief Policy for outgoing-edge iteration.
 *
 * This is the default accessor used by all view classes.  It delegates to
 * @c edges(g,u) / @c target_id(g,e) / @c target(g,e).
 *
 * Stateless — @c [[no_unique_address]] storage costs zero bytes.
 */
struct out_edge_accessor {
  /// Edge range type for a graph @c G.
  template <class G>
  using edge_range_t = adj_list::vertex_edge_range_t<G>;

  /// Edge descriptor type for a graph @c G.
  template <class G>
  using edge_t = adj_list::edge_t<G>;

  /// Return the outgoing edge range for vertex @c u.
  template <adj_list::adjacency_list G>
  [[nodiscard]] constexpr auto edges(G& g, adj_list::vertex_t<G> u) const {
    return adj_list::edges(g, u);
  }

  /// Return the neighbor (target) vertex id from edge @c e.
  template <adj_list::adjacency_list G>
  [[nodiscard]] constexpr auto neighbor_id(G& g, adj_list::edge_t<G> e) const {
    return adj_list::target_id(g, e);
  }

  /// Return the neighbor (target) vertex descriptor from edge @c e.
  template <adj_list::adjacency_list G>
  [[nodiscard]] constexpr auto neighbor(G& g, adj_list::edge_t<G> e) const {
    return adj_list::target(g, e);
  }
};

// =============================================================================
// in_edge_accessor — incoming edge policy
// =============================================================================

/**
 * @brief Policy for incoming-edge iteration.
 *
 * Delegates to @c in_edges(g,u) / @c source_id(g,e) / @c source(g,e).
 * Constrained on @c bidirectional_adjacency_list so that it only compiles
 * for graphs that provide incoming-edge support.
 *
 * Stateless — @c [[no_unique_address]] storage costs zero bytes.
 */
struct in_edge_accessor {
  /// Edge range type for incoming edges of graph @c G.
  template <class G>
  using edge_range_t = adj_list::in_edge_range_t<G>;

  /// Edge descriptor type for incoming edges of graph @c G.
  template <class G>
  using edge_t = adj_list::in_edge_t<G>;

  /// Return the incoming edge range for vertex @c u.
  template <adj_list::bidirectional_adjacency_list G>
  [[nodiscard]] constexpr auto edges(G& g, adj_list::vertex_t<G> u) const {
    return adj_list::in_edges(g, u);
  }

  /// Return the neighbor (source) vertex id from incoming edge @c e.
  template <adj_list::bidirectional_adjacency_list G>
  [[nodiscard]] constexpr auto neighbor_id(G& g, adj_list::in_edge_t<G> e) const {
    return adj_list::source_id(g, e);
  }

  /// Return the neighbor (source) vertex descriptor from incoming edge @c e.
  template <adj_list::bidirectional_adjacency_list G>
  [[nodiscard]] constexpr auto neighbor(G& g, adj_list::in_edge_t<G> e) const {
    return adj_list::source(g, e);
  }
};

} // namespace graph::views
