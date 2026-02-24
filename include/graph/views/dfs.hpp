/**
 * @file dfs.hpp
 * @brief Depth-first search views for vertices and edges.
 *
 * @section overview Overview
 *
 * Provides lazy, single-pass views that traverse a graph in depth-first
 * order starting from a seed vertex.  @c vertices_dfs yields per-vertex
 * @c vertex_info and @c edges_dfs yields per-edge @c edge_info.  An
 * optional value function computes a per-element value included in the
 * structured binding.
 *
 * Both view families expose additional accessors on the view object:
 * - @c depth()       — current depth in the DFS tree
 * - @c num_visited() — total vertices/edges visited so far
 * - @c cancel(c)     — stop traversal (@c cancel_branch or @c cancel_all)
 *
 * Only vertices/edges reachable from the seed are visited.
 *
 * @section variants View Variants
 *
 * | Variant                             | Structured Binding | Description                        |
 * |-------------------------------------|--------------------|------------------------------------|
 * | @c vertices_dfs(g,seed)             | `[v]`              | Vertex DFS (descriptor only)       |
 * | @c vertices_dfs(g,seed,vvf)         | `[v, val]`         | Vertex DFS with value function     |
 * | @c edges_dfs(g,seed)                | `[uv]`             | Edge DFS (edge descriptor only)    |
 * | @c edges_dfs(g,seed,evf)            | `[uv, val]`        | Edge DFS with value function       |
 *
 * Each factory also accepts a vertex descriptor in place of a vertex id,
 * and an optional custom allocator for the internal stack and visited
 * tracker.
 *
 * @section bindings Structured Bindings
 *
 * Vertex DFS:
 * @code
 *   for (auto [v]      : vertices_dfs(g, seed))          // v = vertex_t<G>
 *   for (auto [v, val] : vertices_dfs(g, seed, vvf))     // val = invoke_result_t<VVF, G&, vertex_t<G>>
 * @endcode
 *
 * Edge DFS:
 * @code
 *   for (auto [uv]      : edges_dfs(g, seed))            // uv = edge_t<G>
 *   for (auto [uv, val] : edges_dfs(g, seed, evf))       // val = invoke_result_t<EVF, G&, edge_t<G>>
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
 * onto an internal stack.  Each @c operator++ is amortised O(1) — it pops
 * or pushes at most one stack entry per edge traversed.  Full traversal is
 * O(V + E) time and O(V) space (stack depth ≤ V, visited tracker = V bits).
 *
 * @section chaining Chaining with std::views
 *
 * DFS views are @c input_range (single-pass), so they chain only with
 * adaptors that do not require forward ranges:
 *
 * @code
 *   auto view = vertices_dfs(g, seed)
 *               | std::views::take(5);  // ✅ compiles (take is input-compatible)
 * @endcode
 *
 * Adaptors that buffer or require multiple passes (e.g. @c std::views::reverse)
 * are not supported.
 *
 * @section search_control Search Control
 *
 * @code
 *   auto dfs = vertices_dfs(g, seed);
 *   for (auto [v] : dfs) {
 *       if (found(v))
 *           dfs.cancel(cancel_search::cancel_all);    // stop immediately
 *       if (prune(v))
 *           dfs.cancel(cancel_search::cancel_branch); // skip subtree
 *       std::cout << "depth=" << dfs.depth() << "\n";
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
 * stack).  Iteration may propagate exceptions from the value function; all
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
 * - @ref bfs.hpp — breadth-first search views
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
#include <stack>
#include <vector>
#include <graph/graph_info.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/view_concepts.hpp>
#include <graph/views/search_base.hpp>
#include <graph/views/edge_accessor.hpp>

namespace graph::views {

// Forward declarations
template <adj_list::index_adjacency_list G, class VVF = void, class Alloc = std::allocator<bool>,
          class Accessor = out_edge_accessor>
class vertices_dfs_view;

template <adj_list::index_adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>,
          class Accessor = out_edge_accessor>
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
  /// 2. View and iterators share state: The view exposes depth(), num_visited(), and cancel()
  ///    accessors that reflect current traversal state modified by iterators.
  /// 3. Range-based for loop: The view's cancel() must be able to stop iteration in progress.
  /// 4. Input iterator semantics: DFS is single-pass; shared state correctly models this.
  /// An alternative (state by value + raw pointers) would break if the view is moved.
  /**
   * @brief Shared DFS traversal state.
   *
   * Holds the explicit stack, visited tracker, cancellation flag, depth
   * counter and visit counter.  Shared via @c std::shared_ptr so that
   * iterator copies and the owning view all observe the same state.
   *
   * @par Complexity
   * Time: O(V + E) — visits each reachable vertex once, traverses each
   * reachable edge once.  Space: O(V) — stack ≤ V entries, visited
   * tracker = V bits.
   */
  template <class G, class Alloc, class Accessor = out_edge_accessor>
  struct dfs_state {
    using graph_type         = G;
    using vertex_type        = adj_list::vertex_t<G>;
    using vertex_id_type     = adj_list::vertex_id_t<G>;
    using edge_iterator_type = std::ranges::iterator_t<typename Accessor::template edge_range_t<G>>;
    using allocator_type     = Alloc;
    using entry_type         = stack_entry<vertex_type, edge_iterator_type>;
    using stack_alloc        = typename std::allocator_traits<Alloc>::template rebind_alloc<entry_type>;

    std::stack<entry_type, std::vector<entry_type, stack_alloc>> stack_;
    visited_tracker<vertex_id_type, Alloc>                       visited_;
    cancel_search                                                cancel_ = cancel_search::continue_search;
    std::size_t                                                  depth_  = 0;
    std::size_t                                                  count_  = 0;

    dfs_state(G& g, vertex_type seed_vertex, std::size_t num_vertices, Alloc alloc = {})
          : stack_(std::vector<entry_type, stack_alloc>(alloc)), visited_(num_vertices, alloc) {
      auto edge_range = Accessor{}.edges(g, seed_vertex);
      stack_.push({seed_vertex, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
      visited_.mark_visited(adj_list::vertex_id(g, seed_vertex));
      // Note: count_ is not incremented here. It's incremented in advance() when
      // a vertex is actually yielded by the iterator.
    }
  };

} // namespace dfs_detail

/**
 * @brief DFS vertex view without value function.
 *
 * Traverses vertices reachable from a seed in depth-first order,
 * yielding @c vertex_info{v} per vertex via structured bindings.
 *
 * @code
 *   for (auto [v] : vertices_dfs(g, seed)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal stack and visited tracker
 *
 * @see vertices_dfs_view<G,VVF,Alloc> — with value function
 * @see edges_dfs_view                 — edge-oriented DFS
 */
template <adj_list::index_adjacency_list G, class Alloc, class Accessor>
class vertices_dfs_view<G, void, Alloc, Accessor> : public std::ranges::view_interface<vertices_dfs_view<G, void, Alloc, Accessor>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using edge_type      = typename Accessor::template edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = vertex_info<void, vertex_type, void>;

private:
  using state_type = dfs_detail::dfs_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Input iterator yielding @c vertex_info{v}.
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

    [[nodiscard]] value_type operator*() const { return value_type{state_->stack_.top().vertex}; }

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
      bool this_at_end  = !state_ || state_->stack_.empty();
      bool other_at_end = !other.state_ || other.state_->stack_.empty();
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || state_->stack_.empty(); }

  private:
    void advance() {
      if (!state_ || state_->stack_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->stack_.empty())
          state_->stack_.pop();
        return;
      }

      // Handle cancel_branch: skip current subtree, continue with siblings
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
        state_->cancel_ = cancel_search::continue_search;
      }

      // Find next unvisited vertex using DFS
      while (!state_->stack_.empty()) {
        auto& top = state_->stack_.top();

        // Find next unvisited neighbor from current vertex
        bool found_unvisited = false;
        while (top.edge_iter != top.edge_end) {
          auto edge = *top.edge_iter;
          ++top.edge_iter;

          // Get target vertex descriptor using accessor
          auto target_v   = Accessor{}.neighbor(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Push target vertex with its edge range
            auto edge_range = Accessor{}.edges(*g_, target_v);
            state_->stack_.push({target_v, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
            ++state_->depth_;
            ++state_->count_;
            found_unvisited = true;
            break;
          }
        }

        if (found_unvisited) {
          return; // Found next vertex to visit
        }

        // No more unvisited neighbors, backtrack
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
  };

  /// Sentinel for end of DFS traversal
  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr vertices_dfs_view() noexcept = default;

  /// Construct from vertex descriptor
  vertices_dfs_view(G& g, vertex_type seed_vertex, Alloc alloc = {})
        : g_(&g), state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  vertices_dfs_view(G& g, vertex_id_type seed, Alloc alloc = {})
        : vertices_dfs_view(g, *adj_list::find_vertex(g, seed), alloc) {}

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

  /// Get current depth in DFS tree (stack size)
  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->stack_.size() : 0; }

  /// Get count of vertices visited so far
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  std::shared_ptr<state_type> state_;
};

