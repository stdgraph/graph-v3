/**
 * @file incidence.hpp
 * @brief Incidence views for iterating over edges incident to a vertex.
 *
 * @section overview Overview
 *
 * Provides lazy, range-based views that iterate over every outgoing edge from
 * a given source vertex.  Each iteration step yields an @c edge_info whose
 * fields are exposed via structured bindings.  An optional edge value function
 * (EVF) computes a per-edge value that is included in the binding.
 *
 * @section variants View Variants
 *
 * | Variant                          | Structured Binding | Description                        |
 * |----------------------------------|--------------------|------------------------------------|
 * | @c incidence(g,u)                | `[tid, uv]`        | Standard view (target id + edge)   |
 * | @c incidence(g,u,evf)            | `[tid, uv, val]`   | Standard view with value function  |
 * | @c basic_incidence(g,uid)        | `[tid]`            | Simplified view (target id only)   |
 * | @c basic_incidence(g,uid,evf)    | `[tid, val]`       | Simplified view with value fn      |
 *
 * The standard overloads also accept a vertex id @c uid in place of a
 * descriptor @c u (requires @c index_adjacency_list).
 *
 * @section bindings Structured Bindings
 *
 * Standard view:
 * @code
 *   for (auto [tid, uv]      : incidence(g, u))          // tid = vertex_id_t<G>, uv = edge_t<G>
 *   for (auto [tid, uv, val] : incidence(g, u, evf))     // val = invoke_result_t<EVF, G&, edge_t<G>>
 * @endcode
 *
 * basic_ variant:
 * @code
 *   for (auto [tid]      : basic_incidence(g, uid))       // tid = vertex_id_t<G>
 *   for (auto [tid, val] : basic_incidence(g, uid, evf))  // val = invoke_result_t<EVF, G&, edge_t<G>>
 * @endcode
 *
 * @section iterator_properties Iterator Properties
 *
 * | Property        | Value                                                  |
 * |-----------------|--------------------------------------------------------|
 * | Concept         | @c std::forward_iterator                               |
 * | Range concept   | @c std::ranges::forward_range                          |
 * | Sized           | Yes when @c vertex_edge_range_t<G> is @c sized_range   |
 * | Borrowed        | No (view holds reference)                              |
 * | Common          | Yes (begin/end same type)                              |
 *
 * @section perf Performance Characteristics
 *
 * Construction is O(1).  Iteration is O(deg(u)), one edge per step.  The view
 * holds only a pointer to the graph and the source vertex — no allocation.
 * The @c basic_ variant is lighter still: it never materialises an edge
 * descriptor and returns only the target vertex id.
 *
 * @section chaining Chaining with std::views
 *
 * Views chain with std::views when the value function is a stateless lambda
 * (empty capture list @c []):
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   auto view = g | incidence(0, evf)
 *                 | std::views::take(3);   // ✅ compiles
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
 *   for (auto [tid, uv] : g | incidence(0))              { ... }
 *   for (auto [tid]     : g | basic_incidence(0))        { ... }
 *   for (auto [tid, val]: g | basic_incidence(0, evf))   { ... }
 * @endcode
 *
 * @section supported_graphs Supported Graph Properties
 *
 * - Requires: @c adjacency_list concept
 *   (vertex-id overloads require @c index_adjacency_list)
 * - Works with all @c dynamic_graph container combinations
 * - Works with directed and undirected graphs
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
 * - The source vertex @c u / @c uid must be a valid vertex in the graph.
 *
 * @section see_also See Also
 *
 * - @ref views.md — all views overview
 * - @ref graph_cpo_implementation.md — CPO documentation
 * - @ref view_chaining_limitations.md — chaining design rationale
 * - @ref vertexlist.hpp — whole-graph vertex iteration
 * - @ref neighbors.hpp — adjacent-vertex iteration (without edge descriptors)
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <graph/graph_info.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/edge_accessor.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::adjacency_list G, class EVF = void, class Accessor = out_edge_accessor>
class incidence_view;

template <adj_list::adjacency_list G, class EVF = void, class Accessor = out_edge_accessor>
class basic_incidence_view;

/**
 * @brief Incidence view — standard variant without value function.
 *
 * Iterates over every outgoing edge from a source vertex, yielding
 * @c edge_info<vertex_id_type,false,edge_t<G>,void> per step.
 *
 * Structured binding: @c auto [tid, uv] where
 * - @c tid — @c vertex_id_t<G> (target vertex identifier)
 * - @c uv  — @c edge_t<G>      (edge descriptor)
 *
 * @par Iterator category
 * @c std::forward_iterator — sized when @c vertex_edge_range_t<G> is
 * @c sized_range, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(deg(u)).  Zero allocation.
 *
 * @tparam G Graph type satisfying @c adjacency_list
 *
 * @see incidence(G&, vertex_t<G>) — factory function
 * @see basic_incidence_view — simplified target-id-only variant
 */
