/**
 * @file algorithm_test_types.hpp
 * @brief Type lists and utilities for parameterized algorithm testing
 * 
 * Provides curated type lists for testing graph algorithms across multiple
 * container combinations. Builds on graph_test_types.hpp with algorithm-specific
 * categorization and filtering.
 * 
 * Usage Pattern:
 *   TEMPLATE_TEST_CASE("shortest_path", "[algorithm][dijkstra]",
 *                      DIRECTED_WEIGHTED_TYPES) {
 *       using Graph = TestType;
 *       auto g = fixtures::clrs_dijkstra_graph<Graph>();
 *       // ... test algorithm
 *   }
 * 
 * Categories:
 * - BASIC_DIRECTED_TYPES: Small set for quick testing (vov, dov)
 * - ALL_DIRECTED_TYPES: Comprehensive directed graph containers
 * - ALL_UNDIRECTED_TYPES: Undirected graph containers
 * - SPARSE_VERTEX_TYPES: Map-based containers (sparse vertex IDs)
 * - WEIGHTED_TYPES: Containers with int edge values
 */

#ifndef ALGORITHM_TEST_TYPES_HPP
#define ALGORITHM_TEST_TYPES_HPP

#include "graph_test_types.hpp"
#include <graph/container/dynamic_graph.hpp>

