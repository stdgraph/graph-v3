// france_routes_bgl.cpp — France Routes example using Boost Graph Library (BGL).
//
// Mirrors france_routes_gv3.cpp but uses BGL instead of graph-v3.
// Demonstrates:
//   - Loading a directed weighted road graph from JSON into a BGL adjacency_list
//   - Running Dijkstra's shortest paths from Paris via boost::dijkstra_shortest_paths
//   - Reconstructing and printing the shortest path to each city
//   - Writing a Graphviz DOT file

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using std::cout;
using std::string;
using std::vector;
using namespace std::string_literals;

// ── France Routes (BGL) ────────────────────────────────────────────────────────
// Directed weighted graph of French city road connections.
// Vertex descriptors are plain size_t (vecS container); vertex bundles store the
// city name for reverse lookup.  Edge bundles hold distance (km) and road name.

namespace france_bgl {

using vid_t  = std::size_t; // vertex descriptor == index (vecS)
using dist_t = int;         // distance in km

// ── Bundled properties ─────────────────────────────────────────────────────────

struct VertexProps {
  string name;
};

struct EdgeProps {
  dist_t distance_km = 0;
  string road;               // e.g. "A1", "A11 -> N157"
};

// Directed weighted graph; vecS vertex container keeps vertex_descriptor == vid_t.
using graph_t       = boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                                             VertexProps, EdgeProps>;
using vertex_desc_t = boost::graph_traits<graph_t>::vertex_descriptor; // = size_t
using edge_desc_t   = boost::graph_traits<graph_t>::edge_descriptor;

// ── load_result ────────────────────────────────────────────────────────────────

struct load_result {
  graph_t                           g;
  std::unordered_map<string, vid_t> city_id; // name -> vertex index
};

// Forward declarations
load_result load(const string& filename = BGLWS_DATA_DIR "/france_routes.json");
void        run (const string& filename = BGLWS_DATA_DIR "/france_routes.json");
void        dot (const string& dot_filename  = BGLWS_OUTPUT_DIR "/france_routes_bgl.dot",
                 const string& json_filename = BGLWS_DATA_DIR   "/france_routes.json");

// ── load() ─────────────────────────────────────────────────────────────────────
// Parses the JSON file with the same regex approach as france_routes_gv3.cpp and
// builds the BGL graph.

load_result load(const string& filename) {
  std::ifstream f(filename);
  if (!f)
    throw std::runtime_error("Cannot open file: "s + filename);

  string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

  // ── Parse city names ──────────────────────────────────────────────────────
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
      string name   = (*it)[1].str();
      city_id[name] = city_names.size();
      city_names.push_back(name);
    }
  }

  // ── Build BGL graph — one vertex per city ─────────────────────────────────
  graph_t g(city_names.size());
  for (vid_t i = 0; i < city_names.size(); ++i)
    g[i].name = city_names[i];

  // ── Parse and add directed edges ──────────────────────────────────────────
  static const std::regex route_re(
      R"re(\{\s*"from"\s*:\s*"([^"]+)"\s*,\s*"to"\s*:\s*"([^"]+)"\s*,\s*"distance_km"\s*:\s*(\d+)\s*,\s*"route"\s*:\s*"([^"]+)"\s*\})re");

  for (auto it  = std::sregex_iterator(content.begin(), content.end(), route_re),
            end = std::sregex_iterator{};
       it != end; ++it) {
    auto& m    = *it;
    vid_t  src  = city_id.at(m[1].str());
    vid_t  tgt  = city_id.at(m[2].str());
    dist_t dist = std::stoi(m[3].str());
    boost::add_edge(src, tgt, EdgeProps{dist, m[4].str()}, g);
  }

  return {std::move(g), std::move(city_id)};
}

// ── run() ──────────────────────────────────────────────────────────────────────

