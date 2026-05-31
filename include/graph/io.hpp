/**
 * @file io.hpp
 * @brief Convenience umbrella header for all graph I/O formats.
 *
 * Include this single header to access all built-in graph I/O:
 *   - DOT (GraphViz):       write_dot(), read_dot()
 *   - GraphML (XML):        write_graphml(), read_graphml()
 *   - JSON:                 write_json(), read_json()
 *   - DIMACS:               write_dimacs(), write_dimacs_max_flow(), read_dimacs()
 *   - METIS:                write_metis(), read_metis()
 *   - Adjacency List Text:  write_adjacency_list_text(), read_adjacency_list_text()
 *
 * All writers use std::format for zero-config value serialization when the
 * value type satisfies std::formatter<T>. Custom attribute functions can
 * override the default formatting.
 */

#pragma once

#include <graph/io/adjacency_list_text.hpp>
#include <graph/io/dimacs.hpp>
#include <graph/io/dot.hpp>
#include <graph/io/graphml.hpp>
#include <graph/io/json.hpp>
#include <graph/io/metis.hpp>