namespace graph::test::algorithm {

// =============================================================================
// Type Aliases for Common Algorithm Test Configurations
// =============================================================================

// Basic types with void edge values (for unweighted algorithms)
using vov_void = graph_test_types<vov_tag>::void_type;
using dov_void = graph_test_types<dov_tag>::void_type;
using vol_void = graph_test_types<vol_tag>::void_type;
using dol_void = graph_test_types<dol_tag>::void_type;

// Weighted types with int edge values (for shortest path, MST, etc.)
using vov_weighted = graph_test_types<vov_tag>::int_ev;
using dov_weighted = graph_test_types<dov_tag>::int_ev;
using vol_weighted = graph_test_types<vol_tag>::int_ev;
using dol_weighted = graph_test_types<dol_tag>::int_ev;
using vod_weighted = graph_test_types<vod_tag>::int_ev;
using dod_weighted = graph_test_types<dod_tag>::int_ev;

// Sparse vertex container types (map-based)
using mov_weighted = graph_test_types<mov_tag>::int_ev;
using mod_weighted = graph_test_types<mod_tag>::int_ev;
using mol_weighted = graph_test_types<mol_tag>::int_ev;
using uov_weighted = graph_test_types<uov_tag>::int_ev;
using uod_weighted = graph_test_types<uod_tag>::int_ev;
using uol_weighted = graph_test_types<uol_tag>::int_ev;

// Set-based edge containers (for algorithms requiring ordered edges)
using vos_weighted = graph_test_types<vos_tag>::int_ev;
using dos_weighted = graph_test_types<dos_tag>::int_ev;
using mos_weighted = graph_test_types<mos_tag>::int_ev;

// Forward list edge containers (for algorithms that work with any forward_range)
using vofl_weighted = graph_test_types<vofl_tag>::int_ev;
using dofl_weighted = graph_test_types<dofl_tag>::int_ev;
using mofl_weighted = graph_test_types<mofl_tag>::int_ev;

// =============================================================================
// Curated Type Lists for Algorithm Testing
// =============================================================================

/**
 * @brief Basic directed types - minimal set for quick smoke tests
 * Use for rapid development and debugging
 */
#define BASIC_DIRECTED_TYPES vov_void, dov_void

/**
 * @brief Basic weighted types - minimal set for weighted algorithm tests
 */
#define BASIC_WEIGHTED_TYPES vov_weighted, dov_weighted

/**
 * @brief All directed random-access types (void edges)
 * Comprehensive testing for unweighted directed graph algorithms
 */
#define ALL_DIRECTED_TYPES vov_void, vod_void, dov_void, dod_void

/**
 * @brief All directed weighted types
 * Comprehensive testing for weighted directed graph algorithms
 */
#define ALL_DIRECTED_WEIGHTED_TYPES vov_weighted, vod_weighted, dov_weighted, dod_weighted, vol_weighted, dol_weighted

/**
 * @brief Sparse vertex container types (map/unordered_map based)
 * For testing algorithms with non-contiguous vertex IDs
 */
#define SPARSE_VERTEX_TYPES mov_weighted, mod_weighted, mol_weighted, uov_weighted, uod_weighted, uol_weighted

/**
 * @brief Ordered edge container types (set-based)
 * For algorithms that benefit from or require ordered edges
 */
#define ORDERED_EDGE_TYPES vos_weighted, dos_weighted, mos_weighted

/**
 * @brief Forward list edge container types
 * For testing algorithms with minimal container requirements
 */
#define FORWARD_EDGE_TYPES vofl_weighted, dofl_weighted, mofl_weighted

/**
 * @brief Extended test suite - includes more exotic container combinations
 * For comprehensive compatibility testing
 */
#define EXTENDED_TYPES ALL_DIRECTED_WEIGHTED_TYPES, SPARSE_VERTEX_TYPES, ORDERED_EDGE_TYPES

/**
 * @brief Full test suite - all supported container types
 * Warning: May significantly increase compilation time
 */
#define ALL_ALGORITHM_TYPES ALL_DIRECTED_WEIGHTED_TYPES, SPARSE_VERTEX_TYPES, ORDERED_EDGE_TYPES, FORWARD_EDGE_TYPES

// =============================================================================
// Undirected Graph Types (when undirected_adjacency_list support is added)
// =============================================================================

// Placeholder for future undirected graph types
// #define ALL_UNDIRECTED_TYPES \
//     undirected_vov, undirected_dov, undirected_vol

// =============================================================================
// Helper Traits for Algorithm Requirements
// =============================================================================

/**
 * @brief Check if a graph type has random access edges
 * Required for algorithms that need O(1) edge counting or random access
 */
template <typename G>
concept random_access_edges = std::ranges::random_access_range<vertex_edge_range_t<G>>;

/**
 * @brief Check if a graph type has ordered edges
 * Useful for algorithms that can optimize with sorted edges
 */
template <typename G>
concept ordered_vertex_edges = requires(G& g, vertex_id_t<G> u) {
  { std::ranges::is_sorted(edges(g, u)) };
};

/**
 * @brief Check if a graph type supports sparse vertex IDs
 * True for map-based vertex containers
 */
template <typename G>
struct is_sparse_vertex_container : std::false_type {};

template <typename EV, typename VV, typename GV, typename VId, bool Sourced, typename Traits>
struct is_sparse_vertex_container<container::dynamic_graph<EV, VV, GV, VId, Sourced, Traits>>
      : std::bool_constant<std::same_as<Traits, container::mov_graph_traits<EV, VV, GV, VId, Sourced>> ||
                           std::same_as<Traits, container::mod_graph_traits<EV, VV, GV, VId, Sourced>> ||
                           std::same_as<Traits, container::mol_graph_traits<EV, VV, GV, VId, Sourced>> ||
                           std::same_as<Traits, container::uov_graph_traits<EV, VV, GV, VId, Sourced>> ||
                           std::same_as<Traits, container::uod_graph_traits<EV, VV, GV, VId, Sourced>> ||
                           std::same_as<Traits, container::uol_graph_traits<EV, VV, GV, VId, Sourced>>> {};

template <typename G>
inline constexpr bool is_sparse_vertex_container_v = is_sparse_vertex_container<G>::value;

// =============================================================================
// Algorithm Category Tags
// =============================================================================

/**
 * @brief Tags for categorizing algorithms by their requirements
 * Can be used for SFINAE or concept-based filtering
 */

struct unweighted_algorithm_tag {};
struct weighted_algorithm_tag {};
struct directed_only_tag {};
struct undirected_only_tag {};
struct dag_only_tag {};
struct connected_only_tag {};

// =============================================================================
// Test Data Selection Helpers
// =============================================================================

/**
 * @brief Select appropriate fixture based on graph properties
 */
template <typename Graph>
struct fixture_selector {
  // Use sparse fixtures for sparse vertex containers
  static constexpr bool use_sparse = is_sparse_vertex_container_v<Graph>;
};

} // namespace graph::test::algorithm

#endif // ALGORITHM_TEST_TYPES_HPP
