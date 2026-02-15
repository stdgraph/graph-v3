/**
 * @file topological_sort.hpp
 * @brief Topological sort view for directed acyclic graphs (DAGs)
 * 
 * Provides a view that traverses a DAG in topological order, where each vertex
 * appears before all vertices it has edges to. Uses reverse DFS post-order.
 * 
 * @complexity Time: O(V + E) where V is number of vertices and E is number of edges
 *             DFS visits each vertex once and traverses each edge once
 * @complexity Space: O(V) for post-order vector and visited tracker
 * 
 * @par Examples:
 * @code
 * // Topological sort of all vertices
 * for (auto [v] : vertices_topological_sort(g))
 *     process_vertex_in_topo_order(v);
 * 
 * // With value function
 * for (auto [v, val] : vertices_topological_sort(g, value_fn))
 *     process_vertex_with_value(v, val);
 * @endcode
 * 
 * @note This view processes ALL vertices in the graph, not just from a single seed.
 *       For connected components, vertices are processed in topological order within
 *       each component, with components visited in arbitrary order.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <memory>
#include <stack>
#include <vector>
#include <algorithm>
#include <optional>
#include <tl/expected.hpp>
#include <graph/graph_info.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/search_base.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::index_adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>>
class vertices_topological_sort_view;

template <adj_list::index_adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>>
class edges_topological_sort_view;

namespace topo_detail {

  /**
 * @brief Shared topological sort state
 * 
 * Performs DFS from all unvisited vertices, collecting post-order into a vector,
 * then provides reverse iteration over that vector for topological order.
 * 
 * @complexity Time: O(V + E) - visits each vertex once, traverses each edge once
 * @complexity Space: O(V) - stores all vertices in post-order vector plus visited tracker
 *             When cycle detection enabled: O(2V) for additional recursion stack tracker
 */
  template <class G, class Alloc>
  struct topo_state {
    using graph_type     = G;
    using vertex_type    = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using allocator_type = Alloc;
    using vertex_alloc   = typename std::allocator_traits<Alloc>::template rebind_alloc<vertex_type>;

    std::vector<vertex_type, vertex_alloc> post_order_; // DFS post-order
    visited_tracker<vertex_id_type, Alloc> visited_;
    std::optional<vertex_type>             cycle_vertex_;   // Set if cycle detected
    std::vector<bool, Alloc>               rec_stack_;      // Only allocated if detect_cycles=true
    std::size_t                            count_ = 0;      ///< Iteration progress counter (incremented by iterators)
    cancel_search cancel_ = cancel_search::continue_search; ///< Cancel control for early termination

    topo_state(G& g, Alloc alloc = {}, bool detect_cycles = false)
          : post_order_(vertex_alloc(alloc))
          , visited_(adj_list::num_vertices(g), alloc)
          , rec_stack_(detect_cycles ? adj_list::num_vertices(g) : 0, false, alloc) {
      post_order_.reserve(adj_list::num_vertices(g));

      // Perform DFS from all unvisited vertices
      for (auto v : adj_list::vertices(g)) {
        auto vid = adj_list::vertex_id(g, v);
        if (!visited_.is_visited(vid)) {
          dfs_visit(g, v, detect_cycles);
          if (cycle_vertex_) {
            return; // Early exit on cycle detection
          }
        }
      }

      // Reverse for topological order (only if no cycle)
      if (!cycle_vertex_) {
        std::ranges::reverse(post_order_);
      }
    }

    [[nodiscard]] bool                              has_cycle() const noexcept { return cycle_vertex_.has_value(); }
    [[nodiscard]] const std::optional<vertex_type>& cycle_vertex() const noexcept { return cycle_vertex_; }

  private:
    // Recursive DFS visit for topological sort
    void dfs_visit(G& g, vertex_type v, bool detect_cycles) {
      auto vid = adj_list::vertex_id(g, v);
      visited_.mark_visited(vid);

      if (detect_cycles) {
        rec_stack_[vid] = true;
      }

      // Visit all children
      for (auto edge : adj_list::edges(g, v)) {
        auto target_v   = adj_list::target(g, edge);
        auto target_vid = adj_list::vertex_id(g, target_v);

        if (detect_cycles && rec_stack_[target_vid]) {
          // Back edge detected - target_v closes the cycle
          cycle_vertex_ = target_v;
          return;
        }

        if (!visited_.is_visited(target_vid)) {
          dfs_visit(g, target_v, detect_cycles);
          if (cycle_vertex_) {
            return; // Propagate early exit
          }
        }
      }

      if (detect_cycles) {
        rec_stack_[vid] = false;
      }

      // Add to post-order after all children visited
      post_order_.push_back(v);
    }
  };

} // namespace topo_detail

