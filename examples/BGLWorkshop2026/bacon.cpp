#include "graph/graph.hpp"
#include "graph/views/bfs.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/traits/uol_graph_traits.hpp"
#include "graph/container/traits/mol_graph_traits.hpp"
#include "imdb-graph.hpp"

#include "graph/io/dot.hpp"
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

using std::cout;
using std::vector;
using std::string;
using namespace std::ranges;
using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace graph;

// Example dot conversion to jpeg: dot -Tjpeg output/bacon2.dot -o output/bacon2.jpg

// ── bacon1 ──────────────────────────────────────────────────────────────────
// Simple BFS on an unweighted graph with integral vertex ids.
// costars[actor_id] = vector of pre-computed co-star actor_ids.
namespace bacon1 {

void run() {
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

} // namespace bacon1

// ── bacon2 ──────────────────────────────────────────────────────────────────
// Bipartite movie↔actor graph with string vertex ids and unordered_map-based storage.
namespace bacon2 {

// This outputs the bacon2 graph contents for debugging
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

// Shared types: defined at namespace scope so both run() and dot() share the
// same graph type without repeating the template arguments.

enum class vertex_kind { movie, actor };
using traits_t = graph::container::uol_graph_traits<void, vertex_kind, void, string, false>;
using graph_t  = graph::container::dynamic_adjacency_graph<traits_t>;

// Builds the bipartite movie↔actor graph used by bacon2() and bacon2_dot().
// Both directions of each movies_actors edge are loaded so a BFS starting
// from an actor can reach co-stars via the movies they share.
graph_t load() {
  using namespace graph::container;
  using vertex_data_t = copyable_vertex_t<string, vertex_kind>;
  using edge_data_t   = copyable_edge_t<string, void>;

  graph_t g;

  // Load all movie and actor vertices.
  g.load_vertices(movies, [](const string& title) -> vertex_data_t { return {title, vertex_kind::movie}; });
  g.load_vertices(actors, [](const string& name)  -> vertex_data_t { return {name,  vertex_kind::actor};  });

  // Edges from movies_actors.
  // Add both directions so a BFS from an actor can reach co-stars via the movies they share.
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t { auto& [title, name] = e; return {title, name}; });
  g.load_edges(movies_actors, [](const auto& e) -> edge_data_t { auto& [title, name] = e; return {name, title}; });
  return g;
}

// Holds the loaded graph and the BFS depth map returned by eval().
struct eval_result {
  graph_t                                 g;
  std::unordered_map<string, std::size_t> depth; // key = vertex id (string); value = BFS depth
};

// Builds the graph, runs BFS from "Kevin Bacon", and returns both.
// Depth is keyed by vertex id (string) so it works from const graph references.
// Because the graph is bipartite (actor <-> movie), each co-star step traverses
// two edges (actor -> movie -> actor); an actor's Bacon number is depth / 2.
eval_result eval() {
  using G = graph_t;
  G g = load();

  // Compute Bacon numbers via the edges_bfs view from "Kevin Bacon".
  // edges_bfs dispatches its visited tracker on the id type —
  // std::unordered_set for non-integral ids, std::vector<bool> for integral ids.
  vertex_t<G> seed = *find_vertex(g, "Kevin Bacon"s);
  std::unordered_map<string, std::size_t> depth;
  depth[vertex_id(g, seed)] = 0;
  for (auto&& [uv] : graph::views::edges_bfs(g, seed)) {
    const vertex_id_t<G>& u_id = source_id(g, uv); // const string& — stable ref to unordered_map key
    depth[target_id(g, uv)]    = depth[u_id] + 1;
  }
  return {std::move(g), std::move(depth)};
}

// sparse & non-integral vertex id
// bipartite graph: movies <-> actors; edges link each actor to every film they appeared in (and back)
void run() {
  auto [g, depth] = eval();

  // Print results. Bacon number is the length of the shortest path from the seed actor
  // to each actor, divided by 2 for the bipartite graph.
  for (const string& a : actors) {
    auto it = depth.find(a);
    if (it == depth.end())
      cout << a << " has Bacon number infinity\n";
    else
      cout << a << " has Bacon number " << (it->second / 2) << '\n';
  }
}

// bipartite graph with string vertex ids — loads the same graph as run() and
// writes it to a Graphviz DOT file.  Vertices are coloured by kind (movie/actor)
// and actors are labelled with their Bacon number.
// Edges are deduplicated so the bidirectional pairs collapse to single undirected lines.
void dot(const string& filename = BGLWS_OUTPUT_DIR "/bacon2.dot") {
  auto [g, depth] = eval();

  std::ofstream out(filename);
  if (!out)
    throw std::runtime_error("Cannot open: " + filename);

  out << "graph BaconNumbers {\n";

  // Vertices: actors as blue ellipses with Bacon number; movies as yellow boxes.
  for (auto u : vertices(g)) {
    const string& uid      = vertex_id(g, u);
    bool          is_actor = (vertex_value(g, u) == vertex_kind::actor);
    const string  esc      = graph::io::detail::dot_escape(uid);
    if (is_actor) {
      auto   it_d = depth.find(uid);
      string bn   = (it_d != depth.end()) ? std::to_string(it_d->second / 2) : "inf";
      out << "  " << std::quoted(uid)
          << std::format(" [shape=ellipse, style=filled, fillcolor=lightblue, label=\"{}\\n({})\"",
                         esc, bn)
          << "];\n";
    } else {
      out << "  " << std::quoted(uid)
          << std::format(" [shape=box, style=filled, fillcolor=lightyellow, label=\"{}\"]", esc)
          << ";\n";
    }
  }

  // Edges: emit each undirected pair once using min/max on string keys.
  std::set<std::pair<string, string>> emitted;
  for (auto [sid, tid, uv] : graph::views::edgelist(g)) {
    auto key = (sid < tid) ? std::make_pair(sid, tid) : std::make_pair(tid, sid);
    if (emitted.insert(key).second)
      out << "  " << std::quoted(sid) << " -- " << std::quoted(tid) << ";\n";
  }

  out << "}\n";
  cout << "Wrote " << filename << '\n';
}

} // namespace bacon2

// ── Domain types for namespace bacon3 ────────────────────────────────────────
// Defined at file scope (not inside namespace bacon3) so that std::hash can be
// specialised — std can only be extended at namespace scope.

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

// ── bacon3 ──────────────────────────────────────────────────────────────────
// Bipartite movie↔actor graph using a std::variant as the vertex id.
namespace bacon3 {

void run() {
  using namespace graph::container;

  // movie and actor are defined at file scope (not inside this namespace) so that std::hash
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

} // namespace bacon3


int main() {
  cout << "\n--- bacon1 ---\n";
  bacon1::run();
  cout << "\n--- bacon2 ---\n";
  bacon2::run();
  cout << "\n--- bacon2_dot ---\n";
  bacon2::dot();
  cout << "\n--- bacon3 ---\n";
  bacon3::run();
  return 0;
}
