/**
 * @file test_neighbors.cpp
 * @brief Comprehensive tests for neighbors view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/views/vertexlist.hpp>
#include <vector>
#include <deque>
#include <map>
#include <string>
#include <algorithm>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Vertex with No Neighbors
// =============================================================================

TEST_CASE("neighbors - vertex with no neighbors", "[neighbors][empty]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {},       // vertex 0 - no edges
        {0},      // vertex 1 - edge to 0
        {0, 1}    // vertex 2 - edges to 0 and 1
    };

    SECTION("no value function - empty iteration") {
        auto v0 = vertex_t<Graph>{0};  // vertex 0 has no neighbors
        auto nlist = neighbors(g, v0);
        
        REQUIRE(nlist.begin() == nlist.end());
        REQUIRE(nlist.size() == 0);
    }

    SECTION("with value function - empty iteration") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0, [](auto /*v*/) { return 42; });
        
        REQUIRE(nlist.begin() == nlist.end());
    }
}

// =============================================================================
// Test 2: Single Neighbor
// =============================================================================

TEST_CASE("neighbors - vertex with single neighbor", "[neighbors][single]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1},      // vertex 0 -> neighbor 1
        {}        // vertex 1 - no neighbors
    };

    SECTION("no value function") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0);
        
        REQUIRE(nlist.size() == 1);
        
        auto it = nlist.begin();
        REQUIRE(it != nlist.end());
        
        auto ni = *it;
        // neighbor_info<void, false, vertex_t<G>, void> has 'vertex' member
        REQUIRE(ni.vertex.vertex_id() == 1);
        
        ++it;
        REQUIRE(it == nlist.end());
    }

    SECTION("with value function") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0, [](auto v) { 
            return v.vertex_id() * 10; 
        });
        
        REQUIRE(nlist.size() == 1);
        
        auto ni = *nlist.begin();
        REQUIRE(ni.vertex.vertex_id() == 1);
        REQUIRE(ni.value == 10);  // vertex_id(1) * 10
    }
}

// =============================================================================
// Test 3: Multiple Neighbors
// =============================================================================

TEST_CASE("neighbors - vertex with multiple neighbors", "[neighbors][multiple]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2, 3},  // vertex 0 -> neighbors 1, 2, 3
        {2, 3},     // vertex 1 -> neighbors 2, 3
        {3},        // vertex 2 -> neighbor 3
        {}          // vertex 3 - no neighbors
    };

    SECTION("no value function - iteration") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0);
        
        REQUIRE(nlist.size() == 3);
        
        std::vector<std::size_t> neighbor_ids;
        for (auto ni : nlist) {
            neighbor_ids.push_back(ni.vertex.vertex_id());
        }
        
        REQUIRE(neighbor_ids == std::vector<std::size_t>{1, 2, 3});
    }

    SECTION("with value function") {
        auto v1 = vertex_t<Graph>{1};
        auto nlist = neighbors(g, v1, [](auto v) { 
            return static_cast<int>(v.vertex_id() * 100); 
        });
        
        std::vector<int> values;
        for (auto ni : nlist) {
            values.push_back(ni.value);
        }
        
        REQUIRE(values == std::vector<int>{200, 300});
    }

    SECTION("structured binding - no value function") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0);
        
        std::vector<std::size_t> neighbor_ids;
        for (auto [v] : nlist) {
            neighbor_ids.push_back(v.vertex_id());
        }
        REQUIRE(neighbor_ids == std::vector<std::size_t>{1, 2, 3});
    }

    SECTION("structured binding - with value function") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0, [](auto v) { 
            return v.vertex_id() + 100;
        });
        
        std::vector<std::size_t> neighbor_ids;
        std::vector<std::size_t> values;
        for (auto [v, val] : nlist) {
            neighbor_ids.push_back(v.vertex_id());
            values.push_back(val);
        }
        
        REQUIRE(neighbor_ids == std::vector<std::size_t>{1, 2, 3});
        REQUIRE(values == std::vector<std::size_t>{101, 102, 103});
    }
}

// =============================================================================
// Test 4: Value Function Types
// =============================================================================