/**
 * @brief DFS vertex view with a vertex value function.
 *
 * Traverses vertices reachable from a seed in depth-first order,
 * yielding @c vertex_info{v, val} per vertex via structured bindings.
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
 *   for (auto [v, val] : vertices_dfs(g, seed, vvf)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam VVF   Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @tparam Alloc Allocator for the internal stack and visited tracker
 *
 * @see vertices_dfs_view<G,void,Alloc> — without value function
 * @see edges_dfs_view                  — edge-oriented DFS
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc, class Accessor>
class vertices_dfs_view : public std::ranges::view_interface<vertices_dfs_view<G, VVF, Alloc, Accessor>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_type         = typename Accessor::template edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_info<void, vertex_type, value_result_type>;

private:
  using state_type = dfs_detail::dfs_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Input iterator yielding @c vertex_info{v, val}.
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
      auto v = state_->stack_.top().vertex;
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
      bool this_at_end  = !state_ || state_->stack_.empty();
      bool other_at_end = !other.state_ || other.state_->stack_.empty();
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || state_->stack_.empty(); }

  private:
    void advance() {
      if (!state_ || state_->stack_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->stack_.empty())
          state_->stack_.pop();
        return;
      }

      // Handle cancel_branch: skip current subtree, continue with siblings
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
        state_->cancel_ = cancel_search::continue_search;
      }

      // Find next unvisited vertex using DFS
      while (!state_->stack_.empty()) {
        auto& top = state_->stack_.top();

        // Find next unvisited neighbor from current vertex
        bool found_unvisited = false;
        while (top.edge_iter != top.edge_end) {
          auto edge = *top.edge_iter;
          ++top.edge_iter;

          // Get target vertex descriptor using accessor
          auto target_v   = Accessor{}.neighbor(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Push target vertex with its edge range
            auto edge_range = Accessor{}.edges(*g_, target_v);
            state_->stack_.push({target_v, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
            ++state_->depth_;
            ++state_->count_;
            found_unvisited = true;
            break;
          }
        }

        if (found_unvisited) {
          return; // Found next vertex to visit
        }

        // No more unvisited neighbors, backtrack
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    VVF*                        vvf_ = nullptr;
  };

  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr vertices_dfs_view() noexcept = default;

  /// Construct from vertex descriptor
  vertices_dfs_view(G& g, vertex_type seed_vertex, VVF vvf, Alloc alloc = {})
        : g_(&g)
        , vvf_(std::move(vvf))
        , state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  vertices_dfs_view(G& g, vertex_id_type seed, VVF vvf, Alloc alloc = {})
        : vertices_dfs_view(g, *adj_list::find_vertex(g, seed), std::move(vvf), alloc) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, &vvf_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->stack_.size() : 0; }

  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  [[no_unique_address]] VVF   vvf_{};
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
 * @brief Create a DFS vertex view from a vertex id (no value function).
 *
 * @code
 *   for (auto [v] : vertices_dfs(g, seed)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g    The graph to traverse.  Must outlive the returned view.
 * @param  seed Starting vertex id.
 * @return @c vertices_dfs_view yielding @c vertex_info{v} per reachable vertex.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed) {
  return vertices_dfs_view<G, void, std::allocator<bool>>(g, seed, std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view from a vertex descriptor (no value function).
 *
 * @code
 *   for (auto [v] : vertices_dfs(g, u)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @return @c vertices_dfs_view yielding @c vertex_info{v} per reachable vertex.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return vertices_dfs_view<G, void, std::allocator<bool>>(g, seed_vertex, std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view with a value function (from vertex id).
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
 *   for (auto [v, val] : vertices_dfs(g, seed, vvf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g    The graph to traverse.
 * @param  seed Starting vertex id.
 * @param  vvf  Value function invoked once per vertex.
 * @return @c vertices_dfs_view yielding @c vertex_info{v, val} per reachable vertex.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf) {
  return vertices_dfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed, std::forward<VVF>(vvf),
                                                                       std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view with a value function (from vertex descriptor).
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam VVF Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @param  vvf         Value function invoked once per vertex.
 * @return @c vertices_dfs_view yielding @c vertex_info{v, val} per reachable vertex.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf) {
  return vertices_dfs_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, seed_vertex, std::forward<VVF>(vvf),
                                                                       std::allocator<bool>{});
}

/**
 * @brief Create a DFS vertex view with a custom allocator (from vertex id).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex id.
 * @param  alloc Allocator instance.
 * @return @c vertices_dfs_view yielding @c vertex_info{v} per reachable vertex.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, Alloc alloc) {
  return vertices_dfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief Create a DFS vertex view with a custom allocator (from vertex descriptor).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @param  alloc       Allocator instance.
 * @return @c vertices_dfs_view yielding @c vertex_info{v} per reachable vertex.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
  return vertices_dfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief Create a DFS vertex view with value function and custom allocator
 *        (from vertex id).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam VVF   Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex id.
 * @param  vvf   Value function invoked once per vertex.
 * @param  alloc Allocator instance.
 * @return @c vertices_dfs_view yielding @c vertex_info{v, val} per reachable vertex.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf, Alloc alloc) {
  return vertices_dfs_view<G, std::decay_t<VVF>, Alloc>(g, seed, std::forward<VVF>(vvf), alloc);
}

/**
 * @brief Create a DFS vertex view with value function and custom allocator
 *        (from vertex descriptor).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam VVF   Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @param  vvf         Value function invoked once per vertex.
 * @param  alloc       Allocator instance.
 * @return @c vertices_dfs_view yielding @c vertex_info{v, val} per reachable vertex.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf, Alloc alloc) {
  return vertices_dfs_view<G, std::decay_t<VVF>, Alloc>(g, seed_vertex, std::forward<VVF>(vvf), alloc);
}

// =============================================================================
// edges_dfs_view - DFS edge traversal
// =============================================================================

/**
 * @brief DFS edge view without value function.
 *
 * Traverses tree edges reachable from a seed in depth-first order,
 * yielding @c edge_info{uv} per edge via structured bindings.  The
 * seed vertex itself has no incoming tree edge, so it is skipped.
 *
 * @code
 *   for (auto [uv] : edges_dfs(g, seed)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal stack and visited tracker
 *
 * @see edges_dfs_view<G,EVF,Alloc> — with value function
 * @see vertices_dfs_view           — vertex-oriented DFS
 */
