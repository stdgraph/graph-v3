//
//  CppCon 2022 — Germany Routes Example
//  Refactored from graph-v2 to graph-v3.
//
//  v2→v3 changes:
//    • Removed Catch2 dependency; uses a plain main()
//    • Removed dijkstra_clrs; uses dijkstra_shortest_paths + init_shortest_paths
//    • Weight lambdas now take (const G&, const edge_t<G>&) instead of edge_reference_t<G>
//    • vertex_reference_t<G> replaced by auto
//    • size(vertices(g)) replaced by graph::num_vertices(g)
//    • init_console() stub added (was Windows-only UTF-8 setup)
//

#include "graphviz_output.hpp"
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include "graph/algorithm/dijkstra_shortest_paths.hpp"
#include "rr_adaptor.hpp"
#include <iostream>
#include <list>
#include <cassert>

using namespace std::ranges;
using namespace graph;
using namespace graph::views;
using namespace std::literals;

using std::cout;
using std::endl;
using std::string;
using std::string_view;
using std::pair;
using std::tie;
using std::vector;
using std::list;

// init_console() — on Windows this enables UTF-8 output; on Linux it is a no-op.
void init_console() {}

using city_id_type = uint32_t;

// ── Output helpers ────────────────────────────────────────────────────────────
// In v3 vertex_t<G> is a vertex_descriptor, not a raw reference.  We look up
// city names through vertex_value(g, u) with the descriptor coming from the
// vertexlist / find_vertex machinery.  The two helper structs below carry both
// the graph and the city vertex-id so they can be streamed conveniently.

template <typename G>
struct city_id_fmt {
  const G&       g;
  vertex_id_t<G> id;

  city_id_fmt(const G& graph, vertex_id_t<G> uid) : g(graph), id(uid) {}

  template <typename OS>
  friend OS& operator<<(OS& os, const city_id_fmt& rhs) {
    // *find_vertex(g, id) → vertex_descriptor; vertex_value() → string name
    os << vertex_value(rhs.g, *find_vertex(rhs.g, rhs.id)) << " [" << rhs.id << "]";
    return os;
  }
};

template <typename G>
struct city_fmt {
  const G&       g;
  vertex_id_t<G> id;

  city_fmt(const G& graph, vertex_id_t<G> uid) : g(graph), id(uid) {}

  template <typename OS>
  friend OS& operator<<(OS& os, const city_fmt& rhs) {
    os << vertex_value(rhs.g, *find_vertex(rhs.g, rhs.id));
    return os;
  }
};

// Convenience wrappers to preserve the original names used in the presentation.
template <typename G> city_id_fmt<G> city_id(G& g, vertex_id_t<G> uid) { return {g, uid}; }
template <typename G> city_fmt<G>    city(G& g, vertex_id_t<G> uid)    { return {g, uid}; }

// ── Main ──────────────────────────────────────────────────────────────────────

