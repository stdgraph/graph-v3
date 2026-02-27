/**
 * @file test_edgelist.cpp
 * @brief Comprehensive tests for edgelist view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/edgelist.hpp>
#include <graph/views/vertexlist.hpp>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <algorithm>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Empty Graph
// =============================================================================

TEST_CASE("edgelist - empty graph", "[edgelist][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  SECTION("no value function - empty iteration") {
    auto elist = edgelist(g);

    REQUIRE(elist.begin() == elist.end());

    std::size_t count = 0;
    for ([[maybe_unused]] auto ei : elist) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("with value function - empty iteration") {
    auto elist = edgelist(g, [](const auto&, auto /*e*/) { return 42; });

    REQUIRE(elist.begin() == elist.end());
  }
}

// =============================================================================
// Test 2: Graph with Vertices but No Edges
// =============================================================================

TEST_CASE("edgelist - vertices with no edges", "[edgelist][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {}, // vertex 0 - no edges
        {}, // vertex 1 - no edges
        {}  // vertex 2 - no edges
  };

  SECTION("no value function") {
    auto elist = edgelist(g);

    REQUIRE(elist.begin() == elist.end());
  }

  SECTION("with value function") {
    auto elist = edgelist(g, [](const auto&, auto /*e*/) { return 42; });

    REQUIRE(elist.begin() == elist.end());
  }
}

// =============================================================================
// Test 3: Single Edge
// =============================================================================

TEST_CASE("edgelist - single edge", "[edgelist][single]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1}, // vertex 0 -> edge to 1
        {}   // vertex 1 - no edges
  };

  SECTION("no value function") {
    auto elist = edgelist(g);

    auto it = elist.begin();
    REQUIRE(it != elist.end());

    auto ei = *it;
    REQUIRE(source_id(g, ei.edge) == 0);
    REQUIRE(target_id(g, ei.edge) == 1);

    ++it;
    REQUIRE(it == elist.end());
  }

  SECTION("with value function") {
    auto elist = edgelist(g, [](const auto& g, auto e) {
      return static_cast<int>(source_id(g, e)) * 100 + static_cast<int>(target_id(g, e));
    });

    auto ei = *elist.begin();
    REQUIRE(ei.value == 1); // 0 * 100 + 1
  }

  SECTION("structured binding - no value function") {
    auto elist = edgelist(g);

    std::size_t count = 0;
    for (auto [sid, tid, e] : elist) {
      REQUIRE(sid == 0);
      REQUIRE(tid == 1);
      ++count;
    }
    REQUIRE(count == 1);
  }

  SECTION("structured binding - with value function") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return target_id(g, e) * 10; });

    for (auto [sid, tid, e, val] : elist) {
      REQUIRE(tid == 1);
      REQUIRE(val == 10);
    }
  }
}

// =============================================================================
// Test 4: Multiple Edges from Single Vertex
// =============================================================================

TEST_CASE("edgelist - multiple edges from single vertex", "[edgelist][multiple]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3}, // vertex 0 -> edges to 1, 2, 3
                 {},
                 {},
                 {}};

  SECTION("iteration") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges[1] == std::pair<int, int>{0, 2});
    REQUIRE(edges[2] == std::pair<int, int>{0, 3});
  }

  SECTION("with value function") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return target_id(g, e); });

    std::vector<int> values;
    for (auto [sid, tid, e, val] : elist) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{1, 2, 3});
  }
}

// =============================================================================
// Test 5: Edges from Multiple Vertices (Flattening)
// =============================================================================

