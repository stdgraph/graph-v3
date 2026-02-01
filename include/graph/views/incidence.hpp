/**
 * @file incidence.hpp
 * @brief Incidence view for iterating over edges from a vertex
 * 
 * Provides a view that iterates over all outgoing edges from a given vertex,
 * yielding edge_info<void, false, edge_t<G>, EV> for each edge. Supports
 * optional value functions to compute per-edge values.
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
template <adj_list::adjacency_list G, class EVF = void>
class incidence_view;

/**
 * @brief Incidence view without value function
 * 
 * Iterates over edges from a vertex yielding edge_info<void, false, edge_t<G>, void>
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 */
template <adj_list::adjacency_list G>
class incidence_view<G, void> : public std::ranges::view_interface<incidence_view<G, void>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_range_type = adj_list::vertex_edge_range_t<G>;
    using edge_iterator_type = adj_list::vertex_edge_iterator_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using info_type = edge_info<void, false, edge_type, void>;

    /**
     * @brief Forward iterator yielding edge_info values
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = const value_type&;

        constexpr iterator() noexcept = default;

        constexpr iterator(edge_type e) noexcept
            : current_{e} {}

        [[nodiscard]] constexpr reference operator*() const noexcept {
            return current_;
        }

        constexpr iterator& operator++() noexcept {
            ++current_.edge;
            return *this;
        }

        constexpr iterator operator++(int) noexcept {
            auto tmp = *this;
            ++current_.edge;
            return tmp;
        }

        [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
            return current_.edge == other.current_.edge;
        }

    private:
        value_type current_{};
    };

    using const_iterator = iterator;

    constexpr incidence_view() noexcept = default;

    constexpr incidence_view(G& g, vertex_type u) noexcept
        : g_(&g), source_(u) {}

    [[nodiscard]] constexpr iterator begin() const noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(*std::ranges::begin(edge_range));
    }

    [[nodiscard]] constexpr iterator end() const noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        // edge_descriptor_view's end iterator can be dereferenced to get
        // an edge_descriptor with the end storage position
        return iterator(*std::ranges::end(edge_range));
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
 * @brief Incidence view with value function
 * 
 * Iterates over edges from a vertex yielding edge_info<void, false, edge_t<G>, EV>
 * where EV is the result of invoking the value function on the edge descriptor.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam EVF Edge value function type
 */
template <adj_list::adjacency_list G, class EVF>
class incidence_view : public std::ranges::view_interface<incidence_view<G, EVF>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_range_type = adj_list::vertex_edge_range_t<G>;
    using edge_iterator_type = adj_list::vertex_edge_iterator_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using value_type_result = std::invoke_result_t<EVF, edge_type>;
    using info_type = edge_info<void, false, edge_type, value_type_result>;

    /**
     * @brief Forward iterator yielding edge_info values with computed value
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(edge_type e, EVF* evf) noexcept
            : current_(e), evf_(evf) {}

        [[nodiscard]] constexpr value_type operator*() const {
            return value_type{current_, std::invoke(*evf_, current_)};
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
        edge_type current_{};
        EVF* evf_ = nullptr;
    };

    using const_iterator = iterator;

    constexpr incidence_view() noexcept = default;

    constexpr incidence_view(G& g, vertex_type u, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : g_(&g), source_(u), evf_(std::move(evf)) {}

    [[nodiscard]] constexpr iterator begin() noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(*std::ranges::begin(edge_range), &evf_);
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        auto edge_range = adj_list::edges(*g_, source_);
        return iterator(*std::ranges::end(edge_range), &evf_);
    }

    [[nodiscard]] constexpr auto size() const noexcept 
        requires std::ranges::sized_range<edge_range_type>
    {
        return std::ranges::size(adj_list::edges(*g_, source_));
    }

private:
    G* g_ = nullptr;
    vertex_type source_{};
    [[no_unique_address]] EVF evf_{};
};

// Deduction guides
template <adj_list::adjacency_list G>
incidence_view(G&, adj_list::vertex_t<G>) -> incidence_view<G, void>;

template <adj_list::adjacency_list G, class EVF>
incidence_view(G&, adj_list::vertex_t<G>, EVF) -> incidence_view<G, EVF>;

/**
 * @brief Create an incidence view without value function
 * 
 * @param g The graph to iterate over
 * @param u The source vertex descriptor
 * @return incidence_view yielding edge_info<void, false, edge_descriptor, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_t<G> u) noexcept {
    return incidence_view<G, void>(g, u);
}

/**
 * @brief Create an incidence view with value function
 * 
 * @param g The graph to iterate over
 * @param u The source vertex descriptor
 * @param evf Value function invoked for each edge
 * @return incidence_view yielding edge_info<void, false, edge_descriptor, EV>
 */
template <adj_list::adjacency_list G, class EVF>
    requires edge_value_function<EVF, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_t<G> u, EVF&& evf) {
    return incidence_view<G, std::decay_t<EVF>>(g, u, std::forward<EVF>(evf));
}

/**
 * @brief Create an incidence view using vertex id (convenience overload)
 * 
 * @param g The graph to iterate over
 * @param uid The source vertex id
 * @return incidence_view yielding edge_info<void, false, edge_descriptor, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_id_t<G> uid) noexcept {
    auto u = *adj_list::find_vertex(g, uid);
    return incidence_view<G, void>(g, u);
}

/**
 * @brief Create an incidence view with value function using vertex id (convenience overload)
 * 
 * @param g The graph to iterate over
 * @param uid The source vertex id
 * @param evf Value function invoked for each edge
 * @return incidence_view yielding edge_info<void, false, edge_descriptor, EV>
 */
template <adj_list::adjacency_list G, class EVF>
    requires edge_value_function<EVF, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto incidence(G& g, adj_list::vertex_id_t<G> uid, EVF&& evf) {
    auto u = *adj_list::find_vertex(g, uid);
    return incidence_view<G, std::decay_t<EVF>>(g, u, std::forward<EVF>(evf));
}

} // namespace graph::views
