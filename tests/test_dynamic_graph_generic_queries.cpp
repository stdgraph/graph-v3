/**
 * @file test_dynamic_graph_generic_queries.cpp
 * @brief Generic graph query functions using only CPOs (Phase 6.3.1)
 * 
 * Tests generic graph query functions that work with any graph type using
 * only Customization Point Objects (CPOs). These functions are graph-agnostic
 * and demonstrate the power of the CPO abstraction layer.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/mous_graph_traits.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <ranges>
#include <limits>
#include <numeric>

using namespace graph::container;

//==================================================================================================
// Generic Graph Query Functions (CPO-based)
//==================================================================================================

/**
 * @brief Count the total number of vertices in a graph
 * @tparam G Graph type
 * @param g Graph instance
 * @return Number of vertices
 * 
 * Uses only CPO: vertices()
 */
template<typename G>
size_t count_vertices(const G& g) {
    return std::ranges::distance(vertices(g));
}

/**
 * @brief Count the total number of edges in a graph
 * @tparam G Graph type
 * @param g Graph instance
 * @return Total number of edges
 * 
 * Uses CPOs: vertices(), edges()
 */
template<typename G>
size_t count_edges(const G& g) {
    size_t count = 0;
    for (auto&& v : vertices(g)) {
        count += std::ranges::distance(edges(g, v));
    }
    return count;
}

/**
 * @brief Find the vertex with maximum out-degree
 * @tparam G Graph type
 * @param g Graph instance
 * @return Pair of (vertex_id, degree) or nullopt if graph is empty
 * 
 * Uses CPOs: vertices(), vertex_id(), edges()
 */
template<typename G>
std::optional<std::pair<typename G::vertex_id_type, size_t>> max_degree(const G& g) {
    using VId = typename G::vertex_id_type;
    
    if (std::ranges::empty(vertices(g))) {
        return std::nullopt;
    }
    
    VId max_vid{};
    size_t max_deg = 0;
    bool found = false;
    
    for (auto&& v : vertices(g)) {
        size_t deg = std::ranges::distance(edges(g, v));
        if (!found || deg > max_deg) {
            max_deg = deg;
            max_vid = vertex_id(g, v);
            found = true;
        }
    }
    
    return std::make_pair(max_vid, max_deg);
}

/**
 * @brief Find the vertex with minimum out-degree
 * @tparam G Graph type
 * @param g Graph instance
 * @return Pair of (vertex_id, degree) or nullopt if graph is empty
 * 
 * Uses CPOs: vertices(), vertex_id(), edges()
 */
template<typename G>
std::optional<std::pair<typename G::vertex_id_type, size_t>> min_degree(const G& g) {
    using VId = typename G::vertex_id_type;
    
    if (std::ranges::empty(vertices(g))) {
        return std::nullopt;
    }
    
    VId min_vid{};
    size_t min_deg = std::numeric_limits<size_t>::max();
    bool found = false;
    
    for (auto&& v : vertices(g)) {
        size_t deg = std::ranges::distance(edges(g, v));
        if (!found || deg < min_deg) {
            min_deg = deg;
            min_vid = vertex_id(g, v);
            found = true;
        }
    }
    
    return std::make_pair(min_vid, min_deg);
}

/**
 * @brief Compute the average out-degree of all vertices
 * @tparam G Graph type
 * @param g Graph instance
 * @return Average degree, or 0.0 if graph is empty
 * 
 * Uses CPOs: vertices(), edges()
 */
template<typename G>
double avg_degree(const G& g) {
    size_t vertex_count = 0;
    size_t total_degree = 0;
    
    for (auto&& v : vertices(g)) {
        ++vertex_count;
        total_degree += std::ranges::distance(edges(g, v));
    }
    
    return vertex_count > 0 ? static_cast<double>(total_degree) / vertex_count : 0.0;
}

/**
 * @brief Check if graph has no vertices
 * @tparam G Graph type
 * @param g Graph instance
 * @return true if graph is empty, false otherwise
 * 
 * Uses only CPO: vertices()
 */
template<typename G>
bool is_empty(const G& g) {
    return std::ranges::empty(vertices(g));
}

//==================================================================================================
// Type Aliases for Testing
//==================================================================================================

// Sequential container graphs (integral VId)
using vov_void = dynamic_graph<void, void, void, uint64_t, false,
                                vov_graph_traits<void, void, void, uint64_t, false>>;
using dofl_void = dynamic_graph<void, void, void, uint64_t, false,
                                 dofl_graph_traits<void, void, void, uint64_t, false>>;

// Map-based graphs (string VId)
using mos_void = dynamic_graph<void, void, void, std::string, false,
                                mos_graph_traits<void, void, void, std::string, false>>;
using mous_void = dynamic_graph<void, void, void, std::string, false,
                                 mous_graph_traits<void, void, void, std::string, false>>;

//==================================================================================================
// Phase 6.3.1: Generic Graph Queries Tests
//==================================================================================================

TEST_CASE("count_vertices - empty graph (vov)", "[generic][6.3.1][count_vertices]") {
    vov_void g;
    REQUIRE(count_vertices(g) == 0);
}