template <adj_list::adjacency_list G, class Accessor>
class incidence_view<G, void, Accessor> : public std::ranges::view_interface<incidence_view<G, void, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using info_type          = edge_info<vertex_id_type, false, edge_type, void>;

  /**
   * @brief Forward iterator yielding @c edge_info{tid, uv} per edge.
   *
   * Satisfies @c std::forward_iterator.  All operations are @c noexcept.
   */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, edge_type e) noexcept : g_(g), current_(e) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      return info_type{static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_)), current_};
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
    G*        g_ = nullptr;
    edge_type current_{};
  };

  using const_iterator = iterator;

  constexpr incidence_view() noexcept = default;

  constexpr incidence_view(G& g, vertex_type u) noexcept : g_(&g), source_(u) {}

  [[nodiscard]] constexpr iterator begin() const noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::begin(edge_range));
  }

  [[nodiscard]] constexpr iterator end() const noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    // edge_descriptor_view's end iterator can be dereferenced to get
    // an edge_descriptor with the end storage position
    return iterator(g_, *std::ranges::end(edge_range));
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<edge_range_type>
  {
    return std::ranges::size(Accessor{}.edges(*g_, source_));
  }

private:
  G*          g_ = nullptr;
  vertex_type source_{};
};

/**
 * @brief Incidence view — standard variant with value function.
 *
 * Iterates over every outgoing edge from a source vertex, yielding
 * @c edge_info<vertex_id_type,false,edge_t<G>,EV> where @c EV =
 * @c invoke_result_t<EVF,const G&,edge_t<G>> .
 *
 * Structured binding: @c auto [tid, uv, val] where
 * - @c tid — @c vertex_id_t<G> (target vertex identifier)
 * - @c uv  — @c edge_t<G>      (edge descriptor)
 * - @c val — computed value returned by EVF
 *
 * @par Chaining with std::views
 * Use a stateless lambda (empty capture @c []) for the value function so
 * the view satisfies @c std::ranges::view and chains with @c std::views
 * adaptors.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized when @c vertex_edge_range_t<G> is
 * @c sized_range, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(deg(u)), invoking EVF once per edge.
 * Zero allocation.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 *
 * @see incidence(G&, vertex_t<G>, EVF&&) — factory function
 * @see basic_incidence_view — simplified target-id-only variant
 */
template <adj_list::adjacency_list G, class EVF, class Accessor>
class incidence_view : public std::ranges::view_interface<incidence_view<G, EVF, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using value_type_result  = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type          = edge_info<vertex_id_type, false, edge_type, value_type_result>;

  /**
   * @brief Forward iterator yielding @c edge_info{tid, uv, val} per edge.
   *
   * Satisfies @c std::forward_iterator.  @c operator*() may throw if EVF throws.
   */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, edge_type e, EVF* evf) noexcept : g_(g), current_(e), evf_(evf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      return value_type{static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_)), current_, std::invoke(*evf_, std::as_const(*g_), current_)};
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
    G*        g_ = nullptr;
    edge_type current_{};
    EVF*      evf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr incidence_view() noexcept = default;

  constexpr incidence_view(G& g, vertex_type u, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : g_(&g), source_(u), evf_(std::move(evf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::begin(edge_range), &evf_);
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::end(edge_range), &evf_);
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<edge_range_type>
  {
    return std::ranges::size(Accessor{}.edges(*g_, source_));
  }

private:
  G*                        g_ = nullptr;
  vertex_type               source_{};
  [[no_unique_address]] EVF evf_{};
};

