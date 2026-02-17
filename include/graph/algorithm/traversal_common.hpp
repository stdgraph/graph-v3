/**
 * @file traversal_common.hpp
 * @brief Common utilities and visitor concepts for graph traversal algorithms
 * 
 * This file provides shared functionality used by graph traversal and shortest path algorithms,
 * including:
 * - Visitor concepts that define callback interfaces for algorithm events
 * - Initialization utilities for distance and predecessor tracking
 * - Edge weight function concepts for weighted graph algorithms
 * - Helper types for optional predecessor tracking
 * 
 * The visitor concepts enable customizable behavior during graph traversal without modifying
 * the core algorithm implementations. Algorithms check for the presence of visitor methods
 * at compile time and only invoke those that are defined.
 * 
 * Used by: breadth_first_search, depth_first_search, dijkstra_shortest_paths,
 *          bellman_ford_shortest_paths, and topological_sort.
 */

#pragma once

#include <algorithm>
#include <numeric>
#include <graph/detail/graph_using.hpp>
#include <graph/graph_concepts.hpp>

#ifndef GRAPH_TRAVERSAL_COMMON_HPP
#  define GRAPH_TRAVERSAL_COMMON_HPP

namespace graph {

//
// Edge weight function concepts
//
// Note on std::remove_reference_t<G>:
// Algorithm templates declare G&& (forwarding reference), so for lvalue arguments G deduces
// as a reference type (e.g. vector<…>&). Writing "const G&" when G is already a reference
// triggers reference collapsing: const (vector<…>&) & → vector<…>& — the const is silently
// discarded because it qualifies the reference, not the referent. We use
// std::remove_reference_t<G> in concept constraints, invoke_result_t, and std::function
// default types so that "const std::remove_reference_t<G>&" always means a true const ref.
// Default lambdas use "const auto&" instead of "const G&" to sidestep the issue entirely.
//

/**
 * @brief Concept for a generalized edge weight function with custom comparison and combination.
 * 
 * This concept refines edge_value_function with additional arithmetic constraints: the
 * weight value must be combinable with distances and assignable back to the distance type.
 * 
 * @tparam G Graph type
 * @tparam WF Weight function type (e.g., lambda returning edge weight)
 * @tparam DistanceValue The arithmetic type used for distances
 * @tparam Compare Comparison operation for distance values (e.g., std::less for shortest paths)
 * @tparam Combine Combination operation for distances (e.g., std::plus for distance accumulation)
 */
template <class G, class WF, class DistanceValue, class Compare, class Combine>
concept basic_edge_weight_function =
      edge_value_function<WF, std::remove_reference_t<G>, edge_t<G>> && is_arithmetic_v<DistanceValue> &&
      std::strict_weak_order<Compare, DistanceValue, DistanceValue> &&
      std::assignable_from<
            std::add_lvalue_reference_t<DistanceValue>,
            invoke_result_t<Combine, DistanceValue, invoke_result_t<WF, const std::remove_reference_t<G>&, edge_t<G>>>>;

/**
 * @brief Concept for a standard edge weight function using default comparison and addition.
 * 
 * This is a convenience concept for the common case of shortest path algorithms that use
 * less-than comparison and addition for distance operations. Subsumes edge_value_function
 * (via basic_edge_weight_function) and additionally requires an arithmetic return type.
 * 
 * @tparam G Graph type
 * @tparam WF Weight function type that returns an arithmetic value for each edge
 * @tparam DistanceValue The arithmetic type used for distances
 */
template <class G, class WF, class DistanceValue>
concept edge_weight_function =
      is_arithmetic_v<invoke_result_t<WF, const std::remove_reference_t<G>&, edge_t<G>>> &&
      basic_edge_weight_function<G, WF, DistanceValue, less<DistanceValue>, plus<DistanceValue>>;

//
// Shortest path initialization utilities
//

/**
 * @ingroup graph_algorithms
 * @brief Returns a value representing infinite distance for shortest path algorithms.
 * 
 * Used to initialize distance values before running shortest path algorithms. Vertices with
 * this distance value are considered unreachable.
 * 
 * @tparam DistanceValue The arithmetic type used for distances.
 * 
 * @return A sentinel value representing infinite distance (typically std::numeric_limits<T>::max()).
 */
template <class DistanceValue>
constexpr auto shortest_path_infinite_distance() {
  return std::numeric_limits<DistanceValue>::max();
}

/**
 * @ingroup graph_algorithms
 * @brief Returns a zero distance value.
 * 
 * Used as the initial distance for source vertices in shortest path algorithms.
 * 
 * @tparam DistanceValue The arithmetic type used for distances.
 * 
 * @return A zero-initialized distance value.
 */
template <class DistanceValue>
constexpr auto shortest_path_zero() {
  return DistanceValue();
}

/**
 * @ingroup graph_algorithms
 * @brief Initializes all distance values to infinite distance.
 * 
 * Prepares a distance range for use with shortest path algorithms by setting all values
 * to shortest_path_infinite_distance().
 * 
 * @tparam Distances Random access range type containing distance values.
 * 
 * @param distances The range of distance values to initialize.
 */
template <class Distances>
constexpr void init_shortest_paths(Distances& distances) {
  std::ranges::fill(distances, shortest_path_infinite_distance<range_value_t<Distances>>());
}

/**
 * @ingroup graph_algorithms
 * @brief Initializes distance and predecessor values for shortest path algorithms.
 * 
 * Prepares both distance and predecessor ranges:
 * - Distances are set to shortest_path_infinite_distance()
 * - Predecessors are set to their own indices (each vertex is its own predecessor initially)
 * 
 * @tparam Distances Random access range type containing distance values.
 * @tparam Predecessors Random access range type containing predecessor vertex IDs.
 * 
 * @param distances The range of distance values to initialize.
 * @param predecessors The range of predecessor values to initialize.
 */
template <class Distances, class Predecessors>
constexpr void init_shortest_paths(Distances& distances, Predecessors& predecessors) {
  init_shortest_paths(distances);
  std::iota(predecessors.begin(), predecessors.end(), 0);
}

//
// Visitor concepts
//
// These concepts enable compile-time detection of visitor callback methods. Algorithms check
// for the presence of these methods and invoke them at specific points during traversal.
// Users can implement only the callbacks they need; missing methods are simply not called.
//

// Vertex visitor concepts

/// Concept for visitors that handle vertex initialization events (descriptor overload)
template <class G, class Visitor>
concept has_on_initialize_vertex = requires(Visitor& v, const G& g, const vertex_t<G>& vdesc) {
  { v.on_initialize_vertex(g, vdesc) };
};

/// Concept for visitors that handle vertex initialization events (vertex id overload)
template <class G, class Visitor>
concept has_on_initialize_vertex_id = requires(Visitor& v, const G& g, const vertex_id_t<G>& uid) {
  { v.on_initialize_vertex(g, uid) };
};

/// Concept for visitors that handle vertex discovery events (descriptor overload)
template <class G, class Visitor>
concept has_on_discover_vertex = requires(Visitor& v, const G& g, const vertex_t<G>& vdesc) {
  { v.on_discover_vertex(g, vdesc) };
};

/// Concept for visitors that handle vertex discovery events (vertex id overload)
template <class G, class Visitor>
concept has_on_discover_vertex_id = requires(Visitor& v, const G& g, const vertex_id_t<G>& uid) {
  { v.on_discover_vertex(g, uid) };
};

/// Concept for visitors that handle vertex examination events (descriptor overload)
template <class G, class Visitor>
concept has_on_examine_vertex = requires(Visitor& v, const G& g, const vertex_t<G>& vdesc) {
  { v.on_examine_vertex(g, vdesc) };
};

/// Concept for visitors that handle vertex examination events (vertex id overload)
template <class G, class Visitor>
concept has_on_examine_vertex_id = requires(Visitor& v, const G& g, const vertex_id_t<G>& uid) {
  { v.on_examine_vertex(g, uid) };
};

/// Concept for visitors that handle vertex finish events (descriptor overload)
template <class G, class Visitor>
concept has_on_finish_vertex = requires(Visitor& v, const G& g, const vertex_t<G>& vdesc) {
  { v.on_finish_vertex(g, vdesc) };
};

/// Concept for visitors that handle vertex finish events (vertex id overload)
template <class G, class Visitor>
concept has_on_finish_vertex_id = requires(Visitor& v, const G& g, const vertex_id_t<G>& uid) {
  { v.on_finish_vertex(g, uid) };
};

// Edge visitor concepts

/// Concept for visitors that handle edge examination events
template <class G, class Visitor>
concept has_on_examine_edge = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_examine_edge(g, e) };
};

