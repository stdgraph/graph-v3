/**
 * @file neighbors.hpp
 * @brief Neighbors views for iterating over vertices adjacent to a source vertex.
 *
 * @section overview Overview
 *
 * Provides lazy, range-based views that iterate over every neighbor (target
 * vertex) reachable from a given source vertex via outgoing edges.  Each
 * iteration step yields a @c neighbor_data whose fields are exposed via
 * structured bindings.  An optional vertex value function (VVF) computes a
 * per-neighbor value that is included in the binding.
 *
 * Unlike @ref incidence.hpp, which yields edge descriptors, neighbors views
 * yield the target *vertex* descriptor, giving direct access to vertex
 * properties without an extra @c target(g,uv) call.
 *
 * @section variants View Variants
 *
 * | Variant                          | Structured Binding | Description                         |
 * |----------------------------------|--------------------|-------------------------------------|
 * | @c neighbors(g,u)                | `[tid, n]`         | Standard view (id + descriptor)     |
 * | @c neighbors(g,u,vvf)            | `[tid, n, val]`    | Standard view with value function   |
 * | @c basic_neighbors(g,uid)        | `[tid]`            | Simplified view (target id only)    |
 * | @c basic_neighbors(g,uid,vvf)    | `[tid, val]`       | Simplified view with value fn       |
 *
 * The standard overloads also accept a vertex id @c uid in place of a
 * descriptor @c u (requires @c index_adjacency_list).
 *
 * @section bindings Structured Bindings
 *
 * Standard view:
 * @code
 *   for (auto [tid, n]      : neighbors(g, u))          // tid = vertex_id_t<G>, n = vertex_t<G>
 *   for (auto [tid, n, val] : neighbors(g, u, vvf))     // val = invoke_result_t<VVF, G&, vertex_t<G>>
 * @endcode
 *
 * basic_ variant:
 * @code
 *   for (auto [tid]      : basic_neighbors(g, uid))      // tid = vertex_id_t<G>
 *   for (auto [tid, val] : basic_neighbors(g, uid, vvf)) // val = invoke_result_t<VVF, G&, vertex_t<G>>
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
 * Construction is O(1).  Iteration is O(deg(u)), one neighbor per step.  The
 * view holds only a pointer to the graph and the source vertex — no
 * allocation.  The @c basic_ variant is lighter still: it never materialises
 * a target vertex descriptor and returns only the target id.
 *
 * @section chaining Chaining with std::views
 *
 * Views chain with std::views when the value function is a stateless lambda
 * (empty capture list @c []):
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
 *   auto view = g | neighbors(0, vvf)
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
 *   for (auto [tid, n]  : g | neighbors(0))              { ... }
 *   for (auto [tid]     : g | basic_neighbors(0))        { ... }
 *   for (auto [tid, val]: g | basic_neighbors(0, vvf))   { ... }
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
 * Construction is @c noexcept when VVF is nothrow move-constructible (or
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
 * - @ref incidence.hpp — per-vertex edge iteration (yields edge descriptors)
 * - @ref vertexlist.hpp — whole-graph vertex iteration
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <graph/graph_data.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/edge_accessor.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::adjacency_list G, class VVF = void, class Accessor = out_edge_accessor>
class neighbors_view;

template <adj_list::adjacency_list G, class VVF = void, class Accessor = out_edge_accessor>
class basic_neighbors_view;

/**
 * @brief Neighbors view — standard variant without value function.
 *
 * Iterates over every neighbor of a source vertex, yielding
 * @c neighbor_data<vertex_id_type,false,vertex_t<G>,void> per step.
 *
 * Structured binding: @c auto [tid, n] where
 * - @c tid — @c vertex_id_t<G> (target / neighbor vertex identifier)
 * - @c n   — @c vertex_t<G>    (target vertex descriptor)
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
 * @see neighbors(G&, vertex_t<G>) — factory function
 * @see basic_neighbors_view — simplified target-id-only variant
 * @see incidence_view — edge-descriptor variant
 */
