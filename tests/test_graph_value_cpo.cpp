/**
 * @file test_graph_value_cpo.cpp
 * @brief Comprehensive tests for graph_value(g) CPO
 */

#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>
#include <string>
#include <deque>
#include "graph/adj_list/descriptor.hpp"
#include "graph/adj_list/detail/graph_cpo.hpp"

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Test with Member Function (Highest Priority)
// =============================================================================

struct GraphMetadata {
    std::string name;
    int version;
    double weight_scale;
};

struct GraphWithMemberValue {
    std::vector<std::vector<int>> data;
    GraphMetadata metadata{"TestGraph", 1, 1.0};
    
    // Non-const version returns mutable reference
    GraphMetadata& graph_value() {
        return metadata;
    }
    
    // Const version returns const reference
    const GraphMetadata& graph_value() const {
        return metadata;
    }
};

TEST_CASE("graph_value - member function returns reference", "[graph_value][member]") {
    GraphWithMemberValue g;
    g.data = {{1, 2}, {0, 2}, {0, 1}};
    
    auto& meta = graph_value(g);
    REQUIRE(meta.name == "TestGraph");
    REQUIRE(meta.version == 1);
    REQUIRE(meta.weight_scale == 1.0);
}

TEST_CASE("graph_value - modify through member function", "[graph_value][member][modify]") {
    GraphWithMemberValue g;
    
    // Modify through graph_value
    auto& meta = graph_value(g);
    meta.name = "ModifiedGraph";
    meta.version = 2;
    meta.weight_scale = 2.5;
    
    REQUIRE(g.metadata.name == "ModifiedGraph");
    REQUIRE(g.metadata.version == 2);
    REQUIRE(g.metadata.weight_scale == 2.5);
}

TEST_CASE("graph_value - const graph returns const reference", "[graph_value][member][const]") {
    GraphWithMemberValue g;
    const GraphWithMemberValue& g_const = g;
    
    const auto& meta = graph_value(g_const);
    REQUIRE(meta.name == "TestGraph");
    
    // Verify return type is const reference
    static_assert(std::is_same_v<decltype(graph_value(g_const)), const GraphMetadata&>);
}

TEST_CASE("graph_value - non-const graph returns mutable reference", "[graph_value][member][const]") {
    GraphWithMemberValue g;
    
    auto& meta = graph_value(g);
    meta.version = 99;
    
    REQUIRE(g.metadata.version == 99);
    
    // Verify return type is mutable reference
    static_assert(std::is_same_v<decltype(graph_value(g)), GraphMetadata&>);
}

// =============================================================================
// Test with By-Value Return
// =============================================================================

struct SimpleMetadata {
    int id = 0;
    double score = 0.0;
    
    bool operator==(const SimpleMetadata& other) const {
        return id == other.id && score == other.score;
    }
};

struct GraphWithByValueReturn {
    int graph_id;
    double graph_score;
    std::vector<std::vector<int>> data;
    
    // Explicit constructor to ensure initialization
    GraphWithByValueReturn() : graph_id(42), graph_score(3.14), data() {
        // Debug: verify constructor is called
        if (graph_id != 42) {
            throw std::runtime_error("Constructor failed to initialize graph_id");
        }
    }
    
    // Returns by value
    SimpleMetadata graph_value() const {
        return SimpleMetadata{graph_id, graph_score};
    }
};

TEST_CASE("graph_value - by-value return from member", "[graph_value][member][by_value]") {
    GraphWithByValueReturn g;
    
    // Debug: Check after construction
    REQUIRE(g.graph_id == 42);
    REQUIRE(g.graph_score == 3.14);
    
    SimpleMetadata meta = graph_value(g);
    REQUIRE(meta.id == 42);
    REQUIRE(meta.score == 3.14);
    
    // Verify it's returned by value (prvalue)
    static_assert(std::is_same_v<decltype(graph_value(g)), SimpleMetadata>);
}

TEST_CASE("graph_value - by-value allows independent modification", "[graph_value][member][by_value]") {
    GraphWithByValueReturn g;
    
    [[maybe_unused]] auto meta = graph_value(g);
    meta.id = 999;  // Modify the copy
    
    // Original unchanged
    REQUIRE(g.graph_id == 42);
    REQUIRE(graph_value(g).id == 42);
}

// =============================================================================
// Test with ADL (Medium Priority)
// =============================================================================

