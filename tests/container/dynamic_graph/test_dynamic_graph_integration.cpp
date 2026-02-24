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
#include <graph/container/traits/vos_graph_traits.hpp>
#include <graph/container/traits/vous_graph_traits.hpp>
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
using vov_void = dynamic_graph<void, void, void, uint64_t, false, vov_graph_traits<void, void, void, uint64_t, false>>;
using vofl_void =
      dynamic_graph<void, void, void, uint64_t, false, vofl_graph_traits<void, void, void, uint64_t, false>>;
using dol_void = dynamic_graph<void, void, void, uint64_t, false, dol_graph_traits<void, void, void, uint64_t, false>>;
using dofl_void =
      dynamic_graph<void, void, void, uint64_t, false, dofl_graph_traits<void, void, void, uint64_t, false>>;
using dov_void = dynamic_graph<void, void, void, uint64_t, false, dov_graph_traits<void, void, void, uint64_t, false>>;

// Sequential container graphs (integral VId) - int edges
using vov_int  = dynamic_graph<int, void, void, uint64_t, false, vov_graph_traits<int, void, void, uint64_t, false>>;
using vofl_int = dynamic_graph<int, void, void, uint64_t, false, vofl_graph_traits<int, void, void, uint64_t, false>>;
using dol_int  = dynamic_graph<int, void, void, uint64_t, false, dol_graph_traits<int, void, void, uint64_t, false>>;
using dofl_int = dynamic_graph<int, void, void, uint64_t, false, dofl_graph_traits<int, void, void, uint64_t, false>>;
using dov_int  = dynamic_graph<int, void, void, uint64_t, false, dov_graph_traits<int, void, void, uint64_t, false>>;

// Map-based graphs (string VId) - void edges
using mos_void =
      dynamic_graph<void, void, void, std::string, false, mos_graph_traits<void, void, void, std::string, false>>;
using mol_void =
      dynamic_graph<void, void, void, std::string, false, mol_graph_traits<void, void, void, std::string, false>>;
using mous_void =
      dynamic_graph<void, void, void, std::string, false, mous_graph_traits<void, void, void, std::string, false>>;

// Map-based graphs (string VId) - int edges
using mos_int =
      dynamic_graph<int, void, void, std::string, false, mos_graph_traits<int, void, void, std::string, false>>;
using mol_int =
      dynamic_graph<int, void, void, std::string, false, mol_graph_traits<int, void, void, std::string, false>>;
using mous_int =
      dynamic_graph<int, void, void, std::string, false, mous_graph_traits<int, void, void, std::string, false>>;

// Set-based edge container graphs (integral VId) - void edges
using vos_void = dynamic_graph<void, void, void, uint64_t, false, vos_graph_traits<void, void, void, uint64_t, false>>;

// Unordered set-based edge container graphs (integral VId) - void edges
using vous_void =
      dynamic_graph<void, void, void, uint64_t, false, vous_graph_traits<void, void, void, uint64_t, false>>;

// Set-based edge container graphs (integral VId) - int edges
using vos_int = dynamic_graph<int, void, void, uint64_t, false, vos_graph_traits<int, void, void, uint64_t, false>>;

// String edge value types
using vov_string  = dynamic_graph<std::string,
                                  void,
                                  void,
                                  uint64_t,
                                  false, vov_graph_traits<std::string, void, void, uint64_t, false>>;
using vofl_string = dynamic_graph<std::string,
                                  void,
                                  void,
                                  uint64_t,
                                  false, vofl_graph_traits<std::string, void, void, uint64_t, false>>;
using mos_string  = dynamic_graph<std::string,
                                  void,
                                  void,
                                  std::string,
                                  false, mos_graph_traits<std::string, void, void, std::string, false>>;
using mol_string  = dynamic_graph<std::string,
                                  void,
                                  void,
                                  std::string,
                                  false, mol_graph_traits<std::string, void, void, std::string, false>>;

//==================================================================================================
// Helper: Count all edges
//==================================================================================================

template <typename G>
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
  auto v0         = find_vertex(target_graph, uint64_t{0});
  auto e_rng      = edges(target_graph, *v0);
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
  vov_int                                            source_graph;
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
      edge_list.push_back({std::to_string(vertex_id(source_graph, v)), std::to_string(target_id(source_graph, e))});
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
      edge_list.push_back({std::to_string(vertex_id(source_graph, v)), std::to_string(target_id(source_graph, e))});
    }
  }

  mol_void target_graph;
  target_graph.load_edges(edge_list, std::identity{});

  REQUIRE(target_graph.size() == 3);
  REQUIRE(count_edges(target_graph) == 2);
}

TEST_CASE("Copy vov to mos - sparse IDs", "[integration][6.1.2]") {
  // Source has sparse IDs: 0, 10, 100
  vov_void                                            source_graph;
  std::vector<graph::copyable_edge_t<uint64_t, void>> src_edges{{0, 10}, {10, 100}, {100, 0}};
  source_graph.load_edges(src_edges, std::identity{}, 101);

  std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back({std::to_string(vertex_id(source_graph, v)), std::to_string(target_id(source_graph, e))});
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
      edge_list.push_back({std::to_string(vertex_id(source_graph, v)), std::to_string(target_id(source_graph, e)),
                           edge_value(source_graph, e)});
    }
  }

  mos_int target_graph;
  target_graph.load_edges(edge_list, std::identity{});

  REQUIRE(target_graph.size() == 3);
  REQUIRE(count_edges(target_graph) == 2);

  // Verify edge value
  auto v0         = find_vertex(target_graph, std::string("0"));
  auto e_rng      = edges(target_graph, *v0);
  auto first_edge = *std::ranges::begin(e_rng);
  REQUIRE(edge_value(target_graph, first_edge) == 100);
}

