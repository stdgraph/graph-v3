#pragma once

/**
 * @file basic_views.hpp
 * @brief Convenience header including all basic graph views.
 *
 * This header provides a single include for all basic (non-search) graph views:
 * - vertexlist: Iterate over vertices with optional value function
 * - incidence: Iterate over edges incident to vertices
 * - neighbors: Iterate over neighboring vertices
 * - edgelist: Iterate over all edges (adjacency_list or edge_list)
 */

#include <graph/views/vertexlist.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/edgelist.hpp>
