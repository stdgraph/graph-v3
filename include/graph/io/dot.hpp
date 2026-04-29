/**
 * @file dot.hpp
 * @brief DOT (GraphViz) graph I/O — write and read.
 *
 * Provides:
 *   - write_dot(os, g)                     Zero-config: auto-formats VV/EV via std::format
 *   - write_dot(os, g, vattr_fn, eattr_fn) User-supplied DOT attribute strings
 *   - read_dot(is)                         Parse DOT into dynamic_graph<string, string>
 *
 * DOT reference: https://graphviz.org/doc/info/lang.html
 */

#pragma once

#include <graph/graph.hpp>
#include <graph/io/detail/common.hpp>

#include <format>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace graph::io {

namespace detail {

  /// Escape a string for DOT double-quoted context.
  inline std::string dot_escape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
      switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        default:   out += c;
      }
    }
    return out;
  }

} // namespace detail

// ---------------------------------------------------------------------------
// write_dot — zero-config default (auto-formats values via std::format)
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph in DOT format with auto-detected value formatting.
 *
 * If VV (vertex value type) satisfies std::formatter, vertices are labeled.
 * If EV (edge value type) satisfies std::formatter, edges are labeled.
 * If VV/EV is void or not formattable, attributes are omitted.
 *
 * @param os         Output stream.
 * @param g          Graph satisfying adjacency_list.
 * @param graph_name Optional graph name (defaults to "G").
 */
template <adj_list::adjacency_list G>
void write_dot(std::ostream& os, const G& g, std::string_view graph_name = "G") {
  os << "digraph " << graph_name << " {\n";

  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    os << "  " << uid;

    if constexpr (detail::has_vertex_value<const G>) {
      using VV = std::remove_cvref_t<decltype(graph::vertex_value(g, u))>;
      if constexpr (detail::formattable<VV>) {
        os << " [label=\"" << detail::dot_escape(std::format("{}", graph::vertex_value(g, u))) << "\"]";
      }
    }
    os << ";\n";

    for (auto uv : edges(g, u)) {
      auto tid = target_id(g, uv);
      os << "  " << uid << " -> " << tid;

      if constexpr (detail::has_edge_value<const G>) {
        using EV = std::remove_cvref_t<decltype(graph::edge_value(g, uv))>;
        if constexpr (detail::formattable<EV>) {
          os << " [label=\"" << detail::dot_escape(std::format("{}", graph::edge_value(g, uv))) << "\"]";
        }
      }
      os << ";\n";
    }
  }

  os << "}\n";
}

// ---------------------------------------------------------------------------
// write_dot — user-supplied attribute functions
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph in DOT format with user-supplied attribute functions.
 *
 * @param os           Output stream.
 * @param g            Graph satisfying adjacency_list.
 * @param vertex_attr  Callable (const G&, vertex_id_t<G>) -> string.
 *                     Returns full DOT attribute string, e.g. R"([label="A", color="red"])".
 *                     Return empty string to omit attributes for a vertex.
 * @param edge_attr    Callable (const G&, vertex_id_t<G> source, vertex_id_t<G> target, edge_t<G>) -> string.
 *                     Returns full DOT attribute string, e.g. R"([weight=3.14])".
 *                     Return empty string to omit attributes for an edge.
 * @param graph_name   Optional graph name (defaults to "G").
 */
template <adj_list::adjacency_list G, class VAttrFn, class EAttrFn>
void write_dot(std::ostream& os, const G& g,
               VAttrFn vertex_attr, EAttrFn edge_attr,
               std::string_view graph_name = "G") {
  os << "digraph " << graph_name << " {\n";

  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    os << "  " << uid;

    auto vattr = vertex_attr(g, uid);
    if (!vattr.empty()) {
      os << " " << vattr;
    }
    os << ";\n";

    for (auto uv : edges(g, u)) {
      auto tid = target_id(g, uv);
      os << "  " << uid << " -> " << tid;

      auto eattr = edge_attr(g, uid, tid, uv);
      if (!eattr.empty()) {
        os << " " << eattr;
      }
      os << ";\n";
    }
  }

  os << "}\n";
}