TEST_CASE("Copy dol to mol - int edges", "[integration][6.1.2]") {
  dol_int source_graph({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});

  std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back({std::to_string(vertex_id(source_graph, v)), std::to_string(target_id(source_graph, e)),
                           edge_value(source_graph, e)});
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
  uint64_t                        next_id = 0;
  for (auto&& v : vertices(source_graph)) {
    auto vid = vertex_id(source_graph, v);
    if (!id_map.contains(vid)) {
      id_map[vid] = next_id++;
    }
  }

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back({id_map[vertex_id(source_graph, v)], id_map[target_id(source_graph, e)]});
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
  uint64_t                        next_id = 0;
  for (auto&& v : vertices(source_graph)) {
    auto vid = vertex_id(source_graph, v);
    if (!id_map.contains(vid)) {
      id_map[vid] = next_id++;
    }
  }

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back({id_map[vertex_id(source_graph, v)], id_map[target_id(source_graph, e)]});
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
  uint64_t                        next_id = 0;
  for (auto&& v : vertices(source_graph)) {
    auto vid = vertex_id(source_graph, v);
    if (!id_map.contains(vid)) {
      id_map[vid] = next_id++;
    }
  }

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back(
            {id_map[vertex_id(source_graph, v)], id_map[target_id(source_graph, e)], edge_value(source_graph, e)});
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
  uint64_t                        next_id = 0;
  for (auto&& v : vertices(source_graph)) {
    auto vid = vertex_id(source_graph, v);
    if (!id_map.contains(vid)) {
      id_map[vid] = next_id++;
    }
  }

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back(
            {id_map[vertex_id(source_graph, v)], id_map[target_id(source_graph, e)], edge_value(source_graph, e)});
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
  uint64_t                        next_id = 0;
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e)});
    }
  }

  mous_void target_graph;
  target_graph.load_edges(edge_list, std::identity{});

  REQUIRE(target_graph.size() == 1);
  REQUIRE(count_edges(target_graph) == 1);
}

TEST_CASE("Copy mos to mous - large graph", "[integration][6.1.4]") {
  mos_int                                               source_graph;
  std::vector<graph::copyable_edge_t<std::string, int>> src_edges;
  for (int i = 0; i < 50; ++i) {
    src_edges.push_back({std::to_string(i), std::to_string((i + 1) % 50), i});
  }
  source_graph.load_edges(src_edges, std::identity{});

  std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
  for (auto&& v : vertices(source_graph)) {
    for (auto&& e : edges(source_graph, v)) {
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
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
      edge_list.push_back({vertex_id(source_graph, v), target_id(source_graph, e), edge_value(source_graph, e)});
    }
  }

  mos_int target_graph;
  target_graph.load_edges(edge_list, std::identity{});

  REQUIRE(target_graph.size() == source_graph.size());
  REQUIRE(count_edges(target_graph) == count_edges(source_graph));
}

//==================================================================================================
// Phase 6.5.1: Empty Graph Operations
//==================================================================================================

TEST_CASE("Empty vov to vov - void edges", "[integration][6.5.1][empty]") {
  vov_void source;

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vov_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Empty vov to vofl - void edges", "[integration][6.5.1][empty]") {
  vov_void source;

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;

  vofl_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Empty dov to dol - void edges", "[integration][6.5.1][empty]") {
  dov_void source;

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;

  dol_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Empty mos to mol - void edges", "[integration][6.5.1][empty]") {
  mos_void source;

  std::vector<graph::copyable_edge_t<std::string, void>> edge_list;

  mol_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Empty vov to vov - int edges", "[integration][6.5.1][empty]") {
  vov_int source;

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Empty mos to mous - int edges", "[integration][6.5.1][empty]") {
  mos_int source;

  std::vector<graph::copyable_edge_t<std::string, int>> edge_list;

  mous_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Empty graph - vertices range empty", "[integration][6.5.1][empty]") {
  vov_void g;

  auto vert_rng = vertices(g);
  REQUIRE(vert_rng.begin() == vert_rng.end());
  REQUIRE(std::ranges::distance(vert_rng) == 0);
}

TEST_CASE("Empty map graph - vertices range empty", "[integration][6.5.1][empty]") {
  mos_void g;

  auto vert_rng = vertices(g);
  REQUIRE(vert_rng.begin() == vert_rng.end());
  REQUIRE(std::ranges::distance(vert_rng) == 0);
}

TEST_CASE("Empty graph - find_vertex finds nothing in vov", "[integration][6.5.1][empty]") {
  vov_void g;

  auto it = find_vertex(g, 0);
  REQUIRE(it == vertices(g).end());
}

TEST_CASE("Empty graph - find_vertex finds nothing in mos", "[integration][6.5.1][empty]") {
  mos_void g;

  auto it = find_vertex(g, std::string("missing"));
  REQUIRE(it == vertices(g).end());
}

TEST_CASE("Empty graph - STL for_each does nothing", "[integration][6.5.1][empty]") {
  vov_void g;

  int count = 0;
  std::ranges::for_each(vertices(g), [&](auto&&) { ++count; });
  REQUIRE(count == 0);
}

TEST_CASE("Empty graph - STL count_if returns 0", "[integration][6.5.1][empty]") {
  mos_void g;

  auto result = std::ranges::count_if(vertices(g), [](auto&&) { return true; });
  REQUIRE(result == 0);
}

//==================================================================================================
// Phase 6.5.3: Self-Loop Handling Across Types
//==================================================================================================

TEST_CASE("Self-loop - vov single vertex", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}});

  REQUIRE(g.size() == 1);
  REQUIRE(count_edges(g) == 1);

  auto edge_rng = edges(g, 0);
  REQUIRE(std::ranges::distance(edge_rng) == 1);

  auto e = *edge_rng.begin();
  REQUIRE(target_id(g, e) == 0);
}

