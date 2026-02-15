#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <graph/graph_concepts.hpp>
#include <graph/views/search_base.hpp>

namespace graph::views {

// Re-export from graph namespace so existing view code using graph::views::vertex_value_function
// and graph::views::edge_value_function continues to work without modification.
using graph::vertex_value_function;
using graph::edge_value_function;

/// Concept for search views (DFS/BFS) that provide depth tracking and cancel control.
/// Topological sort views support cancel() and num_visited() but not depth()
/// (flat ordering has no tree structure), so they do not satisfy this concept.
/// Note: num_visited() is used instead of size() to avoid satisfying
/// std::ranges::sized_range, which would break std::views::take and
/// other size-aware adaptors (they'd see size()==0 before iteration).
template <class V>
concept search_view = requires(V& v, const V& cv) {
  { v.cancel() } -> std::convertible_to<cancel_search>;
  { cv.depth() } -> std::convertible_to<std::size_t>;
  { cv.num_visited() } -> std::convertible_to<std::size_t>;
};

} // namespace graph::views