namespace test_adl {
    struct CustomGraph {
        std::vector<std::vector<int>> adjacency;
        std::string graph_name = "ADL_Graph";
    };
    
    // ADL function
    std::string& graph_value(CustomGraph& g) {
        return g.graph_name;
    }
    
    const std::string& graph_value(const CustomGraph& g) {
        return g.graph_name;
    }
}

TEST_CASE("graph_value - ADL function", "[graph_value][adl]") {
    test_adl::CustomGraph g;
    
    auto& name = graph_value(g);
    REQUIRE(name == "ADL_Graph");
}

TEST_CASE("graph_value - ADL with modification", "[graph_value][adl][modify]") {
    test_adl::CustomGraph g;
    
    graph_value(g) = "Modified_ADL";
    REQUIRE(g.graph_name == "Modified_ADL");
}

TEST_CASE("graph_value - ADL const correctness", "[graph_value][adl][const]") {
    test_adl::CustomGraph g;
    const test_adl::CustomGraph& g_const = g;
    
    // Non-const: mutable reference
    static_assert(std::is_same_v<decltype(graph_value(g)), std::string&>);
    
    // Const: const reference
    static_assert(std::is_same_v<decltype(graph_value(g_const)), const std::string&>);
}

// =============================================================================
// Test Member vs ADL Priority
// =============================================================================

namespace test_priority {
    struct GraphWithBoth {
        std::string member_value = "member";
        std::string adl_value = "adl";
        
        std::string& graph_value() {
            return member_value;
        }
    };
    
    // This ADL function should NOT be called (member has priority)
    std::string& graph_value(GraphWithBoth& g) {
        return g.adl_value;
    }
}

TEST_CASE("graph_value - member function has priority over ADL", "[graph_value][priority]") {
    test_priority::GraphWithBoth g;
    
    // Should call member function, not ADL
    auto& value = graph_value(g);
    REQUIRE(value == "member");
    
    value = "modified_member";
    REQUIRE(g.member_value == "modified_member");
    REQUIRE(g.adl_value == "adl");  // Unchanged
}

// =============================================================================
// Test with Different Value Types
// =============================================================================

struct IntValueGraph {
    std::vector<std::vector<int>> data;
    int value = 100;
    
    int& graph_value() { return value; }
    const int& graph_value() const { return value; }
};

TEST_CASE("graph_value - int value type", "[graph_value][types]") {
    IntValueGraph g;
    
    REQUIRE(graph_value(g) == 100);
    graph_value(g) = 200;
    REQUIRE(g.value == 200);
}

struct StringValueGraph {
    std::vector<std::vector<int>> data;
    std::string value = "hello";
    
    std::string& graph_value() { return value; }
    const std::string& graph_value() const { return value; }
};

TEST_CASE("graph_value - string value type", "[graph_value][types]") {
    StringValueGraph g;
    
    REQUIRE(graph_value(g) == "hello");
    graph_value(g) = "world";
    REQUIRE(g.value == "world");
}

struct VectorValueGraph {
    std::vector<std::vector<int>> data;
    std::vector<double> weights = {1.0, 2.0, 3.0};
    
    std::vector<double>& graph_value() { return weights; }
    const std::vector<double>& graph_value() const { return weights; }
};

TEST_CASE("graph_value - vector value type", "[graph_value][types]") {
    VectorValueGraph g;
    
    auto& weights = graph_value(g);
    REQUIRE(weights.size() == 3);
    REQUIRE(weights[0] == 1.0);
    
    weights.push_back(4.0);
    REQUIRE(g.weights.size() == 4);
}

struct MapValueGraph {
    std::vector<std::vector<int>> data;
    std::map<std::string, int> properties = {{"nodes", 10}, {"edges", 15}};
    
    std::map<std::string, int>& graph_value() { return properties; }
    const std::map<std::string, int>& graph_value() const { return properties; }
};

TEST_CASE("graph_value - map value type", "[graph_value][types]") {
    MapValueGraph g;
    
    auto& props = graph_value(g);
    REQUIRE(props["nodes"] == 10);
    REQUIRE(props["edges"] == 15);
    
    props["components"] = 2;
    REQUIRE(g.properties.size() == 3);
}

// =============================================================================
// Test with Complex Nested Structures
// =============================================================================

struct Statistics {
    size_t node_count = 0;
    size_t edge_count = 0;
    double density = 0.0;
};

