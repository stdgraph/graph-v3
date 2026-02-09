/**
 * @file adjacency_list_traits.hpp
 * @brief Traits for querying adjacency list graph capabilities
 * 
 * This file defines traits that can be used to query compile-time properties
 * of graph types, such as whether they support degree(), find_vertex(),
 * find_vertex_edge(), or contains_edge() operations.
 */

#pragma once

#include <concepts>
#include <type_traits>
#include "detail/graph_cpo.hpp"
#include "adjacency_list_concepts.hpp"

namespace graph::adj_list {

// =============================================================================
// Degree Trait
// =============================================================================

namespace detail {
    // Helper to detect if degree(g, u) is valid
    template<typename G>
    concept has_degree_impl = requires(G& g, vertex_t<G> u) {
        { degree(g, u) } -> std::integral;
    };
    
    // Helper to detect if degree(g, uid) is valid
    template<typename G>
    concept has_degree_uid_impl = requires(G& g, vertex_id_t<G> uid) {
        { degree(g, uid) } -> std::integral;
    };
}

/**
 * @brief Trait to check if a graph supports degree operations
 * 
 * A graph has degree support if both degree(g, u) and degree(g, uid) are valid.
 * 
 * Requirements:
 * - degree(g, u) must return an integral type
 * - degree(g, uid) must return an integral type
 * 
 * @tparam G Graph type
 */
template<typename G>
concept has_degree = detail::has_degree_impl<G> && detail::has_degree_uid_impl<G>;

// Convenience variable template
template<typename G>
inline constexpr bool has_degree_v = has_degree<G>;

// =============================================================================
// Find Vertex Trait
// =============================================================================

namespace detail {
    // Helper to detect if find_vertex(g, uid) is valid and returns correct type
    template<typename G>
    concept has_find_vertex_impl = requires(G& g, vertex_id_t<G> uid) {
        { find_vertex(g, uid) } -> std::same_as<vertex_t<G>>;
    };
}

/**
 * @brief Trait to check if a graph supports find_vertex operation
 * 
 * A graph has find_vertex support if find_vertex(g, uid) returns a vertex descriptor.
 * 
 * Requirements:
 * - find_vertex(g, uid) must return vertex_t<G>
 * 
 * @tparam G Graph type
 */
template<typename G>
concept has_find_vertex = detail::has_find_vertex_impl<G>;

// Convenience variable template
template<typename G>
inline constexpr bool has_find_vertex_v = has_find_vertex<G>;

// =============================================================================
// Find Vertex Edge Trait
// =============================================================================

namespace detail {
    // Helper to detect if find_vertex_edge(g, u, v) is valid
    template<typename G>
    concept has_find_vertex_edge_uv_impl = requires(G& g, vertex_t<G> u, vertex_t<G> v) {
        { find_vertex_edge(g, u, v) } -> std::same_as<edge_t<G>>;
    };
    
    // Helper to detect if find_vertex_edge(g, u, vid) is valid
    template<typename G>
    concept has_find_vertex_edge_uvid_impl = requires(G& g, vertex_t<G> u, vertex_id_t<G> vid) {
        { find_vertex_edge(g, u, vid) } -> std::same_as<edge_t<G>>;
    };
    
    // Helper to detect if find_vertex_edge(g, uid, vid) is valid
    template<typename G>
    concept has_find_vertex_edge_uidvid_impl = requires(G& g, vertex_id_t<G> uid, vertex_id_t<G> vid) {
        { find_vertex_edge(g, uid, vid) } -> std::same_as<edge_t<G>>;
    };
}

/**
 * @brief Trait to check if a graph supports find_vertex_edge operations
 * 
 * A graph has find_vertex_edge support if all three overloads are valid:
 * - find_vertex_edge(g, u, v)
 * - find_vertex_edge(g, u, vid)
 * - find_vertex_edge(g, uid, vid)
 * 
 * Requirements:
 * - All overloads must return edge_t<G>
 * 
 * @tparam G Graph type
 */
template<typename G>
concept has_find_vertex_edge = 
    detail::has_find_vertex_edge_uv_impl<G> && 
    detail::has_find_vertex_edge_uvid_impl<G> && 
    detail::has_find_vertex_edge_uidvid_impl<G>;

// Convenience variable template
template<typename G>
inline constexpr bool has_find_vertex_edge_v = has_find_vertex_edge<G>;

// =============================================================================
// Contains Edge Trait
// =============================================================================

namespace detail {
    // Helper to detect if contains_edge(g, u, v) is valid
    template<typename G, typename V>
    concept has_contains_edge_uv_impl = requires(G& g, V u, V v) {
        { contains_edge(g, u, v) } -> std::same_as<bool>;
    };
    
    // Helper to detect if contains_edge(g, uid, vid) is valid with vertex IDs
    template<typename G, typename V>
    concept has_contains_edge_uidvid_impl = 
        std::same_as<std::remove_cvref_t<V>, vertex_id_t<G>> &&
        requires(G& g, V uid, V vid) {
            { contains_edge(g, uid, vid) } -> std::same_as<bool>;
        };
}

/**
 * @brief Trait to check if a graph supports contains_edge operations
 * 
 * A graph has contains_edge support with vertex type V if both overloads are valid:
 * - contains_edge(g, u, v) where u, v are of type V
 * - contains_edge(g, uid, vid) if V is vertex_id_t<G>
 * 
 * Requirements:
 * - Both overloads must return bool
 * 
 * @tparam G Graph type
 * @tparam V Vertex or vertex ID type
 */
template<typename G, typename V>
concept has_contains_edge = 
    detail::has_contains_edge_uv_impl<G, V> && 
    (std::same_as<std::remove_cvref_t<V>, vertex_t<G>> || 
     detail::has_contains_edge_uidvid_impl<G, V>);

// Convenience variable template
template<typename G, typename V>
inline constexpr bool has_contains_edge_v = has_contains_edge<G, V>;

// =============================================================================
// Combined Trait Queries
// =============================================================================

/**
 * @brief Check if a graph supports all basic query operations
 * 
 * A graph has basic queries if it supports:
 * - degree operations
 * - find_vertex operation
 * - find_vertex_edge operations
 * 
 * @tparam G Graph type
 */
template<typename G>
concept has_basic_queries = 
    has_degree<G> && 
    has_find_vertex<G> && 
    has_find_vertex_edge<G>;

// Convenience variable template
template<typename G>
inline constexpr bool has_basic_queries_v = has_basic_queries<G>;

/**
 * @brief Check if a graph supports all query operations including contains_edge
 * 
 * A graph has full queries if it supports:
 * - All basic queries (degree, find_vertex, find_vertex_edge)
 * - contains_edge with vertex descriptors
 * 
 * @tparam G Graph type
 */
template<typename G>
concept has_full_queries = 
    has_basic_queries<G> && 
    has_contains_edge<G, vertex_t<G>>;

// Convenience variable template
template<typename G>
inline constexpr bool has_full_queries_v = has_full_queries<G>;

} // namespace graph::adj_list
