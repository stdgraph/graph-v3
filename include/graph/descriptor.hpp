/**
 * @file descriptor.hpp
 * @brief Core descriptor concepts and type traits for graph descriptors
 */

#pragma once

#include <concepts>
#include <iterator>
#include <tuple>
#include <type_traits>

namespace graph::adj_list {

/**
 * @brief Concept to check if a type is pair-like (has at least 2 members accessible via tuple protocol)
 * 
 * This is used to constrain bidirectional iterator value_types for vertex storage.
 */
template<typename T>
concept pair_like = requires {
    typename std::tuple_size<T>::type;
    requires std::tuple_size<T>::value >= 2;
    { std::get<0>(std::declval<T>()) };
    { std::get<1>(std::declval<T>()) };
};

/**
 * @brief Alternative pair-like concept checking for .first and .second members
 */
template<typename T>
concept has_first_second = requires(T t) {
    { t.first };
    { t.second };
};

/**
 * @brief Combined pair-like concept accepting either tuple protocol or pair members
 */
template<typename T>
concept pair_like_value = pair_like<T> || has_first_second<T>;

// =============================================================================
// Iterator Concepts (Base)
// =============================================================================

/**
 * @brief Concept for valid edge container iterators
 * 
 * Edge iterators must be at least forward iterators
 */
template<typename Iter>
concept edge_iterator = std::forward_iterator<Iter>;

// =============================================================================
// Vertex Value Type Concepts for vertex_id() Extraction
// =============================================================================

/**
 * @brief Concept for simple/direct vertex types (used with random-access containers)
 * 
 * Vertex is stored directly in a random-access container (e.g., vector, deque).
 * The vertex ID is the index in the container.
 * The entire container element represents the vertex data.
 * Example: std::vector<VertexData> where index is the vertex ID.
 */
template<typename Iter>
concept direct_vertex_type = std::random_access_iterator<Iter>;

/**
 * @brief Concept for key-value vertex types (used with associative containers)
 * 
 * Vertex is stored as a key-value pair in an associative container (e.g., map).
 * The key (first element) is the vertex ID.
 * The value (second element) contains the vertex data/properties.
 * Example: std::map<VertexId, VertexData> where VertexId is the vertex ID.
 */
template<typename Iter>
concept keyed_vertex_type = 
    std::forward_iterator<Iter> &&
    !std::random_access_iterator<Iter> &&
    pair_like_value<typename std::iterator_traits<Iter>::value_type>;

/**
 * @brief Comprehensive concept for any valid vertex iterator type
 * 
 * A vertex iterator must be either:
 * - Random access (direct/indexed storage)
 * - Bidirectional with pair-like value_type (keyed storage)
 */
template<typename Iter>
concept vertex_iterator = direct_vertex_type<Iter> || keyed_vertex_type<Iter>;

// =============================================================================
// Vertex Inner Value Pattern Concepts (for inner_value() method)
// =============================================================================

/**
 * @brief Concept for random-access vertex pattern (vector-like)
 * 
 * Used with random-access containers where inner_value returns the entire element.
 * Pattern: container[index] -> returns whole value
 * Example: std::vector<VertexData> where inner_value returns VertexData&
 */
template<typename Iter>
concept random_access_vertex_pattern = std::random_access_iterator<Iter>;

/**
 * @brief Concept for pair-value vertex pattern (map-like)
 * 
 * Used with bidirectional iterators where the value_type is pair-like.
 * inner_value returns the .second part (the data, excluding the key).
 * Pattern: *iterator -> pair{key, data}, inner_value returns data&
 * Example: std::map<int, VertexData> where inner_value returns VertexData& (.second)
 */
template<typename Iter>
concept pair_value_vertex_pattern = 
    std::bidirectional_iterator<Iter> &&
    !std::random_access_iterator<Iter> &&
    pair_like_value<typename std::iterator_traits<Iter>::value_type>;

/**
 * @brief Concept for whole-value vertex pattern (custom bidirectional)
 * 
 * Used with bidirectional iterators where the value_type is NOT pair-like.
 * inner_value returns the entire element (same as underlying_value).
 * Pattern: *iterator -> value, inner_value returns value&
 * Example: Custom bidirectional container with non-pair value_type
 */
template<typename Iter>
concept whole_value_vertex_pattern = 
    std::bidirectional_iterator<Iter> &&
    !std::random_access_iterator<Iter> &&
    !pair_like_value<typename std::iterator_traits<Iter>::value_type>;

/**
 * @brief Comprehensive concept for any valid vertex inner_value pattern
 * 
 * A vertex iterator must match exactly one of these patterns:
 * - Random access: Returns entire container[index] value
 * - Pair value: Returns .second of pair-like value
 * - Whole value: Returns entire dereferenced iterator value
 */
template<typename Iter>
concept has_inner_value_pattern = 
    random_access_vertex_pattern<Iter> || 
    pair_value_vertex_pattern<Iter> || 
    whole_value_vertex_pattern<Iter>;

// =============================================================================
// Vertex Storage Pattern Detection
// =============================================================================

/**
 * @brief Type trait to determine which vertex storage pattern an iterator uses
 */
template<typename Iter>
struct vertex_storage_pattern {
    static constexpr bool is_direct = direct_vertex_type<Iter>;
    static constexpr bool is_keyed = keyed_vertex_type<Iter>;
};

/**
 * @brief Helper variable template for vertex storage pattern detection
 */
template<typename Iter>
inline constexpr auto vertex_storage_pattern_v = vertex_storage_pattern<Iter>{};

/**
 * @brief Enumeration of vertex storage patterns
 */
enum class vertex_pattern {
    direct,   ///< Random-access/direct storage (index-based ID)
    keyed     ///< Key-value storage (key-based ID)
};

/**
 * @brief Type trait to get the vertex pattern as an enum value
 */
template<typename Iter>
struct vertex_pattern_type {
    static constexpr vertex_pattern value = 
        direct_vertex_type<Iter> ? vertex_pattern::direct :
        vertex_pattern::keyed;
};

/**
 * @brief Helper variable template for vertex pattern type
 */
template<typename Iter>
inline constexpr vertex_pattern vertex_pattern_type_v = vertex_pattern_type<Iter>::value;

// =============================================================================
// Vertex Inner Value Pattern Detection
// =============================================================================

/**
 * @brief Type trait to determine which inner_value pattern an iterator uses
 */
template<typename Iter>
struct vertex_inner_value_pattern {
    static constexpr bool is_random_access = random_access_vertex_pattern<Iter>;
    static constexpr bool is_pair_value = pair_value_vertex_pattern<Iter>;
    static constexpr bool is_whole_value = whole_value_vertex_pattern<Iter>;
};

/**
 * @brief Helper variable template for vertex inner_value pattern detection
 */
template<typename Iter>
inline constexpr auto vertex_inner_value_pattern_v = vertex_inner_value_pattern<Iter>{};

/**
 * @brief Enumeration of vertex inner_value patterns
 */
enum class vertex_inner_pattern {
    random_access,  ///< Random-access container, returns container[index]
    pair_value,     ///< Pair-like value, returns .second (data without key)
    whole_value     ///< Non-pair value, returns entire dereferenced iterator
};

/**
 * @brief Type trait to get the vertex inner_value pattern as an enum value
 */
template<typename Iter>
struct vertex_inner_pattern_type {
    static constexpr vertex_inner_pattern value = 
        random_access_vertex_pattern<Iter> ? vertex_inner_pattern::random_access :
        pair_value_vertex_pattern<Iter> ? vertex_inner_pattern::pair_value :
        vertex_inner_pattern::whole_value;
};

/**
 * @brief Helper variable template for vertex inner_value pattern type
 */
template<typename Iter>
inline constexpr vertex_inner_pattern vertex_inner_pattern_type_v = vertex_inner_pattern_type<Iter>::value;

// =============================================================================
// Vertex ID Type Extraction
// =============================================================================

/**
 * @brief Type trait to extract the vertex ID type from an iterator
 * 
 * For direct storage: ID type is size_t (the index)
 * For keyed storage: ID type is the key type (first element of pair)
 */
template<typename Iter>
    requires vertex_iterator<Iter>
struct vertex_id_type;

// Specialization for direct (random-access) storage
template<typename Iter>
    requires direct_vertex_type<Iter>
struct vertex_id_type<Iter> {
    using type = std::size_t;
};

// Specialization for keyed (map-based) storage with .first/.second
template<typename Iter>
    requires keyed_vertex_type<Iter> && 
             requires { std::declval<typename std::iterator_traits<Iter>::value_type>().first; }
struct vertex_id_type<Iter> {
    using type = std::remove_cvref_t<decltype(std::declval<typename std::iterator_traits<Iter>::value_type>().first)>;
};

// Specialization for keyed (map-based) storage using tuple protocol
template<typename Iter>
    requires keyed_vertex_type<Iter> &&
             (!requires { std::declval<typename std::iterator_traits<Iter>::value_type>().first; })
struct vertex_id_type<Iter> {
    using type = std::remove_cvref_t<decltype(std::get<0>(std::declval<typename std::iterator_traits<Iter>::value_type>()))>;
};

/**
 * @brief Helper alias for vertex ID type extraction
 */
template<typename Iter>
    requires vertex_iterator<Iter>
using vertex_id_type_t = typename vertex_id_type<Iter>::type;

// =============================================================================
// Edge Value Type Concepts for target_id() Extraction
// =============================================================================

/**
 * @brief Concept for simple integral edge types
 * 
 * Edge is represented as a simple integral value (the target vertex ID).
 * Example: std::vector<int> where each int is a target vertex ID.
 */
template<typename T>
concept simple_edge_type = std::integral<T>;

/**
 * @brief Concept for pair-like edge types with .first member
 * 
 * Edge is represented as a pair where .first is the target vertex ID
 * and .second contains edge properties.
 * Example: std::pair<int, double> where int is target, double is weight.
 */
template<typename T>
concept pair_edge_type = requires(T t) {
    { t.first };
    { t.second };
} && !std::integral<T>;

/**
 * @brief Concept for tuple-like edge types
 * 
 * Edge is represented as a tuple where the first element is the target vertex ID
 * and remaining elements contain edge properties.
 * Example: std::tuple<int, double, std::string> where int is target.
 */
template<typename T>
concept tuple_edge_type = requires {
    typename std::tuple_size<T>::type;
    requires std::tuple_size<T>::value >= 1;
    { std::get<0>(std::declval<T>()) };
} && !simple_edge_type<T> && !pair_edge_type<T>;

/**
 * @brief Concept for custom struct/class edge types
 * 
 * Edge is represented as a custom type where the entire value is returned.
 * The user is responsible for managing which fields represent the target ID.
 * This is the fallback for types that don't match other patterns.
 */
template<typename T>
concept custom_edge_type = 
    !simple_edge_type<T> && 
    !pair_edge_type<T> && 
    !tuple_edge_type<T>;

/**
 * @brief Comprehensive concept for any valid edge value type
 * 
 * An edge value type must match at least one of the supported patterns:
 * - Simple integral type (target ID only)
 * - Pair-like type (target ID in .first, properties in .second)
 * - Tuple-like type (target ID at index 0, properties in remaining elements)
 * - Custom struct/class (entire value represents edge data)
 */
template<typename T>
concept edge_value_type = 
    simple_edge_type<T> || 
    pair_edge_type<T> || 
    tuple_edge_type<T> || 
    custom_edge_type<T>;

// =============================================================================
// Type Traits for Edge Value Pattern Detection
// =============================================================================

/**
 * @brief Type trait to determine which edge value pattern a type matches
 */
template<typename T>
struct edge_value_pattern {
    static constexpr bool is_simple = simple_edge_type<T>;
    static constexpr bool is_pair = pair_edge_type<T>;
    static constexpr bool is_tuple = tuple_edge_type<T>;
    static constexpr bool is_custom = custom_edge_type<T>;
};

/**
 * @brief Helper variable template for edge value pattern detection
 */
template<typename T>
inline constexpr auto edge_value_pattern_v = edge_value_pattern<T>{};

/**
 * @brief Enumeration of edge value patterns
 */
enum class edge_pattern {
    simple,   ///< Simple integral type
    pair,     ///< Pair-like with .first/.second
    tuple,    ///< Tuple-like with std::get<N>
    custom    ///< Custom struct/class
};

/**
 * @brief Type trait to get the edge pattern as an enum value
 */
template<typename T>
struct edge_pattern_type {
    static constexpr edge_pattern value = 
        simple_edge_type<T> ? edge_pattern::simple :
        pair_edge_type<T> ? edge_pattern::pair :
        tuple_edge_type<T> ? edge_pattern::tuple :
        edge_pattern::custom;
};

/**
 * @brief Helper variable template for edge pattern type
 */
template<typename T>
inline constexpr edge_pattern edge_pattern_type_v = edge_pattern_type<T>::value;

} // namespace graph::adj_list
