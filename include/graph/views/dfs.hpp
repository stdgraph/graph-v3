/**
 * @file dfs.hpp
 * @brief Depth-first search views for vertices and edges
 * 
 * Provides views that traverse a graph in depth-first order from a seed vertex,
 * yielding vertex_info or edge_info for each visited element.
 * 
 * @par Examples:
 * @code
 * // Vertex traversal
 * for (auto [v] : vertices_dfs(g, seed))
 *     process_vertex(v);
 * 
 * // Vertex traversal with value function
 * for (auto [v, val] : vertices_dfs(g, seed, value_fn))
 *     process_vertex_with_value(v, val);
 * 
 * // Edge traversal
 * for (auto [e] : edges_dfs(g, seed))
 *     process_edge(e);
 * 
 * // Cancel search
 * auto dfs = vertices_dfs(g, seed);
 * for (auto [v] : dfs) {
 *     if (should_stop(v))
 *         dfs.cancel(cancel_search::cancel_all);
 * }
 * @endcode
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <memory>
#include <stack>
#include <vector>
#include <graph/graph_info.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/search_base.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::index_adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>>
class vertices_dfs_view;

template <adj_list::index_adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>>
class edges_dfs_view;

namespace dfs_detail {

/// Stack entry for DFS traversal: stores vertex descriptor and edge iterator
template <class Vertex, class EdgeIterator>
struct stack_entry {
    Vertex       vertex;
    EdgeIterator edge_iter;
    EdgeIterator edge_end;
};

/// Shared DFS state - allows iterator copies to share traversal state
/// Uses vertex descriptors internally, vertex IDs only for visited tracking
///
/// Why shared_ptr?
/// 1. Iterator copies must share state: When you copy an iterator (e.g., auto it2 = it1),
///    both must refer to the same DFS traversal. Advancing it1 affects what it2 sees.
/// 2. View and iterators share state: The view exposes depth(), size(), and cancel()
///    accessors that reflect current traversal state modified by iterators.
/// 3. Range-based for loop: The view's cancel() must be able to stop iteration in progress.
/// 4. Input iterator semantics: DFS is single-pass; shared state correctly models this.
/// An alternative (state by value + raw pointers) would break if the view is moved.
template <class G, class Alloc>
struct dfs_state {
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_iterator_type = adj_list::vertex_edge_iterator_t<G>;
    using allocator_type = Alloc;
    using entry_type = stack_entry<vertex_type, edge_iterator_type>;
    using stack_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<entry_type>;
    
    std::stack<entry_type, std::vector<entry_type, stack_alloc>> stack_;
    visited_tracker<vertex_id_type, Alloc> visited_;
    cancel_search cancel_ = cancel_search::continue_search;
    std::size_t depth_ = 0;
    std::size_t count_ = 0;
    
    dfs_state(G& g, vertex_type seed_vertex, std::size_t num_vertices, Alloc alloc = {})
        : stack_(std::vector<entry_type, stack_alloc>(alloc))
        , visited_(num_vertices, alloc) 
    {
        auto edge_range = adj_list::edges(g, seed_vertex);
        stack_.push({seed_vertex, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
        visited_.mark_visited(adj_list::vertex_id(g, seed_vertex));
        // Note: count_ is not incremented here. It's incremented in advance() when
        // a vertex is actually yielded by the iterator.
    }
};

} // namespace dfs_detail

/**
 * @brief DFS vertex view without value function
 * 
 * Iterates over vertices in depth-first order yielding 
 * vertex_info<void, vertex_t<G>, void>
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class Alloc>
class vertices_dfs_view<G, void, Alloc> : public std::ranges::view_interface<vertices_dfs_view<G, void, Alloc>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using allocator_type = Alloc;
    using info_type = vertex_info<void, vertex_type, void>;
    
private:
    using state_type = dfs_detail::dfs_state<G, Alloc>;
    
public:
    /**
     * @brief Input iterator for DFS vertex traversal
     * 
     * Note: This is an input iterator because DFS state is shared and 
     * advancing one iterator affects all copies.
     */
    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;
        
        constexpr iterator() noexcept = default;
        
        iterator(G* g, std::shared_ptr<state_type> state)
            : g_(g), state_(std::move(state)) {}
        
        [[nodiscard]] value_type operator*() const {
            return value_type{state_->stack_.top().vertex};
        }
        
        iterator& operator++() {
            advance();
            return *this;
        }
        
        iterator operator++(int) {
            auto tmp = *this;
            advance();
            return tmp;
        }
        
        [[nodiscard]] bool operator==(const iterator& other) const noexcept {
            // Both at end if state is null or stack is empty
            bool this_at_end = !state_ || state_->stack_.empty();
            bool other_at_end = !other.state_ || other.state_->stack_.empty();
            return this_at_end == other_at_end;
        }
        
        [[nodiscard]] bool at_end() const noexcept {
            return !state_ || state_->stack_.empty();
        }
        
    private:
        void advance() {
            if (!state_ || state_->stack_.empty()) return;
            if (state_->cancel_ == cancel_search::cancel_all) {
                while (!state_->stack_.empty()) state_->stack_.pop();
                return;
            }
            
            // Find next unvisited vertex using DFS
            while (!state_->stack_.empty()) {
                auto& top = state_->stack_.top();
                
                // Find next unvisited neighbor from current vertex
                bool found_unvisited = false;
                while (top.edge_iter != top.edge_end) {
                    auto edge = *top.edge_iter;
                    ++top.edge_iter;
                    
                    // Get target vertex descriptor using CPO
                    auto target_v = adj_list::target(*g_, edge);
                    auto target_vid = adj_list::vertex_id(*g_, target_v);
                    
                    if (!state_->visited_.is_visited(target_vid)) {
                        state_->visited_.mark_visited(target_vid);
                        
                        // Push target vertex with its edge range
                        auto edge_range = adj_list::edges(*g_, target_v);
                        state_->stack_.push({target_v, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
                        ++state_->depth_;
                        ++state_->count_;
                        found_unvisited = true;
                        break;
                    }
                }
                
                if (found_unvisited) {
                    return;  // Found next vertex to visit
                }
                
                // No more unvisited neighbors, backtrack
                state_->stack_.pop();
                if (state_->depth_ > 0) --state_->depth_;
            }
        }
        
        G* g_ = nullptr;
        std::shared_ptr<state_type> state_;
    };
    
    /// Sentinel for end of DFS traversal
    struct sentinel {
        [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept {
            return it.at_end();
        }
    };
    
    constexpr vertices_dfs_view() noexcept = default;
    
    /// Construct from vertex descriptor
    vertices_dfs_view(G& g, vertex_type seed_vertex, Alloc alloc = {})
        : g_(&g)
        , state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc))
    {}
    
    /// Construct from vertex ID (delegates to vertex descriptor constructor)
    vertices_dfs_view(G& g, vertex_id_type seed, Alloc alloc = {})
        : vertices_dfs_view(g, *adj_list::find_vertex(g, seed), alloc)
    {}
    
    [[nodiscard]] iterator begin() { return iterator(g_, state_); }
    [[nodiscard]] sentinel end() const noexcept { return {}; }
    
    /// Get current cancel state
    [[nodiscard]] cancel_search cancel() const noexcept { 
        return state_ ? state_->cancel_ : cancel_search::continue_search; 
    }
    
    /// Set cancel state to stop traversal
    void cancel(cancel_search c) noexcept { 
        if (state_) state_->cancel_ = c; 
    }
    
    /// Get current depth in DFS tree (stack size)
    [[nodiscard]] std::size_t depth() const noexcept { 
        return state_ ? state_->stack_.size() : 0; 
    }
    
    /// Get count of vertices visited so far
    [[nodiscard]] std::size_t size() const noexcept { 
        return state_ ? state_->count_ : 0; 
    }
    
