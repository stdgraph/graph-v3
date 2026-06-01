/**
 * @file generators.hpp
 * @brief Convenience umbrella header for all graph generators.
 *
 * Include this single header to access all built-in graph generators:
 *   - erdos_renyi()      — Erdős–Rényi G(n, p) random graph
 *   - erdos_renyi_gnm()  — Erdős–Rényi G(n, m) fixed-edge-count random graph
 *   - grid_2d()          — 2D grid with 4-connectivity
 *   - barabasi_albert()  — preferential-attachment (scale-free)
 *   - path_graph()       — simple directed path
 *   - complete_graph()   — complete graph K(n)
 *   - watts_strogatz()   — small-world ring lattice with rewiring
 *   - rmat()             — R-MAT recursive-matrix (Graph500-style)
 *   - plod()             — power-law out-degree
 *   - ssca()             — SSCA#2 clique-based benchmark graph
 *
 * All generators return a sorted std::vector<copyable_edge_t<VId, double>>
 * suitable for loading into any graph container via load_edges().
 */

#pragma once

#include <graph/generators/common.hpp>
#include <graph/generators/erdos_renyi.hpp>
#include <graph/generators/gnm.hpp>
#include <graph/generators/grid.hpp>
#include <graph/generators/barabasi_albert.hpp>
#include <graph/generators/path.hpp>
#include <graph/generators/complete.hpp>
#include <graph/generators/watts_strogatz.hpp>
#include <graph/generators/rmat.hpp>
#include <graph/generators/plod.hpp>
#include <graph/generators/ssca.hpp>
