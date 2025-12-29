/**
 * @file test_dynamic_graph_integration.cpp
 * @brief Integration tests for dynamic_graph - cross-trait operations
 * 
 * Phase 6.1: Cross-Traits Graph Construction
 * Tests copying and converting graphs between different trait types.
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vofl_graph_traits.hpp>
#include <graph/container/traits/dol_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/mol_graph_traits.hpp>
#include <graph/container/traits/mous_graph_traits.hpp>
#include <graph/graph_info.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <ranges>
#include <map>

using namespace graph::container;

//==================================================================================================
// Type Aliases
//==================================================================================================

// Sequential container graphs (integral VId) - void edges
using vov_void = dynamic_graph<void, void, void, uint64_t, false,
                                vov_graph_traits<void, void, void, uint64_t, false>>;
using vofl_void = dynamic_graph<void, void, void, uint64_t, false,
                                 vofl_graph_traits<void, void, void, uint64_t, false>>;
using dol_void = dynamic_graph<void, void, void, uint64_t, false,
                                dol_graph_traits<void, void, void, uint64_t, false>>;
using dofl_void = dynamic_graph<void, void, void, uint64_t, false,
                                 dofl_graph_traits<void, void, void, uint64_t, false>>;
using dov_void = dynamic_graph<void, void, void, uint64_t, false,
                                dov_graph_traits<void, void, void, uint64_t, false>>;

// Sequential container graphs (integral VId) - int edges
using vov_int = dynamic_graph<int, void, void, uint64_t, false,
                               vov_graph_traits<int, void, void, uint64_t, false>>;
using vofl_int = dynamic_graph<int, void, void, uint64_t, false,
                                vofl_graph_traits<int, void, void, uint64_t, false>>;
using dol_int = dynamic_graph<int, void, void, uint64_t, false,
                               dol_graph_traits<int, void, void, uint64_t, false>>;
using dofl_int = dynamic_graph<int, void, void, uint64_t, false,
                                dofl_graph_traits<int, void, void, uint64_t, false>>;
using dov_int = dynamic_graph<int, void, void, uint64_t, false,
                               dov_graph_traits<int, void, void, uint64_t, false>>;

// Map-based graphs (string VId) - void edges
using mos_void = dynamic_graph<void, void, void, std::string, false,
                                mos_graph_traits<void, void, void, std::string, false>>;
using mol_void = dynamic_graph<void, void, void, std::string, false,
                                mol_graph_traits<void, void, void, std::string, false>>;
using mous_void = dynamic_graph<void, void, void, std::string, false,
                                 mous_graph_traits<void, void, void, std::string, false>>;

// Map-based graphs (string VId) - int edges
using mos_int = dynamic_graph<int, void, void, std::string, false,
                               mos_graph_traits<int, void, void, std::string, false>>;
using mol_int = dynamic_graph<int, void, void, std::string, false,
                               mol_graph_traits<int, void, void, std::string, false>>;
using mous_int = dynamic_graph<int, void, void, std::string, false,
                                mous_graph_traits<int, void, void, std::string, false>>;

//==================================================================================================
// Helper: Count all edges
//==================================================================================================

template<typename G>
size_t count_edges(const G& g) {
    size_t count = 0;
    for (auto&& v : vertices(g)) {
        count += static_cast<size_t>(std::ranges::distance(edges(g, v)));
    }
    return count;
}

//==================================================================================================
// Phase 6.1.1: Copy Between Sequential Traits (void edges)
//==================================================================================================

TEST_CASE("Copy vov to vofl - void edges", "[integration][6.1.1]") {
    vov_void source_graph({{0, 1}, {1, 2}, {2, 0}});
    
    // Extract edges using CPOs
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
        }
    }
    
    // Load into target
    vofl_void target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == source_graph.size());
    REQUIRE(count_edges(target_graph) == 3);
}

TEST_CASE("Copy vofl to dov - void edges", "[integration][6.1.1]") {
    vofl_void source_graph({{0, 1}, {1, 2}});
    
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
        }
    }
    
    dov_void target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy dofl to dol - void edges", "[integration][6.1.1]") {
    dofl_void source_graph({{0, 1}, {1, 2}, {2, 3}});
    
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
        }
    }
    
    dol_void target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == 4);
    REQUIRE(count_edges(target_graph) == 3);
}

TEST_CASE("Copy vov to vofl - empty graph", "[integration][6.1.1]") {
    vov_void source_graph;
    
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    
    vofl_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 0);
    REQUIRE(count_edges(target_graph) == 0);
}

TEST_CASE("Copy vov to vofl - self-loop", "[integration][6.1.1]") {
    vov_void source_graph({{0, 0}});
    
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
        }
    }
    
    vofl_void target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == 1);
    REQUIRE(count_edges(target_graph) == 1);
}

//==================================================================================================
// Phase 6.1.1: Copy Between Sequential Traits (int edges)
//==================================================================================================

TEST_CASE("Copy vov to vofl - int edges", "[integration][6.1.1]") {
    vov_int source_graph({{0, 1, 100}, {1, 2, 200}});
    
    std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
        }
    }
    
    vofl_int target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
    
    // Verify edge value preserved
    auto v0 = find_vertex(target_graph, uint64_t{0});
    auto e_rng = edges(target_graph, *v0);
    auto first_edge = *std::ranges::begin(e_rng);
    REQUIRE(edge_value(target_graph, first_edge) == 100);
}

TEST_CASE("Copy dofl to dol - int edges", "[integration][6.1.1]") {
    dofl_int source_graph({{0, 1, 10}, {1, 2, 20}});
    
    std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
        }
    }
    
    dol_int target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy vov to dov - int edges large", "[integration][6.1.1]") {
    vov_int source_graph;
    std::vector<graph::copyable_edge_t<uint64_t, int>> src_edges;
    for (uint64_t i = 0; i < 50; ++i) {
        src_edges.push_back({i, (i + 1) % 50, static_cast<int>(i)});
    }
    source_graph.load_edges(src_edges, std::identity{});
    
    std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
        }
    }
    
    dov_int target_graph;
    target_graph.load_edges(edge_list, std::identity{}, source_graph.size());
    
    REQUIRE(target_graph.size() == 50);
    REQUIRE(count_edges(target_graph) == 50);
}

//==================================================================================================
// Phase 6.1.2: Copy Sequential to Map (void edges)
//==================================================================================================

TEST_CASE("Copy vov to mos - void edges", "[integration][6.1.2]") {
    vov_void source_graph({{0, 1}, {1, 2}, {2, 0}});
    
    // Convert uint64_t IDs to string IDs
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                std::to_string(vertex_id(source_graph, v)),
                std::to_string(target_id(source_graph, e))
            });
        }
    }
    
    mos_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 3);
}

TEST_CASE("Copy dol to mol - void edges", "[integration][6.1.2]") {
    dol_void source_graph({{0, 1}, {1, 2}});
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                std::to_string(vertex_id(source_graph, v)),
                std::to_string(target_id(source_graph, e))
            });
        }
    }
    
    mol_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy vov to mos - sparse IDs", "[integration][6.1.2]") {
    // Source has sparse IDs: 0, 10, 100
    vov_void source_graph;
    std::vector<graph::copyable_edge_t<uint64_t, void>> src_edges{
        {0, 10}, {10, 100}, {100, 0}
    };
    source_graph.load_edges(src_edges, std::identity{}, 101);
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                std::to_string(vertex_id(source_graph, v)),
                std::to_string(target_id(source_graph, e))
            });
        }
    }
    
    mos_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    // Map only contains explicitly referenced vertices
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 3);
    
    REQUIRE(find_vertex(target_graph, std::string("0")) != vertices(target_graph).end());
    REQUIRE(find_vertex(target_graph, std::string("10")) != vertices(target_graph).end());
    REQUIRE(find_vertex(target_graph, std::string("100")) != vertices(target_graph).end());
}

//==================================================================================================
// Phase 6.1.2: Copy Sequential to Map (int edges)
//==================================================================================================

TEST_CASE("Copy vov to mos - int edges", "[integration][6.1.2]") {
    vov_int source_graph({{0, 1, 100}, {1, 2, 200}});
    
    std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                std::to_string(vertex_id(source_graph, v)),
                std::to_string(target_id(source_graph, e)),
                edge_value(source_graph, e)
            });
        }
    }
    
    mos_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
    
    // Verify edge value
    auto v0 = find_vertex(target_graph, std::string("0"));
    auto e_rng = edges(target_graph, *v0);
    auto first_edge = *std::ranges::begin(e_rng);
    REQUIRE(edge_value(target_graph, first_edge) == 100);
}

TEST_CASE("Copy dol to mol - int edges", "[integration][6.1.2]") {
    dol_int source_graph({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});
    
    std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                std::to_string(vertex_id(source_graph, v)),
                std::to_string(target_id(source_graph, e)),
                edge_value(source_graph, e)
            });
        }
    }
    
    mol_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 3);
}

//==================================================================================================
// Phase 6.1.3: Copy Map to Sequential (void edges)
//==================================================================================================

TEST_CASE("Copy mos to vov - void edges", "[integration][6.1.3]") {
    mos_void source_graph({{"a", "b"}, {"b", "c"}, {"c", "a"}});
    
    // Create ID mapping: string → uint64_t
    std::map<std::string, uint64_t> id_map;
    uint64_t next_id = 0;
    for (auto&& v : vertices(source_graph)) {
        auto vid = vertex_id(source_graph, v);
        if (!id_map.contains(vid)) {
            id_map[vid] = next_id++;
        }
    }
    
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                id_map[vertex_id(source_graph, v)],
                id_map[target_id(source_graph, e)]
            });
        }
    }
    
    vov_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 3);
}

TEST_CASE("Copy mol to dofl - void edges", "[integration][6.1.3]") {
    mol_void source_graph({{"p", "q"}, {"q", "r"}});
    
    std::map<std::string, uint64_t> id_map;
    uint64_t next_id = 0;
    for (auto&& v : vertices(source_graph)) {
        auto vid = vertex_id(source_graph, v);
        if (!id_map.contains(vid)) {
            id_map[vid] = next_id++;
        }
    }
    
    std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                id_map[vertex_id(source_graph, v)],
                id_map[target_id(source_graph, e)]
            });
        }
    }
    
    dofl_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

//==================================================================================================
// Phase 6.1.3: Copy Map to Sequential (int edges)
//==================================================================================================

TEST_CASE("Copy mos to vov - int edges", "[integration][6.1.3]") {
    mos_int source_graph({{"a", "b", 100}, {"b", "c", 200}});
    
    std::map<std::string, uint64_t> id_map;
    uint64_t next_id = 0;
    for (auto&& v : vertices(source_graph)) {
        auto vid = vertex_id(source_graph, v);
        if (!id_map.contains(vid)) {
            id_map[vid] = next_id++;
        }
    }
    
    std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                id_map[vertex_id(source_graph, v)],
                id_map[target_id(source_graph, e)],
                edge_value(source_graph, e)
            });
        }
    }
    
    vov_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy mol to dofl - int edges", "[integration][6.1.3]") {
    mol_int source_graph({{"a", "b", 1}, {"b", "c", 2}, {"c", "a", 3}});
    
    std::map<std::string, uint64_t> id_map;
    uint64_t next_id = 0;
    for (auto&& v : vertices(source_graph)) {
        auto vid = vertex_id(source_graph, v);
        if (!id_map.contains(vid)) {
            id_map[vid] = next_id++;
        }
    }
    
    std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                id_map[vertex_id(source_graph, v)],
                id_map[target_id(source_graph, e)],
                edge_value(source_graph, e)
            });
        }
    }
    
    dofl_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 3);
}

TEST_CASE("Copy mos to vov - bijective ID mapping", "[integration][6.1.3]") {
    mos_void source_graph({{"x", "y"}, {"y", "z"}, {"z", "x"}});
    
    std::map<std::string, uint64_t> id_map;
    uint64_t next_id = 0;
    for (auto&& v : vertices(source_graph)) {
        auto vid = vertex_id(source_graph, v);
        if (!id_map.contains(vid)) {
            id_map[vid] = next_id++;
        }
    }
    
    // Verify bijection
    REQUIRE(id_map.size() == 3);
    REQUIRE(id_map["x"] != id_map["y"]);
    REQUIRE(id_map["y"] != id_map["z"]);
    REQUIRE(id_map["z"] != id_map["x"]);
}

//==================================================================================================
// Phase 6.1.4: Copy Within Same Category
//==================================================================================================

TEST_CASE("Copy mos to mous - void edges", "[integration][6.1.4]") {
    mos_void source_graph({{"a", "b"}, {"b", "c"}});
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e)
            });
        }
    }
    
    mous_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy mous to mos - void edges", "[integration][6.1.4]") {
    mous_void source_graph({{"p", "q"}, {"q", "r"}, {"r", "p"}});
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e)
            });
        }
    }
    
    mos_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 3);
}

TEST_CASE("Copy mos to mous - int edges", "[integration][6.1.4]") {
    mos_int source_graph({{"x", "y", 10}, {"y", "z", 20}});
    
    std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e),
                edge_value(source_graph, e)
            });
        }
    }
    
    mous_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy mous to mos - int edges", "[integration][6.1.4]") {
    mous_int source_graph({{"a", "b", 100}, {"b", "c", 200}});
    
    std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e),
                edge_value(source_graph, e)
            });
        }
    }
    
    mos_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 3);
    REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy mos to mous - ordering may differ", "[integration][6.1.4]") {
    mos_void source_graph({{"z", "a"}, {"a", "m"}, {"m", "b"}});
    
    // mos is ordered (alphabetically by key)
    // Edges create vertices: z, a (sources) and a, m, b (targets)
    // Vertices in order: a, b, m, z
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e)
            });
        }
    }
    
    mous_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    // mous is unordered - iteration order may differ
    std::vector<std::string> mous_order;
    for (auto&& v : vertices(target_graph)) {
        mous_order.push_back(vertex_id(target_graph, v));
    }
    
    REQUIRE(mous_order.size() == 4); // a, b, m, z (in some order)
}

TEST_CASE("Copy mos to mous - empty graph", "[integration][6.1.4]") {
    mos_void source_graph;
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    
    mous_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 0);
}

TEST_CASE("Copy mos to mous - self-loop", "[integration][6.1.4]") {
    mos_void source_graph({{"only", "only"}});
    
    std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e)
            });
        }
    }
    
    mous_void target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 1);
    REQUIRE(count_edges(target_graph) == 1);
}

TEST_CASE("Copy mos to mous - large graph", "[integration][6.1.4]") {
    mos_int source_graph;
    std::vector<graph::copyable_edge_t<std::string, int>> src_edges;
    for (int i = 0; i < 50; ++i) {
        src_edges.push_back({
            std::to_string(i),
            std::to_string((i + 1) % 50),
            i
        });
    }
    source_graph.load_edges(src_edges, std::identity{});
    
    std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e),
                edge_value(source_graph, e)
            });
        }
    }
    
    mous_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == 50);
    REQUIRE(count_edges(target_graph) == 50);
}

TEST_CASE("Copy mous to mos - preserves all data", "[integration][6.1.4]") {
    mous_int source_graph({{"one", "two", 1}, {"two", "three", 2}, {"three", "one", 3}});
    
    std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
    for (auto&& v : vertices(source_graph)) {
        for (auto&& e : edges(source_graph, v)) {
            edge_list.push_back({
                vertex_id(source_graph, v),
                target_id(source_graph, e),
                edge_value(source_graph, e)
            });
        }
    }
    
    mos_int target_graph;
    target_graph.load_edges(edge_list, std::identity{});
    
    REQUIRE(target_graph.size() == source_graph.size());
    REQUIRE(count_edges(target_graph) == count_edges(source_graph));
}
