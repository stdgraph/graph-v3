/**
 * @file edgelist.hpp
 * @brief Edgelist views for iterating over all edges in a graph.
 *
 * @section overview Overview
 *
 * Provides lazy, range-based views that flatten the two-level adjacency-list
 * structure into a single range of edges.  Each iteration step yields an
 * @c edge_info whose fields are exposed via structured bindings, including
 * both source and target vertex IDs.  An optional edge value function (EVF)
 * computes a per-edge value that is included in the binding.
 *
 * For per-vertex edge iteration use @ref incidence.hpp instead; for
 * edge-list data structures (not adjacency lists) see
 * @c edge_list_edgelist_view at the bottom of this file.
 *
 * @section variants View Variants
 *
 * | Variant                          | Structured Binding  | Description                         |
 * |----------------------------------|---------------------|-------------------------------------|
 * | @c edgelist(g)                   | `[sid, tid, uv]`    | Standard view (ids + edge)          |
 * | @c edgelist(g,evf)               | `[sid, tid, uv, val]` | Standard view with value function |
 * | @c basic_edgelist(g)             | `[sid, tid]`        | Simplified view (ids only)          |
 * | @c basic_edgelist(g,evf)         | `[sid, tid, val]`   | Simplified view with value fn       |
 *
 * @section bindings Structured Bindings
 *
 * Standard view:
 * @code
 *   for (auto [sid, tid, uv]      : edgelist(g))         // sid, tid = vertex_id_t<G>, uv = edge_t<G>
 *   for (auto [sid, tid, uv, val] : edgelist(g, evf))    // val = invoke_result_t<EVF, G&, edge_t<G>>
 * @endcode
 *
 * basic_ variant:
 * @code
 *   for (auto [sid, tid]      : basic_edgelist(g))        // sid, tid = vertex_id_t<G>
 *   for (auto [sid, tid, val] : basic_edgelist(g, evf))   // val = invoke_result_t<EVF, G&, edge_t<G>>
 * @endcode
 *
 * @section iterator_properties Iterator Properties
 *
 * | Property        | Value                                                    |
 * |-----------------|----------------------------------------------------------|
 * | Concept         | @c std::forward_iterator                                 |
 * | Range concept   | @c std::ranges::forward_range                            |
 * | Sized           | Yes when graph provides O(1) @c num_edges (member/ADL)   |
 * | Borrowed        | No (view holds reference)                                |
 * | Common          | Yes (begin/end same type)                                |
 *
 * @section perf Performance Characteristics
 *
 * Construction is O(1).  @c begin() is O(V) in the worst case because it
 * must skip leading vertices that have no edges.  Each @c operator++ is
 * amortised O(1): within a vertex's edge range it is a simple increment,
 * and between vertices it advances to the next non-empty edge range.  Full
 * iteration visits every edge exactly once in O(V + E) time.  The view
 * holds only a pointer to the graph — no allocation.  The @c basic_ variant
 * is lighter still: it never materialises an edge descriptor.
 *
 * @section chaining Chaining with std::views
 *
 * Views chain with std::views when the value function is a stateless lambda
 * (empty capture list @c []):
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   auto view = edgelist(g, evf)
 *               | std::views::take(10);  // ✅ compiles
 * @endcode
 *
 * See @ref view_chaining_limitations.md for the design rationale.
 *
 * @section pipe Pipe Adaptor Syntax
 *
 * Pipe-style usage is available via @c graph::views::adaptors:
 * @code
 *   using namespace graph::views::adaptors;
 *
 *   for (auto [sid, tid, uv] : g | edgelist())             { ... }
 *   for (auto [sid, tid]     : g | basic_edgelist())       { ... }
 *   for (auto [sid, tid, val]: g | basic_edgelist(evf))    { ... }
 * @endcode
 *
 * @section supported_graphs Supported Graph Properties
 *
 * - Requires: @c adjacency_list concept
 * - Works with all @c dynamic_graph container combinations
 * - Works with directed and undirected graphs
 * - @c edge_list_edgelist_view wraps @c basic_sourced_edgelist data structures
 *
 * @section exception_safety Exception Safety
 *
 * Construction is @c noexcept when EVF is nothrow move-constructible (or
 * absent).  Iteration may propagate exceptions from the value function; all
 * other iterator operations are @c noexcept.
 *
 * @section preconditions Preconditions
 *
 * - The graph @c g must outlive the view.
 * - The graph must not be mutated during iteration.
 *
 * @section see_also See Also
 *
 * - @ref views.md — all views overview
 * - @ref graph_cpo_implementation.md — CPO documentation
 * - @ref view_chaining_limitations.md — chaining design rationale
 * - @ref vertexlist.hpp — whole-graph vertex iteration
 * - @ref incidence.hpp — per-vertex edge iteration
 * - @ref neighbors.hpp — per-vertex adjacent-vertex iteration
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <graph/graph_info.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/edge_list/edge_list.hpp>
#include <graph/views/view_concepts.hpp>

namespace graph::views {

namespace edgelist_detail {
  /// @brief Detect O(1) member function g.num_edges()
  template <class G>
  concept _has_num_edges_member = requires(const G& g) {
    { g.num_edges() } -> std::integral;
  };

  /// @brief Detect O(1) ADL num_edges(g)
  template <class G>
  concept _has_num_edges_adl = requires(const G& g) {
    { num_edges(g) } -> std::integral;
  };

  /// @brief Detect if a graph type supports O(1) total edge count.
  /// True when the graph provides g.num_edges() member or ADL num_edges(g).
  /// False for the default CPO fallback (which iterates all vertices, O(V)).
  /// When true, edgelist_view conditionally provides size().
  template <class G>
  concept has_const_time_num_edges = _has_num_edges_member<G> || _has_num_edges_adl<G>;
} // namespace edgelist_detail

// Forward declarations
template <adj_list::adjacency_list G, class EVF = void>
class edgelist_view;

template <adj_list::adjacency_list G, class EVF = void>
class basic_edgelist_view;

/**
 * @brief Edgelist view without value function.
 *
 * Flattens the adjacency-list structure into a single range of edges,
 * yielding @c edge_info{sid,tid,uv} per edge via structured bindings.
 *
 * @code
 *   for (auto [sid, tid, uv] : edgelist(g)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 *
 * @see edgelist_view<G,EVF> — with value function
 * @see basic_edgelist_view  — simplified (ids only)
 */
