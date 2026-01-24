/**
 * @file descriptor_traits.hpp
 * @brief Type traits and utilities for descriptor types
 */

#pragma once

#include "descriptor.hpp"
#include <type_traits>
#include <concepts>

namespace graph::adj_list {

// Forward declarations
template<vertex_iterator VertexIter>
class vertex_descriptor;

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
class edge_descriptor;

template<vertex_iterator VertexIter>
class vertex_descriptor_view;

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
class edge_descriptor_view;

// =============================================================================
// Primary type trait templates
// =============================================================================

/**
 * @brief Primary template - not a descriptor
 */
template<typename T>
struct is_vertex_descriptor : std::false_type {};

/**
 * @brief Specialization for vertex_descriptor
 */
template<vertex_iterator VertexIter>
struct is_vertex_descriptor<vertex_descriptor<VertexIter>> : std::true_type {};

/**
 * @brief Helper variable template for is_vertex_descriptor
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_vertex_descriptor_v = is_vertex_descriptor<std::remove_cv_t<T>>::value;

/**
 * @brief Primary template - not an edge descriptor
 */
template<typename T>
struct is_edge_descriptor : std::false_type {};

/**
 * @brief Specialization for edge_descriptor
 */
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct is_edge_descriptor<edge_descriptor<EdgeIter, VertexIter>> : std::true_type {};

/**
 * @brief Helper variable template for is_edge_descriptor
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_edge_descriptor_v = is_edge_descriptor<std::remove_cv_t<T>>::value;

/**
 * @brief Primary template - not a descriptor
 */
template<typename T>
struct is_descriptor : std::false_type {};

/**
 * @brief Specialization for vertex_descriptor
 */
template<vertex_iterator VertexIter>
struct is_descriptor<vertex_descriptor<VertexIter>> : std::true_type {};

/**
 * @brief Specialization for edge_descriptor
 */
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct is_descriptor<edge_descriptor<EdgeIter, VertexIter>> : std::true_type {};

/**
 * @brief Helper variable template for is_descriptor
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_descriptor_v = is_descriptor<std::remove_cv_t<T>>::value;

// =============================================================================
// Descriptor view traits
// =============================================================================

/**
 * @brief Primary template - not a descriptor view
 */
template<typename T>
struct is_vertex_descriptor_view : std::false_type {};

/**
 * @brief Specialization for vertex_descriptor_view
 */
template<vertex_iterator VertexIter>
struct is_vertex_descriptor_view<vertex_descriptor_view<VertexIter>> : std::true_type {};

/**
 * @brief Helper variable template for is_vertex_descriptor_view
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_vertex_descriptor_view_v = is_vertex_descriptor_view<std::remove_cv_t<T>>::value;

/**
 * @brief Primary template - not an edge descriptor view
 */
template<typename T>
struct is_edge_descriptor_view : std::false_type {};

/**
 * @brief Specialization for edge_descriptor_view
 */
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct is_edge_descriptor_view<edge_descriptor_view<EdgeIter, VertexIter>> : std::true_type {};

/**
 * @brief Helper variable template for is_edge_descriptor_view
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_edge_descriptor_view_v = is_edge_descriptor_view<std::remove_cv_t<T>>::value;

/**
 * @brief Primary template - not a descriptor view
 */
template<typename T>
struct is_descriptor_view : std::false_type {};

/**
 * @brief Specialization for vertex_descriptor_view
 */
template<vertex_iterator VertexIter>
struct is_descriptor_view<vertex_descriptor_view<VertexIter>> : std::true_type {};

/**
 * @brief Specialization for edge_descriptor_view
 */
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct is_descriptor_view<edge_descriptor_view<EdgeIter, VertexIter>> : std::true_type {};

/**
 * @brief Helper variable template for is_descriptor_view
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_descriptor_view_v = is_descriptor_view<std::remove_cv_t<T>>::value;

// =============================================================================
// Type extraction traits
// =============================================================================

/**
 * @brief Extract iterator type from vertex descriptor
 */
template<typename T>
struct descriptor_iterator_type;

template<vertex_iterator VertexIter>
struct descriptor_iterator_type<vertex_descriptor<VertexIter>> {
    using type = VertexIter;
};

template<vertex_iterator VertexIter>
struct descriptor_iterator_type<vertex_descriptor_view<VertexIter>> {
    using type = VertexIter;
};

/**
 * @brief Helper alias for descriptor_iterator_type
 */
template<typename T>
using descriptor_iterator_type_t = typename descriptor_iterator_type<T>::type;

/**
 * @brief Extract edge iterator type from edge descriptor
 */
template<typename T>
struct edge_descriptor_edge_iterator_type;

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct edge_descriptor_edge_iterator_type<edge_descriptor<EdgeIter, VertexIter>> {
    using type = EdgeIter;
};

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct edge_descriptor_edge_iterator_type<edge_descriptor_view<EdgeIter, VertexIter>> {
    using type = EdgeIter;
};

/**
 * @brief Helper alias for edge_descriptor_edge_iterator_type
 */
template<typename T>
using edge_descriptor_edge_iterator_type_t = typename edge_descriptor_edge_iterator_type<T>::type;

/**
 * @brief Extract vertex iterator type from edge descriptor
 */
template<typename T>
struct edge_descriptor_vertex_iterator_type;

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct edge_descriptor_vertex_iterator_type<edge_descriptor<EdgeIter, VertexIter>> {
    using type = VertexIter;
};

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct edge_descriptor_vertex_iterator_type<edge_descriptor_view<EdgeIter, VertexIter>> {
    using type = VertexIter;
};

/**
 * @brief Helper alias for edge_descriptor_vertex_iterator_type
 */
template<typename T>
using edge_descriptor_vertex_iterator_type_t = typename edge_descriptor_vertex_iterator_type<T>::type;

/**
 * @brief Extract storage type from vertex descriptor
 */
template<typename T>
struct descriptor_storage_type;

template<vertex_iterator VertexIter>
struct descriptor_storage_type<vertex_descriptor<VertexIter>> {
    using type = typename vertex_descriptor<VertexIter>::storage_type;
};

/**
 * @brief Helper alias for descriptor_storage_type
 */
template<typename T>
using descriptor_storage_type_t = typename descriptor_storage_type<T>::type;

/**
 * @brief Extract edge storage type from edge descriptor
 */
template<typename T>
struct edge_descriptor_storage_type;

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct edge_descriptor_storage_type<edge_descriptor<EdgeIter, VertexIter>> {
    using type = typename edge_descriptor<EdgeIter, VertexIter>::edge_storage_type;
};

/**
 * @brief Helper alias for edge_descriptor_storage_type
 */
template<typename T>
using edge_descriptor_storage_type_t = typename edge_descriptor_storage_type<T>::type;

// =============================================================================
// Storage category traits
// =============================================================================

/**
 * @brief Check if descriptor uses random access storage (size_t index)
 */
template<typename T>
struct is_random_access_descriptor : std::false_type {};

template<vertex_iterator VertexIter>
struct is_random_access_descriptor<vertex_descriptor<VertexIter>> 
    : std::bool_constant<std::random_access_iterator<VertexIter>> {};

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct is_random_access_descriptor<edge_descriptor<EdgeIter, VertexIter>>
    : std::bool_constant<std::random_access_iterator<EdgeIter>> {};

/**
 * @brief Helper variable template for is_random_access_descriptor
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_random_access_descriptor_v = is_random_access_descriptor<std::remove_cv_t<T>>::value;

/**
 * @brief Check if descriptor uses iterator-based storage
 */
template<typename T>
struct is_iterator_based_descriptor : std::false_type {};

template<vertex_iterator VertexIter>
struct is_iterator_based_descriptor<vertex_descriptor<VertexIter>>
    : std::bool_constant<!std::random_access_iterator<VertexIter>> {};

template<edge_iterator EdgeIter, vertex_iterator VertexIter>
struct is_iterator_based_descriptor<edge_descriptor<EdgeIter, VertexIter>>
    : std::bool_constant<!std::random_access_iterator<EdgeIter>> {};

/**
 * @brief Helper variable template for is_iterator_based_descriptor
 * Removes cv-qualifiers before checking
 */
template<typename T>
inline constexpr bool is_iterator_based_descriptor_v = is_iterator_based_descriptor<std::remove_cv_t<T>>::value;

// =============================================================================
// Concept-based type traits
// =============================================================================

/**
 * @brief Concept for vertex descriptor types
 */
template<typename T>
concept vertex_descriptor_type = is_vertex_descriptor_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for edge descriptor types
 */
template<typename T>
concept edge_descriptor_type = is_edge_descriptor_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for any descriptor type
 */
template<typename T>
concept descriptor_type = is_descriptor_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for vertex descriptor view types
 */
template<typename T>
concept vertex_descriptor_view_type = is_vertex_descriptor_view_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for edge descriptor view types
 */
template<typename T>
concept edge_descriptor_view_type = is_edge_descriptor_view_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for any descriptor view type
 */
template<typename T>
concept descriptor_view_type = is_descriptor_view_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for random access descriptors
 */
template<typename T>
concept random_access_descriptor = 
    descriptor_type<T> && is_random_access_descriptor_v<std::remove_cvref_t<T>>;

/**
 * @brief Concept for iterator-based descriptors
 */
template<typename T>
concept iterator_based_descriptor = 
    descriptor_type<T> && is_iterator_based_descriptor_v<std::remove_cvref_t<T>>;

// =============================================================================
// Utility functions
// =============================================================================

/**
 * @brief Get the descriptor category as a string (for debugging/logging)
 */
template<typename T>
constexpr const char* descriptor_category() noexcept {
    if constexpr (is_vertex_descriptor_v<T>) {
        return "vertex_descriptor";
    } else if constexpr (is_edge_descriptor_v<T>) {
        return "edge_descriptor";
    } else if constexpr (is_vertex_descriptor_view_v<T>) {
        return "vertex_descriptor_view";
    } else if constexpr (is_edge_descriptor_view_v<T>) {
        return "edge_descriptor_view";
    } else {
        return "not_a_descriptor";
    }
}

/**
 * @brief Get the storage category as a string (for debugging/logging)
 */
template<typename T>
constexpr const char* storage_category() noexcept {
    if constexpr (is_random_access_descriptor_v<T>) {
        return "random_access";
    } else if constexpr (is_iterator_based_descriptor_v<T>) {
        return "iterator_based";
    } else {
        return "unknown";
    }
}

} // namespace graph::adj_list
