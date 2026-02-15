/**
 * @file bfs.hpp
 * @brief Breadth-first search views for vertices and edges
 * 
 * Provides views that traverse a graph in breadth-first order from a seed vertex,
 * yielding vertex_info or edge_info for each visited element.
 * 
 * @complexity Time: O(V + E) where V is reachable vertices and E is reachable edges
 *             BFS visits each reachable vertex once and traverses each reachable edge once
 * @complexity Space: O(V) for the queue and visited tracker
 * 
 * @par Examples:
 * @code
 * // Vertex traversal
 * for (auto [v] : vertices_bfs(g, seed))
 *     process_vertex(v);
 * 
 * // Vertex traversal with value function
 * for (auto [v, val] : vertices_bfs(g, seed, value_fn))
 *     process_vertex_with_value(v, val);
 * 
 * // Access depth during traversal
 * auto bfs = vertices_bfs(g, seed);
 * for (auto [v] : bfs)
 *     std::cout << "Vertex " << vertex_id(g, v) << " at depth " << bfs.depth() << "\n";
 * 
 * // Cancel search
 * auto bfs = vertices_bfs(g, seed);
 * for (auto [v] : bfs) {
 *     if (should_stop(v))
 *         bfs.cancel(cancel_search::cancel_all);
 * }
 * @endcode
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include <graph/graph_info.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/search_base.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::index_adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>>
class vertices_bfs_view;

template <adj_list::index_adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>>
class edges_bfs_view;

namespace bfs_detail {

  /// Queue entry for BFS traversal: stores vertex descriptor and its depth
  template <class Vertex>
  struct queue_entry {
    Vertex      vertex;
    std::size_t depth;
  };

  /// Queue entry for BFS edge traversal: stores vertex, depth, and edge iteration state
  template <class Vertex, class EdgeIter>
  struct edge_queue_entry {
    Vertex      vertex;
    std::size_t depth;
    EdgeIter    edge_end;
    EdgeIter    edge_current;
  };

  /**
 * @brief Shared BFS state for vertex traversal
 * 
 * @complexity Time: O(V + E) - visits each reachable vertex once, traverses each reachable edge once
 * @complexity Space: O(V) - queue stores up to V vertices, visited tracker uses O(V) space
 */
  template <class G, class Alloc>
  struct bfs_state {
    using graph_type     = G;
    using vertex_type    = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using allocator_type = Alloc;
    using entry_type     = queue_entry<vertex_type>;
    using queue_alloc    = typename std::allocator_traits<Alloc>::template rebind_alloc<entry_type>;

    std::queue<entry_type, std::deque<entry_type, queue_alloc>> queue_;
    visited_tracker<vertex_id_type, Alloc>                      visited_;
    cancel_search                                               cancel_    = cancel_search::continue_search;
    std::size_t                                                 max_depth_ = 0;
    std::size_t                                                 count_     = 0;

    bfs_state(G& g, vertex_type seed_vertex, std::size_t num_vertices, Alloc alloc = {})
          : queue_(std::deque<entry_type, queue_alloc>(alloc)), visited_(num_vertices, alloc) {
      queue_.push({seed_vertex, 0});
      visited_.mark_visited(adj_list::vertex_id(g, seed_vertex));
    }
  };

  /**
 * @brief Shared BFS state for edge traversal
 * 
 * @complexity Time: O(V + E) - visits each reachable vertex once, traverses each reachable edge once
 * @complexity Space: O(V) - queue and visited tracker use O(V) space
 */
  template <class G, class Alloc>
  struct bfs_edge_state {
    using graph_type     = G;
    using vertex_type    = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_iter_type = adj_list::vertex_edge_iterator_t<G>;
    using allocator_type = Alloc;
    using entry_type     = edge_queue_entry<vertex_type, edge_iter_type>;
    using queue_alloc    = typename std::allocator_traits<Alloc>::template rebind_alloc<entry_type>;

    std::queue<entry_type, std::deque<entry_type, queue_alloc>> queue_;
    visited_tracker<vertex_id_type, Alloc>                      visited_;
    cancel_search                                               cancel_    = cancel_search::continue_search;
    std::size_t                                                 max_depth_ = 0;
    std::size_t                                                 count_     = 0;
    std::optional<vertex_id_type> skip_vertex_id_; // Vertex to skip when processing (for cancel_branch)

    bfs_edge_state(G& g, vertex_type seed_vertex, std::size_t num_vertices, Alloc alloc = {})
          : queue_(std::deque<entry_type, queue_alloc>(alloc)), visited_(num_vertices, alloc) {
      auto edge_range = adj_list::edges(g, seed_vertex);
      auto edge_begin = std::ranges::begin(edge_range);
      auto edge_end   = std::ranges::end(edge_range);
      queue_.push({seed_vertex, 0, edge_end, edge_begin});
      visited_.mark_visited(adj_list::vertex_id(g, seed_vertex));
    }
  };

} // namespace bfs_detail

