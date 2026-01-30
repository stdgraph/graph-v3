/**
 * @file test_partition_id_cpo.cpp
 * @brief Tests for partition_id(g, u) CPO
 * 
 * Tests the partition_id(g, u) CPO with different graph representations.
 * This file focuses on the default implementation which returns 0 (single partition).
 * 
 * Resolution order:
 * 1. g.partition_id(u) - member function (highest priority)
 * 2. partition_id(g, u) - ADL (medium priority)
 * 3. Default: returns 0 (lowest priority) - single partition assumption
 * 
 * Verifies:
 * - Default returns 0 for all vertices (single partition)
 * - Works with different graph storage types
 * - Consistent across multiple calls
 * - Correct noexcept specification
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <map>
#include <deque>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Default Implementation Tests - Single Partition (returns 0)
// =============================================================================

TEST_CASE("partition_id(g,u) - vector graph returns 0 for all vertices", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3},
        {0, 1, 2}
    };
    
    SECTION("returns 0 for first vertex") {
        auto verts = vertices(graph);
        auto v0 = *std::ranges::begin(verts);
        
        auto pid = partition_id(graph, v0);
        REQUIRE(pid == 0);
    }
    
    SECTION("returns 0 for all vertices") {
        auto verts = vertices(graph);
        
        for (auto v : verts) {
            auto pid = partition_id(graph, v);
            REQUIRE(pid == 0);
        }
    }
    
    SECTION("consistent across multiple calls") {
        auto verts = vertices(graph);
        auto it = std::ranges::begin(verts);
        std::ranges::advance(it, 2);
        auto v2 = *it;
        
        auto pid1 = partition_id(graph, v2);
        auto pid2 = partition_id(graph, v2);
        REQUIRE(pid1 == pid2);
        REQUIRE(pid1 == 0);
    }
}

TEST_CASE("partition_id(g,u) - empty graph", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph;
    
    // Empty graph has no vertices, so we can't test partition_id
    // This test just verifies the graph is empty
    auto verts = vertices(graph);
    REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
}

TEST_CASE("partition_id(g,u) - single vertex graph", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2, 3}
    };
    
    auto verts = vertices(graph);
    auto v0 = *std::ranges::begin(verts);
    
    auto pid = partition_id(graph, v0);
    REQUIRE(pid == 0);
}

TEST_CASE("partition_id(g,u) - map-based graph returns 0", "[partition_id][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {0, {1, 2}},
        {1, {2, 3}},
        {2, {3}},
        {3, {}}
    };
    
    SECTION("returns 0 for all vertices") {
        auto verts = vertices(graph);
        
        for (auto v : verts) {
            auto pid = partition_id(graph, v);
            REQUIRE(pid == 0);
        }
    }
    
    SECTION("returns 0 for vertex with sparse ID") {
        auto verts = vertices(graph);
        auto it = std::ranges::begin(verts);
        std::ranges::advance(it, 2);
        auto v2 = *it;
        
        auto pid = partition_id(graph, v2);
        REQUIRE(pid == 0);
    }
}

TEST_CASE("partition_id(g,u) - map with non-contiguous keys", "[partition_id][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {10, {20, 30}},
        {20, {30}},
        {30, {10}},
        {100, {10, 20}}
    };
    
    auto verts = vertices(graph);
    
    for (auto v : verts) {
        auto pid = partition_id(graph, v);
        REQUIRE(pid == 0);
    }
}

TEST_CASE("partition_id(g,u) - deque-based graph", "[partition_id][cpo][default]") {
    using Graph = std::deque<std::deque<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto verts = vertices(graph);
    
    for (auto v : verts) {
        auto pid = partition_id(graph, v);
        REQUIRE(pid == 0);
    }
}

TEST_CASE("partition_id(g,u) - weighted graph with pairs", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<std::pair<int, double>>>;
    Graph graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 3.5}, {3, 4.5}},
        {{3, 5.5}}
    };
    
    auto verts = vertices(graph);
    
    for (auto v : verts) {
        auto pid = partition_id(graph, v);
        REQUIRE(pid == 0);
    }
}

TEST_CASE("partition_id(g,u) - weighted graph with tuples", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<std::tuple<int, double, std::string>>>;
    Graph graph = {
        {{1, 1.5, "a"}, {2, 2.5, "b"}},
        {{2, 3.5, "c"}},
        {{}}
    };
    
    auto verts = vertices(graph);
    
    for (auto v : verts) {
        auto pid = partition_id(graph, v);
        REQUIRE(pid == 0);
    }
}

TEST_CASE("partition_id(g,u) - const graph", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    const Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto verts = vertices(graph);
    
    for (auto v : verts) {
        auto pid = partition_id(graph, v);
        REQUIRE(pid == 0);
    }
}

TEST_CASE("partition_id(g,u) - large graph", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph(1000);
    
    // Initialize edges
    for (size_t i = 0; i < graph.size(); ++i) {
        if (i + 1 < graph.size()) {
            graph[i].push_back(static_cast<int>(i + 1));
        }
    }
    
    auto verts = vertices(graph);
    
    // Check first, middle, and last vertices
    auto it = std::ranges::begin(verts);
    REQUIRE(partition_id(graph, *it) == 0);
    
    std::ranges::advance(it, 500);
    REQUIRE(partition_id(graph, *it) == 0);
    
    std::ranges::advance(it, 499);
    REQUIRE(partition_id(graph, *it) == 0);
}

TEST_CASE("partition_id(g,u) - return type is integral", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1, 2}};
    
    auto verts = vertices(graph);
    auto v0 = *std::ranges::begin(verts);
    
    auto pid = partition_id(graph, v0);
    STATIC_REQUIRE(std::integral<decltype(pid)>);
}

TEST_CASE("partition_id(g,u) - noexcept for default implementation", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1, 2}};
    
    auto verts = vertices(graph);
    auto v0 = *std::ranges::begin(verts);
    
    STATIC_REQUIRE(noexcept(partition_id(graph, v0)));
}

TEST_CASE("partition_id(g,u) - works with vertex descriptors from different storage", "[partition_id][cpo][default]") {
    SECTION("vector storage") {
        using Graph = std::vector<std::vector<int>>;
        Graph graph = {{1}, {2}, {3}};
        
        auto verts = vertices(graph);
        for (auto v : verts) {
            REQUIRE(partition_id(graph, v) == 0);
        }
    }
    
    SECTION("map storage") {
        using Graph = std::map<int, std::vector<int>>;
        Graph graph = {{0, {1}}, {1, {2}}, {2, {3}}};
        
        auto verts = vertices(graph);
        for (auto v : verts) {
            REQUIRE(partition_id(graph, v) == 0);
        }
    }
    
    SECTION("deque storage") {
        using Graph = std::deque<std::deque<int>>;
        Graph graph = {{1}, {2}, {3}};
        
        auto verts = vertices(graph);
        for (auto v : verts) {
            REQUIRE(partition_id(graph, v) == 0);
        }
    }
}

TEST_CASE("partition_id(g,u) - integration with vertex_id", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto verts = vertices(graph);
    
    // All vertices have different IDs but same partition
    std::vector<std::pair<size_t, int>> id_partition_pairs;
    for (auto v : verts) {
        auto vid = vertex_id(graph, v);
        auto pid = partition_id(graph, v);
        id_partition_pairs.emplace_back(vid, pid);
    }
    
    REQUIRE(id_partition_pairs.size() == 3);
    
    // Different vertex IDs
    REQUIRE(id_partition_pairs[0].first == size_t(0));
    REQUIRE(id_partition_pairs[1].first == size_t(1));
    REQUIRE(id_partition_pairs[2].first == size_t(2));
    
    // Same partition for all
    REQUIRE(id_partition_pairs[0].second == 0);
    REQUIRE(id_partition_pairs[1].second == 0);
    REQUIRE(id_partition_pairs[2].second == 0);
}

TEST_CASE("partition_id(g,u) - complete graph K4", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2, 3},  // 0 -> 1, 2, 3
        {0, 2, 3},  // 1 -> 0, 2, 3
        {0, 1, 3},  // 2 -> 0, 1, 3
        {0, 1, 2}   // 3 -> 0, 1, 2
    };
    
    auto verts = vertices(graph);
    
    // All vertices in same partition
    for (auto v : verts) {
        REQUIRE(partition_id(graph, v) == 0);
    }
}

TEST_CASE("partition_id(g,u) - disconnected graph", "[partition_id][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1},        // Component 1: 0 -> 1
        {0},        // Component 1: 1 -> 0
        {3},        // Component 2: 2 -> 3
        {2},        // Component 2: 3 -> 2
        {}          // Component 3: isolated vertex
    };
    
    auto verts = vertices(graph);
    
    // Even though graph has disconnected components,
    // default partition_id returns 0 for all
    for (auto v : verts) {
        REQUIRE(partition_id(graph, v) == 0);
    }
}
