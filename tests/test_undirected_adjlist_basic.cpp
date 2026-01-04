#include <catch2/catch_test_macros.hpp>
#include "graph/container/container_utility.hpp"
#include "graph/container/undirected_adjacency_list.hpp"
#include <string>
#include <vector>

using namespace std;
using namespace graph;
using namespace graph::container;

// =============================================================================
// Phase 4.1: Basic Operations Tests (Native API - No CPOs)
// =============================================================================
// These tests focus on the native graph implementation using member functions
// directly, without using CPOs (Customization Point Objects).
// =============================================================================

// =============================================================================
// Category 1: Construction Tests
// =============================================================================

TEST_CASE("undirected_adjacency_list default constructor", "[native][constructors][basic]") {
    SECTION("default types (all empty_value)") {
        undirected_adjacency_list<> g;
        REQUIRE(g.vertices().empty());
        REQUIRE(g.vertices().size() == 0);
        REQUIRE(g.edges_size() == 0);
    }
    
    SECTION("int/int/int with graph value") {
        undirected_adjacency_list<int, int, int> g(42);
        REQUIRE(g.vertices().empty());
        REQUIRE(g.vertices().size() == 0);
        REQUIRE(g.edges_size() == 0);
        REQUIRE(g.graph_value() == 42);
    }
    
    SECTION("string/string/string with graph value") {
        undirected_adjacency_list<string, string, string> g(string("test"));
        REQUIRE(g.vertices().empty());
        REQUIRE(g.graph_value() == "test");
    }
}

TEST_CASE("undirected_adjacency_list initializer list constructor (no edge values)", "[native][constructors][basic]") {
    undirected_adjacency_list<> g({
        {0, 1},
        {0, 2},
        {1, 2}
    });
    
    REQUIRE(g.vertices().size() == 3);
    REQUIRE(g.edges_size() == 3);
}

TEST_CASE("undirected_adjacency_list initializer list constructor (valued edges)", "[native][constructors][basic]") {
    undirected_adjacency_list<int, int> g({
        {0, 1, 10},
        {0, 2, 20},
        {1, 2, 30}
    });
    
    REQUIRE(g.vertices().size() == 3);
    REQUIRE(g.edges_size() == 3);
}

// =============================================================================
// Category 2: Empty Graph Behavior
// =============================================================================

TEST_CASE("undirected_adjacency_list empty graph operations", "[native][empty][basic]") {
    undirected_adjacency_list<> g;
    
    SECTION("empty graph has no vertices") {
        REQUIRE(g.vertices().empty());
        REQUIRE(g.vertices().size() == 0);
    }
    
    SECTION("empty graph has no edges") {
        REQUIRE(g.edges_size() == 0);
    }
    
    SECTION("begin equals end for empty graph") {
        REQUIRE(g.begin() == g.end());
        REQUIRE(g.cbegin() == g.cend());
    }
    
    SECTION("edges_begin equals edges_end for empty graph") {
        REQUIRE(g.edges_begin() == g.edges_end());
        REQUIRE(g.edges_cbegin() == g.edges_cend());
    }
}

// =============================================================================
// Category 3: Single Vertex Operations
// =============================================================================

TEST_CASE("undirected_adjacency_list single vertex creation", "[native][vertex][basic]") {
    SECTION("create single vertex with void value") {
        undirected_adjacency_list<> g;
        auto v_it = g.create_vertex();
        
        REQUIRE(g.vertices().size() == 1);
        REQUIRE(v_it != g.end());
    }
    
    SECTION("create single vertex with int value") {
        undirected_adjacency_list<int> g;
        auto v_it = g.create_vertex(42);
        
        REQUIRE(g.vertices().size() == 1);
        REQUIRE(v_it != g.end());
        REQUIRE((*v_it).value == 42);
    }
    
    SECTION("create single vertex with string value") {
        undirected_adjacency_list<string> g;
        auto v_it = g.create_vertex(string("test"));
        
        REQUIRE(g.vertices().size() == 1);
        REQUIRE(static_cast<const string&>(*v_it) == "test");
    }
}

TEST_CASE("undirected_adjacency_list single vertex access", "[native][vertex][basic]") {
    undirected_adjacency_list<int> g;
    auto v_it = g.create_vertex(100);
    
    SECTION("vertex can be accessed via iterator") {
        REQUIRE(v_it != g.end());
        REQUIRE((*v_it).value == 100);
    }
    
    SECTION("vertex can be found by index") {
        auto key = v_it - g.begin();  // Use iterator arithmetic to get index
        auto found = g.find_vertex(key);
        REQUIRE(found != g.end());
        REQUIRE(found == v_it);
    }
    
    SECTION("vertex has no edges initially") {
        REQUIRE((*v_it).edges(g, v_it - g.begin()).empty());
        REQUIRE((*v_it).edges_size() == 0);
    }
}

