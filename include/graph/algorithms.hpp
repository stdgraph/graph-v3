/**
 * @file algorithms.hpp
 * @brief Master include file for graph algorithms
 * 
 * This header provides convenient access to all graph algorithms.
 * Include this file to use the complete algorithm library, or include
 * specific algorithm headers for faster compilation.
 * 
 * @example
 * ```cpp
 * #include <graph/algorithms.hpp>
 * 
 * // All algorithms are now available
 * dijkstra_shortest_paths(g, source, distance, predecessor, weight);
 * ```
 * 
 * For faster compilation, include specific headers:
 * ```cpp
 * #include <graph/algorithm/dijkstra_shortest_paths.hpp>
 * ```
 */

#ifndef GRAPH_ALGORITHMS_HPP
#define GRAPH_ALGORITHMS_HPP

// Shortest Path Algorithms
#include "algorithm/dijkstra_shortest_paths.hpp"
#include "algorithm/bellman_ford.hpp"
#include "algorithm/bfs.hpp"

// Community Detection
#include "algorithm/label_propagation.hpp"

// Search Algorithms
#include "algorithm/dfs.hpp"

// Minimum Spanning Tree
#include "algorithm/prim.hpp"
#include "algorithm/kruskal.hpp"

// Connectivity
#include "algorithm/connected_components.hpp"
#include "algorithm/articulation_points.hpp"
#include "algorithm/biconnected_components.hpp"
#include "algorithm/scc.hpp"

// Link Analysis
#include "algorithm/jaccard.hpp"

// Topological Sort & DAG
#include "algorithm/topological_sort.hpp"

// Flow Algorithms
// #include "algorithm/ford_fulkerson.hpp"

/**
 * @defgroup graph_algorithms Graph Algorithms
 * @brief Standard graph algorithms for the graph-v3 library
 * 
 * This module provides efficient implementations of classic graph algorithms
 * that work with the graph-v3 container abstractions.
 * 
 * All algorithms:
 * - Use C++20 concepts and ranges
 * - Work with multiple container types via CPO interface
 * - Follow STL naming conventions
 * - Are thoroughly tested and benchmarked
 * - Include comprehensive documentation
 * 
 * @see docs/algorithms/README.md for complete documentation
 */

// Note: Uncomment includes above as algorithms are implemented

#endif // GRAPH_ALGORITHMS_HPP
