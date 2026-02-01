/**
 * @file test_vertexlist.cpp
 * @brief Comprehensive tests for vertexlist view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/vertexlist.hpp>

#include <vector>
#include <deque>
#include <map>
#include <string>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// Test 1: Empty Graph
// =============================================================================

TEST_CASE("vertexlist - empty graph", "[vertexlist][empty]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g;

    SECTION("no value function") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 0);
        REQUIRE(vlist.begin() == vlist.end());
        
        std::size_t count = 0;
        for ([[maybe_unused]] auto vi : vlist) {
            ++count;
        }
        REQUIRE(count == 0);
    }

    SECTION("with value function") {
        auto vlist = vertexlist(g, [](auto v) { return v.vertex_id(); });
        
        REQUIRE(vlist.size() == 0);
        REQUIRE(vlist.begin() == vlist.end());
    }
}

// =============================================================================
// Test 2: Single Vertex
// =============================================================================

TEST_CASE("vertexlist - single vertex", "[vertexlist][single]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{}};  // One vertex with no edges

    SECTION("no value function") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 1);
        
        auto it = vlist.begin();
        REQUIRE(it != vlist.end());
        
        auto vi = *it;
        REQUIRE(vi.vertex.vertex_id() == 0);
        
        ++it;
        REQUIRE(it == vlist.end());
    }

    SECTION("with value function returning vertex_id") {
        auto vlist = vertexlist(g, [](auto v) { return v.vertex_id() * 2; });
        
        REQUIRE(vlist.size() == 1);
        
        auto vi = *vlist.begin();
        REQUIRE(vi.vertex.vertex_id() == 0);
        REQUIRE(vi.value == 0);  // 0 * 2 = 0
    }
}

// =============================================================================
// Test 3: Multiple Vertices
// =============================================================================

TEST_CASE("vertexlist - multiple vertices", "[vertexlist][multiple]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {
        {1, 2},      // vertex 0 -> edges to 1, 2
        {2, 3},      // vertex 1 -> edges to 2, 3
        {3},         // vertex 2 -> edge to 3
        {}           // vertex 3 -> no edges
    };

    SECTION("no value function - iteration") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 4);
        
        std::vector<std::size_t> ids;
        for (auto vi : vlist) {
            ids.push_back(vi.vertex.vertex_id());
        }
        
        REQUIRE(ids == std::vector<std::size_t>{0, 1, 2, 3});
    }

    SECTION("with value function") {
        auto vlist = vertexlist(g, [](auto v) { 
            return static_cast<int>(v.vertex_id() * 10); 
        });
        
        std::vector<int> values;
        for (auto vi : vlist) {
            values.push_back(vi.value);
        }
        
        REQUIRE(values == std::vector<int>{0, 10, 20, 30});
    }

    SECTION("structured binding - no value function") {
        auto vlist = vertexlist(g);
        
        std::size_t idx = 0;
        for (auto [v] : vlist) {
            REQUIRE(v.vertex_id() == idx);
            ++idx;
        }
        REQUIRE(idx == 4);
    }

    SECTION("structured binding - with value function") {
        auto vlist = vertexlist(g, [&g](auto v) { 
            return g[v.vertex_id()].size();  // number of edges
        });
        
        std::vector<std::size_t> edge_counts;
        for (auto [v, count] : vlist) {
            edge_counts.push_back(count);
        }
        
        REQUIRE(edge_counts == std::vector<std::size_t>{2, 2, 1, 0});
    }
}

// =============================================================================
// Test 4: Value Function Types
// =============================================================================

TEST_CASE("vertexlist - value function types", "[vertexlist][vvf]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1}, {2}, {}};

    SECTION("returning string") {
        auto vlist = vertexlist(g, [](auto v) { 
            return "vertex_" + std::to_string(v.vertex_id()); 
        });
        
        std::vector<std::string> names;
        for (auto [v, name] : vlist) {
            names.push_back(name);
        }
        
        REQUIRE(names == std::vector<std::string>{"vertex_0", "vertex_1", "vertex_2"});
    }

    SECTION("returning double") {
        auto vlist = vertexlist(g, [](auto v) { 
            return static_cast<double>(v.vertex_id()) * 1.5; 
        });
        
        std::vector<double> values;
        for (auto [v, val] : vlist) {
            values.push_back(val);
        }
        
        REQUIRE(values[0] == 0.0);
        REQUIRE(values[1] == 1.5);
        REQUIRE(values[2] == 3.0);
    }

    SECTION("capturing lambda") {
        std::vector<std::string> labels = {"A", "B", "C"};
        
        auto vlist = vertexlist(g, [&labels](auto v) { 
            return labels[v.vertex_id()]; 
        });
        
        std::vector<std::string> result;
        for (auto [v, label] : vlist) {
            result.push_back(label);
        }
        
        REQUIRE(result == std::vector<std::string>{"A", "B", "C"});
    }

    SECTION("mutable lambda") {
        int counter = 0;
        auto vlist = vertexlist(g, [&counter](auto) mutable { 
            return counter++; 
        });
        
        std::vector<int> values;
        for (auto [v, val] : vlist) {
            values.push_back(val);
        }
        
        REQUIRE(values == std::vector<int>{0, 1, 2});
    }
}

// =============================================================================
// Test 5: Deque-based Graph
// =============================================================================

TEST_CASE("vertexlist - deque-based graph", "[vertexlist][deque]") {
    using Graph = std::deque<std::deque<int>>;
    Graph g = {
        {1},
        {2},
        {0}
    };

    SECTION("no value function") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 3);
        
        std::vector<std::size_t> ids;
        for (auto [v] : vlist) {
            ids.push_back(v.vertex_id());
        }
        
        REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
    }

    SECTION("with value function") {
        auto vlist = vertexlist(g, [&g](auto v) { 
            return g[v.vertex_id()].front();  // first edge target
        });
        
        std::vector<int> targets;
        for (auto [v, target] : vlist) {
            targets.push_back(target);
        }
        
        REQUIRE(targets == std::vector<int>{1, 2, 0});
    }
}

// =============================================================================
// Test 6: Range Concepts
// =============================================================================

TEST_CASE("vertexlist - range concepts", "[vertexlist][concepts]") {
    using Graph = std::vector<std::vector<int>>;
    using ViewNoVVF = vertexlist_view<Graph, void>;
    using ViewWithVVF = vertexlist_view<Graph, decltype([](auto) { return 0; })>;

    SECTION("input_range satisfied") {
        STATIC_REQUIRE(std::ranges::input_range<ViewNoVVF>);
        STATIC_REQUIRE(std::ranges::input_range<ViewWithVVF>);
    }

    SECTION("forward_range satisfied") {
        STATIC_REQUIRE(std::ranges::forward_range<ViewNoVVF>);
        STATIC_REQUIRE(std::ranges::forward_range<ViewWithVVF>);
    }

    SECTION("sized_range satisfied") {
        STATIC_REQUIRE(std::ranges::sized_range<ViewNoVVF>);
        STATIC_REQUIRE(std::ranges::sized_range<ViewWithVVF>);
    }

    SECTION("view satisfied") {
        STATIC_REQUIRE(std::ranges::view<ViewNoVVF>);
        STATIC_REQUIRE(std::ranges::view<ViewWithVVF>);
    }
}

// =============================================================================
// Test 7: Iterator Properties
// =============================================================================

TEST_CASE("vertexlist - iterator properties", "[vertexlist][iterator]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1, 2}, {2}, {}};

    SECTION("pre-increment") {
        auto vlist = vertexlist(g);
        auto it = vlist.begin();
        
        REQUIRE((*it).vertex.vertex_id() == 0);
        ++it;
        REQUIRE((*it).vertex.vertex_id() == 1);
        ++it;
        REQUIRE((*it).vertex.vertex_id() == 2);
        ++it;
        REQUIRE(it == vlist.end());
    }

    SECTION("post-increment") {
        auto vlist = vertexlist(g);
        auto it = vlist.begin();
        
        auto old = it++;
        REQUIRE((*old).vertex.vertex_id() == 0);
        REQUIRE((*it).vertex.vertex_id() == 1);
    }

    SECTION("equality comparison") {
        auto vlist = vertexlist(g);
        auto it1 = vlist.begin();
        auto it2 = vlist.begin();
        
        REQUIRE(it1 == it2);
        
        ++it1;
        REQUIRE(it1 != it2);
        
        ++it2;
        REQUIRE(it1 == it2);
    }

    SECTION("default constructed iterators are equal") {
        using Iter = decltype(vertexlist(g).begin());
        Iter it1;
        Iter it2;
        REQUIRE(it1 == it2);
    }
}

// =============================================================================
// Test 8: vertex_info Types
// =============================================================================

TEST_CASE("vertexlist - vertex_info types", "[vertexlist][info]") {
    using Graph = std::vector<std::vector<int>>;
    using VertexType = vertex_t<Graph>;

    SECTION("no value function - info type") {
        using ViewType = vertexlist_view<Graph, void>;
        using InfoType = typename ViewType::info_type;
        
        STATIC_REQUIRE(std::is_void_v<typename InfoType::id_type>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::vertex_type, VertexType>);
        STATIC_REQUIRE(std::is_void_v<typename InfoType::value_type>);
    }

    SECTION("with value function - info type") {
        auto vvf = [](auto) { return 42; };
        using VVFType = decltype(vvf);
        using ViewType = vertexlist_view<Graph, VVFType>;
        using InfoType = typename ViewType::info_type;
        
        STATIC_REQUIRE(std::is_void_v<typename InfoType::id_type>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::vertex_type, VertexType>);
        STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
    }
}

// =============================================================================
// Test 9: Const Graph Access
// =============================================================================

TEST_CASE("vertexlist - const graph", "[vertexlist][const]") {
    using Graph = std::vector<std::vector<int>>;
    const Graph g = {{1}, {2}, {}};

    SECTION("no value function") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 3);
        
        std::size_t count = 0;
        for (auto [v] : vlist) {
            REQUIRE(v.vertex_id() == count);
            ++count;
        }
        REQUIRE(count == 3);
    }

    SECTION("with value function") {
        auto vlist = vertexlist(g, [](auto v) { return v.vertex_id(); });
        
        std::vector<std::size_t> ids;
        for (auto [v, id] : vlist) {
            ids.push_back(id);
        }
        
        REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
    }
}

// =============================================================================
// Test 10: Weighted Graph (pair edges)
// =============================================================================

TEST_CASE("vertexlist - weighted graph", "[vertexlist][weighted]") {
    using Graph = std::vector<std::vector<std::pair<int, double>>>;
    Graph g = {
        {{1, 1.5}, {2, 2.5}},  // vertex 0
        {{2, 3.5}},             // vertex 1
        {}                       // vertex 2
    };

    SECTION("iteration works with pair edges") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 3);
        
        std::vector<std::size_t> ids;
        for (auto [v] : vlist) {
            ids.push_back(v.vertex_id());
        }
        
        REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
    }

    SECTION("value function can access edge data") {
        auto vlist = vertexlist(g, [&g](auto v) {
            // Sum of edge weights for this vertex
            double sum = 0.0;
            for (auto [target, weight] : g[v.vertex_id()]) {
                sum += weight;
            }
            return sum;
        });
        
        std::vector<double> sums;
        for (auto [v, sum] : vlist) {
            sums.push_back(sum);
        }
        
        REQUIRE(sums[0] == 4.0);   // 1.5 + 2.5
        REQUIRE(sums[1] == 3.5);
        REQUIRE(sums[2] == 0.0);
    }
}

// =============================================================================
// Test 11: ranges::distance
// =============================================================================

TEST_CASE("vertexlist - std::ranges algorithms", "[vertexlist][algorithms]") {
    using Graph = std::vector<std::vector<int>>;
    Graph g = {{1, 2}, {2}, {}, {0}};

    SECTION("distance") {
        auto vlist = vertexlist(g);
        auto dist = std::ranges::distance(vlist);
        REQUIRE(dist == 4);
    }

    SECTION("count_if") {
        auto vlist = vertexlist(g, [&g](auto v) { return g[v.vertex_id()].size(); });
        
        auto count = std::ranges::count_if(vlist, [](auto vi) {
            return vi.value > 0;
        });
        
        REQUIRE(count == 3);  // vertices 0, 1, and 3 have edges
    }
}

// =============================================================================
// Test 12: Map-Based Vertex Container (Sparse Vertex IDs)
// =============================================================================

TEST_CASE("vertexlist - map vertices vector edges", "[vertexlist][map]") {
    // Map-based graphs have sparse, non-contiguous vertex IDs
    using Graph = std::map<int, std::vector<int>>;
    Graph g = {
        {100, {200, 300}},   // vertex 100 -> edges to 200, 300
        {200, {300}},        // vertex 200 -> edge to 300
        {300, {}}            // vertex 300 -> no edges
    };

    SECTION("iteration over sparse vertex IDs") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 3);
        
        std::vector<int> ids;
        for (auto [v] : vlist) {
            ids.push_back(v.vertex_id());
        }
        
        // Map maintains sorted order
        REQUIRE(ids == std::vector<int>{100, 200, 300});
    }

    SECTION("with value function") {
        auto vlist = vertexlist(g, [&g](auto v) {
            // Return edge count for each vertex
            return g.at(v.vertex_id()).size();
        });
        
        std::vector<std::size_t> edge_counts;
        for (auto [v, count] : vlist) {
            edge_counts.push_back(count);
        }
        
        REQUIRE(edge_counts == std::vector<std::size_t>{2, 1, 0});
    }

    SECTION("empty map graph") {
        Graph empty_g;
        auto vlist = vertexlist(empty_g);
        
        REQUIRE(vlist.size() == 0);
        REQUIRE(vlist.begin() == vlist.end());
    }

    SECTION("single vertex map") {
        Graph single_g = {{42, {}}};
        auto vlist = vertexlist(single_g);
        
        REQUIRE(vlist.size() == 1);
        
        auto [v] = *vlist.begin();
        REQUIRE(v.vertex_id() == 42);
    }
}

// =============================================================================
// Test 13: Map-Based Edge Container (Sorted Edges)
// =============================================================================

TEST_CASE("vertexlist - vector vertices map edges", "[vertexlist][edge_map]") {
    // Edges stored in map (sorted by target, deduplicated)
    using Graph = std::vector<std::map<int, double>>;
    Graph g = {
        {{1, 1.5}, {2, 2.5}},   // vertex 0 -> (1, 1.5), (2, 2.5)
        {{2, 3.5}},              // vertex 1 -> (2, 3.5)
        {}                       // vertex 2 -> no edges
    };

    SECTION("iteration") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 3);
        
        std::vector<std::size_t> ids;
        for (auto [v] : vlist) {
            ids.push_back(v.vertex_id());
        }
        
        REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
    }

    SECTION("with value function accessing edge weights") {
        auto vlist = vertexlist(g, [&g](auto v) {
            // Sum of edge weights for this vertex
            double sum = 0.0;
            for (auto& [target, weight] : g[v.vertex_id()]) {
                sum += weight;
            }
            return sum;
        });
        
        std::vector<double> sums;
        for (auto [v, sum] : vlist) {
            sums.push_back(sum);
        }
        
        REQUIRE(sums[0] == 4.0);   // 1.5 + 2.5
        REQUIRE(sums[1] == 3.5);
        REQUIRE(sums[2] == 0.0);
    }
}

// =============================================================================
// Test 14: Map Vertices + Map Edges (Fully Sparse Graph)
// =============================================================================

TEST_CASE("vertexlist - map vertices map edges", "[vertexlist][map][edge_map]") {
    // Both vertices and edges in maps - fully sparse graph
    using Graph = std::map<int, std::map<int, double>>;
    Graph g = {
        {10, {{20, 1.0}, {30, 2.0}}},   // vertex 10 -> (20, 1.0), (30, 2.0)
        {20, {{30, 3.0}}},               // vertex 20 -> (30, 3.0)
        {30, {}}                         // vertex 30 -> no edges
    };

    SECTION("iteration") {
        auto vlist = vertexlist(g);
        
        REQUIRE(vlist.size() == 3);
        
        std::vector<int> ids;
        for (auto [v] : vlist) {
            ids.push_back(v.vertex_id());
        }
        
        REQUIRE(ids == std::vector<int>{10, 20, 30});
    }

    SECTION("with value function") {
        auto vlist = vertexlist(g, [&g](auto v) {
            return g.at(v.vertex_id()).size();
        });
        
        std::vector<std::size_t> counts;
        for (auto [v, count] : vlist) {
            counts.push_back(count);
        }
        
        REQUIRE(counts == std::vector<std::size_t>{2, 1, 0});
    }

    SECTION("structured binding access") {
        auto vlist = vertexlist(g, [](auto v) { return v.vertex_id() * 10; });
        
        std::vector<int> scaled_ids;
        for (auto [v, scaled] : vlist) {
            scaled_ids.push_back(scaled);
        }
        
        REQUIRE(scaled_ids == std::vector<int>{100, 200, 300});
    }
}