TEST_CASE("undirected_adjacency_list single vertex iteration", "[native][vertex][basic]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(42);
    
    SECTION("iteration visits single vertex") {
        size_t count = 0;
        for (auto& v : g.vertices()) {
            REQUIRE(v.value == 42);
            ++count;
        }
        REQUIRE(count == 1);
    }
    
    SECTION("const iteration visits single vertex") {
        const auto& cg = g;
        size_t count = 0;
        for (const auto& v : cg.vertices()) {
            REQUIRE(v.value == 42);
            ++count;
        }
        REQUIRE(count == 1);
    }
}

// =============================================================================
// Category 4: Multiple Vertex Operations
// =============================================================================

TEST_CASE("undirected_adjacency_list multiple vertex creation", "[native][vertex][basic]") {
    undirected_adjacency_list<int> g;
    
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto v3 = g.create_vertex(30);
    auto k3 = v3 - g.begin();
    
    REQUIRE(g.vertices().size() == 3);
    REQUIRE(g.vertices()[k1].value == 10);
    REQUIRE(g.vertices()[k2].value == 20);
    REQUIRE(g.vertices()[k3].value == 30);
}

TEST_CASE("undirected_adjacency_list multiple vertex iteration", "[native][vertex][basic]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    SECTION("iteration visits all vertices") {
        vector<int> values;
        for (auto& v : g.vertices()) {
            values.push_back(v.value);
        }
        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }
}

TEST_CASE("undirected_adjacency_list vertex find operations", "[native][vertex][basic]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    SECTION("find existing vertices by index") {
        auto v1 = g.find_vertex(0);
        auto v2 = g.find_vertex(1);
        auto v3 = g.find_vertex(2);
        
        REQUIRE(v1 != g.end());
        REQUIRE(v2 != g.end());
        REQUIRE(v3 != g.end());
        REQUIRE((*v1).value == 10);
        REQUIRE((*v2).value == 20);
        REQUIRE((*v3).value == 30);
    }
    
    SECTION("find_vertex returns end for non-existent key") {
        REQUIRE(g.find_vertex(999) == g.end());
    }
}

// =============================================================================
// Category 5: Single Edge Operations
// =============================================================================

TEST_CASE("undirected_adjacency_list single edge creation", "[native][edge][basic]") {
    SECTION("create edge with void value") {
        undirected_adjacency_list<> g;
        g.create_vertex();
        g.create_vertex();
        
        auto k1 = 0;
        auto k2 = 1;
        auto e_it = g.create_edge(k1, k2);
        
        REQUIRE(g.edges_size() == 1);
        REQUIRE(e_it != g.vertices()[k1].edges_end(g, k1));
    }
    
    SECTION("create edge with int value") {
        undirected_adjacency_list<int, int> g;
        auto v1 = g.create_vertex();
        auto k1 = v1 - g.begin();
        auto v2 = g.create_vertex();
        auto k2 = v2 - g.begin();
        auto e_it = g.create_edge(k1, k2, 100);
        
        REQUIRE(g.edges_size() == 1);
        REQUIRE((*e_it).value == 100);
    }
    
    SECTION("create edge with string value") {
        undirected_adjacency_list<string, string> g;
        auto v1 = g.create_vertex();
        auto k1 = v1 - g.begin();
        auto v2 = g.create_vertex();
        auto k2 = v2 - g.begin();
        auto e_it = g.create_edge(k1, k2, string("edge1"));
        
        REQUIRE(g.edges_size() == 1);
        REQUIRE(static_cast<const string&>(*e_it) == "edge1");
    }
}

TEST_CASE("undirected_adjacency_list single edge access", "[native][edge][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto e_it = g.create_edge(k1, k2, 100);
    
    SECTION("edge is accessible from source vertex") {
        REQUIRE(g.vertices()[k1].edges_size() == 1);
        REQUIRE_FALSE(g.vertices()[k1].edges(g, k1).empty());
        
        auto& e = *g.vertices()[k1].edges_begin(g, k1);
        REQUIRE(e.value == 100);
    }
    
    SECTION("edge is accessible from target vertex (undirected)") {
        REQUIRE(g.vertices()[k2].edges_size() == 1);
        REQUIRE_FALSE(g.vertices()[k2].edges(g, k2).empty());
        
        auto& e = *g.vertices()[k2].edges_begin(g, k2);
        REQUIRE(e.value == 100);
    }
}

