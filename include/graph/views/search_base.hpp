#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace graph::views {

/// Search cancellation control for traversal algorithms
enum class cancel_search {
  continue_search, ///< Continue normal traversal
  cancel_branch,   ///< Skip current subtree/branch, continue with siblings
  cancel_all       ///< Stop entire search immediately
};

/// Visited tracking for search views (DFS/BFS/topological sort).
///
/// The template parameter @c Vertex is the value passed to @c mark_visited /
/// @c is_visited.  It is typically either a vertex descriptor (preferred —
/// avoids re-deriving the id at each call) or a vertex id.  Two storage
/// strategies are selected at compile time:
///
///   - Bitset (`std::vector<bool>` indexed by integer):
///       * @c Vertex is an integral type (a vertex id), OR
///       * @c Vertex has a member @c value() returning an integral type
///         (e.g. a vertex descriptor wrapping a @c size_t index for
///         vector-/deque-backed graphs).
///   - Hash set (`std::unordered_set<Vertex>`):
///       * Anything else, provided @c Vertex is hashable and equality-comparable.
///         Used for sparse / non-numeric ids such as @c std::string, or for
///         vertex descriptors wrapping a map / unordered_map iterator.
///
/// The bitset path is dense and cache-friendly; the hash path avoids
/// pre-sizing and works for non-contiguous id spaces.
template <class Vertex, class Alloc = std::allocator<bool>>
class visited_tracker {
  // Detect whether Vertex can be used directly as a bitset index.
  template <class T>
  static constexpr bool has_integral_value_v = requires(const T& t) {
    { t.value() } -> std::integral;
  };

  static constexpr bool use_bitset = std::is_integral_v<Vertex> || has_integral_value_v<Vertex>;

  static constexpr std::size_t to_index(const Vertex& v) noexcept {
    if constexpr (std::is_integral_v<Vertex>)
      return static_cast<std::size_t>(v);
    else
      return static_cast<std::size_t>(v.value());
  }

  using set_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Vertex>;
  using storage_type =
        std::conditional_t<use_bitset,
                           std::vector<bool, Alloc>,
                           std::unordered_set<Vertex, std::hash<Vertex>, std::equal_to<Vertex>, set_alloc>>;

  storage_type visited_;

  static storage_type make_storage(std::size_t num_vertices, Alloc alloc) {
    if constexpr (use_bitset) {
      return storage_type(num_vertices, false, alloc);
    } else {
      (void)num_vertices;
      return storage_type(0, std::hash<Vertex>{}, std::equal_to<Vertex>{}, set_alloc(alloc));
    }
  }

public:
  /// Construct tracker for a graph with @p num_vertices vertices.
  /// @p num_vertices is used to pre-size the bitset; ignored for the hash variant.
  explicit visited_tracker(std::size_t num_vertices, Alloc alloc = {})
        : visited_(make_storage(num_vertices, alloc)) {}

  /// Check if a vertex has been visited
  [[nodiscard]] bool is_visited(const Vertex& v) const {
    if constexpr (use_bitset)
      return visited_[to_index(v)];
    else
      return visited_.contains(v);
  }

  /// Mark a vertex as visited
  void mark_visited(const Vertex& v) {
    if constexpr (use_bitset)
      visited_[to_index(v)] = true;
    else
      visited_.insert(v);
  }

  /// Mark a vertex as not visited (used by recursion-stack tracking)
  void unmark_visited(const Vertex& v) {
    if constexpr (use_bitset)
      visited_[to_index(v)] = false;
    else
      visited_.erase(v);
  }

  /// Reset all vertices to unvisited state
  void reset() {
    if constexpr (use_bitset)
      std::fill(visited_.begin(), visited_.end(), false);
    else
      visited_.clear();
  }

  /// Get the number of vertices tracked (bitset variant) or marked visited (hash variant).
  [[nodiscard]] std::size_t size() const { return visited_.size(); }
};

} // namespace graph::views