struct GraphProperties {
    std::string name;
    Statistics stats;
    std::vector<std::string> tags;
};

struct ComplexGraph {
    std::vector<std::vector<int>> adjacency;
    GraphProperties props{"ComplexGraph", {100, 500, 0.05}, {"directed", "weighted"}};
    
    GraphProperties& graph_value() { return props; }
    const GraphProperties& graph_value() const { return props; }
};

TEST_CASE("graph_value - complex nested structure", "[graph_value][complex]") {
    ComplexGraph g;
    
    auto& props = graph_value(g);
    REQUIRE(props.name == "ComplexGraph");
    REQUIRE(props.stats.node_count == 100);
    REQUIRE(props.stats.edge_count == 500);
    REQUIRE(props.stats.density == 0.05);
    REQUIRE(props.tags.size() == 2);
    REQUIRE(props.tags[0] == "directed");
}

TEST_CASE("graph_value - modify nested structure", "[graph_value][complex][modify]") {
    ComplexGraph g;
    
    auto& props = graph_value(g);
    props.stats.node_count = 200;
    props.tags.push_back("sparse");
    
    REQUIRE(g.props.stats.node_count == 200);
    REQUIRE(g.props.tags.size() == 3);
}

// =============================================================================
// Test Different Container Types
// =============================================================================

struct DequeGraph {
    std::deque<std::deque<int>> data;
    std::string name = "DequeGraph";
    
    std::string& graph_value() { return name; }
};

TEST_CASE("graph_value - deque-based graph", "[graph_value][containers]") {
    DequeGraph g;
    g.data = {{1}, {0}};
    
    REQUIRE(graph_value(g) == "DequeGraph");
    graph_value(g) = "ModifiedDeque";
    REQUIRE(g.name == "ModifiedDeque");
}

struct MapGraph {
    std::map<int, std::vector<int>> adjacency;
    int graph_id = 42;
    
    int& graph_value() { return graph_id; }
};

TEST_CASE("graph_value - map-based graph", "[graph_value][containers]") {
    MapGraph g;
    g.adjacency[0] = {1, 2};
    g.adjacency[1] = {0};
    
    REQUIRE(graph_value(g) == 42);
    graph_value(g) = 100;
    REQUIRE(g.graph_id == 100);
}

// =============================================================================
// Test Use Cases / Patterns
// =============================================================================

struct WeightedGraph {
    std::vector<std::vector<std::pair<int, double>>> adjacency;
    double global_weight_multiplier = 1.0;
    
    double& graph_value() { return global_weight_multiplier; }
    const double& graph_value() const { return global_weight_multiplier; }
};

TEST_CASE("graph_value - weight scaling pattern", "[graph_value][patterns]") {
    WeightedGraph g;
    g.adjacency = {{{1, 10.0}, {2, 20.0}}, {{0, 15.0}}};
    
    // Get global weight multiplier
    auto& multiplier = graph_value(g);
    REQUIRE(multiplier == 1.0);
    
    // Scale all weights by changing multiplier
    multiplier = 2.0;
    
    // In actual use, algorithms would apply this multiplier
    REQUIRE(graph_value(g) == 2.0);
}

struct TimestampedGraph {
    std::vector<std::vector<int>> adjacency;
    
    struct Metadata {
        std::string created_at;
        std::string modified_at;
        int version;
    };
    
    Metadata meta{"2025-01-01", "2025-01-01", 1};
    
    Metadata& graph_value() { return meta; }
    const Metadata& graph_value() const { return meta; }
};

TEST_CASE("graph_value - version tracking pattern", "[graph_value][patterns]") {
    TimestampedGraph g;
    
    auto& meta = graph_value(g);
    REQUIRE(meta.version == 1);
    
    // Update metadata when graph changes
    meta.modified_at = "2025-11-01";
    meta.version = 2;
    
    REQUIRE(g.meta.version == 2);
}

struct NamedGraph {
    std::vector<std::vector<int>> adjacency;
    std::string name;
    
    NamedGraph(std::string n) : name(std::move(n)) {}
    
    const std::string& graph_value() const { return name; }
};

TEST_CASE("graph_value - graph identification pattern", "[graph_value][patterns]") {
    NamedGraph g1("NetworkA");
    NamedGraph g2("NetworkB");
    
    REQUIRE(graph_value(g1) == "NetworkA");
    REQUIRE(graph_value(g2) == "NetworkB");
}

