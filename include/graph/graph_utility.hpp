/**
 * @file graph_utility.hpp
 * @brief Utility functions and CPOs for graph operations
 * 
 * This file will contain common utility CPOs such as:
 * - vertex_id(g, v) - Get vertex ID
 * - num_vertices(g) - Get number of vertices
 * - num_edges(g) - Get number of edges
 * - vertices(g) - Get vertex range
 * - edges(g) - Get edge range
 * - out_edges(g, v) - Get outgoing edges from vertex
 */

#pragma once

#include <graph/descriptor.hpp>
#include <concepts>
#include <ranges>

namespace graph::adj_list {

// CPO implementation details are in graph/detail/graph_cpo.hpp
// The _Choice_t struct and CPO implementations are defined there.

// Public CPO instances will be defined here
// inline namespace _cpo_instances {
//     inline constexpr _cpo_impls::_vertex_id::_fn vertex_id{};
//     inline constexpr _cpo_impls::_num_vertices::_fn num_vertices{};
//     // ... etc
// }

} // namespace graph::adj_list
