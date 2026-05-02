#include "graph/graph.hpp"
#include "graph/views/bfs.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/traits/uol_graph_traits.hpp"
#include "graph/container/traits/mol_graph_traits.hpp"
#include "imdb-graph.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>
#include <variant>
#include <algorithm>

using std::cout;
using std::vector;
using std::string;
using namespace std::ranges;
using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace graph;

// simple BFS on an unweighted graph with integral vertex ids
// costars[actor_id] = vector of pre-computed co-star actor_ids
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

// This outputs the bacon2() graph contents for debugging
#if 0
  // Show the bipartite structure: each vertex with its kind and neighbors.
  for (auto&& u : vertices(g)) {
    const auto& uid  = vertex_id(g, u);
    const auto& uval = vertex_value(g, u);
    const char* kind = (uval == vertex_kind::movie) ? "movie" : "actor";
    cout << '[' << kind << "] " << uid << '\n';
    for (auto&& uv : edges(g, u)) {
      cout << "    -> " << target_id(g, uv) << '\n';
    }
  }
  cout << '\n';
#endif

// sparse & non-integral vertex id
// bipartite graph: movies <-> actors; edges link each actor to every film they appeared in (and back)
void bacon2() {
  using namespace graph::container;

  // A vertex is either a movie or an actor.
  enum class vertex_kind { movie, actor };
  using vertex_val_t = vertex_kind;

  // Vertex id is the movie/actor name (string). Vertices are stored in an
  // std::unordered_map (hash-based) via the uol traits (unordered_map + list).
  using traits_t = uol_graph_traits<void,         // edge value
                                    vertex_val_t, // vertex value
                                    void,         // graph value
                                    string,       // vertex id
                                    false>;       // not bidirectional
  using G        = dynamic_adjacency_graph<traits_t>;
  G g;

  // Load all movie and actor vertices.
  using vertex_data_t = copyable_vertex_t<string, vertex_val_t>;
  g.load_vertices(movies, [](const string& title) -> vertex_data_t { return {title, vertex_kind::movie}; });
  g.load_vertices(actors, [](const string& name) -> vertex_data_t { return {name, vertex_kind::actor}; });

  // Edges from movies_actors. 
  // Add both directions so a BFS from an actor can reach co-stars via the movies they share.
  using edge_data_t   = copyable_edge_t<string, void>;
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t { auto& [title, name] = e; return {title, name}; });
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t { auto& [title, name] = e; return {name, title}; });

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

  VDesc seed  = *find_vertex(g, "Kevin Bacon"s);
  depth[seed] = 0;
  for (auto&& [uv] : graph::views::edges_bfs(g, seed)) {
    const vertex_id_t<G>&  u_id = source_id(g, uv);   // const string& — stable ref into the edge descriptor
    vertex_t<G>            v    = *find_vertex(g, target_id(g, uv));
    vertex_t<G>            u    = *find_vertex(g, u_id);
    depth[v]                    = depth[u] + 1;
  }

  // Print results. Bacon number is the length of the shortest path from the seed actor 
  // to each actor, divided by 2 for the bipartite graph.
  for (const string& a : actors) {
    auto it = depth.find(*find_vertex(g, a));
    if (it == depth.end())
      cout << a << " has Bacon number infinity\n";
    else
      cout << a << " has Bacon number " << (it->second / 2) << '\n';
  }
}

// ── Domain types for bacon3 ──────────────────────────────────────────────────
// Defined at file scope so that std::hash can be specialised (local classes
// cannot appear as the subject of a std::hash specialisation because namespace
// std can only be extended at namespace scope).

class movie {
public:
  movie() = default; // required by graph vertex storage (value_ = value_type())
  movie(const string& title) : title_(title) {}
  const string& title() const { return title_; }
  // Enables std::variant<monostate,movie,actor>::operator< and std::hash<variant_t>
  auto operator<=>(const movie& o) const { return title_ <=> o.title_; }
  bool operator==(const movie& o)  const { return title_ == o.title_; }
protected:
  string title_;
};

class actor {
public:
  actor() = default;
  actor(const string& name) : name_(name) {}
  const string& name() const { return name_; }
  auto operator<=>(const actor& o) const { return name_ <=> o.name_; }
  bool operator==(const actor& o)  const { return name_ == o.name_; }
protected:
  string name_;
};

namespace std {
template <>
struct hash<movie> {
  size_t operator()(const movie& m) const noexcept {
    return hash<string>{}(m.title());
  }
};
template <>
struct hash<actor> {
  size_t operator()(const actor& a) const noexcept {
    return hash<string>{}(a.name());
  }
};
} // namespace std

