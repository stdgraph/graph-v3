/**
 * @file test_edge_concepts.cpp
 * @brief Unit tests for edge value type concepts and pattern detection
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/descriptor.hpp>
#include <vector>
#include <tuple>
#include <string>

using namespace graph;
using namespace graph::adj_list;

// Test edge data types
struct CustomEdge {
  int         target;
  double      weight;
  std::string label;
};

// =============================================================================
// Simple Edge Type Concept Tests
// =============================================================================

TEST_CASE("simple_edge_type concept identifies integral types", "[concepts][edge_types]") {
  SECTION("Basic integral types are simple edge types") {
    STATIC_REQUIRE(simple_edge_type<int>);
    STATIC_REQUIRE(simple_edge_type<unsigned int>);
    STATIC_REQUIRE(simple_edge_type<size_t>);
    STATIC_REQUIRE(simple_edge_type<uint32_t>);
    STATIC_REQUIRE(simple_edge_type<int64_t>);
  }

  SECTION("Non-integral types are not simple edge types") {
    STATIC_REQUIRE_FALSE(simple_edge_type<double>);
    STATIC_REQUIRE_FALSE(simple_edge_type<std::string>);
    STATIC_REQUIRE_FALSE(simple_edge_type<std::pair<int, double>>);
    STATIC_REQUIRE_FALSE(simple_edge_type<std::tuple<int, double>>);
    STATIC_REQUIRE_FALSE(simple_edge_type<CustomEdge>);
  }
}

// =============================================================================
// Pair Edge Type Concept Tests
// =============================================================================

TEST_CASE("pair_edge_type concept identifies pair-like types", "[concepts][edge_types]") {
  SECTION("std::pair types are pair edge types") {
    STATIC_REQUIRE(pair_edge_type<std::pair<int, double>>);
    STATIC_REQUIRE(pair_edge_type<std::pair<size_t, float>>);
    STATIC_REQUIRE(pair_edge_type<std::pair<int, std::string>>);
  }

  SECTION("Non-pair types are not pair edge types") {
    STATIC_REQUIRE_FALSE(pair_edge_type<int>);
    STATIC_REQUIRE_FALSE(pair_edge_type<double>);
    STATIC_REQUIRE_FALSE(pair_edge_type<std::tuple<int, double>>);
    STATIC_REQUIRE_FALSE(pair_edge_type<CustomEdge>);
  }
}

// =============================================================================
// Tuple Edge Type Concept Tests
// =============================================================================

TEST_CASE("tuple_edge_type concept identifies tuple-like types", "[concepts][edge_types]") {
  SECTION("std::tuple types are tuple edge types") {
    STATIC_REQUIRE(tuple_edge_type<std::tuple<int>>);
    STATIC_REQUIRE(tuple_edge_type<std::tuple<int, double>>);
    STATIC_REQUIRE(tuple_edge_type<std::tuple<int, double, std::string>>);
    STATIC_REQUIRE(tuple_edge_type<std::tuple<size_t, float, int, char>>);
  }

  SECTION("Non-tuple types are not tuple edge types") {
    STATIC_REQUIRE_FALSE(tuple_edge_type<int>);
    STATIC_REQUIRE_FALSE(tuple_edge_type<std::pair<int, double>>);
    STATIC_REQUIRE_FALSE(tuple_edge_type<CustomEdge>);
  }
}

// =============================================================================
// Custom Edge Type Concept Tests
// =============================================================================

TEST_CASE("custom_edge_type concept identifies custom struct/class types", "[concepts][edge_types]") {
  SECTION("Custom structs are custom edge types") { STATIC_REQUIRE(custom_edge_type<CustomEdge>); }

  SECTION("Standard types are not custom edge types") {
    STATIC_REQUIRE_FALSE(custom_edge_type<int>);
    STATIC_REQUIRE_FALSE(custom_edge_type<std::pair<int, double>>);
    STATIC_REQUIRE_FALSE(custom_edge_type<std::tuple<int, double>>);
  }
}

// =============================================================================
// Comprehensive Edge Value Type Concept Tests
// =============================================================================

TEST_CASE("edge_value_type concept accepts all valid edge patterns", "[concepts][edge_types]") {
  SECTION("All edge type patterns satisfy edge_value_type") {
    // Simple types
    STATIC_REQUIRE(edge_value_type<int>);
    STATIC_REQUIRE(edge_value_type<size_t>);

    // Pair types
    STATIC_REQUIRE(edge_value_type<std::pair<int, double>>);
    STATIC_REQUIRE(edge_value_type<std::pair<size_t, std::string>>);

    // Tuple types
    STATIC_REQUIRE(edge_value_type<std::tuple<int>>);
    STATIC_REQUIRE(edge_value_type<std::tuple<int, double>>);
    STATIC_REQUIRE(edge_value_type<std::tuple<int, double, std::string>>);

    // Custom types
    STATIC_REQUIRE(edge_value_type<CustomEdge>);
  }
}

// =============================================================================
// Edge Value Pattern Trait Tests
// =============================================================================

TEST_CASE("edge_value_pattern correctly identifies type patterns", "[traits][edge_types]") {
  SECTION("Simple edge types") {
    using Pattern = edge_value_pattern<int>;
    STATIC_REQUIRE(Pattern::is_simple);
    STATIC_REQUIRE_FALSE(Pattern::is_pair);
    STATIC_REQUIRE_FALSE(Pattern::is_tuple);
    STATIC_REQUIRE_FALSE(Pattern::is_custom);
  }

  SECTION("Pair edge types") {
    using Pattern = edge_value_pattern<std::pair<int, double>>;
    STATIC_REQUIRE_FALSE(Pattern::is_simple);
    STATIC_REQUIRE(Pattern::is_pair);
    STATIC_REQUIRE_FALSE(Pattern::is_tuple);
    STATIC_REQUIRE_FALSE(Pattern::is_custom);
  }

  SECTION("Tuple edge types") {
    using Pattern = edge_value_pattern<std::tuple<int, double, std::string>>;
    STATIC_REQUIRE_FALSE(Pattern::is_simple);
    STATIC_REQUIRE_FALSE(Pattern::is_pair);
    STATIC_REQUIRE(Pattern::is_tuple);
    STATIC_REQUIRE_FALSE(Pattern::is_custom);
  }

  SECTION("Custom edge types") {
    using Pattern = edge_value_pattern<CustomEdge>;
    STATIC_REQUIRE_FALSE(Pattern::is_simple);
    STATIC_REQUIRE_FALSE(Pattern::is_pair);
    STATIC_REQUIRE_FALSE(Pattern::is_tuple);
    STATIC_REQUIRE(Pattern::is_custom);
  }
}

// =============================================================================
// Edge Pattern Enum Tests
// =============================================================================

TEST_CASE("edge_pattern_type_v returns correct enum values", "[traits][edge_types]") {
  SECTION("Simple types return edge_pattern::simple") {
    STATIC_REQUIRE(edge_pattern_type_v<int> == edge_pattern::simple);
    STATIC_REQUIRE(edge_pattern_type_v<size_t> == edge_pattern::simple);
  }

  SECTION("Pair types return edge_pattern::pair") {
    STATIC_REQUIRE(edge_pattern_type_v<std::pair<int, double>> == edge_pattern::pair);
    STATIC_REQUIRE(edge_pattern_type_v<std::pair<size_t, float>> == edge_pattern::pair);
  }

  SECTION("Tuple types return edge_pattern::tuple") {
    STATIC_REQUIRE(edge_pattern_type_v<std::tuple<int>> == edge_pattern::tuple);
    STATIC_REQUIRE(edge_pattern_type_v<std::tuple<int, double>> == edge_pattern::tuple);
  }

  SECTION("Custom types return edge_pattern::custom") {
    STATIC_REQUIRE(edge_pattern_type_v<CustomEdge> == edge_pattern::custom);
  }
}

// =============================================================================
// Mutual Exclusivity Tests
// =============================================================================

TEST_CASE("Edge type patterns are mutually exclusive", "[concepts][edge_types]") {
  SECTION("Each type matches exactly one pattern") {
    // Simple type
    STATIC_REQUIRE(simple_edge_type<int>);
    STATIC_REQUIRE_FALSE(pair_edge_type<int>);
    STATIC_REQUIRE_FALSE(tuple_edge_type<int>);
    STATIC_REQUIRE_FALSE(custom_edge_type<int>);

    // Pair type
    using PairType = std::pair<int, double>;
    STATIC_REQUIRE_FALSE(simple_edge_type<PairType>);
    STATIC_REQUIRE(pair_edge_type<PairType>);
    STATIC_REQUIRE_FALSE(tuple_edge_type<PairType>);
    STATIC_REQUIRE_FALSE(custom_edge_type<PairType>);

    // Tuple type
    using TupleType = std::tuple<int, double>;
    STATIC_REQUIRE_FALSE(simple_edge_type<TupleType>);
    STATIC_REQUIRE_FALSE(pair_edge_type<TupleType>);
    STATIC_REQUIRE(tuple_edge_type<TupleType>);
    STATIC_REQUIRE_FALSE(custom_edge_type<TupleType>);

    // Custom type
    STATIC_REQUIRE_FALSE(simple_edge_type<CustomEdge>);
    STATIC_REQUIRE_FALSE(pair_edge_type<CustomEdge>);
    STATIC_REQUIRE_FALSE(tuple_edge_type<CustomEdge>);
    STATIC_REQUIRE(custom_edge_type<CustomEdge>);
  }
}

// =============================================================================
// Runtime Pattern Detection Tests
// =============================================================================

TEST_CASE("Pattern detection works at runtime", "[traits][edge_types]") {
  SECTION("Can query pattern at runtime") {
    auto simple_pattern = edge_value_pattern_v<int>;
    REQUIRE(simple_pattern.is_simple);
    REQUIRE_FALSE(simple_pattern.is_pair);

    auto pair_pattern = edge_value_pattern_v<std::pair<int, double>>;
    REQUIRE(pair_pattern.is_pair);
    REQUIRE_FALSE(pair_pattern.is_simple);

    auto tuple_pattern = edge_value_pattern_v<std::tuple<int, double>>;
    REQUIRE(tuple_pattern.is_tuple);
    REQUIRE_FALSE(tuple_pattern.is_pair);

    auto custom_pattern = edge_value_pattern_v<CustomEdge>;
    REQUIRE(custom_pattern.is_custom);
    REQUIRE_FALSE(custom_pattern.is_tuple);
  }
}
