/**
 * @file edgelist.hpp
 * @brief Edgelist view for iterating over all edges in a graph
 * 
 * Provides a view that flattens the two-level adjacency list structure into
 * a single range of edges, yielding edge_info<vertex_id_type, true, edge_t<G>, EV>
 * for each edge. Includes both source and target vertex IDs directly.
 * Supports optional value functions to compute per-edge values.
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

// Forward declaration
template <adj_list::adjacency_list G, class EVF = void>
class edgelist_view;

/**
 * @brief Edgelist view without value function
 * 
 * Iterates over all edges in the graph yielding edge_info<vertex_id_type, true, edge_t<G>, void>
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 */
template <adj_list::adjacency_list G>
class edgelist_view<G, void> : public std::ranges::view_interface<edgelist_view<G, void>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_range_type = adj_list::vertex_edge_range_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using info_type = edge_info<vertex_id_type, true, edge_type, void>;

    /**
     * @brief Forward iterator that flattens vertex-edge structure
     * 
     * Iterates through all vertices, and for each vertex, iterates through
     * all of its edges, presenting a single flat sequence of edges.
     * Uses vertex and edge descriptors directly, similar to vertexlist_view and incidence_view.
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(G* g, vertex_type v, vertex_type v_end, 
                          edge_type current_edge, edge_type edge_end) noexcept
            : g_(g), v_(v), v_end_(v_end), current_edge_(current_edge), edge_end_(edge_end) {}

        [[nodiscard]] constexpr value_type operator*() const noexcept {
            auto source_id = adj_list::vertex_id(*g_, v_);
            auto target_id = adj_list::target_id(*g_, current_edge_);
            return value_type{source_id, target_id, current_edge_};
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
                auto begin_it = std::ranges::begin(edge_range);
                auto end_it = std::ranges::end(edge_range);
                if (begin_it != end_it) {
                    current_edge_ = *begin_it;
                    edge_end_ = *end_it;
                    return;  // Found a vertex with edges
                }
                ++v_;
            }
            // Reached end - no more vertices with edges
        }

    private:
        G* g_ = nullptr;
        vertex_type v_{};           // Current vertex descriptor
        vertex_type v_end_{};       // End sentinel for vertices
        edge_type current_edge_{};  // Current edge descriptor
        edge_type edge_end_{};      // End sentinel for current vertex's edges
    };

    using const_iterator = iterator;

    constexpr edgelist_view() noexcept = default;

    constexpr edgelist_view(G& g) noexcept
        : g_(&g) {}

    [[nodiscard]] constexpr iterator begin() const noexcept {
        auto v_range = adj_list::vertices(*g_);
        auto v_begin = *std::ranges::begin(v_range);
        auto v_end = *std::ranges::end(v_range);
        
        // Create iterator and advance to first valid edge
        iterator it(g_, v_begin, v_end, edge_type{}, edge_type{});
        it.advance_to_valid_edge();
        return it;
    }

    [[nodiscard]] constexpr iterator end() const noexcept {
        auto v_range = adj_list::vertices(*g_);
        auto v_end = *std::ranges::end(v_range);
        return iterator(g_, v_end, v_end, edge_type{}, edge_type{});
    }

private:
    G* g_ = nullptr;
};

/**
 * @brief Edgelist view with value function
 * 
 * Iterates over all edges yielding edge_info<vertex_id_type, true, edge_t<G>, EV>
 * where EV is the result of invoking the value function on the edge descriptor.
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam EVF Edge value function type
 */
