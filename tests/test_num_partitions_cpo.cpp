/**
 * @file test_num_partitions_cpo.cpp
 * @brief Tests for num_partitions(g) CPO
 * 
 * Tests the num_partitions(g) CPO with different graph representations.
 * This file focuses on the default implementation which returns 1 (single partition).
 * 
 * Resolution order:
 * 1. g.num_partitions() - member function (highest priority)
 * 2. num_partitions(g) - ADL (medium priority)
 * 3. Default: returns 1 (lowest priority) - single partition assumption
 * 
 * Verifies:
 * - Default returns 1 for all graph types (single partition)
 * - Works with different graph storage types
 * - Consistent across multiple calls
 * - Correct noexcept specification
 * - Integration with partition_id(g, u)
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/detail/graph_cpo.hpp>
#include <vector>
#include <map>
#include <deque>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Default Implementation Tests - Single Partition (returns 1)
// =============================================================================

TEST_CASE("num_partitions(g) - vector graph returns 1", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3},
        {0, 1, 2}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - empty graph returns 1", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph;
    
    // Even empty graphs have 1 partition (the empty partition)
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - single vertex graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2, 3}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - map-based graph returns 1", "[num_partitions][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {0, {1, 2}},
        {1, {2, 3}},
        {2, {3}},
        {3, {}}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - map with non-contiguous keys", "[num_partitions][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {10, {20, 30}},
        {20, {30}},
        {30, {10}},
        {100, {10, 20}}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - deque-based graph", "[num_partitions][cpo][default]") {
    using Graph = std::deque<std::deque<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - weighted graph with pairs", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<std::pair<int, double>>>;
    Graph graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 3.5}, {3, 4.5}},
        {{3, 5.5}}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - weighted graph with tuples", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<std::tuple<int, double, std::string>>>;
    Graph graph = {
        {{1, 1.5, "a"}, {2, 2.5, "b"}},
        {{2, 3.5, "c"}},
        {{}}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - const graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    const Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - large graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph(1000);
    
    // Initialize edges
    for (size_t i = 0; i < graph.size(); ++i) {
        if (i + 1 < graph.size()) {
            graph[i].push_back(static_cast<int>(i + 1));
        }
    }
    
    // Even with 1000 vertices, still 1 partition by default
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - return type is integral", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1, 2}};
    
    auto count = num_partitions(graph);
    STATIC_REQUIRE(std::integral<decltype(count)>);
}

TEST_CASE("num_partitions(g) - noexcept for default implementation", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1, 2}};
    
    STATIC_REQUIRE(noexcept(num_partitions(graph)));
}

TEST_CASE("num_partitions(g) - consistent across multiple calls", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto count1 = num_partitions(graph);
    auto count2 = num_partitions(graph);
    auto count3 = num_partitions(graph);
    
    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);
    REQUIRE(count3 == 1);
    REQUIRE(count1 == count2);
    REQUIRE(count2 == count3);
}

TEST_CASE("num_partitions(g) - works with different storage types", "[num_partitions][cpo][default]") {
    SECTION("vector storage") {
        using Graph = std::vector<std::vector<int>>;
        Graph graph = {{1}, {2}, {3}};
        REQUIRE(num_partitions(graph) == 1);
    }
    
    SECTION("map storage") {
        using Graph = std::map<int, std::vector<int>>;
        Graph graph = {{0, {1}}, {1, {2}}, {2, {3}}};
        REQUIRE(num_partitions(graph) == 1);
    }
    
    SECTION("deque storage") {
        using Graph = std::deque<std::deque<int>>;
        Graph graph = {{1}, {2}, {3}};
        REQUIRE(num_partitions(graph) == 1);
    }
}

TEST_CASE("num_partitions(g) - integration with partition_id", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto num_parts = num_partitions(graph);
    REQUIRE(num_parts == 1);
    
    // All vertices should have partition_id in range [0, num_partitions)
    auto verts = vertices(graph);
    for (auto v : verts) {
        auto pid = partition_id(graph, v);
        REQUIRE(pid >= 0);
        REQUIRE(pid < num_parts);
    }
}

TEST_CASE("num_partitions(g) - complete graph K4", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2, 3},  // 0 -> 1, 2, 3
        {0, 2, 3},  // 1 -> 0, 2, 3
        {0, 1, 3},  // 2 -> 0, 1, 3
        {0, 1, 2}   // 3 -> 0, 1, 2
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - disconnected graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1},        // Component 1: 0 -> 1
        {0},        // Component 1: 1 -> 0
        {3},        // Component 2: 2 -> 3
        {2},        // Component 2: 3 -> 2
        {}          // Component 3: isolated vertex
    };
    
    // Even though graph has disconnected components,
    // default num_partitions returns 1 (not counting connected components)
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - linear chain graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1},
        {2},
        {3},
        {4},
        {}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - star graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2, 3, 4},  // Center connected to all
        {},             // Leaf
        {},             // Leaf
        {},             // Leaf
        {}              // Leaf
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - bidirectional edges", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {0, 2},
        {0, 1}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - self-loops", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {0, 1},     // Self-loop on 0
        {1, 2},     // Self-loop on 1
        {2}         // Self-loop on 2
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - map with empty edge lists", "[num_partitions][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {0, {}},
        {1, {}},
        {2, {}},
        {3, {}}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - relationship between num_vertices and num_partitions", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3},
        {0}
    };
    
    auto num_verts = num_vertices(graph);
    auto num_parts = num_partitions(graph);
    
    REQUIRE(num_verts == 4);
    REQUIRE(num_parts == 1);
    
    // For default single partition: num_vertices >= num_partitions
    REQUIRE(num_verts >= num_parts);
}

TEST_CASE("num_partitions(g) - empty map graph", "[num_partitions][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph;
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - graph with only edges (complete)", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph(5);
    
    // Complete graph: every vertex connects to every other vertex
    for (size_t i = 0; i < graph.size(); ++i) {
        for (size_t j = 0; j < graph.size(); ++j) {
            if (i != j) {
                graph[i].push_back(static_cast<int>(j));
            }
        }
    }
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - cyclic graph", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1},
        {2},
        {3},
        {4},
        {0}  // Back to start
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}

TEST_CASE("num_partitions(g) - directed acyclic graph (DAG)", "[num_partitions][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {3},
        {3},
        {}
    };
    
    auto count = num_partitions(graph);
    REQUIRE(count == 1);
}
