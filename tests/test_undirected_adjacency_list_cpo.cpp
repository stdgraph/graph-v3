/**
 * @file test_undirected_adjacency_list_cpo.cpp
 * @brief Tests for CPO (Customization Point Object) support in undirected_adjacency_list
 * 
 * These tests verify that the undirected_adjacency_list works correctly with the
 * graph CPO interface, allowing algorithms to work with this container.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <graph/detail/graph_cpo.hpp>
#include <string>
#include <vector>

// Use container namespace for the graph types
using graph::container::undirected_adjacency_list;

// Basic graph type aliases for testing - use int for GV to avoid void reference issues
using IntGraph = undirected_adjacency_list<int, int, int>;
using StringGVGraph = undirected_adjacency_list<int, int, std::string>;

TEST_CASE("vertices CPO basic", "[undirected_adjacency_list][cpo][vertices]") {
    IntGraph g(42); // graph value = 42
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    // Test via ADL (friend functions)
    auto verts = vertices(g);
    REQUIRE(std::ranges::forward_range<decltype(verts)>);
    
    size_t count = 0;
    for ([[maybe_unused]] auto v : verts) {
        ++count;
    }
    REQUIRE(count == 3);
}

TEST_CASE("vertex_id CPO", "[undirected_adjacency_list][cpo][vertex_id]") {
    IntGraph g(0);
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    SECTION("vertex_id returns correct id for each vertex") {
        auto verts = vertices(g);
        unsigned int expected_id = 0;
        for (auto v : verts) {
            REQUIRE(graph::vertex_id(g, v) == expected_id);
            ++expected_id;
        }
    }
    
    SECTION("vertex_id on const graph") {
        const IntGraph& cg = g;
        auto verts = vertices(cg);
        auto v = *verts.begin();
        REQUIRE(graph::vertex_id(cg, v) == 0);
    }
    
    SECTION("vertex_id after find_vertex") {
        auto it = find_vertex(g, 2u);
        REQUIRE(it != vertices(g).end());
        REQUIRE(graph::vertex_id(g, *it) == 2);
    }
}

TEST_CASE("num_vertices CPO", "[undirected_adjacency_list][cpo][num_vertices]") {
    IntGraph g(0);
    REQUIRE(num_vertices(g) == 0);
    
    g.create_vertex(10);
    REQUIRE(num_vertices(g) == 1);
    
    g.create_vertex(20);
    g.create_vertex(30);
    REQUIRE(num_vertices(g) == 3);
}

TEST_CASE("find_vertex CPO", "[undirected_adjacency_list][cpo][find_vertex]") {
    IntGraph g(0);
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    SECTION("find existing vertex by id") {
        auto it = find_vertex(g, 0u);
        REQUIRE(it != vertices(g).end());
        REQUIRE(vertex_value(g, *it) == 10);
        
        it = find_vertex(g, 1u);
        REQUIRE(it != vertices(g).end());
        REQUIRE(vertex_value(g, *it) == 20);
        
        it = find_vertex(g, 2u);
        REQUIRE(it != vertices(g).end());
        REQUIRE(vertex_value(g, *it) == 30);
    }
    
    SECTION("find non-existent vertex returns end") {
        auto it = find_vertex(g, 99u);
        REQUIRE(it == vertices(g).end());
    }
    
    SECTION("find vertex on empty graph") {
        IntGraph empty_g(0);
        auto it = find_vertex(empty_g, 0u);
        REQUIRE(it == vertices(empty_g).end());
    }
    
    SECTION("const graph find_vertex") {
        const IntGraph& cg = g;
        auto it = find_vertex(cg, 1u);
        REQUIRE(it != vertices(cg).end());
        REQUIRE(vertex_value(cg, *it) == 20);
    }
}

TEST_CASE("num_edges CPO", "[undirected_adjacency_list][cpo][num_edges]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    g.create_vertex();
    
    REQUIRE(num_edges(g) == 0);
    
    g.create_edge(0, 1, 100);
    REQUIRE(num_edges(g) == 1);
    
    g.create_edge(1, 2, 200);
    REQUIRE(num_edges(g) == 2);
}

TEST_CASE("has_edge CPO", "[undirected_adjacency_list][cpo][has_edge]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    
    REQUIRE_FALSE(has_edge(g));
    
    g.create_edge(0, 1, 100);
    REQUIRE(has_edge(g));
}

TEST_CASE("graph_value CPO", "[undirected_adjacency_list][cpo][graph_value]") {
    StringGVGraph g("my graph");
    
    SECTION("read graph value") {
        REQUIRE(graph_value(g) == "my graph");
    }
    
    SECTION("modify graph value") {
        graph_value(g) = "modified";
        REQUIRE(graph_value(g) == "modified");
    }
    
    SECTION("const graph value") {
        const auto& cg = g;
        REQUIRE(graph_value(cg) == "my graph");
    }
}

TEST_CASE("vertex_value CPO", "[undirected_adjacency_list][cpo][vertex_value]") {
    IntGraph g(0);
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    auto verts = vertices(g);
    auto it = verts.begin();
    
    SECTION("read vertex value via descriptor") {
        REQUIRE(vertex_value(g, *it) == 10);
        ++it;
        REQUIRE(vertex_value(g, *it) == 20);
    }
    
    SECTION("modify vertex value via descriptor") {
        vertex_value(g, *it) = 100;
        REQUIRE(vertex_value(g, *it) == 100);
    }
}

TEST_CASE("edges CPO", "[undirected_adjacency_list][cpo][edges]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    g.create_vertex();
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    g.create_edge(1, 2, 300);
    
    SECTION("edges from vertex 0") {
        auto verts = vertices(g);
        auto v = *verts.begin();  // vertex 0
        auto edge_range = edges(g, v);
        
        size_t count = 0;
        for ([[maybe_unused]] auto e : edge_range) {
            ++count;
        }
        REQUIRE(count == 2); // edges to vertices 1 and 2
    }
    
    SECTION("edges from vertex with no edges") {
        IntGraph g2(0);
        g2.create_vertex();
        auto verts = vertices(g2);
        auto v = *verts.begin();
        auto edge_range = edges(g2, v);
        
        size_t count = 0;
        for ([[maybe_unused]] auto e : edge_range) {
            ++count;
        }
        REQUIRE(count == 0);
    }
}

TEST_CASE("degree CPO", "[undirected_adjacency_list][cpo][degree]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    g.create_vertex();
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    g.create_edge(1, 2, 300);
    
    SECTION("degree of vertex with 2 edges") {
        auto verts = vertices(g);
        auto v0 = *verts.begin();
        REQUIRE(degree(g, v0) == 2);
    }
    
    SECTION("degree of vertex with no edges") {
        IntGraph g2(0);
        g2.create_vertex();
        auto verts = vertices(g2);
        auto v = *verts.begin();
        REQUIRE(degree(g2, v) == 0);
    }
}

TEST_CASE("edge target_id CPO via ADL", "[undirected_adjacency_list][cpo][target_id]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    g.create_vertex();
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    
    auto verts = vertices(g);
    auto v0 = *verts.begin();
    auto edge_range = edges(g, v0);
    
    std::vector<unsigned int> targets;
    for (auto e : edge_range) {
        // Access edge via iterator stored in descriptor
        auto edge_it = e.value();
        targets.push_back(target_id(g, *edge_it));
    }
    
    // Edges from vertex 0 should go to vertices 1 and 2
    REQUIRE(targets.size() == 2);
    // Order may vary, so check both are present
    REQUIRE((std::find(targets.begin(), targets.end(), 1) != targets.end()));
    REQUIRE((std::find(targets.begin(), targets.end(), 2) != targets.end()));
}

TEST_CASE("edge source_id CPO via ADL", "[undirected_adjacency_list][cpo][source_id]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    g.create_edge(0, 1, 100);
    
    auto verts = vertices(g);
    auto v0 = *verts.begin();
    auto edge_range = edges(g, v0);
    
    for (auto e : edge_range) {
        auto edge_it = e.value();
        REQUIRE(source_id(g, *edge_it) == 0);
    }
}

TEST_CASE("edge_value CPO via ADL", "[undirected_adjacency_list][cpo][edge_value]") {
    IntGraph g(0);
    g.create_vertex();
    g.create_vertex();
    g.create_edge(0, 1, 100);
    
    auto verts = vertices(g);
    auto v0 = *verts.begin();
    auto edge_range = edges(g, v0);
    auto e = *edge_range.begin();
    
    SECTION("read edge value") {
        auto edge_it = e.value();
        REQUIRE(edge_value(g, *edge_it) == 100);
    }
    
    SECTION("modify edge value") {
        auto edge_it = e.value();
        edge_value(g, *edge_it) = 999;
        REQUIRE(edge_value(g, *edge_it) == 999);
    }
}

TEST_CASE("CPO integration - graph traversal", "[undirected_adjacency_list][cpo][integration]") {
    IntGraph g(0);
    // Create a simple triangle graph: 0 -- 1 -- 2 -- 0
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    g.create_edge(0, 1, 12);
    g.create_edge(1, 2, 23);
    g.create_edge(2, 0, 31);
    
    SECTION("traverse all vertices and edges using CPOs") {
        int total_vertex_value = 0;
        int total_edge_value = 0;
        size_t total_edges = 0;
        
        for (auto v : vertices(g)) {
            total_vertex_value += vertex_value(g, v);
            
            for (auto e : edges(g, v)) {
                auto edge_it = e.value();
                total_edge_value += edge_value(g, *edge_it);
                ++total_edges;
            }
        }
        
        REQUIRE(total_vertex_value == 60); // 10 + 20 + 30
        // Each edge is counted twice (once from each endpoint)
        REQUIRE(total_edges == 6); // 3 edges * 2
        REQUIRE(total_edge_value == (12 + 23 + 31) * 2); // Each edge value counted twice
    }
}

TEST_CASE("target CPO", "[undirected_adjacency_list][cpo][target]") {
    IntGraph g(0);
    g.create_vertex(10);  // vertex 0
    g.create_vertex(20);  // vertex 1
    g.create_vertex(30);  // vertex 2
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    
    SECTION("target returns vertex descriptor") {
        auto verts = vertices(g);
        auto v0 = *verts.begin();  // vertex 0
        auto edge_range = edges(g, v0);
        
        std::vector<int> target_values;
        for (auto e : edge_range) {
            // Use the target CPO with the edge_descriptor directly
            auto target_v = graph::target(g, e);
            target_values.push_back(vertex_value(g, target_v));
        }
        
        // Edges from vertex 0 go to vertices 1 (value=20) and 2 (value=30)
        REQUIRE(target_values.size() == 2);
        REQUIRE((std::find(target_values.begin(), target_values.end(), 20) != target_values.end()));
        REQUIRE((std::find(target_values.begin(), target_values.end(), 30) != target_values.end()));
    }
    
    SECTION("target on const graph") {
        const IntGraph& cg = g;
        auto verts = vertices(cg);
        auto v0 = *verts.begin();
        auto edge_range = edges(cg, v0);
        auto e = *edge_range.begin();
        
        auto target_v = graph::target(cg, e);
        // Just verify it compiles and returns something valid
        auto tid = graph::vertex_id(cg, target_v);
        REQUIRE((tid == 1 || tid == 2));
    }
}

TEST_CASE("source CPO", "[undirected_adjacency_list][cpo][source]") {
    IntGraph g(0);
    g.create_vertex(10);  // vertex 0
    g.create_vertex(20);  // vertex 1
    g.create_vertex(30);  // vertex 2
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    
    SECTION("source returns vertex descriptor") {
        auto verts = vertices(g);
        auto v0 = *verts.begin();  // vertex 0
        auto edge_range = edges(g, v0);
        
        // All edges from v0 should have v0 as source
        for (auto e : edge_range) {
            auto source_v = graph::source(g, e);
            REQUIRE(graph::vertex_id(g, source_v) == 0);
            REQUIRE(vertex_value(g, source_v) == 10);
        }
    }
    
    SECTION("source on const graph") {
        const IntGraph& cg = g;
        auto verts = vertices(cg);
        auto v0 = *verts.begin();
        auto edge_range = edges(cg, v0);
        auto e = *edge_range.begin();
        
        auto source_v = graph::source(cg, e);
        REQUIRE(graph::vertex_id(cg, source_v) == 0);
    }
}

TEST_CASE("find_vertex_edge CPO", "[undirected_adjacency_list][cpo][find_vertex_edge]") {
    IntGraph g(0);
    g.create_vertex(10);  // vertex 0
    g.create_vertex(20);  // vertex 1
    g.create_vertex(30);  // vertex 2
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    g.create_edge(1, 2, 300);
    
    SECTION("find_vertex_edge with descriptor and vid") {
        auto verts = vertices(g);
        auto v0 = *verts.begin();  // vertex 0
        
        // Find edge from vertex 0 to vertex 1
        // Returns an edge_descriptor
        auto e = graph::find_vertex_edge(g, v0, 1u);
        // Use edge_value CPO with edge_descriptor
        auto edge_it = e.value();
        REQUIRE(edge_value(g, *edge_it) == 100);
        REQUIRE(target_id(g, e) == 1);
    }
    
    SECTION("find_vertex_edge with two vertex ids") {
        // Find edge from vertex 0 to vertex 2
        auto e = graph::find_vertex_edge(g, 0u, 2u);
        auto edge_it = e.value();
        REQUIRE(edge_value(g, *edge_it) == 200);
        REQUIRE(target_id(g, e) == 2);
    }
    
    SECTION("find_vertex_edge between two descriptors") {
        auto verts = vertices(g);
        auto it = verts.begin();
        auto v0 = *it;
        ++it;
        auto v1 = *it;
        
        auto e = graph::find_vertex_edge(g, v0, v1);
        auto edge_it = e.value();
        REQUIRE(edge_value(g, *edge_it) == 100);
    }
    
    SECTION("find_vertex_edge on const graph") {
        const IntGraph& cg = g;
        auto e = graph::find_vertex_edge(cg, 1u, 2u);
        auto edge_it = e.value();
        REQUIRE(edge_value(cg, *edge_it) == 300);
    }
}

TEST_CASE("contains_edge CPO", "[undirected_adjacency_list][cpo][contains_edge]") {
    IntGraph g(0);
    g.create_vertex(10);  // vertex 0
    g.create_vertex(20);  // vertex 1
    g.create_vertex(30);  // vertex 2
    g.create_edge(0, 1, 100);
    g.create_edge(0, 2, 200);
    // No edge between 1 and 2
    
    SECTION("contains_edge with two vertex ids - edge exists") {
        REQUIRE(graph::contains_edge(g, 0u, 1u) == true);
        REQUIRE(graph::contains_edge(g, 0u, 2u) == true);
    }
    
    SECTION("contains_edge with two vertex ids - edge does not exist") {
        REQUIRE(graph::contains_edge(g, 1u, 2u) == false);
    }
    
    SECTION("contains_edge with two vertex descriptors") {
        auto verts = vertices(g);
        auto it = verts.begin();
        auto v0 = *it;
        ++it;
        auto v1 = *it;
        ++it;
        auto v2 = *it;
        
        REQUIRE(graph::contains_edge(g, v0, v1) == true);
        REQUIRE(graph::contains_edge(g, v0, v2) == true);
        REQUIRE(graph::contains_edge(g, v1, v2) == false);
    }
    
    SECTION("contains_edge on const graph") {
        const IntGraph& cg = g;
        REQUIRE(graph::contains_edge(cg, 0u, 1u) == true);
        REQUIRE(graph::contains_edge(cg, 1u, 2u) == false);
    }
}

TEST_CASE("edges(g) graph-level CPO", "[undirected_adjacency_list][cpo][edges]") {
    IntGraph g(0);
    g.create_vertex(10);  // vertex 0
    g.create_vertex(20);  // vertex 1
    g.create_vertex(30);  // vertex 2
    g.create_edge(0, 1, 100);
    g.create_edge(1, 2, 200);
    g.create_edge(0, 2, 300);
    
    SECTION("graph-level edges iteration visits all edges") {
        // Use the graph's edges() member which iterates all edges
        // Note: In undirected graphs, each edge is visited twice
        size_t count = 0;
        int total_value = 0;
        
        for (auto& v : g.vertices()) {
            auto uid = static_cast<unsigned int>(&v - &g.vertices()[0]);
            for (auto& e : v.edges(g, uid)) {
                ++count;
                total_value += e.value;
            }
        }
        
        // Each edge counted twice (once from each endpoint)
        REQUIRE(count == 6);
        REQUIRE(total_value == (100 + 200 + 300) * 2);
    }
    
    SECTION("unique edge count") {
        // The edges_size() returns unique edge count
        REQUIRE(g.edges_size() == 3);
    }
}

TEST_CASE("source_id with vertex descriptor edges", "[undirected_adjacency_list][cpo][source_id]") {
    IntGraph g(0);
    g.create_vertex(10);  // vertex 0
    g.create_vertex(20);  // vertex 1
    g.create_vertex(30);  // vertex 2
    g.create_edge(0, 1, 100);
    g.create_edge(1, 2, 200);
    
    SECTION("source_id via CPO") {
        auto verts = vertices(g);
        auto v1_it = verts.begin();
        ++v1_it;  // vertex 1
        auto v1 = *v1_it;
        
        auto edge_range = edges(g, v1);
        for (auto e : edge_range) {
            // Source should always be v1 (id=1) when iterating from v1
            REQUIRE(source_id(g, e) == 1);
        }
    }
}

TEST_CASE("CPO with empty graph", "[undirected_adjacency_list][cpo][empty]") {
    IntGraph g(42);
    
    SECTION("vertices on empty graph") {
        auto verts = vertices(g);
        REQUIRE(verts.begin() == verts.end());
    }
    
    SECTION("num_vertices on empty graph") {
        REQUIRE(num_vertices(g) == 0);
    }
    
    SECTION("num_edges on empty graph") {
        REQUIRE(num_edges(g) == 0);
    }
    
    SECTION("has_edge on empty graph") {
        REQUIRE_FALSE(has_edge(g));
    }
    
    SECTION("find_vertex on empty graph returns end") {
        auto it = find_vertex(g, 0u);
        REQUIRE(it == vertices(g).end());
    }
    
    SECTION("graph_value on empty graph") {
        REQUIRE(graph_value(g) == 42);
    }
}

TEST_CASE("CPO const correctness", "[undirected_adjacency_list][cpo][const]") {
    IntGraph g(0);
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_edge(0, 1, 100);
    
    const IntGraph& cg = g;
    
    SECTION("all read CPOs work on const graph") {
        REQUIRE(num_vertices(cg) == 2);
        REQUIRE(num_edges(cg) == 1);
        REQUIRE(has_edge(cg));
        
        auto verts = vertices(cg);
        REQUIRE(std::distance(verts.begin(), verts.end()) == 2);
        
        auto v = *verts.begin();
        REQUIRE(graph::vertex_id(cg, v) == 0);
        REQUIRE(vertex_value(cg, v) == 10);
        REQUIRE(degree(cg, v) == 1);
        
        auto edge_range = edges(cg, v);
        auto e = *edge_range.begin();
        REQUIRE(target_id(cg, e) == 1);
        REQUIRE(source_id(cg, e) == 0);
        
        auto target_v = graph::target(cg, e);
        REQUIRE(graph::vertex_id(cg, target_v) == 1);
        
        auto source_v = graph::source(cg, e);
        REQUIRE(graph::vertex_id(cg, source_v) == 0);
    }
}

TEST_CASE("CPO vertex_id consistency", "[undirected_adjacency_list][cpo][vertex_id]") {
    IntGraph g(0);
    for (int i = 0; i < 10; ++i) {
        g.create_vertex(i * 10);
    }
    
    SECTION("vertex_id matches iteration order") {
        unsigned int expected = 0;
        for (auto v : vertices(g)) {
            REQUIRE(graph::vertex_id(g, v) == expected);
            ++expected;
        }
        REQUIRE(expected == 10);
    }
    
    SECTION("vertex_id matches find_vertex result") {
        for (unsigned int i = 0; i < 10; ++i) {
            auto it = find_vertex(g, i);
            REQUIRE(it != vertices(g).end());
            REQUIRE(graph::vertex_id(g, *it) == i);
        }
    }
}

TEST_CASE("CPO edge traversal consistency", "[undirected_adjacency_list][cpo][edges]") {
    IntGraph g(0);
    g.create_vertex(10);  // 0
    g.create_vertex(20);  // 1
    g.create_vertex(30);  // 2
    g.create_vertex(40);  // 3
    // Create a path: 0 -- 1 -- 2 -- 3
    g.create_edge(0, 1, 1);
    g.create_edge(1, 2, 2);
    g.create_edge(2, 3, 3);
    
    SECTION("edge target and source are consistent") {
        for (auto v : vertices(g)) {
            auto vid = graph::vertex_id(g, v);
            for (auto e : edges(g, v)) {
                auto sid = source_id(g, e);
                auto tid = target_id(g, e);
                
                // Source should always be the vertex we're iterating from
                REQUIRE(sid == vid);
                
                // Target should be different from source (no self-loops in this test)
                REQUIRE(tid != sid);
                
                // Verify source/target descriptors
                auto source_v = graph::source(g, e);
                auto target_v = graph::target(g, e);
                REQUIRE(graph::vertex_id(g, source_v) == sid);
                REQUIRE(graph::vertex_id(g, target_v) == tid);
            }
        }
    }
}