template <adj_list::adjacency_list G, class Accessor>
class neighbors_view<G, void, Accessor> : public std::ranges::view_interface<neighbors_view<G, void, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using info_type          = neighbor_data<vertex_id_type, false, vertex_type, void>;

  /**
   * @brief Forward iterator yielding @c neighbor_data{tid, n} per neighbor.
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

    constexpr iterator(G* g, edge_type e) noexcept : g_(g), current_edge_(e) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      auto nbr    = Accessor{}.neighbor(*g_, current_edge_);
      auto nbr_id = static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_edge_));
      return value_type{nbr_id, nbr};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++current_edge_;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      return current_edge_ == other.current_edge_;
    }

  private:
    G*        g_ = nullptr;
    edge_type current_edge_{};
  };

  using const_iterator = iterator;

  constexpr neighbors_view() noexcept = default;

  constexpr neighbors_view(G& g, vertex_type u) noexcept : g_(&g), source_(u) {}

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
 * @brief Neighbors view — standard variant with value function.
 *
 * Iterates over every neighbor of a source vertex, yielding
 * @c neighbor_data<vertex_id_type,false,vertex_t<G>,VV> where @c VV =
 * @c invoke_result_t<VVF,const G&,vertex_t<G>> .
 *
 * Structured binding: @c auto [tid, n, val] where
 * - @c tid — @c vertex_id_t<G> (target / neighbor vertex identifier)
 * - @c n   — @c vertex_t<G>    (target vertex descriptor)
 * - @c val — computed value returned by VVF
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
 * Construction O(1).  Full iteration O(deg(u)), invoking VVF once per
 * neighbor.  Zero allocation.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 *
 * @see neighbors(G&, vertex_t<G>, VVF&&) — factory function
 * @see basic_neighbors_view — simplified target-id-only variant
 */
template <adj_list::adjacency_list G, class VVF, class Accessor>
class neighbors_view : public std::ranges::view_interface<neighbors_view<G, VVF, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using value_type_result  = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type          = neighbor_data<vertex_id_type, false, vertex_type, value_type_result>;

  /**
   * @brief Forward iterator yielding @c neighbor_data{tid, n, val} per neighbor.
   *
   * Satisfies @c std::forward_iterator.  @c operator*() may throw if VVF throws.
   */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, edge_type e, VVF* vvf) noexcept : g_(g), current_edge_(e), vvf_(vvf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      auto nbr    = Accessor{}.neighbor(*g_, current_edge_);
      auto nbr_id = static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_edge_));
      return value_type{nbr_id, nbr, std::invoke(*vvf_, std::as_const(*g_), nbr)};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++current_edge_;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      return current_edge_ == other.current_edge_;
    }

  private:
    G*        g_ = nullptr;
    edge_type current_edge_{};
    VVF*      vvf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr neighbors_view() noexcept = default;

  constexpr neighbors_view(G& g, vertex_type u, VVF vvf) noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), source_(u), vvf_(std::move(vvf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::begin(edge_range), &vvf_);
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::end(edge_range), &vvf_);
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<edge_range_type>
  {
    return std::ranges::size(Accessor{}.edges(*g_, source_));
  }

private:
  G*                        g_ = nullptr;
  vertex_type               source_{};
  [[no_unique_address]] VVF vvf_{};
};

// Deduction guides
template <adj_list::adjacency_list G>
neighbors_view(G&, adj_list::vertex_t<G>) -> neighbors_view<G, void, out_edge_accessor>;

template <adj_list::adjacency_list G, class VVF>
neighbors_view(G&, adj_list::vertex_t<G>, VVF) -> neighbors_view<G, VVF, out_edge_accessor>;

