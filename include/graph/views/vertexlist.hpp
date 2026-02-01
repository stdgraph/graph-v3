/**
 * @file vertexlist.hpp
 * @brief Vertexlist view for iterating over graph vertices
 * 
 * Provides a view that iterates over all vertices in a graph, yielding
 * vertex_info<void, vertex_descriptor, VV> for each vertex. Supports
 * optional value functions to compute per-vertex values.
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

// Forward declaration
template <adj_list::adjacency_list G, class VVF = void>
class vertexlist_view;

/**
 * @brief Vertexlist view without value function
 * 
 * Iterates over vertices yielding vertex_info<void, vertex_t<G>, void>
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 */
template <adj_list::adjacency_list G>
class vertexlist_view<G, void> : public std::ranges::view_interface<vertexlist_view<G, void>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using info_type = vertex_info<void, vertex_type, void>;

    /**
     * @brief Forward iterator yielding vertex_info values
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = const value_type&;

        constexpr iterator() noexcept = default;

        constexpr iterator(G* g, vertex_type v) noexcept
            : g_(g), current_{v} {}

        [[nodiscard]] constexpr reference operator*() const noexcept {
            return current_;
        }

        constexpr iterator& operator++() noexcept {
            ++current_.vertex;
            return *this;
        }

        constexpr iterator operator++(int) noexcept {
            auto tmp = *this;
            ++current_.vertex;
            return tmp;
        }

        [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
            return current_.vertex == other.current_.vertex;
        }

    private:
        G* g_ = nullptr;
        value_type current_{};
    };

    using const_iterator = iterator;

    constexpr vertexlist_view() noexcept = default;

    constexpr explicit vertexlist_view(G& g) noexcept
        : g_(&g) {}

    [[nodiscard]] constexpr iterator begin() const noexcept {
        auto vert_range = adj_list::vertices(*g_);
        return iterator(g_, *std::ranges::begin(vert_range));
    }

    [[nodiscard]] constexpr iterator end() const noexcept {
        auto vert_range = adj_list::vertices(*g_);
        return iterator(g_, *std::ranges::end(vert_range));
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return adj_list::num_vertices(*g_);
    }

private:
    G* g_ = nullptr;
};

/**
 * @brief Vertexlist view with value function
 * 
 * Iterates over vertices yielding vertex_info<void, vertex_t<G>, VV>
 * where VV is the result of invoking the value function on the vertex descriptor.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam VVF Value function type
 */
template <adj_list::adjacency_list G, class VVF>
class vertexlist_view : public std::ranges::view_interface<vertexlist_view<G, VVF>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using value_type_result = std::invoke_result_t<VVF, vertex_type>;
    using info_type = vertex_info<void, vertex_type, value_type_result>;

    /**
     * @brief Forward iterator yielding vertex_info values with computed value
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(G* g, vertex_type v, VVF* vvf) noexcept
            : g_(g), current_(v), vvf_(vvf) {}

        [[nodiscard]] constexpr value_type operator*() const {
            return value_type{current_, std::invoke(*vvf_, current_)};
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

        [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
            return current_ == other.current_;
        }

    private:
        G* g_ = nullptr;
        vertex_type current_{};
        VVF* vvf_ = nullptr;
    };

    using const_iterator = iterator;

    constexpr vertexlist_view() noexcept = default;

    constexpr vertexlist_view(G& g, VVF vvf) noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), vvf_(std::move(vvf)) {}

    [[nodiscard]] constexpr iterator begin() noexcept {
        auto vert_range = adj_list::vertices(*g_);
        return iterator(g_, *std::ranges::begin(vert_range), &vvf_);
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        auto vert_range = adj_list::vertices(*g_);
        return iterator(g_, *std::ranges::end(vert_range), &vvf_);
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return adj_list::num_vertices(*g_);
    }

private:
    G* g_ = nullptr;
    [[no_unique_address]] VVF vvf_{};
};

// Deduction guides
template <adj_list::adjacency_list G>
vertexlist_view(G&) -> vertexlist_view<G, void>;

template <adj_list::adjacency_list G, class VVF>
vertexlist_view(G&, VVF) -> vertexlist_view<G, VVF>;

/**
 * @brief Create a vertexlist view without value function
 * 
 * @param g The graph to iterate over
 * @return vertexlist_view yielding vertex_info<void, vertex_descriptor, void>
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
 * @return vertexlist_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto vertexlist(G& g, VVF&& vvf) {
    return vertexlist_view<G, std::decay_t<VVF>>(g, std::forward<VVF>(vvf));
}

} // namespace graph::views
