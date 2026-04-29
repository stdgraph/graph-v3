/**
 * @file graphml.hpp
 * @brief GraphML (XML) graph I/O — write and read.
 *
 * Provides:
 *   - write_graphml(os, g)                       Zero-config: auto-formats VV/EV via std::format
 *   - write_graphml(os, g, vattr_fn, eattr_fn)   User-supplied attribute maps
 *   - read_graphml(is)                            Parse GraphML into graphml_graph structure
 *
 * GraphML reference: http://graphml.graphdrawing.org/
 *
 * NOTE: This is a lightweight implementation that does not require an XML parser
 * library. It handles the core GraphML subset sufficient for graph interchange.
 */

#pragma once

#include <graph/graph.hpp>
#include <graph/io/detail/common.hpp>

#include <format>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace graph::io {

namespace detail {

  /// Escape XML special characters.
  inline std::string xml_escape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
      switch (c) {
        case '&':  out += "&amp;"; break;
        case '<':  out += "&lt;"; break;
        case '>':  out += "&gt;"; break;
        case '"':  out += "&quot;"; break;
        case '\'': out += "&apos;"; break;
        default:   out += c;
      }
    }
    return out;
  }

} // namespace detail

// ---------------------------------------------------------------------------
// write_graphml — zero-config default
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph in GraphML format with auto-detected value formatting.
 *
 * Vertex values (if formattable) are written as a <data key="d0"> element.
 * Edge values (if formattable) are written as a <data key="d1"> element.
 *
 * @param os         Output stream.
 * @param g          Graph satisfying adjacency_list.
 * @param graph_id   Optional graph element id (defaults to "G").
 */
template <adj_list::adjacency_list G>
void write_graphml(std::ostream& os, const G& g, std::string_view graph_id = "G") {
  os << R"(<?xml version="1.0" encoding="UTF-8"?>)" "\n";
  os << R"(<graphml xmlns="http://graphml.graphdrawing.org/xmlns")" "\n";
  os << R"(         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance")" "\n";
  os << R"(         xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns)"
     << R"( http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">)" "\n";

  // Declare data keys
  bool has_vv = false;
  bool has_ev = false;

  if constexpr (detail::has_vertex_value<const G>) {
    using VV = std::remove_cvref_t<decltype(graph::vertex_value(g, *vertices(g).begin()))>;
    if constexpr (detail::formattable<VV>) {
      has_vv = true;
      os << R"(  <key id="d0" for="node" attr.name="label" attr.type="string"/>)" "\n";
    }
  }

  if constexpr (detail::has_edge_value<const G>) {
    using EV = std::remove_cvref_t<decltype(graph::edge_value(g, *edges(g, *vertices(g).begin()).begin()))>;
    if constexpr (detail::formattable<EV>) {
      has_ev = true;
      os << R"(  <key id="d1" for="edge" attr.name="label" attr.type="string"/>)" "\n";
    }
  }

  os << "  <graph id=\"" << graph_id << "\" edgedefault=\"directed\">\n";

  // Vertices
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    os << "    <node id=\"n" << uid << "\"";

    if constexpr (detail::has_vertex_value<const G>) {
      using VV = std::remove_cvref_t<decltype(graph::vertex_value(g, u))>;
      if constexpr (detail::formattable<VV>) {
        os << ">\n";
        os << "      <data key=\"d0\">" << detail::xml_escape(std::format("{}", graph::vertex_value(g, u)))
           << "</data>\n";
        os << "    </node>\n";
      } else {
        os << "/>\n";
      }
    } else {
      os << "/>\n";
    }
  }

  // Edges
  size_t edge_counter = 0;
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    for (auto uv : edges(g, u)) {
      auto tid = target_id(g, uv);
      os << "    <edge id=\"e" << edge_counter++ << "\" source=\"n" << uid
         << "\" target=\"n" << tid << "\"";

      if constexpr (detail::has_edge_value<const G>) {
        using EV = std::remove_cvref_t<decltype(graph::edge_value(g, uv))>;
        if constexpr (detail::formattable<EV>) {
          os << ">\n";
          os << "      <data key=\"d1\">" << detail::xml_escape(std::format("{}", graph::edge_value(g, uv)))
             << "</data>\n";
          os << "    </edge>\n";
        } else {
          os << "/>\n";
        }
      } else {
        os << "/>\n";
      }
    }
  }

  os << "  </graph>\n";
  os << "</graphml>\n";
}

// ---------------------------------------------------------------------------
// write_graphml — user-supplied attribute functions
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph in GraphML format with user-supplied attribute maps.
 *
 * @param os           Output stream.
 * @param g            Graph satisfying adjacency_list.
 * @param vertex_data  Callable (const G&, vertex_id_t<G>) -> map<string, string>.
 *                     Returns key-value pairs to emit as <data> elements.
 * @param edge_data    Callable (const G&, vertex_id_t<G> src, vertex_id_t<G> tgt, edge_t<G>) -> map<string, string>.
 *                     Returns key-value pairs to emit as <data> elements.
 * @param graph_id     Optional graph element id.
 */