template <adj_list::adjacency_list G>
class edgelist_view<G, void> : public std::ranges::view_interface<edgelist_view<G, void>> {
public:
  using graph_type      = G;
  using vertex_type     = adj_list::vertex_t<G>;
  using vertex_id_type  = adj_list::vertex_id_t<G>;
  using edge_range_type = adj_list::vertex_edge_range_t<G>;
  using edge_type       = adj_list::edge_t<G>;
  using info_type       = edge_info<vertex_id_type, true, edge_type, void>;

  /**
     * @brief Forward iterator yielding @c edge_info{sid, tid, uv}.
     *
     * Walks every vertex in order; for each vertex, walks its outgoing edges.
     * @c operator*() returns an @c edge_info with @c source_id, @c target_id
     * and the edge descriptor.
     */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, vertex_type v, vertex_type v_end, edge_type current_edge, edge_type edge_end) noexcept
          : g_(g), v_(v), v_end_(v_end), current_edge_(current_edge), edge_end_(edge_end) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      auto source_id = adj_list::vertex_id(*g_, v_);
      auto target_id = adj_list::target_id(*g_, current_edge_);
      return value_type{static_cast<vertex_id_type>(source_id), static_cast<vertex_id_type>(target_id), current_edge_};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      // If we've exhausted edges for current vertex, move to next vertex
      if (current_edge_ == edge_end_) {
        ++v_;
        advance_to_valid_edge();
      }
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      // Two iterators are equal if:
      // 1. Both are at the end (v_ == v_end_), or
      // 2. They point to the same vertex and edge
      if (v_ == v_end_ && other.v_ == other.v_end_) {
        return true;
      }
      if (v_ == v_end_ || other.v_ == other.v_end_) {
        return false;
      }
      return v_ == other.v_ && current_edge_ == other.current_edge_;
    }

    /**
         * @brief Advance to the next vertex that has edges, initializing edge descriptors
         * 
         * Skips over vertices with no edges until we find one with edges or reach the end.
         */
    constexpr void advance_to_valid_edge() noexcept {
      while (v_ != v_end_) {
        auto edge_range = adj_list::edges(*g_, v_);
        auto begin_it   = std::ranges::begin(edge_range);
        auto end_it     = std::ranges::end(edge_range);
        if (begin_it != end_it) {
          current_edge_ = *begin_it;
          edge_end_     = *end_it;
          return; // Found a vertex with edges
        }
        ++v_;
      }
      // Reached end - no more vertices with edges
    }

  private:
    G*          g_ = nullptr;
    vertex_type v_{};            // Current vertex descriptor
    vertex_type v_end_{};        // End sentinel for vertices
    edge_type   current_edge_{}; // Current edge descriptor
    edge_type   edge_end_{};     // End sentinel for current vertex's edges
  };

  using const_iterator = iterator;

  constexpr edgelist_view() noexcept = default;

  constexpr edgelist_view(G& g) noexcept : g_(&g) {}

  [[nodiscard]] constexpr iterator begin() const noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_begin = *std::ranges::begin(v_range);
    auto v_end   = *std::ranges::end(v_range);

    // Create iterator and advance to first valid edge
    iterator it(g_, v_begin, v_end, edge_type{}, edge_type{});
    it.advance_to_valid_edge();
    return it;
  }

  [[nodiscard]] constexpr iterator end() const noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_end   = *std::ranges::end(v_range);
    return iterator(g_, v_end, v_end, edge_type{}, edge_type{});
  }

  /// Total number of edges in the graph. Only available when the graph
  /// supports O(1) num_edges (via member or ADL), not the default O(V) fallback.
  [[nodiscard]] constexpr auto size() const noexcept
  requires edgelist_detail::has_const_time_num_edges<G>
  {
    return adj_list::num_edges(*g_);
  }