TEST_CASE("undirected_adjacency_list single edge iteration", "[native][edge][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex();
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex();
    auto k2 = v2 - g.begin();
    g.create_edge(k1, k2, 100);
    
    SECTION("edge iteration from source vertex") {
        size_t count = 0;
        for (auto& e : g.vertices()[k1].edges(g, k1)) {
            REQUIRE(e.value == 100);
            ++count;
        }
        REQUIRE(count == 1);
    }
    
    SECTION("edge iteration from target vertex") {
        size_t count = 0;
        for (auto& e : g.vertices()[k2].edges(g, k2)) {
            REQUIRE(e.value == 100);
            ++count;
        }
        REQUIRE(count == 1);
    }
    
    SECTION("global edge iteration") {
        size_t count = 0;
        for (auto v_it = g.begin(); v_it != g.end(); ++v_it) {
            for (auto& e : (*v_it).edges(g, v_it - g.begin())) {
                ++count;
            }
        }
        // Each edge appears in both vertices' lists, but edges_size() counts once
        REQUIRE(count == 2); // Edge appears twice (once in each vertex's list)
        REQUIRE(g.edges_size() == 1); // But is counted as one unique edge
    }
}

// =============================================================================
// Category 6: Multiple Edge Operations
// =============================================================================

TEST_CASE("undirected_adjacency_list multiple edges from one vertex", "[native][edge][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto v3 = g.create_vertex(30);
    auto k3 = v3 - g.begin();
    
    g.create_edge(k1, k2, 100);
    g.create_edge(k1, k3, 200);
    
    SECTION("source vertex has two edges") {
        REQUIRE(g.vertices()[k1].edges_size() == 2);
        
        vector<int> edge_values;
        for (auto& e : g.vertices()[k1].edges(g, k1)) {
            edge_values.push_back(e.value);
        }
        REQUIRE(edge_values.size() == 2);
        // Order may vary
        REQUIRE((edge_values[0] == 100 || edge_values[0] == 200));
        REQUIRE((edge_values[1] == 100 || edge_values[1] == 200));
        REQUIRE(edge_values[0] != edge_values[1]);
    }
    
    SECTION("graph reports correct edge count") {
        REQUIRE(g.edges_size() == 2);
    }
}

TEST_CASE("undirected_adjacency_list triangle graph", "[native][edge][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex();
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex();
    auto k2 = v2 - g.begin();
    auto v3 = g.create_vertex();
    auto k3 = v3 - g.begin();
    
    g.create_edge(k1, k2, 10);
    g.create_edge(k2, k3, 20);
    g.create_edge(k3, k1, 30);
    
    SECTION("all vertices have degree 2") {
        REQUIRE(g.vertices()[k1].edges_size() == 2);
        REQUIRE(g.vertices()[k2].edges_size() == 2);
        REQUIRE(g.vertices()[k3].edges_size() == 2);
    }
    
    SECTION("graph has 3 edges") {
        REQUIRE(g.edges_size() == 3);
    }
}

TEST_CASE("undirected_adjacency_list complete graph K4", "[native][edge][basic]") {
    undirected_adjacency_list<> g;
    vector<size_t> keys;
    
    // Create 4 vertices
    for (int i = 0; i < 4; ++i) {
        auto v = g.create_vertex();
        keys.push_back(v - g.begin());
    }
    
    // Create all edges (complete graph)
    for (size_t i = 0; i < keys.size(); ++i) {
        for (size_t j = i + 1; j < keys.size(); ++j) {
            g.create_edge(keys[i], keys[j]);
        }
    }
    
    SECTION("graph has 4 vertices") {
        REQUIRE(g.vertices().size() == 4);
    }
    
    SECTION("graph has 6 edges (C(4,2) = 6)") {
        REQUIRE(g.edges_size() == 6);
    }
    
    SECTION("all vertices have degree 3") {
        for (auto k : keys) {
            REQUIRE(g.vertices()[k].edges_size() == 3);
        }
    }
}

// =============================================================================
// Category 7: Edge Removal Operations
// =============================================================================

TEST_CASE("undirected_adjacency_list erase single edge", "[native][edge][erase][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto e_it = g.create_edge(k1, k2, 100);
    
    REQUIRE(g.edges_size() == 1);
    
    // Erase the edge
    g.vertices()[k1].erase_edge(g, e_it);
    
    SECTION("edge is removed") {
        REQUIRE(g.edges_size() == 0);
    }
    
    SECTION("vertices have no edges") {
        REQUIRE(g.vertices()[k1].edges_size() == 0);
        REQUIRE(g.vertices()[k2].edges_size() == 0);
    }
}

