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
#include <set>
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

using vid_t  = size_t; // changing this may require the use of static_cast to avoid warnings
using dist_t = int;    // distance in km

struct route_t {
  dist_t distance_km = 0;
  string road;              // e.g. "A1", "A11 -> N157"
};

// Vertex value = city name string; edge value = route_t (distance + road label).
using traits_t = container::vom_graph_traits<route_t, string, void, vid_t, false>;
using graph_t  = container::dynamic_adjacency_graph<traits_t>;

struct load_result {
  graph_t                           g;
  std::unordered_map<string, vid_t> city_id; // name -> vertex id
};

// Forward declarations
load_result load(const string& filename = BGLWS_DATA_DIR "/france_routes.json");
void        run (const string& filename = BGLWS_DATA_DIR "/france_routes.json");
void        dot (const string& dot_filename  = BGLWS_OUTPUT_DIR "/france_routes.dot",
                 const string& json_filename = BGLWS_DATA_DIR   "/france_routes.json");

load_result load(const string& filename) {
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
      city_id[name]          = city_names.size();
      city_names.push_back(name);
    }
  }

  // ── Load vertices (city name as vertex value) ─────────────────────────────
  using vertex_data = copyable_vertex_t<vid_t, string>;
  vector<vertex_data> vv;
  vv.reserve(city_names.size());
  for (vid_t i = 0; i < city_names.size(); ++i)
    vv.push_back({i, city_names[i]});

  graph_t g;
  g.load_vertices(vv);

  // ── Parse and load edges ──────────────────────────────────────────────────
  // Note: named raw-string delimiter (re) avoids collision with ")" in the pattern.
  static const std::regex route_re(
      R"re(\{\s*"from"\s*:\s*"([^"]+)"\s*,\s*"to"\s*:\s*"([^"]+)"\s*,\s*"distance_km"\s*:\s*(\d+)\s*,\s*"route"\s*:\s*"([^"]+)"\s*\})re");

  using edge_data = copyable_edge_t<vid_t, route_t>; // target vertex id + route info
  vector<edge_data> edges;

  for (auto it  = std::sregex_iterator(content.begin(), content.end(), route_re),
            end = std::sregex_iterator{};
       it != end; ++it) {
    auto& m    = *it;
    vid_t src  = city_id.at(m[1].str());
    vid_t tgt  = city_id.at(m[2].str());
    dist_t dist = std::stoi(m[3].str());
    edges.push_back({src, tgt, route_t{dist, m[4].str()}});
  }

  g.load_edges(edges);
  return {std::move(g), std::move(city_id)};
}

void run(const string& filename) {
  auto [g, city_id] = load(filename);

  // Vertex & edge properties
  auto city_name = [&](const auto& g2, vid_t u) -> string {
    return vertex_value(g2, *find_vertex(g2, u));
  };
  auto route_distance = [](const auto& g2, const auto& uv) -> dist_t {
    return edge_value(g2, uv).distance_km;
  };
  auto route_road = [](const auto& g2, const auto& uv) -> string {
    return edge_value(g2, uv).road;
  };

  // ── Evaluate shortest paths from source ────────────────────────────────────
  const vid_t source_id = city_id.at("Paris");

  // Distance and predecessor vectors indexed by vertex id.
  vector<dist_t> distances(num_vertices(g));
  vector<vid_t>  predecessors(num_vertices(g));
  init_shortest_paths(g, distances, predecessors);

  dijkstra_shortest_paths(g, source_id,
                          container_value_fn(distances),
                          container_value_fn(predecessors),
                          route_distance);

  // ── Print all distances, sorted nearest-first ──────────────────────────────
  {
    vector<std::pair<dist_t, vid_t>> by_dist;
    by_dist.reserve(num_vertices(g));
    for (vid_t uid = 0; uid < num_vertices(g); ++uid) {
        if (uid != source_id)
        by_dist.emplace_back(distances[uid], uid);
    }
    std::ranges::sort(by_dist); // sort by (distance, uid) — default pair comparison

    cout << "Shortest road distances from " << city_name(g, source_id) << ":\n";
    for (auto& [d, uid] : by_dist) {
        if (d == infinite_distance<dist_t>())
        cout << "  " << city_name(g, uid) << ": unreachable\n";
        else
        cout << "  " << city_name(g, uid) << ": " << d << " km\n";
    }
  }

  // ── Reconstruct shortest path from Paris to Nice ────────────────────────────
  {
    const vid_t dest_id = city_id.at("Nice");
    cout << "\nShortest path: " << city_name(g, source_id) << " -> " << city_name(g, dest_id) << "\n";

    vector<vid_t> path;
    for (vid_t cur = dest_id; cur != source_id; cur = predecessors[cur]) {
      if (path.size() >= num_vertices(g)) {
        cout << "  (no path found)\n";
        return;
      }
      path.push_back(cur);
    }
    path.push_back(source_id);
    std::ranges::reverse(path);

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        vid_t      from_id = path[i];
        vid_t      to_id   = path[i + 1];
        auto uv = find_vertex_edge(g, from_id, to_id);
        cout << "  " << city_name(g, from_id) << " -> " << city_name(g, to_id)
            << "  (" << route_distance(g, uv) << " km, " << route_road(g, uv) << ")\n";
    }
    cout << "  Total: " << distances[dest_id] << " km\n";
  }
}

void dot(const string& dot_filename, const string& json_filename) {
  auto [g, city_id] = load(json_filename);

  auto city_name = [&](vertex_t<graph_t> u) -> string {
    return vertex_value(g, u);
  };

  std::ofstream out(dot_filename);
  if (!out)
    throw std::runtime_error("Cannot open output file: "s + dot_filename);

  out << "graph FranceRoutes {\n";

  for (auto u : vertices(g)) {
    out << "  " << vertex_id(g, u) << " [label=\"" << city_name(u) << "\"];\n";
  }

  // Emit each undirected edge once: use the copy where sid < tid so that
  // both Paris->Dijon and Dijon->Paris collapse to a single line.
  std::set<std::pair<vid_t, vid_t>> emitted;
  for (auto [sid, tid, uv] : views::edgelist(g)) {
    auto key = std::make_pair(std::min(sid, tid), std::max(sid, tid));
    if (emitted.insert(key).second) {
      const route_t& r = edge_value(g, uv);
      out << "  " << sid << " -- " << tid
          << " [label=\"" << r.distance_km << " km\\n(" << r.road << ")\""
          << ", weight=" << r.distance_km << "];\n";
    }
  }

  out << "}\n";
  cout << "Wrote " << dot_filename << '\n';
}

} // namespace france

int main() {
  france::run();
  france::dot();
  return 0;
}
