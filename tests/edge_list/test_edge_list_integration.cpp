#include <catch2/catch_test_macros.hpp>
#include <graph/edge_list/edge_list.hpp>
#include <graph/edge_list/edge_list_descriptor.hpp>
#include <graph/graph_info.hpp>
#include <vector>
#include <tuple>

using namespace graph;

// =============================================================================
// Generic Algorithm Tests - Demonstrates unified CPO interface
// =============================================================================

// Generic algorithm that works with ANY edge range using unified CPOs
template<typename EdgeRange>
    requires edge_list::basic_sourced_edgelist<EdgeRange>
auto count_self_loops(EdgeRange&& edges) {
    int count = 0;
    for (auto&& uv : edges) {
        auto src = graph::source_id(edges, uv);
        auto tgt = graph::target_id(edges, uv);
        if (src == tgt) {
            ++count;
        }
    }
    return count;
}

// Generic algorithm that sums edge values (for edge lists with values)
template<typename EdgeRange>
    requires edge_list::has_edge_value<EdgeRange>
auto sum_edge_values(EdgeRange&& edges) {
    using value_t = edge_list::edge_value_t<EdgeRange>;
    value_t sum = 0;
    for (auto&& uv : edges) {
        sum += graph::edge_value(edges, uv);
    }
    return sum;
}

TEST_CASE("Generic algorithm works with std::pair", "[integration][algorithm]") {
    std::vector<std::pair<int, int>> pairs{{1, 2}, {3, 3}, {4, 4}, {5, 6}};
    REQUIRE(count_self_loops(pairs) == 2);
}

TEST_CASE("Generic algorithm works with std::tuple (2-element)", "[integration][algorithm]") {
    std::vector<std::tuple<int, int>> tuples{{1, 1}, {2, 3}, {4, 4}};
    REQUIRE(count_self_loops(tuples) == 2);
}

TEST_CASE("Generic algorithm works with std::tuple (3-element)", "[integration][algorithm]") {
    std::vector<std::tuple<int, int, double>> tuples{
        {1, 2, 1.5},
        {3, 3, 2.5},
        {4, 5, 3.5}
    };
    REQUIRE(count_self_loops(tuples) == 1);
}

TEST_CASE("Generic algorithm works with edge_info (no value)", "[integration][algorithm]") {
    using EI = edge_info<int, true, void, void>;
    std::vector<EI> infos{{1, 2}, {5, 5}, {7, 8}};
    REQUIRE(count_self_loops(infos) == 1);
}

TEST_CASE("Generic algorithm works with edge_info (with value)", "[integration][algorithm]") {
    using EI = edge_info<int, true, void, double>;
    std::vector<EI> infos{
        {1, 2, 10.0},
        {3, 3, 20.0},
        {4, 4, 30.0}
    };
    REQUIRE(count_self_loops(infos) == 2);
}

TEST_CASE("Generic algorithm works with edge_list::edge_descriptor (no value)", "[integration][algorithm]") {
    int s1 = 1, t1 = 1;
    int s2 = 2, t2 = 3;
    int s3 = 4, t3 = 4;
    
    edge_list::edge_descriptor<int, void> e1(s1, t1);
    edge_list::edge_descriptor<int, void> e2(s2, t2);
    edge_list::edge_descriptor<int, void> e3(s3, t3);
    
    std::vector<edge_list::edge_descriptor<int, void>> descs{e1, e2, e3};
    REQUIRE(count_self_loops(descs) == 2);
}

TEST_CASE("Generic algorithm works with edge_list::edge_descriptor (with value)", "[integration][algorithm]") {
    int s1 = 5, t1 = 5;
    int s2 = 6, t2 = 7;
    double v1 = 1.1, v2 = 2.2;
    
    edge_list::edge_descriptor<int, double> e1(s1, t1, v1);
    edge_list::edge_descriptor<int, double> e2(s2, t2, v2);
    
    std::vector<edge_list::edge_descriptor<int, double>> descs{e1, e2};
    REQUIRE(count_self_loops(descs) == 1);
}

// =============================================================================
// Edge Value Algorithm Tests
// =============================================================================

TEST_CASE("sum_edge_values works with 3-tuples", "[integration][edge_value]") {
    std::vector<std::tuple<int, int, double>> tuples{
        {1, 2, 1.5},
        {2, 3, 2.5},
        {3, 4, 3.0}
    };
    REQUIRE(sum_edge_values(tuples) == 7.0);
}

TEST_CASE("sum_edge_values works with edge_info", "[integration][edge_value]") {
    using EI = edge_info<int, true, void, int>;
    std::vector<EI> infos{
        {1, 2, 10},
        {2, 3, 20},
        {3, 4, 30}
    };
    REQUIRE(sum_edge_values(infos) == 60);
}