// ---------------------------------------------------------------------------
// read_dot — simple DOT parser
// ---------------------------------------------------------------------------

/// Result of parsing a DOT file: edges with optional labels.
struct dot_edge {
  std::string source;
  std::string target;
  std::string label; ///< Edge label attribute (empty if none)
};

/// Parsed DOT graph representation.
struct dot_graph {
  std::string              name;           ///< Graph name
  bool                     directed{true}; ///< true=digraph, false=graph
  std::vector<std::string> vertex_ids;     ///< Vertex identifiers in declaration order
  std::vector<std::string> vertex_labels;  ///< Vertex labels (parallel to vertex_ids; empty if none)
  std::vector<dot_edge>    edges;          ///< All edges
};

namespace detail {

  inline std::string trim(std::string_view sv) {
    auto start = sv.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) return {};
    auto end = sv.find_last_not_of(" \t\r\n");
    return std::string(sv.substr(start, end - start + 1));
  }

  inline std::string extract_label(std::string_view attrs) {
    // Simple extraction of label="..." from an attribute string
    auto pos = attrs.find("label");
    if (pos == std::string_view::npos) return {};
    pos = attrs.find('=', pos);
    if (pos == std::string_view::npos) return {};
    pos = attrs.find('"', pos);
    if (pos == std::string_view::npos) return {};
    ++pos;
    auto end = attrs.find('"', pos);
    if (end == std::string_view::npos) return {};
    return std::string(attrs.substr(pos, end - pos));
  }

} // namespace detail

/**
 * @brief Parse a DOT file into a dot_graph structure.
 *
 * Supports a subset of the DOT language:
 *   - digraph / graph declarations
 *   - Node declarations with optional [label="..."] attributes
 *   - Edge declarations (-> or --) with optional [label="..."] attributes
 *   - C/C++ style comments and # line comments
 *   - Semicolons are optional
 *
 * Does NOT support: subgraphs, HTML labels, port syntax, multi-line attributes.
 *
 * @param is Input stream containing DOT text.
 * @return Parsed dot_graph.
 */