/**
 * @brief Topological sort vertex view without value function
 * 
 * Iterates over all vertices in topological order, yielding 
 * vertex_info<void, vertex_t<G>, void>
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class Alloc>
class vertices_topological_sort_view<G, void, Alloc>
      : public std::ranges::view_interface<vertices_topological_sort_view<G, void, Alloc>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using allocator_type = Alloc;
  using info_type      = vertex_info<void, vertex_type, void>;

private:
  using state_type = topo_detail::topo_state<G, Alloc>;

public:
  /**
     * @brief Forward iterator for topological order traversal
     */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(std::shared_ptr<state_type> state, std::size_t index) : state_(std::move(state)), index_(index) {}

    [[nodiscard]] value_type operator*() const { return value_type{state_->post_order_[index_]}; }

    iterator& operator++() {
      ++index_;
      if (state_)
        ++state_->count_;
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator& other) const noexcept {
      // Both at end, or both at same position in same state
      if (!state_ && !other.state_)
        return true;
      if (!state_ || !other.state_)
        return false;
      return state_ == other.state_ && index_ == other.index_;
    }

    [[nodiscard]] bool at_end() const noexcept {
      return !state_ || index_ >= state_->post_order_.size() || state_->cancel_ != cancel_search::continue_search;
    }

  private:
    std::shared_ptr<state_type> state_;
    std::size_t                 index_ = 0;
  };

  /// Sentinel for end of traversal
  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr vertices_topological_sort_view() noexcept = default;

  /// Construct topological sort view for entire graph
  vertices_topological_sort_view(G& g, Alloc alloc = {}) : g_(&g), state_(std::make_shared<state_type>(g, alloc)) {}

  /// Construct with pre-built state (used by _safe functions)
  vertices_topological_sort_view(G& g, std::shared_ptr<state_type> state) : g_(&g), state_(std::move(state)) {}

  [[nodiscard]] iterator begin() { return iterator(state_, 0); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  /// Get total number of vertices in topological order
  [[nodiscard]] std::size_t size() const noexcept { return state_ ? state_->post_order_.size() : 0; }

  /// Get count of vertices consumed during iteration so far.
  /// Starts at 0 and increments with each operator++ call.
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

  /// Get current cancel state
  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  /// Set cancel state to stop iteration early.
  /// cancel_branch is treated as cancel_all (no branch semantics in flat ordering).
  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

private:
  G*                          g_ = nullptr;
  std::shared_ptr<state_type> state_;
};