/**
 * @brief BFS vertex view without value function
 * 
 * Iterates over vertices in breadth-first order yielding 
 * vertex_info<void, vertex_t<G>, void>
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class Alloc>
class vertices_bfs_view<G, void, Alloc> : public std::ranges::view_interface<vertices_bfs_view<G, void, Alloc>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using edge_type      = adj_list::edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = vertex_info<void, vertex_type, void>;

private:
  using state_type = bfs_detail::bfs_state<G, Alloc>;

public:
  /**
     * @brief Input iterator for BFS vertex traversal
     */
  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state) : g_(g), state_(std::move(state)) {}

    [[nodiscard]] value_type operator*() const { return value_type{state_->queue_.front().vertex}; }

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
      bool this_at_end  = !state_ || state_->queue_.empty();
      bool other_at_end = !other.state_ || other.state_->queue_.empty();
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || state_->queue_.empty(); }

  private:
    void advance() {
      if (!state_ || state_->queue_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->queue_.empty())
          state_->queue_.pop();
        return;
      }

      // Get current vertex and its depth
      auto current = state_->queue_.front();
      state_->queue_.pop();

      // Handle cancel_branch: skip exploring children of current vertex
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->cancel_ = cancel_search::continue_search;
        return; // Don't enqueue children, just continue to next in queue
      }

      // Explore neighbors (enqueue unvisited ones)
      auto edge_range = adj_list::edges(*g_, current.vertex);
      for (auto edge : edge_range) {
        auto target_v   = adj_list::target(*g_, edge);
        auto target_vid = adj_list::vertex_id(*g_, target_v);

        if (!state_->visited_.is_visited(target_vid)) {
          state_->visited_.mark_visited(target_vid);
          state_->queue_.push({target_v, current.depth + 1});
          if (current.depth + 1 > state_->max_depth_) {
            state_->max_depth_ = current.depth + 1;
          }
          ++state_->count_;
        }
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
  };

  /// Sentinel for end of BFS traversal
  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr vertices_bfs_view() noexcept = default;

  /// Construct from vertex descriptor
  vertices_bfs_view(G& g, vertex_type seed_vertex, Alloc alloc = {})
        : g_(&g), state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  vertices_bfs_view(G& g, vertex_id_type seed, Alloc alloc = {})
        : vertices_bfs_view(g, *adj_list::find_vertex(g, seed), alloc) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  /// Get current cancel state
  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  /// Set cancel state to stop traversal
  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

  /// Get maximum depth reached so far
  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->max_depth_ : 0; }

  /// Get count of vertices visited so far
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  std::shared_ptr<state_type> state_;
};

