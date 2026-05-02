// france_routes.cpp
// BGL Workshop 2026 — France Routes Example
//
// Demonstrates:
//   - Loading a directed weighted road graph from JSON
//   - Running Dijkstra's shortest paths from Paris
//   - Reconstructing and printing shortest paths to each city

#include "graph/algorithm/dijkstra_shortest_paths.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/traits/vom_graph_traits.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using std::cout;
using std::string;
using std::vector;
using namespace graph;
using namespace graph::views;
using namespace std::string_literals;

// ── France Routes ──────────────────────────────────────────────────────────────
// Directed weighted graph of French city road connections.
// Vertex IDs are contiguous uint32_t indices; vertex values store the city name
// for reverse lookup.  Edge values hold distance (km) and road name.

namespace france {

using vid_t = uint32_t;

struct route_t {
  int    distance_km = 0;
  string road;              // e.g. "A1", "A11 -> N157"
};

// Vertex value = city name string; edge value = route_t (distance + road label).
using traits_t = container::vom_graph_traits<route_t, string, void, vid_t, false>;
using graph_t  = container::dynamic_adjacency_graph<traits_t>;

struct load_result {
  graph_t                           g;
  std::unordered_map<string, vid_t> city_id; // name -> vertex id
};

// Load france_routes.json and return the graph with the city name-to-id map.
// Cities are assigned contiguous integer IDs in the order they appear in "cities".
// Each route object becomes a directed edge carrying a route_t value.
load_result load(const string& filename = BGLWS_DATA_DIR "/france_routes.json") {
  std::ifstream f(filename);
  if (!f)
    throw std::runtime_error("Cannot open file: "s + filename);

  string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

  // ── Parse city names ──────────────────────────────────────────────────────
  // Capture the "cities" array: [ "City1", "City2", ... ]
  static const std::regex cities_re(R"re("cities"\s*:\s*\[([^\]]*)\])re");
  static const std::regex name_re(R"re("([^"]+)")re");

  std::unordered_map<string, vid_t> city_id;
  vector<string>                    city_names;

  std::smatch cm;
  if (std::regex_search(content, cm, cities_re)) {
    string cities_block = cm[1].str();
    for (auto it  = std::sregex_iterator(cities_block.begin(), cities_block.end(), name_re),
              end = std::sregex_iterator{};
         it != end; ++it) {
      string name            = (*it)[1].str();
      city_id[name]          = static_cast<vid_t>(city_names.size());
      city_names.push_back(name);
    }
  }

  // ── Load vertices (city name as vertex value) ─────────────────────────────
  using vertex_data = copyable_vertex_t<vid_t, string>;
  vector<vertex_data> vv;
  vv.reserve(city_names.size());
  for (vid_t i = 0; i < static_cast<vid_t>(city_names.size()); ++i)
    vv.push_back({i, city_names[i]});

  graph_t g;
  g.load_vertices(vv);

  // ── Parse and load edges ──────────────────────────────────────────────────
  // Note: named raw-string delimiter (re) avoids collision with ")" in the pattern.
  static const std::regex route_re(
      R"re(\{\s*"from"\s*:\s*"([^"]+)"\s*,\s*"to"\s*:\s*"([^"]+)"\s*,\s*"distance_km"\s*:\s*(\d+)\s*,\s*"route"\s*:\s*"([^"]+)"\s*\})re");

  using edge_data = copyable_edge_t<vid_t, route_t>;
  vector<edge_data> edges;

  for (auto it  = std::sregex_iterator(content.begin(), content.end(), route_re),
            end = std::sregex_iterator{};
       it != end; ++it) {
    auto& m    = *it;
    vid_t src  = city_id.at(m[1].str());
    vid_t tgt  = city_id.at(m[2].str());
    int   dist = std::stoi(m[3].str());
    edges.push_back({src, tgt, route_t{dist, m[4].str()}});
  }

  g.load_edges(edges);
  return {std::move(g), std::move(city_id)};
}

void run(const string& filename = BGLWS_DATA_DIR "/france_routes.json") {
  using G = graph_t;
  auto [g, city_id] = load(filename);

  // Helper: city name from vertex id.
  auto city_name = [&](vid_t uid) -> string {
    return vertex_value(g, *find_vertex(g, uid));
  };

  const vid_t source_id = city_id.at("Paris");

  // Distance and predecessor vectors indexed by vertex id.
  vector<int>            distances(num_vertices(g));
  vector<vertex_id_t<G>> predecessors(num_vertices(g));
  init_shortest_paths(g, distances, predecessors);

  dijkstra_shortest_paths(g, source_id,
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          [](const auto& g2, const auto& uv) {
                            return edge_value(g2, uv).distance_km;
                          });

  // ── Print all distances, sorted nearest-first ──────────────────────────────
  vector<std::pair<int, vid_t>> by_dist;
  by_dist.reserve(num_vertices(g));
  for (vid_t uid = 0; uid < static_cast<vid_t>(num_vertices(g)); ++uid) {
    if (uid != source_id)
      by_dist.emplace_back(distances[uid], uid);
  }
  std::ranges::sort(by_dist); // sort by (distance, uid) — default pair comparison

  cout << "Shortest road distances from " << city_name(source_id) << ":\n";
  for (auto& [d, uid] : by_dist) {
    if (d == infinite_distance<int>())
      cout << "  " << city_name(uid) << ": unreachable\n";
    else
      cout << "  " << city_name(uid) << ": " << d << " km\n";
  }

  // ── Reconstruct shortest path from Paris to Nice ────────────────────────────
  const vid_t dest_id = city_id.at("Nice");
  cout << "\nShortest path: " << city_name(source_id) << " -> " << city_name(dest_id) << "\n";

  vector<vertex_id_t<G>> path;
  for (auto cur = static_cast<vertex_id_t<G>>(dest_id);
       cur != static_cast<vertex_id_t<G>>(source_id);
       cur = predecessors[cur]) {
    if (path.size() >= num_vertices(g)) {
      cout << "  (no path found)\n";
      return;
    }
    path.push_back(cur);
  }
  path.push_back(source_id);
  std::ranges::reverse(path);

  for (size_t i = 0; i + 1 < path.size(); ++i) {
    vid_t      from_id = static_cast<vid_t>(path[i]);
    vid_t      to_id   = static_cast<vid_t>(path[i + 1]);
    auto       uv      = find_vertex_edge(g, from_id, to_id);
    const route_t& r   = edge_value(g, uv);
    cout << "  " << city_name(from_id) << " -> " << city_name(to_id)
         << "  (" << r.distance_km << " km, " << r.road << ")\n";
  }
  cout << "  Total: " << distances[dest_id] << " km\n";
}

} // namespace france

int main() {
  france::run();
  return 0;
}