TEST_CASE("edgelist - flattening multiple vertex edge lists", "[edgelist][flattening]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // vertex 0 -> edges to 1, 2
        {2, 3}, // vertex 1 -> edges to 2, 3
        {3},    // vertex 2 -> edge to 3
        {}      // vertex 3 - no edges
  };

  SECTION("all edges in order") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    // Edges should come in vertex order, then edge order within vertex
    REQUIRE(edges.size() == 5);
    REQUIRE(edges[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges[1] == std::pair<int, int>{0, 2});
    REQUIRE(edges[2] == std::pair<int, int>{1, 2});
    REQUIRE(edges[3] == std::pair<int, int>{1, 3});
    REQUIRE(edges[4] == std::pair<int, int>{2, 3});
  }

  SECTION("with value function computing edge weight") {
    auto elist = edgelist(g, [](const auto& g, auto e) {
      // Compute edge "weight" as source + target
      return static_cast<int>(source_id(g, e)) + static_cast<int>(target_id(g, e));
    });

    std::vector<int> weights;
    for (auto [sid, tid, e, w] : elist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<int>{1, 2, 3, 4, 5});
  }
}

// =============================================================================
// Test 6: Skipping Empty Vertices
// =============================================================================

TEST_CASE("edgelist - skipping empty vertices", "[edgelist][skip]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {},  // vertex 0 - no edges
        {2}, // vertex 1 -> edge to 2
        {},  // vertex 2 - no edges
        {},  // vertex 3 - no edges
        {5}, // vertex 4 -> edge to 5
        {}   // vertex 5 - no edges
  };

  SECTION("correctly skips empty vertices") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 2);
    REQUIRE(edges[0] == std::pair<int, int>{1, 2});
    REQUIRE(edges[1] == std::pair<int, int>{4, 5});
  }
}

// =============================================================================
// Test 7: Value Function Types
// =============================================================================

TEST_CASE("edgelist - value function types", "[edgelist][evf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {}, {}};

  SECTION("returning string") {
    auto elist = edgelist(g, [](const auto& g, auto e) {
      return std::to_string(source_id(g, e)) + "->" + std::to_string(target_id(g, e));
    });

    std::vector<std::string> labels;
    for (auto [sid, tid, e, label] : elist) {
      labels.push_back(label);
    }

    REQUIRE(labels == std::vector<std::string>{"0->1", "0->2"});
  }

  SECTION("returning double") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return static_cast<double>(target_id(g, e)) * 1.5; });

    std::vector<double> values;
    for (auto [sid, tid, e, val] : elist) {
      values.push_back(val);
    }

    REQUIRE(values[0] == 1.5);
    REQUIRE(values[1] == 3.0);
  }

  SECTION("capturing lambda") {
    int  multiplier = 100;
    auto elist      = edgelist(g, [multiplier](const auto& g, auto e) { return target_id(g, e) * multiplier; });

    std::vector<int> values;
    for (auto [sid, tid, e, val] : elist) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{100, 200});
  }
}

// =============================================================================
// Test 8: Range Algorithms
// =============================================================================

TEST_CASE("edgelist - range algorithms", "[edgelist][algorithm]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3}, {2}, {3}, {}};

  SECTION("std::ranges::distance") {
    auto elist = edgelist(g);
    auto count = std::ranges::distance(elist);
    REQUIRE(count == 5);
  }

  SECTION("std::ranges::count_if") {
    auto elist = edgelist(g);
    auto count = std::ranges::count_if(elist, [&g](auto ei) { return target_id(g, ei.edge) == 3; });
    REQUIRE(count == 2); // 0->3 and 2->3
  }

  SECTION("std::ranges::for_each") {
    auto elist = edgelist(g);
    int  sum   = 0;
    std::ranges::for_each(elist, [&g, &sum](auto ei) { sum += target_id(g, ei.edge); });
    REQUIRE(sum == 11); // 1+2+3+2+3
  }

  SECTION("std::ranges::find_if") {
    auto elist = edgelist(g);
    auto it    = std::ranges::find_if(elist,
                                      [&g](auto ei) { return source_id(g, ei.edge) == 1 && target_id(g, ei.edge) == 2; });
    REQUIRE(it != elist.end());
    REQUIRE(source_id(g, (*it).edge) == 1);
    REQUIRE(target_id(g, (*it).edge) == 2);
  }
}

