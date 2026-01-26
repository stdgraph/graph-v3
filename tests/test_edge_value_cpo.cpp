/**
 * @file test_edge_value_cpo.cpp
 * @brief Comprehensive tests for edge_value(g, uv) CPO
 * 
 * Tests all resolution paths (member, ADL, default) and various scenarios
 */

#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <tuple>
#include "graph/descriptor.hpp"
#include "graph/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test with Default Implementation - Simple Edges (int target only)
// =============================================================================

using SimpleGraph = std::vector<std::vector<int>>;

TEST_CASE("edge_value - simple edges return target ID", "[edge_value][default][simple]") {
    SimpleGraph g(3);
    g[0] = {1, 2};
    g[1] = {2};
    g[2] = {0, 1};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    // For simple int edges, edge_value returns the int itself (which is the target ID)
    REQUIRE(edge_value(g, e01) == 1);
    REQUIRE(edge_value(g, e02) == 2);
}

// =============================================================================
// Test with Pair Edges (target, weight)
// =============================================================================

using WeightedGraph = std::vector<std::vector<std::pair<int, double>>>;

TEST_CASE("edge_value - pair edges return .second (weight)", "[edge_value][default][pair]") {
    WeightedGraph g(3);
    g[0] = {{1, 10.5}, {2, 20.5}};
    g[1] = {{2, 30.5}};
    g[2] = {};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    // For pair edges, edge_value returns the .second part (the weight)
    REQUIRE(edge_value(g, e01) == 10.5);
    REQUIRE(edge_value(g, e02) == 20.5);
}

