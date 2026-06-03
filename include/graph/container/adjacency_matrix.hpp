#pragma once

/**
 * @file adjacency_matrix.hpp
 * @brief Dense adjacency-matrix container that models the graph-v3 adjacency_list CPO concepts.
 *
 * This header provides two presentations of the same idea:
 *
 *   1. graph::container::adjacency_matrix<EV, VId, Directed>
 *        A portable C++20 implementation. It owns two dense `n*n` planes:
 *          - a presence plane (`flags_`) giving O(1) `has_edge(u, v)`, and
 *          - an element plane (`cells_`) holding the edge element for each cell.
 *        The matrix models `index_adjacency_list` through the graph-v3
 *        "inner value pattern": the container itself is a random-access range
 *        whose i-th element is a lightweight, lazily-filtered view over row @p i
 *        that yields only the edges that are present. Because edge elements live
 *        in the owned element plane, the per-vertex edge iterators yield stable
 *        references, which is what `target_id` / `edge_value` require.
 *
 *   2. graph::container::md_adjacency_matrix<EV, VId, Directed>
 *        A C++23 variant (guarded by `__cpp_lib_mdspan`) that reuses the same
 *        owning storage but additionally exposes the dense presence plane through
 *        `std::mdspan`, giving natural `m(u, v)` element access and the option to
 *        select a memory layout (e.g. `layout_right` for contiguous out-edge
 *        rows). `std::mdspan` is used purely as a *view* over the owned
 *        `std::vector`; it is not the owning store and does not, by itself,
 *        model the graph CPO concepts.
 *
 * Concept wiring (how the CPOs resolve, no member `vertices()`/`edges()` needed):
 *   - `graph::vertices(g)`     -> g is a random-access range -> inner-value pattern,
 *                                 wrapped automatically in a `vertex_descriptor_view`.
 *                                 `vertex_id` is the row index (integral) -> index list.
 *   - `graph::out_edges(g, u)` -> `u.inner_value(g)` is the row view, a forward range
 *                                 of edge-value elements -> wrapped in an
 *                                 `edge_descriptor_view`.
 *   - `graph::target_id(g, uv)`-> extracted from the edge iterator's current column.
 *   - `graph::edge_value(g, uv)`-> extracted from the referenced matrix cell (weighted).
 *
 * Cost model (inherent to dense matrices):
 *   - Edge existence / weight lookup: O(1).
 *   - Iterating the out-edges of a vertex: O(n) (the whole row is scanned, absent
 *     cells are skipped), regardless of the vertex degree.
 *   - Space: O(n^2).
 */

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <cassert>
#include <functional>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "graph/graph.hpp"
#include "graph/adj_list/vertex_descriptor_view.hpp"
#include "graph/adj_list/edge_descriptor_view.hpp"

#if defined(__has_include)
#  if __has_include(<mdspan>)
#    include <mdspan>
#  endif
#endif

namespace graph::container {

namespace _amx_detail {

  /// Presence flag storage type (1 byte; 0 = absent, 1 = present).
  using flag_t = std::uint8_t;

  /// Edge element stored per cell: empty marker (unweighted) or edge value (weighted).
  ///
  /// Design note: target-id is not stored in each cell. The edge iterator keeps
  /// the current column, allowing `target_id` to be answered directly from
  /// descriptor position while keeping a single lookup into the row storage.
  template <class EV, class VId>
  using edge_element_t = std::conditional_t<std::is_void_v<EV>, std::monostate, EV>;

  /**
   * @brief Forward iterator over the present edges of a single matrix row.
   *
   * The iterator stores raw pointers into the matrix's owning planes plus the
   * current column. Dereferencing returns a *reference* into the owned element
   * plane (`const edge_element_t&`), so the value stays alive independently of
   * the (lightweight, non-owning) row view that produced the iterator. That is
   * exactly what `edge_descriptor_view` and the `target_id` / `edge_value` CPOs
   * require.
   */
  template <class EV, class VId>
  class row_edge_iterator {
  public:
    using value_type        = edge_element_t<EV, VId>;
    using reference         = const value_type&;
    using pointer           = const value_type*;
    using difference_type   = std::ptrdiff_t;
    using iterator_concept  = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;

