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
 * @brief Concept for edge descriptors
 * 
 * An edge is an edge descriptor that provides access to both source and target vertices.
 * All edge descriptors in the graph library provide these operations.
 * 
 * Requirements:
 * - e must be an edge_descriptor
 * - source_id(g, e) must be valid (returns source vertex ID)
 * - source(g, e) must be valid (returns source vertex descriptor)
 * - target_id(g, e) must be valid (returns target vertex ID)
 * - target(g, e) must be valid (returns target vertex descriptor)
 * 
 * Note: Return types are not constrained to allow better compiler error messages.
 * 
 * Examples:
 * - Edge in adjacency list: edge_descriptor<vector<vector<int>>::iterator, int>
 * - Edge in edge list: edge_descriptor<vector<tuple<int, int, double>>::iterator>
 * - Weighted edge: edge_descriptor with value access
 * 
 * @tparam G Graph type
 * @tparam E Edge type (must be edge_descriptor)
 */
template <class G, class E>
concept edge = is_edge_descriptor_v<std::remove_cvref_t<E>> && requires(G& g, const E& e) {
  source_id(g, e);
  source(g, e);
  target_id(g, e);
  target(g, e);
};

// =============================================================================
// Edge Range Concepts
// =============================================================================

/**
 * @brief Concept for a forward range of outgoing edges
 * 
 * An out_edge_range is a range where each element satisfies the edge concept.
 * This is used to represent the outgoing edges from a vertex in an adjacency list,
 * or all edges in an edge list.
 * 
 * Requirements:
 * - Must be a std::ranges::forward_range
 * - Range value type must satisfy the edge concept
 * 
 * Examples:
 * - Adjacency list: vector<vector<int>> where inner vector contains edges
 * - Edge list: vector<tuple<int, int, double>> where elements are edges
 * - Weighted adjacency: vector<vector<pair<int, double>>> where pairs are edges
 * 
 * @tparam R Range type
 * @tparam G Graph type
 */
template <class R, class G>
concept out_edge_range = std::ranges::forward_range<R> && edge<G, std::ranges::range_value_t<R>>;

// =============================================================================
// Vertex Concepts
// =============================================================================

/**
 * @brief Concept for vertex descriptors
 * 
 * A vertex is a vertex descriptor that provides access to the vertex ID
 * and supports looking up vertices by ID.
 * 
 * Requirements:
 * - uv must be a vertex_descriptor
 * - vertex_id(g, uv) must be valid (returns vertex ID)
 * - find_vertex(g, uid) must be valid (returns vertex descriptor for given ID)
 * 
 * Note: Return types are not constrained to allow better compiler error messages.
 * 
 * Examples:
 * - Vertex in vector-based graph: vertex_descriptor<vector<T>::iterator>
 * - Vertex in map-based graph: vertex_descriptor<map<K, V>::iterator>
 * 
 * @tparam G Graph type
 * @tparam V Vertex type (must be vertex_descriptor)
 */
template <class G, class V>
concept vertex = is_vertex_descriptor_v<std::remove_cvref_t<V>> && requires(G& g, const V& u, vertex_id_t<G> uid) {
  vertex_id(g, u);
  find_vertex(g, uid);
};

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
 * - Range value type must satisfy the vertex concept
 * 
 * Note: sized_range is required as a functional requirement even though
 * performance may be substandard for some containers (e.g., O(n) for map).
 * 
 * Note: forward_range has been chosen over bidirectional_range to allow
 * the use of std::unordered_map as a vertex container.
 * 
 * Examples:
 * - vertex_descriptor_view over std::vector<T>
 * - vertex_descriptor_view over std::map<K, V>
 * - vertex_descriptor_view over std::deque<T>
 * 
 * @tparam R Range type
 * @tparam G Graph type
 */
template <class R, class G>
concept vertex_range = std::ranges::forward_range<R> && std::ranges::sized_range<R> &&
                       vertex<G, std::remove_cvref_t<std::ranges::range_value_t<R>>>;

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
template <class G>
concept index_vertex_range =
      requires(G& g) {
        { vertices(g) } -> vertex_range<G>;
      } && std::integral<vertex_id_t<G>> &&
      std::random_access_iterator<typename vertex_range_t<G>::vertex_desc::iterator_type>;

