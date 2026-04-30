/**
 * @file generators.hpp
 * @brief Convenience umbrella header for all graph generators.
 *
 * Include this single header to access all built-in graph generators:
 *   - erdos_renyi()     — Erdős–Rényi G(n, p) random graph
 *   - grid_2d()         — 2D grid with 4-connectivity
 *   - barabasi_albert() — preferential-attachment (scale-free)
 *   - path_graph()      — simple directed path
 *
 * All generators return a sorted std::vector<copyable_edge_t<VId, double>>
 * suitable for loading into any graph container via load_edges().
 */

#pragma once

#include <graph/generators/common.hpp>
#include <graph/generators/erdos_renyi.hpp>
#include <graph/generators/grid.hpp>
#include <graph/generators/barabasi_albert.hpp>
#include <graph/generators/path.hpp>