void bacon3() {
  using namespace graph::container;

  // movie and actor are defined at file scope (see above) so that std::hash
  // can be specialised — a requirement of the BFS visited tracker when
  // variant_t is used as the vertex id with mol_graph_traits.

  // ── Design Options ──────────────────────────────────────────────────────────
  //
  // movies_actors encodes a bipartite relationship between movies and actors.
  // Two independent choices produce four concrete designs:
  //
  //   Topology:   Bipartite  — movies AND actors are vertices; edges link each
  //                            actor to every film they appeared in (and back).
  //                            bacon_number = BFS depth / 2.
  //               Co-star    — actors only; an edge links two actors who share a
  //                            film and carries the movie object as its value.
  //                            bacon_number = BFS depth directly.
  //
  //   Vertex id:  string   — human-readable name; uol_graph_traits
  //                               (unordered_map); visited tracker → unordered_set.
  //               size_t        — dense integer; vector<bool> bitset tracker;
  //                               fastest BFS; bipartite needs offset encoding.
  //               variant_t     — domain object IS the key; mol_graph_traits
  //                               (std::map); requires operator<=> on movie/actor.
  //                               Movies sort before actors (by variant index).
  //
  // ┌──────┬──────────────────────────┬──────────────┬──────────────────────────────────────┐
  // │      │ Topology                 │ Vertex id    │ Graph trait / notes                  │
  // ├──────┼──────────────────────────┼──────────────┼──────────────────────────────────────┤
  // │  1   │ Bipartite (movie↔actor)  │ string  │ uol_graph_traits (unordered_map+list)│
  // │  2   │ Bipartite (movie↔actor)  │ size_t       │ vol_graph_traits (vector+list)       │
  // │  3   │ Bipartite (movie↔actor)  │ variant_t    │ mol_graph_traits (map+list)  ◄──here │
  // │  4   │ Co-star   (actor↔actor)  │ string  │ uol_graph_traits                     │
  // │  5   │ Co-star   (actor↔actor)  │ size_t       │ vol_graph_traits                     │
  // └──────┴──────────────────────────┴──────────────┴──────────────────────────────────────┘

  // ── variant_t as vertex id ──────────────────────────────────────────────────
  //
  // std::variant<A,B,C>::operator< is defined by the standard (C++17):
  //   1. A valueless variant is less than any non-valueless variant.
  //   2. If indices differ, the variant with the lower index is less.
  //   3. If indices match, the contained values are compared with <.
  //
  // So monostate < movie < actor (by index order), and within each alternative
  // objects are compared by title/name respectively via the operator<=> above.
  //
  // mol_graph_traits uses std::map<VId, vertex_type> which requires only
  // operator< on VId — no hash needed.  Because variant_t is now the key,
  // vertex_value_type can be void: the domain object is already the key.
  //
  // Trade-off vs. string key:
  //   + find_vertex(g, movie{"Top Gun"}) is a single O(log V) map lookup.
  //   + No string↔object mapping needed; the variant directly identifies
  //     whether a vertex is a movie or an actor.
  //   − Requires operator<=> on the domain classes (non-intrusive alternatives
  //     are also possible — see the custom comparator note below).
  //   − std::map lookup (O(log V)) is slower than unordered_map (O(1) avg).
  //
  // Custom comparator alternative (no changes to movie/actor):
  //   If modifying the domain classes is not allowed, mol_graph_traits cannot
  //   be used as-is (it passes VId directly to std::map with default less<>).
  //   Instead, a specialisation of std::less<variant_t> or a custom Traits
  //   struct that parameterises the map comparator would be required.
  //   That is straightforward but out-of-scope here since the member variables
  //   (and by extension the class names) are retained and operator<=> only
  //   adds a public interface without changing the member layout.

  // std::monostate is still listed first for default-constructibility in
  // internal graph storage, but in practice every vertex holds movie or actor.
  using variant_t = std::variant<std::monostate, movie, actor>;

  using traits3 = mol_graph_traits<void,       // edge value — void (bipartite)
                                   void,       // vertex value — void (key IS the object)
                                   void,       // graph value
                                   variant_t,  // vertex id = the domain object
                                   false>;     // not bidirectional
  using G3 = dynamic_adjacency_graph<traits3>;

  // Type aliases for load_vertices / load_edges
  using VD3 = copyable_vertex_t<variant_t, void>;
  using ED3 = copyable_edge_t<variant_t, void>;

  G3 g;
  // Load movie vertices — each variant_t{movie{title}} is the map key.
  g.load_vertices(movies, [](const string& m) -> VD3 {
    return {variant_t{movie{m}}};
  });
  // Load actor vertices.
  g.load_vertices(actors, [](const string& a) -> VD3 {
    return {variant_t{actor{a}}};
  });
  // Bipartite edges (both directions) from movies_actors.
  g.load_edges(movies_actors, [](const auto& e) -> ED3 {
    return {variant_t{movie{std::get<0>(e)}}, variant_t{actor{std::get<1>(e)}}}; // movie → actor
  });
  g.load_edges(movies_actors, [](const auto& e) -> ED3 {
    return {variant_t{actor{std::get<1>(e)}}, variant_t{movie{std::get<0>(e)}}}; // actor → movie
  });

  // BFS from Kevin Bacon — seed is a vertex descriptor found by variant_t key.
  using VDesc = vertex_t<G3>;
  std::map<VDesc, std::size_t> depth; // map because VDesc needs operator<

  VDesc seed = *find_vertex(g, variant_t{actor{"Kevin Bacon"}});
  depth[seed] = 0;

  for (auto&& [uv] : graph::views::edges_bfs(g, seed)) {
    auto u   = *find_vertex(g, source_id(g, uv));
    auto v   = *find_vertex(g, target_id(g, uv));
    depth[v] = depth[u] + 1;
  }

  // Print results — extract the actor name from the variant key.
  for (const auto& a : actors) {
    auto it = depth.find(*find_vertex(g, variant_t{actor{a}}));
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
  cout << "\n--- bacon3 ---\n";
  bacon3();
  return 0;
}