TEST_CASE("Self-loop - vov multiple self-loops", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}, {1, 1}, {2, 2}});

  REQUIRE(g.size() == 3);
  REQUIRE(count_edges(g) == 3);

  for (uint64_t v = 0; v < 3; ++v) {
    auto edge_rng = edges(g, v);
    REQUIRE(std::ranges::distance(edge_rng) == 1);
    REQUIRE(target_id(g, *edge_rng.begin()) == v);
  }
}

TEST_CASE("Self-loop - vov mixed with normal edges", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}, {0, 1}, {1, 1}, {1, 2}});

  REQUIRE(g.size() == 3);
  REQUIRE(count_edges(g) == 4);

  auto edges0 = edges(g, 0);
  REQUIRE(std::ranges::distance(edges0) == 2);

  auto edges1 = edges(g, 1);
  REQUIRE(std::ranges::distance(edges1) == 2);
}

TEST_CASE("Self-loop - vofl preserves self-loops", "[integration][6.5.3][self_loop]") {
  vofl_void g({{0, 0}, {0, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);

  auto edges0 = edges(g, 0);
  REQUIRE(std::ranges::distance(edges0) == 2);
}

TEST_CASE("Self-loop - dov preserves self-loops", "[integration][6.5.3][self_loop]") {
  dov_void g({{0, 0}, {1, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);
}

TEST_CASE("Self-loop - mos with string IDs", "[integration][6.5.3][self_loop]") {
  mos_void g({{"a", "a"}, {"b", "b"}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);

  auto edges_a = edges(g, "a");
  REQUIRE(std::ranges::distance(edges_a) == 1);
  REQUIRE(target_id(g, *edges_a.begin()) == "a");
}

TEST_CASE("Self-loop - mous with string IDs", "[integration][6.5.3][self_loop]") {
  mous_void g({{"a", "a"}, {"b", "b"}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);
}

TEST_CASE("Self-loop - copy vov to vofl preserves", "[integration][6.5.3][self_loop]") {
  vov_void source({{0, 0}, {1, 1}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vofl_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 2);
  REQUIRE(count_edges(target) == 2);

  for (uint64_t v = 0; v < 2; ++v) {
    auto edge_rng = edges(target, v);
    REQUIRE(std::ranges::distance(edge_rng) == 1);
    REQUIRE(target_id(target, *edge_rng.begin()) == v);
  }
}

TEST_CASE("Self-loop - copy vov to dov preserves", "[integration][6.5.3][self_loop]") {
  vov_void source({{0, 0}, {1, 1}, {2, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  dov_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);
}

TEST_CASE("Self-loop - copy mos to mol preserves", "[integration][6.5.3][self_loop]") {
  mos_void source({{"a", "a"}, {"b", "b"}});

  std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  mol_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 2);
  REQUIRE(count_edges(target) == 2);
}

TEST_CASE("Self-loop - copy mos to mous preserves", "[integration][6.5.3][self_loop]") {
  mos_void source({{"x", "x"}, {"y", "y"}, {"z", "z"}});

  std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  mous_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);
}

TEST_CASE("Self-loop - vov with int edges", "[integration][6.5.3][self_loop]") {
  vov_int g({{0, 0, 100}, {1, 1, 200}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);

  auto edges0 = edges(g, 0);
  REQUIRE(std::ranges::distance(edges0) == 1);
  REQUIRE(edge_value(g, *edges0.begin()) == 100);
}

TEST_CASE("Self-loop - mos with int edges", "[integration][6.5.3][self_loop]") {
  mos_int g({{"a", "a", 42}, {"b", "b", 99}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);

  auto edges_a = edges(g, "a");
  REQUIRE(std::ranges::distance(edges_a) == 1);
  REQUIRE(edge_value(g, *edges_a.begin()) == 42);
}

TEST_CASE("Self-loop - degree counts self-loops", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}});

  REQUIRE(degree(g, 0) == 1);
}

TEST_CASE("Self-loop - multiple self-loops on same vertex (vov)", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}, {0, 0}});

  REQUIRE(g.size() == 1);
  REQUIRE(count_edges(g) == 2);
  REQUIRE(degree(g, 0) == 2);
}

TEST_CASE("Self-loop - multiple self-loops on same vertex (vofl)", "[integration][6.5.3][self_loop]") {
  vofl_void g({{0, 0}, {0, 0}, {0, 0}});

  REQUIRE(g.size() == 1);
  REQUIRE(count_edges(g) == 3);
  REQUIRE(degree(g, 0) == 3);
}

TEST_CASE("Self-loop - contains_edge finds self-loop", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}});

  REQUIRE(contains_edge(g, 0, 0));
}

TEST_CASE("Self-loop - find_edge finds self-loop", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}, {0, 1}});

  auto edge_rng = edges(g, 0);
  auto it       = std::ranges::find_if(edge_rng, [&](auto&& e) { return target_id(g, e) == 0; });

  REQUIRE(it != edge_rng.end());
  REQUIRE(target_id(g, *it) == 0);
}

TEST_CASE("Self-loop - count self-loops generically", "[integration][6.5.3][self_loop]") {
  vov_void g({{0, 0}, {0, 1}, {1, 1}, {1, 2}});

  size_t self_loop_count = 0;
  for (auto&& v : vertices(g)) {
    auto vid = vertex_id(g, v);
    for (auto&& e : edges(g, v)) {
      if (target_id(g, e) == vid) {
        ++self_loop_count;
      }
    }
  }

  REQUIRE(self_loop_count == 2);
}

TEST_CASE("Self-loop - count self-loops in map graph", "[integration][6.5.3][self_loop]") {
  mos_void g({{"a", "a"}, {"a", "b"}, {"b", "b"}, {"b", "c"}});

  size_t self_loop_count = 0;
  for (auto&& v : vertices(g)) {
    auto vid = vertex_id(g, v);
    for (auto&& e : edges(g, v)) {
      if (target_id(g, e) == vid) {
        ++self_loop_count;
      }
    }
  }

  REQUIRE(self_loop_count == 2);
}