private:
    G* g_ = nullptr;
    std::shared_ptr<state_type> state_;
};

/**
 * @brief DFS vertex view with value function
 * 
 * Iterates over vertices in depth-first order yielding 
 * vertex_info<void, vertex_t<G>, VV> where VV is the invoke result of VVF.
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam VVF Vertex value function type
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
class vertices_dfs_view : public std::ranges::view_interface<vertices_dfs_view<G, VVF, Alloc>> {
public:
    using graph_type = G;
    using vertex_type = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_type = adj_list::edge_t<G>;
    using allocator_type = Alloc;
    using value_result_type = std::invoke_result_t<VVF, vertex_type>;
    using info_type = vertex_info<void, vertex_type, value_result_type>;
    
private:
    using state_type = dfs_detail::dfs_state<G, Alloc>;
    
public:
    /**
     * @brief Input iterator for DFS vertex traversal with value function
     */
    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = info_type;
        using pointer = const value_type*;
        using reference = value_type;
        
        constexpr iterator() noexcept = default;
        
        iterator(G* g, std::shared_ptr<state_type> state, VVF* vvf)
            : g_(g), state_(std::move(state)), vvf_(vvf) {}
        
        [[nodiscard]] value_type operator*() const {
            auto v = state_->stack_.top().vertex;
            return value_type{v, std::invoke(*vvf_, v)};
        }
        
        iterator& operator++() {
            advance();
            return *this;
        }
        
        iterator operator++(int) {
            auto tmp = *this;
            advance();
            return tmp;
        }
        
        [[nodiscard]] bool operator==(const iterator& other) const noexcept {
            bool this_at_end = !state_ || state_->stack_.empty();
            bool other_at_end = !other.state_ || other.state_->stack_.empty();
            return this_at_end == other_at_end;
        }
        
        [[nodiscard]] bool at_end() const noexcept {
            return !state_ || state_->stack_.empty();
        }
        
    private:
        void advance() {
            if (!state_ || state_->stack_.empty()) return;
            if (state_->cancel_ == cancel_search::cancel_all) {
                while (!state_->stack_.empty()) state_->stack_.pop();
                return;
            }
            
            // Find next unvisited vertex using DFS
            while (!state_->stack_.empty()) {
                auto& top = state_->stack_.top();
                
                // Find next unvisited neighbor from current vertex
                bool found_unvisited = false;
                while (top.edge_iter != top.edge_end) {
                    auto edge = *top.edge_iter;
                    ++top.edge_iter;
                    
                    // Get target vertex descriptor using CPO
                    auto target_v = adj_list::target(*g_, edge);
                    auto target_vid = adj_list::vertex_id(*g_, target_v);
                    
                    if (!state_->visited_.is_visited(target_vid)) {
                        state_->visited_.mark_visited(target_vid);
                        
                        // Push target vertex with its edge range
                        auto edge_range = adj_list::edges(*g_, target_v);
                        state_->stack_.push({target_v, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
                        ++state_->depth_;
                        ++state_->count_;
                        found_unvisited = true;
                        break;
                    }
                }
                
                if (found_unvisited) {
                    return;  // Found next vertex to visit
                }
                
                // No more unvisited neighbors, backtrack
                state_->stack_.pop();
                if (state_->depth_ > 0) --state_->depth_;
            }
        }
        
        G* g_ = nullptr;
        std::shared_ptr<state_type> state_;
        VVF* vvf_ = nullptr;
    };
    
    struct sentinel {
        [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept {
            return it.at_end();
        }
    };
    
    constexpr vertices_dfs_view() noexcept = default;
    
    /// Construct from vertex descriptor
    vertices_dfs_view(G& g, vertex_type seed_vertex, VVF vvf, Alloc alloc = {})
        : g_(&g)
        , vvf_(std::move(vvf))
        , state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc))
    {}
    
    /// Construct from vertex ID (delegates to vertex descriptor constructor)
    vertices_dfs_view(G& g, vertex_id_type seed, VVF vvf, Alloc alloc = {})
        : vertices_dfs_view(g, *adj_list::find_vertex(g, seed), std::move(vvf), alloc)
    {}
    
    [[nodiscard]] iterator begin() { return iterator(g_, state_, &vvf_); }
    [[nodiscard]] sentinel end() const noexcept { return {}; }
    
    [[nodiscard]] cancel_search cancel() const noexcept { 
        return state_ ? state_->cancel_ : cancel_search::continue_search; 
    }
    
    void cancel(cancel_search c) noexcept { 
        if (state_) state_->cancel_ = c; 
    }
    
    [[nodiscard]] std::size_t depth() const noexcept { 
        return state_ ? state_->stack_.size() : 0; 
    }
    
    [[nodiscard]] std::size_t size() const noexcept { 
        return state_ ? state_->count_ : 0; 
    }
    
