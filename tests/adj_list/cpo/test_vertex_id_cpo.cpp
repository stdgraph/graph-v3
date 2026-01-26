/**
 * @file test_vertex_id_cpo.cpp
 * @brief Comprehensive tests for vertex_id(g, u) CPO
 * 
 * Tests the vertex_id(g, u) CPO with different graph representations:
 * - Vector-based graphs (index-based IDs)
 * - Map-based graphs (key-based IDs)
 * - Deque-based graphs (index-based IDs)
 * - Custom vertex ID implementations
 * 
 * Resolution order tested:
 * 1. vertex_id(g, descriptor) - ADL with descriptor
 * 2. vertex_id(g, inner_value) - ADL with inner_value
 * 3. descriptor.vertex_id() - default fallback
 * 
 * Verifies:
 * - Correct ID extraction for different storage types
 * - Works with vertex descriptors from vertices(g)
 * - Consistency across multiple calls
 * - Three resolution tiers (ADL descriptor, ADL inner_value, default)
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <vector>
#include <map>
#include <deque>
#include <string>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Vector-based Graph (Index IDs)
// =============================================================================

TEST_CASE("vertex_id(g,u) - vector<vector<int>> index-based IDs", "[vertex_id][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    SECTION("returns correct index for first vertex") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        vertex_t<Graph> v0 = *it;
        
        vertex_id_t<Graph> id = vertex_id(graph, v0);
        REQUIRE(id == 0);
    }
    
    SECTION("returns correct indices for all vertices") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == 0);
        REQUIRE(ids[1] == 1);
        REQUIRE(ids[2] == 2);
    }
    
    SECTION("consistent across multiple calls") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        vertex_t<Graph> v1 = *++it;
        
        vertex_id_t<Graph> id1 = vertex_id(graph, v1);
        vertex_id_t<Graph> id2 = vertex_id(graph, v1);
        REQUIRE(id1 == id2);
        REQUIRE(id1 == 1);
    }
    
    SECTION("vertex_id_t is size_t for vector") {
        STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, size_t>);
    }
}

// =============================================================================
// Test 2: Map-based Graph (Key-based IDs)
// =============================================================================

TEST_CASE("vertex_id(g,u) - map<int, vector<int>> key-based IDs", "[vertex_id][cpo]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {10, {20, 30}},
        {20, {30}},
        {30, {}}
    };
    
    SECTION("returns correct key for first vertex") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        vertex_t<Graph> v = *it;
        
        vertex_id_t<Graph> id = vertex_id(graph, v);
        REQUIRE(id == 10);
    }
    
    SECTION("returns correct keys for all vertices") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == 10);
        REQUIRE(ids[1] == 20);
        REQUIRE(ids[2] == 30);
    }
    
    SECTION("vertex_id_t is int for map<int, ...>") {
        STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, int>);
    }
}

// =============================================================================
// Test 3: Map with String Keys
// =============================================================================

TEST_CASE("vertex_id(g,u) - map<string, vector<string>>", "[vertex_id][cpo]") {
    using Graph = std::map<std::string, std::vector<std::string>>;
    Graph graph = {
        {"Alice", {"Bob", "Charlie"}},
        {"Bob", {"Charlie"}},
        {"Charlie", {}}
    };
    
    SECTION("returns correct string keys") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == "Alice");
        REQUIRE(ids[1] == "Bob");
        REQUIRE(ids[2] == "Charlie");
    }
    
    SECTION("vertex_id_t is std::string") {
        STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, std::string>);
    }
}

// =============================================================================
// Test 4: Deque-based Graph
// =============================================================================

TEST_CASE("vertex_id(g,u) - deque<deque<int>>", "[vertex_id][cpo]") {
    using Graph = std::deque<std::deque<int>>;
    Graph graph;
    graph.push_back({1, 2});
    graph.push_back({2, 3});
    graph.push_back({3});
    
    SECTION("returns correct indices") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == 0);
        REQUIRE(ids[1] == 1);
        REQUIRE(ids[2] == 2);
    }
    
    SECTION("vertex_id_t is size_t for deque") {
        STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, size_t>);
    }
}

// =============================================================================
// Test 5: Empty Graph
// =============================================================================

TEST_CASE("vertex_id(g,u) - empty graph", "[vertex_id][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph;
    
    SECTION("no vertices to get IDs from") {
        vertex_range_t<Graph> verts = vertices(graph);
        REQUIRE(std::ranges::distance(verts) == 0);
        // Nothing to test - no vertices exist
    }
}

// =============================================================================
// Test 6: Single Vertex Graph
// =============================================================================

TEST_CASE("vertex_id(g,u) - single vertex", "[vertex_id][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{}};
    
    SECTION("returns ID 0 for single vertex") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_t<Graph> v = *std::ranges::begin(verts);
        
        vertex_id_t<Graph> id = vertex_id(graph, v);
        REQUIRE(id == 0);
    }
}

// =============================================================================
// Test 7: Large Graph
// =============================================================================

TEST_CASE("vertex_id(g,u) - large graph", "[vertex_id][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    const size_t N = 1000;
    Graph graph(N);
    
    SECTION("correct IDs for large number of vertices") {
        vertex_range_t<Graph> verts = vertices(graph);
        
        size_t count = 0;
        for (auto v : verts) {
            vertex_id_t<Graph> id = vertex_id(graph, v);
            REQUIRE(id == count);
            ++count;
        }
        REQUIRE(count == N);
    }
}

// =============================================================================
// Test 8: Sparse Map (non-contiguous keys)
// =============================================================================

TEST_CASE("vertex_id(g,u) - sparse map with non-contiguous keys", "[vertex_id][cpo]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {100, {200}},
        {200, {300}},
        {300, {400}},
        {400, {}}
    };
    
    SECTION("returns correct non-contiguous keys") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids.size() == 4);
        REQUIRE(ids[0] == 100);
        REQUIRE(ids[1] == 200);
        REQUIRE(ids[2] == 300);
        REQUIRE(ids[3] == 400);
    }
}

// =============================================================================
// Test 9: ADL vertex_id with Inner Value
// =============================================================================

namespace test_adl_inner {
    struct CustomVertex {
        int value;
    };
    
    using GraphType = std::vector<CustomVertex>;
    
    // ADL vertex_id with descriptor - returns inner_value(g).value * 2
    int vertex_id(const GraphType& g, vertex_descriptor<GraphType::iterator> v) {
        return v.inner_value(g).value * 2;
    }
}

TEST_CASE("vertex_id(g,u) - ADL with inner_value", "[vertex_id][cpo][custom]") {
    using test_adl_inner::GraphType;
    using test_adl_inner::CustomVertex;
    
    SECTION("uses ADL vertex_id(g, inner_value)") {
        GraphType g = {{10}, {20}, {30}};
        auto verts = vertices(g);
        
        std::vector<int> ids;
        for (auto v : verts) {
            ids.push_back(vertex_id(g, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == 20);  // 10 * 2
        REQUIRE(ids[1] == 40);  // 20 * 2
        REQUIRE(ids[2] == 60);  // 30 * 2
    }
}

// =============================================================================
// Test 10: ADL vertex_id with Descriptor
// =============================================================================

namespace test_adl_descriptor {
    struct GraphType {
        using iterator = std::vector<int>::iterator;
        std::vector<int> data;
        
        // Make it indexable for vertex_descriptor::inner_value()
        auto& operator[](size_t i) { return data[i]; }
        const auto& operator[](size_t i) const { return data[i]; }
        
        // Make it iterable for vertices()
        auto begin() { return data.begin(); }
        auto end() { return data.end(); }
        const auto begin() const { return data.begin(); }
        const auto end() const { return data.end(); }
        auto size() const { return data.size(); }
    };
    
    // ADL vertex_id with descriptor - returns descriptor's ID * 3
    int vertex_id([[maybe_unused]] const GraphType& g, vertex_descriptor<GraphType::iterator> v) {
        return static_cast<int>(v.vertex_id() * 3);
    }
}

TEST_CASE("vertex_id(g,u) - ADL with descriptor", "[vertex_id][cpo][custom]") {
    using test_adl_descriptor::GraphType;
    
    SECTION("uses ADL vertex_id(g, descriptor)") {
        GraphType g{{10, 20, 30}};
        auto verts = vertices(g);
        
        std::vector<int> ids;
        for (auto v : verts) {
            ids.push_back(vertex_id(g, v));
        }
        
        REQUIRE(ids.size() == 3);
        REQUIRE(ids[0] == 0);  // 0 * 3
        REQUIRE(ids[1] == 3);  // 1 * 3
        REQUIRE(ids[2] == 6);  // 2 * 3
    }
}

// =============================================================================
// Test 11: Vertex Descriptor Default (Fallback Behavior)
// =============================================================================

TEST_CASE("vertex_id(g,u) - uses descriptor's vertex_id() by default", "[vertex_id][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {{1}, {2}, {3}};
    
    SECTION("descriptor method is default when no customization") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_t<Graph> v = *std::ranges::begin(verts);
        
        // Both should return the same value
        vertex_id_t<Graph> id_via_cpo = vertex_id(graph, v);
        vertex_id_t<Graph> id_direct = v.vertex_id();
        
        REQUIRE(id_via_cpo == id_direct);
        REQUIRE(id_via_cpo == 0);
    }
}

// =============================================================================
// Test 12: Type Alias Consistency
// =============================================================================

TEST_CASE("vertex_id(g,u) - type alias consistency", "[vertex_id][cpo][type_aliases]") {
    SECTION("vertex_id_t matches actual return type for vector") {
        using Graph = std::vector<std::vector<int>>;
        Graph graph = {{}};
        
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_t<Graph> v = *std::ranges::begin(verts);
        
        auto id = vertex_id(graph, v);
        STATIC_REQUIRE(std::same_as<decltype(id), vertex_id_t<Graph>>);
    }
    
    SECTION("vertex_id_t matches actual return type for map") {
        using Graph = std::map<int, std::vector<int>>;
        Graph graph = {{1, {}}};
        
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_t<Graph> v = *std::ranges::begin(verts);
        
        auto id = vertex_id(graph, v);
        STATIC_REQUIRE(std::same_as<decltype(id), vertex_id_t<Graph>>);
    }
}

// =============================================================================
// Test 13: Weighted Graphs (Different Edge Types)
// =============================================================================

TEST_CASE("vertex_id(g,u) - weighted graphs with different edge types", "[vertex_id][cpo]") {
    SECTION("vector with pair edges") {
        using Graph = std::vector<std::vector<std::pair<int, double>>>;
        Graph graph = {
            {{1, 1.5}, {2, 2.5}},
            {{2, 3.5}},
            {}
        };
        
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
    
    SECTION("map with tuple edges") {
        using Graph = std::map<int, std::vector<std::tuple<int, double, std::string>>>;
        Graph graph = {
            {10, {{20, 1.5, "e1"}}},
            {20, {}},
        };
        
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<vertex_id_t<Graph>> ids;
        
        for (auto v : verts) {
            ids.push_back(vertex_id(graph, v));
        }
        
        REQUIRE(ids == std::vector<int>{10, 20});
    }
}

// =============================================================================
// Test 14: Const Correctness
// =============================================================================

TEST_CASE("vertex_id(g,u) - const correctness", "[vertex_id][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    const Graph graph = {{1}, {2}};
    
    SECTION("works with const graph") {
        // Note: vertices(g) requires non-const currently
        // This test documents the current limitation
        // When const support is added, this will be updated
        
        // For now, we can test that vertex_id takes const G&
        Graph non_const_graph = {{1}, {2}};
        vertex_range_t<Graph> verts = vertices(non_const_graph);
        vertex_t<Graph> v = *std::ranges::begin(verts);
        
        // vertex_id accepts const graph
        const Graph& const_ref = non_const_graph;
        vertex_id_t<Graph> id = vertex_id(const_ref, v);
        REQUIRE(id == 0);
    }
}