//==================================================================================================
// Phase 6.5.4: Parallel Edges Across Types
//==================================================================================================

TEST_CASE("Parallel edges - vov allows duplicates", "[integration][6.5.4][parallel]") {
  vov_void g({{0, 1}, {0, 1}, {0, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 3);
  REQUIRE(degree(g, 0) == 3);
}

TEST_CASE("Parallel edges - vofl allows duplicates", "[integration][6.5.4][parallel]") {
  vofl_void g({{0, 1}, {0, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 2);
  REQUIRE(degree(g, 0) == 2);
}

TEST_CASE("Parallel edges - dol allows duplicates", "[integration][6.5.4][parallel]") {
  dol_void g({{0, 1}, {0, 1}, {0, 1}, {0, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 4);
}

TEST_CASE("Parallel edges - copy vov to vos deduplicates", "[integration][6.5.4][parallel]") {
  vov_void source({{0, 1}, {0, 1}, {0, 1}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vos_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 2);
  REQUIRE(count_edges(target) == 1); // Deduplicated to 1 edge
  REQUIRE(degree(target, 0) == 1);
}

TEST_CASE("Parallel edges - copy vofl to vos deduplicates", "[integration][6.5.4][parallel]") {
  vofl_void source({{0, 1}, {0, 1}, {0, 2}, {0, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vos_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2); // 4 edges -> 2 unique edges
  REQUIRE(degree(target, 0) == 2);
}

TEST_CASE("Parallel edges - copy vov to vous deduplicates", "[integration][6.5.4][parallel]") {
  vov_void source({{0, 1}, {0, 1}, {1, 2}, {1, 2}, {1, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vous_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2); // 5 edges -> 2 unique
  REQUIRE(degree(target, 0) == 1);
  REQUIRE(degree(target, 1) == 1);
}

TEST_CASE("Parallel edges - copy dol to mos deduplicates", "[integration][6.5.4][parallel]") {
  dol_void source({{0, 1}, {0, 1}, {0, 2}});

  std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({std::to_string(vertex_id(source, v)), std::to_string(target_id(source, e))});
    }
  }

  mos_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2); // 3 edges -> 2 unique
}

TEST_CASE("Parallel edges - vos has no duplicates", "[integration][6.5.4][parallel]") {
  vos_void g({{0, 1}, {0, 1}, {0, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 1); // Set automatically deduplicates
  REQUIRE(degree(g, 0) == 1);
}

TEST_CASE("Parallel edges - vous has no duplicates", "[integration][6.5.4][parallel]") {
  vous_void g({{0, 1}, {0, 1}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 1);
  REQUIRE(degree(g, 0) == 1);
}

TEST_CASE("Parallel edges - mos has no duplicates", "[integration][6.5.4][parallel]") {
  mos_void g({{"a", "b"}, {"a", "b"}, {"a", "b"}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 1);
}

TEST_CASE("Parallel edges - copy vos to vov no duplicates", "[integration][6.5.4][parallel]") {
  vos_void source({{0, 1}, {0, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vov_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2); // Still no duplicates
}

TEST_CASE("Parallel edges - copy vous to vofl no duplicates", "[integration][6.5.4][parallel]") {
  vous_void source({{0, 1}, {0, 2}, {1, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vofl_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);
}

TEST_CASE("Parallel edges - copy mos to mol no duplicates", "[integration][6.5.4][parallel]") {
  mos_void source({{"a", "b"}, {"a", "c"}});

  std::vector<graph::copyable_edge_t<std::string, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  mol_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);
}

TEST_CASE("Parallel edges - vov with int edges allows duplicates", "[integration][6.5.4][parallel]") {
  vov_int g({{0, 1, 10}, {0, 1, 20}, {0, 1, 30}});

  REQUIRE(g.size() == 2);
  REQUIRE(count_edges(g) == 3);

  auto             edge_rng = edges(g, 0);
  std::vector<int> values;
  for (auto&& e : edge_rng) {
    values.push_back(edge_value(g, e));
  }

  REQUIRE(values.size() == 3);
  REQUIRE(std::ranges::find(values, 10) != values.end());
  REQUIRE(std::ranges::find(values, 20) != values.end());
  REQUIRE(std::ranges::find(values, 30) != values.end());
}

TEST_CASE("Parallel edges - copy vov int to vos deduplicates by target", "[integration][6.5.4][parallel]") {
  vov_int source({{0, 1, 10}, {0, 1, 20}, {0, 1, 30}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), edge_value(source, e)});
    }
  }

  vos_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 2);
  REQUIRE(count_edges(target) == 1); // Deduplicated to 1 edge
                                     // Note: One of the edge values is kept (implementation-defined which one)
}

TEST_CASE("Parallel edges - mixed regular and parallel", "[integration][6.5.4][parallel]") {
  vov_void g({{0, 1}, {0, 1}, {0, 2}, {1, 2}, {1, 2}, {1, 2}});

  REQUIRE(g.size() == 3);
  REQUIRE(count_edges(g) == 6);
  REQUIRE(degree(g, 0) == 3);
  REQUIRE(degree(g, 1) == 3);
}

TEST_CASE("Parallel edges - copy mixed to vos partial deduplication", "[integration][6.5.4][parallel]") {
  vov_void source({{0, 1}, {0, 1}, {0, 2}, {1, 2}, {1, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vos_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3); // 5 edges -> 3 unique (0->1, 0->2, 1->2)
  REQUIRE(degree(target, 0) == 2);
  REQUIRE(degree(target, 1) == 1);
}

TEST_CASE("Parallel edges - vofl self-loops can be parallel", "[integration][6.5.4][parallel]") {
  vofl_void g({{0, 0}, {0, 0}, {0, 0}});

  REQUIRE(g.size() == 1);
  REQUIRE(count_edges(g) == 3);
  REQUIRE(degree(g, 0) == 3);
}

TEST_CASE("Parallel edges - copy vofl self-loops to vos deduplicates", "[integration][6.5.4][parallel]") {
  vofl_void source({{0, 0}, {0, 0}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vos_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 1);
  REQUIRE(count_edges(target) == 1); // Self-loop deduplicated
  REQUIRE(degree(target, 0) == 1);
}

TEST_CASE("Parallel edges - count unique edges in vov", "[integration][6.5.4][parallel]") {
  vov_void g({{0, 1}, {0, 1}, {0, 2}, {1, 0}, {1, 0}});

  // Count unique (source, target) pairs
  std::set<std::pair<uint64_t, uint64_t>> unique_edges;
  for (auto&& v : vertices(g)) {
    auto vid = vertex_id(g, v);
    for (auto&& e : edges(g, v)) {
      unique_edges.insert({vid, target_id(g, e)});
    }
  }

  REQUIRE(count_edges(g) == 5);      // Total edges
  REQUIRE(unique_edges.size() == 3); // Unique edges: (0,1), (0,2), (1,0)
}

//==================================================================================================
// Phase 6.5.5: Value Type Conversions
//==================================================================================================

TEST_CASE("Value conversion - int to string edge values", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 42}, {1, 2, 99}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::to_string(edge_value(source, e))});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);

  auto edge_rng = edges(target, 0);
  REQUIRE(edge_value(target, *edge_rng.begin()) == "42");
}

TEST_CASE("Value conversion - string to int edge values", "[integration][6.5.5][conversion]") {
  vov_string source({{0, 1, "123"}, {1, 2, "456"}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::stoi(edge_value(source, e))});
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);

  auto edge_rng = edges(target, 0);
  REQUIRE(edge_value(target, *edge_rng.begin()) == 123);
}

TEST_CASE("Value conversion - void to int (default values)", "[integration][6.5.5][conversion]") {
  vov_void source({{0, 1}, {1, 2}, {2, 0}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({
            vertex_id(source, v), target_id(source, e),
            100 // Default value
      });
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);

  for (auto&& v : vertices(target)) {
    for (auto&& e : edges(target, v)) {
      REQUIRE(edge_value(target, e) == 100);
    }
  }
}

TEST_CASE("Value conversion - int to void (discard values)", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 42}, {1, 2, 99}, {2, 0, 77}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vov_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);
}

TEST_CASE("Value conversion - transform int values (* 2)", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), edge_value(source, e) * 2});
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == 20);
}

TEST_CASE("Value conversion - map graph int to string", "[integration][6.5.5][conversion]") {
  mos_int source({{"a", "b", 1}, {"b", "c", 2}});

  std::vector<graph::copyable_edge_t<std::string, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::to_string(edge_value(source, e))});
    }
  }

  mos_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);

  auto e_rng = edges(target, "a");
  REQUIRE(edge_value(target, *e_rng.begin()) == "1");
}