/**
 * @brief BFS vertex view with value function
 * 
 * Iterates over vertices in breadth-first order yielding 
 * vertex_info<void, vertex_t<G>, VV> where VV is the invoke result of VVF.
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam VVF Vertex value function type
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
class vertices_bfs_view : public std::ranges::view_interface<vertices_bfs_view<G, VVF, Alloc>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_type         = adj_list::edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_info<void, vertex_type, value_result_type>;

private:
  using state_type = bfs_detail::bfs_state<G, Alloc>;

public:
  /**
     * @brief Input iterator for BFS vertex traversal with value function
     */
  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state, VVF* vvf) : g_(g), state_(std::move(state)), vvf_(vvf) {}

    [[nodiscard]] value_type operator*() const {
      auto v = state_->queue_.front().vertex;
      return value_type{v, std::invoke(*vvf_, std::as_const(*g_), v)};
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
      bool this_at_end  = !state_ || state_->queue_.empty();
      bool other_at_end = !other.state_ || other.state_->queue_.empty();
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || state_->queue_.empty(); }

  private:
    void advance() {
      if (!state_ || state_->queue_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->queue_.empty())
          state_->queue_.pop();
        return;
      }

      auto current = state_->queue_.front();
      state_->queue_.pop();

      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->cancel_ = cancel_search::continue_search;
        return;
      }

      auto edge_range = adj_list::edges(*g_, current.vertex);
      for (auto edge : edge_range) {
        auto target_v   = adj_list::target(*g_, edge);
        auto target_vid = adj_list::vertex_id(*g_, target_v);

        if (!state_->visited_.is_visited(target_vid)) {
          state_->visited_.mark_visited(target_vid);
          state_->queue_.push({target_v, current.depth + 1});
          if (current.depth + 1 > state_->max_depth_) {
            state_->max_depth_ = current.depth + 1;
          }
          ++state_->count_;
        }
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    VVF*                        vvf_ = nullptr;
  };

  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr vertices_bfs_view() noexcept = default;

  /// Construct from vertex descriptor
  vertices_bfs_view(G& g, vertex_type seed_vertex, VVF vvf, Alloc alloc = {})
        : g_(&g)
        , vvf_(std::move(vvf))
        , state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  vertices_bfs_view(G& g, vertex_id_type seed, VVF vvf, Alloc alloc = {})
        : vertices_bfs_view(g, *adj_list::find_vertex(g, seed), std::move(vvf), alloc) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, &vvf_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->max_depth_ : 0; }

  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  [[no_unique_address]] VVF   vvf_{};
  std::shared_ptr<state_type> state_;
};

// Deduction guides for vertex_id
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, adj_list::vertex_id_t<G>, Alloc) -> vertices_bfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
vertices_bfs_view(G&, adj_list::vertex_id_t<G>) -> vertices_bfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class VVF, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, adj_list::vertex_id_t<G>, VVF, Alloc) -> vertices_bfs_view<G, VVF, Alloc>;

// Deduction guides for vertex descriptor
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, adj_list::vertex_t<G>, Alloc) -> vertices_bfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
vertices_bfs_view(G&, adj_list::vertex_t<G>) -> vertices_bfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class VVF, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, adj_list::vertex_t<G>, VVF, Alloc) -> vertices_bfs_view<G, VVF, Alloc>;

