/**
 * @file json.hpp
 * @brief JSON graph I/O — write and read.
 *
 * Provides:
 *   - write_json(os, g)                       Zero-config: auto-formats VV/EV via std::format
 *   - write_json(os, g, vattr_fn, eattr_fn)   User-supplied attribute maps
 *   - read_json(is)                            Parse JSON into json_graph structure
 *
 * Uses a simple JSON format inspired by the JGF (JSON Graph Format):
 *   {
 *     "directed": true,
 *     "nodes": [ {"id": 0, "label": "..."}, ... ],
 *     "edges": [ {"source": 0, "target": 1, "label": "..."}, ... ]
 *   }
 *
 * NOTE: This is a self-contained implementation with no external JSON library dependency.
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

  /// Escape a string for JSON.
  inline std::string json_escape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
      switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        default:
          if (static_cast<unsigned char>(c) < 0x20) {
            out += std::format("\\u{:04x}", static_cast<unsigned>(c));
          } else {
            out += c;
          }
      }
    }
    return out;
  }

} // namespace detail

// ---------------------------------------------------------------------------
// write_json — zero-config default
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph in JSON format with auto-detected value formatting.
 *
 * Output format:
 * {
 *   "directed": true,
 *   "nodes": [
 *     {"id": 0},
 *     {"id": 1, "label": "vertex value"}
 *   ],
 *   "edges": [
 *     {"source": 0, "target": 1, "label": "edge value"}
 *   ]
 * }
 *
 * @param os     Output stream.
 * @param g      Graph satisfying adjacency_list.
 * @param indent Number of spaces for indentation (0 for compact).
 */
template <adj_list::adjacency_list G>
void write_json(std::ostream& os, const G& g, int indent = 2) {
  auto ind = [indent](int level) -> std::string {
    if (indent == 0) return {};
    return std::string(static_cast<size_t>(indent * level), ' ');
  };
  auto nl = [indent]() -> std::string_view { return indent > 0 ? "\n" : ""; };
  auto sep = [indent]() -> std::string_view { return indent > 0 ? " " : ""; };

  os << "{" << nl();
  os << ind(1) << "\"directed\":" << sep() << "true," << nl();

  // Nodes
  os << ind(1) << "\"nodes\":" << sep() << "[" << nl();
  bool first_node = true;
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    if (!first_node) os << "," << nl();
    first_node = false;

    os << ind(2) << "{\"id\":" << sep() << uid;

    if constexpr (detail::has_vertex_value<const G>) {
      using VV = std::remove_cvref_t<decltype(graph::vertex_value(g, u))>;
      if constexpr (detail::formattable<VV>) {
        os << "," << sep() << "\"label\":" << sep() << "\""
           << detail::json_escape(std::format("{}", graph::vertex_value(g, u))) << "\"";
      }
    }
    os << "}";
  }
  os << nl() << ind(1) << "]," << nl();

  // Edges
  os << ind(1) << "\"edges\":" << sep() << "[" << nl();
  bool first_edge = true;
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    for (auto uv : edges(g, u)) {
      auto tid = target_id(g, uv);
      if (!first_edge) os << "," << nl();
      first_edge = false;

      os << ind(2) << "{\"source\":" << sep() << uid << "," << sep()
         << "\"target\":" << sep() << tid;

      if constexpr (detail::has_edge_value<const G>) {
        using EV = std::remove_cvref_t<decltype(graph::edge_value(g, uv))>;
        if constexpr (detail::formattable<EV>) {
          os << "," << sep() << "\"label\":" << sep() << "\""
             << detail::json_escape(std::format("{}", graph::edge_value(g, uv))) << "\"";
        }
      }
      os << "}";
    }
  }
  os << nl() << ind(1) << "]" << nl();
  os << "}" << nl();
}

// ---------------------------------------------------------------------------
// write_json — user-supplied attribute functions
// ---------------------------------------------------------------------------

/**
 * @brief Write a graph in JSON format with user-supplied attribute maps.
 *
 * @param os           Output stream.
 * @param g            Graph satisfying adjacency_list.
 * @param vertex_data  Callable (const G&, vertex_id_t<G>) -> map<string, string>.
 *                     Returns key-value pairs to include in the node object.
 * @param edge_data    Callable (const G&, vertex_id_t<G> src, vertex_id_t<G> tgt, edge_t<G>) -> map<string, string>.
 *                     Returns key-value pairs to include in the edge object.
 * @param indent       Number of spaces for indentation (0 for compact).
 */