template <adj_list::adjacency_list G, class EVF>
class edgelist_view : public std::ranges::view_interface<edgelist_view<G, EVF>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_range_type = adj_list::vertex_edge_range_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using value_type_result = std::invoke_result_t<EVF, edge_type>;
    using info_type = edge_info<vertex_id_type, true, edge_type, value_type_result>;

    /**
     * @brief Forward iterator that flattens vertex-edge structure with value computation
     * 
     * Uses vertex and edge descriptors directly, similar to vertexlist_view and incidence_view.
     */
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(G* g, vertex_type v, vertex_type v_end, 
                          edge_type current_edge, edge_type edge_end, EVF* evf) noexcept
            : g_(g), v_(v), v_end_(v_end), current_edge_(current_edge), 
              edge_end_(edge_end), evf_(evf) {}

        [[nodiscard]] constexpr value_type operator*() const {
            auto source_id = adj_list::vertex_id(*g_, v_);
            auto target_id = adj_list::target_id(*g_, current_edge_);
            return value_type{source_id, target_id, current_edge_, std::invoke(*evf_, current_edge_)};
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
                auto begin_it = std::ranges::begin(edge_range);
                auto end_it = std::ranges::end(edge_range);
                if (begin_it != end_it) {
                    current_edge_ = *begin_it;
                    edge_end_ = *end_it;
                    return;
                }
                ++v_;
            }
        }

    private:
        G* g_ = nullptr;
        vertex_type v_{};            // Current vertex descriptor
        vertex_type v_end_{};        // End sentinel for vertices
        edge_type current_edge_{};   // Current edge descriptor
        edge_type edge_end_{};       // End sentinel for current vertex's edges
        EVF* evf_ = nullptr;
    };

    using const_iterator = iterator;

    constexpr edgelist_view() noexcept = default;

    constexpr edgelist_view(G& g, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : g_(&g), evf_(std::move(evf)) {}

    [[nodiscard]] constexpr iterator begin() noexcept {
        auto v_range = adj_list::vertices(*g_);
        auto v_begin = *std::ranges::begin(v_range);
        auto v_end = *std::ranges::end(v_range);
        
        iterator it(g_, v_begin, v_end, edge_type{}, edge_type{}, &evf_);
        it.advance_to_valid_edge();
        return it;
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        auto v_range = adj_list::vertices(*g_);
        auto v_end = *std::ranges::end(v_range);
        return iterator(g_, v_end, v_end, edge_type{}, edge_type{}, &evf_);
    }

private:
    G* g_ = nullptr;
    [[no_unique_address]] EVF evf_{};
};

// =============================================================================
// Factory Functions
// =============================================================================

/**
 * @brief Create an edgelist view over all edges in an adjacency list
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @param g The graph to iterate over
 * @return edgelist_view<G, void> yielding edge_info<vertex_id_type, true, edge_t<G>, void>
 */
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto edgelist(G& g) noexcept {
    return edgelist_view<G, void>(g);
}

/**
 * @brief Create an edgelist view with value function over all edges
 * 
 * @tparam G Graph type satisfying adjacency_list concept
 * @tparam EVF Edge value function type
 * @param g The graph to iterate over
 * @param evf Function to compute values from edge descriptors
 * @return edgelist_view<G, EVF> yielding edge_info<vertex_id_type, true, edge_t<G>, EV>
 */
template <adj_list::adjacency_list G, class EVF>
    requires edge_value_function<EVF, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto edgelist(G& g, EVF&& evf) 
    noexcept(std::is_nothrow_constructible_v<std::decay_t<EVF>, EVF>)
{
    return edgelist_view<G, std::decay_t<EVF>>(g, std::forward<EVF>(evf));
}

// =============================================================================
// Edge List Views (for edge_list data structures)
// =============================================================================

// Forward declaration
template <edge_list::basic_sourced_edgelist EL, class EVF = void>
class edge_list_edgelist_view;

/**
 * @brief Edgelist view for edge_list without value function
 * 
 * Wraps an edge_list range directly, yielding edge_info<vertex_id_type, true, edge_t<EL>, void>
 * 
 * @tparam EL Edge list type satisfying basic_sourced_edgelist concept
 */
template <edge_list::basic_sourced_edgelist EL>
class edge_list_edgelist_view<EL, void> : public std::ranges::view_interface<edge_list_edgelist_view<EL, void>> {
public:
    using edge_list_type = EL;
    using edge_type = edge_list::edge_t<EL>;
    using vertex_id_type = edge_list::vertex_id_t<EL>;
    using info_type = edge_info<vertex_id_type, true, edge_type, void>;

    /**
     * @brief Forward iterator wrapping edge_list iteration
     */
    class iterator {
    public:
        using base_iterator = std::ranges::iterator_t<EL>;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(EL* el, base_iterator it) noexcept
            : el_(el), current_(it) {}

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

        [[nodiscard]] constexpr bool operator==(const iterator& other) const noexcept {
            return current_ == other.current_;
        }

    private:
        EL* el_ = nullptr;
        base_iterator current_{};
    };

    using const_iterator = iterator;

    constexpr edge_list_edgelist_view() noexcept = default;

    constexpr edge_list_edgelist_view(EL& el) noexcept
        : el_(&el) {}

    [[nodiscard]] constexpr iterator begin() const noexcept {
        return iterator(el_, std::ranges::begin(*el_));
    }

