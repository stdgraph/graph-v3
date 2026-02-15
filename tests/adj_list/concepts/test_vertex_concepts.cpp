/**
 * @file test_vertex_concepts.cpp
 * @brief Unit tests for vertex storage type concepts and pattern detection
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/descriptor.hpp>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <list>
#include <string>

using namespace graph;
using namespace graph::adj_list;

// Test vertex data types
struct VertexData {
  std::string name;
  double      weight;
};

// =============================================================================
// Direct Vertex Type Concept Tests
// =============================================================================

TEST_CASE("direct_vertex_type concept identifies random-access iterators", "[concepts][vertex_types]") {
  SECTION("Random-access container iterators are direct vertex types") {
    STATIC_REQUIRE(direct_vertex_type<std::vector<int>::iterator>);
    STATIC_REQUIRE(direct_vertex_type<std::vector<VertexData>::iterator>);
    STATIC_REQUIRE(direct_vertex_type<std::deque<int>::iterator>);
    STATIC_REQUIRE(direct_vertex_type<std::deque<std::string>::iterator>);
  }

  SECTION("Non-random-access iterators are not direct vertex types") {
    STATIC_REQUIRE_FALSE(direct_vertex_type<std::map<int, std::string>::iterator>);
    STATIC_REQUIRE_FALSE(direct_vertex_type<std::unordered_map<int, double>::iterator>);
    STATIC_REQUIRE_FALSE(direct_vertex_type<std::list<int>::iterator>);
  }

  SECTION("Const iterators work correctly") {
    STATIC_REQUIRE(direct_vertex_type<std::vector<int>::const_iterator>);
    STATIC_REQUIRE_FALSE(direct_vertex_type<std::map<int, int>::const_iterator>);
  }
}

// =============================================================================
// Keyed Vertex Type Concept Tests
// =============================================================================

TEST_CASE("keyed_vertex_type concept identifies map-like iterators", "[concepts][vertex_types]") {
  SECTION("Map iterators are keyed vertex types") {
    STATIC_REQUIRE(keyed_vertex_type<std::map<int, std::string>::iterator>);
    STATIC_REQUIRE(keyed_vertex_type<std::map<size_t, VertexData>::iterator>);
    STATIC_REQUIRE(keyed_vertex_type<std::unordered_map<int, double>::iterator>);
    STATIC_REQUIRE(keyed_vertex_type<std::unordered_map<std::string, int>::iterator>);
  }

  SECTION("Non-map iterators are not keyed vertex types") {
    STATIC_REQUIRE_FALSE(keyed_vertex_type<std::vector<int>::iterator>);
    STATIC_REQUIRE_FALSE(keyed_vertex_type<std::deque<std::string>::iterator>);
    STATIC_REQUIRE_FALSE(keyed_vertex_type<std::list<int>::iterator>);
  }

  SECTION("Const iterators work correctly") {
    STATIC_REQUIRE(keyed_vertex_type<std::map<int, std::string>::const_iterator>);
    STATIC_REQUIRE_FALSE(keyed_vertex_type<std::vector<int>::const_iterator>);
  }
}

// =============================================================================
// Comprehensive Vertex Iterator Concept Tests
// =============================================================================

TEST_CASE("vertex_iterator concept accepts valid vertex iterators", "[concepts][vertex_types]") {
  SECTION("Both direct and keyed vertex types satisfy vertex_iterator") {
    // Direct types
    STATIC_REQUIRE(vertex_iterator<std::vector<int>::iterator>);
    STATIC_REQUIRE(vertex_iterator<std::deque<VertexData>::iterator>);

    // Keyed types
    STATIC_REQUIRE(vertex_iterator<std::map<int, std::string>::iterator>);
    STATIC_REQUIRE(vertex_iterator<std::unordered_map<size_t, double>::iterator>);
  }

  SECTION("Invalid iterators are rejected") {
    // Forward-only iterators without pair-like value_type
    STATIC_REQUIRE_FALSE(vertex_iterator<std::list<int>::iterator>);
  }
}

// =============================================================================
// Vertex Storage Pattern Trait Tests
// =============================================================================

TEST_CASE("vertex_storage_pattern correctly identifies storage patterns", "[traits][vertex_types]") {
  SECTION("Direct storage patterns") {
    using Pattern = vertex_storage_pattern<std::vector<int>::iterator>;
    STATIC_REQUIRE(Pattern::is_direct);
    STATIC_REQUIRE_FALSE(Pattern::is_keyed);
  }

  SECTION("Keyed storage patterns") {
    using Pattern = vertex_storage_pattern<std::map<int, std::string>::iterator>;
    STATIC_REQUIRE_FALSE(Pattern::is_direct);
    STATIC_REQUIRE(Pattern::is_keyed);
  }

  SECTION("Deque uses direct storage") {
    using Pattern = vertex_storage_pattern<std::deque<VertexData>::iterator>;
    STATIC_REQUIRE(Pattern::is_direct);
    STATIC_REQUIRE_FALSE(Pattern::is_keyed);
  }

  SECTION("Unordered_map uses keyed storage") {
    using Pattern = vertex_storage_pattern<std::unordered_map<int, double>::iterator>;
    STATIC_REQUIRE_FALSE(Pattern::is_direct);
    STATIC_REQUIRE(Pattern::is_keyed);
  }
}

// =============================================================================
// Vertex Pattern Enum Tests
// =============================================================================

TEST_CASE("vertex_pattern_type_v returns correct enum values", "[traits][vertex_types]") {
  SECTION("Vector iterators return vertex_pattern::direct") {
    STATIC_REQUIRE(vertex_pattern_type_v<std::vector<int>::iterator> == vertex_pattern::direct);
    STATIC_REQUIRE(vertex_pattern_type_v<std::deque<std::string>::iterator> == vertex_pattern::direct);
  }

  SECTION("Map iterators return vertex_pattern::keyed") {
    STATIC_REQUIRE(vertex_pattern_type_v<std::map<int, double>::iterator> == vertex_pattern::keyed);
    STATIC_REQUIRE(vertex_pattern_type_v<std::unordered_map<size_t, std::string>::iterator> == vertex_pattern::keyed);
  }
}

// =============================================================================
// Mutual Exclusivity Tests
// =============================================================================

TEST_CASE("Vertex storage patterns are mutually exclusive", "[concepts][vertex_types]") {
  SECTION("Each iterator type matches exactly one pattern") {
    // Direct storage
    using VecIter = std::vector<int>::iterator;
    STATIC_REQUIRE(direct_vertex_type<VecIter>);
    STATIC_REQUIRE_FALSE(keyed_vertex_type<VecIter>);

    // Keyed storage
    using MapIter = std::map<int, std::string>::iterator;
    STATIC_REQUIRE_FALSE(direct_vertex_type<MapIter>);
    STATIC_REQUIRE(keyed_vertex_type<MapIter>);
  }
}

// =============================================================================
// Vertex ID Type Extraction Tests
// =============================================================================

TEST_CASE("vertex_id_type_t extracts correct ID types", "[traits][vertex_types]") {
  SECTION("Direct storage has size_t ID type") {
    using VecIter = std::vector<int>::iterator;
    using IDType  = vertex_id_type_t<VecIter>;
    STATIC_REQUIRE(std::same_as<IDType, std::size_t>);
  }

  SECTION("Keyed storage extracts key type as ID") {
    using MapIter = std::map<int, std::string>::iterator;
    using IDType  = vertex_id_type_t<MapIter>;
    STATIC_REQUIRE(std::same_as<IDType, int>);
  }

  SECTION("Different key types are extracted correctly") {
    // size_t key
    using MapIter1 = std::map<size_t, double>::iterator;
    using IDType1  = vertex_id_type_t<MapIter1>;
    STATIC_REQUIRE(std::same_as<IDType1, std::size_t>);

    // uint32_t key
    using MapIter2 = std::map<uint32_t, std::string>::iterator;
    using IDType2  = vertex_id_type_t<MapIter2>;
    STATIC_REQUIRE(std::same_as<IDType2, uint32_t>);

    // string key
    using MapIter3 = std::unordered_map<std::string, int>::iterator;
    using IDType3  = vertex_id_type_t<MapIter3>;
    STATIC_REQUIRE(std::same_as<IDType3, std::string>);
  }
}

// =============================================================================
// Runtime Pattern Detection Tests
// =============================================================================

TEST_CASE("Pattern detection works at runtime", "[traits][vertex_types]") {
  SECTION("Can query vertex storage pattern at runtime") {
    auto direct_pattern = vertex_storage_pattern_v<std::vector<int>::iterator>;
    REQUIRE(direct_pattern.is_direct);
    REQUIRE_FALSE(direct_pattern.is_keyed);

    auto keyed_pattern = vertex_storage_pattern_v<std::map<int, std::string>::iterator>;
    REQUIRE(keyed_pattern.is_keyed);
    REQUIRE_FALSE(keyed_pattern.is_direct);
  }
}

// =============================================================================
// Integration Tests with vertex_descriptor
// =============================================================================

TEST_CASE("Concepts work with actual vertex_descriptor usage", "[integration][vertex_types]") {
  SECTION("Vector-based vertex descriptor uses direct storage") {
    using VecIter = std::vector<int>::iterator;
    STATIC_REQUIRE(vertex_iterator<VecIter>);
    STATIC_REQUIRE(direct_vertex_type<VecIter>);
    STATIC_REQUIRE(vertex_pattern_type_v<VecIter> == vertex_pattern::direct);
    STATIC_REQUIRE(std::same_as<vertex_id_type_t<VecIter>, std::size_t>);
  }

  SECTION("Map-based vertex descriptor uses keyed storage") {
    using MapIter = std::map<int, std::string>::iterator;
    STATIC_REQUIRE(vertex_iterator<MapIter>);
    STATIC_REQUIRE(keyed_vertex_type<MapIter>);
    STATIC_REQUIRE(vertex_pattern_type_v<MapIter> == vertex_pattern::keyed);
    STATIC_REQUIRE(std::same_as<vertex_id_type_t<MapIter>, int>);
  }
}

// =============================================================================
// Const Correctness Tests
// =============================================================================

TEST_CASE("Concepts work correctly with const iterators", "[concepts][vertex_types][const]") {
  SECTION("Const vector iterator") {
    using ConstVecIter = std::vector<int>::const_iterator;
    STATIC_REQUIRE(vertex_iterator<ConstVecIter>);
    STATIC_REQUIRE(direct_vertex_type<ConstVecIter>);
    STATIC_REQUIRE(std::same_as<vertex_id_type_t<ConstVecIter>, std::size_t>);
  }

  SECTION("Const map iterator") {
    using ConstMapIter = std::map<int, std::string>::const_iterator;
    STATIC_REQUIRE(vertex_iterator<ConstMapIter>);
    STATIC_REQUIRE(keyed_vertex_type<ConstMapIter>);
    STATIC_REQUIRE(std::same_as<vertex_id_type_t<ConstMapIter>, int>);
  }
}

// =============================================================================
// Edge Case Tests
// =============================================================================

TEST_CASE("Edge cases in vertex type detection", "[concepts][vertex_types][edge_cases]") {
  SECTION("Empty vertex data types work") {
    struct Empty {};
    STATIC_REQUIRE(vertex_iterator<std::vector<Empty>::iterator>);
    STATIC_REQUIRE(vertex_iterator<std::map<int, Empty>::iterator>);
  }

  SECTION("Complex key types work with maps") {
    using ComplexKey = std::pair<int, int>;
    using MapIter    = std::map<ComplexKey, std::string>::iterator;
    STATIC_REQUIRE(vertex_iterator<MapIter>);
    STATIC_REQUIRE(keyed_vertex_type<MapIter>);
  }

  SECTION("Various vertex data types work") {
    STATIC_REQUIRE(vertex_iterator<std::vector<std::string>::iterator>);
    STATIC_REQUIRE(vertex_iterator<std::vector<std::vector<int>>::iterator>);
    STATIC_REQUIRE(vertex_iterator<std::vector<VertexData>::iterator>);
  }
}
