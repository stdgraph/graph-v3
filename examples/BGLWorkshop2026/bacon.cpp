#include "graph/graph.hpp"
#include "graph/views/bfs.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/traits/uol_graph_traits.hpp"
#include "imdb-graph.hpp"

#include <functional>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include <ranges>
#include <variant>
#include <algorithm>

using std::cout;
using std::vector;
using namespace std::ranges;
using namespace graph;

// simple BFS on an unweighted graph with integral vertex ids
void bacon1() {
  // actors only
  using vid_t              = size_t; // vertex id type
  constexpr vid_t infinity = std::numeric_limits<vid_t>::max();
  const vid_t     seed     = static_cast<vid_t>(std::ranges::find(actors, "Kevin Bacon") - begin(actors));

  using G = vector<vector<vid_t>>;
  G costars{{1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
            {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
            {1, 4}};

  vector<vid_t> bacon_number(size(actors), infinity);
  bacon_number[seed] = 0;

  // Evaluate Bacon numbers via BFS from source. The graph is unweighted, so the shortest
  for (auto&& [uv] : graph::views::edges_bfs(costars, seed)) {
    auto uid          = source_id(costars, uv);
    auto vid          = target_id(costars, uv);
    bacon_number[vid] = bacon_number[uid] + 1;
  }

  // Print results. Bacon number is the length of the shortest path from the seed actor to each actor.
  for (vid_t uid = 0; uid < size(actors); ++uid) {
    if (bacon_number[uid] == infinity)
      cout << actors[uid] << " has Bacon number infinity\n";
    else
      cout << actors[uid] << " has Bacon number " << bacon_number[uid] << '\n';
  }
}

// sparse & non-integral vertex id
void bacon2() {
  using namespace graph::container;

  // A vertex is either a movie or an actor. Use distinct wrapper types so
  // std::variant can discriminate them at runtime.
  struct movie_v {};
  struct actor_v {};
  using vertex_val_t = std::variant<movie_v, actor_v>;

  // Vertex id is the movie/actor name (std::string). Vertices are stored in an
  // std::unordered_map (hash-based) via the uol traits (unordered_map + list).
  using traits_t = uol_graph_traits<void,         // edge value
                                    vertex_val_t, // vertex value
                                    void,         // graph value
                                    std::string,  // vertex id
                                    false>;       // not bidirectional
  using G        = dynamic_adjacency_graph<traits_t>;

  using vertex_data_t = copyable_vertex_t<std::string, vertex_val_t>;
  using edge_data_t   = copyable_edge_t<std::string, void>;

  G g;

  // Load all movie and actor vertices.
  g.load_vertices(movies, [](const std::string& m) -> vertex_data_t { return {m, vertex_val_t{movie_v{}}}; });
  g.load_vertices(actors, [](const std::string& a) -> vertex_data_t { return {a, vertex_val_t{actor_v{}}}; });

  // Edges from movies_actors. Add both directions so a BFS from an actor can
  // reach co-stars via the movies they share.
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t {
    auto& [m, a] = e;
    return {m, a};
  });
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t {
    auto& [m, a] = e;
    return {a, m};
  });

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

  // Compute Bacon numbers via the edges_bfs view from "Kevin Bacon".
  // The graph uses string vertex ids (unordered_map-based); edges_bfs now
  // dispatches its visited tracker on the id type — std::unordered_set for
  // non-integral ids, std::vector<bool> for integral ids — so the same
  // view works for both dense and sparse graphs.
  //
  // Because the graph is bipartite (actor <-> movie), each co-star step
  // traverses two edges (actor -> movie -> actor); an actor's Bacon number
  // is depth/2.
  using VDesc = vertex_t<G>;
  std::unordered_map<VDesc, std::size_t> depth;

  VDesc seed  = *find_vertex(g, std::string{"Kevin Bacon"});
  depth[seed] = 0;

  for (auto&& [uv] : graph::views::edges_bfs(g, seed)) {
    auto u_id = source_id(g, uv);
    auto v    = *find_vertex(g, target_id(g, uv));
    auto u    = *find_vertex(g, u_id);
    depth[v]  = depth[u] + 1;
  }

  for (const auto& a : actors) {
    auto it = depth.find(*find_vertex(g, a));
    if (it == depth.end())
      cout << a << " has Bacon number infinity\n";
    else
      // Divide by 2: the bipartite graph counts both actor→movie and movie→actor
      // hops, so depth 2 = Bacon number 1 (one shared movie).
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