    constexpr row_edge_iterator() noexcept = default;

    constexpr row_edge_iterator(const flag_t* flags, const value_type* cells, VId n, VId col) noexcept
        : flags_(flags), cells_(cells), n_(n), col_(col) {
      skip_absent();
    }

    [[nodiscard]] constexpr reference operator*() const noexcept { return cells_[col_]; }
    [[nodiscard]] constexpr pointer operator->() const noexcept { return cells_ + col_; }
    [[nodiscard]] constexpr VId      target_id() const noexcept { return col_; }

    [[nodiscard]] constexpr reference edge_value_ref() const noexcept {
      return cells_[col_];
    }

    constexpr row_edge_iterator& operator++() noexcept {
      ++col_;
      skip_absent();
      return *this;
    }

    constexpr row_edge_iterator operator++(int) noexcept {
      row_edge_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    [[nodiscard]] friend constexpr bool operator==(const row_edge_iterator& lhs,
                                                   const row_edge_iterator& rhs) noexcept {
      return lhs.col_ == rhs.col_;
    }

  private:
    constexpr void skip_absent() noexcept {
      while (col_ < n_ && flags_[col_] == flag_t{0}) {
        ++col_;
      }
    }

    const flag_t*     flags_ = nullptr;
    const value_type* cells_ = nullptr;
    VId               n_     = 0;
    VId               col_   = 0;
  };

  /**
   * @brief Lightweight, non-owning forward range over the present edges of one row.
   *
   * This is the element type produced by the matrix's vertex iterator (the
   * "inner value" in graph-v3 terms). It owns nothing; it just points into the
   * matrix's dense planes. Returning it by value is safe because the edge
   * iterators it hands out are self-contained and reference the owned planes.
   */
  template <class EV, class VId>
  class row_view : public std::ranges::view_interface<row_view<EV, VId>> {
  public:
    using element_type   = edge_element_t<EV, VId>;
    using iterator       = row_edge_iterator<EV, VId>;
    using const_iterator = iterator;

    constexpr row_view() noexcept = default;

    constexpr row_view(const flag_t* flags, const element_type* cells, VId n) noexcept
        : flags_(flags), cells_(cells), n_(n) {}

    [[nodiscard]] constexpr iterator begin() const noexcept { return iterator{flags_, cells_, n_, VId{0}}; }
    [[nodiscard]] constexpr iterator end() const noexcept { return iterator{flags_, cells_, n_, n_}; }

  private:
    const flag_t*       flags_ = nullptr;
    const element_type* cells_ = nullptr;
    VId                 n_     = 0;
  };

  /**
   * @brief Random-access iterator over the rows of an adjacency matrix.
   *
   * Each dereference produces a `row_view` by value (a proxy reference, like
   * `std::ranges::iota_view`). This makes the matrix itself a random-access
   * range, which is exactly what the graph-v3 inner-value pattern keys on to
   * synthesise `vertices(g)` and integral, index-based vertex ids.
   */
  template <class EV, class VId>
  class vertex_iterator {
    using cell_type = edge_element_t<EV, VId>;

  public:
    using value_type        = row_view<EV, VId>;
    using reference         = row_view<EV, VId>; // proxy prvalue
    using difference_type   = std::ptrdiff_t;
    using iterator_concept  = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;

    constexpr vertex_iterator() noexcept = default;

    constexpr vertex_iterator(const flag_t* flags, const cell_type* cells, VId n, VId row) noexcept
        : flags_(flags), cells_(cells), n_(n), row_(row) {}

