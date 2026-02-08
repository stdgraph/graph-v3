/**
 * @file adjacency_list_concepts.hpp
 * @brief Concepts for adjacency list graph structures
 * 
 * This file defines concepts for edges, vertices, and adjacency lists
 * that are used to constrain graph containers and algorithms.
 */

#pragma once

#include <concepts>
#include <ranges>
#include "detail/graph_cpo.hpp"

namespace graph::adj_list {

// =============================================================================
// Edge Concepts
// =============================================================================

/**
 * @brief Concept for edges that have a target vertex
 * 
 * A targeted_edge is an edge that can provide the target vertex ID and descriptor.
 * This is the most basic edge concept.
 * 
 * Requirements:
 * - target_id(g, e) must be valid (returns vertex ID)
 * - target(g, e) must be valid (returns vertex descriptor)
 * - e must be an edge_descriptor
 * 
 * Note: Return types are not constrained to allow better compiler error messages.
 * 
 * Examples:
 * - Simple edge list: vector<int> where each int is a target vertex ID
 * - Weighted edges: vector<pair<int, double>> where first is target
 * 
 * @tparam G Graph type
 * @tparam E Edge type (must be edge_descriptor)
 */
template<typename G, typename E>
concept targeted_edge = 
    is_edge_descriptor_v<std::remove_cvref_t<E>> &&
    requires(G& g, const E& e) {
        target_id(g, e);
        target(g, e);
    };

/**
 * @brief Concept for edges that have a source vertex
 * 
 * A sourced_edge is an edge that can provide the source vertex ID and descriptor.
 * This is used for bidirectional graphs and edge lists where edges
 * need to track their source.
 * 
 * Requirements:
 * - source_id(g, e) must be valid (returns vertex ID)
 * - source(g, e) must be valid (returns vertex descriptor)
 * - e must be an edge_descriptor
 * 
 * Note: Return types are not constrained to allow better compiler error messages.
 * 
 * Examples:
 * - Edge list: vector<pair<int, int>> where first is source, second is target
 * - Bidirectional edge: struct { int from; int to; }
 * 
 * @tparam G Graph type
 * @tparam E Edge type (must be edge_descriptor)
 */
template<typename G, typename E>
concept sourced_edge = 
    is_edge_descriptor_v<std::remove_cvref_t<E>> &&
    requires(G& g, const E& e) {
        source_id(g, e);
        source(g, e);
    };

/**
 * @brief Concept for edges that have both source and target vertices
 * 
 * A sourced_targeted_edge combines both sourced_edge and targeted_edge,
 * providing access to both the source and target vertex IDs.
 * 
 * Requirements:
 * - Must satisfy both targeted_edge and sourced_edge
 * - Both source_id(g, e) and target_id(g, e) must be valid
 * 
 * Examples:
 * - Full edge list: vector<tuple<int, int, double>> where elements are source, target, weight
 * - Bidirectional weighted edge: struct { int from; int to; double weight; }
 * 
 * @tparam G Graph type
 * @tparam E Edge type
 */
template<typename G, typename E>
concept sourced_targeted_edge = targeted_edge<G, E> && sourced_edge<G, E>;

// =============================================================================
// Edge Range Concepts
// =============================================================================

/**
 * @brief Concept for a forward range of targeted edges
 * 
 * A targeted_edge_range is a range where each element satisfies the
 * targeted_edge concept. This is used to represent the outgoing edges
 * from a vertex in an adjacency list.
 * 
 * Requirements:
 * - Must be a std::ranges::forward_range
 * - Range value type must satisfy targeted_edge
 * 
 * Examples:
 * - Adjacency list: vector<vector<int>> where inner vector is target IDs
 * - Weighted adjacency: vector<vector<pair<int, double>>> where pair is {target, weight}
 * 
 * @tparam R Range type
 * @tparam G Graph type (optional, for compatibility)
 */
template<typename R, typename G = void>
concept targeted_edge_range = 
    std::ranges::forward_range<R> &&
    targeted_edge<G, std::ranges::range_value_t<R>>;

/**
 * @brief Concept for a forward range of sourced and targeted edges
 * 
 * A sourced_targeted_edge_range is a range where each element satisfies
 * both the sourced_edge and targeted_edge concepts. This is used for
 * bidirectional graphs and edge lists where edges need to know both endpoints.
 * 
 * Requirements:
 * - Must be a std::ranges::forward_range
 * - Range value type must satisfy sourced_targeted_edge
 * 
 * Examples:
 * - Edge list: vector<tuple<int, int, double>> where elements are {source, target, weight}
 * - Bidirectional adjacency: where edges store both source and target explicitly
 * 
 * @tparam R Range type
 * @tparam G Graph type (optional, for compatibility)
 */
template<typename R, typename G = void>
concept sourced_targeted_edge_range = 
    std::ranges::forward_range<R> &&
    sourced_targeted_edge<G, std::ranges::range_value_t<R>>;

// =============================================================================
// Vertex Range Concepts
// =============================================================================

/**
 * @brief Concept for a graph with a forward range of vertices
 * 
 * A vertex_range is a graph where vertices can be iterated as a forward range,
 * with each element being a vertex descriptor.
 * 
 * Requirements:
 * - vertices(g) must return a std::ranges::forward_range
 * - vertices(g) must return a std::ranges::sized_range (size() available)
 * - Range value type must be a vertex_descriptor
 * 
 * Note: sized_range is required as a functional requirement even though
 * performance may be substandard for some containers (e.g., O(n) for map).
 * 
 * Note: The vertex_id(g, v) operation is expected to be available but not
 * checked in the concept to avoid circular dependencies and provide better
 * error messages when used incorrectly.
 * 
 * Note: forward_range has been chosen over bidirectional_range to allow
 * the use of std::unordered_map as a vertex container.
 * 
 * Examples:
 * - vertex_descriptor_view over std::vector<T>
 * - vertex_descriptor_view over std::map<K, V>
 * - vertex_descriptor_view over std::deque<T>
 * 
 * @tparam G Graph type
 */
template<typename G>
concept vertex_range = 
    std::ranges::forward_range<vertex_range_t<G>> &&
    std::ranges::sized_range<vertex_range_t<G>> &&
    is_vertex_descriptor_v<std::remove_cvref_t<std::ranges::range_value_t<vertex_range_t<G>>>>;

/**
 * @brief Concept for a graph with random access range of vertices
 * 
 * An index_vertex_range is a vertex_range where the underlying vertex container
 * supports random access, allowing O(1) access to any vertex by index.
 * 
 * Requirements:
 * - The vertex ID type must be integral
 * - Must satisfy vertex_range
 * - The underlying iterator of the vertex_descriptor_view must be a random_access_iterator
 * 
 * Note: We check the underlying iterator type, not the view itself, because
 * vertex_descriptor_view is always a forward_range (synthesizes descriptors on-the-fly)
 * but the underlying container may still support random access.
 * 
 * Examples:
 * - vertex_descriptor_view over std::vector<T> (index-based)
 * - vertex_descriptor_view over std::deque<T> (index-based)
 * 
 * Note: std::map-based graphs do NOT satisfy this concept as they
 * only provide bidirectional iteration, not random access.
 * 
 * @tparam G Graph type
 */
template<typename G>
concept index_vertex_range = 
    std::integral<vertex_id_t<G>> &&
    vertex_range<G> && 
    std::random_access_iterator<typename vertex_range_t<G>::vertex_desc::iterator_type>;

// =============================================================================
// Adjacency List Concepts
// =============================================================================

/**
 * @brief Concept for graphs with adjacency list structure
 * 
 * An adjacency_list is a graph where:
 * - Vertices can be iterated as a vertex_range (forward)
 * - Each vertex has outgoing edges as a targeted_edge_range
 * 
 * Requirements:
 * - vertices(g) returns a vertex_range
 * - edges(g, u) returns a targeted_edge_range for vertex u
 * - Supports vertex_id(g, u) for each vertex
 * - Supports target_id(g, e) and target(g, e) for each edge
 * 
 * Examples:
 * - std::vector<std::vector<int>> - vector-based adjacency list
 * - std::map<int, std::vector<int>> - map-based adjacency list
 * - std::deque<std::vector<pair<int, double>>> - weighted adjacency list
 * 
 * @tparam G Graph type
 */
template<typename G>
concept adjacency_list = 
    vertex_range<G> &&
    requires(G& g, vertex_t<G> u) {
        { edges(g, u) } -> targeted_edge_range<G>;
    };

/**
 * @brief Concept for graphs with index-based adjacency list structure
 * 
 * An index_adjacency_list is an adjacency_list where vertices support
 * random access via an index_vertex_range.
 * 
 * Requirements:
 * - Must satisfy adjacency_list
 * - vertices(g) returns an index_vertex_range (random access)
 * 
 * Examples:
 * - std::vector<std::vector<int>> - contiguous vector storage
 * - std::deque<std::vector<int>> - deque-based storage
 * 
 * Note: std::map-based graphs do NOT satisfy this concept.
 * 
 * @tparam G Graph type
 */
template<typename G>
concept index_adjacency_list = 
    adjacency_list<G> &&
    index_vertex_range<G>;

/**
 * @brief Concept for graphs with sourced adjacency list structure
 * 
 * A sourced_adjacency_list is an adjacency_list where edges also
 * provide source vertex information (sourced_targeted_edge_range).
 * 
 * Requirements:
 * - Must satisfy adjacency_list
 * - edges(g, u) returns a sourced_targeted_edge_range
 * - Supports source_id(g, e) and source(g, e) for each edge
 * 
 * Examples:
 * - Adjacency lists where edges know their source vertex
 * - Useful for bidirectional graph traversal
 * 
 * @tparam G Graph type
 */
template<typename G>
concept sourced_adjacency_list = 
    adjacency_list<G> &&
    requires(G& g, vertex_t<G> u) {
        { edges(g, u) } -> sourced_targeted_edge_range<G>;
    };

/**
 * @brief Concept for graphs with index-based sourced adjacency list structure
 * 
 * An index_sourced_adjacency_list combines the requirements of both
 * index_adjacency_list and sourced_adjacency_list.
 * 
 * Requirements:
 * - Must satisfy index_adjacency_list
 * - Must satisfy sourced_adjacency_list
 * 
 * Examples:
 * - std::vector<std::vector<edge_with_source>> where edges track their source
 * 
 * @tparam G Graph type
 */
template<typename G>
concept index_sourced_adjacency_list = 
    index_adjacency_list<G> &&
    sourced_adjacency_list<G>;

/**
 * @brief Concept for graphs with sorted adjacency lists.
 * 
 * A graph satisfies ordered_edges if the adjacency list for each vertex is sorted by
 * target vertex ID in ascending order. This property enables efficient set intersection
 * algorithms using linear merge operations.
 * 
 * Requirements:
 * - Must satisfy adjacency_list
 * - Edge range must be a forward_range
 * 
 * @tparam G Graph type
 * 
 * @note This is a semantic requirement that cannot be fully checked at compile time.
 *       The algorithm assumes adjacency lists are sorted by target_id in ascending order.
 *       Graph types using std::set, std::map, or similar ordered containers satisfy this.
 * 
 * @note Required for algorithms like triangle_count. Graphs with unsorted adjacency
 *       lists will produce incorrect results.
 * 
 * Examples:
 * - Graphs using std::set for edges (vos, uos, dos)
 * - Graphs using std::map for edges
 * - Any graph where edges are maintained in sorted order by target_id
 * 
 * Counter-examples:
 * - Graphs using std::vector without sorted order (vov)
 * - Graphs using std::unordered_set for edges (vous, mous)
 */
template<typename G>
concept ordered_edges = 
    adjacency_list<G> &&
    requires(G& g, vertex_id_t<G> u) {
        requires std::forward_iterator<decltype(std::ranges::begin(edges(g, u)))>;
    };


} // namespace graph::adj_list