// =============================================================================
// Test 9: Vector of Deques
// =============================================================================

TEST_CASE("edgelist - vector of deques", "[edgelist][container]") {
  using Graph = std::vector<std::deque<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  SECTION("iteration") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges[1] == std::pair<int, int>{0, 2});
    REQUIRE(edges[2] == std::pair<int, int>{1, 2});
  }

  SECTION("with value function") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return target_id(g, e) * 10; });

    std::vector<int> values;
    for (auto [sid, tid, e, val] : elist) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{10, 20, 20});
  }
}

// =============================================================================
// Test 10: Deque of Vectors
// =============================================================================

TEST_CASE("edgelist - deque of vectors", "[edgelist][container]") {
  using Graph = std::deque<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  SECTION("iteration") {
    auto elist = edgelist(g);

    std::vector<std::pair<std::size_t, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
  }
}

// =============================================================================
// Test 11: Iterator Operations
// =============================================================================

TEST_CASE("edgelist - iterator operations", "[edgelist][iterator]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  SECTION("post-increment") {
    auto elist = edgelist(g);
    auto it    = elist.begin();

    auto old_it = it++;
    REQUIRE(target_id(g, (*old_it).edge) == 1);
    REQUIRE(target_id(g, (*it).edge) == 2);
  }

  SECTION("equality comparison") {
    auto elist = edgelist(g);
    auto it1   = elist.begin();
    auto it2   = elist.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
  }

  SECTION("end iterator comparison") {
    auto elist = edgelist(g);
    auto it    = elist.begin();

    // 3 edges total
    ++it;
    ++it;
    ++it;
    REQUIRE(it == elist.end());
  }
}

// =============================================================================
// Test 12: Range Concepts
// =============================================================================

TEST_CASE("edgelist - satisfies range concepts", "[edgelist][concepts]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}};

  SECTION("view without value function") {
    auto elist = edgelist(g);

    STATIC_REQUIRE(std::ranges::range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::view<decltype(elist)>);
  }

  SECTION("view with value function") {
    auto elist = edgelist(g, [](const auto&, auto /*e*/) { return 42; });

    STATIC_REQUIRE(std::ranges::range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::view<decltype(elist)>);
  }
}

// =============================================================================
// Test 13: Map-Based Vertex Container
// =============================================================================

TEST_CASE("edgelist - map-based vertex container", "[edgelist][map]") {
  // Map vertices - non-contiguous vertex IDs
  using Graph = std::map<int, std::vector<int>>;
  Graph g     = {
        {100, {200, 300}}, // vertex 100 -> edges to 200, 300
        {200, {300}},      // vertex 200 -> edge to 300
        {300, {}}          // vertex 300 - no edges
  };

  SECTION("iteration over all edges") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{100, 200});
    REQUIRE(edges[1] == std::pair<int, int>{100, 300});
    REQUIRE(edges[2] == std::pair<int, int>{200, 300});
  }

  SECTION("with value function") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return target_id(g, e) - source_id(g, e); });

    std::vector<int> diffs;
    for (auto [sid, tid, e, diff] : elist) {
      diffs.push_back(diff);
    }

    REQUIRE(diffs == std::vector<int>{100, 200, 100});
  }

  SECTION("empty edge list") {
    Graph empty_g = {{10, {}}, {20, {}}, {30, {}}};

    auto elist = edgelist(empty_g);
    REQUIRE(elist.begin() == elist.end());
  }
}

// =============================================================================
// Test 14: Map-Based Edge Container (Sorted Edges)
// =============================================================================

TEST_CASE("edgelist - vector vertices map edges", "[edgelist][edge_map]") {
  // Edges stored in map (sorted by target, with edge values)
  using Graph = std::vector<std::map<int, double>>;
  Graph g     = {
        {{1, 1.5}, {2, 2.5}}, // vertex 0 -> (1, 1.5), (2, 2.5)
        {{2, 3.5}},           // vertex 1 -> (2, 3.5)
        {}                    // vertex 2 -> no edges
  };

  SECTION("iteration") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges[1] == std::pair<int, int>{0, 2});
    REQUIRE(edges[2] == std::pair<int, int>{1, 2});
  }

  SECTION("accessing edge weights via edge_value") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<double> weights;
    for (auto [sid, tid, e, w] : elist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<double>{1.5, 2.5, 3.5});
  }
}