TEST_CASE("Value conversion - map graph void to int", "[integration][6.5.5][conversion]") {
  mos_void source({{"a", "b"}, {"b", "c"}});

  std::vector<graph::copyable_edge_t<std::string, int>> edge_list;
  int                                                   counter = 1;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), counter++});
    }
  }

  mos_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);
}

TEST_CASE("Value conversion - different trait types with conversion", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 5}, {1, 2, 10}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back(
            {vertex_id(source, v), target_id(source, e), "value_" + std::to_string(edge_value(source, e))});
    }
  }

  vofl_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);
}

TEST_CASE("Value conversion - preserve structure discard values", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 1}, {0, 2, 2}, {1, 2, 3}, {2, 0, 4}});

  std::vector<graph::copyable_edge_t<uint64_t, void>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e)});
    }
  }

  vov_void target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == source.size());
  REQUIRE(count_edges(target) == count_edges(source));
}

TEST_CASE("Value conversion - add default values to empty graph", "[integration][6.5.5][conversion]") {
  vov_void source;

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 0);
  REQUIRE(count_edges(target) == 0);
}

TEST_CASE("Value conversion - convert with self-loops", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 0, 111}, {1, 1, 222}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::to_string(edge_value(source, e))});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 2);
  REQUIRE(count_edges(target) == 2);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == "111");
}

TEST_CASE("Value conversion - complex transformation", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      int val = edge_value(source, e);
      edge_list.push_back({
            vertex_id(source, v), target_id(source, e),
            val * val // Square the value
      });
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == 100);

  auto e1 = edges(target, 1);
  REQUIRE(edge_value(target, *e1.begin()) == 400);
}

TEST_CASE("Value conversion - conditional transformation", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 5}, {1, 2, 15}, {2, 0, 25}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      int val = edge_value(source, e);
      edge_list.push_back({vertex_id(source, v), target_id(source, e), val >= 10 ? "high" : "low"});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == "low");

  auto e1 = edges(target, 1);
  REQUIRE(edge_value(target, *e1.begin()) == "high");
}

TEST_CASE("Value conversion - aggregation during copy", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 10}, {1, 2, 20}, {2, 0, 30}});

  int                                                sum = 0;
  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      sum += edge_value(source, e);
      edge_list.push_back({
            vertex_id(source, v), target_id(source, e),
            sum // Running sum
      });
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(sum == 60);
}

TEST_CASE("Value conversion - vofl to vov with transformation", "[integration][6.5.5][conversion]") {
  vofl_int source({{0, 1, 1}, {1, 2, 2}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), edge_value(source, e) + 100});
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == 101);
}