private:
    G* g_ = nullptr;
    [[no_unique_address]] VVF vvf_{};
    std::shared_ptr<state_type> state_;
};

// Deduction guides for vertex_id
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
vertices_dfs_view(G&, adj_list::vertex_id_t<G>, Alloc) -> vertices_dfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
vertices_dfs_view(G&, adj_list::vertex_id_t<G>) -> vertices_dfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class VVF, class Alloc = std::allocator<bool>>
vertices_dfs_view(G&, adj_list::vertex_id_t<G>, VVF, Alloc) -> vertices_dfs_view<G, VVF, Alloc>;

// Deduction guides for vertex descriptor
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
vertices_dfs_view(G&, adj_list::vertex_t<G>, Alloc) -> vertices_dfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
vertices_dfs_view(G&, adj_list::vertex_t<G>) -> vertices_dfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class VVF, class Alloc = std::allocator<bool>>
vertices_dfs_view(G&, adj_list::vertex_t<G>, VVF, Alloc) -> vertices_dfs_view<G, VVF, Alloc>;

/**
 * @brief Create a DFS vertex view without value function (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for DFS
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed) {
    return vertices_dfs_view<G, void, std::allocator<bool>>(g, seed, std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view without value function (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for DFS
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex) {
    return vertices_dfs_view<G, void, std::allocator<bool>>(g, seed_vertex, std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view with value function (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for DFS
 * @param vvf Value function invoked for each vertex
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf) {
    return vertices_dfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed, std::forward<VVF>(vvf), std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view with value function (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for DFS
 * @param vvf Value function invoked for each vertex
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf) {
    return vertices_dfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed_vertex, std::forward<VVF>(vvf), std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view with custom allocator (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for DFS
 * @param alloc Allocator for internal containers
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G, class Alloc>
    requires (!vertex_value_function<Alloc, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, Alloc alloc) {
    return vertices_dfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief Create a DFS vertex view with custom allocator (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for DFS
 * @param alloc Allocator for internal containers
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G, class Alloc>
    requires (!vertex_value_function<Alloc, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
    return vertices_dfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief Create a DFS vertex view with value function and custom allocator (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for DFS
 * @param vvf Value function invoked for each vertex
 * @param alloc Allocator for internal containers
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf, Alloc alloc) {
    return vertices_dfs_view<G, std::decay_t<VVF>, Alloc>(g, seed, std::forward<VVF>(vvf), alloc);
}

/**
 * @brief Create a DFS vertex view with value function and custom allocator (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for DFS
 * @param vvf Value function invoked for each vertex
 * @param alloc Allocator for internal containers
 * @return vertices_dfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
    requires vertex_value_function<VVF, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf, Alloc alloc) {
    return vertices_dfs_view<G, std::decay_t<VVF>, Alloc>(g, seed_vertex, std::forward<VVF>(vvf), alloc);
}

} // namespace graph::views