/**
 * @brief Topological sort vertex view with value function
 * 
 * Iterates over all vertices in topological order, yielding 
 * vertex_info<void, vertex_t<G>, VV> where VV is the invoke result of VVF.
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam VVF Vertex value function type
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
class vertices_topological_sort_view
      : public std::ranges::view_interface<vertices_topological_sort_view<G, VVF, Alloc>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_info<void, vertex_type, value_result_type>;

private:
  using state_type = topo_detail::topo_state<G, Alloc>;

public:
  /**
     * @brief Forward iterator with value function
     */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state, std::size_t index, VVF* vvf)
          : g_(g), state_(std::move(state)), index_(index), vvf_(vvf) {}

    [[nodiscard]] value_type operator*() const {
      auto v = state_->post_order_[index_];
      return value_type{v, std::invoke(*vvf_, std::as_const(*g_), v)};
    }

    iterator& operator++() {
      ++index_;
      if (state_)
        ++state_->count_;
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator& other) const noexcept {
      if (!state_ && !other.state_)
        return true;
      if (!state_ || !other.state_)
        return false;
      return state_ == other.state_ && index_ == other.index_;
    }

    [[nodiscard]] bool at_end() const noexcept {
      return !state_ || index_ >= state_->post_order_.size() || state_->cancel_ != cancel_search::continue_search;
    }

  private:
    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    std::size_t                 index_ = 0;
    VVF*                        vvf_   = nullptr;
  };

  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr vertices_topological_sort_view() noexcept = default;

  /// Construct with value function
  vertices_topological_sort_view(G& g, VVF vvf, Alloc alloc = {})
        : g_(&g), vvf_(std::move(vvf)), state_(std::make_shared<state_type>(g, alloc)) {}

  /// Construct with value function and pre-built state (used by _safe functions)
  vertices_topological_sort_view(G& g, VVF vvf, std::shared_ptr<state_type> state)
        : g_(&g), vvf_(std::move(vvf)), state_(std::move(state)) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, 0, &vvf_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  [[nodiscard]] std::size_t size() const noexcept { return state_ ? state_->post_order_.size() : 0; }

  /// Get count of vertices consumed during iteration so far.
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

  /// Get current cancel state
  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  /// Set cancel state to stop iteration early.
  /// cancel_branch is treated as cancel_all (no branch semantics in flat ordering).
  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

private:
  G*                          g_ = nullptr;
  [[no_unique_address]] VVF   vvf_{};
  std::shared_ptr<state_type> state_;
};

// Deduction guides
template <class G, class Alloc>
vertices_topological_sort_view(G&, Alloc) -> vertices_topological_sort_view<G, void, Alloc>;

template <class G>
vertices_topological_sort_view(G&) -> vertices_topological_sort_view<G, void, std::allocator<bool>>;

template <class G, class VVF, class Alloc>
vertices_topological_sort_view(G&, VVF, Alloc) -> vertices_topological_sort_view<G, VVF, Alloc>;