    [[nodiscard]] constexpr reference operator*() const noexcept {
      const std::size_t offset = static_cast<std::size_t>(row_) * static_cast<std::size_t>(n_);
      return row_view<EV, VId>{flags_ + offset, cells_ + offset, n_};
    }

    [[nodiscard]] constexpr reference operator[](difference_type d) const noexcept { return *(*this + d); }

    constexpr vertex_iterator& operator++() noexcept {
      ++row_;
      return *this;
    }
    constexpr vertex_iterator operator++(int) noexcept {
      vertex_iterator tmp = *this;
      ++row_;
      return tmp;
    }
    constexpr vertex_iterator& operator--() noexcept {
      --row_;
      return *this;
    }
    constexpr vertex_iterator operator--(int) noexcept {
      vertex_iterator tmp = *this;
      --row_;
      return tmp;
    }

    constexpr vertex_iterator& operator+=(difference_type d) noexcept {
      row_ = static_cast<VId>(static_cast<difference_type>(row_) + d);
      return *this;
    }
    constexpr vertex_iterator& operator-=(difference_type d) noexcept { return *this += -d; }

    [[nodiscard]] friend constexpr vertex_iterator operator+(vertex_iterator it, difference_type d) noexcept {
      return it += d;
    }
    [[nodiscard]] friend constexpr vertex_iterator operator+(difference_type d, vertex_iterator it) noexcept {
      return it += d;
    }
    [[nodiscard]] friend constexpr vertex_iterator operator-(vertex_iterator it, difference_type d) noexcept {
      return it -= d;
    }
    [[nodiscard]] friend constexpr difference_type operator-(const vertex_iterator& lhs,
                                                            const vertex_iterator& rhs) noexcept {
      return static_cast<difference_type>(lhs.row_) - static_cast<difference_type>(rhs.row_);
    }

    [[nodiscard]] friend constexpr bool operator==(const vertex_iterator& lhs, const vertex_iterator& rhs) noexcept {
      return lhs.row_ == rhs.row_;
    }
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(const vertex_iterator& lhs,
                                                                   const vertex_iterator& rhs) noexcept {
      return lhs.row_ <=> rhs.row_;
    }

  private:
    const flag_t*    flags_ = nullptr;
    const cell_type* cells_ = nullptr;
    VId              n_     = 0;
    VId              row_   = 0;
  };

} // namespace _amx_detail

/**
 * @ingroup graph_containers
 * @brief Dense adjacency-matrix graph container (portable C++20).
 *
 * @tparam EV       Edge value (weight) type, or `void` for an unweighted graph.
 * @tparam VId      Vertex id / index type (must be integral). Defaults to `uint32_t`.
 * @tparam Directed When true (default) `add_edge(u, v)` adds only `u -> v`; when
 *                  false the reciprocal `v -> u` is added as well (symmetric matrix).
 *
 * The number of vertices is fixed at construction (a matrix is `order x order`).
 * Edges are added/removed by setting cells; storage never reallocates afterwards,
 * so the row views and their iterators remain valid for the matrix's lifetime.
 */
template <class EV = void, class VId = std::uint32_t, bool Directed = true>
requires std::integral<VId>
class adjacency_matrix {
  static constexpr bool weighted = !std::is_void_v<EV>;
  using flag_t                   = _amx_detail::flag_t;
  using element_type             = _amx_detail::edge_element_t<EV, VId>;

public:
  using vertex_id_type = VId;
  using size_type      = std::size_t;
  using iterator       = _amx_detail::vertex_iterator<EV, VId>;
  using const_iterator = _amx_detail::vertex_iterator<EV, VId>;

  /// Construct an @p order x @p order matrix with no edges.
  constexpr explicit adjacency_matrix(VId order = VId{0})
      : n_(order)
      , flags_(static_cast<std::size_t>(order) * static_cast<std::size_t>(order), flag_t{0})
      , cells_(static_cast<std::size_t>(order) * static_cast<std::size_t>(order), element_type{}) {}

