/**
 * @file graph.hpp
 * @brief Main header file for the graph library with graph concepts and traits
 * 
 * This header provides the complete graph library interface including:
 * - Core descriptor types and concepts
 * - Graph concepts and traits
 * - Graph information structures
 * - Basic containers (edgelist)
 * - Graph views (vertexlist, edgelist, neighbors, incidence, BFS, DFS, topological sort)
 * - Graph containers (when implemented)
 * - Graph algorithms (when implemented)
 * 
 * Include this file to access the full graph library functionality.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <iterator>

// Core descriptor types and concepts (adj_list namespace)
#include <graph/adj_list/descriptor.hpp>
#include <graph/adj_list/descriptor_traits.hpp>
#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/vertex_descriptor_view.hpp>
#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/adj_list/edge_descriptor_view.hpp>

// Graph information and utilities (shared between adj_list and edge_list)
#include <graph/graph_info.hpp>

// Edge list interface
#include <graph/edge_list/edge_list_traits.hpp>
#include <graph/edge_list/edge_list_descriptor.hpp>
#include <graph/edge_list/edge_list.hpp>

// Adjacency list interface
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/adj_list/adjacency_list_traits.hpp>
#include <graph/adj_list/graph_utility.hpp>

// Detail headers
#include <graph/detail/graph_using.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>

// Views
#include <graph/views/view_concepts.hpp>
#include <graph/views/adaptors.hpp>
#include <graph/views/basic_views.hpp>
#include <graph/views/vertexlist.hpp>
#include <graph/views/edgelist.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/incidence.hpp>
#include <graph/views/bfs.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/topological_sort.hpp>

// Future: Container implementations will be included here
// #include <graph/container/adjacency_list.hpp>
// #include <graph/container/compressed_sparse_row.hpp>

// Future: Algorithm implementations will be included here
// #include <graph/algorithm/breadth_first_search.hpp>
// #include <graph/algorithm/depth_first_search.hpp>
// #include <graph/algorithm/dijkstra_shortest_paths.hpp>

/**
 * @namespace graph
 * @brief Root namespace for the graph library
 * 
 * All graph library types, functions, and concepts are defined within this namespace.
 * The library follows C++20 standards and provides customization point objects (CPOs)
 * for all graph operations, allowing adaptation to existing graph data structures.
 * 
 * Namespace Organization:
 * - graph::                   # Root namespace for algorithms, common types, info structures, shared CPOs
 * - graph::adj_list::         # Adjacency list abstractions (CPOs, descriptors, concepts, traits)
 * - graph::edge_list::        # Edge list abstractions (concepts, traits, descriptors)
 * - graph::views::            # Graph views (vertexlist, edgelist, neighbors, BFS, DFS, etc.)
 * - graph::container::        # Concrete graph containers
 * - graph::detail::           # Internal implementation details
 * - graph::experimental::     # Unstable features
 */

// Import adj_list types and CPOs into graph namespace for backward compatibility
namespace graph {
    // Descriptor types
    using adj_list::vertex_descriptor;
    using adj_list::edge_descriptor;
    using adj_list::vertex_descriptor_view;
    using adj_list::edge_descriptor_view;
    
    // Descriptor concepts and traits
    using adj_list::descriptor_type;
    using adj_list::vertex_descriptor_type;
    using adj_list::edge_descriptor_type;
    using adj_list::vertex_iterator;
    using adj_list::edge_iterator;
    using adj_list::is_descriptor_v;
    using adj_list::is_vertex_descriptor_v;
    using adj_list::is_edge_descriptor_v;
    using adj_list::is_vertex_descriptor_view_v;
    using adj_list::is_edge_descriptor_view_v;
    
    // Adjacency list concepts
    using adj_list::edge;
    using adj_list::vertex;
    using adj_list::vertex_edge_range;
    using adj_list::vertex_range;
    using adj_list::index_vertex_range;
    using adj_list::adjacency_list;
    using adj_list::index_adjacency_list;
    using adj_list::ordered_vertex_edges;
    
    // Adjacency list traits
    using adj_list::has_degree;
    using adj_list::has_degree_v;
    using adj_list::has_find_vertex;
    using adj_list::has_find_vertex_v;
    using adj_list::has_find_vertex_edge;
    using adj_list::has_find_vertex_edge_v;
    using adj_list::has_contains_edge;
    using adj_list::has_contains_edge_v;
    using adj_list::define_unordered_edge;
    using adj_list::define_unordered_edge_v;
    using adj_list::has_basic_queries;
    using adj_list::has_basic_queries_v;
    using adj_list::has_full_queries;
    using adj_list::has_full_queries_v;
    
    // CPOs (Customization Point Objects)
    using adj_list::vertices;
    using adj_list::vertex_id;
    using adj_list::find_vertex;
    using adj_list::edges;
    using adj_list::target_id;
    using adj_list::target;
    using adj_list::num_vertices;
    using adj_list::num_edges;
    using adj_list::degree;
    using adj_list::find_vertex_edge;
    using adj_list::contains_edge;
    using adj_list::has_edge;
    using adj_list::vertex_value;
    using adj_list::edge_value;
    using adj_list::graph_value;
    using adj_list::source_id;
    using adj_list::source;
    using adj_list::partition_id;
    using adj_list::num_partitions;
    
    // Type aliases
    using adj_list::vertex_range_t;
    using adj_list::vertex_iterator_t;
    using adj_list::vertex_t;
    using adj_list::vertex_id_t;
    using adj_list::vertex_edge_range_t;
    using adj_list::vertex_edge_iterator_t;
    using adj_list::edge_t;
}