/**
 * @brief Create a BFS vertex view without value function (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_id_t<G> seed) {
  return vertices_bfs_view<G, void, std::allocator<bool>>(g, seed, std::allocator<bool>{});
}

/**
 * @brief Create a BFS vertex view without value function (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return vertices_bfs_view<G, void, std::allocator<bool>>(g, seed_vertex, std::allocator<bool>{});
}

/**
 * @brief Create a BFS vertex view with value function (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @param vvf Value function invoked for each vertex
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf) {
  return vertices_bfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed, std::forward<VVF>(vvf),
                                                                       std::allocator<bool>{});
}

/**
 * @brief Create a BFS vertex view with value function (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @param vvf Value function invoked for each vertex
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf) {
  return vertices_bfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed_vertex, std::forward<VVF>(vvf),
                                                                       std::allocator<bool>{});
}

/**
 * @brief Create a BFS vertex view with custom allocator (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @param alloc Allocator for internal containers
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_id_t<G> seed, Alloc alloc) {
  return vertices_bfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief Create a BFS vertex view with custom allocator (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @param alloc Allocator for internal containers
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, void>
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
  return vertices_bfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief Create a BFS vertex view with value function and custom allocator (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @param vvf Value function invoked for each vertex
 * @param alloc Allocator for internal containers
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf, Alloc alloc) {
  return vertices_bfs_view<G, std::decay_t<VVF>, Alloc>(g, seed, std::forward<VVF>(vvf), alloc);
}

/**
 * @brief Create a BFS vertex view with value function and custom allocator (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @param vvf Value function invoked for each vertex
 * @param alloc Allocator for internal containers
 * @return vertices_bfs_view yielding vertex_info<void, vertex_descriptor, VV>
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf, Alloc alloc) {
  return vertices_bfs_view<G, std::decay_t<VVF>, Alloc>(g, seed_vertex, std::forward<VVF>(vvf), alloc);
}

//=============================================================================
// edges_bfs - BFS edge traversal
//=============================================================================

/**
 * @brief BFS edge view without value function
 * 
 * Iterates over edges in breadth-first order yielding 
 * edge_info<void, false, edge_t<G>, void>
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class Alloc>
class edges_bfs_view<G, void, Alloc> : public std::ranges::view_interface<edges_bfs_view<G, void, Alloc>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using edge_type      = adj_list::edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = edge_info<void, false, edge_type, void>;

private:
  using state_type = bfs_detail::bfs_edge_state<G, Alloc>;

public:
  /**
     * @brief Input iterator for BFS edge traversal
     */
  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state) : g_(g), state_(std::move(state)) {
      // Advance to first edge (seed vertex has no incoming edge to yield)
      advance_to_next_edge();
    }

    [[nodiscard]] value_type operator*() const { return value_type{current_edge_}; }

    iterator& operator++() {
      advance_to_next_edge();
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      advance_to_next_edge();
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator& other) const noexcept {
      bool this_at_end  = !state_ || (state_->queue_.empty() && !has_edge_);
      bool other_at_end = !other.state_ || (other.state_->queue_.empty() && !other.has_edge_);
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || (state_->queue_.empty() && !has_edge_); }

  private:
    void advance_to_next_edge() {
      has_edge_ = false;
      if (!state_ || state_->queue_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->queue_.empty())
          state_->queue_.pop();
        return;
      }

      // Handle cancel_branch: mark the target of last edge to be skipped
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->skip_vertex_id_ = current_target_id_;
        state_->cancel_         = cancel_search::continue_search;
      }

      // Find next tree edge using BFS
      while (!state_->queue_.empty()) {
        auto& current = state_->queue_.front();

        // Check if we should skip this vertex (due to cancel_branch)
        auto current_vid = adj_list::vertex_id(*g_, current.vertex);
        if (state_->skip_vertex_id_ && *state_->skip_vertex_id_ == current_vid) {
          state_->skip_vertex_id_.reset();
          state_->queue_.pop();
          continue;
        }

        // Process edges from current vertex
        while (current.edge_current != current.edge_end) {
          auto edge = *current.edge_current;
          ++current.edge_current;

          auto target_v   = adj_list::target(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Add target vertex to queue with its edge range
            auto target_edge_range = adj_list::edges(*g_, target_v);
            auto target_begin      = std::ranges::begin(target_edge_range);
            auto target_end        = std::ranges::end(target_edge_range);
            state_->queue_.push({target_v, current.depth + 1, target_end, target_begin});

            if (current.depth + 1 > state_->max_depth_) {
              state_->max_depth_ = current.depth + 1;
            }
            ++state_->count_;

            // Store and return this edge
            current_edge_      = edge;
            current_target_id_ = target_vid;
            has_edge_          = true;
            return;
          }
        }

        // No more edges from current vertex, remove it
        state_->queue_.pop();
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    edge_type                   current_edge_{};
    vertex_id_type              current_target_id_{}; // Target vertex ID of current edge
    bool                        has_edge_ = false;
  };

  /// Sentinel for end of BFS traversal
  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr edges_bfs_view() noexcept = default;

  /// Construct from vertex descriptor
  edges_bfs_view(G& g, vertex_type seed_vertex, Alloc alloc = {})
        : g_(&g), state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  edges_bfs_view(G& g, vertex_id_type seed, Alloc alloc = {})
        : edges_bfs_view(g, *adj_list::find_vertex(g, seed), alloc) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  /// Get current cancel state
  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  /// Set cancel state to stop traversal
  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

  /// Get maximum depth reached so far
  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->max_depth_ : 0; }

  /// Get count of edges visited so far
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  std::shared_ptr<state_type> state_;
};

