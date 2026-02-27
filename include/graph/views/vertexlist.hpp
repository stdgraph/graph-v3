/**
 * @file vertexlist.hpp
 * @brief Vertexlist views for iterating over all vertices in a graph.
 *
 * @section overview Overview
 *
 * Provides lazy, range-based views that iterate over every vertex in a graph.
 * Each iteration step yields a @c vertex_data whose fields are exposed via
 * structured bindings.  An optional vertex value function (VVF) computes a
 * per-vertex value that is included in the binding.
 *
 * @section variants View Variants
 *
 * | Variant                        | Structured Binding | Description                        |
 * |--------------------------------|--------------------|------------------------------------|
 * | @c vertexlist(g)               | `[uid, u]`         | Standard view (id + descriptor)    |
 * | @c vertexlist(g,vvf)           | `[uid, u, val]`    | Standard view with value function  |
 * | @c basic_vertexlist(g)         | `[uid]`            | Simplified view (id only)          |
 * | @c basic_vertexlist(g,vvf)     | `[uid, val]`       | Simplified view with value fn      |
 *
 * All variants also accept a subrange (iterator pair or vertex range) to
 * restrict which vertices are visited.
 *
 * @section bindings Structured Bindings
 *
 * Standard view:
 * @code
 *   for (auto [uid, u]      : vertexlist(g))          // uid = vertex_id_t<G>, u = vertex_t<G>
 *   for (auto [uid, u, val] : vertexlist(g, vvf))     // val = invoke_result_t<VVF, G&, vertex_t<G>>
 * @endcode
 *
 * basic_ variant:
 * @code
 *   for (auto [uid]      : basic_vertexlist(g))       // uid = vertex_id_t<G>
 *   for (auto [uid, val] : basic_vertexlist(g, vvf))  // val = invoke_result_t<VVF, G&, vertex_t<G>>
 * @endcode
 *
 * @section iterator_properties Iterator Properties
 *
 * | Property        | Value                           |
 * |-----------------|---------------------------------|
 * | Concept         | @c std::forward_iterator        |
 * | Range concept   | @c std::ranges::forward_range   |
 * | Sized           | Yes (`size()` in O(1))          |
 * | Borrowed        | No (view holds reference)       |
 * | Common          | Yes (begin/end same type)       |
 *
 * @section perf Performance Characteristics
 *
 * Construction is O(1).  Iteration is O(V), one vertex per step.  The view
 * holds only a pointer to the graph and an iterator pair — no allocation.
 * The @c basic_ variant is lighter still: it never materialises a vertex
 * descriptor and returns only the vertex id.
 *
 * @section chaining Chaining with std::views
 *
 * Views chain with std::views when the value function is a stateless lambda
 * (empty capture list @c []):
 *
 * @code
 *   // Stateless lambda — default_initializable & semiregular → satisfies view
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
 *   auto view = g | vertexlist(vvf)
 *                 | std::views::take(5)        // ✅ compiles
 *                 | std::views::filter(...);   // ✅ compiles
 *
 *   // Views without a value function also chain:
 *   auto view = g | vertexlist()
 *                 | std::views::transform([](auto vi) { return vi.id * 2; });
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
 *   for (auto [uid, u]  : g | vertexlist())              { ... }
 *   for (auto [uid]     : g | basic_vertexlist())        { ... }
 *   for (auto [uid, val]: g | basic_vertexlist(vvf))     { ... }
 * @endcode
 *
 * @section supported_graphs Supported Graph Properties
 *
 * - Requires: @c adjacency_list concept
 * - Works with all @c dynamic_graph container combinations
 * - Works with directed and undirected graphs
 *
 * @subsection index_adjacency_list_requirement Subrange Overloads Require @c index_adjacency_list
 *
 * The subrange factory overloads — @c vertexlist(g,first_u,last_u),
 * @c vertexlist(g,first_u,last_u,vvf), @c basic_vertexlist(g,first_uid,last_uid),
 * and @c basic_vertexlist(g,first_uid,last_uid,vvf) — require
 * @c index_adjacency_list rather than plain @c adjacency_list because:
 *
 * 1. **O(1) @c size()** — The view's @c size() member is computed at construction
 *    as `vertex_id(g,last_u) - vertex_id(g,first_u)` (descriptor overloads) or
 *    `last_uid - first_uid` (id overloads).  Subtraction on @c vertex_id_t<G>
 *    requires an @c std::integral vertex ID type.  @c index_adjacency_list
 *    enforces this via @c index_vertex_range.
 *
 * 2. **O(1) @c find_vertex()** — The id-based @c basic_vertexlist overloads call
 *    @c find_vertex(g, first_uid) and @c find_vertex(g, last_uid) to convert IDs
 *    back to descriptors.  On random-access containers (e.g. @c vector, @c deque)
 *    this is O(1); on map-based containers it is O(log N) or O(1) amortised but
 *    the result is not a contiguous-range iterator.  @c index_adjacency_list
 *    ensures the underlying container supports random-access iteration, keeping
 *    construction O(1).
 *
 * Relaxing to `adjacency_list<G> && std::integral<vertex_id_t<G>>` would allow
 * non-random-access containers (e.g. @c map-vertex graphs) at the cost of
 * making @c find_vertex O(log N).  The current stricter constraint is intentional.
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
 * - For subrange overloads, the range must be a valid sub-sequence of
 *   @c vertices(g).
 *
 * @section see_also See Also
 *
 * - @ref views.md — all views overview
 * - @ref graph_cpo_implementation.md — CPO documentation
 * - @ref view_chaining_limitations.md — chaining design rationale
 * - @ref incidence.hpp — per-vertex edge iteration
 * - @ref neighbors.hpp — per-vertex neighbor iteration
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

namespace graph::views {

// Forward declarations
template <adj_list::adjacency_list G, class VVF = void>
class vertexlist_view;

template <adj_list::adjacency_list G, class VVF = void>
class basic_vertexlist_view;

/**
 * @brief Vertexlist view — standard variant without value function.
 *
 * Iterates over every vertex in the graph, yielding
 * @c vertex_data<vertex_id_type,vertex_t<G>,void> per step.
 *
 * Structured binding: @c auto [uid, u] where
 * - @c uid — @c vertex_id_t<G> (vertex identifier)
 * - @c u   — @c vertex_t<G>    (vertex descriptor)
 *
 * @par Iterator category
 * @c std::forward_iterator — sized, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(V).  Zero allocation.
 *
 * @tparam G Graph type satisfying @c adjacency_list
 *
 * @see vertexlist(G&) — factory function
 * @see basic_vertexlist_view — simplified id-only variant
 */
