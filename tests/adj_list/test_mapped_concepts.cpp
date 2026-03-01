/**
 * @file test_mapped_concepts.cpp
 * @brief Static assertion tests for mapped graph concepts
 *
 * Verifies mutual exclusivity between index and mapped concept families,
 * and that both families satisfy the base adjacency_list concept.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/graph.hpp>
#include <graph/container/dynamic_graph.hpp>
#include "../common/algorithm_test_types.hpp"

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test::algorithm;

// =============================================================================
// Index-based graphs satisfy index concepts but NOT mapped concepts
// =============================================================================

TEST_CASE("index graphs satisfy index_adjacency_list", "[mapped_concepts][index]") {
  STATIC_REQUIRE(index_vertex_range<vov_weighted>);
  STATIC_REQUIRE(index_adjacency_list<vov_weighted>);
  STATIC_REQUIRE(adjacency_list<vov_weighted>);
}

TEST_CASE("index graphs do NOT satisfy mapped concepts", "[mapped_concepts][index]") {
  STATIC_REQUIRE_FALSE(mapped_vertex_range<vov_weighted>);
  STATIC_REQUIRE_FALSE(mapped_adjacency_list<vov_weighted>);
}

TEST_CASE("deque-based graphs satisfy index concepts", "[mapped_concepts][index]") {
  STATIC_REQUIRE(index_vertex_range<dov_weighted>);
  STATIC_REQUIRE(index_adjacency_list<dov_weighted>);
  STATIC_REQUIRE_FALSE(mapped_vertex_range<dov_weighted>);
}

// =============================================================================
// Map-based graphs satisfy mapped concepts but NOT index concepts
// =============================================================================

TEST_CASE("map-based graphs satisfy mapped_adjacency_list", "[mapped_concepts][mapped]") {
  // std::map-based vertex containers
  STATIC_REQUIRE(mapped_vertex_range<mov_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<mov_weighted>);
  STATIC_REQUIRE(adjacency_list<mov_weighted>);
}

TEST_CASE("map-based graphs do NOT satisfy index concepts", "[mapped_concepts][mapped]") {
  STATIC_REQUIRE_FALSE(index_vertex_range<mov_weighted>);
  STATIC_REQUIRE_FALSE(index_adjacency_list<mov_weighted>);
}

TEST_CASE("unordered_map-based graphs satisfy mapped concepts", "[mapped_concepts][mapped]") {
  STATIC_REQUIRE(mapped_vertex_range<uov_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<uov_weighted>);
  STATIC_REQUIRE_FALSE(index_vertex_range<uov_weighted>);
}

// =============================================================================
// Both families satisfy the base adjacency_list concept
// =============================================================================

TEST_CASE("both index and mapped satisfy adjacency_list", "[mapped_concepts][base]") {
  STATIC_REQUIRE(adjacency_list<vov_weighted>);
  STATIC_REQUIRE(adjacency_list<dov_weighted>);
  STATIC_REQUIRE(adjacency_list<mov_weighted>);
  STATIC_REQUIRE(adjacency_list<uov_weighted>);
}

// =============================================================================
// hashable_vertex_id
// =============================================================================

TEST_CASE("map-based graphs have hashable vertex IDs", "[mapped_concepts][hashable]") {
  STATIC_REQUIRE(hashable_vertex_id<mov_weighted>);
  STATIC_REQUIRE(hashable_vertex_id<mod_weighted>);
  STATIC_REQUIRE(hashable_vertex_id<mol_weighted>);
  STATIC_REQUIRE(hashable_vertex_id<uov_weighted>);
  STATIC_REQUIRE(hashable_vertex_id<uod_weighted>);
  STATIC_REQUIRE(hashable_vertex_id<uol_weighted>);
}

TEST_CASE("index-based graphs also have hashable vertex IDs (uint32_t)", "[mapped_concepts][hashable]") {
  // uint32_t is hashable, so this should hold for index graphs too
  STATIC_REQUIRE(hashable_vertex_id<vov_weighted>);
}

// =============================================================================
// Mutual exclusivity: index_vertex_range XOR mapped_vertex_range
// =============================================================================

TEST_CASE("index and mapped are mutually exclusive", "[mapped_concepts][exclusive]") {
  // Index types
  STATIC_REQUIRE(index_vertex_range<vov_weighted> && !mapped_vertex_range<vov_weighted>);
  STATIC_REQUIRE(index_vertex_range<dov_weighted> && !mapped_vertex_range<dov_weighted>);

  // Mapped types
  STATIC_REQUIRE(!index_vertex_range<mov_weighted> && mapped_vertex_range<mov_weighted>);
  STATIC_REQUIRE(!index_vertex_range<uov_weighted> && mapped_vertex_range<uov_weighted>);
  STATIC_REQUIRE(!index_vertex_range<mod_weighted> && mapped_vertex_range<mod_weighted>);
  STATIC_REQUIRE(!index_vertex_range<uod_weighted> && mapped_vertex_range<uod_weighted>);
}

// =============================================================================
// Additional map-based types from SPARSE_VERTEX_TYPES
// =============================================================================

TEST_CASE("all SPARSE_VERTEX_TYPES satisfy mapped_adjacency_list", "[mapped_concepts][sparse]") {
  STATIC_REQUIRE(mapped_adjacency_list<mov_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<mod_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<mol_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<uov_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<uod_weighted>);
  STATIC_REQUIRE(mapped_adjacency_list<uol_weighted>);
}
