#include <catch2/catch_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <variant>
#include <vector>
#include <functional>

using namespace graph;
using namespace graph::container;

// Type aliases for different graph types
using vov_uint = dynamic_graph<void, void, void, uint64_t, false,
                                vov_graph_traits<void, void, void, uint64_t, false>>;

using mos_string = dynamic_graph<void, void, void, std::string, false,
                                  mos_graph_traits<void, void, void, std::string, false>>;

using dofl_int = dynamic_graph<void, void, void, int, false,
                                dofl_graph_traits<void, void, void, int, false>>;

using dov_uint = dynamic_graph<void, void, void, uint64_t, false,
                                dov_graph_traits<void, void, void, uint64_t, false>>;

// Type alias for graph containers
using copyable_edge_uint = copyable_edge_t<uint64_t, void>;
using copyable_edge_string = copyable_edge_t<std::string, void>;
using copyable_edge_int = copyable_edge_t<int, void>;
using copyable_vertex_uint = copyable_vertex_t<uint64_t, void>;
using copyable_vertex_string = copyable_vertex_t<std::string, void>;
using copyable_vertex_int = copyable_vertex_t<int, void>;

// Helper for overload pattern
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// Helper: Count vertices using generic CPOs
template<typename G>
size_t count_vertices(const G& g) {
    return std::ranges::distance(vertices(g));
}

// Helper: Count edges using generic CPOs
template<typename G>
size_t count_edges(const G& g) {
    size_t count = 0;
    for (auto&& u : vertices(g)) {
        count += std::ranges::distance(edges(g, u));
    }
    return count;
}

TEST_CASE("Store graphs in std::variant", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string, dofl_int>;
    
    // Create three different graph types
    std::vector<copyable_edge_uint> edges1 = {{0, 1}, {1, 2}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}, {2}};
    std::vector<uint64_t> partitions1;
    vov_uint g1(edges1, vertices1, identity{}, identity{}, partitions1);
    
    std::vector<copyable_edge_string> edges2 = {{"A", "B"}, {"B", "C"}};
    std::vector<copyable_vertex_string> vertices2 = {{"A"}, {"B"}, {"C"}};
    std::vector<std::string> partitions2;
    mos_string g2(edges2, vertices2, identity{}, identity{}, partitions2);
    
    std::vector<copyable_edge_int> edges3 = {{0, 1}, {1, 2}};
    std::vector<copyable_vertex_int> vertices3 = {{0}, {1}, {2}};
    std::vector<int> partitions3;
    dofl_int g3(edges3, vertices3, identity{}, identity{}, partitions3);
    
    // Store in variant
    std::vector<graph_variant> graphs;
    graphs.push_back(std::move(g1));
    graphs.push_back(std::move(g2));
    graphs.push_back(std::move(g3));
    
    REQUIRE(graphs.size() == 3);
    REQUIRE(graphs[0].index() == 0); // vov_uint
    REQUIRE(graphs[1].index() == 1); // mos_string
    REQUIRE(graphs[2].index() == 2); // dofl_int
}

TEST_CASE("Visit variant graphs with std::visit", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string, dofl_int>;
    
    std::vector<copyable_edge_uint> edges1 = {{0, 1}, {1, 2}, {2, 0}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}, {2}};
    std::vector<uint64_t> partitions1;
    vov_uint g1(edges1, vertices1, identity{}, identity{}, partitions1);
    
    graph_variant var = std::move(g1);
    
    // Visit with generic lambda
    auto vertex_count = std::visit([](auto&& g) {
        return count_vertices(g);
    }, var);
    
    auto edge_count = std::visit([](auto&& g) {
        return count_edges(g);
    }, var);
    
    REQUIRE(vertex_count == 3);
    REQUIRE(edge_count == 3);
}

