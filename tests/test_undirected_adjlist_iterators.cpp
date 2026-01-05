#include <catch2/catch_test_macros.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include <vector>
#include <algorithm>

using namespace graph::container;
using std::vector;

// =============================================================================
// Phase 4.2: Iterator Tests
// Comprehensive tests for all iterator types in undirected_adjacency_list
// =============================================================================

TEST_CASE("vertex_iterator - forward iteration", "[iterators][vertex]") {
    undirected_adjacency_list<int> g;
    auto v1_it = g.create_vertex(10);
    auto v2_it = g.create_vertex(20);
    auto v3_it = g.create_vertex(30);
    
    SECTION("iterate through all vertices") {
        vector<int> values;
        for (auto it = g.begin(); it != g.end(); ++it) {
            values.push_back(it->value);
        }
        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }
    
    SECTION("pre-increment") {
        auto it = g.begin();
        REQUIRE(it->value == 10);
        ++it;
        REQUIRE(it->value == 20);
        ++it;
        REQUIRE(it->value == 30);
    }
    
    SECTION("post-increment") {
        auto it = g.begin();
        auto old = it++;
        REQUIRE(old->value == 10);
        REQUIRE(it->value == 20);
    }
}

TEST_CASE("const_vertex_iterator - forward iteration", "[iterators][vertex][const]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    const auto& cg = g;
    
    SECTION("const iteration") {
        vector<int> values;
        for (auto it = cg.begin(); it != cg.end(); ++it) {
            values.push_back(it->value);
        }
        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 10);
    }
    
    SECTION("range-based for loop") {
        vector<int> values;
        for (const auto& v : cg.vertices()) {
            values.push_back(v.value);
        }
        REQUIRE(values.size() == 3);
    }
}

TEST_CASE("vertex_iterator - empty graph", "[iterators][vertex][edge_cases]") {
    undirected_adjacency_list<int> g;
    
    REQUIRE(g.begin() == g.end());
    
    int count = 0;
    for (auto it = g.begin(); it != g.end(); ++it) {
        ++count;
    }
    REQUIRE(count == 0);
}

TEST_CASE("vertex_iterator - single vertex", "[iterators][vertex][edge_cases]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(42);
    
    SECTION("iteration visits one vertex") {
        int count = 0;
        for (auto it = g.begin(); it != g.end(); ++it) {
            ++count;
        }
        REQUIRE(count == 1);
    }
}

TEST_CASE("vertex_iterator - equality", "[iterators][vertex]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    
    SECTION("same position") {
        auto it1 = g.begin();
        auto it2 = g.begin();
        REQUIRE(it1 == it2);
    }
    
    SECTION("different positions") {
        auto it1 = g.begin();
        auto it2 = g.begin();
        ++it2;
        REQUIRE(it1 != it2);
    }
    
    SECTION("end iterator") {
        auto it = g.begin();
        ++it;
        ++it;
        REQUIRE(it == g.end());
    }
}

TEST_CASE("edge_iterator - forward iteration through graph", "[iterators][edge]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v1_it = g.create_vertex();
    auto k1 = v1_it - g.begin();
    auto v2_it = g.create_vertex();
    auto k2 = v2_it - g.begin();
    auto v3_it = g.create_vertex();
    auto k3 = v3_it - g.begin();
    
    g.create_edge(k1, k2, 100);
    g.create_edge(k2, k3, 200);
    
    SECTION("iterate all edges") {
        vector<int> values;
        for (auto it = g.edges_begin(); it != g.edges_end(); ++it) {
            values.push_back(it->value);
        }
        // Each undirected edge appears twice
        REQUIRE(values.size() == 4);
    }
    
    SECTION("find specific edge value") {
        bool found_100 = false;
        for (auto it = g.edges_begin(); it != g.edges_end(); ++it) {
            if (it->value == 100) {
                found_100 = true;
            }
        }
        REQUIRE(found_100);
    }
}

TEST_CASE("edge_iterator - empty graph", "[iterators][edge][edge_cases]") {
    undirected_adjacency_list<empty_value, int> g;
    
    REQUIRE(g.edges_begin() == g.edges_end());
}

