/**
 * @file graph_concepts.hpp
 * @brief Core value function concepts for graph algorithms and views
 *
 * Defines vertex_value_function and edge_value_function — the fundamental concepts for
 * functions that extract computed values from vertex and edge descriptors. These concepts
 * require a const graph reference as the first parameter, enabling:
 *   - Stateless lambdas (empty capture list) for semiregular views and std::views chaining
 *   - Explicit graph access without capturing the graph in a closure
 *
 * These concepts are used by:
 *   - View factories (vertexlist, incidence, neighbors, edgelist, dfs, bfs, topological_sort)
 *   - Algorithm weight function concepts (basic_edge_weight_function subsumes edge_value_function)
 *
 * This header is intentionally lightweight (no graph type dependencies) so it can be included
 * by both view headers and algorithm headers without introducing circular dependencies.
 */

#pragma once

#include <concepts>
#include <type_traits>

namespace graph {

/// Concept for types that can be used as vertex value functions.
/// A vertex value function must be invocable with (const Graph&, VertexDescriptor) and
/// return a non-void type.
template <class VVF, class Graph, class VertexDescriptor>
concept vertex_value_function = std::invocable<VVF, const Graph&, VertexDescriptor> &&
                                (!std::is_void_v<std::invoke_result_t<VVF, const Graph&, VertexDescriptor>>);

/// Concept for types that can be used as edge value functions.
/// An edge value function must be invocable with (const Graph&, EdgeDescriptor) and
/// return a non-void type. This is the base requirement that basic_edge_weight_function
/// refines with additional arithmetic constraints.
template <class EVF, class Graph, class EdgeDescriptor>
concept edge_value_function = std::invocable<EVF, const Graph&, EdgeDescriptor> &&
                              (!std::is_void_v<std::invoke_result_t<EVF, const Graph&, EdgeDescriptor>>);

} // namespace graph