TEST_CASE("Generic operations on variant collection", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_uint> edges1 = {{0, 1}, {1, 2}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}, {2}};
    std::vector<uint64_t> partitions1;
    vov_uint g1(edges1, vertices1, identity{}, identity{}, partitions1);
    
    std::vector<copyable_edge_string> edges2 = {{"X", "Y"}};
    std::vector<copyable_vertex_string> vertices2 = {{"X"}, {"Y"}};
    std::vector<std::string> partitions2;
    mos_string g2(edges2, vertices2, identity{}, identity{}, partitions2);
    
    std::vector<graph_variant> graphs;
    graphs.push_back(std::move(g1));
    graphs.push_back(std::move(g2));
    
    // Count total edges across all graphs
    size_t total_edges = 0;
    for (auto&& var : graphs) {
        total_edges += std::visit([](auto&& g) {
            return count_edges(g);
        }, var);
    }
    
    REQUIRE(total_edges == 3);
}

TEST_CASE("Check which graph type is stored in variant", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string, dofl_int>;
    
    std::vector<copyable_edge_string> edges = {{"A", "B"}};
    std::vector<copyable_vertex_string> vertices = {{"A"}, {"B"}};
    std::vector<std::string> partitions;
    mos_string g(edges, vertices, identity{}, identity{}, partitions);
    
    graph_variant var = std::move(g);
    
    REQUIRE(std::holds_alternative<mos_string>(var));
    REQUIRE_FALSE(std::holds_alternative<vov_uint>(var));
    REQUIRE_FALSE(std::holds_alternative<dofl_int>(var));
}

TEST_CASE("Get specific graph type from variant", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_uint> edges = {{0, 1}, {1, 2}, {2, 3}};
    std::vector<copyable_vertex_uint> vertices = {{0}, {1}, {2}, {3}};
    std::vector<uint64_t> partitions;
    vov_uint g(edges, vertices, identity{}, identity{}, partitions);
    
    graph_variant var = std::move(g);
    
    // Get the graph
    auto& retrieved = std::get<vov_uint>(var);
    REQUIRE(count_vertices(retrieved) == 4);
    REQUIRE(count_edges(retrieved) == 3);
}

TEST_CASE("Visit with overloaded lambdas", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_uint> edges1 = {{0, 1}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}};
    std::vector<uint64_t> partitions1;
    vov_uint g1(edges1, vertices1, identity{}, identity{}, partitions1);
    
    std::vector<copyable_edge_string> edges2 = {{"A", "B"}};
    std::vector<copyable_vertex_string> vertices2 = {{"A"}, {"B"}};
    std::vector<std::string> partitions2;
    mos_string g2(edges2, vertices2, identity{}, identity{}, partitions2);
    
    graph_variant var1 = std::move(g1);
    graph_variant var2 = std::move(g2);
    
    // Visit with type-specific handlers
    auto process = overloaded{
        [](vov_uint& g) { return count_vertices(g) * 10; },
        [](mos_string& g) { return count_vertices(g) * 100; }
    };
    
    auto result1 = std::visit(process, var1);
    auto result2 = std::visit(process, var2);
    
    REQUIRE(result1 == 20);  // 2 vertices * 10
    REQUIRE(result2 == 200); // 2 vertices * 100
}

TEST_CASE("Variant with empty graphs", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_uint> empty_edges;
    std::vector<copyable_vertex_uint> empty_vertices;
    std::vector<uint64_t> partitions;
    vov_uint empty_g(empty_edges, empty_vertices, identity{}, identity{}, partitions);
    
    graph_variant var = std::move(empty_g);
    
    auto vertex_count = std::visit([](auto&& g) {
        return count_vertices(g);
    }, var);
    
    auto edge_count = std::visit([](auto&& g) {
        return count_edges(g);
    }, var);
    
    REQUIRE(vertex_count == 0);
    REQUIRE(edge_count == 0);
}

TEST_CASE("Switch between graph types in variant", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_uint> edges1 = {{0, 1}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}};
    std::vector<uint64_t> partitions1;
    vov_uint g1(edges1, vertices1, identity{}, identity{}, partitions1);
    
    std::vector<copyable_edge_string> edges2 = {{"X", "Y"}, {"Y", "Z"}};
    std::vector<copyable_vertex_string> vertices2 = {{"X"}, {"Y"}, {"Z"}};
    std::vector<std::string> partitions2;
    mos_string g2(edges2, vertices2, identity{}, identity{}, partitions2);
    
    graph_variant var = std::move(g1);
    REQUIRE(std::holds_alternative<vov_uint>(var));
    
    // Replace with different type
    var = std::move(g2);
    REQUIRE(std::holds_alternative<mos_string>(var));
    
    auto edge_count = std::visit([](auto&& g) {
        return count_edges(g);
    }, var);
    
    REQUIRE(edge_count == 2);
}

