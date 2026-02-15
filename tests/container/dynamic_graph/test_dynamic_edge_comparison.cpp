/**
 * @file test_dynamic_edge_comparison.cpp
 * @brief Tests for dynamic_edge comparison operators and std::hash
 * 
 * Phase 4.1: Set Container Support Prerequisites
 * Tests operator<=>, operator==, and std::hash for dynamic_edge
 * 
 * These operators are required for using dynamic_edge with std::set (ordered)
 * and std::unordered_set (unordered) edge containers.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <set>
#include <unordered_set>
#include <vector>
#include <algorithm>

using namespace graph::container;

//==================================================================================================
// Test edge type definitions for all 4 specializations
//==================================================================================================

// EV != void, Sourced = true (primary template)
using edge_ev_sourced =
      dynamic_edge<int, void, void, uint32_t, true, vov_graph_traits<int, void, void, uint32_t, true>>;

// EV = void, Sourced = true
using edge_void_sourced =
      dynamic_edge<void, void, void, uint32_t, true, vov_graph_traits<void, void, void, uint32_t, true>>;

// EV != void, Sourced = false
using edge_ev_unsourced =
      dynamic_edge<int, void, void, uint32_t, false, vov_graph_traits<int, void, void, uint32_t, false>>;

// EV = void, Sourced = false
using edge_void_unsourced =
      dynamic_edge<void, void, void, uint32_t, false, vov_graph_traits<void, void, void, uint32_t, false>>;

//==================================================================================================
// 1. operator<=> Tests - Sourced edges (compare by source_id, then target_id)
//==================================================================================================

TEST_CASE("dynamic_edge operator<=> with Sourced=true, EV!=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(1, 2, 200); // Different value, same ids

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
    REQUIRE(!(e1 < e2));
    REQUIRE(!(e1 > e2));
    REQUIRE(e1 <= e2);
    REQUIRE(e1 >= e2);
  }

  SECTION("less by source_id") {
    edge_ev_sourced e1(1, 5, 100);
    edge_ev_sourced e2(2, 3, 100); // Larger source_id but smaller target_id

    REQUIRE((e1 <=> e2) == std::strong_ordering::less);
    REQUIRE(e1 < e2);
    REQUIRE(!(e1 > e2));
    REQUIRE(e1 <= e2);
    REQUIRE(!(e1 >= e2));
  }

  SECTION("greater by source_id") {
    edge_ev_sourced e1(3, 1, 100);
    edge_ev_sourced e2(2, 5, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::greater);
    REQUIRE(!(e1 < e2));
    REQUIRE(e1 > e2);
    REQUIRE(!(e1 <= e2));
    REQUIRE(e1 >= e2);
  }

  SECTION("same source_id, less by target_id") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(1, 3, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::less);
    REQUIRE(e1 < e2);
  }

  SECTION("same source_id, greater by target_id") {
    edge_ev_sourced e1(1, 5, 100);
    edge_ev_sourced e2(1, 3, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::greater);
    REQUIRE(e1 > e2);
  }
}

TEST_CASE("dynamic_edge operator<=> with Sourced=true, EV=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_void_sourced e1(1, 2);
    edge_void_sourced e2(1, 2);

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("ordering by source_id first, then target_id") {
    edge_void_sourced e1(1, 2);
    edge_void_sourced e2(2, 1);

    REQUIRE(e1 < e2); // source_id 1 < 2
  }

  SECTION("same source, different target") {
    edge_void_sourced e1(1, 3);
    edge_void_sourced e2(1, 2);

    REQUIRE(e1 > e2); // target_id 3 > 2
  }
}

//==================================================================================================
// 2. operator<=> Tests - Unsourced edges (compare by target_id only)
//==================================================================================================

TEST_CASE("dynamic_edge operator<=> with Sourced=false, EV!=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_ev_unsourced e1(2, 100);
    edge_ev_unsourced e2(2, 200); // Different value, same target_id

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("less by target_id") {
    edge_ev_unsourced e1(2, 100);
    edge_ev_unsourced e2(5, 100);

    REQUIRE(e1 < e2);
  }

  SECTION("greater by target_id") {
    edge_ev_unsourced e1(7, 100);
    edge_ev_unsourced e2(3, 100);

    REQUIRE(e1 > e2);
  }
}

TEST_CASE("dynamic_edge operator<=> with Sourced=false, EV=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_void_unsourced e1(5);
    edge_void_unsourced e2(5);

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("ordering by target_id") {
    edge_void_unsourced e1(3);
    edge_void_unsourced e2(7);

    REQUIRE(e1 < e2);
  }
}

//==================================================================================================
// 3. operator== Tests
//==================================================================================================

TEST_CASE("dynamic_edge operator== with Sourced=true", "[edge][comparison][equality]") {
  SECTION("EV != void - equal edges with different values") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(1, 2, 999); // Different value

    REQUIRE(e1 == e2); // Compares only source_id and target_id
  }

  SECTION("EV != void - unequal by source_id") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(3, 2, 100);

    REQUIRE(!(e1 == e2));
    REQUIRE(e1 != e2);
  }

  SECTION("EV != void - unequal by target_id") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(1, 5, 100);

    REQUIRE(!(e1 == e2));
    REQUIRE(e1 != e2);
  }

  SECTION("EV = void - equal edges") {
    edge_void_sourced e1(1, 2);
    edge_void_sourced e2(1, 2);

    REQUIRE(e1 == e2);
  }

  SECTION("EV = void - unequal edges") {
    edge_void_sourced e1(1, 2);
    edge_void_sourced e2(1, 3);

    REQUIRE(e1 != e2);
  }
}

TEST_CASE("dynamic_edge operator== with Sourced=false", "[edge][comparison][equality]") {
  SECTION("EV != void - equal edges with different values") {
    edge_ev_unsourced e1(2, 100);
    edge_ev_unsourced e2(2, 999);

    REQUIRE(e1 == e2);
  }

  SECTION("EV != void - unequal by target_id") {
    edge_ev_unsourced e1(2, 100);
    edge_ev_unsourced e2(5, 100);

    REQUIRE(e1 != e2);
  }

  SECTION("EV = void - equal edges") {
    edge_void_unsourced e1(5);
    edge_void_unsourced e2(5);

    REQUIRE(e1 == e2);
  }

  SECTION("EV = void - unequal edges") {
    edge_void_unsourced e1(5);
    edge_void_unsourced e2(7);

    REQUIRE(e1 != e2);
  }
}

//==================================================================================================
// 4. std::hash Tests
//==================================================================================================

TEST_CASE("std::hash for dynamic_edge with Sourced=true", "[edge][hash]") {
  SECTION("EV != void - equal edges have same hash") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(1, 2, 999); // Different value

    std::hash<edge_ev_sourced> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }

  SECTION("EV != void - different edges likely have different hash") {
    edge_ev_sourced e1(1, 2, 100);
    edge_ev_sourced e2(1, 3, 100); // Different target
    edge_ev_sourced e3(2, 2, 100); // Different source

    std::hash<edge_ev_sourced> hasher;
    // Note: Different hash is not guaranteed but highly likely
    REQUIRE(hasher(e1) != hasher(e2));
    REQUIRE(hasher(e1) != hasher(e3));
  }

  SECTION("EV = void - equal edges have same hash") {
    edge_void_sourced e1(1, 2);
    edge_void_sourced e2(1, 2);

    std::hash<edge_void_sourced> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }
}

TEST_CASE("std::hash for dynamic_edge with Sourced=false", "[edge][hash]") {
  SECTION("EV != void - equal edges have same hash") {
    edge_ev_unsourced e1(2, 100);
    edge_ev_unsourced e2(2, 999);

    std::hash<edge_ev_unsourced> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }

  SECTION("EV = void - equal edges have same hash") {
    edge_void_unsourced e1(5);
    edge_void_unsourced e2(5);

    std::hash<edge_void_unsourced> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }

  SECTION("EV = void - different edges likely have different hash") {
    edge_void_unsourced e1(5);
    edge_void_unsourced e2(7);

    std::hash<edge_void_unsourced> hasher;
    REQUIRE(hasher(e1) != hasher(e2));
  }
}

//==================================================================================================
// 5. Integration with std::set (requires operator<=>)
//==================================================================================================

TEST_CASE("dynamic_edge works with std::set", "[edge][set][integration]") {
  SECTION("Sourced edges - deduplicates by source_id and target_id") {
    std::set<edge_ev_sourced> edge_set;

    edge_set.insert(edge_ev_sourced(1, 2, 100));
    edge_set.insert(edge_ev_sourced(1, 2, 999)); // Duplicate (same ids)
    edge_set.insert(edge_ev_sourced(1, 3, 100));
    edge_set.insert(edge_ev_sourced(2, 1, 100));

    REQUIRE(edge_set.size() == 3); // Only 3 unique edges
  }

  SECTION("Sourced edges - maintains sorted order") {
    std::set<edge_void_sourced> edge_set;

    edge_set.insert(edge_void_sourced(2, 3));
    edge_set.insert(edge_void_sourced(1, 2));
    edge_set.insert(edge_void_sourced(1, 3));
    edge_set.insert(edge_void_sourced(2, 1));

    std::vector<std::pair<uint32_t, uint32_t>> expected = {{1, 2}, {1, 3}, {2, 1}, {2, 3}};

    size_t i = 0;
    for (const auto& e : edge_set) {
      REQUIRE(e.source_id() == expected[i].first);
      REQUIRE(e.target_id() == expected[i].second);
      ++i;
    }
  }

  SECTION("Unsourced edges - deduplicates by target_id") {
    std::set<edge_ev_unsourced> edge_set;

    edge_set.insert(edge_ev_unsourced(2, 100));
    edge_set.insert(edge_ev_unsourced(2, 999)); // Duplicate
    edge_set.insert(edge_ev_unsourced(5, 100));
    edge_set.insert(edge_ev_unsourced(3, 100));

    REQUIRE(edge_set.size() == 3);
  }

  SECTION("Unsourced edges - maintains sorted order") {
    std::set<edge_void_unsourced> edge_set;

    edge_set.insert(edge_void_unsourced(5));
    edge_set.insert(edge_void_unsourced(2));
    edge_set.insert(edge_void_unsourced(8));
    edge_set.insert(edge_void_unsourced(1));

    std::vector<uint32_t> expected = {1, 2, 5, 8};

    size_t i = 0;
    for (const auto& e : edge_set) {
      REQUIRE(e.target_id() == expected[i]);
      ++i;
    }
  }
}

//==================================================================================================
// 6. Integration with std::unordered_set (requires operator== and std::hash)
//==================================================================================================

TEST_CASE("dynamic_edge works with std::unordered_set", "[edge][unordered_set][integration]") {
  SECTION("Sourced edges - deduplicates by source_id and target_id") {
    std::unordered_set<edge_ev_sourced> edge_set;

    edge_set.insert(edge_ev_sourced(1, 2, 100));
    edge_set.insert(edge_ev_sourced(1, 2, 999)); // Duplicate
    edge_set.insert(edge_ev_sourced(1, 3, 100));
    edge_set.insert(edge_ev_sourced(2, 1, 100));

    REQUIRE(edge_set.size() == 3);
  }

  SECTION("Sourced edges - find works correctly") {
    std::unordered_set<edge_void_sourced> edge_set;

    edge_set.insert(edge_void_sourced(1, 2));
    edge_set.insert(edge_void_sourced(2, 3));

    REQUIRE(edge_set.find(edge_void_sourced(1, 2)) != edge_set.end());
    REQUIRE(edge_set.find(edge_void_sourced(1, 5)) == edge_set.end());
  }

  SECTION("Unsourced edges - deduplicates by target_id") {
    std::unordered_set<edge_ev_unsourced> edge_set;

    edge_set.insert(edge_ev_unsourced(2, 100));
    edge_set.insert(edge_ev_unsourced(2, 999)); // Duplicate
    edge_set.insert(edge_ev_unsourced(5, 100));

    REQUIRE(edge_set.size() == 2);
  }

  SECTION("Unsourced edges - find works correctly") {
    std::unordered_set<edge_void_unsourced> edge_set;

    edge_set.insert(edge_void_unsourced(3));
    edge_set.insert(edge_void_unsourced(7));

    REQUIRE(edge_set.find(edge_void_unsourced(3)) != edge_set.end());
    REQUIRE(edge_set.find(edge_void_unsourced(5)) == edge_set.end());
  }
}

//==================================================================================================
// 7. Edge case tests
//==================================================================================================

TEST_CASE("dynamic_edge comparison edge cases", "[edge][comparison][edge-cases]") {
  SECTION("default constructed edges are equal") {
    edge_void_unsourced e1;
    edge_void_unsourced e2;

    REQUIRE(e1 == e2);
    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("edge with id 0") {
    edge_void_unsourced e1(0);
    edge_void_unsourced e2(0);

    REQUIRE(e1 == e2);
    REQUIRE(std::hash<edge_void_unsourced>{}(e1) == std::hash<edge_void_unsourced>{}(e2));
  }

  SECTION("large vertex ids") {
    uint32_t          max_id = std::numeric_limits<uint32_t>::max();
    edge_void_sourced e1(max_id, max_id);
    edge_void_sourced e2(max_id, max_id);

    REQUIRE(e1 == e2);
    REQUIRE(std::hash<edge_void_sourced>{}(e1) == std::hash<edge_void_sourced>{}(e2));
  }

  SECTION("self-loop edges") {
    edge_void_sourced e1(5, 5);
    edge_void_sourced e2(5, 5);

    REQUIRE(e1 == e2);
  }

  SECTION("reverse edges are not equal for sourced") {
    edge_void_sourced e1(1, 2);
    edge_void_sourced e2(2, 1);

    REQUIRE(e1 != e2);
    REQUIRE(e1 < e2); // (1, 2) < (2, 1) by source_id
  }
}