TEST_CASE("edge_iterator - graph with vertices but no edges", "[iterators][edge][edge_cases]") {
    undirected_adjacency_list<empty_value, int> g;
    g.create_vertex();
    g.create_vertex();
    
    REQUIRE(g.edges_begin() == g.edges_end());
}

TEST_CASE("edge_iterator - single edge", "[iterators][edge][edge_cases]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v1_it = g.create_vertex();
    auto k1 = v1_it - g.begin();
    auto v2_it = g.create_vertex();
    auto k2 = v2_it - g.begin();
    
    g.create_edge(k1, k2, 100);
    
    int count = 0;
    for (auto it = g.edges_begin(); it != g.edges_end(); ++it) {
        ++count;
    }
    REQUIRE(count == 2);  // Undirected edge appears twice
}

TEST_CASE("vertex_edge_iterator - edges from specific vertex", "[iterators][vertex_edge]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v0_it = g.create_vertex();
    auto k0 = v0_it - g.begin();
    auto v1_it = g.create_vertex();
    auto k1 = v1_it - g.begin();
    auto v2_it = g.create_vertex();
    auto k2 = v2_it - g.begin();
    auto v3_it = g.create_vertex();
    auto k3 = v3_it - g.begin();
    
    g.create_edge(k0, k1, 10);
    g.create_edge(k0, k2, 20);
    g.create_edge(k0, k3, 30);
    
    SECTION("iterate edges from vertex 0") {
        auto& v0 = g.vertices()[k0];
        vector<int> values;
        for (auto& e : v0.edges(g, k0)) {
            values.push_back(e.value);
        }
        REQUIRE(values.size() == 3);
        REQUIRE(std::find(values.begin(), values.end(), 10) != values.end());
        REQUIRE(std::find(values.begin(), values.end(), 20) != values.end());
        REQUIRE(std::find(values.begin(), values.end(), 30) != values.end());
    }
    
    SECTION("vertex with one edge") {
        auto& v1 = g.vertices()[k1];
        int count = 0;
        for (auto& e : v1.edges(g, k1)) {
            ++count;
        }
        REQUIRE(count == 1);
    }
}

TEST_CASE("vertex_edge_iterator - bidirectional", "[iterators][vertex_edge]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v0_it = g.create_vertex();
    auto k0 = v0_it - g.begin();
    auto v1_it = g.create_vertex();
    auto k1 = v1_it - g.begin();
    auto v2_it = g.create_vertex();
    auto k2 = v2_it - g.begin();
    
    g.create_edge(k0, k1, 10);
    g.create_edge(k0, k2, 20);
    
    auto& v0 = g.vertices()[k0];
    
    SECTION("forward then backward") {
        auto edges_range = v0.edges(g, k0);
        auto it = edges_range.begin();
        ++it;
        REQUIRE(it->value == 20);
        --it;
        REQUIRE(it->value == 10);
    }
    
    SECTION("post-decrement") {
        auto edges_range = v0.edges(g, k0);
        auto it = edges_range.begin();
        ++it;
        auto old = it--;
        REQUIRE(old->value == 20);
        REQUIRE(it->value == 10);
    }
}

TEST_CASE("vertex_edge_iterator - empty edge list", "[iterators][vertex_edge][edge_cases]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v_it = g.create_vertex();
    auto k = v_it - g.begin();
    
    auto& v = g.vertices()[k];
    auto edges_range = v.edges(g, k);
    
    REQUIRE(edges_range.begin() == edges_range.end());
}

TEST_CASE("const_vertex_edge_iterator", "[iterators][vertex_edge][const]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v0_it = g.create_vertex();
    auto k0 = v0_it - g.begin();
    auto v1_it = g.create_vertex();
    auto k1 = v1_it - g.begin();
    
    g.create_edge(k0, k1, 100);
    
    const auto& cg = g;
    
    SECTION("const iteration") {
        auto& v0 = cg.vertices()[k0];
        int count = 0;
        for (auto& e : v0.edges(cg, k0)) {
            ++count;
        }
        REQUIRE(count == 1);
    }
}