/**
 * @brief Create a neighbors view over adjacent vertices (no value function).
 *
 * @code
 *   for (auto [tid, n] : neighbors(g, u)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g  The graph to iterate over.  Must outlive the returned view.
 * @param  u  The source vertex descriptor.
 * @return @c neighbors_view yielding @c neighbor_data{tid, n} per neighbor.
 *
 * @pre  @c u is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto neighbors(G& g, adj_list::vertex_t<G> u) noexcept {
  return neighbors_view<G, void>(g, u);
}

/**
 * @brief Create a neighbors view from a vertex id (convenience overload).
 *
 * Resolves @c uid to a descriptor via @c find_vertex and delegates to the
 * descriptor-based overload.
 *
 * @code
 *   for (auto [tid, n] : neighbors(g, uid)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g   The graph to iterate over.
 * @param  uid The source vertex id.
 * @return @c neighbors_view yielding @c neighbor_data{tid, n} per neighbor.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto neighbors(G& g, const adj_list::vertex_id_t<G>& uid) noexcept {
  auto u = *adj_list::find_vertex(g, uid);
  return neighbors(g, u);
}

/**
 * @brief Create a neighbors view with a vertex value function.
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 2; };
 *   for (auto [tid, n, val] : neighbors(g, u, vvf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  u   The source vertex descriptor.
 * @param  vvf Value function invoked once per neighbor.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c neighbors_view yielding @c neighbor_data{tid, n, val} per neighbor.
 *
 * @pre  @c u is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto neighbors(G& g, adj_list::vertex_t<G> u, VVF&& vvf) {
  return neighbors_view<G, std::decay_t<VVF>>(g, u, std::forward<VVF>(vvf));
}

/**
 * @brief Create a neighbors view with value function from a vertex id
 *        (convenience overload).
 *
 * Resolves @c uid to a descriptor via @c find_vertex and delegates to the
 * descriptor-based overload.
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 2; };
 *   for (auto [tid, n, val] : neighbors(g, uid, vvf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g   The graph to iterate over.
 * @param  uid The source vertex id.
 * @param  vvf Value function invoked once per neighbor.
 * @return @c neighbors_view yielding @c neighbor_data{tid, n, val} per neighbor.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto neighbors(G& g, const adj_list::vertex_id_t<G>& uid, VVF&& vvf) {
  auto u = *adj_list::find_vertex(g, uid);
  return neighbors(g, u, std::forward<VVF>(vvf));
}

// =============================================================================
// basic_neighbors_view — id only (no vertex descriptor in return type)
// =============================================================================

/**
 * @brief Basic neighbors view — simplified variant without value function.
 *
 * Iterates over every neighbor of a source vertex, yielding
 * @c neighbor_data<vertex_id_type,false,void,void>.  No target vertex
 * descriptor is materialised — only the target id is returned, making
 * this the lightest-weight neighbor iteration available.
 *
 * Structured binding: @c auto [tid] where
 * - @c tid — @c vertex_id_t<G> (target / neighbor vertex identifier)
 *
 * @par When to use
 * Prefer @c basic_neighbors when you only need target IDs (e.g. for
 * connectivity traversal in algorithms that index external containers).
 * For access to the target vertex descriptor, use @c neighbors_view
 * instead.  For edge descriptors, use @c incidence_view.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized when @c vertex_edge_range_t<G> is
 * @c sized_range, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(deg(u)).  Zero allocation.
 * Avoids the target vertex lookup that the standard variant performs.
 *
 * @tparam G Graph type satisfying @c adjacency_list
 *
 * @see basic_neighbors(G&, vertex_id_t<G>) — factory function
 * @see neighbors_view — standard variant with target descriptor
 */
template <adj_list::adjacency_list G, class Accessor>
class basic_neighbors_view<G, void, Accessor> : public std::ranges::view_interface<basic_neighbors_view<G, void, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using info_type          = neighbor_data<vertex_id_type, false, void, void>;

  /**
   * @brief Forward iterator yielding @c neighbor_data{tid} per neighbor.
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

    constexpr iterator(G* g, edge_type e) noexcept : g_(g), current_edge_(e) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      return value_type{static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_edge_))};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++current_edge_;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      return current_edge_ == other.current_edge_;
    }

  private:
    G*        g_ = nullptr;
    edge_type current_edge_{};
  };

  using const_iterator = iterator;

  constexpr basic_neighbors_view() noexcept = default;

  constexpr basic_neighbors_view(G& g, vertex_type u) noexcept : g_(&g), source_(u) {}

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
 * @brief Basic neighbors view — simplified variant with value function.
 *
 * Iterates over every neighbor of a source vertex, yielding
 * @c neighbor_data<vertex_id_type,false,void,VV> where @c VV =
 * @c invoke_result_t<VVF,const G&,vertex_t<G>> .  No target vertex
 * descriptor is materialised.
 *
 * Structured binding: @c auto [tid, val] where
 * - @c tid — @c vertex_id_t<G> (target / neighbor vertex identifier)
 * - @c val — computed value returned by VVF
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
 * Construction O(1).  Full iteration O(deg(u)), invoking VVF once per
 * neighbor.  Zero allocation.  Avoids the target vertex lookup that the
 * standard variant performs.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 *
 * @see basic_neighbors(G&, vertex_id_t<G>, VVF&&) — factory function
 * @see neighbors_view — standard variant with target descriptor
 */
