/**
 * @file bfs.hpp
 * @brief Breadth-first search views for vertices and edges.
 *
 * @section overview Overview
 *
 * Provides lazy, single-pass views that traverse a graph in breadth-first
 * order starting from a seed vertex.  @c vertices_bfs yields per-vertex
 * @c vertex_data and @c edges_bfs yields per-edge @c edge_data.  An optional
 * value function computes a per-element value included in the structured
 * binding.
 *
 * Both view families expose additional accessors on the view object:
 * - @c depth()       — maximum BFS depth reached so far
 * - @c num_visited() — total vertices/edges visited so far
 * - @c cancel(c)     — stop traversal (@c cancel_branch or @c cancel_all)
 *
 * Only vertices/edges reachable from the seed are visited.
 *
 * @section variants View Variants
 *
 * | Variant                             | Structured Binding | Description                        |
 * |-------------------------------------|--------------------|------------------------------------|
 * | @c vertices_bfs(g,seed)             | `[v]`              | Vertex BFS (descriptor only)       |
 * | @c vertices_bfs(g,seed,vvf)         | `[v, val]`         | Vertex BFS with value function     |
 * | @c edges_bfs(g,seed)                | `[uv]`             | Edge BFS (edge descriptor only)    |
 * | @c edges_bfs(g,seed,evf)            | `[uv, val]`        | Edge BFS with value function       |
 *
 * Each factory also accepts a vertex descriptor in place of a vertex id,
 * and an optional custom allocator for the internal queue and visited
 * tracker.
 *
 * @section bindings Structured Bindings
 *
 * Vertex BFS:
 * @code
 *   for (auto [v]      : vertices_bfs(g, seed))          // v = vertex_t<G>
 *   for (auto [v, val] : vertices_bfs(g, seed, vvf))     // val = invoke_result_t<VVF, G&, vertex_t<G>>
 * @endcode
 *
 * Edge BFS:
 * @code
 *   for (auto [uv]      : edges_bfs(g, seed))            // uv = edge_t<G>
 *   for (auto [uv, val] : edges_bfs(g, seed, evf))       // val = invoke_result_t<EVF, G&, edge_t<G>>
 * @endcode
 *
 * @section iterator_properties Iterator Properties
 *
 * | Property        | Value                                             |
 * |-----------------|---------------------------------------------------|
 * | Concept         | @c std::input_iterator                            |
 * | Range concept   | @c std::ranges::input_range                       |
 * | Sized           | No                                                |
 * | Borrowed        | No (view owns internal state)                     |
 * | Common          | No (uses sentinel)                                |
 *
 * @section perf Performance Characteristics
 *
 * Construction allocates an O(V) visited tracker and pushes the seed vertex
 * onto an internal queue.  Each @c operator++ is amortised O(1) — it
 * dequeues one entry and enqueues at most deg(v) new entries.  Full
 * traversal is O(V + E) time and O(V) space (queue width ≤ V, visited
 * tracker = V bits).
 *
 * @section chaining Chaining with std::views
 *
 * BFS views are @c input_range (single-pass), so they chain only with
 * adaptors that do not require forward ranges:
 *
 * @code
 *   auto view = vertices_bfs(g, seed)
 *               | std::views::take(5);  // ✅ compiles (take is input-compatible)
 * @endcode
 *
 * Adaptors that buffer or require multiple passes (e.g. @c std::views::reverse)
 * are not supported.
 *
 * @section search_control Search Control
 *
 * @code
 *   auto bfs = vertices_bfs(g, seed);
 *   for (auto [v] : bfs) {
 *       if (found(v))
 *           bfs.cancel(cancel_search::cancel_all);    // stop immediately
 *       if (prune(v))
 *           bfs.cancel(cancel_search::cancel_branch); // skip children
 *       std::cout << "depth=" << bfs.depth() << "\n";
 *   }
 * @endcode
 *
 * @section supported_graphs Supported Graph Properties
 *
 * - Requires: @c index_adjacency_list concept
 * - Works with all @c dynamic_graph container combinations
 * - Works with directed and undirected graphs
 *
 * @section exception_safety Exception Safety
 *
 * Construction may throw @c std::bad_alloc (allocates visited tracker +
 * queue).  Iteration may propagate exceptions from the value function; all
 * other iterator operations are @c noexcept.
 *
 * @section preconditions Preconditions
 *
 * - The graph @c g must outlive the view.
 * - The graph must not be mutated during iteration.
 * - The seed vertex must be a valid vertex in the graph.
 *
 * @section see_also See Also
 *
 * - @ref dfs.hpp — depth-first search views
 * - @ref views.md — all views overview
 * - @ref graph_cpo_implementation.md — CPO documentation
 * - @ref view_chaining_limitations.md — chaining design rationale
 */

