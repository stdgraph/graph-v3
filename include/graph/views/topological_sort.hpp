/**
 * @file topological_sort.hpp
 * @brief Topological sort views for directed acyclic graphs (DAGs).
 *
 * Provides vertex- and edge-oriented views that traverse a directed graph in
 * topological order — every vertex appears before all vertices it has edges
 * to.  The ordering is computed eagerly via reverse DFS post-order over the
 * **entire** graph (not from a single seed); disconnected components are
 * each processed in topological order, with inter-component order arbitrary.
 *
 * A separate set of @c _safe factory functions enables cycle detection: they
 * return @c tl::expected, yielding the view on success or the vertex that
 * closes a back edge on failure.
 *
 * ---
 *
 * ## View Variants
 *
 * | Factory                                    | Binding      | Description                        |
 * |--------------------------------------------|--------------|------------------------------------|
 * | `vertices_topological_sort(g)`             | `[v]`        | Vertices in topological order      |
 * | `vertices_topological_sort(g, vvf)`        | `[v, val]`   | With vertex value function         |
 * | `edges_topological_sort(g)`                | `[uv]`       | Edges grouped by source, topo order|
 * | `edges_topological_sort(g, evf)`           | `[uv, val]`  | With edge value function           |
 * | `vertices_topological_sort_safe(g)`        | `[v]`        | With cycle detection               |
 * | `edges_topological_sort_safe(g)`           | `[uv]`       | With cycle detection               |
 *
 * Each factory also has a custom-allocator overload (+ VVF/EVF variant).
 *
 * ---
 *
 * ## Structured Bindings
 *
 * ### Vertex views
 * @code
 *   for (auto [v]      : vertices_topological_sort(g))           // vertex_data{v}
 *   for (auto [v, val] : vertices_topological_sort(g, vvf))      // vertex_data{v, val}
 * @endcode
 *
 * | Element | Type                                     | Meaning             |
 * |---------|------------------------------------------|---------------------|
 * | `v`     | `vertex_t<G>`                            | Vertex descriptor   |
 * | `val`   | `invoke_result_t<VVF, const G&, vertex_t<G>>` | Computed value |
 *
 * ### Edge views
 * @code
 *   for (auto [uv]      : edges_topological_sort(g))             // edge_data{uv}
 *   for (auto [uv, val] : edges_topological_sort(g, evf))        // edge_data{uv, val}
 * @endcode
 *
 * | Element | Type                                     | Meaning             |
 * |---------|------------------------------------------|---------------------|
 * | `uv`    | `edge_t<G>`                              | Edge descriptor     |
 * | `val`   | `invoke_result_t<EVF, const G&, edge_t<G>>` | Computed value  |
 *
 * ---
 *
 * ## Iterator Properties
 *
 * | Property       | Value                                         |
 * |----------------|-----------------------------------------------|
 * | Category       | `std::forward_iterator_tag`                   |
 * | Sized          | Vertex views: yes (`size()`); edge views: no  |
 * | Common range   | No — uses sentinel                            |
 * | Borrowed range | No — state held via `shared_ptr`              |
 *
 * Unlike search views (DFS/BFS), these iterators are **forward** iterators
 * because the topological order is fully materialised before iteration
 * begins.
 *
 * ---
 *
 * ## Performance
 *
 * | Metric | Bound                                                  |
 * |--------|--------------------------------------------------------|
 * | Time   | O(V + E) — DFS visits every vertex / edge once         |
 * | Space  | O(V) — post-order vector + visited tracker (+ O(V)     |
 * |        | recursion stack when cycle detection is enabled)        |
 *
 * ---
 *
 * ## Chaining
 *
 * The views satisfy `std::ranges::forward_range` (non-common, sentinel-
 * based).  They compose with range adaptors that accept forward ranges
 * (e.g. `std::views::take`, `std::views::filter`).
 *
 * ---
 *
 * ## Cycle Detection
 *
 * The `_safe` factory functions allocate an additional recursion stack
 * (`std::vector<bool>`) and detect back edges during the DFS phase.
 * If a cycle is found the factory returns `tl::unexpected(cycle_vertex)`.
 *
 * @code
 *   auto result = vertices_topological_sort_safe(g);
 *   if (result) {
 *       for (auto [v] : *result) { ... }
 *   } else {
 *       auto cycle_v = result.error();   // vertex closing the back edge
 *   }
 * @endcode
 *
 * ---
 *
 * ## Search Control
 *
 * The view exposes `cancel()` / `cancel(cancel_search)` to stop iteration
 * early.  Because the ordering is flat (not tree-shaped), `cancel_branch`
 * is treated identically to `cancel_all`.
 *
 * ---
 *
 * ## Supported Graphs
 *
 * Requires @c index_adjacency_list (integer vertex IDs, O(1) vertex
 * access).  Intended for **directed** graphs; the result on undirected
 * graphs is well-defined but rarely meaningful.
 *
 * ---
 *
 * ## Exception Safety
 *
 * - Construction: may throw if DFS allocation fails — strong guarantee.
 * - Iteration: `operator*` and `operator++` are non-throwing for the
 *   view itself; a user-supplied value function may throw.
 *
 * ---
 *
 * ## Preconditions
 *
 * - The graph must remain valid and unmodified for the view's lifetime.
 * - For the non-safe factories, behaviour is defined even when cycles
 *   exist — the ordering is simply not a valid topological sort.
 *
 * ---
 *
 * @note This view processes **all** vertices in the graph, not just those
 *       reachable from a single seed.
 *
 * @see dfs.hpp  — depth-first search views (single-seed, lazy)
 * @see bfs.hpp  — breadth-first search views (single-seed, lazy)
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
class vertices_topological_sort_view;

template <adj_list::index_adjacency_list G, class EVF = void, class Alloc = std::allocator<bool>,
          class Accessor = out_edge_accessor>
class edges_topological_sort_view;

namespace topo_detail {

  /**
   * @brief Shared topological-sort state.
   *
   * Performs DFS from every unvisited vertex, collecting a post-order
   * vector, then reverses it to obtain topological order.  When
   * @c detect_cycles is true an additional recursion-stack vector
   * tracks back edges.
   *
   * Shared via @c std::shared_ptr so that all iterators and the owning
   * view observe the same materialised ordering.
   *
   * @par Complexity
   * Time: O(V + E).  Space: O(V) (O(2V) with cycle detection).
   */
  template <class G, class Alloc, class Accessor = out_edge_accessor>
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
      for (auto edge : Accessor{}.edges(g, v)) {
        auto target_v   = Accessor{}.neighbor(g, edge);
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
 * @brief Topological sort vertex view without value function.
 *
 * Iterates over all vertices in topological order, yielding
 * @c vertex_data{v} per vertex via structured bindings.
 *
 * @code
 *   for (auto [v] : vertices_topological_sort(g)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for internal containers
 *
 * @see vertices_topological_sort_view<G,VVF,Alloc> — with value function
 * @see edges_topological_sort_view                 — edge-oriented variant
 */
template <adj_list::index_adjacency_list G, class Alloc, class Accessor>
class vertices_topological_sort_view<G, void, Alloc, Accessor>
      : public std::ranges::view_interface<vertices_topological_sort_view<G, void, Alloc, Accessor>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using vertex_id_type = adj_list::vertex_id_t<G>;
  using allocator_type = Alloc;
  using info_type      = vertex_data<void, vertex_type, void>;

private:
  using state_type = topo_detail::topo_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Forward iterator yielding @c vertex_data{v}.
     *
     * Iterates over the pre-computed post-order vector in reverse
     * (topological) order.  Supports multi-pass traversal.
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
 * @brief Topological sort vertex view with a vertex value function.
 *
 * Iterates over all vertices in topological order, yielding
 * @c vertex_data{v, val} per vertex via structured bindings.
 *
 * @code
 *   auto vvf = [](const auto& g, auto v) { return vertex_id(g, v); };
 *   for (auto [v, val] : vertices_topological_sort(g, vvf)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam VVF   Vertex value function — @c invocable<const G&, vertex_t<G>>
 * @tparam Alloc Allocator for internal containers
 *
 * @see vertices_topological_sort_view<G,void,Alloc> — without value function
 * @see edges_topological_sort_view                  — edge-oriented variant
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc, class Accessor>
class vertices_topological_sort_view
      : public std::ranges::view_interface<vertices_topological_sort_view<G, VVF, Alloc, Accessor>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using vertex_id_type    = adj_list::vertex_id_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<VVF, const G&, vertex_type>;
  using info_type         = vertex_data<void, vertex_type, value_result_type>;

private:
  using state_type = topo_detail::topo_state<G, Alloc, Accessor>;

public:
  /**
     * @brief Forward iterator yielding @c vertex_data{v, val}.
     *
     * Multi-pass.  The value function is invoked on each dereference.
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
 * @brief Topological sort edge view without value function.
 *
 * Iterates over all edges grouped by source vertex in topological order,
 * yielding @c edge_data{uv} per edge via structured bindings.  Vertices
 * with no outgoing edges are silently skipped.
 *
 * @code
 *   for (auto [uv] : edges_topological_sort(g)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam Alloc Allocator for internal containers
 *
 * @see edges_topological_sort_view<G,EVF,Alloc> — with value function
 * @see vertices_topological_sort_view           — vertex-oriented variant
 */
template <adj_list::index_adjacency_list G, class Alloc, class Accessor>
class edges_topological_sort_view<G, void, Alloc, Accessor>
      : public std::ranges::view_interface<edges_topological_sort_view<G, void, Alloc, Accessor>> {
public:
  using graph_type     = G;
  using vertex_type    = adj_list::vertex_t<G>;
  using edge_type      = typename Accessor::template edge_t<G>;
  using allocator_type = Alloc;
  using info_type      = edge_data<void, false, edge_type, void>;

private:
  using state_type     = topo_detail::topo_state<G, Alloc, Accessor>;
  using edge_iter_type = std::ranges::iterator_t<typename Accessor::template edge_range_t<G>>;

public:
  /**
     * @brief Forward iterator yielding @c edge_data{uv}.
     *
     * Walks the adjacency-list edges of each source vertex in
     * topological order.  Multi-pass.
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
        auto edge_range = Accessor{}.edges(*g_, v);
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
          auto edge_range = Accessor{}.edges(*g_, v);
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
          auto edge_range = Accessor{}.edges(*g_, v);
          edge_it_        = std::ranges::begin(edge_range);
          edge_end_       = std::ranges::end(edge_range);
        }
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    std::size_t                 vertex_index_ = 0;
    edge_iter_type              edge_it_{};
    edge_iter_type              edge_end_{};
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
 * @brief Topological sort edge view with an edge value function.
 *
 * Iterates over all edges grouped by source vertex in topological order,
 * yielding @c edge_data{uv, val} per edge via structured bindings.
 *
 * @code
 *   auto evf = [](const auto& g, auto uv) { return target_id(g, uv); };
 *   for (auto [uv, val] : edges_topological_sort(g, evf)) { ... }
 * @endcode
 *
 * @tparam G     Graph type satisfying @c index_adjacency_list
 * @tparam EVF   Edge value function — @c invocable<const G&, edge_t<G>>
 * @tparam Alloc Allocator for internal containers
 *
 * @see edges_topological_sort_view<G,void,Alloc> — without value function
 * @see vertices_topological_sort_view            — vertex-oriented variant
 */
template <adj_list::index_adjacency_list G, class EVF, class Alloc, class Accessor>
class edges_topological_sort_view : public std::ranges::view_interface<edges_topological_sort_view<G, EVF, Alloc, Accessor>> {
public:
  using graph_type        = G;
  using vertex_type       = adj_list::vertex_t<G>;
  using edge_type         = typename Accessor::template edge_t<G>;
  using allocator_type    = Alloc;
  using value_result_type = std::invoke_result_t<EVF, const G&, edge_type>;
  using info_type         = edge_data<void, false, edge_type, value_result_type>;

private:
  using state_type     = topo_detail::topo_state<G, Alloc, Accessor>;
  using edge_iter_type = std::ranges::iterator_t<typename Accessor::template edge_range_t<G>>;

public:
  /**
     * @brief Forward iterator yielding @c edge_data{uv, val}.
     *
     * Multi-pass.  The value function is invoked on each dereference.
     */
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
        auto edge_range = Accessor{}.edges(*g_, v);
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
          auto edge_range = Accessor{}.edges(*g_, v);
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
          auto edge_range = Accessor{}.edges(*g_, v);
          edge_it_        = std::ranges::begin(edge_range);
          edge_end_       = std::ranges::end(edge_range);
        }
      }
    }

    G*                          g_ = nullptr;
    std::shared_ptr<state_type> state_;
    std::size_t                 vertex_index_ = 0;
    edge_iter_type              edge_it_{};
    edge_iter_type              edge_end_{};
    EVF*                        evf_ = nullptr;
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
 * @brief Topological vertex traversal, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g  The graph to traverse.
 * @return A @c vertices_topological_sort_view whose iterators yield @c vertex_data{v}.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_topological_sort(G& g) {
  return vertices_topological_sort_view<G, void, std::allocator<bool>>(g, std::allocator<bool>{});
}

/**
 * @brief Topological vertex traversal with value function.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam VVF  Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @param  g    The graph to traverse.
 * @param  vvf  Value function invoked per vertex.
 * @return A @c vertices_topological_sort_view whose iterators yield @c vertex_data{v, val}.
 */
template <adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort(G& g, VVF&& vvf) {
  return vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>>(g, std::forward<VVF>(vvf),
                                                                                    std::allocator<bool>{});
}

/**
 * @brief Topological vertex traversal, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for the post-order vector and visited tracker.
 * @param  g      The graph to traverse.
 * @param  alloc  Allocator instance.
 * @return A @c vertices_topological_sort_view whose iterators yield @c vertex_data{v}.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!vertex_value_function<Alloc, G, adj_list::vertex_t<G>>)
[[nodiscard]] auto vertices_topological_sort(G& g, Alloc alloc) {
  return vertices_topological_sort_view<G, void, Alloc>(g, alloc);
}

/**
 * @brief Topological vertex traversal with value function, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam VVF    Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @tparam Alloc  Allocator for the post-order vector and visited tracker.
 * @param  g      The graph to traverse.
 * @param  vvf    Value function invoked per vertex.
 * @param  alloc  Allocator instance.
 * @return A @c vertices_topological_sort_view whose iterators yield @c vertex_data{v, val}.
 */
template <adj_list::index_adjacency_list G, class VVF, class Alloc>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort(G& g, VVF&& vvf, Alloc alloc) {
  return vertices_topological_sort_view<G, std::decay_t<VVF>, Alloc>(g, std::forward<VVF>(vvf), alloc);
}

/**
 * @brief Topological edge traversal, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g  The graph to traverse.
 * @return An @c edges_topological_sort_view whose iterators yield @c edge_data{uv}.
 */
template <adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_topological_sort(G& g) {
  return edges_topological_sort_view<G, void, std::allocator<bool>>(g, std::allocator<bool>{});
}

/**
 * @brief Topological edge traversal with value function.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam EVF  Edge value function — @c invocable<const G&, edge_t<G>>.
 * @param  g    The graph to traverse.
 * @param  evf  Value function invoked per edge.
 * @return An @c edges_topological_sort_view whose iterators yield @c edge_data{uv, val}.
 */
template <adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] auto edges_topological_sort(G& g, EVF&& evf) {
  return edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>>(g, std::forward<EVF>(evf),
                                                                                 std::allocator<bool>{});
}

/**
 * @brief Topological edge traversal, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for the post-order vector and visited tracker.
 * @param  g      The graph to traverse.
 * @param  alloc  Allocator instance.
 * @return An @c edges_topological_sort_view whose iterators yield @c edge_data{uv}.
 */
template <adj_list::index_adjacency_list G, class Alloc>
requires(!edge_value_function<Alloc, G, adj_list::edge_t<G>>)
[[nodiscard]] auto edges_topological_sort(G& g, Alloc alloc) {
  return edges_topological_sort_view<G, void, Alloc>(g, alloc);
}

/**
 * @brief Topological edge traversal with value function, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam EVF    Edge value function — @c invocable<const G&, edge_t<G>>.
 * @tparam Alloc  Allocator for the post-order vector and visited tracker.
 * @param  g      The graph to traverse.
 * @param  evf    Value function invoked per edge.
 * @param  alloc  Allocator instance.
 * @return An @c edges_topological_sort_view whose iterators yield @c edge_data{uv, val}.
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
 * @brief Topological vertex traversal with cycle detection, default allocator.
 *
 * Performs DFS with a recursion-stack tracker.  Returns the view on
 * success, or the vertex that closes a back edge on failure.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g  The graph to traverse.
 * @return @c tl::expected containing the view, or the cycle vertex.
 *
 * @code
 *   auto result = vertices_topological_sort_safe(g);
 *   if (result) {
 *       for (auto [v] : *result) { ... }
 *   } else {
 *       auto cycle_v = result.error();
 *   }
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
 * @brief Topological vertex traversal with value function and cycle detection.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam VVF  Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @param  g    The graph to traverse.
 * @param  vvf  Value function invoked per vertex.
 * @return @c tl::expected containing the view, or the cycle vertex.
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
 * @brief Topological vertex traversal with cycle detection, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for internal containers.
 * @param  g      The graph to traverse.
 * @param  alloc  Allocator instance.
 * @return @c tl::expected containing the view, or the cycle vertex.
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
 * @brief Topological vertex traversal with value function, allocator, and cycle detection.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam VVF    Vertex value function — @c invocable<const G&, vertex_t<G>>.
 * @tparam Alloc  Allocator for internal containers.
 * @param  g      The graph to traverse.
 * @param  vvf    Value function invoked per vertex.
 * @param  alloc  Allocator instance.
 * @return @c tl::expected containing the view, or the cycle vertex.
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
 * @brief Topological edge traversal with cycle detection, default allocator.
 *
 * @tparam G  Graph type satisfying @c index_adjacency_list.
 * @param  g  The graph to traverse.
 * @return @c tl::expected containing the view, or the cycle vertex.
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
 * @brief Topological edge traversal with value function and cycle detection.
 *
 * @tparam G    Graph type satisfying @c index_adjacency_list.
 * @tparam EVF  Edge value function — @c invocable<const G&, edge_t<G>>.
 * @param  g    The graph to traverse.
 * @param  evf  Value function invoked per edge.
 * @return @c tl::expected containing the view, or the cycle vertex.
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
 * @brief Topological edge traversal with cycle detection, custom allocator.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam Alloc  Allocator for internal containers.
 * @param  g      The graph to traverse.
 * @param  alloc  Allocator instance.
 * @return @c tl::expected containing the view, or the cycle vertex.
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
 * @brief Topological edge traversal with value function, allocator, and cycle detection.
 *
 * @tparam G      Graph type satisfying @c index_adjacency_list.
 * @tparam EVF    Edge value function — @c invocable<const G&, edge_t<G>>.
 * @tparam Alloc  Allocator for internal containers.
 * @param  g      The graph to traverse.
 * @param  evf    Value function invoked per edge.
 * @param  alloc  Allocator instance.
 * @return @c tl::expected containing the view, or the cycle vertex.
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

//=============================================================================
// Accessor-parameterized factory functions
//=============================================================================
// Usage: vertices_topological_sort<in_edge_accessor>(g)
//        edges_topological_sort<in_edge_accessor>(g)

/// Topological vertex traversal with explicit Accessor.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_topological_sort(G& g) {
  return vertices_topological_sort_view<G, void, std::allocator<bool>, Accessor>(g, std::allocator<bool>{});
}

/// Topological vertex traversal with explicit Accessor and value function.
template <class Accessor, adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort(G& g, VVF&& vvf) {
  return vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>(
        g, std::forward<VVF>(vvf), std::allocator<bool>{});
}

/// Topological edge traversal with explicit Accessor.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_topological_sort(G& g) {
  return edges_topological_sort_view<G, void, std::allocator<bool>, Accessor>(g, std::allocator<bool>{});
}

/// Topological edge traversal with explicit Accessor and value function.
template <class Accessor, adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, typename Accessor::template edge_t<G>>
[[nodiscard]] auto edges_topological_sort(G& g, EVF&& evf) {
  return edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>(
        g, std::forward<EVF>(evf), std::allocator<bool>{});
}

//=============================================================================
// Accessor-parameterized safe factory functions
//=============================================================================

/// Topological vertex traversal with explicit Accessor and cycle detection.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto vertices_topological_sort_safe(G& g)
      -> tl::expected<vertices_topological_sort_view<G, void, std::allocator<bool>, Accessor>, adj_list::vertex_t<G>> {
  using view_type = vertices_topological_sort_view<G, void, std::allocator<bool>, Accessor>;

  auto state =
        std::make_shared<topo_detail::topo_state<G, std::allocator<bool>, Accessor>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::move(state));
}

/// Topological vertex traversal with explicit Accessor, value function, and cycle detection.
template <class Accessor, adj_list::index_adjacency_list G, class VVF>
requires vertex_value_function<VVF, G, adj_list::vertex_t<G>>
[[nodiscard]] auto vertices_topological_sort_safe(G& g, VVF&& vvf)
      -> tl::expected<vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>,
                      adj_list::vertex_t<G>> {
  using view_type = vertices_topological_sort_view<G, std::decay_t<VVF>, std::allocator<bool>, Accessor>;

  auto state =
        std::make_shared<topo_detail::topo_state<G, std::allocator<bool>, Accessor>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::forward<VVF>(vvf), std::move(state));
}

/// Topological edge traversal with explicit Accessor and cycle detection.
template <class Accessor, adj_list::index_adjacency_list G>
[[nodiscard]] auto edges_topological_sort_safe(G& g)
      -> tl::expected<edges_topological_sort_view<G, void, std::allocator<bool>, Accessor>, adj_list::vertex_t<G>> {
  using view_type = edges_topological_sort_view<G, void, std::allocator<bool>, Accessor>;

  auto state =
        std::make_shared<topo_detail::topo_state<G, std::allocator<bool>, Accessor>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::move(state));
}

/// Topological edge traversal with explicit Accessor, value function, and cycle detection.
template <class Accessor, adj_list::index_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, typename Accessor::template edge_t<G>>
[[nodiscard]] auto edges_topological_sort_safe(G& g, EVF&& evf)
      -> tl::expected<edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>,
                      adj_list::vertex_t<G>> {
  using view_type = edges_topological_sort_view<G, std::decay_t<EVF>, std::allocator<bool>, Accessor>;

  auto state =
        std::make_shared<topo_detail::topo_state<G, std::allocator<bool>, Accessor>>(g, std::allocator<bool>{}, true);

  if (state->has_cycle()) {
    return tl::unexpected(state->cycle_vertex().value());
  }

  return view_type(g, std::forward<EVF>(evf), std::move(state));
}

} // namespace graph::views