TEST_CASE("neighbors - value function types", "[neighbors][vvf]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},   // vertex 0 -> neighbors 1, 2
        {},
        {}
    };
    auto v0 = vertex_t<Graph>{0};

    SECTION("returning string") {
        auto nlist = neighbors(g, v0, [](auto v) { 
            return "neighbor_" + std::to_string(v.vertex_id()); 
        });
        
        std::vector<std::string> names;
        for (auto [v, name] : nlist) {
            names.push_back(name);
        }
        
        REQUIRE(names == std::vector<std::string>{"neighbor_1", "neighbor_2"});
    }

    SECTION("returning double") {
        auto nlist = neighbors(g, v0, [](auto v) { 
            return static_cast<double>(v.vertex_id()) * 1.5; 
        });
        
        std::vector<double> values;
        for (auto [v, val] : nlist) {
            values.push_back(val);
        }
        
        REQUIRE(values[0] == 1.5);
        REQUIRE(values[1] == 3.0);
    }

    SECTION("capturing lambda") {
        std::size_t multiplier = 7;
        auto nlist = neighbors(g, v0, [multiplier](auto v) { 
            return static_cast<int>(v.vertex_id() * multiplier); 
        });
        
        std::vector<int> values;
        for (auto [v, val] : nlist) {
            values.push_back(val);
        }
        
        REQUIRE(values == std::vector<int>{7, 14});
    }
}

// =============================================================================
// Test 5: Vertex Descriptor Access
// =============================================================================

TEST_CASE("neighbors - vertex descriptor access", "[neighbors][descriptor]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2, 3},  // vertex 0 -> neighbors 1, 2, 3
        {},
        {},
        {}
    };
    auto v0 = vertex_t<Graph>{0};

    SECTION("vertex_id access") {
        auto nlist = neighbors(g, v0);
        
        std::vector<std::size_t> ids;
        for (auto [v] : nlist) {
            ids.push_back(v.vertex_id());
        }
        
        REQUIRE(ids == std::vector<std::size_t>{1, 2, 3});
    }

    SECTION("vertex descriptor type") {
        auto nlist = neighbors(g, v0);
        
        for (auto [v] : nlist) {
            // v should be a vertex descriptor
            STATIC_REQUIRE(std::is_same_v<decltype(v), vertex_t<Graph>>);
            (void)v;  // Suppress unused warning
        }
    }
}

// =============================================================================
// Test 6: Weighted Graph (Pair Edges)
// =============================================================================

TEST_CASE("neighbors - weighted graph", "[neighbors][weighted]") {
    // Graph with weighted edges: vector<vector<pair<target, weight>>>
    using Graph = std::vector<std::vector<std::pair<int, double>>>;
    Graph g = {
        {{1, 1.5}, {2, 2.5}},   // vertex 0 -> (1, 1.5), (2, 2.5)
        {{2, 3.5}},              // vertex 1 -> (2, 3.5)
        {}
    };

    SECTION("no value function - neighbor iteration") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0);
        
        REQUIRE(nlist.size() == 2);
        
        std::vector<std::size_t> neighbor_ids;
        for (auto [v] : nlist) {
            neighbor_ids.push_back(v.vertex_id());
        }
        
        REQUIRE(neighbor_ids == std::vector<std::size_t>{1, 2});
    }

    SECTION("value function accessing neighbor properties") {
        auto v0 = vertex_t<Graph>{0};
        // Value function that just returns neighbor ID squared
        auto nlist = neighbors(g, v0, [](auto v) { 
            return v.vertex_id() * v.vertex_id(); 
        });
        
        std::vector<std::size_t> values;
        for (auto [v, val] : nlist) {
            values.push_back(val);
        }
        
        REQUIRE(values[0] == 1);  // 1^2
        REQUIRE(values[1] == 4);  // 2^2
    }
}

// =============================================================================
// Test 7: Range Concepts
// =============================================================================

