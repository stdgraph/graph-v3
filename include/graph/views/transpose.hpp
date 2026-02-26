/**
 * @file transpose.hpp
 * @brief Zero-cost transpose adaptor for bidirectional graphs.
 *
 * Wraps a bidirectional graph so that the roles of outgoing and incoming
 * edges are exchanged:
 *
 * | Original CPO        | Transpose CPO              |
 * |---------------------|----------------------------|
 * | edges(g, u)         | in_edges(underlying, u)    |
 * | in_edges(g, u)      | edges(underlying, u)       |
 * | target_id(g, e)     | source_id(underlying, e)   |
 * | source_id(g, e)     | target_id(underlying, e)   |
 * | target(g, e)        | source(underlying, e)      |
 * | source(g, e)        | target(underlying, e)      |
 * | degree(g, v)        | in_degree(underlying, v)   |
 * | in_degree(g, v)     | degree(underlying, v)      |
 *
 * All other CPOs (vertices, num_vertices, find_vertex, vertex_id,
 * edge_value, graph_value, etc.) forward to the underlying graph.
 *
 * @section concept_satisfaction Concept Satisfaction
 *
 * When the underlying graph satisfies `index_bidirectional_adjacency_list`,
 * `transpose_view<G>` also satisfies `index_bidirectional_adjacency_list`.
 *
 * @section limitations Known Limitations
 *
 * The `target_id` / `source_id` ADL overrides resolve correctly for
 * graphs whose edge descriptors use index-based storage (random-access
 * containers: vov, vod, dov, dod, dofl, etc.).  For forward-iterator
 * containers (vol, dol), the CPO's tier-1 ("native edge member") fires
 * before the ADL tier, bypassing the swap.  A future accessor-parameterised
 * DFS/BFS (Phases 5 + 7) will eliminate this limitation.
 *
 * For algorithm use with ALL container types, prefer the single-graph
 * `kosaraju(g, component)` overload, which handles reverse traversal
 * internally without needing transpose_view.
 *
 * @section usage Usage
 *
 * @code
 *   auto tv = graph::views::transpose(bidir_graph);
 *
 *   // Iterate "forward" edges of the transpose = in-edges of original
 *   for (auto&& [uid, u] : views::vertexlist(tv)) {
 *     for (auto&& [tid, e] : views::incidence(tv, u)) {
 *       // tid is the source vertex in the original graph
 *     }
 *   }
 * @endcode
 *
 * @see connected_components.hpp — kosaraju(g, component) bidirectional overload
 */

#pragma once

#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

namespace graph::views {

// ============================================================================
// transpose_view — bidirectional graph adaptor
// ============================================================================

/**
 * @brief Adaptor that presents a bidirectional graph with edges reversed.
 *
 * Stores a pointer to the underlying graph.  All ADL friend functions
 * delegate to the underlying graph, swapping outgoing/incoming roles.
 *
 * @tparam G Underlying graph type (must satisfy bidirectional_adjacency_list)
 */
template <adj_list::bidirectional_adjacency_list G>
class transpose_view {
public:
  using graph_type = G;

  /// Construct from a mutable reference to the underlying graph.
  explicit constexpr transpose_view(G& g) noexcept : g_(&g) {}

  /// Access the underlying graph.
  [[nodiscard]] constexpr G&       base() noexcept { return *g_; }
  [[nodiscard]] constexpr const G& base() const noexcept { return *g_; }

  // ===========================================================================
  // Vertex CPO friends — forwarded unchanged
  // ===========================================================================

  friend constexpr auto vertices(transpose_view& tv) { return adj_list::vertices(*tv.g_); }
  friend constexpr auto vertices(const transpose_view& tv) { return adj_list::vertices(*tv.g_); }

  friend constexpr auto num_vertices(const transpose_view& tv) { return adj_list::num_vertices(*tv.g_); }

  friend constexpr auto find_vertex(transpose_view& tv, const adj_list::vertex_id_t<G>& uid) {
    return adj_list::find_vertex(*tv.g_, uid);
  }
  friend constexpr auto find_vertex(const transpose_view& tv, const adj_list::vertex_id_t<G>& uid) {
    return adj_list::find_vertex(*tv.g_, uid);
  }

  template <typename V>
  friend constexpr auto vertex_id(const transpose_view& tv, const V& v) {
    return adj_list::vertex_id(*tv.g_, v);
  }

  template <typename V>
  friend constexpr auto has_edges(const transpose_view& tv, const V& v) {
    return adj_list::has_edges(*tv.g_, v);
  }