/**
 * @brief BFS edge view with value function
 * 
 * Iterates over edges in breadth-first order yielding 
 * edge_info<void, false, edge_t<G>, EV> where EV is the invoke result of EVF.
 * 
 * @tparam G Graph type satisfying index_adjacency_list concept
 * @tparam EVF Edge value function type
 * @tparam Alloc Allocator type for internal containers
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
class edges_bfs_view : public std::ranges::view_interface<edges_bfs_view<G, EVF, Alloc>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_type         = adj_list::edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_info<void, false, edge_type, value_result_type>;

private:
  using state_type = bfs_detail::bfs_edge_state<G, Alloc>;

public:
  /**
     * @brief Input iterator for BFS edge traversal with value function
     */
  class iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = info_type;
    using pointer           = const value_type*;
    using reference         = value_type;

    constexpr iterator() noexcept = default;

    iterator(G* g, std::shared_ptr<state_type> state, EVF* evf) : g_(g), state_(std::move(state)), evf_(evf) {
      // Advance to first edge
      advance_to_next_edge();
    }

    [[nodiscard]] value_type operator*() const {
      return value_type{current_edge_, std::invoke(*evf_, std::as_const(*g_), current_edge_)};
    }

    iterator& operator++() {
      advance_to_next_edge();
      return *this;
    }

    iterator operator++(int) {
      auto tmp = *this;
      advance_to_next_edge();
      return tmp;
    }

    [[nodiscard]] bool operator==(const iterator& other) const noexcept {
      bool this_at_end  = !state_ || (state_->queue_.empty() && !has_edge_);
      bool other_at_end = !other.state_ || (other.state_->queue_.empty() && !other.has_edge_);
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || (state_->queue_.empty() && !has_edge_); }

  private:
    void advance_to_next_edge() {
      has_edge_ = false;
      if (!state_ || state_->queue_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->queue_.empty())
          state_->queue_.pop();
        return;
      }

      // Handle cancel_branch: mark the target of last edge to be skipped
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->skip_vertex_id_ = current_target_id_;
        state_->cancel_         = cancel_search::continue_search;
      }

      // Find next tree edge using BFS
      while (!state_->queue_.empty()) {
        auto& current = state_->queue_.front();

        // Check if we should skip this vertex (due to cancel_branch)
        auto current_vid = adj_list::vertex_id(*g_, current.vertex);
        if (state_->skip_vertex_id_ && *state_->skip_vertex_id_ == current_vid) {
          state_->skip_vertex_id_.reset();
          state_->queue_.pop();
          continue;
        }

        // Process edges from current vertex
        while (current.edge_current != current.edge_end) {
          auto edge = *current.edge_current;
          ++current.edge_current;

          auto target_v   = adj_list::target(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Add target vertex to queue with its edge range
            auto target_edge_range = adj_list::edges(*g_, target_v);
            auto target_begin      = std::ranges::begin(target_edge_range);
            auto target_end        = std::ranges::end(target_edge_range);
            state_->queue_.push({target_v, current.depth + 1, target_end, target_begin});

            if (current.depth + 1 > state_->max_depth_) {
              state_->max_depth_ = current.depth + 1;
            }
            ++state_->count_;

            // Store and return this edge
            current_edge_      = edge;
            current_target_id_ = target_vid;
            has_edge_          = true;
            return;
          }
        }

        // No more edges from current vertex, remove it
        state_->queue_.pop();
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    EVF*                        evf_ = nullptr;
    edge_type                   current_edge_{};
    vertex_id_type              current_target_id_{}; // Target vertex ID of current edge
    bool                        has_edge_ = false;
  };

  /// Sentinel for end of BFS traversal
  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr edges_bfs_view() noexcept = default;

  /// Construct from vertex descriptor
  edges_bfs_view(G& g, vertex_type seed_vertex, EVF evf, Alloc alloc = {})
        : g_(&g)
        , evf_(std::move(evf))
        , state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  edges_bfs_view(G& g, vertex_id_type seed, EVF evf, Alloc alloc = {})
        : edges_bfs_view(g, *adj_list::find_vertex(g, seed), std::move(evf), alloc) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, &evf_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->max_depth_ : 0; }

  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  [[no_unique_address]] EVF   evf_;
  std::shared_ptr<state_type> state_;
};