TEST_CASE("neighbors - range concepts", "[neighbors][concepts]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1, 2}, {}, {}};
    auto v0 = vertex_t<Graph>{0};

    SECTION("no value function") {
        auto nlist = neighbors(g, v0);
        
        STATIC_REQUIRE(std::ranges::input_range<decltype(nlist)>);
        STATIC_REQUIRE(std::ranges::forward_range<decltype(nlist)>);
        STATIC_REQUIRE(std::ranges::sized_range<decltype(nlist)>);
        STATIC_REQUIRE(std::ranges::view<decltype(nlist)>);
    }

    SECTION("with value function") {
        auto nlist = neighbors(g, v0, [](auto v) { return v.vertex_id(); });
        
        STATIC_REQUIRE(std::ranges::input_range<decltype(nlist)>);
        STATIC_REQUIRE(std::ranges::forward_range<decltype(nlist)>);
        STATIC_REQUIRE(std::ranges::sized_range<decltype(nlist)>);
    }
}

// =============================================================================
// Test 8: Iterator Properties
// =============================================================================

TEST_CASE("neighbors - iterator properties", "[neighbors][iterator]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1, 2, 3}, {}, {}, {}};
    auto v0 = vertex_t<Graph>{0};

    SECTION("pre-increment returns reference") {
        auto nlist = neighbors(g, v0);
        auto it = nlist.begin();
        auto& ref = ++it;
        REQUIRE(&ref == &it);
    }

    SECTION("post-increment returns copy") {
        auto nlist = neighbors(g, v0);
        auto it = nlist.begin();
        auto copy = it++;
        REQUIRE(copy != it);
    }

    SECTION("equality comparison") {
        auto nlist = neighbors(g, v0);
        auto it1 = nlist.begin();
        auto it2 = nlist.begin();
        REQUIRE(it1 == it2);
        ++it1;
        REQUIRE(it1 != it2);
    }
}

// =============================================================================
// Test 9: neighbor_info Type Verification
// =============================================================================

TEST_CASE("neighbors - neighbor_info type verification", "[neighbors][types]") {
    using Graph = std::vector<std::vector<int>>;
    using VertexType = vertex_t<Graph>;

    SECTION("no value function - neighbor_info<void, false, vertex_t, void>") {
        using ViewType = neighbors_view<Graph, void>;
        using InfoType = typename ViewType::info_type;
        
        STATIC_REQUIRE(std::is_same_v<InfoType, neighbor_info<void, false, VertexType, void>>);
        
        // Verify neighbor_info members
        STATIC_REQUIRE(std::is_same_v<typename InfoType::source_id_type, void>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, void>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::vertex_type, VertexType>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, void>);
    }

    SECTION("with value function - neighbor_info<void, false, vertex_t, int>") {
        using VVF = int(*)(VertexType);
        using ViewType = neighbors_view<Graph, VVF>;
        using InfoType = typename ViewType::info_type;
        
        STATIC_REQUIRE(std::is_same_v<InfoType, neighbor_info<void, false, VertexType, int>>);
        
        // Verify neighbor_info members
        STATIC_REQUIRE(std::is_same_v<typename InfoType::source_id_type, void>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, void>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::vertex_type, VertexType>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
    }
}

// =============================================================================
// Test 10: std::ranges Algorithms
// =============================================================================

TEST_CASE("neighbors - std::ranges algorithms", "[neighbors][algorithms]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2, 3, 4, 5},  // vertex 0 -> neighbors 1-5
        {},
        {},
        {},
        {},
        {}
    };
    auto v0 = vertex_t<Graph>{0};

    SECTION("std::ranges::distance") {
        auto nlist = neighbors(g, v0);
        REQUIRE(std::ranges::distance(nlist) == 5);
    }

    SECTION("std::ranges::count_if") {
        auto nlist = neighbors(g, v0);
        auto count = std::ranges::count_if(nlist, [](auto ni) {
            return ni.vertex.vertex_id() > 2;
        });
        REQUIRE(count == 3);  // neighbors 3, 4, 5
    }
}

// =============================================================================
// Test 11: Deque-based Graph
// =============================================================================