private:
  G* g_ = nullptr;
};

/**
 * @brief Edgelist view with an edge value function.
 *
 * Flattens the adjacency-list structure into a single range of edges,
 * yielding @c edge_info{sid,tid,uv,val} per edge via structured bindings.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [sid, tid, uv, val] : edgelist(g, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 *
 * @see edgelist_view<G,void> — without value function
 * @see basic_edgelist_view   — simplified (ids only)
 */
template <adj_list::adjacency_list G, class EVF>
class edgelist_view : public std::ranges::view_interface<edgelist_view<G, EVF>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_range_type   = adj_list::vertex_edge_range_t<G>;
  using edge_type         = adj_list::edge_t<G>;
  using value_type_result = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_info<vertex_id_type, true, edge_type, value_type_result>;

  /**
     * @brief Forward iterator yielding @c edge_info{sid, tid, uv, val}.
     *
     * Walks every vertex in order; for each vertex, walks its outgoing edges.
     * @c operator*() invokes the edge value function and returns an
     * @c edge_info with @c source_id, @c target_id, the edge descriptor
     * and the computed value.
     */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(
          G* g, vertex_type v, vertex_type v_end, edge_type current_edge, edge_type edge_end, EVF* evf) noexcept
          : g_(g), v_(v), v_end_(v_end), current_edge_(current_edge), edge_end_(edge_end), evf_(evf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      auto source_id = adj_list::vertex_id(*g_, v_);
      auto target_id = adj_list::target_id(*g_, current_edge_);
      return value_type{static_cast<vertex_id_type>(source_id), static_cast<vertex_id_type>(target_id), current_edge_, std::invoke(*evf_, std::as_const(*g_), current_edge_)};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      if (current_edge_ == edge_end_) {
        ++v_;
        advance_to_valid_edge();
      }
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      if (v_ == v_end_ && other.v_ == other.v_end_) {
        return true;
      }
      if (v_ == v_end_ || other.v_ == other.v_end_) {
        return false;
      }
      return v_ == other.v_ && current_edge_ == other.current_edge_;
    }

    /**
         * @brief Advance to the next vertex that has edges, initializing edge descriptors
         */
    constexpr void advance_to_valid_edge() noexcept {
      while (v_ != v_end_) {
        auto edge_range = adj_list::edges(*g_, v_);
        auto begin_it   = std::ranges::begin(edge_range);
        auto end_it     = std::ranges::end(edge_range);
        if (begin_it != end_it) {
          current_edge_ = *begin_it;
          edge_end_     = *end_it;
          return;
        }
        ++v_;
      }
    }

  private:
    G*          g_ = nullptr;
    vertex_type v_{};            // Current vertex descriptor
    vertex_type v_end_{};        // End sentinel for vertices
    edge_type   current_edge_{}; // Current edge descriptor
    edge_type   edge_end_{};     // End sentinel for current vertex's edges
    EVF*        evf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr edgelist_view() noexcept = default;

  constexpr edgelist_view(G& g, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : g_(&g), evf_(std::move(evf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_begin = *std::ranges::begin(v_range);
    auto v_end   = *std::ranges::end(v_range);

    iterator it(g_, v_begin, v_end, edge_type{}, edge_type{}, &evf_);
    it.advance_to_valid_edge();
    return it;
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_end   = *std::ranges::end(v_range);
    return iterator(g_, v_end, v_end, edge_type{}, edge_type{}, &evf_);
  }

  /// Total number of edges in the graph. Only available when the graph
  /// supports O(1) num_edges (via member or ADL), not the default O(V) fallback.
  [[nodiscard]] constexpr auto size() const noexcept
  requires edgelist_detail::has_const_time_num_edges<G>
  {
    return adj_list::num_edges(*g_);
  }

private:
  G*                        g_ = nullptr;
  [[no_unique_address]] EVF evf_{};
};

// =============================================================================
// Factory Functions
// =============================================================================

/**
 * @brief Create an edgelist view over all edges in an adjacency list (no value function).
 *
 * @code
 *   for (auto [sid, tid, uv] : edgelist(g)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g  The graph to iterate over.  Must outlive the returned view.
 * @return @c edgelist_view yielding @c edge_info{sid, tid, uv} per edge.
 *
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto edgelist(G& g) noexcept {
  return edgelist_view<G, void>(g);
}

/**
 * @brief Create an edgelist view with an edge value function.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [sid, tid, uv, val] : edgelist(g, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  evf Value function invoked once per edge.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c edgelist_view yielding @c edge_info{sid, tid, uv, val} per edge.
 *
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto edgelist(G&    g,
                                      EVF&& evf) noexcept(std::is_nothrow_constructible_v<std::decay_t<EVF>, EVF>) {
  return edgelist_view<G, std::decay_t<EVF>>(g, std::forward<EVF>(evf));
}

// =============================================================================
// basic_edgelist_view — ids only (no edge descriptor in return type)
// =============================================================================

/**
 * @brief Basic edgelist view without value function (ids only).
 *
 * Simplified variant that yields only source and target vertex ids,
 * omitting the edge descriptor.  Use when only ids are needed.
 *
 * @code
 *   for (auto [sid, tid] : basic_edgelist(g)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 *
 * @see basic_edgelist_view<G,EVF> — with value function
 * @see edgelist_view              — standard view (with edge descriptor)
 */
template <adj_list::adjacency_list G>
class basic_edgelist_view<G, void> : public std::ranges::view_interface<basic_edgelist_view<G, void>> {
public:
  using graph_type      = G;
  using vertex_type     = adj_list::vertex_t<G>;
  using vertex_id_type  = adj_list::vertex_id_t<G>;
  using edge_range_type = adj_list::vertex_edge_range_t<G>;
  using edge_type       = adj_list::edge_t<G>;
  using info_type       = edge_info<vertex_id_type, true, void, void>;

  /// @brief Forward iterator yielding @c edge_info{sid, tid} — source/target ids only.
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, vertex_type v, vertex_type v_end, edge_type current_edge, edge_type edge_end) noexcept
          : g_(g), v_(v), v_end_(v_end), current_edge_(current_edge), edge_end_(edge_end) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      return value_type{static_cast<vertex_id_type>(adj_list::vertex_id(*g_, v_)),
                        static_cast<vertex_id_type>(adj_list::target_id(*g_, current_edge_))};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      if (current_edge_ == edge_end_) {
        ++v_;
        advance_to_valid_edge();
      }
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      if (v_ == v_end_ && other.v_ == other.v_end_) {
        return true;
      }
      if (v_ == v_end_ || other.v_ == other.v_end_) {
        return false;
      }
      return v_ == other.v_ && current_edge_ == other.current_edge_;
    }

    constexpr void advance_to_valid_edge() noexcept {
      while (v_ != v_end_) {
        auto edge_range = adj_list::edges(*g_, v_);
        auto begin_it   = std::ranges::begin(edge_range);
        auto end_it     = std::ranges::end(edge_range);
        if (begin_it != end_it) {
          current_edge_ = *begin_it;
          edge_end_     = *end_it;
          return;
        }
        ++v_;
      }
    }

  private:
    G*          g_ = nullptr;
    vertex_type v_{};
    vertex_type v_end_{};
    edge_type   current_edge_{};
    edge_type   edge_end_{};
  };

  using const_iterator = iterator;

  constexpr basic_edgelist_view() noexcept = default;

  constexpr basic_edgelist_view(G& g) noexcept : g_(&g) {}

  [[nodiscard]] constexpr iterator begin() const noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_begin = *std::ranges::begin(v_range);
    auto v_end   = *std::ranges::end(v_range);

    iterator it(g_, v_begin, v_end, edge_type{}, edge_type{});
    it.advance_to_valid_edge();
    return it;
  }

  [[nodiscard]] constexpr iterator end() const noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_end   = *std::ranges::end(v_range);
    return iterator(g_, v_end, v_end, edge_type{}, edge_type{});
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires edgelist_detail::has_const_time_num_edges<G>
  {
    return adj_list::num_edges(*g_);
  }

