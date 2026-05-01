#include "graph/graph.hpp"
#include "graph/views/bfs.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/traits/mol_graph_traits.hpp"
#include "imdb-graph.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <ranges>
#include <variant>

using std::cout;
using std::ranges::iota_view;
using std::vector;
using namespace graph;
using namespace graph::views;

void bacon1() {
  using G = vector<vector<size_t>>;
  G costars{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
            {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
            {1, 4}};

  vector<size_t> bacon_number(size(actors));

  for (auto&& [uv] : edges_bfs(costars, std::size_t{1})) {
    auto u = source_id(costars, uv);
    auto v = target_id(costars, uv);
    bacon_number[v] = bacon_number[u] + 1;
  }

  for(size_t i : iota_view(0u, size(actors))) {
    cout << actors[i] << " has Bacon number " << bacon_number[i] << '\n';
  }
}

void bacon2() {
  using namespace graph::container;

  // A vertex is either a movie or an actor. Use distinct wrapper types so
  // std::variant can discriminate them at runtime.
  struct movie_v {};
  struct actor_v {};
  using vertex_val_t = std::variant<movie_v, actor_v>;

  // Vertex id is the movie/actor name (std::string). Vertices are stored in a
  // std::map (ordered by name) via the mol traits (map + list).
  using traits_t = mol_graph_traits<void,          // edge value
                                    vertex_val_t, // vertex value
                                    void,         // graph value
                                    std::string,  // vertex id
                                    false>;       // not bidirectional
  using G = dynamic_adjacency_graph<traits_t>;

  using vertex_data_t = copyable_vertex_t<std::string, vertex_val_t>;
  using edge_data_t   = copyable_edge_t<std::string, void>;

  G g;

  // Load all movie and actor vertices.
  g.load_vertices(movies, [](const std::string& m) -> vertex_data_t { return {m, vertex_val_t{movie_v{}}}; });
  g.load_vertices(actors, [](const std::string& a) -> vertex_data_t { return {a, vertex_val_t{actor_v{}}}; });

  // Edges from movies_actors. Add both directions so a BFS from an actor can
  // reach co-stars via the movies they share.
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t { auto& [m, a] = e; return {m, a}; });
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t { auto& [m, a] = e; return {a, m}; });

#if 0
  // Show the bipartite structure: each vertex with its kind and neighbors.
  for (auto&& u : vertices(g)) {
    const auto& uid  = vertex_id(g, u);
    const auto& uval = vertex_value(g, u);
    const char* kind = std::holds_alternative<movie_v>(uval) ? "movie" : "actor";
    cout << '[' << kind << "] " << uid << '\n';
    for (auto&& uv : edges(g, u)) {
      cout << "    -> " << target_id(g, uv) << '\n';
    }
  }
  cout << '\n';
#endif

  // Compute Bacon numbers via plain BFS from "Kevin Bacon". The graph uses
  // string vertex ids (map-based), so the index-based edges_bfs view does
  // not apply. Because the graph is bipartite (actor <-> movie), each co-star
  // step traverses two edges (actor -> movie -> actor); an actor's Bacon
  // number is depth/2.
  std::map<std::string, std::size_t> depth;
  std::queue<std::string> q;
  depth["Kevin Bacon"] = 0;
  q.push("Kevin Bacon");

  while (!q.empty()) {
    auto uid = q.front();
    q.pop();
    auto u = find_vertex(g, uid);
    for (auto&& uv : edges(g, *u)) {
      const auto& vid = target_id(g, uv);
      if (depth.try_emplace(vid, depth[uid] + 1).second)
        q.push(vid);
    }
  }

  for (const auto& a : actors) {
    auto it = depth.find(a);
    if (it == depth.end())
      cout << a << " has Bacon number infinity\n";
    else
      cout << a << " has Bacon number " << (it->second / 2) << '\n';
  }
}



int main() {
  cout << "\n--- bacon1 ---\n";
  bacon1();
  cout << "\n--- bacon2 ---\n";
  bacon2();
  return 0;
}