#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>
#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include <graph/graph_data.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/search_base.hpp>
#include <graph/views/edge_accessor.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::index_adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>,
          class Accessor = out_edge_accessor>
class vertices_bfs_view;

template <adj_list::index_adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>,
          class Accessor = out_edge_accessor>
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
   * @brief Shared BFS traversal state for vertex iteration.
   *
   * Holds the FIFO queue, visited tracker, cancellation flag, max-depth
   * counter and visit counter.  Shared via @c std::shared_ptr so that
   * iterator copies and the owning view all observe the same state.
   *
   * @par Complexity
   * Time: O(V + E) — visits each reachable vertex once, traverses each
   * reachable edge once.  Space: O(V) — queue width ≤ V, visited
   * tracker = V bits.
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
   * @brief Shared BFS traversal state for edge iteration.
   *
   * Similar to @c bfs_state but additionally stores per-vertex edge
   * iterators so that tree edges can be yielded one at a time.
   *
   * @par Complexity
   * Time: O(V + E).  Space: O(V).
   */
  template <class G, class Alloc, class Accessor = out_edge_accessor>
  struct bfs_edge_state {
    using graph_type     = G;
    using vertex_type    = adj_list::vertex_t<G>;
    using vertex_id_type = adj_list::vertex_id_t<G>;
    using edge_iter_type = std::ranges::iterator_t<typename Accessor::template edge_range_t<G>>;
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
      auto edge_range = Accessor{}.edges(g, seed_vertex);
      auto edge_begin = std::ranges::begin(edge_range);
      auto edge_end   = std::ranges::end(edge_range);
      queue_.push({seed_vertex, 0, edge_end, edge_begin});
      visited_.mark_visited(adj_list::vertex_id(g, seed_vertex));
    }
  };

} // namespace bfs_detail

/**
 * @brief BFS vertex view without value function.
 *
 * Traverses vertices reachable from a seed in breadth-first order,
 * yielding @c vertex_data{v} per vertex via structured bindings.
 *
 * @code
 *   for (auto [v] : vertices_bfs(g, seed)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal queue and visited tracker
 *
 * @see vertices_bfs_view<G,VVF,Alloc> — with value function
 * @see edges_bfs_view                 — edge-oriented BFS
 */
template <adj_list::index_adjacency_list G, class Alloc, class Accessor>
class vertices_bfs_view<G, void, Alloc, Accessor> : public std::ranges::view_interface<vertices_bfs_view<G, void, Alloc, Accessor>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using edge_type      = typename Accessor::template edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = vertex_data<void, vertex_type, void>;

private:
  using state_type = bfs_detail::bfs_state<G, Alloc>;