TEST_CASE("neighbors - deque-based graph", "[neighbors][deque]") {
    using Graph = std::deque<std::deque<int>>;
    Graph g = {
        {1, 2},
        {2},
        {}
    };

    auto v0 = vertex_t<Graph>{0};
    auto nlist = neighbors(g, v0);
    
    std::vector<std::size_t> neighbor_ids;
    for (auto [v] : nlist) {
        neighbor_ids.push_back(v.vertex_id());
    }
    
    REQUIRE(neighbor_ids == std::vector<std::size_t>{1, 2});
}

// =============================================================================
// Test 12: All Vertices Iteration (vertexlist + neighbors)
// =============================================================================

TEST_CASE("neighbors - iterating all vertices", "[neighbors][all]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},      // vertex 0 -> 1, 2
        {2},         // vertex 1 -> 2
        {}           // vertex 2 -> no neighbors
    };

    // Collect all neighbor relationships
    std::vector<std::pair<std::size_t, std::size_t>> all_neighbors;
    
    for (auto [id, v] : vertexlist(g)) {
        auto source_id = id;
        for (auto [neighbor] : neighbors(g, v)) {
            all_neighbors.emplace_back(source_id, neighbor.vertex_id());
        }
    }
    
    REQUIRE(all_neighbors.size() == 3);
    REQUIRE(all_neighbors[0] == std::pair<std::size_t, std::size_t>{0, 1});
    REQUIRE(all_neighbors[1] == std::pair<std::size_t, std::size_t>{0, 2});
    REQUIRE(all_neighbors[2] == std::pair<std::size_t, std::size_t>{1, 2});
}

// =============================================================================
// Test 13: Map-Based Vertex Container (Sparse Vertex IDs)
// =============================================================================

TEST_CASE("neighbors - map vertices vector edges", "[neighbors][map]") {
    // Map-based graphs have sparse, non-contiguous vertex IDs
    using Graph = std::map<int, std::vector<int>>;
    Graph g = {
        {100, {200, 300}},   // vertex 100 -> neighbors 200, 300
        {200, {300}},        // vertex 200 -> neighbor 300
        {300, {}}            // vertex 300 -> no neighbors
    };

    SECTION("iteration over neighbors from sparse vertex") {
        // Get vertex 100
        auto verts = vertices(g);
        auto v100 = *verts.begin();
        REQUIRE(v100.vertex_id() == 100);
        
        auto nlist = neighbors(g, v100);
        
        REQUIRE(nlist.size() == 2);
        
        std::vector<int> neighbor_ids;
        for (auto [v] : nlist) {
            neighbor_ids.push_back(v.vertex_id());
        }
        
        REQUIRE(neighbor_ids == std::vector<int>{200, 300});
    }

    SECTION("empty neighbor list") {
        auto verts = vertices(g);
        auto it = verts.begin();
        std::advance(it, 2);  // Get vertex 300
        auto v300 = *it;
        REQUIRE(v300.vertex_id() == 300);
        
        auto nlist = neighbors(g, v300);
        REQUIRE(nlist.size() == 0);
        REQUIRE(nlist.begin() == nlist.end());
    }

    SECTION("with value function") {
        auto verts = vertices(g);
        auto v100 = *verts.begin();
        
        auto nlist = neighbors(g, v100, [](auto v) {
            return v.vertex_id() - 100;  // Offset from base
        });
        
        std::vector<int> offsets;
        for (auto [v, offset] : nlist) {
            offsets.push_back(offset);
        }
        
        REQUIRE(offsets == std::vector<int>{100, 200});  // 200-100=100, 300-100=200
    }

    SECTION("iterate all vertices and neighbors") {
        std::vector<std::pair<int, int>> all_neighbors;
        
        for (auto [id, v] : vertexlist(g)) {
            auto source_id = id;
            for (auto [neighbor] : neighbors(g, v)) {
                all_neighbors.emplace_back(source_id, neighbor.vertex_id());
            }
        }
        
        REQUIRE(all_neighbors.size() == 3);
        REQUIRE(all_neighbors[0] == std::pair<int, int>{100, 200});
        REQUIRE(all_neighbors[1] == std::pair<int, int>{100, 300});
        REQUIRE(all_neighbors[2] == std::pair<int, int>{200, 300});
    }
}

// =============================================================================
// Test 14: Map-Based Edge Container (Sorted Edges)
// =============================================================================

