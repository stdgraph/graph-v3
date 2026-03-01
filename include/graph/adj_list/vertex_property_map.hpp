/**
 * @file vertex_property_map.hpp
 * @brief Per-vertex property map abstraction for graph algorithms
 *
 * Provides a uniform interface for storing per-vertex data regardless of whether
 * the graph uses index-based (vector, deque) or key-based (map, unordered_map)
 * vertex containers.
 *
 * Key components:
 * - vertex_property_map<G, T>: Type alias — vector<T> for index graphs, unordered_map<VId, T> for mapped
 * - make_vertex_property_map: Factory functions (eager and lazy initialization)
 * - vertex_property_map_contains: Test whether a vertex ID has an entry
 * - vertex_property_map_get: Read with default fallback (no insertion for unordered_map)
 */

#pragma once

#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/views/vertexlist.hpp>
#include <unordered_map>
#include <vector>

namespace graph::adj_list {

// =============================================================================
// vertex_property_map type alias
// =============================================================================

/**
 * @brief Per-vertex associative container: vector<T> for index graphs,
 *        unordered_map<vertex_id_t<G>, T> for mapped graphs.
 *
 * @tparam G Graph type
 * @tparam T Value type stored per vertex
 */
template <class G, class T>
using vertex_property_map = std::conditional_t<
    index_vertex_range<G>,
    std::vector<T>,
    std::unordered_map<vertex_id_t<G>, T>>;

// =============================================================================
// Factory functions
// =============================================================================

/**
 * @brief Eager initialization: create a vertex_property_map with every vertex pre-populated.
 *
 * For index graphs: vector<T>(N, init_value).  O(V) always.
 * For mapped graphs: unordered_map with all keys inserted.  O(V).
 *
 * Use when the algorithm reads all vertices before writing (e.g. component labels).
 *
 * @tparam G Graph type
 * @tparam T Value type
 * @param g The graph
 * @param init_value Initial value for every vertex entry
 * @return A vertex_property_map<G, T> with all vertices pre-populated
 */
template <class G, class T>
constexpr auto make_vertex_property_map(const G& g, const T& init_value) {
  if constexpr (index_vertex_range<G>) {
    return std::vector<T>(num_vertices(g), init_value);
  } else {
    std::unordered_map<vertex_id_t<G>, T> m;
    m.reserve(static_cast<size_t>(num_vertices(g)));
    for (auto&& [uid, u] : graph::views::vertexlist(g)) {
      m.emplace(uid, init_value);
    }
    return m;
  }
}

/**
 * @brief Lazy initialization: create an empty vertex_property_map with capacity hint.
 *
 * For index graphs: vector<T>(N) with default-constructed values (dense, O(V)).
 * For mapped graphs: empty unordered_map with reserved buckets (O(1) until use).
 *
 * Use with vertex_property_map_get(m, uid, default_val) when absence has a semantic meaning
 * (e.g. infinity for distances, White for DFS color, false for visited).
 *
 * @tparam G Graph type
 * @tparam T Value type
 * @param g The graph
 * @return A vertex_property_map<G, T> — sized for index graphs, empty+reserved for mapped
 */
template <class G, class T>
constexpr auto make_vertex_property_map(const G& g) {
  if constexpr (index_vertex_range<G>) {
    return std::vector<T>(num_vertices(g));
  } else {
    std::unordered_map<vertex_id_t<G>, T> m;
    m.reserve(static_cast<size_t>(num_vertices(g)));
    return m;
  }
}

// =============================================================================
// Access helpers
// =============================================================================

/**
 * @brief Test whether a vertex ID has an entry in the map.
 *
 * For index graphs (vector): always returns true — all indices in [0, size) are valid.
 * For mapped graphs (unordered_map): calls m.contains(uid).
 *
 * @tparam Map Container type (vector or unordered_map)
 * @tparam Key Vertex ID type
 * @param m The vertex map
 * @param uid The vertex ID to check
 * @return true if the vertex has an entry
 */
template <class Map, class Key>
constexpr bool vertex_property_map_contains(const Map& m, const Key& uid) {
  if constexpr (std::ranges::random_access_range<Map>) {
    return true; // vector: all indices in [0, size) are valid
  } else {
    return m.contains(uid);
  }
}

/**
 * @brief Read a value from a vertex map with a default fallback, without insertion.
 *
 * For index graphs (vector): returns m[uid] (always valid).
 * For mapped graphs (unordered_map): returns the mapped value if present,
 * otherwise returns default_val — no entry is inserted.
 *
 * @tparam Map Container type (vector or unordered_map)
 * @tparam Key Vertex ID type
 * @tparam T Default value type
 * @param m The vertex map (const — no modification)
 * @param uid The vertex ID to look up
 * @param default_val Value to return if uid is not present
 * @return The stored value or default_val
 */
template <class Map, class Key, class T>
constexpr auto vertex_property_map_get(const Map& m, const Key& uid, const T& default_val) {
  if constexpr (std::ranges::random_access_range<Map>) {
    return m[static_cast<size_t>(uid)];
  } else {
    auto it = m.find(uid);
    return it != m.end() ? it->second : default_val;
  }
}

} // namespace graph::adj_list

// Pull into graph namespace for convenient algorithm usage
namespace graph {
using adj_list::vertex_property_map;
using adj_list::make_vertex_property_map;
using adj_list::vertex_property_map_contains;
using adj_list::vertex_property_map_get;

// =============================================================================
// vertex_property_map_value_t — extract per-vertex value type from any vertex_property_map container
// =============================================================================

namespace detail {

template <class C>
constexpr bool has_mapped_type_v = requires { typename C::mapped_type; };

template <class Container, bool HasMapped = has_mapped_type_v<Container>>
struct vertex_property_map_value_impl;

template <class Container>
struct vertex_property_map_value_impl<Container, true> {
  using type = typename Container::mapped_type;
};

template <class Container>
struct vertex_property_map_value_impl<Container, false> {
  using type = typename Container::value_type;
};

} // namespace detail

/**
 * @brief Extracts the per-vertex value type from a vertex_property_map container.
 *
 * For vector<T>:           T  (via value_type)
 * For unordered_map<K, V>: V  (via mapped_type)
 * For _null_range_type:    size_t (via value_type, since it extends vector<size_t>)
 *
 * @tparam Container A vertex_property_map container type (vector or unordered_map)
 */
template <class Container>
using vertex_property_map_value_t = typename detail::vertex_property_map_value_impl<Container>::type;

// =============================================================================
// vertex_property_map_for — concept for subscriptable per-vertex property maps
// =============================================================================

/**
 * @brief Concept for a container usable as a per-vertex property map for graph G.
 *
 * Requires subscript access with the graph's vertex ID type, returning a value
 * convertible to the map's value type. This is the precise constraint for algorithm
 * parameters like Distances and Predecessors — the algorithm subscripts by vertex ID,
 * it never iterates the map as a range.
 *
 * Satisfied by:
 * - vector<T> for index graphs (VId is integral)
 * - unordered_map<VId, T> / map<VId, T> for mapped graphs
 *
 * @tparam M Container type (e.g. vector<T>, unordered_map<VId, T>)
 * @tparam G Graph type
 */
template <class M, class G>
concept vertex_property_map_for = requires(M& m, const vertex_id_t<G>& uid) {
  { m[uid] } -> std::convertible_to<vertex_property_map_value_t<M>>;
};

} // namespace graph