// =============================================================================
// Adjacency List Concepts
// =============================================================================

/**
 * @brief Concept for graphs with adjacency list structure
 * 
 * An adjacency_list is a graph where:
 * - Vertices can be iterated as a vertex_range (forward)
 * - Each vertex has outgoing edges as an out_edge_range
 * 
 * Requirements:
 * - vertices(g) returns a vertex_range
 * - out_edges(g, u) returns an out_edge_range for vertex u
 * - Supports vertex_id(g, u) for each vertex
 * - Supports source_id(g, e), source(g, e), target_id(g, e), and target(g, e) for each edge
 * 
 * Examples:
 * - std::vector<std::vector<int>> - vector-based adjacency list
 * - std::map<int, std::vector<int>> - map-based adjacency list
 * - std::deque<std::vector<pair<int, double>>> - weighted adjacency list
 * 
 * @tparam G Graph type
 */
template <class G>
concept adjacency_list = requires(G& g, vertex_t<G> u) {
  { vertices(g) } -> vertex_range<G>;
  { out_edges(g, u) } -> out_edge_range<G>;
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
template <class G>
concept index_adjacency_list = adjacency_list<G> && index_vertex_range<G>;

/**
 * @brief Concept for graphs with sorted adjacency lists.
 * 
 * A graph satisfies ordered_vertex_edges if the adjacency list for each vertex is sorted by
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
template <class G>
concept ordered_vertex_edges = adjacency_list<G> && requires(G& g, vertex_t<G> u) {
  requires std::forward_iterator<decltype(std::ranges::begin(out_edges(g, u)))>;
};

// =============================================================================
// Incoming Edge Concepts
// =============================================================================

/**
 * @brief Concept for a forward range of incoming edges
 * 
 * An in_edge_range is a range where each element satisfies the edge concept.
 * This is used to represent incoming edges to a vertex in a bidirectional graph.
 * The incoming edge descriptor type (in_edge_t<G>) may differ from the outgoing
 * edge descriptor type (edge_t<G>), but both must support the full edge interface
 * (source_id, source, target_id, target).
 * 
 * Requirements:
 * - Must be a std::ranges::forward_range
 * - Range value type must satisfy the edge concept
 * 
 * @tparam R Range type
 * @tparam G Graph type
 */
template <class R, class G>
concept in_edge_range = std::ranges::forward_range<R> && edge<G, std::ranges::range_value_t<R>>;

/**
 * @brief Concept for graphs with bidirectional adjacency list structure
 * 
 * A bidirectional_adjacency_list is an adjacency_list that also provides
 * access to incoming edges for each vertex. This enables traversing the
 * graph in reverse (from targets back to sources).
 * 
 * Requirements:
 * - Must satisfy adjacency_list
 * - in_edges(g, u) must return an in_edge_range
 * - source_id(g, ie) must return a value convertible to vertex_id_t<G>
 *   for incoming edge elements
 * 
 * Note: source_id on an incoming edge returns the ID of the vertex from
 * which the edge originates (the "source" in the original directed sense).
 * 
 * @tparam G Graph type
 */
template <class G>
concept bidirectional_adjacency_list =
      adjacency_list<G> &&
      requires(G& g, vertex_t<G> u, in_edge_t<G> ie) {
        { in_edges(g, u) } -> in_edge_range<G>;
        { source_id(g, ie) } -> std::convertible_to<vertex_id_t<G>>;
      };

/**
 * @brief Concept for bidirectional graphs with index-based vertex access
 * 
 * An index_bidirectional_adjacency_list combines bidirectional traversal
 * with O(1) index-based vertex lookup.
 * 
 * Requirements:
 * - Must satisfy bidirectional_adjacency_list
 * - Must satisfy index_vertex_range (random access vertices)
 * 
 * @tparam G Graph type
 */
template <class G>
concept index_bidirectional_adjacency_list =
      bidirectional_adjacency_list<G> && index_vertex_range<G>;



} // namespace graph::adj_list