// =============================================================================
// Test 15: Map Vertices + Map Edges (Fully Sparse Graph)
// =============================================================================

TEST_CASE("edgelist - map vertices map edges", "[edgelist][map][edge_map]") {
  // Both vertices and edges in maps - fully sparse graph
  using Graph = std::map<int, std::map<int, double>>;
  Graph g     = {
        {10, {{20, 1.0}, {30, 2.0}}}, // vertex 10 -> (20, 1.0), (30, 2.0)
        {20, {{30, 3.0}}},            // vertex 20 -> (30, 3.0)
        {30, {}}                      // vertex 30 -> no edges
  };

  SECTION("iteration over all edges") {
    auto elist = edgelist(g);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{10, 20});
    REQUIRE(edges[1] == std::pair<int, int>{10, 30});
    REQUIRE(edges[2] == std::pair<int, int>{20, 30});
  }

  SECTION("with edge value function") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<double> weights;
    for (auto [sid, tid, e, w] : elist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<double>{1.0, 2.0, 3.0});
  }

  SECTION("combined source, target, weight extraction") {
    auto elist = edgelist(g, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<std::tuple<int, int, double>> all_edges;
    for (auto [sid, tid, e, w] : elist) {
      all_edges.emplace_back(sid, tid, w);
    }

    REQUIRE(all_edges.size() == 3);
    REQUIRE(std::get<0>(all_edges[0]) == 10);
    REQUIRE(std::get<1>(all_edges[0]) == 20);
    REQUIRE(std::get<2>(all_edges[0]) == 1.0);

    REQUIRE(std::get<0>(all_edges[2]) == 20);
    REQUIRE(std::get<1>(all_edges[2]) == 30);
    REQUIRE(std::get<2>(all_edges[2]) == 3.0);
  }
}

// =============================================================================
// =============================================================================
// EDGE_LIST TESTS (Step 2.4.1)
// =============================================================================
// =============================================================================

// =============================================================================
// Test 16: edge_list with pairs
// =============================================================================

TEST_CASE("edgelist - edge_list with pairs", "[edgelist][edge_list]") {
  using EdgeList = std::vector<std::pair<int, int>>;
  EdgeList el    = {{1, 2}, {2, 3}, {3, 4}, {4, 5}};

  SECTION("no value function") {
    auto elist = edgelist(el);

    REQUIRE(elist.size() == 4);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 4);
    REQUIRE(edges[0] == std::pair<int, int>{1, 2});
    REQUIRE(edges[1] == std::pair<int, int>{2, 3});
    REQUIRE(edges[2] == std::pair<int, int>{3, 4});
    REQUIRE(edges[3] == std::pair<int, int>{4, 5});
  }

  SECTION("with value function") {
    auto elist = edgelist(el, [](auto& el, auto e) { return source_id(el, e) + target_id(el, e); });

    std::vector<int> sums;
    for (auto [sid, tid, e, sum] : elist) {
      sums.push_back(sum);
    }

    REQUIRE(sums == std::vector<int>{3, 5, 7, 9});
  }
}

// =============================================================================
// Test 17: edge_list with tuples (2-tuples, no value)
// =============================================================================

TEST_CASE("edgelist - edge_list with 2-tuples", "[edgelist][edge_list]") {
  using EdgeList = std::vector<std::tuple<int, int>>;
  EdgeList el    = {{0, 1}, {1, 2}, {2, 0}};

  SECTION("iteration") {
    auto elist = edgelist(el);

    REQUIRE(elist.size() == 3);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges[1] == std::pair<int, int>{1, 2});
    REQUIRE(edges[2] == std::pair<int, int>{2, 0});
  }
}