template <adj_list::adjacency_list G>
class vertexlist_view<G, void> : public std::ranges::view_interface<vertexlist_view<G, void>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using info_type      = vertex_data<vertex_id_type, vertex_type, void>;

  /**
   * @brief Forward iterator yielding @c vertex_data{uid, u} per vertex.
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

    constexpr iterator(G* g, vertex_type v) noexcept : g_(g), current_(v) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      return value_type{adj_list::vertex_id(*g_, current_), current_};
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
    G*          g_ = nullptr;
    vertex_type current_{};
  };

  using const_iterator = iterator;

  constexpr vertexlist_view() noexcept = default;

  constexpr explicit vertexlist_view(G& g) noexcept : g_(&g), size_(adj_list::num_vertices(g)) {
    auto vert_range = adj_list::vertices(g);
    first_          = *std::ranges::begin(vert_range);
    last_           = *std::ranges::end(vert_range);
  }

  constexpr vertexlist_view(G& g, vertex_type first, vertex_type last, std::size_t size) noexcept
        : g_(&g), first_(first), last_(last), size_(size) {}

  [[nodiscard]] constexpr iterator begin() const noexcept { return iterator(g_, first_); }

  [[nodiscard]] constexpr iterator end() const noexcept { return iterator(g_, last_); }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

private:
  G*          g_ = nullptr;
  vertex_type first_{};
  vertex_type last_{};
  std::size_t size_ = 0;
};

/**
 * @brief Vertexlist view — standard variant with value function.
 *
 * Iterates over every vertex, yielding
 * @c vertex_data<vertex_id_type,vertex_t<G>,VV> where @c VV =
 * @c invoke_result_t<VVF,const G&,vertex_t<G>> .
 *
 * Structured binding: @c auto [uid, u, val] where
 * - @c uid — @c vertex_id_t<G> (vertex identifier)
 * - @c u   — @c vertex_t<G>    (vertex descriptor)
 * - @c val — computed value returned by VVF
 *
 * @par Chaining with std::views
 * Use a stateless lambda (empty capture @c []) for the value function so
 * the view satisfies @c std::ranges::view and chains with @c std::views
 * adaptors.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(V), invoking VVF once per vertex.
 * Zero allocation.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 *
 * @see vertexlist(G&, VVF&&) — factory function
 * @see basic_vertexlist_view — simplified id-only variant
 */
