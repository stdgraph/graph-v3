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
        auto elist = edgelist(g, [](auto /*e*/) { return 42; });
        
        REQUIRE(elist.begin() == elist.end());
    }
}

// =============================================================================
// Test 2: Graph with Vertices but No Edges
// =============================================================================

TEST_CASE("edgelist - vertices with no edges", "[edgelist][empty]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {},  // vertex 0 - no edges
        {},  // vertex 1 - no edges
        {}   // vertex 2 - no edges
    };

    SECTION("no value function") {
        auto elist = edgelist(g);
        
        REQUIRE(elist.begin() == elist.end());
    }

    SECTION("with value function") {
        auto elist = edgelist(g, [](auto /*e*/) { return 42; });
        
        REQUIRE(elist.begin() == elist.end());
    }
}

// =============================================================================
// Test 3: Single Edge
// =============================================================================

TEST_CASE("edgelist - single edge", "[edgelist][single]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1},  // vertex 0 -> edge to 1
        {}    // vertex 1 - no edges
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
        auto elist = edgelist(g, [&g](auto e) { 
            return static_cast<int>(source_id(g, e)) * 100 + static_cast<int>(target_id(g, e)); 
        });
        
        auto ei = *elist.begin();
        REQUIRE(ei.value == 1);  // 0 * 100 + 1
    }

    SECTION("structured binding - no value function") {
        auto elist = edgelist(g);
        
        std::size_t count = 0;
        for (auto [e] : elist) {
            REQUIRE(source_id(g, e) == 0);
            REQUIRE(target_id(g, e) == 1);
            ++count;
        }
        REQUIRE(count == 1);
    }

    SECTION("structured binding - with value function") {
        auto elist = edgelist(g, [&g](auto e) {
            return target_id(g, e) * 10;
        });
        
        for (auto [e, val] : elist) {
            REQUIRE(target_id(g, e) == 1);
            REQUIRE(val == 10);
        }
    }
}

// =============================================================================
// Test 4: Multiple Edges from Single Vertex
// =============================================================================