// =============================================================================
// Test 18: edge_list with 3-tuples (weighted edges)
// =============================================================================

TEST_CASE("edgelist - edge_list with 3-tuples (weighted)", "[edgelist][edge_list]") {
  using EdgeList = std::vector<std::tuple<int, int, double>>;
  EdgeList el    = {{0, 1, 1.5}, {1, 2, 2.5}, {2, 3, 3.5}};

  SECTION("no value function") {
    auto elist = edgelist(el);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{0, 1});
    REQUIRE(edges[2] == std::pair<int, int>{2, 3});
  }

  SECTION("value function accessing edge_value") {
    auto elist = edgelist(el, [](auto& el, auto e) { return edge_value(el, e); });

    std::vector<double> weights;
    for (auto [sid, tid, e, w] : elist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<double>{1.5, 2.5, 3.5});
  }

  SECTION("value function computing derived value") {
    auto elist = edgelist(el, [](auto& el, auto e) { return edge_value(el, e) * 2.0; });

    std::vector<double> doubled;
    for (auto [sid, tid, e, val] : elist) {
      doubled.push_back(val);
    }

    REQUIRE(doubled == std::vector<double>{3.0, 5.0, 7.0});
  }
}

// =============================================================================
// Test 19: edge_list with edge_data
// =============================================================================

TEST_CASE("edgelist - edge_list with edge_data", "[edgelist][edge_list]") {
  using EdgeType = edge_data<int, true, void, void>;
  using EdgeList = std::vector<EdgeType>;

  EdgeList el = {EdgeType{10, 20}, EdgeType{20, 30}, EdgeType{30, 40}};

  SECTION("no value function") {
    auto elist = edgelist(el);

    REQUIRE(elist.size() == 3);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges[0] == std::pair<int, int>{10, 20});
    REQUIRE(edges[1] == std::pair<int, int>{20, 30});
    REQUIRE(edges[2] == std::pair<int, int>{30, 40});
  }

  SECTION("with value function") {
    auto elist = edgelist(el, [](auto& el, auto e) { return target_id(el, e) - source_id(el, e); });

    std::vector<int> diffs;
    for (auto [sid, tid, e, diff] : elist) {
      diffs.push_back(diff);
    }

    REQUIRE(diffs == std::vector<int>{10, 10, 10});
  }
}

// =============================================================================
// Test 20: edge_list with edge_data (with value)
// =============================================================================

TEST_CASE("edgelist - edge_list with edge_data with value", "[edgelist][edge_list]") {
  using EdgeType = edge_data<int, true, void, double>;
  using EdgeList = std::vector<EdgeType>;

  EdgeList el = {EdgeType{1, 2, 0.5}, EdgeType{2, 3, 1.5}, EdgeType{3, 1, 2.5}};

  SECTION("accessing edge_value") {
    auto elist = edgelist(el, [](auto& el, auto e) { return edge_value(el, e); });

    std::vector<double> weights;
    for (auto [sid, tid, e, w] : elist) {
      weights.push_back(w);
    }

    REQUIRE(weights == std::vector<double>{0.5, 1.5, 2.5});
  }
}

// =============================================================================
// Test 21: Empty edge_list
// =============================================================================

TEST_CASE("edgelist - empty edge_list", "[edgelist][edge_list][empty]") {
  using EdgeList = std::vector<std::pair<int, int>>;
  EdgeList el;

  SECTION("no value function") {
    auto elist = edgelist(el);

    REQUIRE(elist.size() == 0);
    REQUIRE(elist.begin() == elist.end());
  }

  SECTION("with value function") {
    auto elist = edgelist(el, [](auto& /*el*/, auto /*e*/) { return 42; });

    REQUIRE(elist.begin() == elist.end());
  }
}

// =============================================================================
// Test 22: edge_list range concepts
// =============================================================================

