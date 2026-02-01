/**
 * @file test_dfs.cpp
 * @brief Comprehensive tests for DFS views
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/dfs.hpp>
#include <graph/views/vertexlist.hpp>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <set>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Basic DFS Traversal Order
// =============================================================================

TEST_CASE("vertices_dfs - basic traversal order", "[dfs][vertices]") {
    //     0
    //    / \
    //   1   2
    //  / \
    // 3   4
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},     // 0 -> 1, 2
        {3, 4},     // 1 -> 3, 4
        {},         // 2 (leaf)
        {},         // 3 (leaf)
        {}          // 4 (leaf)
    };

    SECTION("DFS from vertex 0") {
        std::vector<int> visited_order;
        
        for (auto [v] : vertices_dfs(g, 0)) {
            visited_order.push_back(static_cast<int>(vertex_id(g, v)));
        }
        
        // DFS should visit in depth-first order
        // Starting at 0, go to 1, then 3, backtrack to 1, go to 4, backtrack to 0, go to 2
        REQUIRE(visited_order.size() == 5);
        REQUIRE(visited_order[0] == 0);  // Start at seed
        
        // All vertices visited
        std::set<int> visited_set(visited_order.begin(), visited_order.end());
        REQUIRE(visited_set == std::set<int>{0, 1, 2, 3, 4});
    }

    SECTION("DFS from leaf vertex") {
        std::vector<int> visited_order;
        
        for (auto [v] : vertices_dfs(g, 3)) {
            visited_order.push_back(static_cast<int>(vertex_id(g, v)));
        }
        
        // Only vertex 3 should be visited (no outgoing edges)
        REQUIRE(visited_order == std::vector<int>{3});
    }
}

// =============================================================================
// Test 2: Structured Bindings
// =============================================================================

TEST_CASE("vertices_dfs - structured bindings", "[dfs][vertices][bindings]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},
        {2},
        {}
    };

    SECTION("structured binding [v]") {
        std::vector<int> ids;
        
        for (auto [v] : vertices_dfs(g, 0)) {
            ids.push_back(static_cast<int>(vertex_id(g, v)));
        }
        
        REQUIRE(ids.size() == 3);
    }

    SECTION("structured binding [v, val] with value function") {
        auto dfs = vertices_dfs(g, 0, [&g](auto v) { 
            return vertex_id(g, v) * 10; 
        });
        
        std::vector<std::pair<int, int>> results;
        for (auto [v, val] : dfs) {
            results.emplace_back(static_cast<int>(vertex_id(g, v)), val);
        }
        
        REQUIRE(results.size() == 3);
        for (auto& [id, val] : results) {
            REQUIRE(val == id * 10);
        }
    }
}

// =============================================================================
// Test 3: Visited Tracking (No Revisits)
// =============================================================================

TEST_CASE("vertices_dfs - visited tracking", "[dfs][vertices][visited]") {
    // Graph with cycle: 0 -> 1 -> 2 -> 0
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1},        // 0 -> 1
        {2},        // 1 -> 2
        {0}         // 2 -> 0 (back edge)
    };

    std::vector<int> visited_order;
    
    for (auto [v] : vertices_dfs(g, 0)) {
        visited_order.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    // Each vertex visited exactly once despite cycle
    REQUIRE(visited_order.size() == 3);
    std::set<int> visited_set(visited_order.begin(), visited_order.end());
    REQUIRE(visited_set == std::set<int>{0, 1, 2});
}

// =============================================================================
// Test 4: Value Function Types
// =============================================================================

TEST_CASE("vertices_dfs - value function types", "[dfs][vertices][vvf]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},
        {},
        {}
    };

    SECTION("returning int") {
        auto dfs = vertices_dfs(g, 0, [&g](auto v) { 
            return static_cast<int>(vertex_id(g, v)); 
        });
        
        int sum = 0;
        for (auto [v, val] : dfs) {
            sum += val;
        }
        REQUIRE(sum == 0 + 1 + 2);
    }

    SECTION("returning string") {
        auto dfs = vertices_dfs(g, 0, [&g](auto v) { 
            return "v" + std::to_string(vertex_id(g, v)); 
        });
        
        std::vector<std::string> names;
        for (auto [v, name] : dfs) {
            names.push_back(name);
        }
        
        REQUIRE(names.size() == 3);
        REQUIRE(std::find(names.begin(), names.end(), "v0") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "v1") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "v2") != names.end());
    }

    SECTION("capturing lambda") {
        int multiplier = 5;
        auto dfs = vertices_dfs(g, 0, [&g, multiplier](auto v) { 
            return static_cast<int>(vertex_id(g, v)) * multiplier; 
        });
        
        std::vector<int> values;
        for (auto [v, val] : dfs) {
            values.push_back(val);
        }
        
        REQUIRE(values.size() == 3);
        // Values are id * 5
        std::set<int> value_set(values.begin(), values.end());
        REQUIRE(value_set == std::set<int>{0, 5, 10});
    }
}

// =============================================================================
// Test 5: Depth and Size Accessors
// =============================================================================

TEST_CASE("vertices_dfs - depth and size accessors", "[dfs][vertices][accessors]") {
    //     0
    //    /|\
    //   1 2 3
    //   |
    //   4
    //   |
    //   5
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2, 3},  // 0 -> 1, 2, 3
        {4},        // 1 -> 4
        {},         // 2 (leaf)
        {},         // 3 (leaf)
        {5},        // 4 -> 5
        {}          // 5 (leaf)
    };

    auto dfs = vertices_dfs(g, 0);
    
    // Before iteration
    REQUIRE(dfs.depth() == 1);  // seed is on stack
    REQUIRE(dfs.size() == 0);   // no vertices counted yet
    
    // Iterate
    std::vector<int> visited;
    for (auto [v] : dfs) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    // After full iteration
    REQUIRE(visited.size() == 6);
    REQUIRE(dfs.size() == 5);  // All vertices except seed are counted by advance()
}

// =============================================================================
// Test 6: Graph Topologies
// =============================================================================

TEST_CASE("vertices_dfs - tree topology", "[dfs][vertices][topology]") {
    // Binary tree
    //       0
    //      / \
    //     1   2
    //    / \
    //   3   4
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},
        {3, 4},
        {},
        {},
        {}
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    REQUIRE(visited.size() == 5);
    REQUIRE(visited[0] == 0);  // Start
    // DFS goes deep first: 0 -> 1 -> 3, backtrack to 1 -> 4, backtrack to 0 -> 2
}

TEST_CASE("vertices_dfs - cycle topology", "[dfs][vertices][topology]") {
    // Ring: 0 -> 1 -> 2 -> 3 -> 0
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1},
        {2},
        {3},
        {0}
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    // Should visit all 4 vertices exactly once
    REQUIRE(visited.size() == 4);
    std::set<int> visited_set(visited.begin(), visited.end());
    REQUIRE(visited_set == std::set<int>{0, 1, 2, 3});
}

TEST_CASE("vertices_dfs - DAG topology", "[dfs][vertices][topology]") {
    // Diamond DAG
    //     0
    //    / \
    //   1   2
    //    \ /
    //     3
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},
        {3},
        {3},
        {}
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    // Vertex 3 visited only once even though reachable from both 1 and 2
    REQUIRE(visited.size() == 4);
    REQUIRE(std::count(visited.begin(), visited.end(), 3) == 1);
}

TEST_CASE("vertices_dfs - disconnected graph", "[dfs][vertices][topology]") {
    // Two components: 0-1-2 and 3-4
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1},
        {2},
        {},
        {4},
        {}
    };

    SECTION("DFS from component 1") {
        std::vector<int> visited;
        for (auto [v] : vertices_dfs(g, 0)) {
            visited.push_back(static_cast<int>(vertex_id(g, v)));
        }
        
        // Only vertices 0, 1, 2 reachable from 0
        REQUIRE(visited.size() == 3);
        std::set<int> visited_set(visited.begin(), visited.end());
        REQUIRE(visited_set == std::set<int>{0, 1, 2});
    }

    SECTION("DFS from component 2") {
        std::vector<int> visited;
        for (auto [v] : vertices_dfs(g, 3)) {
            visited.push_back(static_cast<int>(vertex_id(g, v)));
        }
        
        // Only vertices 3, 4 reachable from 3
        REQUIRE(visited.size() == 2);
        std::set<int> visited_set(visited.begin(), visited.end());
        REQUIRE(visited_set == std::set<int>{3, 4});
    }
}

// =============================================================================
// Test 7: Empty Graph and Single Vertex
// =============================================================================

TEST_CASE("vertices_dfs - single vertex graph", "[dfs][vertices][edge_cases]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {}  // Single vertex with no edges
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    REQUIRE(visited == std::vector<int>{0});
}

TEST_CASE("vertices_dfs - vertex with no outgoing edges", "[dfs][vertices][edge_cases]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1},
        {},
        {0}  // Has incoming but no reachable from it
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 1)) {  // Start from leaf
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    REQUIRE(visited == std::vector<int>{1});
}

// =============================================================================
// Test 8: search_view Concept
// =============================================================================

TEST_CASE("vertices_dfs - search_view concept", "[dfs][vertices][concepts]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1}, {}};
    
    auto dfs = vertices_dfs(g, 0);
    
    STATIC_REQUIRE(search_view<decltype(dfs)>);
    
    // Verify accessors exist and return correct types
    REQUIRE(dfs.cancel() == cancel_search::continue_search);
    REQUIRE(dfs.depth() >= 0);
    REQUIRE(dfs.size() >= 0);
}

// =============================================================================
// Test 9: Range Concepts
// =============================================================================

TEST_CASE("vertices_dfs - range concepts", "[dfs][vertices][concepts]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1}, {}};
    
    auto dfs = vertices_dfs(g, 0);
    
    STATIC_REQUIRE(std::ranges::input_range<decltype(dfs)>);
    STATIC_REQUIRE(std::ranges::view<decltype(dfs)>);
    
    // DFS is input range (not forward) due to shared state
    // Cannot iterate twice independently
}

// =============================================================================
// Test 10: vertex_info Type Verification
// =============================================================================

TEST_CASE("vertices_dfs - vertex_info type verification", "[dfs][vertices][types]") {
    using Graph = std::vector<std::vector<int>>;
    using VertexType = vertex_t<Graph>;

    SECTION("no value function") {
        using ViewType = vertices_dfs_view<Graph, void, std::allocator<bool>>;
        using InfoType = typename ViewType::info_type;
        
        STATIC_REQUIRE(std::is_same_v<InfoType, vertex_info<void, VertexType, void>>);
    }

    SECTION("with value function") {
        using VVF = int(*)(VertexType);
        using ViewType = vertices_dfs_view<Graph, VVF, std::allocator<bool>>;
        using InfoType = typename ViewType::info_type;
        
        STATIC_REQUIRE(std::is_same_v<InfoType, vertex_info<void, VertexType, int>>);
    }
}

// =============================================================================
// Test 11: Deque-based Graph
// =============================================================================

TEST_CASE("vertices_dfs - deque-based graph", "[dfs][vertices][deque]") {
    using Graph = std::deque<std::deque<int>>;
    Graph g = {
        {1, 2},
        {},
        {}
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    REQUIRE(visited.size() == 3);
}

// =============================================================================
// Test 12: Weighted Graph
// =============================================================================

TEST_CASE("vertices_dfs - weighted graph", "[dfs][vertices][weighted]") {
    using Graph = std::vector<std::vector<std::pair<int, double>>>;
    Graph g = {
        {{1, 1.5}, {2, 2.5}},
        {{2, 3.5}},
        {}
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    REQUIRE(visited.size() == 3);
    std::set<int> visited_set(visited.begin(), visited.end());
    REQUIRE(visited_set == std::set<int>{0, 1, 2});
}

// =============================================================================
// Test 13: Large Graph Performance
// =============================================================================

TEST_CASE("vertices_dfs - large linear graph", "[dfs][vertices][performance]") {
    // Linear chain: 0 -> 1 -> 2 -> ... -> 999
    using Graph = std::vector<std::vector<int>>;
    Graph g(1000);
    for (int i = 0; i < 999; ++i) {
        g[i].push_back(i + 1);
    }

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    REQUIRE(visited.size() == 1000);
    
    // Verify order: should be 0, 1, 2, ..., 999
    for (int i = 0; i < 1000; ++i) {
        REQUIRE(visited[i] == i);
    }
}

// =============================================================================
// Test 14: DFS Pre-order Property
// =============================================================================

TEST_CASE("vertices_dfs - pre-order property", "[dfs][vertices][order]") {
    // Verify parent is visited before children
    //     0
    //    / \
    //   1   2
    //  /|   |\
    // 3 4   5 6
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},     // 0
        {3, 4},     // 1
        {5, 6},     // 2
        {},         // 3
        {},         // 4
        {},         // 5
        {}          // 6
    };

    std::vector<int> visited;
    for (auto [v] : vertices_dfs(g, 0)) {
        visited.push_back(static_cast<int>(vertex_id(g, v)));
    }
    
    // Find positions
    auto pos = [&visited](int id) {
        return std::find(visited.begin(), visited.end(), id) - visited.begin();
    };
    
    // Parent visited before children
    REQUIRE(pos(0) < pos(1));
    REQUIRE(pos(0) < pos(2));
    REQUIRE(pos(1) < pos(3));
    REQUIRE(pos(1) < pos(4));
    REQUIRE(pos(2) < pos(5));
    REQUIRE(pos(2) < pos(6));
}