template <adj_list::adjacency_list G, class VVF, class Accessor>
class basic_neighbors_view : public std::ranges::view_interface<basic_neighbors_view<G, VVF, Accessor>> {
public:
  using graph_type         = G;
  using accessor_type      = Accessor;
  using vertex_type        = adj_list::vertex_t<G>;
  using vertex_id_type     = adj_list::vertex_id_t<G>;
  using edge_range_type    = typename Accessor::template edge_range_t<G>;
  using edge_iterator_type = std::ranges::iterator_t<edge_range_type>;
  using edge_type          = typename Accessor::template edge_t<G>;
  using value_type_result  = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type          = neighbor_data<vertex_id_type, false, void, value_type_result>;

  /**
   * @brief Forward iterator yielding @c neighbor_data{tid, val} per neighbor.
   *
   * Satisfies @c std::forward_iterator.  @c operator*() may throw if VVF throws.
   */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    constexpr iterator(G* g, edge_type e, VVF* vvf) noexcept : g_(g), current_edge_(e), vvf_(vvf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      auto nbr    = Accessor{}.neighbor(*g_, current_edge_);
      auto nbr_id = static_cast<vertex_id_type>(Accessor{}.neighbor_id(*g_, current_edge_));
      return value_type{nbr_id, std::invoke(*vvf_, std::as_const(*g_), nbr)};
    }

    constexpr iterator& operator++() noexcept {
      ++current_edge_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      auto tmp = *this;
      ++current_edge_;
      return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
      return current_edge_ == other.current_edge_;
    }

  private:
    G*        g_ = nullptr;
    edge_type current_edge_{};
    VVF*      vvf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr basic_neighbors_view() noexcept = default;

  constexpr basic_neighbors_view(basic_neighbors_view&&)            = default;
  constexpr basic_neighbors_view& operator=(basic_neighbors_view&&) = default;

  constexpr basic_neighbors_view(const basic_neighbors_view&)            = default;
  constexpr basic_neighbors_view& operator=(const basic_neighbors_view&) = default;

  constexpr basic_neighbors_view(G& g, vertex_type u, VVF vvf) noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), source_(u), vvf_(std::move(vvf)) {}

  [[nodiscard]] constexpr iterator begin() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::begin(edge_range), &vvf_);
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    auto edge_range = Accessor{}.edges(*g_, source_);
    return iterator(g_, *std::ranges::end(edge_range), &vvf_);
  }

  [[nodiscard]] constexpr auto size() const noexcept
  requires std::ranges::sized_range<edge_range_type>
  {
    return std::ranges::size(Accessor{}.edges(*g_, source_));
  }

private:
  G*                        g_ = nullptr;
  vertex_type               source_{};
  [[no_unique_address]] VVF vvf_{};
};

// Deduction guides for basic_neighbors_view
template <adj_list::adjacency_list G>
basic_neighbors_view(G&, adj_list::vertex_t<G>) -> basic_neighbors_view<G, void, out_edge_accessor>;

template <adj_list::adjacency_list G, class VVF>
basic_neighbors_view(G&, adj_list::vertex_t<G>, VVF) -> basic_neighbors_view<G, VVF, out_edge_accessor>;

// =============================================================================
// Factory functions: basic_neighbors
// =============================================================================