// Deduction guides for edges_bfs_view
template <class G, class Alloc>
edges_bfs_view(G&, adj_list::vertex_id_t<G>, Alloc) -> edges_bfs_view<G, void, Alloc>;

template <class G, class Alloc>
edges_bfs_view(G&, adj_list::vertex_t<G>, Alloc) -> edges_bfs_view<G, void, Alloc>;

template <class G, class EVF, class Alloc>
edges_bfs_view(G&, adj_list::vertex_id_t<G>, EVF, Alloc) -> edges_bfs_view<G, EVF, Alloc>;

template <class G, class EVF, class Alloc>
edges_bfs_view(G&, adj_list::vertex_t<G>, EVF, Alloc) -> edges_bfs_view<G, EVF, Alloc>;

//=============================================================================
// Factory functions for edges_bfs
//=============================================================================

/**
 * @brief Create a BFS edge view (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_id_t<G> seed) {
  return edges_bfs_view<G, void, std::allocator<bool>>(g, seed);
}

/**
 * @brief Create a BFS edge view (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, void>
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return edges_bfs_view<G, void, std::allocator<bool>>(g, seed_vertex);
}

/**
 * @brief Create a BFS edge view with value function (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @param evf Value function invoked for each edge
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, EV>
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>> && (!std::is_same_v<std::decay_t<EVF>, std::allocator<bool>>)
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_id_t<G> seed, EVF&& evf) {
  return edges_bfs_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, seed, std::forward<EVF>(evf));
}

/**
 * @brief Create a BFS edge view with value function (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @param evf Value function invoked for each edge
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, EV>
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>> && (!std::is_same_v<std::decay_t<EVF>, std::allocator<bool>>)
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf) {
  return edges_bfs_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, seed_vertex, std::forward<EVF>(evf));
}

/**
 * @brief Create a BFS edge view with custom allocator (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @param alloc Allocator for internal containers
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, void>
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_id_t<G> seed, Alloc alloc) {
  return edges_bfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief Create a BFS edge view with custom allocator (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @param alloc Allocator for internal containers
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, void>
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
  return edges_bfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief Create a BFS edge view with value function and custom allocator (from vertex ID)
 * 
 * @param g The graph to traverse
 * @param seed Starting vertex ID for BFS
 * @param evf Value function invoked for each edge
 * @param alloc Allocator for internal containers
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, EV>
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_id_t<G> seed, EVF&& evf, Alloc alloc) {
  return edges_bfs_view<G, std::decay_t<EVF>, Alloc>(g, seed, std::forward<EVF>(evf), alloc);
}

/**
 * @brief Create a BFS edge view with value function and custom allocator (from vertex descriptor)
 * 
 * @param g The graph to traverse
 * @param seed_vertex Starting vertex descriptor for BFS
 * @param evf Value function invoked for each edge
 * @param alloc Allocator for internal containers
 * @return edges_bfs_view yielding edge_info<void, false, edge_descriptor, EV>
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf, Alloc alloc) {
  return edges_bfs_view<G, std::decay_t<EVF>, Alloc>(g, seed_vertex, std::forward<EVF>(evf), alloc);
}

} // namespace graph::views