/// Concept for visitors that handle edge relaxation events (distance was improved)
template <class G, class Visitor>
concept has_on_edge_relaxed = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_edge_relaxed(g, e) };
};

/// Concept for visitors that handle non-relaxation events (distance was not improved)
template <class G, class Visitor>
concept has_on_edge_not_relaxed = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_edge_not_relaxed(g, e) };
};

/// Concept for visitors that handle edge minimization events (used in negative cycle detection)
template <class G, class Visitor>
concept has_on_edge_minimized = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_edge_minimized(g, e) };
};

/// Concept for visitors that handle non-minimization events
template <class G, class Visitor>
concept has_on_edge_not_minimized = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_edge_not_minimized(g, e) };
};

// DFS-specific visitor concepts

/// Concept for visitors that handle DFS start events (descriptor overload)
template <class G, class Visitor>
concept has_on_start_vertex = requires(Visitor& v, const G& g, const vertex_t<G>& vdesc) {
  { v.on_start_vertex(g, vdesc) };
};

/// Concept for visitors that handle DFS start events (vertex id overload)
template <class G, class Visitor>
concept has_on_start_vertex_id = requires(Visitor& v, const G& g, const vertex_id_t<G>& uid) {
  { v.on_start_vertex(g, uid) };
};

/// Concept for visitors that handle tree edge events (edge to undiscovered vertex)
template <class G, class Visitor>
concept has_on_tree_edge = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_tree_edge(g, e) };
};