// Deduction guides
template <adj_list::adjacency_list G>
incidence_view(G&, adj_list::vertex_t<G>) -> incidence_view<G, void, out_edge_accessor>;

template <adj_list::adjacency_list G, class EVF>
incidence_view(G&, adj_list::vertex_t<G>, EVF) -> incidence_view<G, EVF, out_edge_accessor>;

/**
 * @brief Create an incidence view over edges from a vertex (no value function).
 *
 * @code
 *   for (auto [tid, uv] : incidence(g, u)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g  The graph to iterate over.  Must outlive the returned view.
 * @param  u  The source vertex descriptor.
 * @return @c incidence_view yielding @c edge_info{tid, uv} per edge.
 *
 * @pre  @c u is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_t<G> u) noexcept {
  return incidence_view<G, void>(g, u);
}

/**
 * @brief Create an incidence view with an edge value function.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [tid, uv, val] : incidence(g, u, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  u   The source vertex descriptor.
 * @param  evf Value function invoked once per edge.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c incidence_view yielding @c edge_info{tid, uv, val} per edge.
 *
 * @pre  @c u is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_t<G> u, EVF&& evf) {
  return incidence_view<G, std::decay_t<EVF>>(g, u, std::forward<EVF>(evf));
}

/**
 * @brief Create an incidence view from a vertex id (convenience overload).
 *
 * Resolves @c uid to a descriptor via @c find_vertex and delegates to the
 * descriptor-based overload.
 *
 * @code
 *   for (auto [tid, uv] : incidence(g, uid)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g   The graph to iterate over.
 * @param  uid The source vertex id.
 * @return @c incidence_view yielding @c edge_info{tid, uv} per edge.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto incidence(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return incidence(g, u);
}

/**
 * @brief Create an incidence view with value function from a vertex id
 *        (convenience overload).
 *
 * Resolves @c uid to a descriptor via @c find_vertex and delegates to the
 * descriptor-based overload.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return edge_value(g, uv); };
 *   for (auto [tid, uv, val] : incidence(g, uid, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g   The graph to iterate over.
 * @param  uid The source vertex id.
 * @param  evf Value function invoked once per edge.
 * @return @c incidence_view yielding @c edge_info{tid, uv, val} per edge.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto incidence(G& g, const adj_list::vertex_id_t<G>& uid, EVF&& evf) {
  auto u = *adj_list::find_vertex(g, uid);
  return incidence(g, u, std::forward<EVF>(evf));
}

// =============================================================================
// basic_incidence_view — id only (no edge descriptor in return type)
// =============================================================================

/**
 * @brief Basic incidence view — simplified variant without value function.
 *
 * Iterates over every outgoing edge from a source vertex, yielding
 * @c edge_info<vertex_id_type,false,void,void>.  No edge descriptor is
 * materialised — only the target vertex id is returned, making this the
 * lightest-weight edge iteration available from a single vertex.
 *
 * Structured binding: @c auto [tid] where
 * - @c tid — @c vertex_id_t<G> (target vertex identifier)
 *
 * @par When to use
 * Prefer @c basic_incidence when you only need target IDs (e.g. for
 * connectivity traversal in algorithms such as BFS, DFS, topological sort).
 * For access to the edge descriptor or stored edge value, use
 * @c incidence_view instead.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized when @c vertex_edge_range_t<G> is
 * @c sized_range, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(deg(u)).  Zero allocation.  Avoids
 * the edge descriptor that the standard variant returns.
 *
 * @tparam G Graph type satisfying @c adjacency_list
 *
 * @see basic_incidence(G&, vertex_id_t<G>) — factory function
 * @see incidence_view — standard variant with edge descriptor
 */