  /**
   * @brief Construct an @p order x @p order matrix and load it from a range of edges.
   *
   * Each element of @p erng is projected to a `copyable_edge_t<VId, EV>` (i.e.
   * `{source_id, target_id [, value]}`) by @p eproj and inserted via `add_edge`.
   * When the projected value type already is `copyable_edge_t<VId, EV>`,
   * `std::identity` (the default) can be used. This mirrors the edge-range +
   * projection constructors of `dynamic_graph` and `compressed_graph`.
   *
   * The matrix is sized to @p order; edge endpoints are NOT scanned to grow it,
   * so every projected `source_id` / `target_id` must be `< order`.
   *
   * @tparam ERng  Range type of the source edge data.
   * @tparam EProj Projection from an @p erng element to `copyable_edge_t<VId, EV>`.
   *
   * @param order The number of vertices (matrix is `order x order`).
   * @param erng  The source range of edge data.
   * @param eproj The projection function (or `std::identity` when already projected).
   */
  template <std::ranges::input_range ERng, class EProj = std::identity>
  requires copyable_edge<std::invoke_result_t<EProj&, std::ranges::range_reference_t<ERng>>, VId, EV>
  constexpr adjacency_matrix(VId order, ERng&& erng, EProj eproj = EProj{}) : adjacency_matrix(order) {
    for (auto&& elem : erng) {
      const copyable_edge_t<VId, EV>& uv = eproj(elem);
      if constexpr (weighted) {
        add_edge(static_cast<VId>(uv.source_id), static_cast<VId>(uv.target_id), uv.value);
      } else {
        add_edge(static_cast<VId>(uv.source_id), static_cast<VId>(uv.target_id));
      }
    }
  }

  // ---- range interface (drives the inner-value pattern) -------------------

  [[nodiscard]] constexpr iterator       begin() noexcept { return make_iter(VId{0}); }
  [[nodiscard]] constexpr iterator       end() noexcept { return make_iter(n_); }
  [[nodiscard]] constexpr const_iterator begin() const noexcept { return make_iter(VId{0}); }
  [[nodiscard]] constexpr const_iterator end() const noexcept { return make_iter(n_); }
  [[nodiscard]] constexpr std::size_t    size() const noexcept { return static_cast<std::size_t>(n_); }

  /// Row access, required by the graph-v3 `underlying_value` machinery.
  [[nodiscard]] constexpr _amx_detail::row_view<EV, VId> operator[](std::size_t row) const noexcept {
    const std::size_t offset = row * static_cast<std::size_t>(n_);
    return _amx_detail::row_view<EV, VId>{flags_.data() + offset, cells_.data() + offset, n_};
  }

  // ---- queries ------------------------------------------------------------

  [[nodiscard]] constexpr VId         num_vertices() const noexcept { return n_; }
  [[nodiscard]] constexpr VId         order() const noexcept { return n_; }
  [[nodiscard]] constexpr std::size_t num_edges() const noexcept { return edge_count_; }

  /// True iff edge (u, v) is present.
  [[nodiscard]] constexpr bool exists(VId u, VId v) const noexcept { return flags_[index(u, v)] != flag_t{0}; }

  /// Back-compat alias for existence checks.
  [[nodiscard]] constexpr bool has_edge(VId u, VId v) const noexcept { return exists(u, v); }

  /// Natural read-only value access for edge (u, v); requires the edge to exist.
  template <class E = EV>
  requires weighted
  [[nodiscard]] constexpr const E& operator()(VId u, VId v) const noexcept {
    assert(exists(u, v));
    return cells_[index(u, v)];
  }

  // ---- mutation -----------------------------------------------------------

  /// Add an unweighted edge u -> v (and v -> u when undirected).
  template <bool W = weighted>
  requires(!W)
  constexpr void add_edge(VId u, VId v) {
    set_cell(u, v);
    if constexpr (!Directed) {
      set_cell(v, u);
    }
  }

