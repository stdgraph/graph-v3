/**
 * @file test_label_propagation.cpp
 * @brief Tests for label propagation algorithm from label_propagation.hpp
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/algorithm/label_propagation.hpp>
#include "../common/graph_fixtures.hpp"
#include "../common/algorithm_test_types.hpp"
#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <vector>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;
using namespace graph::test;
using namespace graph::test::fixtures;
using namespace graph::test::algorithm;

// =============================================================================
// Helper Functions
// =============================================================================

/// Returns true if every element of @p label is one of @p expected_labels.
template <typename Label, typename Set>
bool all_labelled(const Label& label, const Set& expected_labels) {
  for (auto&& lbl : label) {
    if (expected_labels.find(lbl) == expected_labels.end()) {
      return false;
    }
  }
  return true;
}

/// Returns true if every vertex shares the same label.
template <typename Label>
bool fully_converged(const Label& label) {
  if (label.empty())
    return true;
  auto first = label[0];
  return std::all_of(label.begin(), label.end(), [&](auto&& v) { return v == first; });
}

// =============================================================================
// Overload 1 — no empty_label
// =============================================================================

TEST_CASE("label_propagation - empty graph", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph            g;
  std::vector<int> label;
  std::mt19937     rng{42};

  // Should return without crash
  label_propagation(g, label, rng);

  REQUIRE(label.empty());
}

TEST_CASE("label_propagation - single vertex no edges", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph g;
  g.resize_vertices(1);
  std::vector<int> label = {7};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  REQUIRE(label[0] == 7); // unchanged — no neighbours
}

TEST_CASE("label_propagation - single edge two different labels", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Bidirectional to simulate undirected
  Graph g({{0, 1}, {1, 0}});

  std::vector<int> label = {10, 20};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // After convergence both vertices must share one of the original labels
  REQUIRE(label[0] == label[1]);
  REQUIRE((label[0] == 10 || label[0] == 20));
}

TEST_CASE("label_propagation - path graph all same label", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Bidirectional path: 0-1-2-3
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});

  std::vector<int> label = {5, 5, 5, 5};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // Already converged — should stay the same
  for (auto& l : label) {
    REQUIRE(l == 5);
  }
}

TEST_CASE("label_propagation - path graph alternating labels", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});

  std::vector<int> label = {0, 1, 0, 1};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // Should reach a stable result (all vertices have some valid label)
  std::set<int> valid = {0, 1};
  REQUIRE(all_labelled(label, valid));
}

TEST_CASE("label_propagation - cycle graph 5 vertices", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Bidirectional cycle: 0-1-2-3-4-0
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});

  std::vector<int> label = {0, 1, 2, 3, 4};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // All vertices should converge to a single label
  REQUIRE(fully_converged(label));
}

TEST_CASE("label_propagation - complete graph K4 majority wins", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // K4: every pair connected bidirectionally
  Graph g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}});

  // Label 99 is the majority (3 out of 4)
  std::vector<int> label = {99, 99, 99, 42};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // Majority label should win
  for (auto& l : label) {
    REQUIRE(l == 99);
  }
}

TEST_CASE("label_propagation - disconnected graph", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Component 1: 0-1 (bidirectional), Component 2: 2-3 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {2, 3}, {3, 2}});

  std::vector<int> label = {10, 20, 30, 40};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // Each component converges independently
  REQUIRE(label[0] == label[1]);
  REQUIRE(label[2] == label[3]);
  // Components may have different labels
  REQUIRE((label[0] == 10 || label[0] == 20));
  REQUIRE((label[2] == 30 || label[2] == 40));
}

TEST_CASE("label_propagation - max_iters = 0", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph g({{0, 1}, {1, 0}});

  std::vector<int> label = {10, 20};
  std::mt19937     rng{42};

  label_propagation(g, label, rng, size_t{0});

  // No iterations performed — labels unchanged
  REQUIRE(label[0] == 10);
  REQUIRE(label[1] == 20);
}

TEST_CASE("label_propagation - max_iters = 1", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Star graph: centre 0 connected to 1,2,3,4 (bidirectional)
  Graph g({{0, 1},
           {1, 0},
           {0, 2},
           {2, 0},
           {0, 3},
           {3, 0},
           {0, 4},
           {4, 0}});

  std::vector<int> label = {0, 1, 1, 1, 1};
  std::mt19937     rng{42};

  label_propagation(g, label, rng, size_t{1});

  // After exactly one round: centre 0 should adopt label 1 (majority of neighbours)
  // Leaves may or may not change depending on processing order, but centre should change.
  // We just verify the result has valid labels.
  std::set<int> valid = {0, 1};
  REQUIRE(all_labelled(label, valid));
}

TEST_CASE("label_propagation - all vertices same label", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}});

  std::vector<int> label = {3, 3, 3};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // Already converged — should stay unchanged
  for (auto& l : label) {
    REQUIRE(l == 3);
  }
}

TEST_CASE("label_propagation - tie breaking", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Triangle with three different labels — every vertex sees a tie
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}});

  std::vector<int> label = {10, 20, 30};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  // Result should be one of the original labels (not some arbitrary value)
  std::set<int> valid = {10, 20, 30};
  REQUIRE(all_labelled(label, valid));
  // Should converge to one label
  REQUIRE(fully_converged(label));
}

// =============================================================================
// Overload 2 — with empty_label sentinel
// =============================================================================

TEST_CASE("label_propagation - empty_label: all unlabelled", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}});

  std::vector<int> label = {-1, -1, -1};
  std::mt19937     rng{42};

  label_propagation(g, label, -1, rng);

  // All unlabelled, no source of labels — should remain -1
  for (auto& l : label) {
    REQUIRE(l == -1);
  }
}

TEST_CASE("label_propagation - empty_label: one labelled vertex propagates", "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Path: 0-1-2-3 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});

  std::vector<int> label = {42, -1, -1, -1};
  std::mt19937     rng{42};

  label_propagation(g, label, -1, rng);

  // Label should propagate outward from vertex 0
  for (auto& l : label) {
    REQUIRE(l == 42);
  }
}

TEST_CASE("label_propagation - empty_label: disconnected labelled + unlabelled",
          "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Component 1: 0-1 (bidirectional), labelled
  // Component 2: 2-3 (bidirectional), unlabelled
  Graph g({{0, 1}, {1, 0}, {2, 3}, {3, 2}});

  std::vector<int> label = {5, 5, -1, -1};
  std::mt19937     rng{42};

  label_propagation(g, label, -1, rng);

  // Component 1 stays labelled
  REQUIRE(label[0] == 5);
  REQUIRE(label[1] == 5);
  // Component 2 stays unlabelled — no source to propagate from
  REQUIRE(label[2] == -1);
  REQUIRE(label[3] == -1);
}

TEST_CASE("label_propagation - empty_label: mixed pre-labelled and unlabelled",
          "[algorithm][label_propagation]") {
  using Graph = vov_void;

  // Triangle 0-1-2, plus vertex 3 connected to 2 (bidirectional)
  Graph g({{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});

  std::vector<int> label = {7, -1, 7, -1};
  std::mt19937     rng{42};

  label_propagation(g, label, -1, rng);

  // All should acquire label 7
  for (auto& l : label) {
    REQUIRE(l == 7);
  }
}

TEST_CASE("label_propagation - empty_label: no empty labels present behaves like overload 1",
          "[algorithm][label_propagation]") {
  using Graph = vov_void;

  Graph g({{0, 1}, {1, 0}});

  // Use -1 as empty_label but no vertex actually has it
  std::vector<int> label1 = {10, 20};
  std::vector<int> label2 = {10, 20};
  std::mt19937     rng1{42};
  std::mt19937     rng2{42};

  label_propagation(g, label1, rng1);
  label_propagation(g, label2, -1, rng2);

  // Both overloads should produce the same result
  REQUIRE(label1 == label2);
}

// =============================================================================
// Parameterized Tests — container independence
// =============================================================================

TEMPLATE_TEST_CASE("label_propagation - single edge (typed)", "[algorithm][label_propagation]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}});

  std::vector<int> label = {10, 20};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  REQUIRE(label[0] == label[1]);
  REQUIRE((label[0] == 10 || label[0] == 20));
}

TEMPLATE_TEST_CASE("label_propagation - path graph (typed)", "[algorithm][label_propagation]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}});

  std::vector<int> label = {1, 2, 1, 2};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  std::set<int> valid = {1, 2};
  REQUIRE(all_labelled(label, valid));
}

TEMPLATE_TEST_CASE("label_propagation - cycle graph (typed)", "[algorithm][label_propagation]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {1, 2}, {2, 1}, {2, 3}, {3, 2}, {3, 4}, {4, 3}, {4, 0}, {0, 4}});

  std::vector<int> label = {0, 1, 2, 3, 4};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  REQUIRE(fully_converged(label));
}

TEMPLATE_TEST_CASE("label_propagation - disconnected graph (typed)", "[algorithm][label_propagation]",
                   vov_void, dov_void) {
  using Graph = TestType;

  Graph g({{0, 1}, {1, 0}, {2, 3}, {3, 2}});

  std::vector<int> label = {10, 20, 30, 40};
  std::mt19937     rng{42};

  label_propagation(g, label, rng);

  REQUIRE(label[0] == label[1]);
  REQUIRE(label[2] == label[3]);
}
