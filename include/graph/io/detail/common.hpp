/**
 * @file detail/common.hpp
 * @brief Shared concepts and utilities for graph I/O headers.
 */

#pragma once

#include <graph/graph.hpp>

#include <format>
#include <string>
#include <string_view>

namespace graph::io::detail {

/// Detect whether T is formattable via std::format (C++20-safe).
template <class T>
concept formattable = requires(const T& v, std::format_context& ctx) {
  std::formatter<std::remove_cvref_t<T>, char>{}.format(v, ctx);
};

/// Detect whether vertex_value(g, u) is available for graph G.
template <class G>
concept has_vertex_value = requires(G& g, vertex_t<G> u) {
  graph::vertex_value(g, u);
};

/// Detect whether edge_value(g, uv) is available for graph G.
template <class G>
concept has_edge_value = requires(G& g, edge_t<G> e) {
  graph::edge_value(g, e);
};

} // namespace graph::io::detail