/**
 * @brief Topological sort edge view without value function
 * 
 * Iterates over all edges in topological order (by source vertex), yielding
 * edge_info<void, false, edge_t<G>, void>
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class Alloc>
class edges_topological_sort_view<G, void, Alloc>
      : public std::ranges::view_interface<edges_topological_sort_view<G, void, Alloc>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using edge_type      = adj_list::edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = edge_info<void, false, edge_type, void>;

private:
  using state_type = topo_detail::topo_state<G, Alloc>;

public:
  /**
     * @brief Forward iterator for edges in topological order
     */
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state, std::size_t v_idx)
          : g_(g), state_(std::move(state)), vertex_index_(v_idx) {
      if (!at_end()) {
        auto v          = state_->post_order_[vertex_index_];
        auto edge_range = adj_list::edges(*g_, v);
        edge_it_        = std::ranges::begin(edge_range);
        edge_end_       = std::ranges::end(edge_range);
        skip_to_first_edge(); // Position without incrementing count_
      }
    }

    [[nodiscard]] value_type operator*() const { return value_type{*edge_it_}; }

    iterator& operator++() {
      ++edge_it_;
      advance_to_next_edge();
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator& other) const noexcept {
      if (at_end() && other.at_end())
        return true;
      if (at_end() || other.at_end())
        return false;
      return vertex_index_ == other.vertex_index_ && edge_it_ == other.edge_it_;
    }

    [[nodiscard]] bool at_end() const noexcept {
      return !state_ || vertex_index_ >= state_->post_order_.size() ||
             state_->cancel_ != cancel_search::continue_search;
    }

  private:
    /// Initial positioning: find first edge without incrementing count_
    void skip_to_first_edge() {
      while (vertex_index_ < state_->post_order_.size()) {
        if (edge_it_ != edge_end_) {
          return;
        }
        ++vertex_index_;
        if (vertex_index_ < state_->post_order_.size()) {
          auto v          = state_->post_order_[vertex_index_];
          auto edge_range = adj_list::edges(*g_, v);
          edge_it_        = std::ranges::begin(edge_range);
          edge_end_       = std::ranges::end(edge_range);
        }
      }
    }

    /// Advance to next edge, incrementing count_ for each exhausted source vertex
    void advance_to_next_edge() {
      // Find next edge, advancing to next vertex if needed
      while (vertex_index_ < state_->post_order_.size()) {
        if (edge_it_ != edge_end_) {
          return; // Found an edge
        }

        // Done with previous vertex's edges — count it
        ++state_->count_;
        ++vertex_index_;
        if (vertex_index_ < state_->post_order_.size()) {
          auto v          = state_->post_order_[vertex_index_];
          auto edge_range = adj_list::edges(*g_, v);
          edge_it_        = std::ranges::begin(edge_range);
          edge_end_       = std::ranges::end(edge_range);
        }
      }
    }

    G*                                  g_ = nullptr;
    std::shared_ptr<state_type>         state_;
    std::size_t                         vertex_index_ = 0;
    adj_list::vertex_edge_iterator_t<G> edge_it_{};
    adj_list::vertex_edge_iterator_t<G> edge_end_{};
  };

  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr edges_topological_sort_view() noexcept = default;

  edges_topological_sort_view(G& g, Alloc alloc = {}) : g_(&g), state_(std::make_shared<state_type>(g, alloc)) {}

  /// Construct with pre-built state (used by _safe functions)
  edges_topological_sort_view(G& g, std::shared_ptr<state_type> state) : g_(&g), state_(std::move(state)) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, 0); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  /// Get count of source vertices whose edges have been fully yielded.
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

  /// Get current cancel state
  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  /// Set cancel state to stop iteration early.
  /// cancel_branch is treated as cancel_all (no branch semantics in flat ordering).
  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

private:
  G*                          g_ = nullptr;
  std::shared_ptr<state_type> state_;
};

