/**
 * @file test_adjacency_list_vertex_concepts.cpp
 * @brief Unit tests for adjacency list vertex and graph concepts
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <deque>
#include <map>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// vertex_range Concept Tests
// =============================================================================

TEST_CASE("vertex_range concept - vector<vector<int>>", "[adjacency_list][concepts][vertex_range]") {
    using Graph = std::vector<std::vector<int>>;
    
    STATIC_REQUIRE(vertex_range<Graph>);
    
    Graph g = {{1, 2}, {2, 3}, {0}};
    auto verts = vertices(g);
    
    // Should be sized
    REQUIRE(std::ranges::size(verts) == 3);
    
    // Should be forward range (can iterate multiple times)
    int count1 = 0;
    for ([[maybe_unused]] auto v : verts) {
        count1++;
    }
    
    int count2 = 0;
    for ([[maybe_unused]] auto v : verts) {
        count2++;
    }
    
    REQUIRE(count1 == 3);
    REQUIRE(count2 == 3);
}

TEST_CASE("vertex_range concept - map<int, vector<int>>", "[adjacency_list][concepts][vertex_range]") {
    using Graph = std::map<int, std::vector<int>>;
    
    STATIC_REQUIRE(vertex_range<Graph>);
    
    Graph g = {{0, {1, 2}}, {1, {2}}, {2, {}}};
    auto verts = vertices(g);
    
    // Map-based vertex_range doesn't provide size() (only forward iteration)
    // REQUIRE(std::ranges::size(verts) == 3);
    
    // Check we can get vertex IDs
    for (auto v : verts) {
        auto vid = vertex_id(g, v);
        REQUIRE(vid >= 0);
        REQUIRE(vid <= 2);
    }
}

TEST_CASE("vertex_range concept - deque<deque<int>>", "[adjacency_list][concepts][vertex_range]") {
    using Graph = std::deque<std::deque<int>>;
    
    STATIC_REQUIRE(vertex_range<Graph>);
    
    Graph g = {{1, 2}, {0, 2}, {0, 1}};
    auto verts = vertices(g);
    
    REQUIRE(std::ranges::size(verts) == 3);
}

TEST_CASE("vertex_range concept - empty graph", "[adjacency_list][concepts][vertex_range]") {
    using Graph = std::vector<std::vector<int>>;
    
    Graph g;
    auto verts = vertices(g);
    
    REQUIRE(std::ranges::size(verts) == 0);
}

// =============================================================================
// index_vertex_range Concept Tests
// =============================================================================

TEST_CASE("index_vertex_range concept - vector<vector<int>>", "[adjacency_list][concepts][index_vertex_range]") {
    using Graph = std::vector<std::vector<int>>;
    
    // Vector's underlying iterator is random access, so index_vertex_range is satisfied
    // Note: vertex_descriptor_view itself is forward-only, but we check the underlying iterator
    STATIC_REQUIRE(index_vertex_range<Graph>);
    
    // The view is still forward-only (descriptors synthesized on-the-fly)
    STATIC_REQUIRE_FALSE(std::ranges::random_access_range<vertex_range_t<Graph>>);
    
    // And it satisfies vertex_range
    STATIC_REQUIRE(vertex_range<Graph>);
    
    Graph g = {{1, 2}, {2, 3}, {0}};
    auto verts = vertices(g);
    
    // Can iterate vertices
    int count = 0;
    for (auto v : verts) {
        REQUIRE(static_cast<int>(vertex_id(g, v)) == count);
        count++;
    }
    REQUIRE(count == 3);
}

TEST_CASE("index_vertex_range concept - deque<deque<int>>", "[adjacency_list][concepts][index_vertex_range]") {
    using Graph = std::deque<std::deque<int>>;
    
    // Deque's underlying iterator is random access, so index_vertex_range is satisfied
    STATIC_REQUIRE(index_vertex_range<Graph>);
    
    // The view is still forward-only
    STATIC_REQUIRE_FALSE(std::ranges::random_access_range<vertex_range_t<Graph>>);
    
    // And it satisfies the basic vertex_range
    STATIC_REQUIRE(vertex_range<Graph>);
}

TEST_CASE("index_vertex_range concept - map does NOT satisfy", "[adjacency_list][concepts][index_vertex_range]") {
    using Graph = std::map<int, std::vector<int>>;
    
    // Map only supports bidirectional, not random access
    STATIC_REQUIRE_FALSE(index_vertex_range<Graph>);
    STATIC_REQUIRE_FALSE(std::ranges::random_access_range<vertex_range_t<Graph>>);
    
    // But it does satisfy vertex_range
    STATIC_REQUIRE(vertex_range<Graph>);
}

// =============================================================================
// adjacency_list Concept Tests
// =============================================================================

TEST_CASE("adjacency_list concept - vector<vector<int>>", "[adjacency_list][concepts][graph]") {
    using Graph = std::vector<std::vector<int>>;
    
    STATIC_REQUIRE(adjacency_list<Graph>);
    
    Graph g = {{1, 2}, {2, 3}, {0, 1}};
    
    // Should have vertices
    auto verts = vertices(g);
    REQUIRE(std::ranges::size(verts) == 3);
    
    // Each vertex should have edges (or be isolated)
    for (auto v : verts) {
        [[maybe_unused]] auto edge_range = edges(g, v);
        // edge_range size is always valid (>= 0 is implicit for size_t)
    }
}

TEST_CASE("adjacency_list concept - map<int, vector<int>>", "[adjacency_list][concepts][graph]") {
    using Graph = std::map<int, std::vector<int>>;
    
    STATIC_REQUIRE(adjacency_list<Graph>);
    
    Graph g = {{0, {1, 2}}, {1, {2}}, {2, {}}};
    
    auto verts = vertices(g);
    // Map-based vertex range doesn't provide size() (forward iteration only)
    // REQUIRE(std::ranges::size(verts) == 3);
    
    // But we can iterate
    int count = 0;
    for ([[maybe_unused]] auto v : verts) {
        count++;
    }
    REQUIRE(count == 3);
}

TEST_CASE("adjacency_list concept - weighted edges", "[adjacency_list][concepts][graph]") {
    using Graph = std::vector<std::vector<std::pair<int, double>>>;
    
    STATIC_REQUIRE(adjacency_list<Graph>);
    
    Graph g = {{{1, 1.5}, {2, 2.5}}, {{2, 3.5}}, {}};
    
    auto verts = vertices(g);
    REQUIRE(std::ranges::size(verts) == 3);
    
    auto v0 = *verts.begin();
    auto edge_range = edges(g, v0);
    REQUIRE(std::ranges::size(edge_range) == 2);
}

TEST_CASE("adjacency_list concept - empty graph", "[adjacency_list][concepts][graph]") {
    using Graph = std::vector<std::vector<int>>;
    
    STATIC_REQUIRE(adjacency_list<Graph>);
    
    Graph g;
    auto verts = vertices(g);
    REQUIRE(std::ranges::size(verts) == 0);
}

// =============================================================================
// index_adjacency_list Concept Tests
// =============================================================================

TEST_CASE("index_adjacency_list concept - vector<vector<int>>", "[adjacency_list][concepts][index_graph]") {
    using Graph = std::vector<std::vector<int>>;
    
    // Vector's underlying iterator is random access, so index_adjacency_list is satisfied
    STATIC_REQUIRE(index_adjacency_list<Graph>);
    
    // And they satisfy adjacency_list
    STATIC_REQUIRE(adjacency_list<Graph>);
    
    Graph g = {{1, 2}, {2, 3}, {0}};
    
    auto verts = vertices(g);
    
    // Can iterate vertices
    int count = 0;
    for (auto v : verts) {
        REQUIRE(static_cast<int>(vertex_id(g, v)) == count);
        count++;
    }
}

TEST_CASE("index_adjacency_list concept - deque<deque<int>>", "[adjacency_list][concepts][index_graph]") {
    using Graph = std::deque<std::deque<int>>;
    
    // Deque's underlying iterator is random access, so index_adjacency_list is satisfied
    STATIC_REQUIRE(index_adjacency_list<Graph>);
    STATIC_REQUIRE(adjacency_list<Graph>);
}

TEST_CASE("index_adjacency_list concept - map does NOT satisfy", "[adjacency_list][concepts][index_graph]") {
    using Graph = std::map<int, std::vector<int>>;
    
    STATIC_REQUIRE_FALSE(index_adjacency_list<Graph>);
    
    // But does satisfy adjacency_list
    STATIC_REQUIRE(adjacency_list<Graph>);
}

// =============================================================================
// Concept Hierarchy Tests
// =============================================================================

TEST_CASE("Concept hierarchy - index_adjacency_list implies adjacency_list", "[adjacency_list][concepts][hierarchy]") {
    using Graph1 = std::vector<std::vector<int>>;
    
    // Vector's underlying iterator is random access, so index_adjacency_list is satisfied
    STATIC_REQUIRE(index_adjacency_list<Graph1>);
    STATIC_REQUIRE(adjacency_list<Graph1>);
    
    // Deque also satisfies index_adjacency_list (random access underlying iterator)
    using Graph2 = std::deque<std::deque<int>>;
    STATIC_REQUIRE(index_adjacency_list<Graph2>);
    STATIC_REQUIRE(adjacency_list<Graph2>);
}

TEST_CASE("Concept hierarchy - index_vertex_range implies vertex_range", "[adjacency_list][concepts][hierarchy]") {
    using Graph1 = std::vector<std::vector<int>>;
    using Graph2 = std::map<int, std::vector<int>>;
    
    
    // Vector's underlying iterator is random access, so index_vertex_range is satisfied
    STATIC_REQUIRE(index_vertex_range<Graph1>);
    STATIC_REQUIRE(vertex_range<Graph1>);
    
    // Map's underlying iterator is NOT random access, so index_vertex_range is NOT satisfied
    STATIC_REQUIRE(vertex_range<Graph2>);
    STATIC_REQUIRE_FALSE(index_vertex_range<Graph2>);
}

// =============================================================================
// Integration Tests
// =============================================================================

TEST_CASE("Concepts work with actual graph operations", "[adjacency_list][concepts][integration]") {
    using Graph = std::vector<std::vector<int>>;
    
    STATIC_REQUIRE(adjacency_list<Graph>);
    // Vector's underlying iterator is random access, so index_adjacency_list is satisfied
    STATIC_REQUIRE(index_adjacency_list<Graph>);
    
    Graph g = {{1, 2, 3}, {0, 2, 3}, {0, 1, 3}, {0, 1, 2}};
    
    // Complete graph K4
    auto verts = vertices(g);
    REQUIRE(std::ranges::size(verts) == 4);
    
    // Each vertex should have 3 outgoing edges
    for (auto v : verts) {
        auto edge_range = edges(g, v);
        REQUIRE(std::ranges::size(edge_range) == 3);
        
        // Check all edges are valid
        for (auto e : edge_range) {
            auto tid = target_id(g, e);
            REQUIRE(tid >= 0);
            REQUIRE(tid < 4);
        }
    }
}

TEST_CASE("Concepts distinguish container types correctly", "[adjacency_list][concepts][integration]") {
    // Vector and deque satisfy index_adjacency_list (random access underlying iterator)
    // Map does NOT (bidirectional iterator)
    
    using VectorGraph = std::vector<std::vector<int>>;
    STATIC_REQUIRE(index_adjacency_list<VectorGraph>);
    STATIC_REQUIRE(adjacency_list<VectorGraph>);
    
    using MapGraph = std::map<int, std::vector<int>>;
    STATIC_REQUIRE(adjacency_list<MapGraph>);
    STATIC_REQUIRE_FALSE(index_adjacency_list<MapGraph>);
    
    using DequeGraph = std::deque<std::deque<int>>;
    STATIC_REQUIRE(index_adjacency_list<DequeGraph>);
    STATIC_REQUIRE(adjacency_list<DequeGraph>);
}