template <adj_list::index_adjacency_list G, class Alloc, class Accessor>
class edges_dfs_view<G, void, Alloc, Accessor> : public std::ranges::view_interface<edges_dfs_view<G, void, Alloc, Accessor>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using edge_type      = typename Accessor::template edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = edge_info<void, false, edge_type, void>;

private:
  using state_type = dfs_detail::dfs_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Input iterator yielding @c edge_info{uv}.
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
      bool this_at_end  = !state_ || (state_->stack_.empty() && !has_edge_);
      bool other_at_end = !other.state_ || (other.state_->stack_.empty() && !other.has_edge_);
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || (state_->stack_.empty() && !has_edge_); }

  private:
    void advance_to_next_edge() {
      has_edge_ = false;
      if (!state_ || state_->stack_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->stack_.empty())
          state_->stack_.pop();
        return;
      }

      // Handle cancel_branch: skip current subtree, continue with siblings
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
        state_->cancel_ = cancel_search::continue_search;
      }

      // Find next tree edge using DFS
      while (!state_->stack_.empty()) {
        auto& top = state_->stack_.top();

        // Find next unvisited neighbor from current vertex
        while (top.edge_iter != top.edge_end) {
          auto edge = *top.edge_iter;
          ++top.edge_iter;

          // Get target vertex descriptor using accessor
          auto target_v   = Accessor{}.neighbor(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            // Push target vertex with its edge range
            auto edge_range = Accessor{}.edges(*g_, target_v);
            state_->stack_.push({target_v, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
            ++state_->depth_;
            ++state_->count_;

            // Store current edge and return
            current_edge_ = edge;
            has_edge_     = true;
            return;
          }
        }

        // No more unvisited neighbors, backtrack
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    edge_type                   current_edge_{};
    bool                        has_edge_ = false;
  };

  /// Sentinel for end of DFS traversal
  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr edges_dfs_view() noexcept = default;

  /// Construct from vertex descriptor
  edges_dfs_view(G& g, vertex_type seed_vertex, Alloc alloc = {})
        : g_(&g), state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  edges_dfs_view(G& g, vertex_id_type seed, Alloc alloc = {})
        : edges_dfs_view(g, *adj_list::find_vertex(g, seed), alloc) {}

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

  /// Get current depth in DFS tree (stack size)
  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->stack_.size() : 0; }

  /// Get count of edges visited so far
  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  std::shared_ptr<state_type> state_;
};