public:
  /**
     * @brief Input iterator yielding @c vertex_data{v}.
     *
     * Single-pass: all copies share state via @c shared_ptr, so advancing
     * one iterator advances them all.
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
      auto edge_range = Accessor{}.edges(*g_, current.vertex);
      for (auto edge : edge_range) {
        auto target_v   = Accessor{}.neighbor(*g_, edge);
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
  vertices_bfs_view(G& g, const vertex_id_type& seed, Alloc alloc = {})
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
 * @brief BFS vertex view with a vertex value function.
 *
 * Traverses vertices reachable from a seed in breadth-first order,
 * yielding @c vertex_data{v, val} per vertex via structured bindings.
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
 *   for (auto [v, val] : vertices_bfs(g, seed, vvf)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam VVF   Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @tparam Alloc Allocator for the internal queue and visited tracker
 *
 * @see vertices_bfs_view<G,void,Alloc> — without value function
 * @see edges_bfs_view                  — edge-oriented BFS
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc, class Accessor>
class vertices_bfs_view : public std::ranges::view_interface<vertices_bfs_view<G, VVF, Alloc, Accessor>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_type         = typename Accessor::template edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_data<void, vertex_type, value_result_type>;

private:
  using state_type = bfs_detail::bfs_state<G, Alloc>;

public:
  /**
     * @brief Input iterator yielding @c vertex_data{v, val}.
     *
     * Single-pass: all copies share state via @c shared_ptr.
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

      auto edge_range = Accessor{}.edges(*g_, current.vertex);
      for (auto edge : edge_range) {
        auto target_v   = Accessor{}.neighbor(*g_, edge);
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
  vertices_bfs_view(G& g, const vertex_id_type& seed, VVF vvf, Alloc alloc = {})
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
vertices_bfs_view(G&, const adj_list::vertex_id_t<G>&, Alloc) -> vertices_bfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
vertices_bfs_view(G&, const adj_list::vertex_id_t<G>&) -> vertices_bfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class VVF, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, const adj_list::vertex_id_t<G>&, VVF, Alloc) -> vertices_bfs_view<G, VVF, Alloc>;

// Deduction guides for vertex descriptor
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, adj_list::vertex_t<G>, Alloc) -> vertices_bfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
vertices_bfs_view(G&, adj_list::vertex_t<G>) -> vertices_bfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class VVF, class Alloc = std::allocator<bool>>
vertices_bfs_view(G&, adj_list::vertex_t<G>, VVF, Alloc) -> vertices_bfs_view<G, VVF, Alloc>;

/**
 * @brief BFS vertex traversal from a vertex ID, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex ID.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v}.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_bfs(G& g, const adj_list::vertex_id_t<G>& seed) {
  return vertices_bfs_view<G, void, std::allocator<bool>>(g, seed, std::allocator<bool>{});
}

/**
 * @brief BFS vertex traversal from a vertex descriptor, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v}.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return vertices_bfs_view<G, void, std::allocator<bool>>(g, seed_vertex, std::allocator<bool>{});
}

/**
 * @brief BFS vertex traversal with value function, from a vertex ID.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam VVF  Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex ID.
 * @param  vvf   Value function invoked per vertex.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v, val}.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, const adj_list::vertex_id_t<G>& seed, VVF&& vvf) {
  return vertices_bfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed, std::forward<VVF>(vvf),
                                                                       std::allocator<bool>{});
}

/**
 * @brief BFS vertex traversal with value function, from a vertex descriptor.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam VVF  Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @param  vvf          Value function invoked per vertex.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v, val}.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf) {
  return vertices_bfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed_vertex, std::forward<VVF>(vvf),
                                                                       std::allocator<bool>{});
}

/**
 * @brief BFS vertex traversal from a vertex ID, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g      The graph to traverse.
 * @param  seed   Starting vertex ID.
 * @param  alloc  Allocator instance.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v}.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_bfs(G& g, const adj_list::vertex_id_t<G>& seed, Alloc alloc) {
  return vertices_bfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief BFS vertex traversal from a vertex descriptor, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @param  alloc        Allocator instance.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v}.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
  return vertices_bfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief BFS vertex traversal with value function, from a vertex ID, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam VVF    Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g      The graph to traverse.
 * @param  seed   Starting vertex ID.
 * @param  vvf    Value function invoked per vertex.
 * @param  alloc  Allocator instance.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v, val}.
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, const adj_list::vertex_id_t<G>& seed, VVF&& vvf, Alloc alloc) {
  return vertices_bfs_view<G, std::decay_t<VVF>, Alloc>(g, seed, std::forward<VVF>(vvf), alloc);
}

/**
 * @brief BFS vertex traversal with value function, from a vertex descriptor, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam VVF    Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @param  vvf          Value function invoked per vertex.
 * @param  alloc        Allocator instance.
 * @return A @c vertices_bfs_view whose iterators yield @c vertex_data{v, val}.
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
 * @brief BFS edge view without value function.
 *
 * Traverses tree edges reachable from a seed in breadth-first order,
 * yielding @c edge_data{uv} per edge via structured bindings.  The
 * seed vertex itself has no incoming tree edge, so it is skipped.
 *
 * @code
 *   for (auto [uv] : edges_bfs(g, seed)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal queue and visited tracker
 *
 * @see edges_bfs_view<G,EVF,Alloc> — with value function
 * @see vertices_bfs_view           — vertex-oriented BFS
 */