TEST_CASE("Variant graphs with different sizes", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, dofl_int>;
    
    // Small graph
    std::vector<copyable_edge_uint> edges1 = {{0, 1}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}};
    std::vector<uint64_t> partitions1;
    vov_uint small_g(edges1, vertices1, identity{}, identity{}, partitions1);
    
    // Large graph
    std::vector<copyable_edge_int> edges2;
    std::vector<copyable_vertex_int> vertices2;
    for (int i = 0; i < 50; ++i) {
        vertices2.push_back({i});
        if (i > 0) {
            edges2.push_back({i - 1, i});
        }
    }
    std::vector<int> partitions2;
    dofl_int large_g(edges2, vertices2, identity{}, identity{}, partitions2);
    
    std::vector<graph_variant> graphs;
    graphs.push_back(std::move(small_g));
    graphs.push_back(std::move(large_g));
    
    auto count1 = std::visit([](auto&& g) { return count_vertices(g); }, graphs[0]);
    auto count2 = std::visit([](auto&& g) { return count_vertices(g); }, graphs[1]);
    
    REQUIRE(count1 == 2);
    REQUIRE(count2 == 50);
}

TEST_CASE("Filter variant graphs by property", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string, dofl_int>;
    
    std::vector<graph_variant> graphs;
    
    // Graph 1: 5 vertices
    std::vector<copyable_edge_uint> edges1;
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}, {2}, {3}, {4}};
    std::vector<uint64_t> partitions1;
    graphs.emplace_back(vov_uint(edges1, vertices1, identity{}, identity{}, partitions1));
    
    // Graph 2: 2 vertices
    std::vector<copyable_edge_string> edges2;
    std::vector<copyable_vertex_string> vertices2 = {{"A"}, {"B"}};
    std::vector<std::string> partitions2;
    graphs.emplace_back(mos_string(edges2, vertices2, identity{}, identity{}, partitions2));
    
    // Graph 3: 10 vertices
    std::vector<copyable_edge_int> edges3;
    std::vector<copyable_vertex_int> vertices3;
    for (int i = 0; i < 10; ++i) {
        vertices3.push_back({i});
    }
    std::vector<int> partitions3;
    graphs.emplace_back(dofl_int(edges3, vertices3, identity{}, identity{}, partitions3));
    
    // Count graphs with more than 3 vertices
    size_t large_graphs = 0;
    for (auto&& var : graphs) {
        auto vcount = std::visit([](auto&& g) { return count_vertices(g); }, var);
        if (vcount > 3) {
            ++large_graphs;
        }
    }
    
    REQUIRE(large_graphs == 2);
}

TEST_CASE("Aggregate statistics across variant graphs", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<graph_variant> graphs;
    
    // Create multiple graphs
    for (int i = 0; i < 5; ++i) {
        std::vector<copyable_edge_uint> edges;
        std::vector<copyable_vertex_uint> vertices;
        for (uint64_t j = 0; j < static_cast<uint64_t>(i + 1); ++j) {
            vertices.push_back({j});
            if (j > 0) {
                edges.push_back({j - 1, j});
            }
        }
        std::vector<uint64_t> partitions;
        graphs.emplace_back(vov_uint(edges, vertices, identity{}, identity{}, partitions));
    }
    
    // Calculate total vertices and edges
    size_t total_vertices = 0;
    size_t total_edges = 0;
    
    for (auto&& var : graphs) {
        std::visit([&](auto&& g) {
            total_vertices += count_vertices(g);
            total_edges += count_edges(g);
        }, var);
    }
    
    REQUIRE(total_vertices == 15); // 1+2+3+4+5
    REQUIRE(total_edges == 10);    // 0+1+2+3+4
}