  // ===========================================================================
  // Edge CPO friends — directions swapped
  // ===========================================================================

  /// edges(transpose, v) → in_edges(underlying, v)
  template <typename V>
  friend constexpr auto edges(transpose_view& tv, V&& v) {
    return adj_list::in_edges(*tv.g_, std::forward<V>(v));
  }
  template <typename V>
  friend constexpr auto edges(const transpose_view& tv, V&& v) {
    return adj_list::in_edges(*tv.g_, std::forward<V>(v));
  }

  /// in_edges(transpose, v) → edges(underlying, v)
  template <typename V>
  friend constexpr auto in_edges(transpose_view& tv, V&& v) {
    return adj_list::edges(*tv.g_, std::forward<V>(v));
  }
  template <typename V>
  friend constexpr auto in_edges(const transpose_view& tv, V&& v) {
    return adj_list::edges(*tv.g_, std::forward<V>(v));
  }

  // ===========================================================================
  // Edge property CPO friends — source/target swapped
  // ===========================================================================

  /// target_id(transpose, e) → source_id(underlying, e)
  template <typename E>
  friend constexpr auto target_id(const transpose_view& tv, const E& e) {
    return adj_list::source_id(*tv.g_, e);
  }
  template <typename E>
  friend constexpr auto target_id(transpose_view& tv, const E& e) {
    return adj_list::source_id(*tv.g_, e);
  }

  /// source_id(transpose, e) → target_id(underlying, e)
  template <typename E>
  friend constexpr auto source_id(const transpose_view& tv, const E& e) {
    return adj_list::target_id(*tv.g_, e);
  }
  template <typename E>
  friend constexpr auto source_id(transpose_view& tv, const E& e) {
    return adj_list::target_id(*tv.g_, e);
  }

  /// target(transpose, e) → source(underlying, e)
  template <adj_list::edge_descriptor_type E>
  friend constexpr auto target(const transpose_view& tv, const E& e) {
    return adj_list::source(*tv.g_, e);
  }
  template <adj_list::edge_descriptor_type E>
  friend constexpr auto target(transpose_view& tv, const E& e) {
    return adj_list::source(*tv.g_, e);
  }

  /// source(transpose, e) → target(underlying, e)
  template <adj_list::edge_descriptor_type E>
  friend constexpr auto source(const transpose_view& tv, const E& e) {
    return adj_list::target(*tv.g_, e);
  }
  template <adj_list::edge_descriptor_type E>
  friend constexpr auto source(transpose_view& tv, const E& e) {
    return adj_list::target(*tv.g_, e);
  }

  /// edge_value — forwarded unchanged
  template <typename E>
  friend constexpr decltype(auto) edge_value(const transpose_view& tv, const E& e) {
    return adj_list::edge_value(*tv.g_, e);
  }
  template <typename E>
  friend constexpr decltype(auto) edge_value(transpose_view& tv, const E& e) {
    return adj_list::edge_value(*tv.g_, e);
  }

  // ===========================================================================
  // Degree CPO friends — swapped
  // ===========================================================================

  /// degree(transpose, v) → in_degree(underlying, v)
  template <typename V>
  friend constexpr auto degree(const transpose_view& tv, const V& v) {
    return adj_list::in_degree(*tv.g_, v);
  }

  /// in_degree(transpose, v) → degree(underlying, v)
  template <typename V>
  friend constexpr auto in_degree(const transpose_view& tv, const V& v) {
    return adj_list::degree(*tv.g_, v);
  }

  // ===========================================================================
  // num_edges — forwarded
  // ===========================================================================

  template <typename V>
  friend constexpr auto num_edges(const transpose_view& tv, const V& v) {
    return adj_list::num_edges(*tv.g_, v);
  }

  friend constexpr auto num_edges(const transpose_view& tv) { return adj_list::num_edges(*tv.g_); }

private:
  G* g_;
};

// ============================================================================
// Factory function
// ============================================================================

/**
 * @brief Create a transpose view of a bidirectional graph.
 *
 * @tparam G Graph type (must satisfy bidirectional_adjacency_list)
 * @param g The graph to transpose (by reference; must outlive the view)
 * @return transpose_view<G>
 */
template <adj_list::bidirectional_adjacency_list G>
[[nodiscard]] constexpr transpose_view<G> transpose(G& g) noexcept {
  return transpose_view<G>(g);
}

} // namespace graph::views
