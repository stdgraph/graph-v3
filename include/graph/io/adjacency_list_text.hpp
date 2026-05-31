/**
 * @file adjacency_list_text.hpp
 * @brief Adjacency-list text graph I/O — write and read.
 *
 * Provides:
 *   - write_adjacency_list_text(os, g)   Emit a graph as a textual adjacency list
 *   - read_adjacency_list_text(is)       Parse a textual adjacency list
 *
 * This is the plain whitespace-delimited adjacency dump in the spirit of BGL's
 * `operator<<` / `operator>>` for graphs.  It is NOT CSV: one line per vertex,
 * the vertex id, a `:` separator, then its out-neighbours separated by spaces:
 *
 *   0: 1 2
 *   1: 2
 *   2: 0 3
 *   3:
 *
 * The format carries structure only (no vertex/edge values).  Vertices with no
 * out-edges still produce a line (`<id>:`) so the vertex set is preserved.
 *
 * NOTE: Self-contained — no external dependencies.
 */

#pragma once

#include <graph/graph.hpp>
#include <graph/io/detail/common.hpp>

#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace graph::io {

// ---------------------------------------------------------------------------
// write_adjacency_list_text
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph as a textual adjacency list.
 *
 * Emits one line per vertex: `<id>: <t0> <t1> ...`.  Vertices with no
 * out-edges produce a trailing-colon line so the full vertex set round-trips.
 *
 * @param os Output stream.
 * @param g  Graph satisfying adjacency_list.
 */
template <adj_list::adjacency_list G>
void write_adjacency_list_text(std::ostream& os, const G& g) {
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    os << uid << ':';
    for (auto uv : edges(g, u)) {
      os << ' ' << target_id(g, uv);
    }
    os << '\n';
  }
}

// ---------------------------------------------------------------------------
// read_adjacency_list_text
// ---------------------------------------------------------------------------

/// A single parsed adjacency-list-text edge.
struct adjacency_list_text_edge {
  std::string source;
  std::string target;
};

/// Parsed adjacency-list-text graph.
struct adjacency_list_text_graph {
  std::vector<std::string>              vertex_ids; ///< Vertices in declaration order
  std::vector<adjacency_list_text_edge> edges;      ///< All edges
};

/**
 * @brief Parse a textual adjacency list into an adjacency_list_text_graph.
 *
 * Each non-empty line is `<id>: <t0> <t1> ...`.  The `:` separator is
 * optional — a line may also be a whitespace-separated `<id> <t0> <t1> ...`.
 * Both the source vertex and every target are registered as vertices.
 *
 * @param is Input stream containing adjacency-list text.
 * @return Parsed adjacency_list_text_graph.
 */
inline adjacency_list_text_graph read_adjacency_list_text(std::istream& is) {
  adjacency_list_text_graph result;
  std::string               line;

  auto ensure_vertex = [&](const std::string& id) {
    for (const auto& existing : result.vertex_ids) {
      if (existing == id) return;
    }
    result.vertex_ids.push_back(id);
  };

  while (std::getline(is, line)) {
    // Normalize: replace a leading "id:" colon with a space separator.
    auto colon = line.find(':');
    if (colon != std::string::npos) line[colon] = ' ';

    std::istringstream ls(line);
    std::string        source;
    if (!(ls >> source)) continue; // blank line

    ensure_vertex(source);

    std::string target;
    while (ls >> target) {
      ensure_vertex(target);
      result.edges.push_back({source, target});
    }
  }

  return result;
}

} // namespace graph::io
