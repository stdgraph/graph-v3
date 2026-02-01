#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <graph/views/search_base.hpp>

namespace graph::views {

/// Concept for types that can be used as vertex value functions
/// Value functions must be invocable with a vertex descriptor and return a non-void type
template <class VVF, class VertexDescriptor>
concept vertex_value_function =
    std::invocable<VVF, VertexDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<VVF, VertexDescriptor>>);

/// Concept for types that can be used as edge value functions
/// Value functions must be invocable with an edge descriptor and return a non-void type
template <class EVF, class EdgeDescriptor>
concept edge_value_function =
    std::invocable<EVF, EdgeDescriptor> &&
    (!std::is_void_v<std::invoke_result_t<EVF, EdgeDescriptor>>);

/// Concept for search views (DFS/BFS/topological sort)
/// Search views provide depth(), size() accessors and cancel() control
template <class V>
concept search_view = requires(V& v, const V& cv) {
  { v.cancel() } -> std::convertible_to<cancel_search>;
  { cv.depth() } -> std::convertible_to<std::size_t>;
  { cv.size() } -> std::convertible_to<std::size_t>;
};

} // namespace graph::views
