/**
 * @file io.hpp
 * @brief Convenience umbrella header for all graph I/O formats.
 *
 * Include this single header to access all built-in graph I/O:
 *   - DOT (GraphViz):  write_dot(), read_dot()
 *   - GraphML (XML):   write_graphml(), read_graphml()
 *   - JSON:            write_json(), read_json()
 *
 * All writers use std::format for zero-config value serialization when the
 * value type satisfies std::formatter<T>. Custom attribute functions can
 * override the default formatting.
 */

#pragma once

#include <graph/io/dot.hpp>
#include <graph/io/graphml.hpp>
#include <graph/io/json.hpp>
