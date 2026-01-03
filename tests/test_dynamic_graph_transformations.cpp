/**
 * @file test_dynamic_graph_transformations.cpp
 * @brief Tests for Phase 6.3.3: Generic Graph Transformations
 * 
 * This file tests generic graph transformation functions that work with any
 * graph type using only CPO-based abstractions. These functions create new
 * graphs based on transformations of existing graphs.
 * 
 * Functions tested:
 * - extract_subgraph(g, vids): Create subgraph with selected vertices
 * - copy_graph_generic(g): Generic copy to same graph type
 * - reverse_edges(g): Create new graph with reversed edges
 * - filter_edges(g, predicate): Create graph with subset of edges
 * 
 * Graph types tested: vov, mos, dofl, dov
 * Test count: 30 tests
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <graph/graph_info.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <map>
#include <ranges>

using namespace graph;
using namespace graph::container;

// Type aliases for testing
using vov_void = dynamic_graph<void, void, void, uint64_t, false,
                               vov_graph_traits<void, void, void, uint64_t, false>>;

using mos_void = dynamic_graph<void, void, void, std::string, false,
                               mos_graph_traits<void, void, void, std::string, false>>;

using dofl_void = dynamic_graph<void, void, void, uint64_t, false,
                                dofl_graph_traits<void, void, void, uint64_t, false>>;

using dov_void = dynamic_graph<void, void, void, uint64_t, false,
                               dov_graph_traits<void, void, void, uint64_t, false>>;

// ============================================================================
// Generic Transformation Functions (CPO-based)
// ============================================================================

/**
 * @brief Extract a subgraph containing only specified vertices
 * @tparam G Graph type
 * @param g Source graph
 * @param vertex_ids Vector of vertex IDs to include in subgraph
 * @return New graph of same type containing only specified vertices and edges between them
 * 
 * Note: For integral vertex IDs, this function remaps IDs to be contiguous starting from 0.
 * For string IDs, the original IDs are preserved.
 */
template<typename G>
G extract_subgraph(const G& g, const std::vector<typename G::vertex_id_type>& vertex_ids) {
    using VId = typename G::vertex_id_type;
    std::set<VId> included_vertices(vertex_ids.begin(), vertex_ids.end());
    
    // For integral types, create a mapping from old IDs to new contiguous IDs
    std::map<VId, VId> id_mapping;
    if constexpr (std::is_integral_v<VId>) {
        VId new_id = 0;
        for (const auto& old_id : vertex_ids) {
            id_mapping[old_id] = new_id++;
        }
    }
    
    // Collect edges where both source and target are in the included set
    std::vector<copyable_edge_t<VId, void>> subgraph_edges;
    
    for (auto&& v : vertices(g)) {
        VId source_id = vertex_id(g, v);
        if (included_vertices.contains(source_id)) {
            for (auto&& e : edges(g, v)) {
                VId target_id_val = target_id(g, e);
                if (included_vertices.contains(target_id_val)) {
                    if constexpr (std::is_integral_v<VId>) {
                        // Remap IDs for integral types
                        subgraph_edges.push_back({id_mapping[source_id], id_mapping[target_id_val]});
                    } else {
                        // Preserve string IDs
                        subgraph_edges.push_back({source_id, target_id_val});
                    }
                }
            }
        }
    }
    
    // Create new graph and load edges
    G result;
    if constexpr (std::is_integral_v<VId>) {
        result.load_edges(subgraph_edges, std::identity{}, vertex_ids.size());
    } else {
        result.load_edges(subgraph_edges, std::identity{}, vertex_ids.size());
    }
    return result;
}

/**
 * @brief Create a copy of a graph using only CPOs
 * @tparam G Graph type
 * @param g Source graph
 * @return New graph that is a copy of the source
 */
template<typename G>
G copy_graph_generic(const G& g) {
    using VId = typename G::vertex_id_type;
    
    // Extract all edges using CPOs
    std::vector<copyable_edge_t<VId, void>> edge_list;
    for (auto&& v : vertices(g)) {
        for (auto&& e : edges(g, v)) {
            edge_list.push_back({vertex_id(g, v), target_id(g, e)});
        }
    }
    
    // Create new graph and load edges
    G result;
    result.load_edges(edge_list, std::identity{}, g.size());
    return result;
}

/**
 * @brief Create a new graph with all edges reversed
 * @tparam G Graph type
 * @param g Source graph
 * @return New graph where every edge u->v becomes v->u
 */