private:
  G* g_ = nullptr;
};

/**
 * @brief Basic edgelist view with an edge value function (ids + value, no descriptor).
 *
 * Simplified variant that yields source id, target id and the computed
 * edge value, omitting the edge descriptor.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [sid, tid, val] : basic_edgelist(g, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 *
 * @see basic_edgelist_view<G,void> — without value function
 * @see edgelist_view               — standard view (with edge descriptor)
 */
template <adj_list::adjacency_list G, class EVF>
class basic_edgelist_view : public std::ranges::view_interface<basic_edgelist_view<G, EVF>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_range_type   = adj_list::vertex_edge_range_t<G>;
  using edge_type         = adj_list::edge_t<G>;
  using value_type_result = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_info<vertex_id_type, true, void, value_type_result>;

  /// @brief Forward iterator yielding @c edge_info{sid, tid, val} — ids + computed value.
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(
          G* g, vertex_type v, vertex_type v_end, edge_type current_edge, edge_type edge_end, EVF* evf) noexcept
          : g_(g), v_(v), v_end_(v_end), current_edge_(current_edge), edge_end_(edge_end), evf_(evf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      return value_type{static_cast<vertex_id_type>(adj_list::vertex_id(*g_, v_)),
                        static_cast<vertex_id_type>(adj_list::target_id(*g_, current_edge_)),
                        std::invoke(*evf_, std::as_const(*g_), current_edge_)};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      if (current_edge_ == edge_end_) {
        ++v_;
        advance_to_valid_edge();
      }
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      if (v_ == v_end_ && other.v_ == other.v_end_) {
        return true;
      }
      if (v_ == v_end_ || other.v_ == other.v_end_) {
        return false;
      }
      return v_ == other.v_ && current_edge_ == other.current_edge_;
    }

    constexpr void advance_to_valid_edge() noexcept {
      while (v_ != v_end_) {
        auto edge_range = adj_list::edges(*g_, v_);
        auto begin_it   = std::ranges::begin(edge_range);
        auto end_it     = std::ranges::end(edge_range);
        if (begin_it != end_it) {
          current_edge_ = *begin_it;
          edge_end_     = *end_it;
          return;
        }
        ++v_;
      }
    }

  private:
    G*          g_ = nullptr;
    vertex_type v_{};
    vertex_type v_end_{};
    edge_type   current_edge_{};
    edge_type   edge_end_{};
    EVF*        evf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr basic_edgelist_view() noexcept = default;

  constexpr basic_edgelist_view(basic_edgelist_view&&)            = default;
  constexpr basic_edgelist_view& operator=(basic_edgelist_view&&) = default;

  constexpr basic_edgelist_view(const basic_edgelist_view&)            = default;
  constexpr basic_edgelist_view& operator=(const basic_edgelist_view&) = default;

  constexpr basic_edgelist_view(G& g, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : g_(&g), evf_(std::move(evf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_begin = *std::ranges::begin(v_range);
    auto v_end   = *std::ranges::end(v_range);

    iterator it(g_, v_begin, v_end, edge_type{}, edge_type{}, &evf_);
    it.advance_to_valid_edge();
    return it;
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    auto v_range = adj_list::vertices(*g_);
    auto v_end   = *std::ranges::end(v_range);
    return iterator(g_, v_end, v_end, edge_type{}, edge_type{}, &evf_);
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires edgelist_detail::has_const_time_num_edges<G>
  {
    return adj_list::num_edges(*g_);
  }

private:
  G*                        g_ = nullptr;
  [[no_unique_address]] EVF evf_{};
};

// Deduction guides for basic_edgelist_view
template <adj_list::adjacency_list G>
basic_edgelist_view(G&) -> basic_edgelist_view<G, void>;

template <adj_list::adjacency_list G, class EVF>
basic_edgelist_view(G&, EVF) -> basic_edgelist_view<G, EVF>;

// =============================================================================
// Factory functions: basic_edgelist
// =============================================================================

/**
 * @brief Create a basic edgelist view (source + target ids only, no descriptor).
 *
 * @code
 *   for (auto [sid, tid] : basic_edgelist(g)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g  The graph to iterate over.  Must outlive the returned view.
 * @return @c basic_edgelist_view yielding @c edge_info{sid, tid} per edge.
 *
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_edgelist(G& g) noexcept {
  return basic_edgelist_view<G, void>(g);
}

/**
 * @brief Create a basic edgelist view with an edge value function
 *        (source + target ids + value, no descriptor).
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [sid, tid, val] : basic_edgelist(g, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  evf Value function invoked once per edge.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c basic_edgelist_view yielding @c edge_info{sid, tid, val} per edge.
 *
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto basic_edgelist(G& g, EVF&& evf) {
  return basic_edgelist_view<G, std::decay_t<EVF>>(g, std::forward<EVF>(evf));
}

// =============================================================================
// Edge List Views (for edge_list data structures)
// =============================================================================

// Forward declaration
template <edge_list::basic_sourced_edgelist EL, class EVF = void>
class edge_list_edgelist_view;

/**
 * @brief Edgelist view wrapping a native @c edge_list data structure (no value function).
 *
 * Unlike the adjacency-list overloads above, this view iterates directly over
 * an @c edge_list range, yielding @c edge_info{sid,tid,uv} per edge.
 *
 * @code
 *   for (auto [sid, tid, uv] : edgelist(el)) { ... }
 * @endcode
 *
 * @tparam EL Edge list type satisfying @c basic_sourced_edgelist
 *
 * @see edge_list_edgelist_view<EL,EVF> — with value function
 * @see edgelist_view                   — adjacency-list counterpart
 */
template <edge_list::basic_sourced_edgelist EL>
class edge_list_edgelist_view<EL, void> : public std::ranges::view_interface<edge_list_edgelist_view<EL, void>> {
public:
  using edge_list_type = EL;
  using edge_type      = edge_list::edge_t<EL>;
  using vertex_id_type = edge_list::vertex_id_t<EL>;
  using info_type      = edge_info<vertex_id_type, true, edge_type, void>;

  /**
     * @brief Forward iterator yielding @c edge_info{sid, tid, uv} from the edge list.
     */
  class iterator {
  public:
    using base_iterator     = std::ranges::iterator_t<EL>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(EL* el, base_iterator it) noexcept : el_(el), current_(it) {}

    [[nodiscard]] constexpr value_type operator*() const {
      auto& edge = *current_;
      return value_type{graph::source_id(*el_, edge), graph::target_id(*el_, edge), edge};
    }

    constexpr iterator& operator++() noexcept {
      ++current_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++current_;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept { return current_ == other.current_; }

  private:
    EL*           el_ = nullptr;
    base_iterator current_{};
  };

  using const_iterator = iterator;

  constexpr edge_list_edgelist_view() noexcept = default;

  constexpr edge_list_edgelist_view(EL& el) noexcept : el_(&el) {}

  [[nodiscard]] constexpr iterator begin() const noexcept { return iterator(el_, std::ranges::begin(*el_)); }

  [[nodiscard]] constexpr iterator end() const noexcept { return iterator(el_, std::ranges::end(*el_)); }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<EL>
  {
    return std::ranges::size(*el_);
  }

private:
  EL* el_ = nullptr;
};

/**
 * @brief Edgelist view wrapping a native @c edge_list with a value function.
 *
 * Iterates directly over an @c edge_list range, yielding
 * @c edge_info{sid,tid,uv,val} per edge.
 *
 * @code
 *   auto evf = [](auto& el, auto& e) { return graph::edge_value(el, e); };
 *   for (auto [sid, tid, uv, val] : edgelist(el, evf)) { ... }
 * @endcode
 *
 * @tparam EL  Edge list type satisfying @c basic_sourced_edgelist
 * @tparam EVF Edge value function — @c invocable<EL&, edge_t<EL>>
 *
 * @see edge_list_edgelist_view<EL,void> — without value function
 * @see edgelist_view                    — adjacency-list counterpart
 */
template <edge_list::basic_sourced_edgelist EL, class EVF>
class edge_list_edgelist_view : public std::ranges::view_interface<edge_list_edgelist_view<EL, EVF>> {
public:
  using edge_list_type    = EL;
  using edge_type         = edge_list::edge_t<EL>;
  using vertex_id_type    = edge_list::vertex_id_t<EL>;
  using value_type_result = std::invoke_result_t<EVF, EL&, edge_type>;
  using info_type         = edge_info<vertex_id_type, true, edge_type, value_type_result>;

  /**
     * @brief Forward iterator yielding @c edge_info{sid, tid, uv, val} from the edge list.
     */
  class iterator {
  public:
    using base_iterator     = std::ranges::iterator_t<EL>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(EL* el, base_iterator it, EVF* evf) noexcept : el_(el), current_(it), evf_(evf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      auto& edge = *current_;
      return value_type{graph::source_id(*el_, edge), graph::target_id(*el_, edge), edge,
                        std::invoke(*evf_, *el_, edge)};
    }

    constexpr iterator& operator++() noexcept {
      ++current_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++current_;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept { return current_ == other.current_; }

  private:
    EL*           el_ = nullptr;
    base_iterator current_{};
    EVF*          evf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr edge_list_edgelist_view() noexcept = default;

  constexpr edge_list_edgelist_view(EL& el, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : el_(&el), evf_(std::move(evf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept { return iterator(el_, std::ranges::begin(*el_), &evf_); }

  [[nodiscard]] constexpr iterator end() noexcept { return iterator(el_, std::ranges::end(*el_), &evf_); }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<EL>
  {
    return std::ranges::size(*el_);
  }

private:
  EL*                       el_ = nullptr;
  [[no_unique_address]] EVF evf_{};
};

// =============================================================================
// Factory Functions for edge_list
// =============================================================================

/**
 * @brief Create an edgelist view over a native @c edge_list (no value function).
 *
 * @code
 *   for (auto [sid, tid, uv] : edgelist(el)) { ... }
 * @endcode
 *
 * @tparam EL Edge list type satisfying @c basic_sourced_edgelist
 * @param  el The edge list to iterate over.  Must outlive the returned view.
 * @return @c edge_list_edgelist_view yielding @c edge_info{sid, tid, uv} per edge.
 *
 * @post The edge list is not modified.
 */
template <edge_list::basic_sourced_edgelist EL>
requires(!adj_list::adjacency_list<EL>) // Disambiguation: prefer adjacency_list overload
[[nodiscard]] constexpr auto edgelist(EL& el) noexcept {
  return edge_list_edgelist_view<EL, void>(el);
}

/**
 * @brief Create an edgelist view over a native @c edge_list with a value function.
 *
 * @code
 *   auto evf = [](auto& el, auto& e) { return graph::edge_value(el, e); };
 *   for (auto [sid, tid, uv, val] : edgelist(el, evf)) { ... }
 * @endcode
 *
 * @tparam EL  Edge list type satisfying @c basic_sourced_edgelist
 * @tparam EVF Edge value function — @c invocable<EL&, edge_t<EL>>
 * @param  el  The edge list to iterate over.  Must outlive the returned view.
 * @param  evf Value function invoked once per edge.
 * @return @c edge_list_edgelist_view yielding @c edge_info{sid, tid, uv, val} per edge.
 *
 * @post The edge list is not modified.
 */
template <edge_list::basic_sourced_edgelist EL, class EVF>
requires(!adj_list::adjacency_list<EL>) && // Disambiguation: prefer adjacency_list overload
        std::invocable<EVF, EL&, edge_list::edge_t<EL>>
[[nodiscard]] constexpr auto edgelist(EL&   el,
                                      EVF&& evf) noexcept(std::is_nothrow_constructible_v<std::decay_t<EVF>, EVF>) {
  return edge_list_edgelist_view<EL, std::decay_t<EVF>>(el, std::forward<EVF>(evf));
}

} // namespace graph::views