template <adj_list::adjacency_list G, class VDataFn, class EDataFn>
void write_json(std::ostream& os, const G& g,
                VDataFn vertex_data, EDataFn edge_data,
                int indent = 2) {
  auto ind = [indent](int level) -> std::string {
    if (indent == 0) return {};
    return std::string(static_cast<size_t>(indent * level), ' ');
  };
  auto nl = [indent]() -> std::string_view { return indent > 0 ? "\n" : ""; };
  auto sep = [indent]() -> std::string_view { return indent > 0 ? " " : ""; };

  os << "{" << nl();
  os << ind(1) << "\"directed\":" << sep() << "true," << nl();

  // Nodes
  os << ind(1) << "\"nodes\":" << sep() << "[" << nl();
  bool first_node = true;
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    if (!first_node) os << "," << nl();
    first_node = false;

    os << ind(2) << "{\"id\":" << sep() << uid;

    auto vd = vertex_data(g, uid);
    for (const auto& [k, v] : vd) {
      os << "," << sep() << "\"" << detail::json_escape(k) << "\":" << sep()
         << "\"" << detail::json_escape(v) << "\"";
    }
    os << "}";
  }
  os << nl() << ind(1) << "]," << nl();

  // Edges
  os << ind(1) << "\"edges\":" << sep() << "[" << nl();
  bool first_edge = true;
  for (auto u : vertices(g)) {
    auto uid = vertex_id(g, u);
    for (auto uv : edges(g, u)) {
      auto tid = target_id(g, uv);
      if (!first_edge) os << "," << nl();
      first_edge = false;

      os << ind(2) << "{\"source\":" << sep() << uid << "," << sep()
         << "\"target\":" << sep() << tid;

      auto ed = edge_data(g, uid, tid, uv);
      for (const auto& [k, v] : ed) {
        os << "," << sep() << "\"" << detail::json_escape(k) << "\":" << sep()
           << "\"" << detail::json_escape(v) << "\"";
      }
      os << "}";
    }
  }
  os << nl() << ind(1) << "]" << nl();
  os << "}" << nl();
}

// ---------------------------------------------------------------------------
// read_json — simple JSON graph parser
// ---------------------------------------------------------------------------

/// Parsed JSON graph node.
struct json_node {
  std::string                        id;
  std::map<std::string, std::string> attrs; ///< All attributes as strings
};

/// Parsed JSON graph edge.
struct json_edge {
  std::string                        source;
  std::string                        target;
  std::map<std::string, std::string> attrs; ///< All attributes as strings
};

/// Parsed JSON graph.
struct json_graph {
  bool                   directed{true};
  std::vector<json_node> nodes;
  std::vector<json_edge> edges;
};

namespace detail {

  // Minimal JSON tokenizer for graph parsing
  enum class json_token_type { string, number, lbrace, rbrace, lbracket, rbracket, colon, comma, true_val, false_val, null_val, eof };

  struct json_token {
    json_token_type type;
    std::string     value;
  };

  class json_lexer {
    std::string_view src_;
    size_t           pos_{0};

    void skip_ws() {
      while (pos_ < src_.size() && (src_[pos_] == ' ' || src_[pos_] == '\t' ||
                                     src_[pos_] == '\n' || src_[pos_] == '\r'))
        ++pos_;
    }

  public:
    explicit json_lexer(std::string_view s) : src_(s) {}

    json_token next() {
      skip_ws();
      if (pos_ >= src_.size()) return {json_token_type::eof, {}};

      char c = src_[pos_];
      switch (c) {
        case '{': ++pos_; return {json_token_type::lbrace, "{"};
        case '}': ++pos_; return {json_token_type::rbrace, "}"};
        case '[': ++pos_; return {json_token_type::lbracket, "["};
        case ']': ++pos_; return {json_token_type::rbracket, "]"};
        case ':': ++pos_; return {json_token_type::colon, ":"};
        case ',': ++pos_; return {json_token_type::comma, ","};
        case '"': {
          ++pos_;
          std::string val;
          while (pos_ < src_.size() && src_[pos_] != '"') {
            if (src_[pos_] == '\\' && pos_ + 1 < src_.size()) {
              ++pos_;
              switch (src_[pos_]) {
                case '"':  val += '"'; break;
                case '\\': val += '\\'; break;
                case 'n':  val += '\n'; break;
                case 'r':  val += '\r'; break;
                case 't':  val += '\t'; break;
                case 'b':  val += '\b'; break;
                case 'f':  val += '\f'; break;
                default:   val += src_[pos_]; break;
              }
            } else {
              val += src_[pos_];
            }
            ++pos_;
          }
          if (pos_ < src_.size()) ++pos_; // skip closing quote
          return {json_token_type::string, std::move(val)};
        }
        default: {
          if (c == '-' || (c >= '0' && c <= '9')) {
            size_t start = pos_;
            if (c == '-') ++pos_;
            while (pos_ < src_.size() && ((src_[pos_] >= '0' && src_[pos_] <= '9') ||
                                           src_[pos_] == '.' || src_[pos_] == 'e' ||
                                           src_[pos_] == 'E' || src_[pos_] == '+' || src_[pos_] == '-'))
              ++pos_;
            return {json_token_type::number, std::string(src_.substr(start, pos_ - start))};
          }
          if (src_.substr(pos_, 4) == "true") { pos_ += 4; return {json_token_type::true_val, "true"}; }
          if (src_.substr(pos_, 5) == "false") { pos_ += 5; return {json_token_type::false_val, "false"}; }
          if (src_.substr(pos_, 4) == "null") { pos_ += 4; return {json_token_type::null_val, "null"}; }
          ++pos_; // skip unknown
          return next();
        }
      }
    }