/**
 * @brief DFS edge view with an edge value function.
 *
 * Traverses tree edges reachable from a seed in depth-first order,
 * yielding @c edge_info{uv, val} per edge via structured bindings.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [uv, val] : edges_dfs(g, seed, evf)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam EVF   Edge value function — @c invocable<const G&, edge_t<G>>
 * @tparam Alloc Allocator for the internal stack and visited tracker
 *
 * @see edges_dfs_view<G,void,Alloc> — without value function
 * @see vertices_dfs_view            — vertex-oriented DFS
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc, class Accessor>
class edges_dfs_view : public std::ranges::view_interface<edges_dfs_view<G, EVF, Alloc, Accessor>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using edge_type         = typename Accessor::template edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_info<void, false, edge_type, value_result_type>;

private:
  using state_type = dfs_detail::dfs_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Input iterator yielding @c edge_info{uv, val}.
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
      bool this_at_end  = !state_ || (state_->stack_.empty() && !has_edge_);
      bool other_at_end = !other.state_ || (other.state_->stack_.empty() && !other.has_edge_);
      return this_at_end == other_at_end;
    }

    [[nodiscard]] bool at_end() const noexcept { return !state_ || (state_->stack_.empty() && !has_edge_); }

  private:
    void advance_to_next_edge() {
      has_edge_ = false;
      if (!state_ || state_->stack_.empty())
        return;
      if (state_->cancel_ == cancel_search::cancel_all) {
        while (!state_->stack_.empty())
          state_->stack_.pop();
        return;
      }

      // Handle cancel_branch: skip current subtree, continue with siblings
      if (state_->cancel_ == cancel_search::cancel_branch) {
        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
        state_->cancel_ = cancel_search::continue_search;
      }

      while (!state_->stack_.empty()) {
        auto& top = state_->stack_.top();

        while (top.edge_iter != top.edge_end) {
          auto edge = *top.edge_iter;
          ++top.edge_iter;

          auto target_v   = Accessor{}.neighbor(*g_, edge);
          auto target_vid = adj_list::vertex_id(*g_, target_v);

          if (!state_->visited_.is_visited(target_vid)) {
            state_->visited_.mark_visited(target_vid);

            auto edge_range = Accessor{}.edges(*g_, target_v);
            state_->stack_.push({target_v, std::ranges::begin(edge_range), std::ranges::end(edge_range)});
            ++state_->depth_;
            ++state_->count_;

            current_edge_ = edge;
            has_edge_     = true;
            return;
          }
        }

        state_->stack_.pop();
        if (state_->depth_ > 0)
          --state_->depth_;
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    EVF*                        evf_ = nullptr;
    edge_type                   current_edge_{};
    bool                        has_edge_ = false;
  };

  struct sentinel {
    [[nodiscard]] constexpr bool operator==(const iterator& it) const noexcept { return it.at_end(); }
  };

  constexpr edges_dfs_view() noexcept = default;

  /// Construct from vertex descriptor
  edges_dfs_view(G& g, vertex_type seed_vertex, EVF evf, Alloc alloc = {})
        : g_(&g)
        , evf_(std::move(evf))
        , state_(std::make_shared<state_type>(g, seed_vertex, adj_list::num_vertices(g), alloc)) {}

  /// Construct from vertex ID (delegates to vertex descriptor constructor)
  edges_dfs_view(G& g, vertex_id_type seed, EVF evf, Alloc alloc = {})
        : edges_dfs_view(g, *adj_list::find_vertex(g, seed), std::move(evf), alloc) {}

  [[nodiscard]] iterator begin() { return iterator(g_, state_, &evf_); }
  [[nodiscard]] sentinel end() const noexcept { return {}; }

  [[nodiscard]] cancel_search cancel() const noexcept {
    return state_ ? state_->cancel_ : cancel_search::continue_search;
  }

  void cancel(cancel_search c) noexcept {
    if (state_)
      state_->cancel_ = c;
  }

  [[nodiscard]] std::size_t depth() const noexcept { return state_ ? state_->stack_.size() : 0; }

  [[nodiscard]] std::size_t num_visited() const noexcept { return state_ ? state_->count_ : 0; }

private:
  G*                          g_ = nullptr;
  [[no_unique_address]] EVF   evf_{};
  std::shared_ptr<state_type> state_;
};

// Deduction guides for edges_dfs_view - vertex_id
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
edges_dfs_view(G&, adj_list::vertex_id_t<G>, Alloc) -> edges_dfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
edges_dfs_view(G&, adj_list::vertex_id_t<G>) -> edges_dfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class EVF, class Alloc = std::allocator<bool>>
edges_dfs_view(G&, adj_list::vertex_id_t<G>, EVF, Alloc) -> edges_dfs_view<G, EVF, Alloc>;

// Deduction guides for edges_dfs_view - vertex descriptor
template <adj_list::index_adjacency_list G, class Alloc = std::allocator<bool>>
edges_dfs_view(G&, adj_list::vertex_t<G>, Alloc) -> edges_dfs_view<G, void, Alloc>;

template <adj_list::index_adjacency_list G>
edges_dfs_view(G&, adj_list::vertex_t<G>) -> edges_dfs_view<G, void, std::allocator<bool>>;

template <adj_list::index_adjacency_list G, class EVF, class Alloc = std::allocator<bool>>
edges_dfs_view(G&, adj_list::vertex_t<G>, EVF, Alloc) -> edges_dfs_view<G, EVF, Alloc>;

// =============================================================================
// edges_dfs factory functions
// =============================================================================

/**
 * @brief Create a DFS edge view from a vertex id (no value function).
 *
 * @code
 *   for (auto [uv] : edges_dfs(g, seed)) { ... }
 * @endcode
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g    The graph to traverse.  Must outlive the returned view.
 * @param  seed Starting vertex id.
 * @return @c edges_dfs_view yielding @c edge_info{uv} per reachable tree edge.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_id_t<G> seed) {
  return edges_dfs_view<G, void, std::allocator<bool>>(g, seed, std::allocator<bool>{});
}

/**
 * @brief Create a DFS edge view from a vertex descriptor (no value function).
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @return @c edges_dfs_view yielding @c edge_info{uv} per reachable tree edge.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return edges_dfs_view<G, void, std::allocator<bool>>(g, seed_vertex, std::allocator<bool>{});
}

/**
 * @brief Create a DFS edge view with a value function (from vertex id).
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [uv, val] : edges_dfs(g, seed, evf)) { ... }
 * @endcode
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g    The graph to traverse.
 * @param  seed Starting vertex id.
 * @param  evf  Value function invoked once per tree edge.
 * @return @c edges_dfs_view yielding @c edge_info{uv, val} per reachable tree edge.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_id_t<G> seed, EVF&& evf) {
  return edges_dfs_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, seed, std::forward<EVF>(evf),
                                                                    std::allocator<bool>{});
}

/**
 * @brief Create a DFS edge view with a value function (from vertex descriptor).
 *
 * @tparam G   Graph type satisfying @c index_adjacency_list
 * @tparam EVF Edge value function — @c invocable<const G&, edge_t<G>>
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @param  evf         Value function invoked once per tree edge.
 * @return @c edges_dfs_view yielding @c edge_info{uv, val} per reachable tree edge.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf) {
  return edges_dfs_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, seed_vertex, std::forward<EVF>(evf),
                                                                    std::allocator<bool>{});
}

/**
 * @brief Create a DFS edge view with a custom allocator (from vertex id).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex id.
 * @param  alloc Allocator instance.
 * @return @c edges_dfs_view yielding @c edge_info{uv} per reachable tree edge.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_id_t<G> seed, Alloc alloc) {
  return edges_dfs_view<G, void, Alloc>(g, seed, alloc);
}

/**
 * @brief Create a DFS edge view with a custom allocator (from vertex descriptor).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @param  alloc       Allocator instance.
 * @return @c edges_dfs_view yielding @c edge_info{uv} per reachable tree edge.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_t<G> seed_vertex, Alloc alloc) {
  return edges_dfs_view<G, void, Alloc>(g, seed_vertex, alloc);
}

/**
 * @brief Create a DFS edge view with value function and custom allocator
 *        (from vertex id).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam EVF   Edge value function — @c invocable<const G&, edge_t<G>>
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g     The graph to traverse.
 * @param  seed  Starting vertex id.
 * @param  evf   Value function invoked once per tree edge.
 * @param  alloc Allocator instance.
 * @return @c edges_dfs_view yielding @c edge_info{uv, val} per reachable tree edge.
 *
 * @pre  @c seed is a valid vertex id in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_id_t<G> seed, EVF&& evf, Alloc alloc) {
  return edges_dfs_view<G, std::decay_t<EVF>, Alloc>(g, seed, std::forward<EVF>(evf), alloc);
}

/**
 * @brief Create a DFS edge view with value function and custom allocator
 *        (from vertex descriptor).
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam EVF   Edge value function — @c invocable<const G&, edge_t<G>>
 * @tparam Alloc Allocator for the internal stack and visited tracker
 * @param  g           The graph to traverse.
 * @param  seed_vertex Starting vertex descriptor.
 * @param  evf         Value function invoked once per tree edge.
 * @param  alloc       Allocator instance.
 * @return @c edges_dfs_view yielding @c edge_info{uv, val} per reachable tree edge.
 *
 * @pre  @c seed_vertex is a valid vertex descriptor in @c g.
 * @post The graph is not modified.
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf, Alloc alloc) {
  return edges_dfs_view<G, std::decay_t<EVF>, Alloc>(g, seed_vertex, std::forward<EVF>(evf), alloc);
}

//=============================================================================
// Accessor-parameterized factory functions
//=============================================================================
// Usage: vertices_dfs<in_edge_accessor>(g, seed)
//        edges_dfs<in_edge_accessor>(g, seed)

/// DFS vertex traversal with explicit Accessor, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed) {
  return vertices_dfs_view<G, void, std::allocator<bool>, Accessor>(g, seed, std::allocator<bool>{});
}

/// DFS vertex traversal with explicit Accessor, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return vertices_dfs_view<G, void, std::allocator<bool>, Accessor>(g, seed_vertex, std::allocator<bool>{});
}

/// DFS vertex traversal with explicit Accessor and value function, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_id_t<G> seed, VVF&& vvf) {
  return vertices_dfs_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>(g, seed, std::forward<VVF>(vvf),
                                                                                  std::allocator<bool>{});
}

/// DFS vertex traversal with explicit Accessor and value function, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_dfs(G& g, adj_list::vertex_t<G> seed_vertex, VVF&& vvf) {
  return vertices_dfs_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>(g, seed_vertex,
                                                                                  std::forward<VVF>(vvf),
                                                                                  std::allocator<bool>{});
}

/// DFS edge traversal with explicit Accessor, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_id_t<G> seed) {
  return edges_dfs_view<G, void, std::allocator<bool>, Accessor>(g, seed, std::allocator<bool>{});
}

/// DFS edge traversal with explicit Accessor, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_t<G> seed_vertex) {
  return edges_dfs_view<G, void, std::allocator<bool>, Accessor>(g, seed_vertex, std::allocator<bool>{});
}

/// DFS edge traversal with explicit Accessor and value function, from vertex ID.
template <class Accessor, adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, typename Accessor::template edge_t<G>>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_id_t<G> seed, EVF&& evf) {
  return edges_dfs_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>(g, seed, std::forward<EVF>(evf),
                                                                               std::allocator<bool>{});
}

/// DFS edge traversal with explicit Accessor and value function, from vertex descriptor.
template <class Accessor, adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, typename Accessor::template edge_t<G>>
[[nodiscard]] auto edges_dfs(G& g, adj_list::vertex_t<G> seed_vertex, EVF&& evf) {
  return edges_dfs_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>(g, seed_vertex,
                                                                               std::forward<EVF>(evf),
                                                                               std::allocator<bool>{});
}

} // namespace graph::views