// =============================================================================
// Test Const Overload Selection
// =============================================================================

struct OverloadTestGraph {
    std::vector<std::vector<int>> data;
    mutable int call_count = 0;
    std::string value = "test";
    
    std::string& graph_value() {
        call_count++;
        return value;
    }
    
    const std::string& graph_value() const {
        return value;
    }
};

TEST_CASE("graph_value - correct overload selection", "[graph_value][const][overload]") {
    OverloadTestGraph g;
    
    // Non-const: should call non-const overload
    [[maybe_unused]] auto& val1 = graph_value(g);
    REQUIRE(g.call_count == 1);
    
    // Const: should call const overload (doesn't increment call_count)
    const OverloadTestGraph& g_const = g;
    [[maybe_unused]] auto& val2 = graph_value(g_const);
    REQUIRE(g.call_count == 1);  // Unchanged
}

// =============================================================================
// Test Edge Cases
// =============================================================================

struct EmptyValueGraph {
    std::vector<std::vector<int>> data;
    struct Empty {};
    Empty empty;
    
    Empty& graph_value() { return empty; }
};

TEST_CASE("graph_value - empty struct value", "[graph_value][edge_cases]") {
    EmptyValueGraph g;
    
    // Should compile and work even with empty struct
    [[maybe_unused]] auto& val = graph_value(g);
}

struct BoolValueGraph {
    std::vector<std::vector<int>> data;
    bool is_directed = true;
    
    bool& graph_value() { return is_directed; }
    bool graph_value() const { return is_directed; }  // Return by value for const
};

TEST_CASE("graph_value - bool value type", "[graph_value][edge_cases]") {
    BoolValueGraph g;
    
    REQUIRE(graph_value(g) == true);
    graph_value(g) = false;
    REQUIRE(g.is_directed == false);
    
    // Const version returns by value
    const BoolValueGraph& g_const = g;
    static_assert(std::is_same_v<decltype(graph_value(g_const)), bool>);
}

// =============================================================================
// Test with Multiple Graphs
// =============================================================================

TEST_CASE("graph_value - multiple independent graphs", "[graph_value][multi]") {
    GraphWithMemberValue g1, g2, g3;
    
    graph_value(g1).name = "Graph1";
    graph_value(g2).name = "Graph2";
    graph_value(g3).name = "Graph3";
    
    REQUIRE(g1.metadata.name == "Graph1");
    REQUIRE(g2.metadata.name == "Graph2");
    REQUIRE(g3.metadata.name == "Graph3");
}

// =============================================================================
// Test Noexcept Specification
// =============================================================================

struct NoexceptGraph {
    std::vector<std::vector<int>> data;
    int value = 0;
    
    int& graph_value() noexcept { return value; }
    const int& graph_value() const noexcept { return value; }
};

TEST_CASE("graph_value - noexcept propagation", "[graph_value][noexcept]") {
    NoexceptGraph g;
    
    // Should be noexcept when underlying function is noexcept
    static_assert(noexcept(graph_value(g)));
    
    REQUIRE(graph_value(g) == 0);
}

// =============================================================================
// Test Large Value Types
// =============================================================================

struct LargeMetadata {
    std::string name;
    std::vector<double> weights;
    std::map<std::string, std::string> properties;
    Statistics stats;
};

struct LargeValueGraph {
    std::vector<std::vector<int>> data;
    LargeMetadata meta{
        "LargeGraph",
        {1.0, 2.0, 3.0, 4.0, 5.0},
        {{"type", "social"}, {"category", "network"}},
        {1000, 5000, 0.005}
    };
    
    LargeMetadata& graph_value() { return meta; }
    const LargeMetadata& graph_value() const { return meta; }
};

TEST_CASE("graph_value - large metadata structure", "[graph_value][large]") {
    LargeValueGraph g;
    
    auto& meta = graph_value(g);
    REQUIRE(meta.name == "LargeGraph");
    REQUIRE(meta.weights.size() == 5);
    REQUIRE(meta.properties.size() == 2);
    REQUIRE(meta.stats.node_count == 1000);
    
    // Modify parts
    meta.weights.push_back(6.0);
    meta.properties["algorithm"] = "dijkstra";
    
    REQUIRE(g.meta.weights.size() == 6);
    REQUIRE(g.meta.properties.size() == 3);
}