template <adj_list::adjacency_list G, class Accessor>
class basic_incidence_view<G, void, Accessor> : public std::ranges::view_interface<basic_incidence_view<G, void, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using info_type          = edge_info<vertex_id_type, false, void, void>;

  /**
   * @brief Forward iterator yielding @c edge_info{tid} per edge.
   *
   * Satisfies @c std::forward_iterator.  All operations are @c noexcept.
   */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, edge_type e) noexcept : g_(g), current_(e) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      return value_type{static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_))};
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
    G*        g_ = nullptr;
    edge_type current_{};
  };

  using const_iterator = iterator;

  constexpr basic_incidence_view() noexcept = default;

  constexpr basic_incidence_view(G& g, vertex_type u) noexcept : g_(&g), source_(u) {}

  [[nodiscard]] constexpr iterator begin() const noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::begin(edge_range));
  }

  [[nodiscard]] constexpr iterator end() const noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::end(edge_range));
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<edge_range_type>
  {
    return std::ranges::size(Accessor{}.edges(*g_, source_));
  }

private:
  G*          g_ = nullptr;
  vertex_type source_{};
};

/**
 * @brief Basic incidence view — simplified variant with value function.
 *
 * Iterates over every outgoing edge from a source vertex, yielding
 * @c edge_info<vertex_id_type,false,void,EV> where @c EV =
 * @c invoke_result_t<EVF,const G&,edge_t<G>> .  No edge descriptor is
 * materialised.
 *
 * Structured binding: @c auto [tid, val] where
 * - @c tid — @c vertex_id_t<G> (target vertex identifier)
 * - @c val — computed value returned by EVF
 *
 * @par Chaining with std::views
 * Use a stateless lambda (empty capture @c []) for the value function so
 * the view satisfies @c std::ranges::view and chains with @c std::views
 * adaptors.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized when @c vertex_edge_range_t<G> is
 * @c sized_range, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(deg(u)), invoking EVF once per edge.
 * Zero allocation.  Avoids the edge descriptor that the standard variant
 * returns.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 *
 * @see basic_incidence(G&, vertex_id_t<G>, EVF&&) — factory function
 * @see incidence_view — standard variant with edge descriptor
 */
template <adj_list::adjacency_list G, class EVF, class Accessor>
class basic_incidence_view : public std::ranges::view_interface<basic_incidence_view<G, EVF, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using value_type_result  = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type          = edge_info<vertex_id_type, false, void, value_type_result>;

  /**
   * @brief Forward iterator yielding @c edge_info{tid, val} per edge.
   *
   * Satisfies @c std::forward_iterator.  @c operator*() may throw if EVF throws.
   */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, edge_type e, EVF* evf) noexcept : g_(g), current_(e), evf_(evf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      return value_type{static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_)), std::invoke(*evf_, std::as_const(*g_), current_)};
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
    G*        g_ = nullptr;
    edge_type current_{};
    EVF*      evf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr basic_incidence_view() noexcept = default;

  constexpr basic_incidence_view(basic_incidence_view&&)            = default;
  constexpr basic_incidence_view& operator=(basic_incidence_view&&) = default;

  constexpr basic_incidence_view(const basic_incidence_view&)            = default;
  constexpr basic_incidence_view& operator=(const basic_incidence_view&) = default;

  constexpr basic_incidence_view(G& g, vertex_type u, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : g_(&g), source_(u), evf_(std::move(evf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::begin(edge_range), &evf_);
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::end(edge_range), &evf_);
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<edge_range_type>
  {
    return std::ranges::size(Accessor{}.edges(*g_, source_));
  }

private:
  G*                        g_ = nullptr;
  vertex_type               source_{};
  [[no_unique_address]] EVF evf_{};
};

// Deduction guides for basic_incidence_view
template <adj_list::adjacency_list G>
basic_incidence_view(G&, adj_list::vertex_t<G>) -> basic_incidence_view<G, void, out_edge_accessor>;

template <adj_list::adjacency_list G, class EVF>
basic_incidence_view(G&, adj_list::vertex_t<G>, EVF) -> basic_incidence_view<G, EVF, out_edge_accessor>;

// =============================================================================
// Factory functions: basic_incidence
// =============================================================================