TEST_CASE("Value conversion - multiple edges same value conversion", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 7}, {0, 2, 7}, {1, 2, 7}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back(
            {vertex_id(source, v), target_id(source, e), "lucky_" + std::to_string(edge_value(source, e))});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 3);

  for (auto&& v : vertices(target)) {
    for (auto&& e : edges(target, v)) {
      REQUIRE(edge_value(target, e) == "lucky_7");
    }
  }
}

TEST_CASE("Value conversion - large values", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 1000000}, {1, 2, 2000000}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::to_string(edge_value(source, e))});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == "1000000");
}

TEST_CASE("Value conversion - negative to positive", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, -10}, {1, 2, -20}});

  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::abs(edge_value(source, e))});
    }
  }

  vov_int target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == 10);
}

TEST_CASE("Value conversion - format string values", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 42}, {1, 2, 99}});

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back(
            {vertex_id(source, v), target_id(source, e), "[" + std::to_string(edge_value(source, e)) + "]"});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);

  auto e0 = edges(target, 0);
  REQUIRE(edge_value(target, *e0.begin()) == "[42]");
}

TEST_CASE("Value conversion - chain transformations", "[integration][6.5.5][conversion]") {
  // First: int graph
  vov_int g1({{0, 1, 10}});

  // Convert to string
  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list1;
  for (auto&& v : vertices(g1)) {
    for (auto&& e : edges(g1, v)) {
      edge_list1.push_back({vertex_id(g1, v), target_id(g1, e), std::to_string(edge_value(g1, e))});
    }
  }

  vov_string g2;
  g2.load_edges(edge_list1, std::identity{});

  // Convert back to int with transformation
  std::vector<graph::copyable_edge_t<uint64_t, int>> edge_list2;
  for (auto&& v : vertices(g2)) {
    for (auto&& e : edges(g2, v)) {
      edge_list2.push_back({vertex_id(g2, v), target_id(g2, e), std::stoi(edge_value(g2, e)) * 2});
    }
  }

  vov_int g3;
  g3.load_edges(edge_list2, std::identity{});

  REQUIRE(g3.size() == 2);
  auto e = edges(g3, 0);
  REQUIRE(edge_value(g3, *e.begin()) == 20);
}

TEST_CASE("Value conversion - preserve edge count across conversion", "[integration][6.5.5][conversion]") {
  vov_int source({{0, 1, 1}, {0, 2, 2}, {1, 2, 3}, {2, 0, 4}, {2, 1, 5}});

  size_t original_count = count_edges(source);

  std::vector<graph::copyable_edge_t<uint64_t, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), std::to_string(edge_value(source, e))});
    }
  }

  vov_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(count_edges(target) == original_count);
}

TEST_CASE("Value conversion - map to different container with values", "[integration][6.5.5][conversion]") {
  mos_int source({{"a", "b", 10}, {"b", "c", 20}});

  std::vector<graph::copyable_edge_t<std::string, std::string>> edge_list;
  for (auto&& v : vertices(source)) {
    for (auto&& e : edges(source, v)) {
      edge_list.push_back({vertex_id(source, v), target_id(source, e), "val:" + std::to_string(edge_value(source, e))});
    }
  }

  mol_string target;
  target.load_edges(edge_list, std::identity{});

  REQUIRE(target.size() == 3);
  REQUIRE(count_edges(target) == 2);
}

//==================================================================================================
// Phase 6.5.6: Real-World Graph Examples
//==================================================================================================

TEST_CASE("Real-world - social network basic", "[integration][6.5.6][real_world]") {
  // People (vertices) with friendships (edges)
  mos_void social_graph({{"Alice", "Bob"}, {"Alice", "Charlie"}, {"Bob", "David"}, {"Charlie", "David"}});

  REQUIRE(social_graph.size() == 4);
  REQUIRE(count_edges(social_graph) == 4);

  // Find Alice's friends
  auto                     alice_edges = edges(social_graph, "Alice");
  std::vector<std::string> friends;
  for (auto&& e : alice_edges) {
    friends.push_back(target_id(social_graph, e));
  }

  REQUIRE(friends.size() == 2);
  REQUIRE(std::ranges::find(friends, "Bob") != friends.end());
  REQUIRE(std::ranges::find(friends, "Charlie") != friends.end());
}

TEST_CASE("Real-world - social network degree", "[integration][6.5.6][real_world]") {
  mos_void social_graph({{"Alice", "Bob"}, {"Alice", "Charlie"}, {"Alice", "David"}, {"Bob", "Charlie"}});

  // Alice has 3 friends
  REQUIRE(degree(social_graph, "Alice") == 3);

  // Bob has 1 friend (in outgoing edges)
  REQUIRE(degree(social_graph, "Bob") == 1);

  // David has no outgoing friendships
  REQUIRE(degree(social_graph, "David") == 0);
}

TEST_CASE("Real-world - social network find mutual friends", "[integration][6.5.6][real_world]") {
  mos_void social_graph({{"Alice", "Charlie"}, {"Alice", "David"}, {"Bob", "Charlie"}, {"Bob", "David"}});

  // Find mutual friends of Alice and Bob
  auto alice_edges = edges(social_graph, "Alice");
  auto bob_edges   = edges(social_graph, "Bob");

  std::set<std::string> alice_friends;
  for (auto&& e : alice_edges) {
    alice_friends.insert(target_id(social_graph, e));
  }

  std::vector<std::string> mutual;
  for (auto&& e : bob_edges) {
    if (alice_friends.contains(target_id(social_graph, e))) {
      mutual.push_back(target_id(social_graph, e));
    }
  }

  REQUIRE(mutual.size() == 2);
  REQUIRE(std::ranges::find(mutual, "Charlie") != mutual.end());
  REQUIRE(std::ranges::find(mutual, "David") != mutual.end());
}