    [[nodiscard]] constexpr iterator end() const noexcept {
        return iterator(el_, std::ranges::end(*el_));
    }

    [[nodiscard]] constexpr auto size() const noexcept
        requires std::ranges::sized_range<EL>
    {
        return std::ranges::size(*el_);
    }

private:
    EL* el_ = nullptr;
};

/**
 * @brief Edgelist view for edge_list with value function
 * 
 * Wraps an edge_list range, yielding edge_info<vertex_id_type, true, edge_t<EL>, EV>
 * where EV is the result of invoking the value function on the edge.
 * 
 * @tparam EL Edge list type satisfying basic_sourced_edgelist concept
 * @tparam EVF Edge value function type
 */
template <edge_list::basic_sourced_edgelist EL, class EVF>
class edge_list_edgelist_view : public std::ranges::view_interface<edge_list_edgelist_view<EL, EVF>> {
public:
    using edge_list_type = EL;
    using edge_type = edge_list::edge_t<EL>;
    using vertex_id_type = edge_list::vertex_id_t<EL>;
    using value_type_result = std::invoke_result_t<EVF, EL&, edge_type>;
    using info_type = edge_info<vertex_id_type, true, edge_type, value_type_result>;

    /**
     * @brief Forward iterator wrapping edge_list iteration with value computation
     */
    class iterator {
    public:
        using base_iterator = std::ranges::iterator_t<EL>;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;

        constexpr iterator() noexcept = default;

        constexpr iterator(EL* el, base_iterator it, EVF* evf) noexcept
            : el_(el), current_(it), evf_(evf) {}

        [[nodiscard]] constexpr value_type operator*() const {
            auto& edge = *current_;
            return value_type{graph::source_id(*el_, edge), graph::target_id(*el_, edge), edge, std::invoke(*evf_, *el_, edge)};
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
        EL* el_ = nullptr;
        base_iterator current_{};
        EVF* evf_ = nullptr;
    };

    using const_iterator = iterator;

    constexpr edge_list_edgelist_view() noexcept = default;

    constexpr edge_list_edgelist_view(EL& el, EVF evf) noexcept(std::is_nothrow_move_constructible_v<EVF>)
        : el_(&el), evf_(std::move(evf)) {}

    [[nodiscard]] constexpr iterator begin() noexcept {
        return iterator(el_, std::ranges::begin(*el_), &evf_);
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        return iterator(el_, std::ranges::end(*el_), &evf_);
    }

    [[nodiscard]] constexpr auto size() const noexcept
        requires std::ranges::sized_range<EL>
    {
        return std::ranges::size(*el_);
    }

private:
    EL* el_ = nullptr;
    [[no_unique_address]] EVF evf_{};
};

// =============================================================================
// Factory Functions for edge_list
// =============================================================================

/**
 * @brief Create an edgelist view over an edge_list
 * 
 * @tparam EL Edge list type satisfying basic_sourced_edgelist concept
 * @param el The edge list to iterate over
 * @return edge_list_edgelist_view<EL, void> yielding edge_info<vertex_id_type, true, edge_t<EL>, void>
 */
template <edge_list::basic_sourced_edgelist EL>
    requires (!adj_list::adjacency_list<EL>)  // Disambiguation: prefer adjacency_list overload
[[nodiscard]] constexpr auto edgelist(EL& el) noexcept {
    return edge_list_edgelist_view<EL, void>(el);
}

/**
 * @brief Create an edgelist view with value function over an edge_list
 * 
 * @tparam EL Edge list type satisfying basic_sourced_edgelist concept
 * @tparam EVF Edge value function type (receives edge_list& and edge)
 * @param el The edge list to iterate over
 * @param evf Function to compute values from edges
 * @return edge_list_edgelist_view<EL, EVF> yielding edge_info<vertex_id_type, true, edge_t<EL>, EV>
 */
template <edge_list::basic_sourced_edgelist EL, class EVF>
    requires (!adj_list::adjacency_list<EL>) &&  // Disambiguation: prefer adjacency_list overload
             std::invocable<EVF, EL&, edge_list::edge_t<EL>>
[[nodiscard]] constexpr auto edgelist(EL& el, EVF&& evf) 
    noexcept(std::is_nothrow_constructible_v<std::decay_t<EVF>, EVF>)
{
    return edge_list_edgelist_view<EL, std::decay_t<EVF>>(el, std::forward<EVF>(evf));
}

} // namespace graph::views