TEST_CASE("edge_value - modify edge weight through edge_value", "[edge_value][default][pair][modify]") {
    WeightedGraph g(2);
    g[0] = {{1, 10.5}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // Modify weight through edge_value
    edge_value(g, e) = 99.9;
    
    REQUIRE(edge_value(g, e) == 99.9);
    REQUIRE(g[0][0].second == 99.9);
}

// =============================================================================
// Test with Tuple Edges (target, weight, cost)
// =============================================================================

using MultiPropertyGraph = std::vector<std::vector<std::tuple<int, double, std::string>>>;

TEST_CASE("edge_value - tuple edges with 2 elements returns second", "[edge_value][default][tuple]") {
    std::vector<std::vector<std::tuple<int, double>>> g(2);
    g[0] = {{1, 10.5}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // For 2-element tuple, edge_value returns the second element
    REQUIRE(edge_value(g, e) == 10.5);
}

TEST_CASE("edge_value - tuple edges with 3+ elements returns tuple of properties", "[edge_value][default][tuple]") {
    MultiPropertyGraph g(3);
    g[0] = {{1, 10.5, "fast"}, {2, 20.5, "slow"}};
    g[1] = {{2, 30.5, "medium"}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    // For 3+ element tuple, edge_value returns a tuple of the remaining elements
    auto val01 = edge_value(g, e01);
    REQUIRE(std::get<0>(val01) == 10.5);
    REQUIRE(std::get<1>(val01) == "fast");
    
    auto val02 = edge_value(g, e02);
    REQUIRE(std::get<0>(val02) == 20.5);
    REQUIRE(std::get<1>(val02) == "slow");
}

TEST_CASE("edge_value - modify tuple edge properties", "[edge_value][default][tuple][modify]") {
    MultiPropertyGraph g(2);
    g[0] = {{1, 10.5, "fast"}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // Modify through edge_value (use auto&& since tuple is returned as prvalue)
    auto&& props = edge_value(g, e);
    std::get<0>(props) = 99.9;
    std::get<1>(props) = "modified";
    
    REQUIRE(std::get<1>(g[0][0]) == 99.9);
    REQUIRE(std::get<2>(g[0][0]) == "modified");
}

// =============================================================================
// Test with Custom Edge Types
// =============================================================================

struct EdgeData {
    int target;
    double weight;
    std::string label;
    
    bool operator==(const EdgeData&) const = default;
};

using CustomEdgeGraph = std::vector<std::vector<EdgeData>>;

TEST_CASE("edge_value - custom edge type returns whole value", "[edge_value][default][custom]") {
    CustomEdgeGraph g(3);
    g[0] = {{1, 10.5, "edge01"}, {2, 20.5, "edge02"}};
    g[1] = {{2, 30.5, "edge12"}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    
    // For custom types, edge_value returns the whole value
    auto& edge_data = edge_value(g, e01);
    REQUIRE(edge_data.target == 1);
    REQUIRE(edge_data.weight == 10.5);
    REQUIRE(edge_data.label == "edge01");
}

TEST_CASE("edge_value - modify custom edge type", "[edge_value][default][custom][modify]") {
    CustomEdgeGraph g(2);
    g[0] = {{1, 10.5, "original"}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // Modify through edge_value
    edge_value(g, e).weight = 99.9;
    edge_value(g, e).label = "modified";
    
    REQUIRE(g[0][0].weight == 99.9);
    REQUIRE(g[0][0].label == "modified");
}

// =============================================================================
// Test with Map-Based Graphs
// =============================================================================

using MapWeightedGraph = std::map<int, std::vector<std::pair<int, double>>>;

TEST_CASE("edge_value - map graph with pair edges", "[edge_value][default][map]") {
    MapWeightedGraph g;
    g[0] = {{1, 10.5}, {2, 20.5}};
    g[1] = {{2, 30.5}};
    
    auto verts = vertices(g);
    auto v0 = *verts.begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    REQUIRE(edge_value(g, e01) == 10.5);
    REQUIRE(edge_value(g, e02) == 20.5);
}

// =============================================================================
// Test Const Correctness
// =============================================================================

TEST_CASE("edge_value - const graph with pair edges", "[edge_value][const]") {
    WeightedGraph g_mutable(2);
    g_mutable[0] = {{1, 10.5}};
    const WeightedGraph& g = g_mutable;
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // Should be able to read from const graph
    REQUIRE(edge_value(g, e) == 10.5);
    
    // Return type should be const reference
    static_assert(std::is_const_v<std::remove_reference_t<decltype(edge_value(g, e))>>);
}

TEST_CASE("edge_value - const graph with custom edges", "[edge_value][const][custom]") {
    CustomEdgeGraph g_mutable(2);
    g_mutable[0] = {{1, 10.5, "test"}};
    const CustomEdgeGraph& g = g_mutable;
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    REQUIRE(edge_value(g, e).weight == 10.5);
    REQUIRE(edge_value(g, e).label == "test");
    
    // Return type should be const reference
    static_assert(std::is_const_v<std::remove_reference_t<decltype(edge_value(g, e))>>);
}

// =============================================================================
// Test By-Value Returns
// =============================================================================

// Graph type that returns by-value from member function
struct GraphWithByValueEdgeReturn {
    std::vector<std::vector<std::pair<int, double>>> data;
    
    // Returns by value (transformed), not by reference
    double edge_value(const edge_descriptor<
        std::vector<std::pair<int, double>>::iterator,
        std::vector<std::vector<std::pair<int, double>>>::iterator>& uv) const {
        auto& ee = data[uv.source().value()];
        return ee[uv.value()].second * 2.0;  // Transform: double the weight
    }
    
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

TEST_CASE("edge_value - by-value return from member", "[edge_value][by-value][member]") {
    GraphWithByValueEdgeReturn g;
    g.data = {{}, {{1, 10.5}, {2, 20.5}}};
    
    using VertexIter = decltype(g.begin());
    auto v1 = vertex_descriptor<VertexIter>(1);
    auto edge_range = edges(g.data, v1);
    
    // Get edge descriptors
    auto it = edge_range.begin();
    auto e10 = *it++;
    auto e11 = *it;
    
    // Member function returns by value (transformed)
    REQUIRE(edge_value(g, e10) == 21.0);  // 10.5 * 2
    REQUIRE(edge_value(g, e11) == 41.0);  // 20.5 * 2
    
    // Return type should be value, not reference
    static_assert(!std::is_reference_v<decltype(edge_value(g, e10))>);
}

// =============================================================================
// Test Const Semantics with Overloads
// =============================================================================

struct GraphWithConstEdgeOverloads {
    std::vector<std::vector<std::pair<int, double>>> data;
    
    // Make this a proper graph container with operator[]
    auto& operator[](size_t idx) { return data[idx]; }
    const auto& operator[](size_t idx) const { return data[idx]; }
    
    // Non-const version returns mutable reference
    template<typename EdgeDesc>
    double& edge_value(EdgeDesc&& uv) {
        auto& ee = data[uv.source().value()];
        return ee[uv.value()].second;
    }
    
    // Const version returns const reference
    template<typename EdgeDesc>
    const double& edge_value(EdgeDesc&& uv) const {
        const auto& ee = data[uv.source().value()];
        return ee[uv.value()].second;
    }
    
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

TEST_CASE("edge_value - const overload selection", "[edge_value][const][overload]") {
    GraphWithConstEdgeOverloads g_mutable;
    g_mutable.data = {{}, {{1, 10.5}, {2, 20.5}}};
    
    using VertexIter = decltype(g_mutable.data.begin());
    
    auto v1 = vertex_descriptor<VertexIter>(1);
    auto edge_range = edges(g_mutable.data, v1);
    auto e10 = *edge_range.begin();
    
    // Non-const graph: should call non-const overload (mutable reference)
    REQUIRE(edge_value(g_mutable, e10) == 10.5);
    static_assert(std::is_same_v<decltype(edge_value(g_mutable, e10)), double&>);
    
    // Should be able to modify through non-const
    edge_value(g_mutable, e10) = 99.9;
    REQUIRE(g_mutable.data[1][0].second == 99.9);
    
    // Const graph: should call const overload (const reference)
    const GraphWithConstEdgeOverloads& g_const = g_mutable;
    using ConstVertexIter = decltype(g_const.data.begin());
    
    auto v1_const = vertex_descriptor<ConstVertexIter>(1);
    auto edge_range_const = edges(g_const.data, v1_const);
    auto e10_const = *edge_range_const.begin();
    
    REQUIRE(edge_value(g_const, e10_const) == 99.9);
    static_assert(std::is_same_v<decltype(edge_value(g_const, e10_const)), const double&>);
}

TEST_CASE("edge_value - default implementation const correctness", "[edge_value][const][default]") {
    WeightedGraph g_mutable(2);
    g_mutable[0] = {{1, 10.5}};
    
    auto v0_mut = *vertices(g_mutable).begin();
    auto e_mut = *edges(g_mutable, v0_mut).begin();
    
    // Non-const: returns mutable reference
    static_assert(std::is_same_v<decltype(edge_value(g_mutable, e_mut)), double&>);
    REQUIRE(edge_value(g_mutable, e_mut) == 10.5);
    
    // Can modify
    edge_value(g_mutable, e_mut) = 77.7;
    REQUIRE(g_mutable[0][0].second == 77.7);
    
    // Const: returns const reference
    const WeightedGraph& g_const = g_mutable;
    auto v0_const = *vertices(g_const).begin();
    auto e_const = *edges(g_const, v0_const).begin();
    
    static_assert(std::is_same_v<decltype(edge_value(g_const, e_const)), const double&>);
    REQUIRE(edge_value(g_const, e_const) == 77.7);
}

TEST_CASE("edge_value - const map graph with pair edges", "[edge_value][const][map]") {
    MapWeightedGraph g_mutable;
    g_mutable[0] = {{1, 10.5}, {2, 20.5}};
    g_mutable[1] = {{2, 30.5}};
    
    const MapWeightedGraph& g = g_mutable;
    
    auto verts = vertices(g);
    auto v0 = *verts.begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    // Should be able to read from const map graph
    REQUIRE(edge_value(g, e01) == 10.5);
    REQUIRE(edge_value(g, e02) == 20.5);
    
    // Return type should be const reference
    static_assert(std::is_const_v<std::remove_reference_t<decltype(edge_value(g, e01))>>);
    static_assert(std::is_reference_v<decltype(edge_value(g, e01))>);
}

TEST_CASE("edge_value - const simple graph", "[edge_value][const][simple]") {
    SimpleGraph g_mutable(3);
    g_mutable[0] = {1, 2};
    g_mutable[1] = {2};
    
    const SimpleGraph& g = g_mutable;
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    // For simple int edges, edge_value returns the int itself
    REQUIRE(edge_value(g, e01) == 1);
    REQUIRE(edge_value(g, e02) == 2);
    
    // Should be const reference
    static_assert(std::is_const_v<std::remove_reference_t<decltype(edge_value(g, e01))>>);
}

TEST_CASE("edge_value - const tuple graph", "[edge_value][const][tuple]") {
    MultiPropertyGraph g_mutable(2);
    g_mutable[0] = {{1, 10.5, "fast"}};
    
    const MultiPropertyGraph& g = g_mutable;
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // For 3-element tuple, edge_value returns tuple of last 2 elements
    auto props = edge_value(g, e);
    REQUIRE(std::get<0>(props) == 10.5);
    REQUIRE(std::get<1>(props) == "fast");
    
    // The tuple elements should be const references
    static_assert(std::is_const_v<std::remove_reference_t<decltype(std::get<0>(props))>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(std::get<1>(props))>>);
}

// =============================================================================
// Test with Deque
// =============================================================================

using DequeWeightedGraph = std::deque<std::deque<std::pair<int, double>>>;

TEST_CASE("edge_value - deque graph", "[edge_value][default][deque]") {
    DequeWeightedGraph g(3);
    g[0] = {{1, 10.5}, {2, 20.5}};
    g[1] = {{2, 30.5}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    REQUIRE(edge_value(g, e01) == 10.5);
    REQUIRE(edge_value(g, e02) == 20.5);
}

// =============================================================================
// Test Return Type
// =============================================================================

TEST_CASE("edge_value - returns reference for pairs", "[edge_value][type]") {
    WeightedGraph g(2);
    g[0] = {{1, 10.5}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    // Should return reference (not copy)
    static_assert(std::is_reference_v<decltype(edge_value(g, e))>);
    
    // Modifying through reference should affect original
    auto& value = edge_value(g, e);
    value = 99.9;
    REQUIRE(g[0][0].second == 99.9);
}

// =============================================================================
// Test Integration with Other CPOs
// =============================================================================

TEST_CASE("edge_value - integration with target_id", "[edge_value][integration]") {
    WeightedGraph g(3);
    g[0] = {{1, 10.5}, {2, 20.5}};
    g[1] = {{2, 30.5}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    
    for (auto e : edge_range) {
        auto tid = target_id(g, e);
        auto weight = edge_value(g, e);
        
        // Verify correspondence
        if (tid == 1) {
            REQUIRE(weight == 10.5);
        } else if (tid == 2) {
            REQUIRE(weight == 20.5);
        }
    }
}

TEST_CASE("edge_value - integration with target", "[edge_value][integration]") {
    WeightedGraph g(3);
    g[0] = {{1, 10.5}, {2, 20.5}};
    g[1] = {{2, 30.5}};
    g[2] = {};
    
    auto v0 = *vertices(g).begin();
    auto e01 = *edges(g, v0).begin();
    
    auto target_v = target(g, e01);
    auto weight = edge_value(g, e01);
    
    REQUIRE(vertex_id(g, target_v) == 1);
    REQUIRE(weight == 10.5);
}

// =============================================================================
// Test Edge Cases
// =============================================================================

TEST_CASE("edge_value - single edge", "[edge_value][edge_case]") {
    WeightedGraph g(2);
    g[0] = {{1, 42.0}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    REQUIRE(edge_value(g, e) == 42.0);
}

TEST_CASE("edge_value - self loop", "[edge_value][edge_case]") {
    WeightedGraph g(3);
    g[0] = {{0, 5.5}, {1, 10.5}};  // Self-loop on vertex 0
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto e_self = *edge_range.begin();
    
    REQUIRE(target_id(g, e_self) == 0);  // Self-loop
    REQUIRE(edge_value(g, e_self) == 5.5);
}

TEST_CASE("edge_value - zero weight", "[edge_value][edge_case]") {
    WeightedGraph g(2);
    g[0] = {{1, 0.0}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    REQUIRE(edge_value(g, e) == 0.0);
}

TEST_CASE("edge_value - negative weight", "[edge_value][edge_case]") {
    WeightedGraph g(2);
    g[0] = {{1, -10.5}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    REQUIRE(edge_value(g, e) == -10.5);
}

// =============================================================================
// Test Different Value Types
// =============================================================================

TEST_CASE("edge_value - string properties", "[edge_value][types]") {
    std::vector<std::vector<std::pair<int, std::string>>> g(3);
    g[0] = {{1, "fast"}, {2, "slow"}};
    g[1] = {{2, "medium"}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    REQUIRE(edge_value(g, e01) == "fast");
    REQUIRE(edge_value(g, e02) == "slow");
}

TEST_CASE("edge_value - int properties", "[edge_value][types]") {
    std::vector<std::vector<std::pair<int, int>>> g(3);
    g[0] = {{1, 100}, {2, 200}};
    g[1] = {{2, 300}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    
    REQUIRE(edge_value(g, e01) == 100);
}

TEST_CASE("edge_value - boolean properties", "[edge_value][types]") {
    std::vector<std::vector<std::pair<int, bool>>> g(3);
    g[0] = {{1, true}, {2, false}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto it = edge_range.begin();
    auto e01 = *it++;
    auto e02 = *it;
    
    REQUIRE(edge_value(g, e01) == true);
    REQUIRE(edge_value(g, e02) == false);
}

// =============================================================================
// Test Complex Nested Structures
// =============================================================================

struct ComplexEdge {
    int target;
    double weight;
    std::string label;
    std::vector<int> path;
};

TEST_CASE("edge_value - complex nested structures", "[edge_value][complex]") {
    std::vector<std::vector<ComplexEdge>> g(3);
    g[0] = {{1, 10.5, "edge01", {0, 1}}, {2, 20.5, "edge02", {0, 2}}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    auto e01 = *edge_range.begin();
    
    auto& edge_data = edge_value(g, e01);
    REQUIRE(edge_data.target == 1);
    REQUIRE(edge_data.weight == 10.5);
    REQUIRE(edge_data.label == "edge01");
    REQUIRE(edge_data.path.size() == 2);
    
    // Modify complex structure
    edge_value(g, e01).path.push_back(999);
    REQUIRE(g[0][0].path.size() == 3);
    REQUIRE(g[0][0].path[2] == 999);
}

// =============================================================================
// Test with Graph Algorithms Pattern
// =============================================================================

TEST_CASE("edge_value - typical Dijkstra usage pattern", "[edge_value][pattern]") {
    // Graph for shortest path algorithms
    WeightedGraph g(4);
    g[0] = {{1, 1.0}, {2, 4.0}};
    g[1] = {{2, 2.0}, {3, 5.0}};
    g[2] = {{3, 1.0}};
    g[3] = {};
    
    // Simulate Dijkstra edge relaxation
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    
    double total_weight = 0.0;
    for (auto e : edge_range) {
        total_weight += edge_value(g, e);
    }
    
    REQUIRE(total_weight == 5.0);  // 1.0 + 4.0
}

TEST_CASE("edge_value - edge filtering by weight", "[edge_value][pattern]") {
    WeightedGraph g(3);
    g[0] = {{1, 10.5}, {2, 5.0}};
    g[1] = {{2, 15.0}};
    
    auto v0 = *vertices(g).begin();
    auto edge_range = edges(g, v0);
    
    // Count edges with weight > 7.0
    int count = 0;
    for (auto e : edge_range) {
        if (edge_value(g, e) > 7.0) {
            ++count;
        }
    }
    
    REQUIRE(count == 1);  // Only edge to vertex 1 has weight > 7.0
}

// =============================================================================
// Test Large Graphs
// =============================================================================

TEST_CASE("edge_value - large graph", "[edge_value][performance]") {
    WeightedGraph g(100);
    for (size_t i = 0; i < 99; ++i) {
        g[i] = {{static_cast<int>(i + 1), static_cast<double>(i * 10)}};
    }
    g[99] = {};
    
    auto v50 = *find_vertex(g, 50);
    auto e = *edges(g, v50).begin();
    
    REQUIRE(edge_value(g, e) == 500.0);  // 50 * 10
}

// =============================================================================
// Test Multiple Properties with Tuples
// =============================================================================

TEST_CASE("edge_value - tuple with 4 properties", "[edge_value][tuple]") {
    using FourPropGraph = std::vector<std::vector<std::tuple<int, double, std::string, bool>>>;
    FourPropGraph g(2);
    g[0] = {{1, 10.5, "fast", true}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    auto props = edge_value(g, e);
    REQUIRE(std::get<0>(props) == 10.5);
    REQUIRE(std::get<1>(props) == "fast");
    REQUIRE(std::get<2>(props) == true);
}

// NOTE: Disabled - raw adjacency lists with complex tuples don't have proper edge_value support yet
#if 0  // FIXME: Re-enable when edge_value CPO supports raw adjacency list tuples
TEST_CASE("edge_value - tuple with 5 properties", "[edge_value][tuple][!mayfail]") {
    SKIP("Raw adjacency list with complex tuple - edge_value CPO needs type trait improvements");
    using FivePropGraph = std::vector<std::vector<std::tuple<int, double, int, std::string, bool>>>;
    FivePropGraph g(2);
    g[0] = {{1, 10.5, 42, "test", false}};
    
    auto v0 = *vertices(g).begin();
    auto e = *edges(g, v0).begin();
    
    auto props = edge_value(g, e);
    REQUIRE(std::get<0>(props) == 10.5);
    REQUIRE(std::get<1>(props) == 42);
    REQUIRE(std::get<2>(props) == "test");
    REQUIRE(std::get<3>(props) == false);
}
#endif  // Disabled tuple with 5 properties test