template <adj_list::adjacency_list G, class VVF>
class vertexlist_view : public std::ranges::view_interface<vertexlist_view<G, VVF>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using value_type_result = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_data<vertex_id_type, vertex_type, value_type_result>;

  /**
   * @brief Forward iterator yielding @c vertex_data{uid, u, val} per vertex.
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

    constexpr iterator(G* g, vertex_type v, VVF* vvf) noexcept : g_(g), current_(v), vvf_(vvf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      return value_type{adj_list::vertex_id(*g_, current_), current_, std::invoke(*vvf_, std::as_const(*g_), current_)};
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
    G*          g_ = nullptr;
    vertex_type current_{};
    VVF*        vvf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr vertexlist_view() noexcept = default;

  constexpr vertexlist_view(vertexlist_view&&)            = default;
  constexpr vertexlist_view& operator=(vertexlist_view&&) = default;

  constexpr vertexlist_view(const vertexlist_view&)            = default;
  constexpr vertexlist_view& operator=(const vertexlist_view&) = default;

  constexpr vertexlist_view(G& g, VVF vvf) noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), vvf_(std::move(vvf)), size_(adj_list::num_vertices(g)) {
    auto vert_range = adj_list::vertices(g);
    first_          = *std::ranges::begin(vert_range);
    last_           = *std::ranges::end(vert_range);
  }

  constexpr vertexlist_view(G& g, VVF vvf, vertex_type first, vertex_type last, std::size_t size)
        noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), vvf_(std::move(vvf)), first_(first), last_(last), size_(size) {}

  [[nodiscard]] constexpr iterator begin() noexcept { return iterator(g_, first_, &vvf_); }

  [[nodiscard]] constexpr iterator end() noexcept { return iterator(g_, last_, &vvf_); }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

private:
  G*                        g_ = nullptr;
  [[no_unique_address]] VVF vvf_{};
  vertex_type               first_{};
  vertex_type               last_{};
  std::size_t               size_ = 0;
};

// Deduction guides for vertexlist_view
template <adj_list::adjacency_list G>
vertexlist_view(G&) -> vertexlist_view<G, void>;

template <adj_list::adjacency_list G, class VVF>
vertexlist_view(G&, VVF) -> vertexlist_view<G, VVF>;

// =============================================================================
// basic_vertexlist_view — id only (no vertex descriptor in return type)
// =============================================================================

/**
 * @brief Basic vertexlist view — simplified variant without value function.
 *
 * Iterates over every vertex, yielding @c vertex_data<vertex_id_type,void,void>.
 * No vertex descriptor is materialised — only the vertex id is returned,
 * making this the lightest-weight vertex iteration available.
 *
 * Structured binding: @c auto [uid] where
 * - @c uid — @c vertex_id_t<G> (vertex identifier)
 *
 * @par When to use
 * Prefer @c basic_vertexlist when you only need vertex IDs (e.g. to index
 * into external containers such as distance / predecessor arrays).  For
 * access to the vertex descriptor or stored vertex value, use
 * @c vertexlist_view instead.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(V).  Zero allocation.  Avoids
 * the descriptor lookup that the standard variant performs.
 *
 * @tparam G Graph type satisfying @c adjacency_list
 *
 * @see basic_vertexlist(G&) — factory function
 * @see vertexlist_view — standard variant with descriptor
 */
