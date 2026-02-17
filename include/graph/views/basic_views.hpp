#pragma once

/**
 * @file basic_views.hpp
 * @brief Convenience header including all basic graph views.
 *
 * This header provides a single include for all basic (non-search) graph views:
 * - vertexlist / basic_vertexlist: Iterate over vertices with optional value function
 * - incidence / basic_incidence: Iterate over edges incident to vertices
 * - neighbors / basic_neighbors: Iterate over neighboring vertices
 * - edgelist / basic_edgelist: Iterate over all edges (adjacency_list or edge_list)
 *
 * The basic_ variants return simplified bindings (ids only, no descriptors),
 * while the standard variants include full vertex/edge descriptors.
 *
 * Pipe adaptor objects are available in graph::views::adaptors (see adaptors.hpp).
 */

#include <graph/views/vertexlist.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/edgelist.hpp>
