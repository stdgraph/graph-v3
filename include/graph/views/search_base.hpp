#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

namespace graph::views {

/// Search cancellation control for traversal algorithms
enum class cancel_search {
  continue_search, ///< Continue normal traversal
  cancel_branch,   ///< Skip current subtree/branch, continue with siblings
  cancel_all       ///< Stop entire search immediately
};

/// Visited tracking for search views (DFS/BFS/topological sort)
/// Uses vector<bool> for space efficiency with large graphs
template <class VId, class Alloc = std::allocator<bool>>
class visited_tracker {
  std::vector<bool, Alloc> visited_;

public:
  /// Construct tracker for a graph with num_vertices vertices
  explicit visited_tracker(std::size_t num_vertices, Alloc alloc = {}) : visited_(num_vertices, false, alloc) {}

  /// Check if a vertex has been visited
  [[nodiscard]] bool is_visited(VId id) const { return visited_[static_cast<std::size_t>(id)]; }

  /// Mark a vertex as visited
  void mark_visited(VId id) { visited_[static_cast<std::size_t>(id)] = true; }

  /// Reset all vertices to unvisited state
  void reset() { std::fill(visited_.begin(), visited_.end(), false); }

  /// Get the number of vertices being tracked
  [[nodiscard]] std::size_t size() const { return visited_.size(); }
};

} // namespace graph::views