template<typename G>
G reverse_edges(const G& g) {
    using VId = typename G::vertex_id_type;
    
    // Collect all edges and reverse them
    std::vector<copyable_edge_t<VId, void>> reversed_edges;
    for (auto&& v : vertices(g)) {
        VId source_id = vertex_id(g, v);
        for (auto&& e : edges(g, v)) {
            VId target_id_val = target_id(g, e);
            // Reverse: target becomes source, source becomes target
            reversed_edges.push_back({target_id_val, source_id});
        }
    }
    
    // Create new graph with reversed edges
    G result;
    result.load_edges(reversed_edges, std::identity{}, g.size());
    return result;
}

/**
 * @brief Filter edges based on a predicate
 * @tparam G Graph type
 * @tparam Pred Predicate type: bool(VId source, VId target)
 * @param g Source graph
 * @param predicate Function that returns true if edge should be kept
 * @return New graph containing only edges that satisfy the predicate
 */
template<typename G, typename Pred>
G filter_edges(const G& g, Pred predicate) {
    using VId = typename G::vertex_id_type;
    
    // Collect edges that satisfy the predicate
    std::vector<copyable_edge_t<VId, void>> filtered_edges;
    for (auto&& v : vertices(g)) {
        VId source_id = vertex_id(g, v);
        for (auto&& e : edges(g, v)) {
            VId target_id_val = target_id(g, e);
            if (predicate(source_id, target_id_val)) {
                filtered_edges.push_back({source_id, target_id_val});
            }
        }
    }
    
    // Create new graph with filtered edges
    G result;
    result.load_edges(filtered_edges, std::identity{}, g.size());
    return result;
}

// ============================================================================
// Test Cases: extract_subgraph
// ============================================================================

TEST_CASE("extract_subgraph - empty vertex list (vov)", "[6.3.3][extract_subgraph][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{});
    REQUIRE(sub.size() == 0);
}

TEST_CASE("extract_subgraph - single vertex (vov)", "[6.3.3][extract_subgraph][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{1});
    REQUIRE(sub.size() == 1);
    // No edges since vertex 1's edges go to 0 and 2, which aren't included
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 0);
}

TEST_CASE("extract_subgraph - two connected vertices (vov)", "[6.3.3][extract_subgraph][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{0, 1});
    REQUIRE(sub.size() == 2);
    // Should have edge 0->1
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 1);
}

TEST_CASE("extract_subgraph - complete subgraph (vov)", "[6.3.3][extract_subgraph][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{0, 1, 2});
    REQUIRE(sub.size() == 3);
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 3);  // All edges preserved
}

TEST_CASE("extract_subgraph - map-based graph (mos)", "[6.3.3][extract_subgraph][transform]") {
    mos_void g({{"A", "B"}, {"B", "C"}, {"C", "A"}});
    auto sub = extract_subgraph(g, std::vector<std::string>{"A", "B"});
    REQUIRE(sub.size() == 2);
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 1);  // Only A->B
}

TEST_CASE("extract_subgraph - deque-based graph (dofl)", "[6.3.3][extract_subgraph][transform]") {
    dofl_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{1, 2});
    REQUIRE(sub.size() == 2);
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 1);  // Only 1->2
}

TEST_CASE("extract_subgraph - disconnected vertices (vov)", "[6.3.3][extract_subgraph][transform]") {
    vov_void g({{0, 1}, {2, 3}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{0, 2});
    REQUIRE(sub.size() == 2);
    // No edges between 0 and 2
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 0);
}

// ============================================================================
// Test Cases: copy_graph_generic
// ============================================================================

TEST_CASE("copy_graph_generic - empty graph (vov)", "[6.3.3][copy_graph][transform]") {
    vov_void g;
    auto copy = copy_graph_generic(g);
    REQUIRE(copy.size() == 0);
}

TEST_CASE("copy_graph_generic - simple graph (vov)", "[6.3.3][copy_graph][transform]") {
    vov_void g({{0, 1}, {1, 2}});
    auto copy = copy_graph_generic(g);
    REQUIRE(copy.size() == g.size());
    
    size_t orig_edges = 0, copy_edges = 0;
    for (auto&& v : vertices(g)) {
        orig_edges += std::ranges::distance(edges(g, v));
    }
    for (auto&& v : vertices(copy)) {
        copy_edges += std::ranges::distance(edges(copy, v));
    }
    REQUIRE(orig_edges == copy_edges);
}

TEST_CASE("copy_graph_generic - graph with self-loop (vov)", "[6.3.3][copy_graph][transform]") {
    vov_void g({{0, 0}, {0, 1}});
    auto copy = copy_graph_generic(g);
    REQUIRE(copy.size() == 2);
    
    size_t copy_edges = 0;
    for (auto&& v : vertices(copy)) {
        copy_edges += std::ranges::distance(edges(copy, v));
    }
    REQUIRE(copy_edges == 2);
}