    json_token peek() {
      size_t saved = pos_;
      auto   tok   = next();
      pos_ = saved;
      return tok;
    }
  };

  // Simple recursive descent for the graph JSON subset
  inline void skip_json_value(json_lexer& lex) {
    auto tok = lex.next();
    if (tok.type == json_token_type::lbrace) {
      while (lex.peek().type != json_token_type::rbrace && lex.peek().type != json_token_type::eof) {
        lex.next(); // key
        lex.next(); // colon
        skip_json_value(lex);
        if (lex.peek().type == json_token_type::comma) lex.next();
      }
      lex.next(); // rbrace
    } else if (tok.type == json_token_type::lbracket) {
      while (lex.peek().type != json_token_type::rbracket && lex.peek().type != json_token_type::eof) {
        skip_json_value(lex);
        if (lex.peek().type == json_token_type::comma) lex.next();
      }
      lex.next(); // rbracket
    }
    // For string/number/bool/null, already consumed
  }

  inline std::map<std::string, std::string> parse_json_object_flat(json_lexer& lex) {
    std::map<std::string, std::string> result;
    // Assume '{' already consumed
    while (lex.peek().type != json_token_type::rbrace && lex.peek().type != json_token_type::eof) {
      auto key = lex.next(); // key (string)
      lex.next();            // colon
      auto val = lex.peek();
      if (val.type == json_token_type::string || val.type == json_token_type::number ||
          val.type == json_token_type::true_val || val.type == json_token_type::false_val ||
          val.type == json_token_type::null_val) {
        auto v = lex.next();
        result[key.value] = v.value;
      } else {
        skip_json_value(lex); // skip nested objects/arrays
      }
      if (lex.peek().type == json_token_type::comma) lex.next();
    }
    lex.next(); // rbrace
    return result;
  }

} // namespace detail

/**
 * @brief Parse a JSON graph file into a json_graph structure.
 *
 * Expected format:
 * {
 *   "directed": true|false,
 *   "nodes": [ {"id": ..., ...}, ... ],
 *   "edges": [ {"source": ..., "target": ..., ...}, ... ]
 * }
 *
 * Node "id" and edge "source"/"target" may be strings or numbers.
 * All other fields are stored as string key-value pairs in attrs.
 *
 * @param is Input stream containing JSON text.
 * @return Parsed json_graph.
 */
inline json_graph read_json(std::istream& is) {
  json_graph result;
  std::string content((std::istreambuf_iterator<char>(is)),
                       std::istreambuf_iterator<char>());

  detail::json_lexer lex(content);

  auto tok = lex.next(); // '{'
  if (tok.type != detail::json_token_type::lbrace) return result;

  while (lex.peek().type != detail::json_token_type::rbrace &&
         lex.peek().type != detail::json_token_type::eof) {
    auto key = lex.next(); // key
    lex.next();            // colon

    if (key.value == "directed") {
      auto val = lex.next();
      result.directed = (val.value == "true" || val.value == "1");
    } else if (key.value == "nodes") {
      lex.next(); // '['
      while (lex.peek().type != detail::json_token_type::rbracket &&
             lex.peek().type != detail::json_token_type::eof) {
        lex.next(); // '{'
        auto obj = detail::parse_json_object_flat(lex);

        json_node node;
        if (auto it = obj.find("id"); it != obj.end()) {
          node.id = it->second;
          obj.erase(it);
        }
        node.attrs = std::move(obj);
        result.nodes.push_back(std::move(node));

        if (lex.peek().type == detail::json_token_type::comma) lex.next();
      }
      lex.next(); // ']'
    } else if (key.value == "edges") {
      lex.next(); // '['
      while (lex.peek().type != detail::json_token_type::rbracket &&
             lex.peek().type != detail::json_token_type::eof) {
        lex.next(); // '{'
        auto obj = detail::parse_json_object_flat(lex);

        json_edge edge;
        if (auto it = obj.find("source"); it != obj.end()) {
          edge.source = it->second;
          obj.erase(it);
        }
        if (auto it = obj.find("target"); it != obj.end()) {
          edge.target = it->second;
          obj.erase(it);
        }
        edge.attrs = std::move(obj);
        result.edges.push_back(std::move(edge));

        if (lex.peek().type == detail::json_token_type::comma) lex.next();
      }
      lex.next(); // ']'
    } else {
      detail::skip_json_value(lex);
    }

    if (lex.peek().type == detail::json_token_type::comma) lex.next();
  }

  return result;
}

} // namespace graph::io