TEST_CASE("count_vertices - single vertex (vov)", "[generic][6.3.1][count_vertices]") {
    vov_void g({{0, 1}});
    REQUIRE(count_vertices(g) == 2);
}

TEST_CASE("count_vertices - multiple vertices (vov)", "[generic][6.3.1][count_vertices]") {
    vov_void g({{0, 1}, {1, 2}, {2, 3}, {3, 4}});
    REQUIRE(count_vertices(g) == 5);
}

TEST_CASE("count_vertices - map-based graph (mos)", "[generic][6.3.1][count_vertices]") {
    mos_void g({{"A", "B"}, {"B", "C"}, {"C", "D"}});
    REQUIRE(count_vertices(g) == 4);
}

TEST_CASE("count_vertices - deque-based graph (dofl)", "[generic][6.3.1][count_vertices]") {
    dofl_void g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(count_vertices(g) == 3);
}

TEST_CASE("count_vertices - unordered map graph (mous)", "[generic][6.3.1][count_vertices]") {
    mous_void g({{"X", "Y"}, {"Y", "Z"}});
    REQUIRE(count_vertices(g) == 3);
}

TEST_CASE("count_vertices - graph with isolated vertices (vov)", "[generic][6.3.1][count_vertices]") {
    vov_void g({{0, 1}});
    g.resize_vertices(10);
    REQUIRE(count_vertices(g) == 10);
}

TEST_CASE("count_edges - empty graph (vov)", "[generic][6.3.1][count_edges]") {
    vov_void g;
    REQUIRE(count_edges(g) == 0);
}

TEST_CASE("count_edges - single edge (vov)", "[generic][6.3.1][count_edges]") {
    vov_void g({{0, 1}});
    REQUIRE(count_edges(g) == 1);
}

TEST_CASE("count_edges - multiple edges (vov)", "[generic][6.3.1][count_edges]") {
    vov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 3}});
    REQUIRE(count_edges(g) == 4);
}

TEST_CASE("count_edges - map-based graph (mos)", "[generic][6.3.1][count_edges]") {
    mos_void g({{"A", "B"}, {"B", "C"}, {"C", "A"}});
    REQUIRE(count_edges(g) == 3);
}

TEST_CASE("count_edges - deque-based graph (dofl)", "[generic][6.3.1][count_edges]") {
    dofl_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});
    REQUIRE(count_edges(g) == 4);
}

TEST_CASE("count_edges - unordered map graph (mous)", "[generic][6.3.1][count_edges]") {
    mous_void g({{"X", "Y"}, {"Y", "Z"}, {"Z", "X"}});
    REQUIRE(count_edges(g) == 3);
}

TEST_CASE("count_edges - graph with self-loops (vov)", "[generic][6.3.1][count_edges]") {
    vov_void g({{0, 0}, {0, 1}, {1, 1}, {1, 2}});
    REQUIRE(count_edges(g) == 4);
}

TEST_CASE("count_edges - graph with no edges (vov)", "[generic][6.3.1][count_edges]") {
    vov_void g;
    g.resize_vertices(5);
    REQUIRE(count_edges(g) == 0);
}

TEST_CASE("max_degree - empty graph (vov)", "[generic][6.3.1][max_degree]") {
    vov_void g;
    auto result = max_degree(g);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("max_degree - single vertex no edges (vov)", "[generic][6.3.1][max_degree]") {
    vov_void g;
    g.resize_vertices(1);
    auto result = max_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 0);
    REQUIRE(result->second == 0);
}

TEST_CASE("max_degree - all vertices same degree (vov)", "[generic][6.3.1][max_degree]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto result = max_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->second == 1);
}

TEST_CASE("max_degree - one vertex high degree (vov)", "[generic][6.3.1][max_degree]") {
    vov_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 2}});
    auto result = max_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 0);
    REQUIRE(result->second == 4);
}

TEST_CASE("max_degree - map-based graph (mos)", "[generic][6.3.1][max_degree]") {
    mos_void g({{"A", "B"}, {"A", "C"}, {"A", "D"}, {"B", "C"}});
    auto result = max_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == "A");
    REQUIRE(result->second == 3);
}

TEST_CASE("max_degree - deque-based graph (dofl)", "[generic][6.3.1][max_degree]") {
    dofl_void g({{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}});
    auto result = max_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->second == 2);
    // Vertices 0 and 3 both have degree 2
}

TEST_CASE("max_degree - unordered map graph (mous)", "[generic][6.3.1][max_degree]") {
    mous_void g({{"X", "Y"}, {"X", "Z"}, {"Y", "Z"}});
    auto result = max_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == "X");
    REQUIRE(result->second == 2);
}

TEST_CASE("min_degree - empty graph (vov)", "[generic][6.3.1][min_degree]") {
    vov_void g;
    auto result = min_degree(g);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("min_degree - single vertex no edges (vov)", "[generic][6.3.1][min_degree]") {
    vov_void g;
    g.resize_vertices(1);
    auto result = min_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 0);
    REQUIRE(result->second == 0);
}

