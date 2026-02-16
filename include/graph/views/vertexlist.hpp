/**
 * @file vertexlist.hpp
 * @brief Vertexlist view for iterating over graph vertices
 * 
 * Provides a view that iterates over all vertices in a graph, yielding
 * vertex_info<vertex_id_type, vertex_descriptor, VV> for each vertex. Supports
 * optional value functions to compute per-vertex values.
 * 
 * @section chaining_with_std_views Chaining with std::views
 * 
 * Views created WITHOUT value functions chain perfectly with std::views:
 * @code
 *   auto view = g | vertexlist()
 *                 | std::views::transform([](auto vi) { return vi.id * 2; })
 *                 | std::views::filter([](auto doubled_id) { return doubled_id > 0; });
 * @endcode
 * 
 * @warning LIMITATION: Views with capturing lambda value functions cannot chain with std::views
 *
 * This FAILS to compile in C++20:
 * @code
 *   // OLD approach (capturing) - broken for chaining:
 *   auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
 *   auto view = g | vertexlist(vvf) | std::views::take(2);  // ❌ Won't compile
 *
 *   // NEW approach (parameter-based) - works for chaining:
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v) * 10; };
 *   auto view = g | vertexlist(vvf) | std::views::take(2);  // ✅ Compiles
 * @endcode
 *
 * The new parameter-based approach passes the graph as first argument to the
 * value function, eliminating the need for lambda captures. Stateless lambdas
 * (empty capture list) are default_initializable and semiregular, which allows
 * the view to satisfy std::ranges::view and chain with std::views.
 * 
 * @section workarounds Workarounds
 * 
 * 1. RECOMMENDED: Use views without value functions, then transform:
 * @code
 *   auto view = g | vertexlist()
 *                 | std::views::transform([](auto vi) { 
 *                     return std::make_tuple(vi.id, vi.vertex, vi.id * 10);
 *                   });
 * @endcode
 * 
 * 2. Don't chain - use value functions standalone:
 * @code
 *   auto vvf = [&g](auto v) { return vertex_id(g, v) * 10; };
 *   auto view = g | vertexlist(vvf);  // ✅ Works fine, just don't chain further
 * @endcode
 * 
 * 3. Extract to container first, then chain:
 * @code
 *   std::vector<vertex_info<...>> vertices;
 *   for (auto vi : g | vertexlist()) vertices.push_back(vi);
 *   auto view = vertices | std::views::transform(...);
 * @endcode
 * 
 * @section cpp26_fix C++26 Fix
 * 
 * C++26 will introduce std::copyable_function (P2548) or similar type-erased
 * function wrappers that are always semiregular, which will solve this issue.
 * Future implementation could wrap VVF in such a type to enable chaining
 * with capturing lambdas.
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

namespace graph::views {

// Forward declarations
template <adj_list::adjacency_list G, class VVF = void>
class vertexlist_view;

template <adj_list::adjacency_list G, class VVF = void>
class basic_vertexlist_view;

/**
 * @brief Vertexlist view without value function
 * 
 * Iterates over vertices yielding vertex_info<vertex_id_type, vertex_t<G>, void>
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 */
template <adj_list::adjacency_list G>
class vertexlist_view<G, void> : public std::ranges::view_interface<vertexlist_view<G, void>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using info_type      = vertex_info<vertex_id_type, vertex_type, void>;

  /**
     * @brief Forward iterator yielding vertex_info values
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
 * @brief Vertexlist view with value function
 * 
 * Iterates over vertices yielding vertex_info<vertex_id_type, vertex_t<G>, VV>
 * where VV is the result of invoking the value function on the vertex descriptor.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam VVF Value function type
 */
template <adj_list::adjacency_list G, class VVF>
class vertexlist_view : public std::ranges::view_interface<vertexlist_view<G, VVF>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using value_type_result = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_info<vertex_id_type, vertex_type, value_type_result>;

  /**
     * @brief Forward iterator yielding vertex_info values with computed value
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
 * @brief Basic vertexlist view without value function (id only)
 * 
 * Iterates over vertices yielding vertex_info<vertex_id_type, void, void>.
 * Use this when only vertex IDs are needed (e.g., in algorithms using index_adjacency_list).
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 */
template <adj_list::adjacency_list G>
class basic_vertexlist_view<G, void> : public std::ranges::view_interface<basic_vertexlist_view<G, void>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using info_type      = vertex_info<vertex_id_type, void, void>;

  /**
     * @brief Forward iterator yielding vertex_info values (id only)
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
 * @brief Basic vertexlist view with value function (id + value, no vertex descriptor)
 * 
 * Iterates over vertices yielding vertex_info<vertex_id_type, void, VV>
 * where VV is the result of invoking the value function on the vertex.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam VVF Value function type
 */