  /// Add a weighted edge u -> v (and v -> u when undirected).
  template <class E = EV>
  requires weighted
  constexpr void add_edge(VId u, VId v, E value) {
    set_cell(u, v);
    cells_[index(u, v)] = value;
    if constexpr (!Directed) {
      set_cell(v, u);
      cells_[index(v, u)] = value;
    }
  }

  template <typename G, typename EdgeDesc>
  requires std::same_as<std::remove_cvref_t<G>, adjacency_matrix> &&
           requires(const EdgeDesc& uv) {
             uv.value().target_id();
           }
  [[nodiscard]] friend constexpr auto target_id(G&& /*g*/, const EdgeDesc& uv) noexcept {
    return uv.value().target_id();
  }

  template <typename G, typename EdgeDesc>
  requires weighted && std::same_as<std::remove_cvref_t<G>, adjacency_matrix> &&
           requires(const EdgeDesc& uv) {
             uv.value().edge_value_ref();
           }
  [[nodiscard]] friend constexpr decltype(auto) edge_value(G&& /*g*/, const EdgeDesc& uv) noexcept {
    return uv.value().edge_value_ref();
  }

protected:
  [[nodiscard]] constexpr const flag_t* flags_data() const noexcept { return flags_.data(); }

private:
  [[nodiscard]] constexpr std::size_t index(VId u, VId v) const noexcept {
    return static_cast<std::size_t>(u) * static_cast<std::size_t>(n_) + static_cast<std::size_t>(v);
  }

  constexpr void set_cell(VId u, VId v) {
    const std::size_t idx  = index(u, v);
    flag_t&           cell = flags_[idx];
    if (cell == flag_t{0}) {
      cell = flag_t{1};
      ++edge_count_;
    }
  }

  [[nodiscard]] constexpr iterator make_iter(VId row) const noexcept {
    return iterator{flags_.data(), cells_.data(), n_, row};
  }

  VId                       n_          = 0;
  std::size_t               edge_count_ = 0;
  std::vector<flag_t>       flags_;
  std::vector<element_type> cells_;
};

#if defined(__cpp_lib_mdspan)

/**
 * @ingroup graph_containers
 * @brief C++23 adjacency matrix that adds a `std::mdspan` access layer.
 *
 * Identical concept wiring and owning storage as @ref adjacency_matrix, but the
 * dense presence plane is additionally exposed as a 2-D `std::mdspan` view. This
 * gives ergonomic, zero-overhead `m(u, v)` element access and lets callers
 * reason about layout (`layout_right` keeps each out-edge row contiguous). The
 * `mdspan` is a *view* over the owned `std::vector`; it never owns the data and
 * is re-formed on demand so it cannot dangle across mutation.
 */
template <class EV = void, class VId = std::uint32_t, bool Directed = true>
requires std::integral<VId>
class md_adjacency_matrix : public adjacency_matrix<EV, VId, Directed> {
  using base   = adjacency_matrix<EV, VId, Directed>;
  using flag_t = _amx_detail::flag_t;

public:
  using extents_type = std::dextents<std::size_t, 2>;
  using flag_mdspan  = std::mdspan<const flag_t, extents_type, std::layout_right>;

  using base::base;

  /// 2-D view of the presence plane: `presence()[u, v] != 0` iff edge u->v exists.
  [[nodiscard]] flag_mdspan presence() const noexcept {
    const std::size_t n = static_cast<std::size_t>(this->order());
    return flag_mdspan{this->flags_data(), n, n};
  }

  /// True iff edge (u, v) is present.
  [[nodiscard]] bool exists(VId u, VId v) const noexcept {
    return presence()[static_cast<std::size_t>(u), static_cast<std::size_t>(v)] != flag_t{0};
  }
};

#endif // __cpp_lib_mdspan

} // namespace graph::container
