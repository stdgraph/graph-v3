# Create abstract edge list support

The abstract edge list, a complement to the abstract adjacency list, needs to be implemented.

An edge list is a container or range of edge values that can be projected to the `edge_info` 
data structure defined in `graph_info.hpp`. This provides an alternative graph representation 
focused on edges rather than vertices. 

There are multiple (and subtle forms) of edge list, as follows:
- "edge list" with a space between the words refers to the abstract edge list data structure.
- The "edge_list" namespace (with an underscore) is the namespace for the abstract edge list
  implementation.
- "edgelist" (one word) refers to the edgelist view used by the abstract adjacency list data structure.

## Goals
Create an implementation for an edge list in the `graph::edge_list` namespace, a peer to 
`graph::adj_list`. 

In the future, there will be an edgelist view that iterates through all outgoing edges of 
all vertices that will also project `edge_info` values. This will allow algorithms to work 
equally well against either:
- The abstract edge list data structure, or
- All edges in an abstract adjacency list (via the edgelist view)

## Important Details
`/mnt/d/dev_graph/graph-v2` contains an earlier version of this library (graph-v2) that uses
a reference-based define instead of the value-based descriptors. It has a similar directory
layout, including the `edge_list/` directory for the edge list implementation that can be used 
as a reference. I don't think our edge list implementation needs descriptors, so the reference
implementation may be able to be copied with little change.

The adjacency list views for an edge list will be named `edgelist` in order to distinguish from the
abstract edge list data structure in the `edge_list` namespace.

`graph_info.hpp` contains common definitions of classes/structs that are shared between both the 
adjacency list and edge list data structures. While `edge_info` is the primary shared type, others 
could be added in the future.