template <adj_list::adjacency_list G, class VVF>
class basic_vertexlist_view : public std::ranges::view_interface<basic_vertexlist_view<G, VVF>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using value_type_result = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_info<vertex_id_type, void, value_type_result>;

  /**
     * @brief Forward iterator yielding vertex_info values (id + computed value)
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
 * @brief Create a vertexlist view without value function
 * 
 * @param g The graph to iterate over
 * @return vertexlist_view yielding vertex_info<vertex_id_type, vertex_descriptor, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto vertexlist(G& g) noexcept {
  return vertexlist_view<G, void>(g);
}

/**
 * @brief Create a vertexlist view with value function
 * 
 * @param g The graph to iterate over
 * @param vvf Value function invoked for each vertex
 * @return vertexlist_view yielding vertex_info<vertex_id_type, vertex_descriptor, VV>
 */
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto vertexlist(G& g, VVF&& vvf) {
  return vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf));
}

/**
 * @brief Create a vertexlist view over a descriptor-based subrange [first_u, last_u)
 * 
 * @param g The graph to iterate over
 * @param first_u First vertex descriptor (inclusive)
 * @param last_u Past-the-end vertex descriptor (exclusive)
 * @return vertexlist_view yielding vertex_info<vertex_id_type, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] constexpr auto vertexlist(G& g, adj_list::vertex_t<G> first_u, adj_list::vertex_t<G> last_u) noexcept {
  auto sz = static_cast<std::size_t>(adj_list::vertex_id(g, last_u) - adj_list::vertex_id(g, first_u));
  return vertexlist_view<G, void>(g, first_u, last_u, sz);
}

/**
 * @brief Create a vertexlist view over a descriptor-based subrange with value function
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto
vertexlist(G& g, adj_list::vertex_t<G> first_u, adj_list::vertex_t<G> last_u, VVF&& vvf) {
  auto sz = static_cast<std::size_t>(adj_list::vertex_id(g, last_u) - adj_list::vertex_id(g, first_u));
  return vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf), first_u, last_u, sz);
}

/**
 * @brief Create a vertexlist view over a vertex range
 * 
 * @param g The graph
 * @param vr A vertex range (e.g., from vertices(g))
 * @return vertexlist_view yielding vertex_info<vertex_id_type, vertex_descriptor, void>
 */
template <adj_list::adjacency_list G, class VR>
requires adj_list::vertex_range<VR, G>
[[nodiscard]] constexpr auto vertexlist(G& g, VR&& vr) {
  return vertexlist_view<G, void>(g, *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

/**
 * @brief Create a vertexlist view over a vertex range with value function
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
 * @brief Create a basic vertexlist view without value function (id only)
 * 
 * @param g The graph to iterate over
 * @return basic_vertexlist_view yielding vertex_info<vertex_id_type, void, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto basic_vertexlist(G& g) noexcept {
  return basic_vertexlist_view<G, void>(g);
}

/**
 * @brief Create a basic vertexlist view with value function (id + value, no descriptor)
 * 
 * @param g The graph to iterate over
 * @param vvf Value function invoked for each vertex
 * @return basic_vertexlist_view yielding vertex_info<vertex_id_type, void, VV>
 */
template <adj_list::adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_vertexlist(G& g, VVF&& vvf) {
  return basic_vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf));
}

/**
 * @brief Create a basic vertexlist view over an id-based subrange [first_uid, last_uid)
 * 
 * @param g The graph to iterate over
 * @param first_uid First vertex ID (inclusive)
 * @param last_uid Past-the-end vertex ID (exclusive)
 * @return basic_vertexlist_view yielding vertex_info<vertex_id_type, void, void>
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
 * @brief Create a basic vertexlist view over an id-based subrange with value function
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
 * @brief Create a basic vertexlist view over a vertex range (id only)
 * 
 * @param g The graph
 * @param vr A vertex range (e.g., from vertices(g))
 * @return basic_vertexlist_view yielding vertex_info<vertex_id_type, void, void>
 */
template <adj_list::adjacency_list G, class VR>
requires adj_list::vertex_range<VR, G>
[[nodiscard]] constexpr auto basic_vertexlist(G& g, VR&& vr) {
  return basic_vertexlist_view<G, void>(g, *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

/**
 * @brief Create a basic vertexlist view over a vertex range with value function
 */
template <adj_list::adjacency_list G, class VR, class VVF>
requires adj_list::vertex_range<VR, G> && vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto basic_vertexlist(G& g, VR&& vr, VVF&& vvf) {
  return basic_vertexlist_view<G, std::decay_t<VVF>>(
        g, std::forward<VVF>(vvf), *std::ranges::begin(vr), *std::ranges::end(vr), std::ranges::size(vr));
}

} // namespace graph::views