/**
 * @brief Create a basic neighbors view (target id only, no descriptor).
 *
 * @code
 *   for (auto [tid] : basic_neighbors(g, uid)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  uid The source vertex id.
 * @return @c basic_neighbors_view yielding @c neighbor_data{tid} per neighbor.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_neighbors(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_neighbors_view<G, void>(g, u);
}

/**
 * @brief Create a basic neighbors view with value function (target id + value).
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 2; };
 *   for (auto [tid, val] : basic_neighbors(g, uid, vvf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  uid The source vertex id.
 * @param  vvf Value function invoked once per neighbor.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c basic_neighbors_view yielding @c neighbor_data{tid, val} per neighbor.
 *
 * @pre  @c uid is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_neighbors(G& g, const adj_list::vertex_id_t<G>& uid, VVF&& vvf) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_neighbors_view<G, std::decay_t<VVF>>(g, u, std::forward<VVF>(vvf));
}

// =============================================================================
// Explicit outgoing factories: out_neighbors / basic_out_neighbors
// =============================================================================

/// @brief Create an outgoing neighbors view (no value function).
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto out_neighbors(G& g, adj_list::vertex_t<G> u) noexcept {
  return neighbors_view<G, void, out_edge_accessor>(g, u);
}

/// @brief Create an outgoing neighbors view with vertex value function.
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto out_neighbors(G& g, adj_list::vertex_t<G> u, VVF&& vvf) {
  return neighbors_view<G, std::decay_t<VVF>, out_edge_accessor>(g, u, std::forward<VVF>(vvf));
}

/// @brief Create an outgoing neighbors view from vertex id.
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto out_neighbors(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return out_neighbors(g, u);
}

/// @brief Create an outgoing neighbors view with VVF from vertex id.
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto out_neighbors(G& g, const adj_list::vertex_id_t<G>& uid, VVF&& vvf) {
  auto u = *adj_list::find_vertex(g, uid);
  return out_neighbors(g, u, std::forward<VVF>(vvf));
}

/// @brief Create a basic outgoing neighbors view (target id only).
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_out_neighbors(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_neighbors_view<G, void, out_edge_accessor>(g, u);
}

/// @brief Create a basic outgoing neighbors view with VVF.
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_out_neighbors(G& g, const adj_list::vertex_id_t<G>& uid, VVF&& vvf) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_neighbors_view<G, std::decay_t<VVF>, out_edge_accessor>(g, u, std::forward<VVF>(vvf));
}

// =============================================================================
// Incoming factories: in_neighbors / basic_in_neighbors
// =============================================================================

/// @brief Create an incoming neighbors view (no value function).
template <adj_list::bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto in_neighbors(G& g, adj_list::vertex_t<G> u) noexcept {
  return neighbors_view<G, void, in_edge_accessor>(g, u);
}

/// @brief Create an incoming neighbors view with vertex value function.
template <adj_list::bidirectional_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto in_neighbors(G& g, adj_list::vertex_t<G> u, VVF&& vvf) {
  return neighbors_view<G, std::decay_t<VVF>, in_edge_accessor>(g, u, std::forward<VVF>(vvf));
}

/// @brief Create an incoming neighbors view from vertex id.
template <adj_list::index_bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto in_neighbors(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return in_neighbors(g, u);
}

/// @brief Create an incoming neighbors view with VVF from vertex id.
template <adj_list::index_bidirectional_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto in_neighbors(G& g, const adj_list::vertex_id_t<G>& uid, VVF&& vvf) {
  auto u = *adj_list::find_vertex(g, uid);
  return in_neighbors(g, u, std::forward<VVF>(vvf));
}

/// @brief Create a basic incoming neighbors view (source id only).
template <adj_list::index_bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto basic_in_neighbors(G& g, const adj_list::vertex_id_t<G>& uid) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_neighbors_view<G, void, in_edge_accessor>(g, u);
}

/// @brief Create a basic incoming neighbors view with VVF.
template <adj_list::index_bidirectional_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_in_neighbors(G& g, const adj_list::vertex_id_t<G>& uid, VVF&& vvf) {
  auto u = *adj_list::find_vertex(g, uid);
  return basic_neighbors_view<G, std::decay_t<VVF>, in_edge_accessor>(g, u, std::forward<VVF>(vvf));
}

} // namespace graph::views
