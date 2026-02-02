/**
 * @file views.hpp
 * @brief Unified header for all graph views
 * 
 * This master header includes all graph view implementations, providing
 * lazy, range-based access to graph elements using C++20 ranges and
 * structured bindings.
 * 
 * @par View Categories
 * 
 * **Basic Views:**
 * - vertexlist: View of all vertices
 * - incidence: View of edges from a vertex
 * - neighbors: View of neighbor vertices
 * - edgelist: View of all edges
 * 
 * **Search Views:**
 * - vertices_dfs: Depth-first vertex traversal
 * - edges_dfs: Depth-first edge traversal
 * - vertices_bfs: Breadth-first vertex traversal
 * - edges_bfs: Breadth-first edge traversal
 * - vertices_topological_sort: Topological order vertex traversal
 * - edges_topological_sort: Topological order edge traversal
 * 
 * **Range Adaptors:**
 * All views support C++20 pipe syntax via adaptors:
 * @code
 * auto g = make_graph();
 * 
 * // Basic views with pipe syntax
 * for (auto [v] : g | vertexlist()) { ... }
 * for (auto [e] : g | incidence(uid)) { ... }
 * for (auto [v] : g | neighbors(uid)) { ... }
 * for (auto [e] : g | edgelist()) { ... }
 * 
 * // Search views with pipe syntax
 * for (auto [v] : g | vertices_dfs(seed)) { ... }
 * for (auto [e] : g | edges_bfs(seed)) { ... }
 * for (auto [v] : g | vertices_topological_sort()) { ... }
 * 
 * // Chaining with standard range adaptors
 * for (auto id : g | vertexlist()
 *                  | std::views::transform(...)
 *                  | std::views::filter(...)) { ... }
 * @endcode
 * 
 * @par Value Functions
 * All views support optional value functions that transform the output:
 * @code
 * auto vvf = [](auto v) { return some_value; };
 * for (auto [v, val] : g | vertexlist(vvf)) { ... }
 * @endcode
 * 
 * @see graph::views::adaptors Namespace containing all adaptor objects
 */

#pragma once

// Basic views (vertexlist, incidence, neighbors, edgelist)
#include <graph/views/basic_views.hpp>

// Depth-first search views
#include <graph/views/dfs.hpp>

// Breadth-first search views
#include <graph/views/bfs.hpp>

// Topological sort views
#include <graph/views/topological_sort.hpp>

// Range adaptor closures for pipe syntax
#include <graph/views/adaptors.hpp>
