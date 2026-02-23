/**
 * @file dijkstra_clrs_example.cpp
 * @brief Example demonstrating Dijkstra's shortest path algorithm
 * 
 * This example shows how to:
 * - Create a weighted directed graph using the graph library
 * - Run Dijkstra's algorithm to find shortest paths
 * - Extract and print the shortest path to a destination
 */

#include <iostream>
#include <vector>
#include <limits>
#include "graph/graph.hpp"
#include "graph/container/dynamic_graph.hpp"
#include "graph/container/traits/vol_graph_traits.hpp"
#include "dijkstra_clrs.hpp"

using namespace std;
using namespace graph;

// Graph type with double edge weights, using vector for vertices and list for edges
using Graph = container::dynamic_graph<double,
                                       void,
                                       void,
                                       unsigned,
                                       false,
                                       container::vol_graph_traits<double, void, void, unsigned>>;

void print_path(const vector<unsigned>& predecessor, unsigned source, unsigned target) {
  if (target == source) {
    cout << target;
    return;
  }
  if (predecessor[target] == numeric_limits<unsigned>::max()) {
    cout << "No path exists";
    return;
  }

  // Build path in reverse
  vector<unsigned> path;
  unsigned         current = target;
  while (current != source) {
    path.push_back(current);
    current = predecessor[current];
  }
  path.push_back(source);

  // Print path from source to target
  for (auto it = path.rbegin(); it != path.rend(); ++it) {
    cout << *it;
    if (it + 1 != path.rend())
      cout << " -> ";
  }
}

int main() {
  // Create a weighted graph
  // Example graph from CLRS (Introduction to Algorithms)
  //
  //        (0)
  //       / | \
    //     10  5  \
    //     /   |   2
  //   (1)  (2)  \
    //     \   |    \
    //      1  9   3  \
    //       \ |  /    \
    //        (3)-------(4)
  //              7
  //
  // Vertices: 0, 1, 2, 3, 4

  // Create graph using initializer list of edges
  Graph g({{0, 1, 10.0},
           {0, 2, 5.0},
           {0, 4, 2.0},
           {1, 3, 1.0},
           {2, 1, 3.0},
           {2, 3, 9.0},
           {2, 4, 2.0},
           {3, 4, 7.0},
           {4, 3, 3.0}});

  cout << "Graph created with " << num_vertices(g) << " vertices\n\n";

  // Run Dijkstra's algorithm from vertex 0
  unsigned         source = 0;
  vector<double>   distance(num_vertices(g));
  vector<unsigned> predecessor(num_vertices(g));

  // Weight function extracts the edge value
  auto weight_fn = [](const auto& g, const auto& edge_desc) -> double { return edge_value(g, edge_desc); };

  dijkstra_clrs(g, source, distance, predecessor, weight_fn);

  // Print results
  cout << "Shortest paths from vertex " << source << ":\n";
  cout << string(50, '-') << "\n";

  for (unsigned target = 0; target < num_vertices(g); ++target) {
    cout << "To vertex " << target << ": ";

    if (distance[target] == numeric_limits<double>::max()) {
      cout << "unreachable\n";
    } else {
      cout << "distance = " << distance[target] << ", path = ";
      print_path(predecessor, source, target);
      cout << "\n";
    }
  }

  cout << "\n" << string(50, '-') << "\n";

  // Example: Find specific path
  unsigned destination = 3;
  cout << "\nShortest path from " << source << " to " << destination << ":\n";
  cout << "Distance: " << distance[destination] << "\n";
  cout << "Path: ";
  print_path(predecessor, source, destination);
  cout << "\n";

  // Run from a different source
  cout << "\n" << string(50, '=') << "\n\n";
  source = 2;
  fill(distance.begin(), distance.end(), 0.0);
  fill(predecessor.begin(), predecessor.end(), numeric_limits<unsigned>::max());

  dijkstra_clrs(g, source, distance, predecessor, weight_fn);

  cout << "Shortest paths from vertex " << source << ":\n";
  cout << string(50, '-') << "\n";

  for (unsigned target = 0; target < num_vertices(g); ++target) {
    cout << "To vertex " << target << ": ";

    if (distance[target] == numeric_limits<double>::max()) {
      cout << "unreachable\n";
    } else {
      cout << "distance = " << distance[target] << ", path = ";
      print_path(predecessor, source, target);
      cout << "\n";
    }
  }

  return 0;
}