/**
 * @brief Create a basic incidence view (target id only, no edge descriptor).
 *
 * @code
 *   for (auto [tid] : basic_incidence(g, uid)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  uid The source vertex id.
 * @return @c basic_incidence_view yielding @c edge_info{tid} per edge.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_incidence(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_incidence_view<G, void>(g, u);
}

/**
 * @brief Create a basic incidence view with value function (target id + value).
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return edge_value(g, uv); };
 *   for (auto [tid, val] : basic_incidence(g, uid, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  uid The source vertex id.
 * @param  evf Value function invoked once per edge.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c basic_incidence_view yielding @c edge_info{tid, val} per edge.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto basic_incidence(G& g, const adj_list::vertex_id_t<G>& uid, EVF&& evf) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_incidence_view<G, std::decay_t<EVF>>(g, u, std::forward<EVF>(evf));
}

// =============================================================================
// Explicit outgoing factories: out_incidence / basic_out_incidence
// =============================================================================

/// @brief Create an outgoing incidence view (no value function).
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto out_incidence(G& g, adj_list::vertex_t<G> u) noexcept {
  return incidence_view<G, void, out_edge_accessor>(g, u);
}

/// @brief Create an outgoing incidence view with edge value function.
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto out_incidence(G& g, adj_list::vertex_t<G> u, EVF&& evf) {
  return incidence_view<G, std::decay_t<EVF>, out_edge_accessor>(g, u, std::forward<EVF>(evf));
}

/// @brief Create an outgoing incidence view from vertex id.
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto out_incidence(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return out_incidence(g, u);
}

/// @brief Create an outgoing incidence view with EVF from vertex id.
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto out_incidence(G& g, const adj_list::vertex_id_t<G>& uid, EVF&& evf) {
  auto u = *adj_list::find_vertex(g, uid);
  return out_incidence(g, u, std::forward<EVF>(evf));
}

/// @brief Create a basic outgoing incidence view (target id only).
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_out_incidence(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_incidence_view<G, void, out_edge_accessor>(g, u);
}

/// @brief Create a basic outgoing incidence view with EVF.
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto basic_out_incidence(G& g, const adj_list::vertex_id_t<G>& uid, EVF&& evf) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_incidence_view<G, std::decay_t<EVF>, out_edge_accessor>(g, u, std::forward<EVF>(evf));
}

// =============================================================================
// Incoming factories: in_incidence / basic_in_incidence
// =============================================================================

/// @brief Create an incoming incidence view (no value function).
template <adj_list::bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto in_incidence(G& g, adj_list::vertex_t<G> u) noexcept {
  return incidence_view<G, void, in_edge_accessor>(g, u);
}

/// @brief Create an incoming incidence view with edge value function.
template <adj_list::bidirectional_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::in_edge_t<G>>
[[nodiscard]] constexpr auto in_incidence(G& g, adj_list::vertex_t<G> u, EVF&& evf) {
  return incidence_view<G, std::decay_t<EVF>, in_edge_accessor>(g, u, std::forward<EVF>(evf));
}

/// @brief Create an incoming incidence view from vertex id.
template <adj_list::index_bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto in_incidence(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return in_incidence(g, u);
}

/// @brief Create an incoming incidence view with EVF from vertex id.
template <adj_list::index_bidirectional_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::in_edge_t<G>>
[[nodiscard]] constexpr auto in_incidence(G& g, const adj_list::vertex_id_t<G>& uid, EVF&& evf) {
  auto u = *adj_list::find_vertex(g, uid);
  return in_incidence(g, u, std::forward<EVF>(evf));
}

/// @brief Create a basic incoming incidence view (source id only).
template <adj_list::index_bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto basic_in_incidence(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_incidence_view<G, void, in_edge_accessor>(g, u);
}

/// @brief Create a basic incoming incidence view with EVF.
template <adj_list::index_bidirectional_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::in_edge_t<G>>
[[nodiscard]] constexpr auto basic_in_incidence(G& g, const adj_list::vertex_id_t<G>& uid, EVF&& evf) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_incidence_view<G, std::decay_t<EVF>, in_edge_accessor>(g, u, std::forward<EVF>(evf));
}

} // namespace graph::views
