/**
 * @file test_dynamic_edge_comparison.cpp
 * @brief Tests for dynamic_out_edge and dynamic_in_edge comparison operators and std::hash
 *
 * Phase 4.1 / Phase 5 refactor: Set Container Support Prerequisites
 * Tests operator<=>, operator==, and std::hash for:
 *   - dynamic_out_edge (out-edges: compare by target_id)
 *   - dynamic_in_edge  (in-edges:  compare by source_id)
 *
 * These operators are required for using edges with std::set (ordered)
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

//==============================================================================
// Type aliases — dynamic_out_edge (out-edges, compare by target_id)
//==============================================================================

// EV != void
using edge_ev_out =
      dynamic_out_edge<int, void, void, uint32_t, false, vov_graph_traits<int, void, void, uint32_t>>;

// EV = void
using edge_void_out =
      dynamic_out_edge<void, void, void, uint32_t, false, vov_graph_traits<void, void, void, uint32_t>>;

//==============================================================================
// Type aliases — dynamic_in_edge (in-edges, compare by source_id)
//==============================================================================

// EV != void
using edge_ev_in =
      dynamic_in_edge<int, void, void, uint32_t, false, vov_graph_traits<int, void, void, uint32_t>>;

// EV = void
using edge_void_in =
      dynamic_in_edge<void, void, void, uint32_t, false, vov_graph_traits<void, void, void, uint32_t>>;

//==============================================================================
// 1. operator<=> Tests - dynamic_out_edge (compare by target_id only)
//==============================================================================

TEST_CASE("dynamic_out_edge operator<=> EV!=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_ev_out e1(2, 100);
    edge_ev_out e2(2, 200); // Different value, same target_id

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
    REQUIRE(!(e1 < e2));
    REQUIRE(!(e1 > e2));
    REQUIRE(e1 <= e2);
    REQUIRE(e1 >= e2);
  }

  SECTION("less by target_id") {
    edge_ev_out e1(2, 100);
    edge_ev_out e2(5, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::less);
    REQUIRE(e1 < e2);
    REQUIRE(!(e1 > e2));
    REQUIRE(e1 <= e2);
    REQUIRE(!(e1 >= e2));
  }

  SECTION("greater by target_id") {
    edge_ev_out e1(7, 100);
    edge_ev_out e2(3, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::greater);
    REQUIRE(!(e1 < e2));
    REQUIRE(e1 > e2);
    REQUIRE(!(e1 <= e2));
    REQUIRE(e1 >= e2);
  }
}

TEST_CASE("dynamic_out_edge operator<=> EV=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_void_out e1(5);
    edge_void_out e2(5);

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("ordering by target_id") {
    edge_void_out e1(3);
    edge_void_out e2(7);

    REQUIRE(e1 < e2);
  }
}

//==============================================================================
// 2. operator<=> Tests - dynamic_in_edge (compare by source_id only)
//==============================================================================

TEST_CASE("dynamic_in_edge operator<=> EV!=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_ev_in e1(1, 100);
    edge_ev_in e2(1, 200); // Different value, same source_id

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
    REQUIRE(!(e1 < e2));
    REQUIRE(!(e1 > e2));
    REQUIRE(e1 <= e2);
    REQUIRE(e1 >= e2);
  }

  SECTION("less by source_id") {
    edge_ev_in e1(1, 100);
    edge_ev_in e2(2, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::less);
    REQUIRE(e1 < e2);
    REQUIRE(!(e1 > e2));
    REQUIRE(e1 <= e2);
    REQUIRE(!(e1 >= e2));
  }

  SECTION("greater by source_id") {
    edge_ev_in e1(3, 100);
    edge_ev_in e2(2, 100);

    REQUIRE((e1 <=> e2) == std::strong_ordering::greater);
    REQUIRE(!(e1 < e2));
    REQUIRE(e1 > e2);
    REQUIRE(!(e1 <= e2));
    REQUIRE(e1 >= e2);
  }
}

TEST_CASE("dynamic_in_edge operator<=> EV=void", "[edge][comparison][spaceship]") {
  SECTION("equal edges") {
    edge_void_in e1(2);
    edge_void_in e2(2);

    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("ordering by source_id") {
    edge_void_in e1(1);
    edge_void_in e2(2);

    REQUIRE(e1 < e2); // source_id 1 < 2
  }

  SECTION("reverse order") {
    edge_void_in e1(3);
    edge_void_in e2(1);

    REQUIRE(e1 > e2);
  }
}

//==============================================================================
// 3. operator== Tests - dynamic_out_edge
//==============================================================================

TEST_CASE("dynamic_out_edge operator==", "[edge][comparison][equality]") {
  SECTION("EV != void - equal edges with different values") {
    edge_ev_out e1(2, 100);
    edge_ev_out e2(2, 999); // Different value

    REQUIRE(e1 == e2); // compares only target_id
  }

  SECTION("EV != void - unequal by target_id") {
    edge_ev_out e1(2, 100);
    edge_ev_out e2(5, 100);

    REQUIRE(!(e1 == e2));
    REQUIRE(e1 != e2);
  }

  SECTION("EV = void - equal edges") {
    edge_void_out e1(5);
    edge_void_out e2(5);

    REQUIRE(e1 == e2);
  }

  SECTION("EV = void - unequal edges") {
    edge_void_out e1(5);
    edge_void_out e2(7);

    REQUIRE(e1 != e2);
  }
}

//==============================================================================
// 4. operator== Tests - dynamic_in_edge
//==============================================================================

TEST_CASE("dynamic_in_edge operator==", "[edge][comparison][equality]") {
  SECTION("EV != void - equal edges with different values") {
    edge_ev_in e1(1, 100);
    edge_ev_in e2(1, 999); // Different value

    REQUIRE(e1 == e2); // compares only source_id
  }

  SECTION("EV != void - unequal by source_id") {
    edge_ev_in e1(1, 100);
    edge_ev_in e2(3, 100);

    REQUIRE(!(e1 == e2));
    REQUIRE(e1 != e2);
  }

  SECTION("EV = void - equal edges") {
    edge_void_in e1(2);
    edge_void_in e2(2);

    REQUIRE(e1 == e2);
  }

  SECTION("EV = void - unequal edges") {
    edge_void_in e1(2);
    edge_void_in e2(3);

    REQUIRE(e1 != e2);
  }
}

//==============================================================================
// 5. std::hash Tests - dynamic_out_edge
//==============================================================================

TEST_CASE("std::hash for dynamic_out_edge", "[edge][hash]") {
  SECTION("EV != void - equal edges have same hash") {
    edge_ev_out e1(2, 100);
    edge_ev_out e2(2, 999); // Different value

    std::hash<edge_ev_out> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }

  SECTION("EV != void - different target_ids likely have different hash") {
    edge_ev_out e1(2, 100);
    edge_ev_out e2(5, 100);

    std::hash<edge_ev_out> hasher;
    REQUIRE(hasher(e1) != hasher(e2));
  }

  SECTION("EV = void - equal edges have same hash") {
    edge_void_out e1(5);
    edge_void_out e2(5);

    std::hash<edge_void_out> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }

  SECTION("EV = void - different edges likely have different hash") {
    edge_void_out e1(5);
    edge_void_out e2(7);

    std::hash<edge_void_out> hasher;
    REQUIRE(hasher(e1) != hasher(e2));
  }
}

//==============================================================================
// 6. std::hash Tests - dynamic_in_edge
//==============================================================================

TEST_CASE("std::hash for dynamic_in_edge", "[edge][hash]") {
  SECTION("EV != void - equal edges have same hash") {
    edge_ev_in e1(1, 100);
    edge_ev_in e2(1, 999); // Different value

    std::hash<edge_ev_in> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }

  SECTION("EV != void - different source_ids likely have different hash") {
    edge_ev_in e1(1, 100);
    edge_ev_in e2(3, 100);

    std::hash<edge_ev_in> hasher;
    REQUIRE(hasher(e1) != hasher(e2));
  }

  SECTION("EV = void - equal edges have same hash") {
    edge_void_in e1(2);
    edge_void_in e2(2);

    std::hash<edge_void_in> hasher;
    REQUIRE(hasher(e1) == hasher(e2));
  }
}

//==============================================================================
// 7. Integration with std::set (requires operator<=>)
//==============================================================================

TEST_CASE("dynamic_out_edge works with std::set", "[edge][set][integration]") {
  SECTION("deduplicates by target_id") {
    std::set<edge_ev_out> edge_set;

    edge_set.insert(edge_ev_out(2, 100));
    edge_set.insert(edge_ev_out(2, 999)); // Duplicate (same target_id)
    edge_set.insert(edge_ev_out(5, 100));
    edge_set.insert(edge_ev_out(3, 100));

    REQUIRE(edge_set.size() == 3);
  }

  SECTION("maintains sorted order by target_id") {
    std::set<edge_void_out> edge_set;

    edge_set.insert(edge_void_out(5));
    edge_set.insert(edge_void_out(2));
    edge_set.insert(edge_void_out(8));
    edge_set.insert(edge_void_out(1));

    std::vector<uint32_t> expected = {1, 2, 5, 8};

    size_t i = 0;
    for (const auto& e : edge_set) {
      REQUIRE(e.target_id() == expected[i]);
      ++i;
    }
  }
}

TEST_CASE("dynamic_in_edge works with std::set", "[edge][set][integration]") {
  SECTION("deduplicates by source_id") {
    std::set<edge_ev_in> edge_set;

    edge_set.insert(edge_ev_in(1, 100));
    edge_set.insert(edge_ev_in(1, 999)); // Duplicate (same source_id)
    edge_set.insert(edge_ev_in(2, 100));
    edge_set.insert(edge_ev_in(3, 100));

    REQUIRE(edge_set.size() == 3);
  }

  SECTION("maintains sorted order by source_id") {
    std::set<edge_void_in> edge_set;

    edge_set.insert(edge_void_in(3));
    edge_set.insert(edge_void_in(1));
    edge_set.insert(edge_void_in(4));
    edge_set.insert(edge_void_in(2));

    std::vector<uint32_t> expected = {1, 2, 3, 4};

    size_t i = 0;
    for (const auto& e : edge_set) {
      REQUIRE(e.source_id() == expected[i]);
      ++i;
    }
  }
}

//==============================================================================
// 8. Integration with std::unordered_set
//==============================================================================

TEST_CASE("dynamic_out_edge works with std::unordered_set",
          "[edge][unordered_set][integration]") {
  SECTION("deduplicates by target_id") {
    std::unordered_set<edge_ev_out> edge_set;

    edge_set.insert(edge_ev_out(2, 100));
    edge_set.insert(edge_ev_out(2, 999)); // Duplicate
    edge_set.insert(edge_ev_out(5, 100));

    REQUIRE(edge_set.size() == 2);
  }

  SECTION("find works correctly") {
    std::unordered_set<edge_void_out> edge_set;

    edge_set.insert(edge_void_out(3));
    edge_set.insert(edge_void_out(7));

    REQUIRE(edge_set.find(edge_void_out(3)) != edge_set.end());
    REQUIRE(edge_set.find(edge_void_out(5)) == edge_set.end());
  }
}

TEST_CASE("dynamic_in_edge works with std::unordered_set",
          "[edge][unordered_set][integration]") {
  SECTION("deduplicates by source_id") {
    std::unordered_set<edge_ev_in> edge_set;

    edge_set.insert(edge_ev_in(1, 100));
    edge_set.insert(edge_ev_in(1, 999)); // Duplicate
    edge_set.insert(edge_ev_in(3, 100));

    REQUIRE(edge_set.size() == 2);
  }

  SECTION("find works correctly") {
    std::unordered_set<edge_void_in> edge_set;

    edge_set.insert(edge_void_in(1));
    edge_set.insert(edge_void_in(2));

    REQUIRE(edge_set.find(edge_void_in(1)) != edge_set.end());
    REQUIRE(edge_set.find(edge_void_in(5)) == edge_set.end());
  }
}

//==============================================================================
// 9. Edge case tests
//==============================================================================

TEST_CASE("dynamic edge comparison edge cases", "[edge][comparison][edge-cases]") {
  SECTION("default constructed out-edges are equal") {
    edge_void_out e1;
    edge_void_out e2;

    REQUIRE(e1 == e2);
    REQUIRE((e1 <=> e2) == std::strong_ordering::equal);
  }

  SECTION("out-edge with target_id 0") {
    edge_void_out e1(0);
    edge_void_out e2(0);

    REQUIRE(e1 == e2);
    REQUIRE(std::hash<edge_void_out>{}(e1) == std::hash<edge_void_out>{}(e2));
  }

  SECTION("in-edge with source_id 0") {
    edge_void_in e1(0);
    edge_void_in e2(0);

    REQUIRE(e1 == e2);
    REQUIRE(std::hash<edge_void_in>{}(e1) == std::hash<edge_void_in>{}(e2));
  }

  SECTION("out-edge large vertex ids") {
    uint32_t      max_id = std::numeric_limits<uint32_t>::max();
    edge_void_out e1(max_id);
    edge_void_out e2(max_id);

    REQUIRE(e1 == e2);
    REQUIRE(std::hash<edge_void_out>{}(e1) == std::hash<edge_void_out>{}(e2));
  }

  SECTION("in-edge large vertex ids") {
    uint32_t     max_id = std::numeric_limits<uint32_t>::max();
    edge_void_in e1(max_id);
    edge_void_in e2(max_id);

    REQUIRE(e1 == e2);
    REQUIRE(std::hash<edge_void_in>{}(e1) == std::hash<edge_void_in>{}(e2));
  }
}