inline dot_graph read_dot(std::istream& is) {
  dot_graph result;
  std::string content((std::istreambuf_iterator<char>(is)),
                       std::istreambuf_iterator<char>());

  // Strip C-style comments
  std::string cleaned;
  cleaned.reserve(content.size());
  for (size_t i = 0; i < content.size(); ++i) {
    if (i + 1 < content.size() && content[i] == '/' && content[i + 1] == '/') {
      while (i < content.size() && content[i] != '\n') ++i;
    } else if (i + 1 < content.size() && content[i] == '/' && content[i + 1] == '*') {
      i += 2;
      while (i + 1 < content.size() && !(content[i] == '*' && content[i + 1] == '/')) ++i;
      ++i; // skip '/'
    } else if (content[i] == '#') {
      while (i < content.size() && content[i] != '\n') ++i;
    } else {
      cleaned += content[i];
    }
  }

  // Find graph type and name
  auto dg_pos = cleaned.find("digraph");
  auto g_pos  = cleaned.find("graph");
  size_t header_end;
  if (dg_pos != std::string::npos && (g_pos == std::string::npos || dg_pos <= g_pos)) {
    result.directed = true;
    header_end = dg_pos + 7;
  } else if (g_pos != std::string::npos) {
    result.directed = false;
    header_end = g_pos + 5;
  } else {
    return result; // Not a valid DOT file
  }

  // Extract name (between keyword and '{')
  auto brace = cleaned.find('{', header_end);
  if (brace == std::string::npos) return result;
  result.name = detail::trim(std::string_view(cleaned).substr(header_end, brace - header_end));

  // Extract body
  auto end_brace = cleaned.rfind('}');
  if (end_brace == std::string::npos || end_brace <= brace) return result;
  std::string body(cleaned, brace + 1, end_brace - brace - 1);

  // Split body into statements (by ';' or newlines)
  std::vector<std::string> statements;
  std::string current;
  int bracket_depth = 0;
  for (char c : body) {
    if (c == '[') ++bracket_depth;
    if (c == ']') --bracket_depth;
    if ((c == ';' || c == '\n') && bracket_depth == 0) {
      auto s = detail::trim(current);
      if (!s.empty()) statements.push_back(std::move(s));
      current.clear();
    } else {
      current += c;
    }
  }
  if (auto s = detail::trim(current); !s.empty()) statements.push_back(std::move(s));

  std::string edge_op = result.directed ? "->" : "--";

  // Track known vertices
  auto ensure_vertex = [&](const std::string& id) {
    for (size_t i = 0; i < result.vertex_ids.size(); ++i) {
      if (result.vertex_ids[i] == id) return;
    }
    result.vertex_ids.push_back(id);
    result.vertex_labels.push_back({});
  };

  for (const auto& stmt : statements) {
    // Skip graph-level attributes (key=value without nodes)
    if (stmt.find(edge_op) == std::string::npos && stmt.find('[') == std::string::npos &&
        stmt.find('=') != std::string::npos) {
      continue;
    }

    // Check if it's an edge statement
    auto arrow_pos = stmt.find(edge_op);
    if (arrow_pos != std::string::npos) {
      // Edge: "A -> B [attrs]" or "A -> B"
      std::string lhs = detail::trim(std::string_view(stmt).substr(0, arrow_pos));
      std::string rhs_full(stmt, arrow_pos + edge_op.size());
      
      // Extract attributes if present
      std::string attrs;
      std::string rhs;
      auto attr_pos = rhs_full.find('[');
      if (attr_pos != std::string::npos) {
        rhs = detail::trim(std::string_view(rhs_full).substr(0, attr_pos));
        auto attr_end = rhs_full.find(']', attr_pos);
        if (attr_end != std::string::npos) {
          attrs = std::string(rhs_full, attr_pos + 1, attr_end - attr_pos - 1);
        }
      } else {
        rhs = detail::trim(rhs_full);
      }

      // Remove quotes from identifiers
      auto unquote = [](std::string s) -> std::string {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
          return s.substr(1, s.size() - 2);
        }
        return s;
      };
      lhs = unquote(lhs);
      rhs = unquote(rhs);

      ensure_vertex(lhs);
      ensure_vertex(rhs);
      result.edges.push_back({lhs, rhs, detail::extract_label(attrs)});
    } else {
      // Node statement: "A [attrs]" or just "A"
      std::string id;
      std::string attrs;
      auto attr_pos = stmt.find('[');
      if (attr_pos != std::string::npos) {
        id = detail::trim(std::string_view(stmt).substr(0, attr_pos));
        auto attr_end = stmt.find(']', attr_pos);
        if (attr_end != std::string::npos) {
          attrs = std::string(stmt, attr_pos + 1, attr_end - attr_pos - 1);
        }
      } else {
        id = detail::trim(stmt);
      }

      // Remove quotes
      if (id.size() >= 2 && id.front() == '"' && id.back() == '"') {
        id = id.substr(1, id.size() - 2);
      }

      if (id.empty() || id == "node" || id == "edge" || id == "graph") continue;

      ensure_vertex(id);
      if (!attrs.empty()) {
        // Find this vertex and set its label
        for (size_t i = 0; i < result.vertex_ids.size(); ++i) {
          if (result.vertex_ids[i] == id) {
            result.vertex_labels[i] = detail::extract_label(attrs);
            break;
          }
        }
      }
    }
  }

  return result;
}

} // namespace graph::io