TEST_CASE("edgelist - edge_list satisfies range concepts", "[edgelist][edge_list][concepts]") {
  using EdgeList = std::vector<std::pair<int, int>>;
  EdgeList el    = {{1, 2}, {3, 4}};

  SECTION("view without value function") {
    auto elist = edgelist(el);

    STATIC_REQUIRE(std::ranges::range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::view<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::sized_range<decltype(elist)>);
  }

  SECTION("view with value function") {
    auto elist = edgelist(el, [](auto& el, auto e) { return source_id(el, e); });

    STATIC_REQUIRE(std::ranges::range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::forward_range<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::view<decltype(elist)>);
    STATIC_REQUIRE(std::ranges::sized_range<decltype(elist)>);
  }
}

// =============================================================================
// Test 23: edge_list iterator operations
// =============================================================================

TEST_CASE("edgelist - edge_list iterator operations", "[edgelist][edge_list][iterator]") {
  using EdgeList = std::vector<std::pair<int, int>>;
  EdgeList el    = {{1, 2}, {2, 3}, {3, 4}};

  SECTION("post-increment") {
    auto elist = edgelist(el);
    auto it    = elist.begin();

    auto old_it = it++;
    REQUIRE(target_id(el, (*old_it).edge) == 2);
    REQUIRE(target_id(el, (*it).edge) == 3);
  }

  SECTION("equality comparison") {
    auto elist = edgelist(el);
    auto it1   = elist.begin();
    auto it2   = elist.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
  }

  SECTION("end iterator comparison") {
    auto elist = edgelist(el);
    auto it    = elist.begin();

    ++it;
    ++it;
    ++it;
    REQUIRE(it == elist.end());
  }
}

// =============================================================================
// Test 24: edge_list with string vertex IDs
// =============================================================================

TEST_CASE("edgelist - edge_list with string vertex IDs", "[edgelist][edge_list][string]") {
  using EdgeList = std::vector<std::pair<std::string, std::string>>;
  EdgeList el    = {{"A", "B"}, {"B", "C"}, {"C", "A"}};

  SECTION("iteration") {
    auto elist = edgelist(el);

    REQUIRE(elist.size() == 3);

    std::vector<std::pair<std::string, std::string>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges[0] == std::pair<std::string, std::string>{"A", "B"});
    REQUIRE(edges[1] == std::pair<std::string, std::string>{"B", "C"});
    REQUIRE(edges[2] == std::pair<std::string, std::string>{"C", "A"});
  }

  SECTION("with value function creating labels") {
    auto elist = edgelist(el, [](auto& el, auto e) { return source_id(el, e) + "->" + target_id(el, e); });

    std::vector<std::string> labels;
    for (auto [sid, tid, e, label] : elist) {
      labels.push_back(label);
    }

    REQUIRE(labels == std::vector<std::string>{"A->B", "B->C", "C->A"});
  }
}

// =============================================================================
// Test 25: edge_list with range algorithms
// =============================================================================

TEST_CASE("edgelist - edge_list with range algorithms", "[edgelist][edge_list][algorithm]") {
  using EdgeList = std::vector<std::pair<int, int>>;
  EdgeList el    = {{1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}};

  SECTION("std::ranges::distance") {
    auto elist = edgelist(el);
    REQUIRE(std::ranges::distance(elist) == 5);
  }

  SECTION("std::ranges::count_if") {
    auto elist = edgelist(el);
    auto count = std::ranges::count_if(elist, [&el](auto ei) { return target_id(el, ei.edge) > 3; });
    REQUIRE(count == 3); // edges to 4, 5, 6
  }

  SECTION("std::ranges::for_each") {
    auto elist = edgelist(el);
    int  sum   = 0;
    std::ranges::for_each(elist, [&el, &sum](auto ei) { sum += target_id(el, ei.edge); });
    REQUIRE(sum == 20); // 2+3+4+5+6
  }
}

// =============================================================================
// Test 26: Deque-based edge_list
// =============================================================================