TEST_CASE("Real-world - road network with distances", "[integration][6.5.6][real_world]") {
  // Cities (vertices) with roads (edges) and distances (edge values)
  mos_int road_network({{"Seattle", "Portland", 174},
                        {"Portland", "Eugene", 110},
                        {"Seattle", "Spokane", 280},
                        {"Spokane", "Boise", 390}});

  REQUIRE(road_network.size() == 5);
  REQUIRE(count_edges(road_network) == 4);

  // Check distance from Seattle to Portland
  auto seattle_roads = edges(road_network, "Seattle");
  for (auto&& road : seattle_roads) {
    if (target_id(road_network, road) == "Portland") {
      REQUIRE(edge_value(road_network, road) == 174);
    }
  }
}

TEST_CASE("Real-world - road network find neighbors", "[integration][6.5.6][real_world]") {
  mos_int road_network({{"CityA", "CityB", 50}, {"CityA", "CityC", 75}, {"CityB", "CityD", 100}});

  // Find cities directly connected to CityA
  auto                     cityA_roads = edges(road_network, "CityA");
  std::vector<std::string> neighbors;
  for (auto&& road : cityA_roads) {
    neighbors.push_back(target_id(road_network, road));
  }

  REQUIRE(neighbors.size() == 2);
  REQUIRE(std::ranges::find(neighbors, "CityB") != neighbors.end());
  REQUIRE(std::ranges::find(neighbors, "CityC") != neighbors.end());
}

TEST_CASE("Real-world - road network shortest direct route", "[integration][6.5.6][real_world]") {
  mos_int road_network({{"A", "B", 100}, {"A", "C", 50}, {"A", "D", 200}});

  // Find shortest road from A
  auto        a_roads      = edges(road_network, "A");
  int         min_distance = std::numeric_limits<int>::max();
  std::string closest_city;

  for (auto&& road : a_roads) {
    int dist = edge_value(road_network, road);
    if (dist < min_distance) {
      min_distance = dist;
      closest_city = target_id(road_network, road);
    }
  }

  REQUIRE(min_distance == 50);
  REQUIRE(closest_city == "C");
}

TEST_CASE("Real-world - dependency graph tasks", "[integration][6.5.6][real_world]") {
  // Tasks (vertices) with dependencies (edges)
  mos_void dependencies({
        {"Task_A", "Task_B"}, // B depends on A
        {"Task_A", "Task_C"}, // C depends on A
        {"Task_B", "Task_D"}, // D depends on B
        {"Task_C", "Task_D"}  // D depends on C
  });

  REQUIRE(dependencies.size() == 4);
  REQUIRE(count_edges(dependencies) == 4);

  // Task_D has no dependencies it provides
  REQUIRE(degree(dependencies, "Task_D") == 0);

  // Task_A provides dependencies to 2 tasks
  REQUIRE(degree(dependencies, "Task_A") == 2);
}

TEST_CASE("Real-world - dependency graph find prerequisites", "[integration][6.5.6][real_world]") {
  mos_void dependencies({{"Prereq1", "Course"}, {"Prereq2", "Course"}, {"Prereq3", "Course"}});

  // Count how many prerequisites Course has (need to count incoming edges)
  // In this representation, prerequisites point to the course
  size_t prereq_count = 0;
  for (auto&& v : vertices(dependencies)) {
    auto vid = vertex_id(dependencies, v);
    for (auto&& e : edges(dependencies, v)) {
      if (target_id(dependencies, e) == "Course") {
        ++prereq_count;
      }
    }
  }

  REQUIRE(prereq_count == 3);
}

TEST_CASE("Real-world - dependency graph topological ordering check", "[integration][6.5.6][real_world]") {
  mos_void dependencies({{"Build", "Test"}, {"Test", "Deploy"}});

  // Verify dependency chain exists
  REQUIRE(dependencies.size() == 3);

  // Build comes before Test
  auto build_edges = edges(dependencies, "Build");
  bool builds_test = false;
  for (auto&& e : build_edges) {
    if (target_id(dependencies, e) == "Test") {
      builds_test = true;
    }
  }
  REQUIRE(builds_test);

  // Test comes before Deploy
  auto test_edges   = edges(dependencies, "Test");
  bool tests_deploy = false;
  for (auto&& e : test_edges) {
    if (target_id(dependencies, e) == "Deploy") {
      tests_deploy = true;
    }
  }
  REQUIRE(tests_deploy);
}

TEST_CASE("Real-world - citation network", "[integration][6.5.6][real_world]") {
  // Papers (vertices) with citations (edges)
  mos_int citations({{"Paper_A", "Paper_B", 2020}, {"Paper_A", "Paper_C", 2021}, {"Paper_B", "Paper_D", 2022}});

  REQUIRE(citations.size() == 4);

  // Paper_A cites 2 papers
  REQUIRE(degree(citations, "Paper_A") == 2);

  // Check citation years
  auto paperA_cites = edges(citations, "Paper_A");
  for (auto&& cite : paperA_cites) {
    int year = edge_value(citations, cite);
    REQUIRE(year >= 2020);
    REQUIRE(year <= 2022);
  }
}

TEST_CASE("Real-world - web page links", "[integration][6.5.6][real_world]") {
  // Web pages (vertices) with hyperlinks (edges)
  mos_void web({{"index.html", "about.html"},
                {"index.html", "contact.html"},
                {"about.html", "team.html"},
                {"contact.html", "index.html"}});

  REQUIRE(web.size() == 4);

  // index.html has 2 outgoing links
  REQUIRE(degree(web, "index.html") == 2);

  // team.html has no outgoing links
  REQUIRE(degree(web, "team.html") == 0);
}

TEST_CASE("Real-world - organizational hierarchy", "[integration][6.5.6][real_world]") {
  // Employees (vertices) with reporting relationships (edges)
  mos_void org_chart({{"CEO", "VP_Engineering"},
                      {"CEO", "VP_Sales"},
                      {"VP_Engineering", "Engineer1"},
                      {"VP_Engineering", "Engineer2"},
                      {"VP_Sales", "SalesRep1"}});

  REQUIRE(org_chart.size() == 6);

  // CEO has 2 direct reports
  REQUIRE(degree(org_chart, "CEO") == 2);

  // VP_Engineering has 2 direct reports
  REQUIRE(degree(org_chart, "VP_Engineering") == 2);

  // Engineer1 has no direct reports
  REQUIRE(degree(org_chart, "Engineer1") == 0);
}