/**
 * @brief Topological sort edge view with value function
 * 
 * Iterates over all edges in topological order (by source vertex), yielding
 * edge_info<void, false, edge_t<G>, EV> where EV is the invoke result of EVF.
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam EVF Edge value function type
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
class edges_topological_sort_view : public std::ranges::view_interface<edges_topological_sort_view<G, EVF, Alloc>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using edge_type         = adj_list::edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_info<void, false, edge_type, value_result_type>;

private:
  using state_type = topo_detail::topo_state<G, Alloc>;

public:
  class iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state, std::size_t v_idx, EVF* evf)
          : g_(g), state_(std::move(state)), vertex_index_(v_idx), evf_(evf) {
      if (!at_end()) {
        auto v          = state_->post_order_[vertex_index_];
        auto edge_range = adj_list::edges(*g_, v);
        edge_it_        = std::ranges::begin(edge_range);
        edge_end_       = std::ranges::end(edge_range);
        skip_to_first_edge(); // Position without incrementing count_
      }
    }

    [[nodiscard]] value_type operator*() const {
      return value_type{*edge_it_, std::invoke(*evf_, std::as_const(*g_), *edge_it_)};
    }

    iterator& operator++() {
      ++edge_it_;
      advance_to_next_edge();
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator& other) const noexcept {
      if (at_end() && other.at_end())
        return true;
      if (at_end() || other.at_end())
        return false;
      return vertex_index_ == other.vertex_index_ && edge_it_ == other.edge_it_;
    }

    [[nodiscard]] bool at_end() const noexcept {
      return !state_ || vertex_index_ >= state_->post_order_.size() ||
             state_->cancel_ != cancel_search::continue_search;
    }

  private:
    /// Initial positioning: find first edge without incrementing count_
    void skip_to_first_edge() {
      while (vertex_index_ < state_->post_order_.size()) {
        if (edge_it_ != edge_end_) {
          return;
        }
        ++vertex_index_;
        if (vertex_index_ < state_->post_order_.size()) {
          auto v          = state_->post_order_[vertex_index_];
          auto edge_range = adj_list::edges(*g_, v);
          edge_it_        = std::ranges::begin(edge_range);
          edge_end_       = std::ranges::end(edge_range);
        }
      }
    }

    /// Advance to next edge, incrementing count_ for each exhausted source vertex
    void advance_to_next_edge() {
      while (vertex_index_ < state_->post_order_.size()) {
        if (edge_it_ != edge_end_) {
          return;
        }

        ++state_->count_;
        ++vertex_index_;
        if (vertex_index_ < state_->post_order_.size()) {
          auto v          = state_->post_order_[vertex_index_];
          auto edge_range = adj_list::edges(*g_, v);
          edge_it_        = std::ranges::begin(edge_range);
          edge_end_       = std::ranges::end(edge_range);
        }
      }
    }

    G*                                  g_ = nullptr;
    std::shared_ptr<state_type>         state_;
    std::size_t                         vertex_index_ = 0;
    adj_list::vertex_edge_iterator_t<G> edge_it_{};
    adj_list::vertex_edge_iterator_t<G> edge_end_{};
    EVF*                                evf_ = nullptr;
  };

  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr edges_topological_sort_view() noexcept = default;

  edges_topological_sort_view(G& g, EVF evf, Alloc alloc = {})
        : g_(&g), evf_(std::move(evf)), state_(std::make_shared<state_type>(g, alloc)) {}

  /// Construct with value function and pre-built state (used by _safe functions)
  edges_topological_sort_view(G& g, EVF evf, std::shared_ptr<state_type> state)
        : g_(&g), evf_(std::move(evf)), state_(std::move(state)) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, 0, &evf_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  /// Get count of source vertices whose edges have been fully yielded.
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

  /// Get current cancel state
  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  /// Set cancel state to stop iteration early.
  /// cancel_branch is treated as cancel_all (no branch semantics in flat ordering).
  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

private:
  G*                          g_ = nullptr;
  [[no_unique_address]] EVF   evf_{};
  std::shared_ptr<state_type> state_;
};

// Deduction guides for edges
template <class G, class Alloc>
edges_topological_sort_view(G&, Alloc) -> edges_topological_sort_view<G, void, Alloc>;

template <class G>
edges_topological_sort_view(G&) -> edges_topological_sort_view<G, void, std::allocator<bool>>;

template <class G, class EVF, class Alloc>
edges_topological_sort_view(G&, EVF, Alloc) -> edges_topological_sort_view<G, EVF, Alloc>;

//=============================================================================
// Factory functions
//=============================================================================

/**
 * @brief Create a topological sort vertex view without value function
 * 
 * @param g The graph to traverse
 * @return vertices_topological_sort_view yielding vertices in topological order
 * 
 * @complexity Time: O(V + E) - DFS traversal of entire graph
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_topological_sort(G& g) {
  return vertices_topological_sort_view<G, void, std::allocator<bool>>(g, std::allocator<bool>{});
}

/**
 * @brief Create a topological sort vertex view with value function
 * 
 * @param g The graph to traverse
 * @param vvf Value function invoked for each vertex
 * @return vertices_topological_sort_view with values
 * 
 * @complexity Time: O(V + E) - DFS traversal plus value function invocations
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort(G& g, VVF&& vvf) {
  return vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, std::forward<VVF>(vvf),
                                                                                    std::allocator<bool>{});
}

/**
 * @brief Create a topological sort vertex view with custom allocator
 * 
 * @param g The graph to traverse
 * @param alloc Allocator for internal containers
 * @return vertices_topological_sort_view with custom allocator
 * 
 * @complexity Time: O(V + E) - DFS traversal of entire graph
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_topological_sort(G& g, Alloc alloc) {
  return vertices_topological_sort_view<G, void, Alloc>(g, alloc);
}

/**
 * @brief Create a topological sort vertex view with value function and custom allocator
 * 
 * @param g The graph to traverse
 * @param vvf Value function invoked for each vertex
 * @param alloc Allocator for internal containers
 * @return vertices_topological_sort_view with value function and custom allocator
 * 
 * @complexity Time: O(V + E) - DFS traversal plus value function invocations
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort(G& g, VVF&& vvf, Alloc alloc) {
  return vertices_topological_sort_view<G, std::decay_t<VVF>, Alloc>(g, std::forward<VVF>(vvf), alloc);
}

/**
 * @brief Create a topological sort edge view without value function
 * 
 * @param g The graph to traverse
 * @return edges_topological_sort_view yielding edges in topological order (by source vertex)
 * 
 * @complexity Time: O(V + E) - DFS traversal plus edge iteration
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_topological_sort(G& g) {
  return edges_topological_sort_view<G, void, std::allocator<bool>>(g, std::allocator<bool>{});
}

/**
 * @brief Create a topological sort edge view with value function
 * 
 * @param g The graph to traverse
 * @param evf Value function invoked for each edge
 * @return edges_topological_sort_view with values
 * 
 * @complexity Time: O(V + E) - DFS traversal plus value function invocations
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_topological_sort(G& g, EVF&& evf) {
  return edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, std::forward<EVF>(evf),
                                                                                 std::allocator<bool>{});
}

/**
 * @brief Create a topological sort edge view with custom allocator
 * 
 * @param g The graph to traverse
 * @param alloc Allocator for internal containers
 * @return edges_topological_sort_view with custom allocator
 * 
 * @complexity Time: O(V + E) - DFS traversal plus edge iteration
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_topological_sort(G& g, Alloc alloc) {
  return edges_topological_sort_view<G, void, Alloc>(g, alloc);
}

/**
 * @brief Create a topological sort edge view with value function and custom allocator
 * 
 * @param g The graph to traverse
 * @param evf Value function invoked for each edge
 * @param alloc Allocator for internal containers
 * @return edges_topological_sort_view with value function and custom allocator
 * 
 * @complexity Time: O(V + E) - DFS traversal plus value function invocations
 * @complexity Space: O(V) - stores all vertices in result vector
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_topological_sort(G& g, EVF&& evf, Alloc alloc) {
  return edges_topological_sort_view<G, std::decay_t<EVF>, Alloc>(g, std::forward<EVF>(evf), alloc);
}

//=============================================================================
// Safe factory functions with cycle detection
//=============================================================================

/**
 * @brief Create a topological sort vertex view with cycle detection
 * 
 * Returns a view on success, or the vertex that closes a cycle if one is detected.
 * Uses DFS with recursion stack tracking to detect back edges (cycles).
 * 
 * @param g The graph to traverse
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 * 
 * @par Example:
 * @code
 * auto result = vertices_topological_sort_safe(g);
 * if (result) {
 *     for (auto [v] : *result) {
 *         process_vertex(v);
 *     }
 * } else {
 *     auto cycle_v = result.error();
 *     std::cerr << "Cycle detected at vertex " << vertex_id(g, cycle_v) << "\n";
 * }
 * @endcode
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_topological_sort_safe(G& g)
      -> tl::expected<vertices_topological_sort_view<G, void, std::allocator<bool>>, adj_list::vertex_t<G>> {
  using view_type = vertices_topological_sort_view<G, void, std::allocator<bool>>;

  auto state = std::make_shared<topo_detail::topo_state<G, std::allocator<bool>>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::move(state));
}

/**
 * @brief Create a topological sort vertex view with value function and cycle detection
 * 
 * @param g The graph to traverse
 * @param vvf Value function invoked for each vertex
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection plus value function invocations
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort_safe(G& g, VVF&& vvf)
      -> tl::expected<vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>>,
                      adj_list::vertex_t<G>> {
  using view_type = vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>>;

  auto state = std::make_shared<topo_detail::topo_state<G, std::allocator<bool>>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::forward<VVF>(vvf), std::move(state));
}

/**
 * @brief Create a topological sort vertex view with custom allocator and cycle detection
 * 
 * @param g The graph to traverse
 * @param alloc Allocator for internal containers
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_topological_sort_safe(G& g, Alloc alloc)
      -> tl::expected<vertices_topological_sort_view<G, void, Alloc>, adj_list::vertex_t<G>> {
  using view_type = vertices_topological_sort_view<G, void, Alloc>;

  auto state = std::make_shared<topo_detail::topo_state<G, Alloc>>(g, alloc, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::move(state));
}

/**
 * @brief Create a topological sort vertex view with value function, allocator, and cycle detection
 * 
 * @param g The graph to traverse
 * @param vvf Value function invoked for each vertex
 * @param alloc Allocator for internal containers
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection plus value function invocations
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort_safe(G& g, VVF&& vvf, Alloc alloc)
      -> tl::expected<vertices_topological_sort_view<G, std::decay_t<VVF>, Alloc>, adj_list::vertex_t<G>> {
  using view_type = vertices_topological_sort_view<G, std::decay_t<VVF>, Alloc>;

  auto state = std::make_shared<topo_detail::topo_state<G, Alloc>>(g, alloc, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::forward<VVF>(vvf), std::move(state));
}

/**
 * @brief Create a topological sort edge view with cycle detection
 * 
 * Returns a view on success, or the vertex that closes a cycle if one is detected.
 * 
 * @param g The graph to traverse
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection plus edge iteration
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_topological_sort_safe(G& g)
      -> tl::expected<edges_topological_sort_view<G, void, std::allocator<bool>>, adj_list::vertex_t<G>> {
  using view_type = edges_topological_sort_view<G, void, std::allocator<bool>>;

  auto state = std::make_shared<topo_detail::topo_state<G, std::allocator<bool>>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::move(state));
}

/**
 * @brief Create a topological sort edge view with value function and cycle detection
 * 
 * @param g The graph to traverse
 * @param evf Value function invoked for each edge
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection plus value function invocations
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_topological_sort_safe(G& g, EVF&& evf)
      -> tl::expected<edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>>, adj_list::vertex_t<G>> {
  using view_type = edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>>;

  auto state = std::make_shared<topo_detail::topo_state<G, std::allocator<bool>>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::forward<EVF>(evf), std::move(state));
}

/**
 * @brief Create a topological sort edge view with custom allocator and cycle detection
 * 
 * @param g The graph to traverse
 * @param alloc Allocator for internal containers
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection plus edge iteration
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_topological_sort_safe(G& g, Alloc alloc)
      -> tl::expected<edges_topological_sort_view<G, void, Alloc>, adj_list::vertex_t<G>> {
  using view_type = edges_topological_sort_view<G, void, Alloc>;

  auto state = std::make_shared<topo_detail::topo_state<G, Alloc>>(g, alloc, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::move(state));
}

/**
 * @brief Create a topological sort edge view with value function, allocator, and cycle detection
 * 
 * @param g The graph to traverse
 * @param evf Value function invoked for each edge
 * @param alloc Allocator for internal containers
 * @return expected containing view on success, or cycle vertex on failure
 * 
 * @complexity Time: O(V + E) - DFS traversal with cycle detection plus value function invocations
 *             Space: O(2V) - post-order vector + visited tracker + recursion stack
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_topological_sort_safe(G& g, EVF&& evf, Alloc alloc)
      -> tl::expected<edges_topological_sort_view<G, std::decay_t<EVF>, Alloc>, adj_list::vertex_t<G>> {
  using view_type = edges_topological_sort_view<G, std::decay_t<EVF>, Alloc>;

  auto state = std::make_shared<topo_detail::topo_state<G, Alloc>>(g, alloc, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::forward<EVF>(evf), std::move(state));
}

} // namespace graph::views
