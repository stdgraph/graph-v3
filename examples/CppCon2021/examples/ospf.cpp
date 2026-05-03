//
//  Authors
//  Copyright
//  License
//  No Warranty Disclaimer
//

#include <iomanip>
#include <iostream>
#include <list>

#include "graph/algorithm/dijkstra_shortest_paths.hpp"
#include "ospf-graph.hpp"
#include "utilities.hpp"

int main() {

  static_assert(graph::adjacency_list<decltype(ospf_index_adjacency_list)>);

  // ── ospf_index_adjacency_list: vector<vector<tuple<size_t,size_t>>> ──────
  // Each edge tuple is {target_id, weight}.
  // In v3: init_shortest_paths takes the graph first; dijkstra uses
  // container_value_fn to adapt vectors into (g, uid) -> value& functions,
  // and the weight function signature is (const G&, const edge_t<G>&).

  std::vector<size_t> d(size(ospf_index_adjacency_list));
  std::vector<size_t> p(size(ospf_index_adjacency_list));
  graph::init_shortest_paths(ospf_index_adjacency_list, d, p);
  graph::dijkstra_shortest_paths(
        ospf_index_adjacency_list,
        std::size_t{5},
        graph::container_value_fn(d),
        graph::container_value_fn(p),
        [](const auto& /*g*/, const auto& uv) { return std::get<1>(*uv.value()); });

  std::cout << "----------------" << std::endl;
  std::cout << "Contents of ospf_index_adjacency_list (the correct answer)" << std::endl;

  for (size_t i = 0; i < size(ospf_vertices); ++i) {
    std::cout << std::setw(6) << ospf_vertices[i] << std::setw(6) << d[i] << std::endl;
  }

  std::cout << "----------------" << std::endl;
  std::cout << "Results from make_property_graph(osp_vertices)" << std::endl;

  // ── make_property_graph: vector<vector<tuple<size_t,size_t>>> ────────────
  // Each edge tuple is {target_id, weight} (property = weight).

  auto G = make_property_graph(ospf_vertices, ospf_edges, true);

  // Alternatively
  auto H = make_property_graph<decltype(ospf_vertices), decltype(ospf_edges),
                               std::vector<std::list<std::tuple<size_t, size_t>>>>(ospf_vertices, ospf_edges, true);
  auto I = make_property_graph<decltype(ospf_vertices), decltype(ospf_edges),
                               std::vector<std::vector<std::tuple<size_t, size_t>>>>(ospf_vertices, ospf_edges, true);

  static_assert(graph::adjacency_list<decltype(G)>);

  p.resize(graph::num_vertices(G));
  std::vector<size_t> e(graph::num_vertices(G));
  graph::init_shortest_paths(G, e, p);
  graph::dijkstra_shortest_paths(
        G,
        std::size_t{5},
        graph::container_value_fn(e),
        graph::container_value_fn(p),
        [](const auto& /*g*/, const auto& uv) { return std::get<1>(*uv.value()); });

  bool pass = true;
  for (size_t i = 0; i < size(ospf_vertices); ++i) {
    std::cout << std::setw(6) << ospf_vertices[i] << std::setw(6) << e[i] << std::endl;
    if (e[i] != d[i])
      pass = false;
  }
  std::cout << (pass ? "***PASS***" : "***FAIL***") << std::endl;

  std::cout << "----------------" << std::endl;
  std::cout << "Results from make_index_graph(osp_vertices)" << std::endl;

  // ── make_index_graph: vector<vector<tuple<size_t,size_t>>> ───────────────
  // Each edge tuple is {target_id, edge_index_into_ospf_edges}.
  // Access the original weight via ospf_edges[edge_index].

  auto J = make_index_graph(ospf_vertices, ospf_edges, true);

  p.resize(graph::num_vertices(J));
  std::vector<size_t> f(graph::num_vertices(J));
  graph::init_shortest_paths(J, f, p);
  graph::dijkstra_shortest_paths(
        J,
        std::size_t{5},
        graph::container_value_fn(f),
        graph::container_value_fn(p),
        [](const auto& /*g*/, const auto& uv) {
          // *uv.value() = tuple<target_id, edge_index>
          auto edge_index = std::get<1>(*uv.value());
          return std::get<2>(ospf_edges[edge_index]);
        });

  bool pass2 = true;
  for (size_t i = 0; i < size(ospf_vertices); ++i) {
    std::cout << std::setw(6) << ospf_vertices[i] << std::setw(6) << e[i] << std::endl;
    if (e[i] != d[i])
      pass2 = false;
  }
  std::cout << (pass2 ? "***PASS***" : "***FAIL***") << std::endl;

  return 0;
}