template <adj_list::index_adjacency_list G, class Alloc, class Accessor>
class edges_bfs_view<G, void, Alloc, Accessor> : public std::ranges::view_interface<edges_bfs_view<G, void, Alloc, Accessor>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using edge_type      = typename Accessor::template edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = edge_data<void, false, edge_type, void>;

private:
  using state_type = bfs_detail::bfs_edge_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Input iterator yielding @c edge_data{uv}.
     *
     * Advances to the first tree edge on construction (the seed vertex
     * has no incoming tree edge).  Single-pass: all copies share state.
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

          auto target_v   = Accessor{}.neighbor(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Add target vertex to queue with its edge range
            auto target_edge_range = Accessor{}.edges(*g_, target_v);
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
  edges_bfs_view(G& g, const vertex_id_type& seed, Alloc alloc = {})
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
 * @brief BFS edge view with an edge value function.
 *
 * Traverses tree edges reachable from a seed in breadth-first order,
 * yielding @c edge_data{uv, val} per edge via structured bindings.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [uv, val] : edges_bfs(g, seed, evf)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam EVF   Edge value function — @c invocable<const G&, edge_t<G>>
 * @tparam Alloc Allocator for the internal queue and visited tracker
 *
 * @see edges_bfs_view<G,void,Alloc> — without value function
 * @see vertices_bfs_view            — vertex-oriented BFS
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc, class Accessor>
class edges_bfs_view : public std::ranges::view_interface<edges_bfs_view<G, EVF, Alloc, Accessor>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_type         = typename Accessor::template edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_data<void, false, edge_type, value_result_type>;

private:
  using state_type = bfs_detail::bfs_edge_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Input iterator yielding @c edge_data{uv, val}.
     *
     * Single-pass: all copies share state via @c shared_ptr.
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

          auto target_v   = Accessor{}.neighbor(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Add target vertex to queue with its edge range
            auto target_edge_range = Accessor{}.edges(*g_, target_v);
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
  edges_bfs_view(G& g, const vertex_id_type& seed, EVF evf, Alloc alloc = {})
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
edges_bfs_view(G&, const adj_list::vertex_id_t<G>&, Alloc) -> edges_bfs_view<G, void, Alloc>;

template <class G, class Alloc>
edges_bfs_view(G&, adj_list::vertex_t<G>, Alloc) -> edges_bfs_view<G, void, Alloc>;

template <class G, class EVF, class Alloc>
edges_bfs_view(G&, const adj_list::vertex_id_t<G>&, EVF, Alloc) -> edges_bfs_view<G, EVF, Alloc>;

template <class G, class EVF, class Alloc>
edges_bfs_view(G&, adj_list::vertex_t<G>, EVF, Alloc) -> edges_bfs_view<G, EVF, Alloc>;

//=============================================================================
// Factory functions for edges_bfs
//=============================================================================

/**
 * @brief BFS edge traversal from a vertex ID, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex ID.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv}.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_bfs(G& g, const adj_list::vertex_id_t<G>& seed) {
  return edges_bfs_view<G, void, std::allocator<bool>>(g, seed);
}

/**
 * @brief BFS edge traversal from a vertex descriptor, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv}.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return edges_bfs_view<G, void, std::allocator<bool>>(g, seed_vertex);
}

/**
 * @brief BFS edge traversal with value function, from a vertex ID.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam EVF  Edge value function — @c invocable<const G&, edge_t<G>>.
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex ID.
 * @param  evf   Value function invoked per edge.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv, val}.
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>> && (!std::is_same_v<std::decay_t<EVF>, std::allocator<bool>>)
[[nodiscard]] auto edges_bfs(G& g, const adj_list::vertex_id_t<G>& seed, EVF&& evf) {
  return edges_bfs_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, seed, std::forward<EVF>(evf));
}

/**
 * @brief BFS edge traversal with value function, from a vertex descriptor.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam EVF  Edge value function — @c invocable<const G&, edge_t<G>>.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @param  evf          Value function invoked per edge.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv, val}.
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>> && (!std::is_same_v<std::decay_t<EVF>, std::allocator<bool>>)
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf) {
  return edges_bfs_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, seed_vertex, std::forward<EVF>(evf));
}

/**
 * @brief BFS edge traversal from a vertex ID, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g      The graph to traverse.
 * @param  seed   Starting vertex ID.
 * @param  alloc  Allocator instance.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv}.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_bfs(G& g, const adj_list::vertex_id_t<G>& seed, Alloc alloc) {
  return edges_bfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief BFS edge traversal from a vertex descriptor, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @param  alloc        Allocator instance.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv}.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
  return edges_bfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief BFS edge traversal with value function, from a vertex ID, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam EVF    Edge value function — @c invocable<const G&, edge_t<G>>.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g      The graph to traverse.
 * @param  seed   Starting vertex ID.
 * @param  evf    Value function invoked per edge.
 * @param  alloc  Allocator instance.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv, val}.
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_bfs(G& g, const adj_list::vertex_id_t<G>& seed, EVF&& evf, Alloc alloc) {
  return edges_bfs_view<G, std::decay_t<EVF>, Alloc>(g, seed, std::forward<EVF>(evf), alloc);
}

/**
 * @brief BFS edge traversal with value function, from a vertex descriptor, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam EVF    Edge value function — @c invocable<const G&, edge_t<G>>.
 * @tparam Alloc  Allocator for the internal queue and visited tracker.
 * @param  g            The graph to traverse.
 * @param  seed_vertex  Starting vertex descriptor.
 * @param  evf          Value function invoked per edge.
 * @param  alloc        Allocator instance.
 * @return An @c edges_bfs_view whose iterators yield @c edge_data{uv, val}.
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf, Alloc alloc) {
  return edges_bfs_view<G, std::decay_t<EVF>, Alloc>(g, seed_vertex, std::forward<EVF>(evf), alloc);
}

//=============================================================================
// Accessor-parameterized factory functions
//=============================================================================
// Usage: vertices_bfs<in_edge_accessor>(g, seed)
//        edges_bfs<in_edge_accessor>(g, seed)

/// BFS vertex traversal with explicit Accessor, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_bfs(G& g, const adj_list::vertex_id_t<G>& seed) {
  return vertices_bfs_view<G, void, std::allocator<bool>, Accessor>(g, seed, std::allocator<bool>{});
}

/// BFS vertex traversal with explicit Accessor, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return vertices_bfs_view<G, void, std::allocator<bool>, Accessor>(g, seed_vertex, std::allocator<bool>{});
}

/// BFS vertex traversal with explicit Accessor and value function, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, const adj_list::vertex_id_t<G>& seed, VVF&& vvf) {
  return vertices_bfs_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>(g, seed, std::forward<VVF>(vvf),
                                                                                  std::allocator<bool>{});
}

/// BFS vertex traversal with explicit Accessor and value function, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_bfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf) {
  return vertices_bfs_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>(g, seed_vertex,
                                                                                  std::forward<VVF>(vvf),
                                                                                  std::allocator<bool>{});
}

/// BFS edge traversal with explicit Accessor, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_bfs(G& g, const adj_list::vertex_id_t<G>& seed) {
  return edges_bfs_view<G, void, std::allocator<bool>, Accessor>(g, seed);
}

/// BFS edge traversal with explicit Accessor, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return edges_bfs_view<G, void, std::allocator<bool>, Accessor>(g, seed_vertex);
}

/// BFS edge traversal with explicit Accessor and value function, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, typename Accessor::template edge_t<G>>
[[nodiscard]] auto edges_bfs(G& g, const adj_list::vertex_id_t<G>& seed, EVF&& evf) {
  return edges_bfs_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>(g, seed, std::forward<EVF>(evf));
}

/// BFS edge traversal with explicit Accessor and value function, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, typename Accessor::template edge_t<G>>
[[nodiscard]] auto edges_bfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf) {
  return edges_bfs_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>(g, seed_vertex,
                                                                               std::forward<EVF>(evf));
}

} // namespace graph::views