void run(const string& filename) {
  auto [g, city_id] = load(filename);

  const vid_t  n         = static_cast<vid_t>(boost::num_vertices(g));
  const vid_t  source_id = city_id.at("Paris");
  constexpr dist_t inf   = std::numeric_limits<dist_t>::max();

  // ── Dijkstra ──────────────────────────────────────────────────────────────
  // boost::dijkstra_shortest_paths initialises distances (inf) and source (0)
  // internally; predecessor_map is initialised to identity.
  vector<dist_t> distances(n, inf);
  vector<vid_t>  predecessors(n);
  std::iota(predecessors.begin(), predecessors.end(), 0); // each vertex → itself

  auto idx      = boost::get(boost::vertex_index, g);
  auto dist_map = boost::make_iterator_property_map(distances.begin(),    idx);
  auto pred_map = boost::make_iterator_property_map(predecessors.begin(), idx);
  auto wt_map   = boost::get(&EdgeProps::distance_km, g);

  boost::dijkstra_shortest_paths(g, source_id,
      boost::predecessor_map(pred_map)
      .distance_map(dist_map)
      .weight_map(wt_map));

  // ── Print all distances, sorted nearest-first ─────────────────────────────
  {
    vector<std::pair<dist_t, vid_t>> by_dist;
    by_dist.reserve(n);
    for (vid_t uid = 0; uid < n; ++uid)
      if (uid != source_id)
        by_dist.emplace_back(distances[uid], uid);
    std::ranges::sort(by_dist); // sort by (distance, uid)

    cout << "Shortest road distances from " << g[source_id].name << ":\n";
    for (auto& [d, uid] : by_dist) {
      if (d == inf)
        cout << "  " << g[uid].name << ": unreachable\n";
      else
        cout << "  " << g[uid].name << ": " << d << " km\n";
    }
  }

  // ── Reconstruct shortest path from Paris to Nice ──────────────────────────
  {
    const vid_t dest_id = city_id.at("Nice");
    cout << "\nShortest path: " << g[source_id].name << " -> " << g[dest_id].name << "\n";

    vector<vid_t> path;
    for (vid_t cur = dest_id; cur != source_id; cur = predecessors[cur]) {
      if (path.size() >= n) {
        cout << "  (no path found)\n";
        return;
      }
      path.push_back(cur);
    }
    path.push_back(source_id);
    std::ranges::reverse(path);

    for (size_t i = 0; i + 1 < path.size(); ++i) {
      vid_t from = path[i];
      vid_t to   = path[i + 1];

      // Find the directed edge from → to by scanning out-edges.
      edge_desc_t ed{};
      bool        found = false;
      for (auto [eit, eend] = boost::out_edges(from, g); eit != eend && !found; ++eit) {
        if (boost::target(*eit, g) == to) {
          ed    = *eit;
          found = true;
        }
      }
      if (!found) { cout << "  (edge not found)\n"; return; }

      const EdgeProps& ep = g[ed];
      cout << "  " << g[from].name << " -> " << g[to].name
           << "  (" << ep.distance_km << " km, " << ep.road << ")\n";
    }
    cout << "  Total: " << distances[dest_id] << " km\n";
  }
}

// ── dot() ──────────────────────────────────────────────────────────────────────
// Writes an undirected DOT file.  The graph is stored as directed (one entry
// per direction in the JSON) so directed pairs are deduplicated with min/max.

void dot(const string& dot_filename, const string& json_filename) {
  auto [g, city_id] = load(json_filename);

  std::ofstream out(dot_filename);
  if (!out)
    throw std::runtime_error("Cannot open output file: "s + dot_filename);

  out << "graph FranceRoutes_BGL {\n";

  // Vertices
  for (auto [vit, vend] = boost::vertices(g); vit != vend; ++vit) {
    vid_t vd = *vit;
    out << "  " << vd << " [label=\"" << g[vd].name << "\"];\n";
  }

  // Edges — emit each undirected pair once using min/max on vertex ids.
  std::set<std::pair<vid_t, vid_t>> emitted;
  for (auto [eit, eend] = boost::edges(g); eit != eend; ++eit) {
    vid_t s   = boost::source(*eit, g);
    vid_t t   = boost::target(*eit, g);
    auto  key = std::make_pair(std::min(s, t), std::max(s, t));
    if (emitted.insert(key).second) {
      const EdgeProps& ep = g[*eit];
      out << "  " << s << " -- " << t
          << " [label=\"" << ep.distance_km << " km\\n(" << ep.road << ")\""
          << ", weight=" << ep.distance_km << "];\n";
    }
  }

  out << "}\n";
  cout << "Wrote " << dot_filename << '\n';
}

} // namespace france_bgl

int main() {
  france_bgl::run();
  france_bgl::dot();
  return 0;
}