TEST_CASE("neighbors - vector vertices map edges", "[neighbors][edge_map]") {
    // Edges stored in map (sorted by target, with edge values)
    using Graph = std::vector<std::map<int, double>>;
    Graph g = {
        {{1, 1.5}, {2, 2.5}},   // vertex 0 -> (1, 1.5), (2, 2.5)
        {{2, 3.5}},              // vertex 1 -> (2, 3.5)
        {}                       // vertex 2 -> no neighbors
    };

    SECTION("iteration") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0);
        
        REQUIRE(nlist.size() == 2);
        
        std::vector<std::size_t> neighbor_ids;
        for (auto [v] : nlist) {
            neighbor_ids.push_back(v.vertex_id());
        }
        
        // Map edges are sorted by target_id (key)
        REQUIRE(neighbor_ids == std::vector<std::size_t>{1, 2});
    }

    SECTION("with value function") {
        auto v0 = vertex_t<Graph>{0};
        auto nlist = neighbors(g, v0, [](auto v) {
            return v.vertex_id() * 10;
        });
        
        std::vector<std::size_t> values;
        for (auto [v, val] : nlist) {
            values.push_back(val);
        }
        
        REQUIRE(values == std::vector<std::size_t>{10, 20});
    }

    SECTION("single neighbor vertex") {
        auto v1 = vertex_t<Graph>{1};
        auto nlist = neighbors(g, v1);
        
        REQUIRE(nlist.size() == 1);
        
        auto [v] = *nlist.begin();
        REQUIRE(v.vertex_id() == 2);
    }
}

// =============================================================================
// Test 15: Map Vertices + Map Edges (Fully Sparse Graph)
// =============================================================================

TEST_CASE("neighbors - map vertices map edges", "[neighbors][map][edge_map]") {
    // Both vertices and edges in maps - fully sparse graph
    using Graph = std::map<int, std::map<int, double>>;
    Graph g = {
        {10, {{20, 1.0}, {30, 2.0}}},   // vertex 10 -> (20, 1.0), (30, 2.0)
        {20, {{30, 3.0}}},               // vertex 20 -> (30, 3.0)
        {30, {}}                         // vertex 30 -> no neighbors
    };

    SECTION("iteration") {
        auto verts = vertices(g);
        auto v10 = *verts.begin();
        REQUIRE(v10.vertex_id() == 10);
        
        auto nlist = neighbors(g, v10);
        
        REQUIRE(nlist.size() == 2);
        
        std::vector<int> neighbor_ids;
        for (auto [v] : nlist) {
            neighbor_ids.push_back(v.vertex_id());
        }
        
        REQUIRE(neighbor_ids == std::vector<int>{20, 30});
    }

    SECTION("with value function") {
        auto verts = vertices(g);
        auto v10 = *verts.begin();
        
        auto nlist = neighbors(g, v10, [](auto v) {
            return v.vertex_id() * 2;
        });
        
        std::vector<int> values;
        for (auto [v, val] : nlist) {
            values.push_back(val);
        }
        
        REQUIRE(values == std::vector<int>{40, 60});
    }

    SECTION("all neighbors traversal") {
        std::vector<std::pair<int, int>> all_neighbors;
        
        for (auto [id, v] : vertexlist(g)) {
            auto source_id = id;
            for (auto [neighbor] : neighbors(g, v)) {
                all_neighbors.emplace_back(source_id, neighbor.vertex_id());
            }
        }
        
        REQUIRE(all_neighbors.size() == 3);
        REQUIRE(all_neighbors[0] == std::pair<int, int>{10, 20});
        REQUIRE(all_neighbors[1] == std::pair<int, int>{10, 30});
        REQUIRE(all_neighbors[2] == std::pair<int, int>{20, 30});
    }

    SECTION("neighbor descriptor type correct") {
        auto verts = vertices(g);
        auto v10 = *verts.begin();
        
        for (auto [v] : neighbors(g, v10)) {
            // v should be a vertex_t<Graph>
            STATIC_REQUIRE(std::is_same_v<decltype(v), vertex_t<Graph>>);
            (void)v;  // Suppress unused warning
        }
    }
}