template <adj_list::adjacency_list G>
class basic_vertexlist_view<G, void> : public std::ranges::view_interface<basic_vertexlist_view<G, void>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using info_type      = vertex_data<vertex_id_type, void, void>;

  /**
   * @brief Forward iterator yielding @c vertex_data{uid} per vertex.
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

    constexpr iterator(G* g, vertex_type v) noexcept : g_(g), current_(v) {}

    [[nodiscard]] constexpr value_type operator*() const noexcept {
      return value_type{adj_list::vertex_id(*g_, current_)};
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
    G*          g_ = nullptr;
    vertex_type current_{};
  };

  using const_iterator = iterator;

  constexpr basic_vertexlist_view() noexcept = default;

  constexpr explicit basic_vertexlist_view(G& g) noexcept : g_(&g), size_(adj_list::num_vertices(g)) {
    auto vert_range = adj_list::vertices(g);
    first_          = *std::ranges::begin(vert_range);
    last_           = *std::ranges::end(vert_range);
  }

  constexpr basic_vertexlist_view(G& g, vertex_type first, vertex_type last, std::size_t size) noexcept
        : g_(&g), first_(first), last_(last), size_(size) {}

  [[nodiscard]] constexpr iterator begin() const noexcept { return iterator(g_, first_); }

  [[nodiscard]] constexpr iterator end() const noexcept { return iterator(g_, last_); }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

private:
  G*          g_ = nullptr;
  vertex_type first_{};
  vertex_type last_{};
  std::size_t size_ = 0;
};

/**
 * @brief Basic vertexlist view — simplified variant with value function.
 *
 * Iterates over every vertex, yielding
 * @c vertex_data<vertex_id_type,void,VV> where @c VV =
 * @c invoke_result_t<VVF,const G&,vertex_t<G>> .  No vertex descriptor is
 * materialised.
 *
 * Structured binding: @c auto [uid, val] where
 * - @c uid — @c vertex_id_t<G> (vertex identifier)
 * - @c val — computed value returned by VVF
 *
 * @par Chaining with std::views
 * Use a stateless lambda (empty capture @c []) for the value function so
 * the view satisfies @c std::ranges::view and chains with @c std::views
 * adaptors.
 *
 * @par Iterator category
 * @c std::forward_iterator — sized, common range.
 *
 * @par Performance
 * Construction O(1).  Full iteration O(V), invoking VVF once per vertex.
 * Zero allocation.  Avoids the descriptor lookup that the standard
 * variant performs.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 *
 * @see basic_vertexlist(G&, VVF&&) — factory function
 * @see vertexlist_view — standard variant with descriptor
 */
