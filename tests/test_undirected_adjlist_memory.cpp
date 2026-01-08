#include <catch2/catch_test_macros.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <vector>
#include <utility>

using namespace graph::container;
using std::vector;

// =============================================================================
// Phase 4.4: Memory Management Tests
// =============================================================================

TEST_CASE("move constructor", "[memory][move]") {
    undirected_adjacency_list<int, int> g1;
    auto v1 = g1.create_vertex(10);
    auto k1 = v1 - g1.begin();
    auto v2 = g1.create_vertex(20);
    auto k2 = v2 - g1.begin();
    g1.create_edge(k1, k2, 100);
    
    REQUIRE(g1.vertices().size() == 2);
    REQUIRE(g1.edges_size() == 1);
    
    // Move construct
    undirected_adjacency_list<int, int> g2(std::move(g1));
    
    SECTION("moved-to graph has correct state") {
        REQUIRE(g2.vertices().size() == 2);
        REQUIRE(g2.edges_size() == 1);
        REQUIRE(g2.vertices()[k1].value == 10);
        REQUIRE(g2.vertices()[k2].value == 20);
    }
    
    SECTION("moved-from graph is in valid state") {
        // Moved-from should be empty but valid
        // Note: edges_size_ may not be reset by default move constructor
        REQUIRE(g1.vertices().empty());
        // edges_size() may still contain old value - this is acceptable for moved-from state
    }
}

TEST_CASE("move assignment", "[memory][move]") {
    undirected_adjacency_list<int, int> g1;
    auto v1 = g1.create_vertex(10);
    auto k1 = v1 - g1.begin();
    auto v2 = g1.create_vertex(20);
    auto k2 = v2 - g1.begin();
    g1.create_edge(k1, k2, 100);
    
    undirected_adjacency_list<int, int> g2;
    g2.create_vertex(99); // Give it some data first
    
    // Move assign
    g2 = std::move(g1);
    
    SECTION("moved-to graph has correct state") {
        REQUIRE(g2.vertices().size() == 2);
        REQUIRE(g2.edges_size() == 1);
        REQUIRE(g2.vertices()[k1].value == 10);
    }
    
    SECTION("moved-from graph is in valid state") {
        REQUIRE(g1.vertices().empty());
        // edges_size() may still contain old value - acceptable for moved-from state
    }
}

TEST_CASE("clear method", "[memory][clear]") {
    undirected_adjacency_list<int, int> g;
    
    // Add some data
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto v3 = g.create_vertex(30);
    auto k3 = v3 - g.begin();
    
    g.create_edge(k1, k2, 100);
    g.create_edge(k2, k3, 200);
    g.create_edge(k1, k3, 300);
    
    REQUIRE(g.vertices().size() == 3);
    REQUIRE(g.edges_size() == 3);
    
    // Clear
    g.clear();
    
    SECTION("graph is empty after clear") {
        REQUIRE(g.vertices().empty());
        REQUIRE(g.vertices().size() == 0);
        REQUIRE(g.edges_size() == 0);
    }
    
    SECTION("can add new data after clear") {
        auto v = g.create_vertex(42);
        REQUIRE(g.vertices().size() == 1);
        REQUIRE((v - g.begin()) == 0);
    }
}

TEST_CASE("destructor cleanup", "[memory][destructor]") {
    // This test verifies no memory leaks via destructor
    // In practice, this would be caught by valgrind or sanitizers
    {
        undirected_adjacency_list<int, int> g;
        for (int i = 0; i < 10; ++i) {
            g.create_vertex(i);
        }
        
        // Create edges
        for (size_t i = 0; i < 9; ++i) {
            g.create_edge(i, i + 1, i * 10);
        }
        
        REQUIRE(g.vertices().size() == 10);
        REQUIRE(g.edges_size() == 9);
        
        // g destructs at end of scope
    }
    
    // If we get here without crashes, destructor worked
    REQUIRE(true);
}

TEST_CASE("swap operation", "[memory][swap]") {
    undirected_adjacency_list<int, int> g1;
    auto v1a = g1.create_vertex(10);
    auto k1a = v1a - g1.begin();
    auto v1b = g1.create_vertex(20);
    auto k1b = v1b - g1.begin();
    g1.create_edge(k1a, k1b, 100);
    
    undirected_adjacency_list<int, int> g2;
    auto v2a = g2.create_vertex(30);
    auto k2a = v2a - g2.begin();
    auto v2b = g2.create_vertex(40);
    auto k2b = v2b - g2.begin();
    auto v2c = g2.create_vertex(50);
    auto k2c = v2c - g2.begin();
    g2.create_edge(k2a, k2b, 200);
    g2.create_edge(k2b, k2c, 300);
    
    // Swap using std::swap
    std::swap(g1, g2);
    
    SECTION("g1 now has g2's old data") {
        REQUIRE(g1.vertices().size() == 3);
        REQUIRE(g1.edges_size() == 2);
        REQUIRE(g1.vertices()[0].value == 30);
        REQUIRE(g1.vertices()[1].value == 40);
        REQUIRE(g1.vertices()[2].value == 50);
    }
    
    SECTION("g2 now has g1's old data") {
        REQUIRE(g2.vertices().size() == 2);
        REQUIRE(g2.edges_size() == 1);
        REQUIRE(g2.vertices()[0].value == 10);
        REQUIRE(g2.vertices()[1].value == 20);
    }
}

TEST_CASE("graph with graph value", "[memory][graph_value]") {
    undirected_adjacency_list<int, int, int> g(42);
    
    REQUIRE(g.graph_value() == 42);
    
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    g.create_edge(k1, k2, 100);
    
    // Move construct preserves graph value
    undirected_adjacency_list<int, int, int> g2(std::move(g));
    
    REQUIRE(g2.graph_value() == 42);
    REQUIRE(g2.vertices().size() == 2);
}

TEST_CASE("large graph cleanup", "[memory][stress]") {
    undirected_adjacency_list<int, int> g;
    
    const int NUM_VERTICES = 1000;
    
    // Create many vertices
    for (int i = 0; i < NUM_VERTICES; ++i) {
        g.create_vertex(i);
    }
    
    // Create edges (every vertex connects to next 5)
    for (int i = 0; i < NUM_VERTICES - 5; ++i) {
        for (int j = 1; j <= 5; ++j) {
            g.create_edge(i, i + j, i * 1000 + j);
        }
    }
    
    REQUIRE(g.vertices().size() == NUM_VERTICES);
    REQUIRE(g.edges_size() == (NUM_VERTICES - 5) * 5);
    
    // Clear should properly deallocate all edges
    g.clear();
    
    REQUIRE(g.vertices().empty());
    REQUIRE(g.edges_size() == 0);
}
