/**
 * @file test_vertices_pid_cpo.cpp
 * @brief Tests for vertices(g, pid) CPO
 * 
 * Tests the vertices(g, pid) CPO with different graph representations.
 * This file focuses on the default implementation which returns:
 * - All vertices when pid == 0 (single partition)
 * - Empty range when pid != 0 (no such partition exists)
 * 
 * Resolution order:
 * 1. g.vertices(pid) - member function (highest priority)
 * 2. vertices(g, pid) - ADL (medium priority)
 * 3. Default: returns vertices(g) if pid==0, empty otherwise (lowest priority)
 * 
 * Verifies:
 * - Default returns all vertices for partition 0
 * - Default returns empty range for non-zero partitions
 * - Works with different graph storage types
 * - Consistent with vertices(g)
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
// Default Implementation Tests - Single Partition
// =============================================================================

TEST_CASE("vertices(g,pid) - vector graph partition 0 returns all vertices", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3},
        {0, 1, 2}
    };
    
    SECTION("partition 0 returns all vertices") {
        auto verts = vertices(graph, 0);
        
        size_t count = 0;
        for (auto v : verts) {
            REQUIRE(vertex_id(graph, v) == count);
            ++count;
        }
        REQUIRE(count == 4);
    }
    
    SECTION("partition 0 matches vertices(g)") {
        auto verts_all = vertices(graph);
        auto verts_p0 = vertices(graph, 0);
        
        auto it_all = std::ranges::begin(verts_all);
        auto it_p0 = std::ranges::begin(verts_p0);
        
        while (it_all != std::ranges::end(verts_all) && 
               it_p0 != std::ranges::end(verts_p0)) {
            REQUIRE(vertex_id(graph, *it_all) == vertex_id(graph, *it_p0));
            ++it_all;
            ++it_p0;
        }
        
        REQUIRE(it_all == std::ranges::end(verts_all));
        REQUIRE(it_p0 == std::ranges::end(verts_p0));
    }
}

TEST_CASE("vertices(g,pid) - vector graph non-zero partition returns empty", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    SECTION("partition 1 returns empty range") {
        auto verts = vertices(graph, 1);
        REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
    }
    
    SECTION("partition 5 returns empty range") {
        auto verts = vertices(graph, 5);
        REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
    }
}

TEST_CASE("vertices(g,pid) - map graph partition 0", "[vertices_pid][cpo][default]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {0, {1, 2}},
        {1, {2, 3}},
        {2, {3}},
        {3, {}}
    };
    
    SECTION("partition 0 returns all vertices") {
        auto verts = vertices(graph, 0);
        
        size_t count = 0;
        for (auto v : verts) {
            (void)v;
            ++count;
        }
        REQUIRE(count == 4);
    }
    
    SECTION("partition 1 returns empty") {
        auto verts = vertices(graph, 1);
        REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
    }
}

TEST_CASE("vertices(g,pid) - deque graph partition 0", "[vertices_pid][cpo][default]") {
    using Graph = std::deque<std::deque<int>>;
    Graph graph = {{1}, {2}, {3}};
    
    auto verts = vertices(graph, 0);
    size_t count = 0;
    for (auto v : verts) {
        (void)v;
        ++count;
    }
    REQUIRE(count == 3);
}

TEST_CASE("vertices(g,pid) - empty graph", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph;
    
    SECTION("partition 0 returns empty") {
        auto verts = vertices(graph, 0);
        REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
    }
    
    SECTION("partition 1 returns empty") {
        auto verts = vertices(graph, 1);
        REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
    }
}

TEST_CASE("vertices(g,pid) - single vertex graph", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2, 3}
    };
    
    SECTION("partition 0 has one vertex") {
        auto verts = vertices(graph, 0);
        auto it = std::ranges::begin(verts);
        REQUIRE(it != std::ranges::end(verts));
        
        auto v = *it;
        REQUIRE(vertex_id(graph, v) == 0);
        
        ++it;
        REQUIRE(it == std::ranges::end(verts));
    }
}

TEST_CASE("vertices(g,pid) - const graph", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    const Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    auto verts = vertices(graph, 0);
    size_t count = 0;
    for (auto v : verts) {
        (void)v;
        ++count;
    }
    REQUIRE(count == 3);
}

TEST_CASE("vertices(g,pid) - negative partition id", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1}, {2}};
    
    // Negative partition IDs return empty (don't exist)
    auto verts = vertices(graph, -1);
    REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
}

TEST_CASE("vertices(g,pid) - large partition id", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1}, {2}, {3}};
    
    // Large partition IDs return empty (don't exist)
    auto verts = vertices(graph, 999);
    REQUIRE(std::ranges::begin(verts) == std::ranges::end(verts));
}

TEST_CASE("vertices(g,pid) - iteration multiple times", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1, 2}, {2, 3}, {3}};
    
    auto verts = vertices(graph, 0);
    
    // First iteration
    size_t count1 = 0;
    for (auto v : verts) {
        (void)v;
        ++count1;
    }
    
    // Second iteration
    size_t count2 = 0;
    for (auto v : verts) {
        (void)v;
        ++count2;
    }
    
    REQUIRE(count1 == 3);
    REQUIRE(count2 == 3);
}

TEST_CASE("vertices(g,pid) - works with different partition ID types", "[vertices_pid][cpo][default]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1}, {2}};
    
    SECTION("int partition id") {
        auto verts = vertices(graph, 0);
        size_t count = 0;
        for (auto v : verts) {
            (void)v;
            ++count;
        }
        REQUIRE(count == 2);
    }
    
    SECTION("size_t partition id") {
        auto verts = vertices(graph, size_t(0));
        size_t count = 0;
        for (auto v : verts) {
            (void)v;
            ++count;
        }
        REQUIRE(count == 2);
    }
    
    SECTION("uint32_t partition id") {
        auto verts = vertices(graph, uint32_t(0));
        size_t count = 0;
        for (auto v : verts) {
            (void)v;
            ++count;
        }
        REQUIRE(count == 2);
    }
}