template <adj_list::adjacency_list G, class VDataFn, class EDataFn>
void write_graphml(std::ostream& os, const G& g,
                   VDataFn vertex_data, EDataFn edge_data,
                   std::string_view graph_id = "G") {
  // Collect all data keys by scanning the first vertex/edge
  std::vector<std::pair<std::string, std::string>> vkeys; // (id, attr.name)
  std::vector<std::pair<std::string, std::string>> ekeys;

  // First pass: discover keys from first vertex/edge
  if (num_vertices(g) > 0) {
    auto u   = *vertices(g).begin();
    auto uid = vertex_id(g, u);
    auto vd  = vertex_data(g, uid);
    int  idx = 0;
    for (const auto& [k, v] : vd) {
      vkeys.emplace_back(std::format("vd{}", idx++), k);
    }

    if (!edges(g, u).empty()) {
      auto uv  = *edges(g, u).begin();
      auto tid = target_id(g, uv);
      auto ed  = edge_data(g, uid, tid, uv);
      idx = 0;
      for (const auto& [k, v] : ed) {
        ekeys.emplace_back(std::format("ed{}", idx++), k);
      }
    }
  }

  os << R"(<?xml version="1.0" encoding="UTF-8"?>)" "\n";
  os << R"(<graphml xmlns="http://graphml.graphdrawing.org/xmlns")" "\n";
  os << R"(         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance")" "\n";
  os << R"(         xsi:schemaLocation="http://graphml.graphdrawing.org/xmlns)"
     << R"( http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd">)" "\n";

  for (const auto& [id, name] : vkeys) {
    os << "  <key id=\"" << id << "\" for=\"node\" attr.name=\"" << detail::xml_escape(name)
       << "\" attr.type=\"string\"/>\n";
  }
  for (const auto& [id, name] : ekeys) {
    os << "  <key id=\"" << id << "\" for=\"edge\" attr.name=\"" << detail::xml_escape(name)
       << "\" attr.type=\"string\"/>\n";
  }

  os << "  <graph id=\"" << graph_id << "\" edgedefault=\"directed\">\n";

  // Vertices
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    auto vd  = vertex_data(g, uid);

    if (vd.empty()) {
      os << "    <node id=\"n" << uid << "\"/>\n";
    } else {
      os << "    <node id=\"n" << uid << "\">\n";
      size_t idx = 0;
      for (const auto& [k, v] : vd) {
        if (idx < vkeys.size()) {
          os << "      <data key=\"" << vkeys[idx].first << "\">" << detail::xml_escape(v) << "</data>\n";
        }
        ++idx;
      }
      os << "    </node>\n";
    }
  }

  // Edges
  size_t edge_counter = 0;
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    for (auto uv : edges(g, u)) {
      auto tid = target_id(g, uv);
      auto ed  = edge_data(g, uid, tid, uv);

      os << "    <edge id=\"e" << edge_counter++ << "\" source=\"n" << uid
         << "\" target=\"n" << tid << "\"";

      if (ed.empty()) {
        os << "/>\n";
      } else {
        os << ">\n";
        size_t idx = 0;
        for (const auto& [k, v] : ed) {
          if (idx < ekeys.size()) {
            os << "      <data key=\"" << ekeys[idx].first << "\">" << detail::xml_escape(v) << "</data>\n";
          }
          ++idx;
        }
        os << "    </edge>\n";
      }
    }
  }

  os << "  </graph>\n";
  os << "</graphml>\n";
}

// ---------------------------------------------------------------------------
// read_graphml — simple GraphML parser
// ---------------------------------------------------------------------------

/// Parsed GraphML node.
struct graphml_node {
  std::string                        id;
  std::map<std::string, std::string> data; ///< key-id -> value
};

/// Parsed GraphML edge.
struct graphml_edge {
  std::string                        id;
  std::string                        source;
  std::string                        target;
  std::map<std::string, std::string> data; ///< key-id -> value
};

/// Parsed GraphML key declaration.
struct graphml_key {
  std::string id;
  std::string for_elem; ///< "node" or "edge"
  std::string name;
  std::string type;
};

/// Parsed GraphML graph.
struct graphml_graph {
  std::string              id;
  bool                     directed{true};
  std::vector<graphml_key> keys;
  std::vector<graphml_node> nodes;
  std::vector<graphml_edge> edges;

  /// Look up a key's attr.name by its id.
  std::string key_name(const std::string& key_id) const {
    for (const auto& k : keys) {
      if (k.id == key_id) return k.name;
    }
    return key_id;
  }
};

namespace detail {

  inline std::string extract_xml_attr(std::string_view tag, std::string_view attr_name) {
    auto pos = tag.find(attr_name);
    if (pos == std::string_view::npos) return {};
    pos = tag.find('=', pos);
    if (pos == std::string_view::npos) return {};
    pos = tag.find('"', pos);
    if (pos == std::string_view::npos) return {};
    ++pos;
    auto end = tag.find('"', pos);
    if (end == std::string_view::npos) return {};
    return std::string(tag.substr(pos, end - pos));
  }