TEST_CASE("Transform variant graphs", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, dov_uint>;
    
    std::vector<copyable_edge_uint> edge_list = {{0, 1}, {1, 2}, {2, 3}};
    std::vector<copyable_vertex_uint> vertex_list = {{0}, {1}, {2}, {3}};
    std::vector<uint64_t> partitions;
    vov_uint g(edge_list, vertex_list, identity{}, identity{}, partitions);
    
    graph_variant var = std::move(g);
    
    // Apply transformation (count edges per vertex)
    std::vector<size_t> degrees;
    std::visit([&](auto&& graph) {
        for (auto&& u : vertices(graph)) {
            degrees.push_back(std::ranges::distance(edges(graph, u)));
        }
    }, var);
    
    REQUIRE(degrees.size() == 4);
    REQUIRE(degrees[0] == 1); // vertex 0 -> 1
    REQUIRE(degrees[1] == 1); // vertex 1 -> 2
    REQUIRE(degrees[2] == 1); // vertex 2 -> 3
    REQUIRE(degrees[3] == 0); // vertex 3 (no outgoing edges)
}

TEST_CASE("Conditional processing based on graph type", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_string> edges = {{"node1", "node2"}};
    std::vector<copyable_vertex_string> vertices = {{"node1"}, {"node2"}};
    std::vector<std::string> partitions;
    mos_string g(edges, vertices, identity{}, identity{}, partitions);
    
    graph_variant var = std::move(g);
    
    // Process differently based on type
    bool is_associative = std::visit([](auto&& g) {
        using G = std::decay_t<decltype(g)>;
        if constexpr (std::is_same_v<G, mos_string>) {
            return true;
        } else {
            return false;
        }
    }, var);
    
    REQUIRE(is_associative);
}

TEST_CASE("Exception safety with variant graphs", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string>;
    
    std::vector<copyable_edge_uint> edges = {{0, 1}};
    std::vector<copyable_vertex_uint> vertices = {{0}, {1}};
    std::vector<uint64_t> partitions;
    vov_uint g(edges, vertices, identity{}, identity{}, partitions);
    
    graph_variant var = std::move(g);
    
    // Visit should not throw with valid graphs
    REQUIRE_NOTHROW(std::visit([](auto&& g) {
        return count_vertices(g);
    }, var));
    
    // Verify variant is still valid after visit
    REQUIRE(var.index() == 0);
    REQUIRE(std::holds_alternative<vov_uint>(var));
}

TEST_CASE("Vector of heterogeneous graphs with complex operations", "[6.4.3]") {
    using graph_variant = std::variant<vov_uint, mos_string, dofl_int>;
    
    std::vector<graph_variant> graphs;
    
    // Create cycle graph with vov
    std::vector<copyable_edge_uint> edges1 = {{0, 1}, {1, 2}, {2, 0}};
    std::vector<copyable_vertex_uint> vertices1 = {{0}, {1}, {2}};
    std::vector<uint64_t> partitions1;
    graphs.emplace_back(vov_uint(edges1, vertices1, identity{}, identity{}, partitions1));
    
    // Create star graph with mos
    std::vector<copyable_edge_string> edges2 = {
        {"center", "leaf1"},
        {"center", "leaf2"},
        {"center", "leaf3"}
    };
    std::vector<copyable_vertex_string> vertices2 = {
        {"center"}, {"leaf1"}, {"leaf2"}, {"leaf3"}
    };
    std::vector<std::string> partitions2;
    graphs.emplace_back(mos_string(edges2, vertices2, identity{}, identity{}, partitions2));
    
    // Create path graph with dofl
    std::vector<copyable_edge_int> edges3 = {{0, 1}, {1, 2}, {2, 3}};
    std::vector<copyable_vertex_int> vertices3 = {{0}, {1}, {2}, {3}};
    std::vector<int> partitions3;
    graphs.emplace_back(dofl_int(edges3, vertices3, identity{}, identity{}, partitions3));
    
    // Find graph with most edges
    size_t max_edges = 0;
    size_t max_index = 0;
    
    for (size_t i = 0; i < graphs.size(); ++i) {
        auto edge_count = std::visit([](auto&& g) {
            return count_edges(g);
        }, graphs[i]);
        
        if (edge_count > max_edges) {
            max_edges = edge_count;
            max_index = i;
        }
    }
    
    REQUIRE(max_edges == 3);
    REQUIRE((max_index == 0 || max_index == 1 || max_index == 2)); // All have 3 edges
}