TEST_CASE("copy_graph_generic - map-based graph (mos)", "[6.3.3][copy_graph][transform]") {
    mos_void g({{"A", "B"}, {"B", "C"}});
    auto copy = copy_graph_generic(g);
    REQUIRE(copy.size() == 3);
    
    size_t copy_edges = 0;
    for (auto&& v : vertices(copy)) {
        copy_edges += std::ranges::distance(edges(copy, v));
    }
    REQUIRE(copy_edges == 2);
}

TEST_CASE("copy_graph_generic - deque-based graph (dofl)", "[6.3.3][copy_graph][transform]") {
    dofl_void g({{0, 1}, {1, 2}, {2, 0}});
    auto copy = copy_graph_generic(g);
    REQUIRE(copy.size() == 3);
    
    size_t copy_edges = 0;
    for (auto&& v : vertices(copy)) {
        copy_edges += std::ranges::distance(edges(copy, v));
    }
    REQUIRE(copy_edges == 3);
}

// ============================================================================
// Test Cases: reverse_edges
// ============================================================================

TEST_CASE("reverse_edges - empty graph (vov)", "[6.3.3][reverse_edges][transform]") {
    vov_void g;
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 0);
}

TEST_CASE("reverse_edges - single edge (vov)", "[6.3.3][reverse_edges][transform]") {
    vov_void g({{0, 1}});
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 2);
    
    // Original has 0->1, reversed should have 1->0
    bool found_reversed = false;
    for (auto&& v : vertices(reversed)) {
        if (vertex_id(reversed, v) == 1u) {
            for (auto&& e : edges(reversed, v)) {
                if (target_id(reversed, e) == 0u) {
                    found_reversed = true;
                }
            }
        }
    }
    REQUIRE(found_reversed);
}

TEST_CASE("reverse_edges - cycle (vov)", "[6.3.3][reverse_edges][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 3);
    
    size_t edge_count = 0;
    for (auto&& v : vertices(reversed)) {
        edge_count += std::ranges::distance(edges(reversed, v));
    }
    REQUIRE(edge_count == 3);
    
    // Cycle should be reversed: 0->2, 2->1, 1->0
    bool has_0_to_2 = false, has_2_to_1 = false, has_1_to_0 = false;
    for (auto&& v : vertices(reversed)) {
        uint64_t vid = vertex_id(reversed, v);
        for (auto&& e : edges(reversed, v)) {
            uint64_t tid = target_id(reversed, e);
            if (vid == 0u && tid == 2u) has_0_to_2 = true;
            if (vid == 2u && tid == 1u) has_2_to_1 = true;
            if (vid == 1u && tid == 0u) has_1_to_0 = true;
        }
    }
    REQUIRE(has_0_to_2);
    REQUIRE(has_2_to_1);
    REQUIRE(has_1_to_0);
}

TEST_CASE("reverse_edges - self-loop (vov)", "[6.3.3][reverse_edges][transform]") {
    vov_void g({{0, 0}});
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 1);
    
    // Self-loop should remain self-loop
    bool has_self_loop = false;
    for (auto&& v : vertices(reversed)) {
        for (auto&& e : edges(reversed, v)) {
            if (vertex_id(reversed, v) == target_id(reversed, e)) {
                has_self_loop = true;
            }
        }
    }
    REQUIRE(has_self_loop);
}

TEST_CASE("reverse_edges - map-based graph (mos)", "[6.3.3][reverse_edges][transform]") {
    mos_void g({{"A", "B"}, {"B", "C"}});
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 3);
    
    // Original: A->B, B->C; Reversed: B->A, C->B
    bool has_B_to_A = false, has_C_to_B = false;
    for (auto&& v : vertices(reversed)) {
        std::string vid = vertex_id(reversed, v);
        for (auto&& e : edges(reversed, v)) {
            std::string tid = target_id(reversed, e);
            if (vid == "B" && tid == "A") has_B_to_A = true;
            if (vid == "C" && tid == "B") has_C_to_B = true;
        }
    }
    REQUIRE(has_B_to_A);
    REQUIRE(has_C_to_B);
}

TEST_CASE("reverse_edges - deque-based graph (dofl)", "[6.3.3][reverse_edges][transform]") {
    dofl_void g({{0, 1}, {1, 2}});
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 3);
    
    size_t edge_count = 0;
    for (auto&& v : vertices(reversed)) {
        edge_count += std::ranges::distance(edges(reversed, v));
    }
    REQUIRE(edge_count == 2);
}