TEST_CASE("edgelist - deque-based edge_list", "[edgelist][edge_list][container]") {
  using EdgeList = std::deque<std::pair<int, int>>;
  EdgeList el    = {{1, 2}, {2, 3}, {3, 4}};

  SECTION("iteration") {
    auto elist = edgelist(el);

    std::vector<std::pair<int, int>> edges;
    for (auto [sid, tid, e] : elist) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<int, int>{1, 2});
  }
}

// =============================================================================
// Conditional size() tests for adj-list edgelist_view
// =============================================================================

namespace {
/// Wrapper around vector<vector<int>> that adds a num_edges() member for O(1) edge count.
/// Satisfies adjacency_list concept and exposes O(1) num_edges().
struct counted_graph : std::vector<std::vector<int>> {
  using base = std::vector<std::vector<int>>;
  using base::base;

  // Track total edges
  [[nodiscard]] std::size_t num_edges() const noexcept {
    std::size_t n = 0;
    for (auto& row : *this)
      n += row.size();
    return n;
  }
};
} // anonymous namespace

TEST_CASE("edgelist - adj list view is NOT sized_range for vector<vector<int>>", "[edgelist][size][concepts]") {
  using Graph = std::vector<std::vector<int>>;

  SECTION("void EVF") {
    using View = decltype(edgelist(std::declval<Graph&>()));
    STATIC_REQUIRE_FALSE(std::ranges::sized_range<View>);
  }

  SECTION("with EVF") {
    Graph g     = {{1}, {}};
    auto  elist = edgelist(g, [](const auto&, auto) { return 1; });
    STATIC_REQUIRE_FALSE(std::ranges::sized_range<decltype(elist)>);
  }
}

TEST_CASE("edgelist - adj list view IS sized_range when graph has O(1) num_edges", "[edgelist][size][concepts]") {
  SECTION("void EVF - concept check") {
    using View = decltype(edgelist(std::declval<counted_graph&>()));
    STATIC_REQUIRE(std::ranges::sized_range<View>);
  }

  SECTION("with EVF - concept check") {
    counted_graph g     = {{1, 2}, {3}, {}};
    auto          elist = edgelist(g, [](const auto&, auto) { return 42; });
    STATIC_REQUIRE(std::ranges::sized_range<decltype(elist)>);
  }
}

TEST_CASE("edgelist - size() returns correct count from graph with num_edges()", "[edgelist][size]") {
  SECTION("non-empty graph") {
    counted_graph g     = {{1, 2}, {3}, {}, {0}}; // 4 edges total
    auto          elist = edgelist(g);
    REQUIRE(elist.size() == 4);
  }

  SECTION("empty graph") {
    counted_graph g;
    auto          elist = edgelist(g);
    REQUIRE(elist.size() == 0);
  }

  SECTION("graph with no edges") {
    counted_graph g     = {{}, {}, {}};
    auto          elist = edgelist(g);
    REQUIRE(elist.size() == 0);
  }

  SECTION("with EVF") {
    counted_graph g     = {{1}, {2}, {}}; // 2 edges
    auto          elist = edgelist(g, [](const auto&, auto) { return 99; });
    REQUIRE(elist.size() == 2);
  }
}

TEST_CASE("edgelist - has_const_time_num_edges concept correctness", "[edgelist][size][concepts]") {
  STATIC_REQUIRE_FALSE(edgelist_detail::has_const_time_num_edges<std::vector<std::vector<int>>>);
  STATIC_REQUIRE(edgelist_detail::has_const_time_num_edges<counted_graph>);
}

TEST_CASE("edgelist - edge_list_edgelist_view size() still works (vector)", "[edgelist][edge_list][size]") {
  using EdgeList = std::vector<std::pair<int, int>>;
  EdgeList el    = {{0, 1}, {1, 2}, {2, 3}};
  auto     elist = edgelist(el);
  STATIC_REQUIRE(std::ranges::sized_range<decltype(elist)>);
  REQUIRE(elist.size() == 3);
}