TEST_CASE("Real-world - airline routes with costs", "[integration][6.5.6][real_world]") {
  // Airports (vertices) with flights (edges) and prices (edge values)
  mos_int flights({{"LAX", "JFK", 350}, {"LAX", "ORD", 200}, {"ORD", "JFK", 150}, {"JFK", "LHR", 500}});

  REQUIRE(flights.size() == 4);
  REQUIRE(count_edges(flights) == 4);

  // Find cheapest flight from LAX
  auto lax_flights = edges(flights, "LAX");
  int  min_price   = std::numeric_limits<int>::max();

  for (auto&& flight : lax_flights) {
    int price = edge_value(flights, flight);
    if (price < min_price) {
      min_price = price;
    }
  }

  REQUIRE(min_price == 200);
}

TEST_CASE("Real-world - recipe ingredients", "[integration][6.5.6][real_world]") {
  // Recipes (vertices) with ingredient dependencies (edges)
  mos_void recipe_deps(
        {{"Cake", "Flour"}, {"Cake", "Eggs"}, {"Cake", "Sugar"}, {"Frosting", "Sugar"}, {"Frosting", "Butter"}});

  REQUIRE(recipe_deps.size() == 6); // Cake, Flour, Eggs, Sugar, Frosting, Butter

  // Cake needs 3 ingredients
  REQUIRE(degree(recipe_deps, "Cake") == 3);

  // Frosting needs 2 ingredients
  REQUIRE(degree(recipe_deps, "Frosting") == 2);
}

TEST_CASE("Real-world - computer network topology", "[integration][6.5.6][real_world]") {
  // Computers (vertices) with network connections (edges) and bandwidth (edge values)
  mos_int network({{"Server1", "Switch1", 1000},
                   {"Server2", "Switch1", 1000},
                   {"Switch1", "Router", 10000},
                   {"Router", "Internet", 1000}});

  REQUIRE(network.size() == 5);

  // Switch1 connects to 1 device (Router)
  REQUIRE(degree(network, "Switch1") == 1);

  // Check Switch1 to Router bandwidth
  auto switch_edges = edges(network, "Switch1");
  for (auto&& conn : switch_edges) {
    if (target_id(network, conn) == "Router") {
      REQUIRE(edge_value(network, conn) == 10000);
    }
  }
}

TEST_CASE("Real-world - gene regulatory network", "[integration][6.5.6][real_world]") {
  // Genes (vertices) with regulatory relationships (edges)
  mos_int gene_network({{"GeneA", "GeneB", 1},  // activates
                        {"GeneA", "GeneC", -1}, // represses
                        {"GeneB", "GeneD", 1}});

  REQUIRE(gene_network.size() == 4);

  // GeneA regulates 2 genes
  REQUIRE(degree(gene_network, "GeneA") == 2);

  // Check activation/repression
  auto geneA_edges = edges(gene_network, "GeneA");
  for (auto&& reg : geneA_edges) {
    int effect = edge_value(gene_network, reg);
    REQUIRE((effect == 1 || effect == -1));
  }
}

TEST_CASE("Real-world - supply chain", "[integration][6.5.6][real_world]") {
  // Companies/locations (vertices) with shipment routes (edges) and lead times (edge values)
  mos_int supply_chain({{"Supplier", "Warehouse", 3},
                        {"Warehouse", "Store1", 1},
                        {"Warehouse", "Store2", 1},
                        {"Warehouse", "Store3", 2}});

  REQUIRE(supply_chain.size() == 5);

  // Warehouse distributes to 3 stores
  REQUIRE(degree(supply_chain, "Warehouse") == 3);

  // Supplier ships to 1 location
  REQUIRE(degree(supply_chain, "Supplier") == 1);
}

TEST_CASE("Real-world - course prerequisites complex", "[integration][6.5.6][real_world]") {
  // Courses (vertices) with prerequisite relationships (edges)
  mos_void courses(
        {{"Math101", "Math201"}, {"Math101", "Physics101"}, {"Math201", "Math301"}, {"Physics101", "Physics201"}});

  REQUIRE(courses.size() == 5);

  // Math101 is prerequisite for 2 courses
  REQUIRE(degree(courses, "Math101") == 2);

  // Math301 has no courses that depend on it (in this representation)
  REQUIRE(degree(courses, "Math301") == 0);
}

TEST_CASE("Real-world - social media followers", "[integration][6.5.6][real_world]") {
  // Users (vertices) with follow relationships (edges)
  mos_void followers({
        {"@alice", "@bob"}, {"@alice", "@charlie"}, {"@bob", "@charlie"}, {"@charlie", "@alice"} // mutual follow
  });

  REQUIRE(followers.size() == 3);
  REQUIRE(count_edges(followers) == 4);

  // @alice follows 2 people
  REQUIRE(degree(followers, "@alice") == 2);

  // Check if @charlie and @alice follow each other
  bool alice_follows_charlie = false;
  bool charlie_follows_alice = false;

  auto alice_edges = edges(followers, "@alice");
  for (auto&& e : alice_edges) {
    if (target_id(followers, e) == "@charlie") {
      alice_follows_charlie = true;
    }
  }

  auto charlie_edges = edges(followers, "@charlie");
  for (auto&& e : charlie_edges) {
    if (target_id(followers, e) == "@alice") {
      charlie_follows_alice = true;
    }
  }

  REQUIRE(alice_follows_charlie);
  REQUIRE(charlie_follows_alice);
}