template <adj_list::adjacency_list G, class VVF>
class basic_vertexlist_view : public std::ranges::view_interface<basic_vertexlist_view<G, VVF>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using value_type_result = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_data<vertex_id_type, void, value_type_result>;

  /**
   * @brief Forward iterator yielding @c vertex_data{uid, val} per vertex.
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

    constexpr iterator(G* g, vertex_type v, VVF* vvf) noexcept : g_(g), current_(v), vvf_(vvf) {}

    [[nodiscard]] constexpr value_type operator*() const {
      return value_type{adj_list::vertex_id(*g_, current_), std::invoke(*vvf_, std::as_const(*g_), current_)};
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
    G*          g_ = nullptr;
    vertex_type current_{};
    VVF*        vvf_ = nullptr;
  };

  using const_iterator = iterator;

  constexpr basic_vertexlist_view() noexcept = default;

  constexpr basic_vertexlist_view(basic_vertexlist_view&&)            = default;
  constexpr basic_vertexlist_view& operator=(basic_vertexlist_view&&) = default;

  constexpr basic_vertexlist_view(const basic_vertexlist_view&)            = default;
  constexpr basic_vertexlist_view& operator=(const basic_vertexlist_view&) = default;

  constexpr basic_vertexlist_view(G& g, VVF vvf) noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), vvf_(std::move(vvf)), size_(adj_list::num_vertices(g)) {
    auto vert_range = adj_list::vertices(g);
    first_          = *std::ranges::begin(vert_range);
    last_           = *std::ranges::end(vert_range);
  }

  constexpr basic_vertexlist_view(G& g, VVF vvf, vertex_type first, vertex_type last, std::size_t size)
        noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), vvf_(std::move(vvf)), first_(first), last_(last), size_(size) {}

  [[nodiscard]] constexpr iterator begin() noexcept { return iterator(g_, first_, &vvf_); }

  [[nodiscard]] constexpr iterator end() noexcept { return iterator(g_, last_, &vvf_); }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }

private:
  G*                        g_ = nullptr;
  [[no_unique_address]] VVF vvf_{};
  vertex_type               first_{};
  vertex_type               last_{};
  std::size_t               size_ = 0;
};

// Deduction guides for basic_vertexlist_view
template <adj_list::adjacency_list G>
basic_vertexlist_view(G&) -> basic_vertexlist_view<G, void>;

template <adj_list::adjacency_list G, class VVF>
basic_vertexlist_view(G&, VVF) -> basic_vertexlist_view<G, VVF>;

// =============================================================================
// Factory functions: vertexlist
// =============================================================================

/**
 * @brief Create a vertexlist view over all vertices (no value function).
 *
 * @code
 *   for (auto [uid, u] : vertexlist(g)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g  The graph to iterate over.  Must outlive the returned view.
 * @return @c vertexlist_view yielding @c vertex_data{uid, u} per vertex.
 *
 * @pre  None.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto vertexlist(G& g) noexcept {
  return vertexlist_view<G, void>(g);
}

/**
 * @brief Create a vertexlist view with a vertex value function.
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 2; };
 *   for (auto [uid, u, val] : vertexlist(g, vvf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  vvf Value function invoked once per vertex.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c vertexlist_view yielding @c vertex_data{uid, u, val} per vertex.
 *
 * @pre  None.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto vertexlist(G& g, VVF&& vvf) {
  return vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf));
}

/**
 * @brief Create a vertexlist view over a descriptor-based subrange @c [first_u,last_u).
 *
 * @tparam G       Graph type satisfying @c index_adjacency_list
 * @param  g       The graph to iterate over.  Must outlive the returned view.
 * @param  first_u First vertex descriptor (inclusive).
 * @param  last_u  Past-the-end vertex descriptor (exclusive).
 * @return @c vertexlist_view over the requested subrange.
 *
 * @pre  @c [first_u,last_u) is a valid subrange of @c vertices(g).
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto vertexlist(G& g, adj_list::vertex_t<G> first_u, adj_list::vertex_t<G> last_u) noexcept {
  auto sz = static_cast<std::size_t>(adj_list::vertex_id(g, last_u) - adj_list::vertex_id(g, first_u));
  return vertexlist_view<G, void>(g, first_u, last_u, sz);
}

/**
 * @brief Create a vertexlist view over a descriptor-based subrange with value function.
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam VVF Vertex value function
 * @param  g       The graph to iterate over.
 * @param  first_u First vertex descriptor (inclusive).
 * @param  last_u  Past-the-end vertex descriptor (exclusive).
 * @param  vvf     Value function invoked once per vertex.
 * @return @c vertexlist_view over the requested subrange.
 *
 * @pre  @c [first_u,last_u) is a valid subrange of @c vertices(g).
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto
vertexlist(G& g, adj_list::vertex_t<G> first_u, adj_list::vertex_t<G> last_u, VVF&& vvf) {
  auto sz = static_cast<std::size_t>(adj_list::vertex_id(g, last_u) - adj_list::vertex_id(g, first_u));
  return vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf), first_u, last_u, sz);
}

/**
 * @brief Create a vertexlist view over a vertex range.
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @tparam VR Vertex range type satisfying @c vertex_range<VR,G>
 * @param  g  The graph.
 * @param  vr A vertex range (e.g. from @c vertices(g)).
 * @return @c vertexlist_view over the supplied range.
 *
 * @pre  @c vr is a valid subrange of @c vertices(g).
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VR>
requires adj_list::vertex_range<VR, G>
[[nodiscard]] constexpr auto vertexlist(G& g, VR&& vr) {
  return vertexlist_view<G, void>(g, *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

/**
 * @brief Create a vertexlist view over a vertex range with value function.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VR  Vertex range type satisfying @c vertex_range<VR,G>
 * @tparam VVF Vertex value function
 * @param  g   The graph.
 * @param  vr  A vertex range.
 * @param  vvf Value function invoked once per vertex.
 * @return @c vertexlist_view over the supplied range.
 *
 * @pre  @c vr is a valid subrange of @c vertices(g).
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VR, class VVF>
requires adj_list::vertex_range<VR, G> && vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto vertexlist(G& g, VR&& vr, VVF&& vvf) {
  return vertexlist_view<G, std::decay_t<VVF>>(
        g, std::forward<VVF>(vvf), *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

// =============================================================================
// Factory functions: basic_vertexlist
// =============================================================================

/**
 * @brief Create a basic vertexlist view (id only, no descriptor).
 *
 * @code
 *   for (auto [uid] : basic_vertexlist(g)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @param  g  The graph to iterate over.  Must outlive the returned view.
 * @return @c basic_vertexlist_view yielding @c vertex_data{uid} per vertex.
 *
 * @pre  None.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_vertexlist(G& g) noexcept {
  return basic_vertexlist_view<G, void>(g);
}

/**
 * @brief Create a basic vertexlist view with value function (id + value, no descriptor).
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 2; };
 *   for (auto [uid, val] : basic_vertexlist(g, vvf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g   The graph to iterate over.  Must outlive the returned view.
 * @param  vvf Value function invoked once per vertex.
 *             Use a stateless lambda for @c std::views chaining support.
 * @return @c basic_vertexlist_view yielding @c vertex_data{uid, val} per vertex.
 *
 * @pre  None.
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_vertexlist(G& g, VVF&& vvf) {
  return basic_vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf));
}

/**
 * @brief Create a basic vertexlist view over an id-based subrange @c [first_uid,last_uid).
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g         The graph to iterate over.
 * @param  first_uid First vertex ID (inclusive).
 * @param  last_uid  Past-the-end vertex ID (exclusive).
 * @return @c basic_vertexlist_view over the requested id subrange.
 *
 * @pre  @c first_uid and @c last_uid are valid vertex IDs (or past-the-end).
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto
basic_vertexlist(G& g, adj_list::vertex_id_t<G> first_uid, adj_list::vertex_id_t<G> last_uid) {
  auto first = *adj_list::find_vertex(g, first_uid);
  auto last  = *adj_list::find_vertex(g, last_uid);
  auto sz    = static_cast<std::size_t>(last_uid - first_uid);
  return basic_vertexlist_view<G, void>(g, first, last, sz);
}

/**
 * @brief Create a basic vertexlist view over an id-based subrange with value function.
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam VVF Vertex value function
 * @param  g         The graph to iterate over.
 * @param  first_uid First vertex ID (inclusive).
 * @param  last_uid  Past-the-end vertex ID (exclusive).
 * @param  vvf       Value function invoked once per vertex.
 * @return @c basic_vertexlist_view over the requested id subrange.
 *
 * @pre  @c first_uid and @c last_uid are valid vertex IDs (or past-the-end).
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto
basic_vertexlist(G& g, adj_list::vertex_id_t<G> first_uid, adj_list::vertex_id_t<G> last_uid, VVF&& vvf) {
  auto first = *adj_list::find_vertex(g, first_uid);
  auto last  = *adj_list::find_vertex(g, last_uid);
  auto sz    = static_cast<std::size_t>(last_uid - first_uid);
  return basic_vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf), first, last, sz);
}

/**
 * @brief Create a basic vertexlist view over a vertex range (id only).
 *
 * @tparam G  Graph type satisfying @c adjacency_list
 * @tparam VR Vertex range type satisfying @c vertex_range<VR,G>
 * @param  g  The graph.
 * @param  vr A vertex range (e.g. from @c vertices(g)).
 * @return @c basic_vertexlist_view over the supplied range.
 *
 * @pre  @c vr is a valid subrange of @c vertices(g).
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VR>
requires adj_list::vertex_range<VR, G>
[[nodiscard]] constexpr auto basic_vertexlist(G& g, VR&& vr) {
  return basic_vertexlist_view<G, void>(g, *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

/**
 * @brief Create a basic vertexlist view over a vertex range with value function.
 *
 * @tparam G   Graph type satisfying @c adjacency_list
 * @tparam VR  Vertex range type satisfying @c vertex_range<VR,G>
 * @tparam VVF Vertex value function
 * @param  g   The graph.
 * @param  vr  A vertex range.
 * @param  vvf Value function invoked once per vertex.
 * @return @c basic_vertexlist_view over the supplied range.
 *
 * @pre  @c vr is a valid subrange of @c vertices(g).
 * @post The graph is not modified.
 */
template <adj_list::adjacency_list G, class VR, class VVF>
requires adj_list::vertex_range<VR, G> && vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_vertexlist(G& g, VR&& vr, VVF&& vvf) {
  return basic_vertexlist_view<G, std::decay_t<VVF>>(
        g, std::forward<VVF>(vvf), *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

} // namespace graph::views
