#include "graph/graph.hpp"
#include "graph/views/bfs.hpp"
#include "imdb-graph.hpp"

#include <iostream>
#include <string>
#include <vector>

std::vector<std::vector<size_t>> costars{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
                                         {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
                                         {1, 4}};

int main() {

  std::vector<size_t> bacon_number(size(actors));

  // In v3, edges_bfs yields [uv] (edge descriptor only).
  // Extract source and target IDs from the edge descriptor via CPOs.
  for (auto&& [uv] : graph::views::edges_bfs(costars, std::size_t{1})) {
    auto u = graph::source_id(costars, uv);
    auto v = graph::target_id(costars, uv);
    bacon_number[v] = bacon_number[u] + 1;
  }

  for (size_t i = 0; i < size(actors); ++i) {
    std::cout << actors[i] << " has Bacon number " << bacon_number[i] << std::endl;
  }

  return 0;
}