TEST_CASE("sum_edge_values works with edge_list::edge_descriptor", "[integration][edge_value]") {
    int s1 = 1, t1 = 2, s2 = 2, t2 = 3;
    double v1 = 5.5, v2 = 4.5;
    
    edge_list::edge_descriptor<int, double> e1(s1, t1, v1);
    edge_list::edge_descriptor<int, double> e2(s2, t2, v2);
    
    std::vector<edge_list::edge_descriptor<int, double>> descs{e1, e2};
    REQUIRE(sum_edge_values(descs) == 10.0);
}

// =============================================================================
// Mixed Edge Types in Same Compilation Unit
// =============================================================================

TEST_CASE("Different edge types work together in same compilation unit", "[integration][mixed]") {
    // Pairs
    std::vector<std::pair<int, int>> pairs{{1, 1}, {2, 3}};
    
    // Tuples
    std::vector<std::tuple<int, int, double>> tuples{
        {4, 4, 1.0},
        {5, 6, 2.0}
    };
    
    // edge_info
    using EI = edge_info<int, true, void, void>;
    std::vector<EI> infos{{7, 8}, {9, 9}};
    
    // edge_descriptors
    int s1 = 10, t1 = 10;
    edge_list::edge_descriptor<int, void> ed(s1, t1);
    std::vector<edge_list::edge_descriptor<int, void>> descs{ed};
    
    // All work with the same algorithm
    REQUIRE(count_self_loops(pairs) == 1);
    REQUIRE(count_self_loops(tuples) == 1);
    REQUIRE(count_self_loops(infos) == 1);
    REQUIRE(count_self_loops(descs) == 1);
    
    // Total self-loops
    REQUIRE(count_self_loops(pairs) + count_self_loops(tuples) + 
            count_self_loops(infos) + count_self_loops(descs) == 4);
}

// =============================================================================
// String Vertex IDs (Non-Integral)
// =============================================================================

TEST_CASE("Generic algorithm works with string vertex IDs", "[integration][strings]") {
    std::vector<std::pair<std::string, std::string>> edges{
        {"Alice", "Bob"},
        {"Bob", "Bob"},
        {"Charlie", "Dave"},
        {"Eve", "Eve"}
    };
    
    REQUIRE(count_self_loops(edges) == 2);
}

TEST_CASE("Type aliases work correctly with string vertex IDs", "[integration][strings]") {
    using edge_list_type = std::vector<std::pair<std::string, std::string>>;
    
    using vid = edge_list::vertex_id_t<edge_list_type>;
    using edge = edge_list::edge_t<edge_list_type>;
    
    STATIC_REQUIRE(std::is_same_v<vid, std::string>);
    STATIC_REQUIRE(std::is_same_v<edge, std::pair<std::string, std::string>>);
}

// =============================================================================
// Concept Satisfaction Verification
// =============================================================================

TEST_CASE("All edge types satisfy basic_sourced_edgelist", "[integration][concepts]") {
    STATIC_REQUIRE(edge_list::basic_sourced_edgelist<std::vector<std::pair<int, int>>>);
    STATIC_REQUIRE(edge_list::basic_sourced_edgelist<std::vector<std::tuple<int, int>>>);
    STATIC_REQUIRE(edge_list::basic_sourced_edgelist<std::vector<std::tuple<int, int, double>>>);
    STATIC_REQUIRE(edge_list::basic_sourced_edgelist<std::vector<edge_info<int, true, void, void>>>);
    STATIC_REQUIRE(edge_list::basic_sourced_edgelist<std::vector<edge_list::edge_descriptor<int, void>>>);
}

TEST_CASE("Integral types satisfy basic_sourced_index_edgelist", "[integration][concepts]") {
    STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<std::vector<std::pair<int, int>>>);
    STATIC_REQUIRE(!edge_list::basic_sourced_index_edgelist<std::vector<std::pair<std::string, std::string>>>);
}

TEST_CASE("Valued edge types satisfy has_edge_value", "[integration][concepts]") {
    STATIC_REQUIRE(!edge_list::has_edge_value<std::vector<std::pair<int, int>>>);
    STATIC_REQUIRE(!edge_list::has_edge_value<std::vector<std::tuple<int, int>>>);
    STATIC_REQUIRE(edge_list::has_edge_value<std::vector<std::tuple<int, int, double>>>);
    STATIC_REQUIRE(!edge_list::has_edge_value<std::vector<edge_info<int, true, void, void>>>);
    STATIC_REQUIRE(edge_list::has_edge_value<std::vector<edge_info<int, true, void, double>>>);
    STATIC_REQUIRE(!edge_list::has_edge_value<std::vector<edge_list::edge_descriptor<int, void>>>);
    STATIC_REQUIRE(edge_list::has_edge_value<std::vector<edge_list::edge_descriptor<int, double>>>);
}