TEST_CASE("edgelist - multiple edges from single vertex", "[edgelist][multiple]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2, 3},  // vertex 0 -> edges to 1, 2, 3
        {},
        {},
        {}
    };

    SECTION("iteration") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
        }
        
        REQUIRE(edges.size() == 3);
        REQUIRE(edges[0] == std::pair<int, int>{0, 1});
        REQUIRE(edges[1] == std::pair<int, int>{0, 2});
        REQUIRE(edges[2] == std::pair<int, int>{0, 3});
    }

    SECTION("with value function") {
        auto elist = edgelist(g, [&g](auto e) {
            return target_id(g, e);
        });
        
        std::vector<int> values;
        for (auto [e, val] : elist) {
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
    Graph g = {
        {1, 2},     // vertex 0 -> edges to 1, 2
        {2, 3},     // vertex 1 -> edges to 2, 3
        {3},        // vertex 2 -> edge to 3
        {}          // vertex 3 - no edges
    };

    SECTION("all edges in order") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
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
        auto elist = edgelist(g, [&g](auto e) {
            // Compute edge "weight" as source + target
            return static_cast<int>(source_id(g, e)) + static_cast<int>(target_id(g, e));
        });
        
        std::vector<int> weights;
        for (auto [e, w] : elist) {
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
    Graph g = {
        {},         // vertex 0 - no edges
        {2},        // vertex 1 -> edge to 2
        {},         // vertex 2 - no edges
        {},         // vertex 3 - no edges  
        {5},        // vertex 4 -> edge to 5
        {}          // vertex 5 - no edges
    };

    SECTION("correctly skips empty vertices") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
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
    Graph g = {
        {1, 2},
        {},
        {}
    };

    SECTION("returning string") {
        auto elist = edgelist(g, [&g](auto e) { 
            return std::to_string(source_id(g, e)) + "->" + 
                   std::to_string(target_id(g, e)); 
        });
        
        std::vector<std::string> labels;
        for (auto [e, label] : elist) {
            labels.push_back(label);
        }
        
        REQUIRE(labels == std::vector<std::string>{"0->1", "0->2"});
    }

    SECTION("returning double") {
        auto elist = edgelist(g, [&g](auto e) { 
            return static_cast<double>(target_id(g, e)) * 1.5; 
        });
        
        std::vector<double> values;
        for (auto [e, val] : elist) {
            values.push_back(val);
        }
        
        REQUIRE(values[0] == 1.5);
        REQUIRE(values[1] == 3.0);
    }

    SECTION("capturing lambda") {
        int multiplier = 100;
        auto elist = edgelist(g, [&g, multiplier](auto e) { 
            return target_id(g, e) * multiplier; 
        });
        
        std::vector<int> values;
        for (auto [e, val] : elist) {
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
    Graph g = {
        {1, 2, 3},
        {2},
        {3},
        {}
    };

    SECTION("std::ranges::distance") {
        auto elist = edgelist(g);
        auto count = std::ranges::distance(elist);
        REQUIRE(count == 5);
    }

    SECTION("std::ranges::count_if") {
        auto elist = edgelist(g);
        auto count = std::ranges::count_if(elist, [&g](auto ei) { 
            return target_id(g, ei.edge) == 3; 
        });
        REQUIRE(count == 2);  // 0->3 and 2->3
    }

    SECTION("std::ranges::for_each") {
        auto elist = edgelist(g);
        int sum = 0;
        std::ranges::for_each(elist, [&g, &sum](auto ei) { 
            sum += target_id(g, ei.edge); 
        });
        REQUIRE(sum == 11);  // 1+2+3+2+3
    }

    SECTION("std::ranges::find_if") {
        auto elist = edgelist(g);
        auto it = std::ranges::find_if(elist, [&g](auto ei) { 
            return source_id(g, ei.edge) == 1 && target_id(g, ei.edge) == 2; 
        });
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
    Graph g = {
        {1, 2},
        {2},
        {}
    };

    SECTION("iteration") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
        }
        
        REQUIRE(edges.size() == 3);
        REQUIRE(edges[0] == std::pair<int, int>{0, 1});
        REQUIRE(edges[1] == std::pair<int, int>{0, 2});
        REQUIRE(edges[2] == std::pair<int, int>{1, 2});
    }

    SECTION("with value function") {
        auto elist = edgelist(g, [&g](auto e) {
            return target_id(g, e) * 10;
        });
        
        std::vector<int> values;
        for (auto [e, val] : elist) {
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
    Graph g = {
        {1, 2},
        {2},
        {}
    };

    SECTION("iteration") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<std::size_t, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
        }
        
        REQUIRE(edges.size() == 3);
    }
}

// =============================================================================
// Test 11: Iterator Operations
// =============================================================================

TEST_CASE("edgelist - iterator operations", "[edgelist][iterator]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},
        {2},
        {}
    };

    SECTION("post-increment") {
        auto elist = edgelist(g);
        auto it = elist.begin();
        
        auto old_it = it++;
        REQUIRE(target_id(g, (*old_it).edge) == 1);
        REQUIRE(target_id(g, (*it).edge) == 2);
    }

    SECTION("equality comparison") {
        auto elist = edgelist(g);
        auto it1 = elist.begin();
        auto it2 = elist.begin();
        
        REQUIRE(it1 == it2);
        ++it1;
        REQUIRE(it1 != it2);
    }

    SECTION("end iterator comparison") {
        auto elist = edgelist(g);
        auto it = elist.begin();
        
        // 3 edges total
        ++it; ++it; ++it;
        REQUIRE(it == elist.end());
    }
}

// =============================================================================
// Test 12: Range Concepts
// =============================================================================

TEST_CASE("edgelist - satisfies range concepts", "[edgelist][concepts]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1}, {2}, {}};

    SECTION("view without value function") {
        auto elist = edgelist(g);
        
        STATIC_REQUIRE(std::ranges::range<decltype(elist)>);
        STATIC_REQUIRE(std::ranges::forward_range<decltype(elist)>);
        STATIC_REQUIRE(std::ranges::view<decltype(elist)>);
    }

    SECTION("view with value function") {
        auto elist = edgelist(g, [](auto /*e*/) { return 42; });
        
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
    Graph g = {
        {100, {200, 300}},  // vertex 100 -> edges to 200, 300
        {200, {300}},       // vertex 200 -> edge to 300
        {300, {}}           // vertex 300 - no edges
    };

    SECTION("iteration over all edges") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
        }
        
        REQUIRE(edges.size() == 3);
        REQUIRE(edges[0] == std::pair<int, int>{100, 200});
        REQUIRE(edges[1] == std::pair<int, int>{100, 300});
        REQUIRE(edges[2] == std::pair<int, int>{200, 300});
    }

    SECTION("with value function") {
        auto elist = edgelist(g, [&g](auto e) {
            return target_id(g, e) - source_id(g, e);
        });
        
        std::vector<int> diffs;
        for (auto [e, diff] : elist) {
            diffs.push_back(diff);
        }
        
        REQUIRE(diffs == std::vector<int>{100, 200, 100});
    }

    SECTION("empty edge list") {
        Graph empty_g = {
            {10, {}},
            {20, {}},
            {30, {}}
        };
        
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
    Graph g = {
        {{1, 1.5}, {2, 2.5}},   // vertex 0 -> (1, 1.5), (2, 2.5)
        {{2, 3.5}},              // vertex 1 -> (2, 3.5)
        {}                       // vertex 2 -> no edges
    };

    SECTION("iteration") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
        }
        
        REQUIRE(edges.size() == 3);
        REQUIRE(edges[0] == std::pair<int, int>{0, 1});
        REQUIRE(edges[1] == std::pair<int, int>{0, 2});
        REQUIRE(edges[2] == std::pair<int, int>{1, 2});
    }

    SECTION("accessing edge weights via edge_value") {
        auto elist = edgelist(g, [&g](auto e) {
            return edge_value(g, e);
        });
        
        std::vector<double> weights;
        for (auto [e, w] : elist) {
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
    Graph g = {
        {10, {{20, 1.0}, {30, 2.0}}},   // vertex 10 -> (20, 1.0), (30, 2.0)
        {20, {{30, 3.0}}},               // vertex 20 -> (30, 3.0)
        {30, {}}                         // vertex 30 -> no edges
    };

    SECTION("iteration over all edges") {
        auto elist = edgelist(g);
        
        std::vector<std::pair<int, int>> edges;
        for (auto [e] : elist) {
            edges.emplace_back(source_id(g, e), target_id(g, e));
        }
        
        REQUIRE(edges.size() == 3);
        REQUIRE(edges[0] == std::pair<int, int>{10, 20});
        REQUIRE(edges[1] == std::pair<int, int>{10, 30});
        REQUIRE(edges[2] == std::pair<int, int>{20, 30});
    }

    SECTION("with edge value function") {
        auto elist = edgelist(g, [&g](auto e) {
            return edge_value(g, e);
        });
        
        std::vector<double> weights;
        for (auto [e, w] : elist) {
            weights.push_back(w);
        }
        
        REQUIRE(weights == std::vector<double>{1.0, 2.0, 3.0});
    }

    SECTION("combined source, target, weight extraction") {
        auto elist = edgelist(g, [&g](auto e) {
            return edge_value(g, e);
        });
        
        std::vector<std::tuple<int, int, double>> all_edges;
        for (auto [e, w] : elist) {
            all_edges.emplace_back(source_id(g, e), target_id(g, e), w);
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