TEST_CASE("vertex_vertex_iterator - adjacent vertices", "[iterators][vertex_vertex]") {
    undirected_adjacency_list<int> g;
    auto v0_it = g.create_vertex(10);
    auto k0 = v0_it - g.begin();
    auto v1_it = g.create_vertex(20);
    auto k1 = v1_it - g.begin();
    auto v2_it = g.create_vertex(30);
    auto k2 = v2_it - g.begin();
    
    g.create_edge(k0, k1);
    g.create_edge(k0, k2);
    
    SECTION("iterate adjacent vertices") {
        auto& v0 = g.vertices()[k0];
        vector<unsigned int> keys;
        auto edges_range = v0.edges(g, k0);
        for (auto it = edges_range.begin(); it != edges_range.end(); ++it) {
            keys.push_back(it->target_vertex_key(g));
        }
        REQUIRE(keys.size() == 2);
        REQUIRE(std::find(keys.begin(), keys.end(), k1) != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), k2) != keys.end());
    }
    
    SECTION("access adjacent vertex values") {
        auto& v0 = g.vertices()[k0];
        vector<int> values;
        auto edges_range = v0.edges(g, k0);
        for (auto it = edges_range.begin(); it != edges_range.end(); ++it) {
            auto target_key = it->target_vertex_key(g);
            values.push_back(g.vertices()[target_key].value);
        }
        REQUIRE(values.size() == 2);
        REQUIRE(std::find(values.begin(), values.end(), 20) != values.end());
        REQUIRE(std::find(values.begin(), values.end(), 30) != values.end());
    }
}

TEST_CASE("iterators with std::distance", "[iterators][algorithms]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    auto count = std::distance(g.begin(), g.end());
    REQUIRE(count == 3);
}

TEST_CASE("iterators with std::find_if", "[iterators][algorithms]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    
    auto it = std::find_if(g.begin(), g.end(), [](const auto& v) {
        return v.value == 20;
    });
    
    REQUIRE(it != g.end());
    REQUIRE(it->value == 20);
}

TEST_CASE("iterators with std::count_if", "[iterators][algorithms]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    g.create_vertex(30);
    g.create_vertex(20);
    
    auto count = std::count_if(g.begin(), g.end(), [](const auto& v) {
        return v.value == 20;
    });
    
    REQUIRE(count == 2);
}

TEST_CASE("iterator copy and assignment", "[iterators][edge_cases]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(10);
    g.create_vertex(20);
    
    SECTION("copy constructor") {
        auto it1 = g.begin();
        auto it2 = it1;
        REQUIRE(it1 == it2);
        REQUIRE(it2->value == 10);
    }
    
    SECTION("copy assignment") {
        auto it1 = g.begin();
        auto it2 = g.end();
        it2 = it1;
        REQUIRE(it1 == it2);
    }
}

TEST_CASE("iterator dereference operators", "[iterators]") {
    undirected_adjacency_list<int> g;
    g.create_vertex(42);
    
    SECTION("operator*") {
        auto it = g.begin();
        auto& vertex = *it;
        REQUIRE(vertex.value == 42);
    }
    
    SECTION("operator->") {
        auto it = g.begin();
        REQUIRE(it->value == 42);
    }
}

TEST_CASE("vertex_edge_iterator with parallel edges", "[iterators][parallel_edges]") {
    undirected_adjacency_list<empty_value, int> g;
    auto v0_it = g.create_vertex();
    auto k0 = v0_it - g.begin();
    auto v1_it = g.create_vertex();
    auto k1 = v1_it - g.begin();
    
    g.create_edge(k0, k1, 100);
    g.create_edge(k0, k1, 200);
    g.create_edge(k0, k1, 300);
    
    SECTION("all parallel edges visible") {
        auto& v0 = g.vertices()[k0];
        vector<int> values;
        for (auto& e : v0.edges(g, k0)) {
            values.push_back(e.value);
        }
        REQUIRE(values.size() == 3);
        REQUIRE(std::find(values.begin(), values.end(), 100) != values.end());
        REQUIRE(std::find(values.begin(), values.end(), 200) != values.end());
        REQUIRE(std::find(values.begin(), values.end(), 300) != values.end());
    }
}
