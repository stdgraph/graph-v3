#include <catch2/catch_test_macros.hpp>
#include <graph/views/search_base.hpp>

using namespace graph::views;

TEST_CASE("cancel_search enum values", "[views][search_base]") {
  SECTION("enum has correct values") {
    auto continue_val = cancel_search::continue_search;
    auto branch_val   = cancel_search::cancel_branch;
    auto all_val      = cancel_search::cancel_all;

    // Ensure they're distinct
    REQUIRE(continue_val != branch_val);
    REQUIRE(continue_val != all_val);
    REQUIRE(branch_val != all_val);
  }

  SECTION("can assign and compare") {
    cancel_search cs = cancel_search::continue_search;
    REQUIRE(cs == cancel_search::continue_search);

    cs = cancel_search::cancel_branch;
    REQUIRE(cs == cancel_search::cancel_branch);

    cs = cancel_search::cancel_all;
    REQUIRE(cs == cancel_search::cancel_all);
  }
}

TEST_CASE("visited_tracker basic functionality", "[views][search_base]") {
  SECTION("construction with size") {
    visited_tracker<std::size_t> tracker(10);
    REQUIRE(tracker.size() == 10);
  }

  SECTION("initial state is unvisited") {
    visited_tracker<std::size_t> tracker(5);
    for (std::size_t i = 0; i < 5; ++i) {
      REQUIRE_FALSE(tracker.is_visited(i));
    }
  }

  SECTION("mark_visited and is_visited") {
    visited_tracker<std::size_t> tracker(10);

    REQUIRE_FALSE(tracker.is_visited(3));
    tracker.mark_visited(3);
    REQUIRE(tracker.is_visited(3));

    // Other vertices remain unvisited
    REQUIRE_FALSE(tracker.is_visited(2));
    REQUIRE_FALSE(tracker.is_visited(4));
  }

  SECTION("multiple visits") {
    visited_tracker<std::size_t> tracker(10);

    tracker.mark_visited(0);
    tracker.mark_visited(5);
    tracker.mark_visited(9);

    REQUIRE(tracker.is_visited(0));
    REQUIRE(tracker.is_visited(5));
    REQUIRE(tracker.is_visited(9));
    REQUIRE_FALSE(tracker.is_visited(1));
    REQUIRE_FALSE(tracker.is_visited(4));
    REQUIRE_FALSE(tracker.is_visited(8));
  }
}

TEST_CASE("visited_tracker reset", "[views][search_base]") {
  visited_tracker<std::size_t> tracker(10);

  // Mark several as visited
  tracker.mark_visited(2);
  tracker.mark_visited(5);
  tracker.mark_visited(7);

  REQUIRE(tracker.is_visited(2));
  REQUIRE(tracker.is_visited(5));
  REQUIRE(tracker.is_visited(7));

  // Reset
  tracker.reset();

  // All should be unvisited now
  for (std::size_t i = 0; i < 10; ++i) {
    REQUIRE_FALSE(tracker.is_visited(i));
  }
}

TEST_CASE("visited_tracker with different VId types", "[views][search_base]") {
  SECTION("size_t") {
    visited_tracker<std::size_t> tracker(5);
    tracker.mark_visited(std::size_t{2});
    REQUIRE(tracker.is_visited(std::size_t{2}));
  }

  SECTION("int") {
    visited_tracker<int> tracker(5);
    tracker.mark_visited(2);
    REQUIRE(tracker.is_visited(2));
  }

  SECTION("unsigned int") {
    visited_tracker<unsigned int> tracker(5);
    tracker.mark_visited(2u);
    REQUIRE(tracker.is_visited(2u));
  }

  SECTION("uint32_t") {
    visited_tracker<std::uint32_t> tracker(5);
    tracker.mark_visited(std::uint32_t{2});
    REQUIRE(tracker.is_visited(std::uint32_t{2}));
  }
}

TEST_CASE("visited_tracker edge cases", "[views][search_base]") {
  SECTION("empty tracker") {
    visited_tracker<std::size_t> tracker(0);
    REQUIRE(tracker.size() == 0);
  }

  SECTION("single vertex") {
    visited_tracker<std::size_t> tracker(1);
    REQUIRE(tracker.size() == 1);
    REQUIRE_FALSE(tracker.is_visited(0));

    tracker.mark_visited(0);
    REQUIRE(tracker.is_visited(0));
  }

  SECTION("large graph") {
    visited_tracker<std::size_t> tracker(10000);
    REQUIRE(tracker.size() == 10000);

    tracker.mark_visited(0);
    tracker.mark_visited(5000);
    tracker.mark_visited(9999);

    REQUIRE(tracker.is_visited(0));
    REQUIRE(tracker.is_visited(5000));
    REQUIRE(tracker.is_visited(9999));
    REQUIRE_FALSE(tracker.is_visited(1));
    REQUIRE_FALSE(tracker.is_visited(5001));
  }

  SECTION("visit all vertices") {
    visited_tracker<std::size_t> tracker(10);
    for (std::size_t i = 0; i < 10; ++i) {
      tracker.mark_visited(i);
    }

    for (std::size_t i = 0; i < 10; ++i) {
      REQUIRE(tracker.is_visited(i));
    }
  }
}

TEST_CASE("visited_tracker with custom allocator", "[views][search_base]") {
  // Using default allocator explicitly
  std::allocator<bool> alloc;
  visited_tracker<std::size_t, std::allocator<bool>> tracker(5, alloc);

  REQUIRE(tracker.size() == 5);
  tracker.mark_visited(2);
  REQUIRE(tracker.is_visited(2));
}