// ============================================================================
// Test Cases: filter_edges
// ============================================================================

TEST_CASE("filter_edges - keep all edges (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto filtered = filter_edges(g, [](uint64_t, uint64_t) { return true; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 3);
}

TEST_CASE("filter_edges - remove all edges (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto filtered = filter_edges(g, [](uint64_t, uint64_t) { return false; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 0);
}

TEST_CASE("filter_edges - keep edges where source < target (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g({{0, 1}, {1, 2}, {2, 0}});
    auto filtered = filter_edges(g, [](uint64_t s, uint64_t t) { return s < t; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 2);  // 0->1 and 1->2, but not 2->0
}

TEST_CASE("filter_edges - remove self-loops (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g({{0, 0}, {0, 1}, {1, 1}, {1, 2}});
    auto filtered = filter_edges(g, [](uint64_t s, uint64_t t) { return s != t; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 2);  // Only 0->1 and 1->2
}

TEST_CASE("filter_edges - keep specific targets (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g({{0, 1}, {0, 2}, {0, 3}, {1, 2}});
    // Keep only edges to vertex 2
    auto filtered = filter_edges(g, [](uint64_t, uint64_t t) { return t == 2; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 2);  // 0->2 and 1->2
}

TEST_CASE("filter_edges - map-based graph (mos)", "[6.3.3][filter_edges][transform]") {
    mos_void g({{"A", "B"}, {"B", "C"}, {"C", "A"}});
    // Keep only edges where target != "A"
    auto filtered = filter_edges(g, [](const std::string&, const std::string& t) { return t != "A"; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 2);  // A->B and B->C, but not C->A
}

TEST_CASE("filter_edges - deque-based graph (dofl)", "[6.3.3][filter_edges][transform]") {
    dofl_void g({{0, 1}, {1, 2}, {2, 3}, {3, 0}});
    // Keep only edges where source is even
    auto filtered = filter_edges(g, [](uint64_t s, uint64_t) { return s % 2 == 0; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 2);  // 0->1 and 2->3
}

TEST_CASE("filter_edges - empty graph (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g;
    auto filtered = filter_edges(g, [](uint64_t, uint64_t) { return true; });
    REQUIRE(filtered.size() == 0);
}

// ============================================================================
// Additional Edge Cases
// ============================================================================

TEST_CASE("extract_subgraph - with self-loops (vov)", "[6.3.3][extract_subgraph][transform]") {
    vov_void g({{0, 0}, {0, 1}, {1, 1}});
    auto sub = extract_subgraph(g, std::vector<uint64_t>{0});
    REQUIRE(sub.size() == 1);
    // Should have self-loop 0->0
    size_t edge_count = 0;
    for (auto&& v : vertices(sub)) {
        edge_count += std::ranges::distance(edges(sub, v));
    }
    REQUIRE(edge_count == 1);
}

TEST_CASE("copy_graph_generic - complex graph (dov)", "[6.3.3][copy_graph][transform]") {
    dov_void g({{0, 1}, {0, 2}, {1, 2}, {2, 0}});
    auto copy = copy_graph_generic(g);
    REQUIRE(copy.size() == 3);
    
    size_t copy_edges = 0;
    for (auto&& v : vertices(copy)) {
        copy_edges += std::ranges::distance(edges(copy, v));
    }
    REQUIRE(copy_edges == 4);
}

TEST_CASE("reverse_edges - bidirectional edges (vov)", "[6.3.3][reverse_edges][transform]") {
    vov_void g({{0, 1}, {1, 0}});  // Bidirectional between 0 and 1
    auto reversed = reverse_edges(g);
    REQUIRE(reversed.size() == 2);
    
    // After reversal, should still be bidirectional
    size_t edge_count = 0;
    for (auto&& v : vertices(reversed)) {
        edge_count += std::ranges::distance(edges(reversed, v));
    }
    REQUIRE(edge_count == 2);
}

TEST_CASE("filter_edges - complex predicate (vov)", "[6.3.3][filter_edges][transform]") {
    vov_void g({{0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3}});
    // Keep edges where (source + target) is even
    auto filtered = filter_edges(g, [](uint64_t s, uint64_t t) { return (s + t) % 2 == 0; });
    
    size_t edge_count = 0;
    for (auto&& v : vertices(filtered)) {
        edge_count += std::ranges::distance(edges(filtered, v));
    }
    REQUIRE(edge_count == 2);  // 0->2 (sum=2), 1->3 (sum=4)
}