TEST_CASE("undirected_adjacency_list erase one edge from multi-edge vertex", "[native][edge][erase][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto v3 = g.create_vertex(30);
    auto k3 = v3 - g.begin();
    
    auto e12 = g.create_edge(k1, k2, 100);
    auto e13 = g.create_edge(k1, k3, 200);
    
    REQUIRE(g.edges_size() == 2);
    REQUIRE(g.vertices()[k1].edges_size() == 2);
    
    // Erase one edge
    g.vertices()[k1].erase_edge(g, e12);
    
    SECTION("one edge remains") {
        REQUIRE(g.edges_size() == 1);
    }
    
    SECTION("source vertex has one edge") {
        REQUIRE(g.vertices()[k1].edges_size() == 1);
        auto& remaining = *g.vertices()[k1].edges_begin(g, k1);
        REQUIRE(remaining.value == 200);
    }
    
    SECTION("removed target has no edges") {
        REQUIRE(g.vertices()[k2].edges_size() == 0);
    }
    
    SECTION("remaining target has one edge") {
        REQUIRE(g.vertices()[k3].edges_size() == 1);
    }
}

// =============================================================================
// Category 8: Vertex Value Modification
// =============================================================================

TEST_CASE("undirected_adjacency_list modify vertex values", "[native][vertex][value][basic]") {
    undirected_adjacency_list<int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    
    SECTION("modify vertex value") {
        g.vertices()[k1].value = 100;
        REQUIRE(g.vertices()[k1].value == 100);
    }
    
    SECTION("modify multiple vertex values") {
        g.vertices()[k1].value = 100;
        g.vertices()[k2].value = 200;
        REQUIRE(g.vertices()[k1].value == 100);
        REQUIRE(g.vertices()[k2].value == 200);
    }
}

TEST_CASE("undirected_adjacency_list vertex values with edges", "[native][vertex][value][edge][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    g.create_edge(k1, k2, 100);
    
    SECTION("modify vertex value doesn't affect edges") {
        g.vertices()[k1].value = 999;
        REQUIRE(g.vertices()[k1].value == 999);
        REQUIRE(g.edges_size() == 1);
        
        auto& e = *g.vertices()[k1].edges_begin(g, k1);
        REQUIRE(e.value == 100);
    }
}

// =============================================================================
// Category 9: Edge Value Modification
// =============================================================================

TEST_CASE("undirected_adjacency_list modify edge values", "[native][edge][value][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    auto k1 = v1 - g.begin();
    auto v2 = g.create_vertex(20);
    auto k2 = v2 - g.begin();
    auto e_it = g.create_edge(k1, k2, 100);
    
    SECTION("modify edge value from source vertex") {
        (*e_it).value = 999;
        REQUIRE((*e_it).value == 999);
    }
    
    SECTION("modified edge value visible from target vertex") {
        (*e_it).value = 999;
        
        auto& e_from_target = *g.vertices()[k2].edges_begin(g, k2);
        REQUIRE(e_from_target.value == 999);
    }
}

// =============================================================================
// Category 10: Self-Loop Behavior
// =============================================================================

TEST_CASE("undirected_adjacency_list self-loop creation", "[native][edge][self-loop][basic]") {
    undirected_adjacency_list<int, int> g;
    auto v1 = g.create_vertex(10);
    
    auto k1 = v1 - g.begin();
    auto e_it = g.create_edge(k1, k1, 100);
    
    SECTION("self-loop is created") {
        REQUIRE(g.edges_size() == 1);
        REQUIRE(e_it != g.vertices()[k1].edges_end(g, k1));
    }
    
    SECTION("self-loop appears in vertex's edge list") {
        // Self-loop may appear once or twice depending on implementation
        REQUIRE(g.vertices()[k1].edges_size() >= 1);
    }
}

// =============================================================================
// Category 11: Graph Value Operations
// =============================================================================

TEST_CASE("undirected_adjacency_list graph value access", "[native][graph-value][basic]") {
    SECTION("graph value with int") {
        undirected_adjacency_list<int, int, int> g(42);
        REQUIRE(g.graph_value() == 42);
    }
    
    SECTION("graph value modification") {
        undirected_adjacency_list<int, int, int> g(42);
        g.graph_value() = 100;
        REQUIRE(g.graph_value() == 100);
    }
    
    SECTION("graph value with string") {
        undirected_adjacency_list<string, string, string> g(string("test"));
        REQUIRE(g.graph_value() == "test");
        g.graph_value() = string("modified");
        REQUIRE(g.graph_value() == "modified");
    }
}
