/**
 * @file map_graph_test_data.hpp
 * @brief Test data for map-based vertex container tests
 * 
 * Provides static test data that works with on-demand vertex creation
 * (vertices created from edge endpoints rather than resize_vertices).
 * 
 * Key features tested:
 * - Sparse vertex IDs (non-contiguous, e.g., 100, 500, 1000)
 * - Standard contiguous vertex IDs for comparison
 * - Expected results for each data set
 * 
 * Note: Edge data is provided as initializer_lists via graph builder functions
 * because map-based dynamic_graph only supports initializer_list construction.
 */

#ifndef MAP_GRAPH_TEST_DATA_HPP
#define MAP_GRAPH_TEST_DATA_HPP

#include <graph/graph_data.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <cstdint>

namespace graph::test::map_data {

using graph::copyable_edge_t;

// =============================================================================
// Expected results for basic test data (contiguous IDs: 0, 1, 2, 3)
// Graph structure: 0 -> 1 -> 2 -> 3
//                  0 -> 2
// =============================================================================

struct basic_expected {
  static constexpr size_t                  vertex_count   = 4;
  static constexpr size_t                  edge_count     = 4;
  static constexpr std::array<uint32_t, 4> vertex_ids     = {0, 1, 2, 3};
  static constexpr std::array<size_t, 4>   out_degrees    = {2, 1, 1, 0}; // degree of vertex 0, 1, 2, 3
  static constexpr int                     edge_value_sum = 100;          // 10 + 20 + 30 + 40
};

/**
 * @brief Build graph with basic edges (void edge value)
 */
template <typename Graph>
Graph make_basic_graph_void() {
  return Graph({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
}

/**
 * @brief Build graph with basic edges (int edge value)
 */
template <typename Graph>
Graph make_basic_graph_int() {
  return Graph({{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}});
}

// =============================================================================
// Expected results for sparse test data (non-contiguous IDs: 100, 500, 1000, 5000)
// This is the key feature of map-based vertex containers!
// Graph structure: 100 -> 500 -> 1000 -> 5000
//                  100 -> 1000
// =============================================================================

struct sparse_expected {
  static constexpr size_t                  vertex_count      = 4;
  static constexpr size_t                  edge_count        = 4;
  static constexpr std::array<uint32_t, 4> vertex_ids_sorted = {100, 500, 1000, 5000};
  static constexpr std::array<size_t, 4>   out_degrees       = {2, 1, 1, 0}; // degree by sorted order
  static constexpr int                     edge_value_sum    = 120;          // 15 + 25 + 35 + 45

  // For unordered containers (check contains rather than order)
  static constexpr uint32_t min_id = 100;
  static constexpr uint32_t max_id = 5000;
};

/**
 * @brief Build graph with sparse vertex IDs (void edge value)
 */
template <typename Graph>
Graph make_sparse_graph_void() {
  return Graph({{100, 500}, {100, 1000}, {500, 1000}, {1000, 5000}});
}

/**
 * @brief Build graph with sparse vertex IDs (int edge value)
 */
template <typename Graph>
Graph make_sparse_graph_int() {
  return Graph({{100, 500, 15}, {100, 1000, 25}, {500, 1000, 35}, {1000, 5000, 45}});
}

// =============================================================================
// Expected results for very sparse test data (widely scattered IDs)
// =============================================================================

struct very_sparse_expected {
  static constexpr size_t                  vertex_count      = 5; // 1, 2, 500000, 1000000, 2000000
  static constexpr size_t                  edge_count        = 3;
  static constexpr std::array<uint32_t, 5> vertex_ids_sorted = {1, 2, 500000, 1000000, 2000000};
};

/**
 * @brief Build graph with very sparse vertex IDs
 */
template <typename Graph>
Graph make_very_sparse_graph() {
  return Graph({{1, 1000000}, {1000000, 2000000}, {2, 500000}});
}

// =============================================================================
// Expected results for self-loop test data
// =============================================================================

struct self_loop_expected {
  static constexpr size_t vertex_count = 2; // 100, 200
  static constexpr size_t edge_count   = 3;
};

/**
 * @brief Build graph with self-loops
 */
template <typename Graph>
Graph make_self_loop_graph() {
  return Graph({{100, 100}, {100, 200}, {200, 200}});
}

// =============================================================================
// Expected results for string vertex ID test data
// =============================================================================

struct string_expected {
  static constexpr size_t vertex_count = 4; // alice, bob, charlie, dave
  static constexpr size_t edge_count   = 4;
  // Sorted order: alice, bob, charlie, dave
  static inline const std::vector<std::string> vertex_ids_sorted = {"alice", "bob", "charlie", "dave"};
  static constexpr int                         edge_value_sum    = 750;
};

// Note: String graph builders would need separate Graph types with string VId
// For now, string tests are done inline in test files

// =============================================================================
// Helper functions
// =============================================================================

/**
 * @brief Check if a container (sorted after copying) matches expected values
 */
template <typename Container, typename Expected>
bool matches_sorted(const Container& actual, const Expected& expected) {
  std::vector<typename Container::value_type> sorted_actual(actual.begin(), actual.end());
  std::ranges::sort(sorted_actual);

  std::vector<typename Expected::value_type> sorted_expected(expected.begin(), expected.end());
  std::ranges::sort(sorted_expected);

  return sorted_actual == sorted_expected;
}

/**
 * @brief Check if a value is in a container
 */
template <typename Container, typename T>
bool contains(const Container& c, const T& value) {
  return std::find(c.begin(), c.end(), value) != c.end();
}

} // namespace graph::test::map_data

#endif // MAP_GRAPH_TEST_DATA_HPP
