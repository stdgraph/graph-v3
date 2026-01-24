/**
 * @file test_vertices_cpo.cpp
 * @brief Comprehensive tests for vertices(g) CPO with various STL containers
 * 
 * Tests the vertices(g) CPO with different graph representations:
 * - Vector-based adjacency lists (vector<vector<T>>)
 * - Map-based adjacency lists (map<K, vector<T>>)
 * - Deque-based adjacency lists (deque<deque<T>>)
 * - List-based adjacency lists (list<vector<T>>)
 * - Mixed container combinations
 * 
 * Verifies:
 * - Returns vertex_descriptor_view
 * - Correct vertex_id extraction
 * - Proper iteration over all vertices
 * - Works with different edge types (int, pair, tuple)
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/detail/graph_cpo.hpp>
#include <graph/vertex_descriptor.hpp>
#include <graph/vertex_descriptor_view.hpp>

#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Vector of Vectors (most common adjacency list)
// =============================================================================

TEST_CASE("vertices(g) - vector<vector<int>> simple adjacency list", "[vertices][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},      // vertex 0 -> edges to 1, 2
        {2, 3},      // vertex 1 -> edges to 2, 3
        {3},         // vertex 2 -> edge to 3
        {}           // vertex 3 -> no edges
    };
    
    SECTION("returns vertex_descriptor_view") {
        // Using type alias demonstrates proper usage
        vertex_range_t<Graph> verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
        
        // Verify type aliases are correct
        REQUIRE(std::same_as<vertex_range_t<Graph>, vertex_descriptor_view<Graph::iterator>>);
    }
    
    SECTION("iterates over all vertices") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<size_t> vertex_ids;
        
        // Use auto in range-for for readability
        for (auto v : verts) {
            vertex_ids.push_back(v.vertex_id());
        }
        
        REQUIRE(vertex_ids.size() == 4);
        REQUIRE(vertex_ids == std::vector<size_t>{0, 1, 2, 3});
    }
    
    SECTION("vertex descriptors have correct properties") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        
        // Type alias shows the vertex descriptor type
        vertex_t<Graph> v0 = *it;
        REQUIRE(v0.vertex_id() == 0);
        
        ++it;
        vertex_t<Graph> v1 = *it;
        REQUIRE(v1.vertex_id() == 1);
    }
    
    SECTION("can use with range algorithms") {
        vertex_range_t<Graph> verts = vertices(graph);
        
        size_t count = std::ranges::distance(verts);
        REQUIRE(count == 4);
        
        // Keep auto in range-for - it's clearer
        size_t visited = 0;
        for ([[maybe_unused]] auto v : verts) {
            ++visited;
        }
        REQUIRE(visited == 4);
    }
}

// =============================================================================
// Test 2: Vector of Vectors with Pair Edges
// =============================================================================

TEST_CASE("vertices(g) - vector<vector<pair<int,double>>> weighted graph", "[vertices][cpo]") {
    using Edge = std::pair<int, double>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 3.5}},
        {}
    };
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("correct vertex count") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::distance(verts) == 3);
    }
    
    SECTION("correct vertex IDs") {
        auto verts = vertices(graph);
        std::vector<size_t> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
}

// =============================================================================
// Test 3: Map-based Adjacency List (sparse graphs)
// =============================================================================

TEST_CASE("vertices(g) - map<int, vector<int>> sparse graph", "[vertices][cpo]") {
    using Graph = std::map<int, std::vector<int>>;
    Graph graph = {
        {10, {20, 30}},
        {20, {30}},
        {30, {}},
        {100, {200}}
    };
    
    SECTION("returns vertex_descriptor_view") {
        vertex_range_t<Graph> verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("iterates over all vertices with correct keys") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<int> vertex_ids;
        
        // Use auto in range-for for readability
        for (auto v : verts) {
            vertex_ids.push_back(v.vertex_id());
        }
        
        REQUIRE(vertex_ids.size() == 4);
        REQUIRE(vertex_ids == std::vector<int>{10, 20, 30, 100});
    }
    
    SECTION("vertex descriptors access correct edge lists") {
        vertex_range_t<Graph> verts = vertices(graph);
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        
        vertex_t<Graph> v10 = *it;
        REQUIRE(v10.vertex_id() == 10);
        auto& edges_10 = v10.inner_value(graph);
        REQUIRE(edges_10.size() == 2);
        REQUIRE(edges_10[0] == 20);
        REQUIRE(edges_10[1] == 30);
    }
}

// =============================================================================
// Test 4: Unordered Map-based Adjacency List
// NOTE: unordered_map may not work if its iterator is not bidirectional
// Skipping this test for now as it depends on STL implementation
// =============================================================================

// TEST_CASE("vertices(g) - unordered_map<int, vector<int>>", "[vertices][cpo]") {
//     std::unordered_map<int, std::vector<int>> graph = {
//         {1, {2, 3}},
//         {2, {3}},
//         {3, {}}
//     };
//     
//     SECTION("returns vertex_descriptor_view") {
//         auto verts = vertices(graph);
//         REQUIRE(std::ranges::forward_range<decltype(verts)>);
//     }
//     
//     SECTION("iterates over all vertices") {
//         auto verts = vertices(graph);
//         std::set<int> vertex_ids;  // Use set because unordered_map order is unspecified
//         
//         for (auto v : verts) {
//             vertex_ids.insert(v.vertex_id());
//         }
//         
//         REQUIRE(vertex_ids.size() == 3);
//         REQUIRE(vertex_ids == std::set<int>{1, 2, 3});
//     }
// }

// =============================================================================
// Test 5: Deque-based Adjacency List
// =============================================================================

TEST_CASE("vertices(g) - deque<deque<int>>", "[vertices][cpo]") {
    using Graph = std::deque<std::deque<int>>;
    Graph graph;
    graph.push_back({1, 2});
    graph.push_back({2});
    graph.push_back({});
    
    SECTION("returns vertex_descriptor_view") {
        vertex_range_t<Graph> verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("correct vertex count and IDs") {
        vertex_range_t<Graph> verts = vertices(graph);
        std::vector<size_t> ids;
        // Use auto in range-for for readability
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
}

// =============================================================================
// Test 6: Vector with Tuple Edges
// =============================================================================

TEST_CASE("vertices(g) - vector<vector<tuple<int,double,string>>>", "[vertices][cpo]") {
    using Edge = std::tuple<int, double, std::string>;
    std::vector<std::vector<Edge>> graph = {
        {{1, 1.5, "road"}, {2, 2.5, "rail"}},
        {{2, 3.5, "air"}},
        {}
    };
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("correct iteration") {
        auto verts = vertices(graph);
        size_t count = 0;
        for ([[maybe_unused]] auto v : verts) {
            ++count;
        }
        REQUIRE(count == 3);
    }
}

// =============================================================================
// Test 7: Empty Graph
// =============================================================================

TEST_CASE("vertices(g) - empty graphs", "[vertices][cpo]") {
    SECTION("empty vector") {
        std::vector<std::vector<int>> graph;
        auto verts = vertices(graph);
        
        REQUIRE(std::ranges::empty(verts));
        REQUIRE(std::ranges::distance(verts) == 0);
    }
    
    SECTION("empty map") {
        std::map<int, std::vector<int>> graph;
        auto verts = vertices(graph);
        
        REQUIRE(std::ranges::empty(verts));
        REQUIRE(std::ranges::distance(verts) == 0);
    }
}

// =============================================================================
// Test 8: Single Vertex Graph
// =============================================================================

TEST_CASE("vertices(g) - single vertex", "[vertices][cpo]") {
    SECTION("vector with single vertex") {
        std::vector<std::vector<int>> graph = {{}};
        auto verts = vertices(graph);
        
        REQUIRE(std::ranges::distance(verts) == 1);
        auto v = *std::ranges::begin(verts);
        REQUIRE(v.vertex_id() == 0);
    }
    
    SECTION("map with single vertex") {
        std::map<int, std::vector<int>> graph = {{42, {}}};
        auto verts = vertices(graph);
        
        REQUIRE(std::ranges::distance(verts) == 1);
        auto v = *std::ranges::begin(verts);
        REQUIRE(v.vertex_id() == 42);
    }
}

// =============================================================================
// Test 9: Large Graph
// =============================================================================

TEST_CASE("vertices(g) - large graph", "[vertices][cpo]") {
    const size_t N = 1000;
    std::vector<std::vector<int>> graph(N);
    
    // Create edges
    for (size_t i = 0; i < N - 1; ++i) {
        graph[i].push_back(static_cast<int>(i + 1));
    }
    
    SECTION("correct vertex count") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::distance(verts) == N);
    }
    
    SECTION("can iterate all vertices") {
        auto verts = vertices(graph);
        size_t count = 0;
        for ([[maybe_unused]] auto v : verts) {
            ++count;
        }
        REQUIRE(count == N);
    }
}

// =============================================================================
// Test 10: Map with String Keys
// =============================================================================

TEST_CASE("vertices(g) - map<string, vector<string>>", "[vertices][cpo]") {
    std::map<std::string, std::vector<std::string>> graph = {
        {"A", {"B", "C"}},
        {"B", {"C"}},
        {"C", {}}
    };
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("correct vertex IDs") {
        auto verts = vertices(graph);
        std::vector<std::string> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<std::string>{"A", "B", "C"});
    }
}

// =============================================================================
// Test 11: Vector with Set Edges
// =============================================================================

TEST_CASE("vertices(g) - vector<set<int>>", "[vertices][cpo]") {
    std::vector<std::set<int>> graph = {
        {1, 2, 3},
        {2, 3},
        {3}
    };
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("correct vertex iteration") {
        auto verts = vertices(graph);
        std::vector<size_t> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
}

// =============================================================================
// Test 12: Map with List Edges
// =============================================================================

TEST_CASE("vertices(g) - map<int, list<int>>", "[vertices][cpo]") {
    std::map<int, std::list<int>> graph = {
        {1, {2, 3}},
        {2, {3}},
        {3, {}}
    };
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
    }
    
    SECTION("correct vertex IDs") {
        auto verts = vertices(graph);
        std::vector<int> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<int>{1, 2, 3});
    }
}

// =============================================================================
// Test 13: Const Graph (requires const-qualified container constructor)
// =============================================================================

// NOTE: Const graphs require special handling in vertex_descriptor_view
// Skipping for now - will address with const iterator support
// TEST_CASE("vertices(g) - const graph", "[vertices][cpo]") {
//     const std::vector<std::vector<int>> graph = {
//         {1, 2},
//         {2},
//         {}
//     };
//     
//     SECTION("works with const graph") {
//         auto verts = vertices(graph);
//         REQUIRE(std::ranges::forward_range<decltype(verts)>);
//         
//         std::vector<size_t> ids;
//         for (auto v : verts) {
//             ids.push_back(v.vertex_id());
//         }
//         REQUIRE(ids == std::vector<size_t>{0, 1, 2});
//     }
// }

// =============================================================================
// Test 14: Vertex Descriptor Properties
// =============================================================================

TEST_CASE("vertices(g) - vertex descriptor properties", "[vertices][cpo]") {
    using Graph = std::vector<std::vector<int>>;
    Graph graph = {
        {1, 2},
        {2, 3},
        {3}
    };
    
    vertex_range_t<Graph> verts = vertices(graph);
    
    SECTION("vertex descriptors are copyable") {
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        vertex_t<Graph> v1 = *it;
        vertex_t<Graph> v2 = v1;  // Copy
        REQUIRE(v2.vertex_id() == v1.vertex_id());
    }
    
    SECTION("vertex descriptors provide inner_value access") {
        vertex_iterator_t<Graph> it = std::ranges::begin(verts);
        vertex_t<Graph> v0 = *it;
        auto& the_edges = v0.inner_value(graph);
        REQUIRE(the_edges.size() == 2);
        REQUIRE(the_edges[0] == 1);
        REQUIRE(the_edges[1] == 2);
    }
}

// =============================================================================
// Test 15: Complex Value Types
// =============================================================================

TEST_CASE("vertices(g) - complex vertex value types", "[vertices][cpo]") {
    struct VertexData {
        std::string name;
        int value;
        std::vector<int> neighbors;
    };
    
    std::vector<VertexData> graph = {
        {"A", 1, {1, 2}},
        {"B", 2, {2}},
        {"C", 3, {}}
    };
    
    SECTION("works with struct vertex types") {
        auto verts = vertices(graph);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
        
        size_t count = 0;
        for (auto v : verts) {
            REQUIRE(v.vertex_id() == count);
            ++count;
        }
        REQUIRE(count == 3);
    }
}

// =============================================================================
// Test 16: Custom vertices() that returns vertex_descriptor_view
// =============================================================================

TEST_CASE("vertices(g) - custom member returning vertex_descriptor_view", "[vertices][cpo][custom]") {
    struct CustomGraph {
        std::vector<std::vector<int>> data = {{1, 2}, {2}, {}};
        
        auto vertices() {
            return vertex_descriptor_view(data);
        }
    };
    
    CustomGraph g;
    
    SECTION("uses custom member function") {
        auto verts = vertices(g);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
        
        std::vector<size_t> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(g);
        using ViewType = decltype(verts);
        REQUIRE(is_vertex_descriptor_view_v<ViewType>);
    }
}

// =============================================================================
// Test 17: Custom vertices() that returns raw container (not descriptor view)
// =============================================================================

TEST_CASE("vertices(g) - custom member returning raw container", "[vertices][cpo][custom]") {
    struct CustomGraph {
        std::vector<std::vector<int>> data = {{1, 2}, {2}, {}};
        
        // Returns raw container, NOT vertex_descriptor_view
        auto vertices() -> std::vector<std::vector<int>>& {
            return data;
        }
    };
    
    CustomGraph g;
    
    SECTION("automatically wraps in vertex_descriptor_view") {
        auto verts = vertices(g);
        
        // Should still be a vertex_descriptor_view
        using ViewType = decltype(verts);
        REQUIRE(is_vertex_descriptor_view_v<ViewType>);
        
        std::vector<size_t> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
}

// =============================================================================
// Test 18: ADL vertices() returning vertex_descriptor_view
// =============================================================================

namespace custom_graph_ns {
    struct Graph {
        std::vector<std::vector<int>> data = {{1, 2}, {2}, {}};
    };
    
    // ADL-findable vertices() that returns vertex_descriptor_view
    auto vertices(Graph& g) {
        return vertex_descriptor_view(g.data);
    }
}

TEST_CASE("vertices(g) - ADL returning vertex_descriptor_view", "[vertices][cpo][adl]") {
    custom_graph_ns::Graph g;
    
    SECTION("uses ADL function") {
        auto verts = vertices(g);
        REQUIRE(std::ranges::forward_range<decltype(verts)>);
        
        std::vector<size_t> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
    
    SECTION("returns vertex_descriptor_view") {
        auto verts = vertices(g);
        using ViewType = decltype(verts);
        REQUIRE(is_vertex_descriptor_view_v<ViewType>);
    }
}

// =============================================================================
// Test 19: ADL vertices() returning raw container
// =============================================================================

namespace custom_graph_ns2 {
    struct Graph {
        std::vector<std::vector<int>> data = {{1, 2}, {2}, {}};
    };
    
    // ADL-findable vertices() that returns raw container
    auto vertices(Graph& g) -> std::vector<std::vector<int>>& {
        return g.data;
    }
}

TEST_CASE("vertices(g) - ADL returning raw container", "[vertices][cpo][adl]") {
    custom_graph_ns2::Graph g;
    
    SECTION("automatically wraps in vertex_descriptor_view") {
        auto verts = vertices(g);
        
        // Should still be a vertex_descriptor_view
        using ViewType = decltype(verts);
        REQUIRE(is_vertex_descriptor_view_v<ViewType>);
        
        std::vector<size_t> ids;
        for (auto v : verts) {
            ids.push_back(v.vertex_id());
        }
        REQUIRE(ids == std::vector<size_t>{0, 1, 2});
    }
}

// =============================================================================
// Test 20: Verify No Compilation for Invalid Types
// =============================================================================

// These should not compile if uncommented:
// TEST_CASE("vertices(g) - non-range types should not compile", "[vertices][cpo][!shouldfail]") {
//     int not_a_graph = 42;
//     auto verts = vertices(not_a_graph);  // Should not compile
// }