int main() {
  init_console();

  // ── Vertex data ────────────────────────────────────────────────────────────
  using city_name_type  = string;
  using city_names_type = vector<city_name_type>;
  city_names_type city_names = {
        "Frankfürt", "Mannheim", "Karlsruhe", "Augsburg", "Würzburg",
        "Nürnberg",  "Kassel",   "Erfurt",    "München",  "Stuttgart"};

  // ── Edge data ──────────────────────────────────────────────────────────────
  // copyable_edge_t<VId, W> = edge_data<VId, true, void, W> = {source_id, target_id, value}
  using route_data            = copyable_edge_t<city_id_type, double>;
  vector<route_data> routes_doubled = {
        {0, 1, 85.0},  {0, 4, 217.0}, {0, 6, 173.0},
        {1, 0, 85.0},  {1, 2, 80.0},
        {2, 1, 80.0},  {2, 3, 250.0},
        {3, 2, 250.0}, {3, 8, 84.0},
        {4, 0, 217.0}, {4, 5, 103.0}, {4, 7, 186.0},
        {5, 4, 103.0}, {5, 8, 167.0}, {5, 9, 183.0},
        {6, 0, 173.0}, {6, 8, 502.0},
        {7, 4, 186.0},
        {8, 3, 84.0},  {8, 5, 167.0}, {8, 6, 502.0},
        {9, 5, 183.0},
  };

  // ── Graph ──────────────────────────────────────────────────────────────────
  struct route {          // edge stored in the adjacency list
    city_id_type target_id = 0;
    double       distance  = 0.0; // km
  };
  using AdjList = vector<list<route>>;
  using G       = rr_adaptor<AdjList, city_names_type>;

  G g(city_names, routes_doubled);

  city_id_type frankfurt_id = 0;

  // ── Traverse vertices & outgoing edges ─────────────────────────────────────
  cout << "Traverse the vertices & outgoing edges" << endl;
  for (auto&& [uid, u] : vertexlist(g)) {
    cout << city_id(g, uid) << endl;
    for (auto&& [vid, uv] : graph::views::incidence(g, uid)) {
      cout << "   --> " << city_id(g, vid) << endl;
    }
  }

  // ── Shortest Paths — segment count ─────────────────────────────────────────
  {
    // Weight function: each hop costs 1 segment.
    // v3 signature: (const G&, const edge_t<G>&) → W
    auto weight_1 = [](const auto& /*g*/, const auto& /*uv*/) -> int { return 1; };

    vector<int>            distances(graph::num_vertices(g));
    vector<vertex_id_t<G>> predecessor(graph::num_vertices(g));
    graph::init_shortest_paths(g, distances, predecessor);
    graph::dijkstra_shortest_paths(g, frankfurt_id,
                                   graph::container_value_fn(distances),
                                   graph::container_value_fn(predecessor),
                                   weight_1);

    cout << "Shortest distance (segments) from " << city_id(g, frankfurt_id) << endl;
    for (vertex_id_t<G> uid = 0; uid < graph::num_vertices(g); ++uid)
      if (distances[uid] > 0)
        cout << "  --> " << city_id(g, uid) << " - " << distances[uid] << " segments" << endl;
  }

  // ── Shortest Paths — km ────────────────────────────────────────────────────
  {
    // Weight function: return the route distance (km) from the edge descriptor.
    // *uv.value() = route struct; graph::edge_value(g, uv) returns route.distance.
    auto weight = [](const auto& g2, const auto& uv) { return graph::edge_value(g2, uv); };

    vector<double>         distances(graph::num_vertices(g));
    vector<vertex_id_t<G>> predecessor(graph::num_vertices(g));
    graph::init_shortest_paths(g, distances, predecessor);
    graph::dijkstra_shortest_paths(g, frankfurt_id,
                                   graph::container_value_fn(distances),
                                   graph::container_value_fn(predecessor),
                                   weight);

    // Print the name of Frankfurt using vertex_value + find_vertex
    auto frankfurt = *find_vertex(g, frankfurt_id);
    cout << "Shortest distance (km) from " << vertex_value(g, frankfurt) << endl;
    for (vertex_id_t<G> uid = 0; uid < graph::num_vertices(g); ++uid) {
      if (distances[uid] > 0)
        cout << "  --> " << city_id(g, uid) << " - " << distances[uid] << "km" << endl;
    }

    // Find farthest city
    vertex_id_t<G> farthest_id   = frankfurt_id;
    double         farthest_dist = 0.0;
    for (vertex_id_t<G> uid = 0; uid < graph::num_vertices(g); ++uid) {
      if (distances[uid] > farthest_dist) {
        farthest_dist = distances[uid];
        farthest_id   = uid;
      }
    }

    cout << "The farthest city from " << city(g, frankfurt_id) << " is "
         << city(g, farthest_id) << " at " << distances[farthest_id] << "km" << endl;
    cout << "The shortest path from " << city(g, farthest_id)
         << " to " << city(g, frankfurt_id) << " is:\n  ";
    for (vertex_id_t<G> uid = farthest_id; uid != frankfurt_id; uid = predecessor[uid]) {
      if (uid != farthest_id)
        cout << " -- ";
      cout << city_id(g, uid);
    }
    cout << " -- " << city_id(g, frankfurt_id) << endl;
  }

  return 0;
}