/// Concept for visitors that handle back edge events (edge to ancestor in DFS tree, indicates cycle)
template <class G, class Visitor>
concept has_on_back_edge = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_back_edge(g, e) };
};

/// Concept for visitors that handle forward or cross edge events (edge to already-finished vertex)
template <class G, class Visitor>
concept has_on_forward_or_cross_edge = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_forward_or_cross_edge(g, e) };
};

/// Concept for visitors that handle edge finish events (after edge and its target are fully processed)
template <class G, class Visitor>
concept has_on_finish_edge = requires(Visitor& v, const G& g, const edge_t<G>& e) {
  { v.on_finish_edge(g, e) };
};

//
// Visitor types
//

/// Empty visitor type for algorithms that don't require custom callbacks
struct empty_visitor {};

/**
 * @brief A null range type for optional predecessor tracking in shortest path algorithms.
 * 
 * This is a unique type that algorithms can detect at compile time to determine whether
 * predecessor tracking should be performed. It derives from std::vector<size_t> but remains
 * perpetually empty regardless of operations performed on it.
 * 
 * This enables a single algorithm implementation to support both cases:
 * - When predecessors are needed: use a real vector that stores parent vertices
 * - When predecessors are not needed: use this type to avoid tracking overhead
 * 
 * Implementation detail: Not part of the P1709 graph library proposal.
 */
class _null_range_type : public std::vector<size_t> {
  using T         = size_t;
  using Allocator = std::allocator<T>;
  using Base      = std::vector<T, Allocator>;

public:
  _null_range_type() noexcept(noexcept(Allocator())) = default;
  explicit _null_range_type(const Allocator& alloc) noexcept {}
  _null_range_type(Base::size_type count, const T& value, const Allocator& alloc = Allocator()) {}
  explicit _null_range_type(Base::size_type count, const Allocator& alloc = Allocator()) {}
  template <class InputIt>
  _null_range_type(InputIt first, InputIt last, const Allocator& alloc = Allocator()) {}
  _null_range_type(const _null_range_type& other) : Base() {}
  _null_range_type(const _null_range_type& other, const Allocator& alloc) {}
  _null_range_type(_null_range_type&& other) noexcept {}
  _null_range_type(_null_range_type&& other, const Allocator& alloc) {}
  _null_range_type(std::initializer_list<T> init, const Allocator& alloc = Allocator()) {}
};

/// Global instance of the null range used when predecessor tracking is not needed
inline static _null_range_type _null_predecessors;

} // namespace graph

#endif // GRAPH_TRAVERSAL_COMMON_HPP