  inline std::string xml_unescape(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
      if (s[i] == '&') {
        if (s.substr(i, 4) == "&lt;") { out += '<'; i += 3; }
        else if (s.substr(i, 4) == "&gt;") { out += '>'; i += 3; }
        else if (s.substr(i, 5) == "&amp;") { out += '&'; i += 4; }
        else if (s.substr(i, 6) == "&quot;") { out += '"'; i += 5; }
        else if (s.substr(i, 6) == "&apos;") { out += '\''; i += 5; }
        else { out += s[i]; }
      } else {
        out += s[i];
      }
    }
    return out;
  }

} // namespace detail

/**
 * @brief Parse a GraphML file into a graphml_graph structure.
 *
 * Supports the core GraphML elements: <key>, <graph>, <node>, <edge>, <data>.
 * Does NOT support: nested graphs, ports, hyperedges, default values.
 *
 * @param is Input stream containing GraphML XML.
 * @return Parsed graphml_graph.
 */
inline graphml_graph read_graphml(std::istream& is) {
  graphml_graph result;
  std::string   content((std::istreambuf_iterator<char>(is)),
                         std::istreambuf_iterator<char>());

  // Simple tag-by-tag parsing (no full XML parser dependency)
  size_t pos = 0;
  auto   find_tag = [&](size_t from) -> std::pair<size_t, size_t> {
    auto start = content.find('<', from);
    if (start == std::string::npos) return {std::string::npos, std::string::npos};
    auto end = content.find('>', start);
    if (end == std::string::npos) return {std::string::npos, std::string::npos};
    return {start, end + 1};
  };

  auto get_tag_name = [](std::string_view tag) -> std::string {
    // Skip '<' and optional '/'
    size_t start = 1;
    if (tag.size() > 1 && tag[1] == '/') start = 2;
    if (tag.size() > 1 && tag[1] == '?') start = 2;
    auto end = tag.find_first_of(" \t\n/>", start);
    if (end == std::string_view::npos) end = tag.size() - 1;
    return std::string(tag.substr(start, end - start));
  };

  // State machine
  enum class State { top, in_graph, in_node, in_edge, in_data };
  State       state = State::top;
  graphml_node current_node;
  graphml_edge current_edge;
  std::string  current_data_key;
  std::string  current_data_value;

  while (pos < content.size()) {
    auto [tag_start, tag_end] = find_tag(pos);
    if (tag_start == std::string::npos) break;

    std::string_view tag_sv(content.data() + tag_start, tag_end - tag_start);
    std::string      tag_name = get_tag_name(tag_sv);
    bool             is_close = (tag_sv.size() > 1 && tag_sv[1] == '/');
    bool             is_self_close = (tag_sv.size() > 1 && tag_sv[tag_sv.size() - 2] == '/');

    if (tag_name == "key" && !is_close) {
      graphml_key k;
      k.id       = detail::extract_xml_attr(tag_sv, "id");
      k.for_elem = detail::extract_xml_attr(tag_sv, "for");
      k.name     = detail::extract_xml_attr(tag_sv, "attr.name");
      k.type     = detail::extract_xml_attr(tag_sv, "attr.type");
      result.keys.push_back(std::move(k));
    } else if (tag_name == "graph" && !is_close) {
      result.id = detail::extract_xml_attr(tag_sv, "id");
      auto ed   = detail::extract_xml_attr(tag_sv, "edgedefault");
      result.directed = (ed != "undirected");
      state = State::in_graph;
    } else if (tag_name == "node" && !is_close) {
      current_node = {};
      current_node.id = detail::extract_xml_attr(tag_sv, "id");
      if (is_self_close) {
        result.nodes.push_back(std::move(current_node));
      } else {
        state = State::in_node;
      }
    } else if (tag_name == "node" && is_close) {
      result.nodes.push_back(std::move(current_node));
      current_node = {};
      state = State::in_graph;
    } else if (tag_name == "edge" && !is_close) {
      current_edge = {};
      current_edge.id     = detail::extract_xml_attr(tag_sv, "id");
      current_edge.source = detail::extract_xml_attr(tag_sv, "source");
      current_edge.target = detail::extract_xml_attr(tag_sv, "target");
      if (is_self_close) {
        result.edges.push_back(std::move(current_edge));
      } else {
        state = State::in_edge;
      }
    } else if (tag_name == "edge" && is_close) {
      result.edges.push_back(std::move(current_edge));
      current_edge = {};
      state = State::in_graph;
    } else if (tag_name == "data" && !is_close && !is_self_close) {
      current_data_key = detail::extract_xml_attr(tag_sv, "key");
      // Extract text content between <data> and </data>
      auto data_end = content.find("</data>", tag_end);
      if (data_end != std::string::npos) {
        current_data_value = detail::xml_unescape(
            std::string_view(content.data() + tag_end, data_end - tag_end));

        if (state == State::in_node) {
          current_node.data[current_data_key] = current_data_value;
        } else if (state == State::in_edge) {
          current_edge.data[current_data_key] = current_data_value;
        }
        pos = data_end + 7; // skip </data>
        continue;
      }
    }

    pos = tag_end;
  }

  return result;
}

} // namespace graph::io