TEST_CASE("min_degree - all vertices same degree (vov)", "[generic][6.3.1][min_degree]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto result = min_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->second == 1);
}

TEST_CASE("min_degree - one vertex low degree (vov)", "[generic][6.3.1][min_degree]") {
    vov_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 3}});
    auto result = min_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 3);
    REQUIRE(result->second == 0);
}

TEST_CASE("min_degree - map-based graph (mos)", "[generic][6.3.1][min_degree]") {
    mos_void g({{"A", "B"}, {"A", "C"}, {"B", "C"}, {"C", "D"}});
    auto result = min_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == "D");
    REQUIRE(result->second == 0);
}

TEST_CASE("min_degree - deque-based graph (dofl)", "[generic][6.3.1][min_degree]") {
    dofl_void g({{0, 1}, {0, 2}, {1, 2}});
    auto result = min_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == 2);
    REQUIRE(result->second == 0);
}

TEST_CASE("min_degree - unordered map graph (mous)", "[generic][6.3.1][min_degree]") {
    mous_void g({{"X", "Y"}, {"Y", "Z"}});
    auto result = min_degree(g);
    REQUIRE(result.has_value());
    REQUIRE(result->first == "Z");
    REQUIRE(result->second == 0);
}

TEST_CASE("avg_degree - empty graph (vov)", "[generic][6.3.1][avg_degree]") {
    vov_void g;
    REQUIRE(avg_degree(g) == 0.0);
}

TEST_CASE("avg_degree - single vertex no edges (vov)", "[generic][6.3.1][avg_degree]") {
    vov_void g;
    g.resize_vertices(1);
    REQUIRE(avg_degree(g) == 0.0);
}

TEST_CASE("avg_degree - uniform degree graph (vov)", "[generic][6.3.1][avg_degree]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    REQUIRE(avg_degree(g) == 1.0);
}

TEST_CASE("avg_degree - mixed degrees (vov)", "[generic][6.3.1][avg_degree]") {
    vov_void g({{0, 1}, {0, 2}, {1, 2}});
    // Degrees: 0->2, 1->1, 2->0
    // Average: (2 + 1 + 0) / 3 = 1.0
    REQUIRE(avg_degree(g) == 1.0);
}

TEST_CASE("avg_degree - map-based graph (mos)", "[generic][6.3.1][avg_degree]") {
    mos_void g({{"A", "B"}, {"A", "C"}, {"B", "C"}, {"C", "D"}});
    // Degrees: A->2, B->1, C->1, D->0
    // Average: (2 + 1 + 1 + 0) / 4 = 1.0
    REQUIRE(avg_degree(g) == 1.0);
}

TEST_CASE("avg_degree - deque-based graph (dofl)", "[generic][6.3.1][avg_degree]") {
    dofl_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}});
    // Degrees: 0->3, 1->1, 2->0, 3->0
    // Average: (3 + 1 + 0 + 0) / 4 = 1.0
    REQUIRE(avg_degree(g) == 1.0);
}

TEST_CASE("avg_degree - unordered map graph (mous)", "[generic][6.3.1][avg_degree]") {
    mous_void g({{"X", "Y"}, {"Y", "Z"}, {"Z", "X"}});
    // All have degree 1
    REQUIRE(avg_degree(g) == 1.0);
}

TEST_CASE("avg_degree - high average degree (vov)", "[generic][6.3.1][avg_degree]") {
    vov_void g({{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}});
    // Degrees: 0->3, 1->3, 2->0, 3->0
    // Average: (3 + 3 + 0 + 0) / 4 = 1.5
    REQUIRE(avg_degree(g) == 1.5);
}

TEST_CASE("is_empty - empty graph (vov)", "[generic][6.3.1][is_empty]") {
    vov_void g;
    REQUIRE(is_empty(g));
}

TEST_CASE("is_empty - non-empty graph (vov)", "[generic][6.3.1][is_empty]") {
    vov_void g({{0, 1}});
    REQUIRE_FALSE(is_empty(g));
}

TEST_CASE("is_empty - map-based empty graph (mos)", "[generic][6.3.1][is_empty]") {
    mos_void g;
    REQUIRE(is_empty(g));
}

TEST_CASE("is_empty - map-based non-empty graph (mos)", "[generic][6.3.1][is_empty]") {
    mos_void g({{"A", "B"}});
    REQUIRE_FALSE(is_empty(g));
}

TEST_CASE("is_empty - deque-based empty graph (dofl)", "[generic][6.3.1][is_empty]") {
    dofl_void g;
    REQUIRE(is_empty(g));
}

TEST_CASE("is_empty - deque-based non-empty graph (dofl)", "[generic][6.3.1][is_empty]") {
    dofl_void g({{0, 1}});
    REQUIRE_FALSE(is_empty(g));
}

TEST_CASE("is_empty - unordered map empty graph (mous)", "[generic][6.3.1][is_empty]") {
    mous_void g;
    REQUIRE(is_empty(g));
}

TEST_CASE("is_empty - unordered map non-empty graph (mous)", "[generic][6.3.1][is_empty]") {
    mous_void g({{"X", "Y"}});
    REQUIRE_FALSE(is_empty(g));
}
