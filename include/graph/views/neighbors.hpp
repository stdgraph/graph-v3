/**
 * @file neighbors.hpp
 * @brief Neighbors view for iterating over adjacent vertices
 * 
 * Provides a view that iterates over all neighbor vertices reachable from
 * a given vertex, yielding neighbor_info<void, false, vertex_t<G>, VV> for
 * each neighbor. Supports optional value functions to compute per-neighbor values.
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
class neighbors_view;

/**
 * @brief Neighbors view without value function
 * 
 * Iterates over neighbors yielding neighbor_info<void, false, vertex_t<G>, void>
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 */
template <adj_list::adjacency_list G>
class neighbors_view<G, void> : public std::ranges::view_interface<neighbors_view<G, void>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_range_type = adj_list::vertex_edge_range_t<G>;
    using edge_iterator_type = adj_list::vertex_edge_iterator_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using info_type = neighbor_info<vertex_id_type, false, vertex_type, void>;

    /**
     * @brief Forward iterator yielding neighbor_info values
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(G* g, edge_type e) noexcept
            : g_(g), current_edge_(e) {}

        [[nodiscard]] constexpr value_type operator*() const noexcept {
            // Get target vertex descriptor from the edge
            auto target = adj_list::target(*g_, current_edge_);
            auto target_id = adj_list::vertex_id(*g_, target);
            return value_type{target_id, target};
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
        G* g_ = nullptr;
        edge_type current_edge_{};
    };

    using const_iterator = iterator;

    constexpr neighbors_view() noexcept = default;

    constexpr neighbors_view(G& g, vertex_type u) noexcept
        : g_(&g), source_(u) {}

    [[nodiscard]] constexpr iterator begin() const noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(g_, *std::ranges::begin(edge_range));
    }

    [[nodiscard]] constexpr iterator end() const noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(g_, *std::ranges::end(edge_range));
    }

    [[nodiscard]] constexpr auto size() const noexcept 
        requires std::ranges::sized_range<edge_range_type>
    {
        return std::ranges::size(adj_list::edges(*g_, source_));
    }

private:
    G* g_ = nullptr;
    vertex_type source_{};
};

/**
 * @brief Neighbors view with value function
 * 
 * Iterates over neighbors yielding neighbor_info<void, false, vertex_t<G>, VV>
 * where VV is the result of invoking the value function on the target vertex descriptor.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam VVF Vertex value function type
 */
template <adj_list::adjacency_list G, class VVF>
class neighbors_view : public std::ranges::view_interface<neighbors_view<G, VVF>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_range_type = adj_list::vertex_edge_range_t<G>;
    using edge_iterator_type = adj_list::vertex_edge_iterator_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using value_type_result = std::invoke_result_t<VVF, vertex_type>;
    using info_type = neighbor_info<vertex_id_type, false, vertex_type, value_type_result>;

    /**
     * @brief Forward iterator yielding neighbor_info values with computed value
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(G* g, edge_type e, VVF* vvf) noexcept
            : g_(g), current_edge_(e), vvf_(vvf) {}

        [[nodiscard]] constexpr value_type operator*() const {
            auto target = adj_list::target(*g_, current_edge_);
            auto target_id = adj_list::vertex_id(*g_, target);
            return value_type{target_id, target, std::invoke(*vvf_, target)};
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
        G* g_ = nullptr;
        edge_type current_edge_{};
        VVF* vvf_ = nullptr;
    };

    using const_iterator = iterator;

    constexpr neighbors_view() noexcept = default;

    constexpr neighbors_view(G& g, vertex_type u, VVF vvf) noexcept(std::is_nothrow_move_constructible_v<VVF>)
        : g_(&g), source_(u), vvf_(std::move(vvf)) {}

    [[nodiscard]] constexpr iterator begin() noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(g_, *std::ranges::begin(edge_range), &vvf_);
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(g_, *std::ranges::end(edge_range), &vvf_);
    }

    [[nodiscard]] constexpr auto size() const noexcept 
        requires std::ranges::sized_range<edge_range_type>
    {
        return std::ranges::size(adj_list::edges(*g_, source_));
    }

private:
    G* g_ = nullptr;
    vertex_type source_{};
    [[no_unique_address]] VVF vvf_{};
};

// Deduction guides
template <adj_list::adjacency_list G>
neighbors_view(G&, adj_list::vertex_t<G>) -> neighbors_view<G, void>;

template <adj_list::adjacency_list G, class VVF>
neighbors_view(G&, adj_list::vertex_t<G>, VVF) -> neighbors_view<G, VVF>;

/**
 * @brief Create a neighbors view without value function
 * 
 * @param g The graph to iterate over
 * @param u The source vertex descriptor
 * @return neighbors_view yielding neighbor_info<void, false, vertex_descriptor, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto neighbors(G& g, adj_list::vertex_t<G> u) noexcept {
    return neighbors_view<G, void>(g, u);
}

/**
 * @brief Create a neighbors view without value function (vertex_id overload)
 * 
 * @param g The graph to iterate over
 * @param uid The source vertex id
 * @return neighbors_view yielding neighbor_info<void, false, vertex_descriptor, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto neighbors(G& g, adj_list::vertex_id_t<G> uid) noexcept {
    auto u = adj_list::find_vertex(g, uid);
    return neighbors_view<G, void>(g, *u);
}

/**
 * @brief Create a neighbors view with value function
 * 
 * @param g The graph to iterate over
 * @param u The source vertex descriptor
 * @param vvf Value function invoked for each target vertex
 * @return neighbors_view yielding neighbor_info<void, false, vertex_descriptor, VV>
 */
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto neighbors(G& g, adj_list::vertex_t<G> u, VVF&& vvf) {
    return neighbors_view<G, std::decay_t<VVF>>(g, u, std::forward<VVF>(vvf));
}

/**
 * @brief Create a neighbors view with value function (vertex_id overload)
 * 
 * @param g The graph to iterate over
 * @param uid The source vertex id
 * @param vvf Value function invoked for each target vertex
 * @return neighbors_view yielding neighbor_info<void, false, vertex_descriptor, VV>
 */
template <adj_list::adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] constexpr auto neighbors(G& g, adj_list::vertex_id_t<G> uid, VVF&& vvf) {
    auto u = adj_list::find_vertex(g, uid);
    return neighbors_view<G, std::decay_t<VVF>>(g, *u, std::forward<VVF>(vvf));
}

} // namespace graph::views
