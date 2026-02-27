# Create abstract edge list support

The abstract edge list, a complement to the abstract adjacency list, needs to be implemented.

An edge list is a range of edge values that have a target_id, source_id and optional edge_value.
This can be represented by projecting values on into an `edge_data` data structure defined in 
`graph_data.hpp`. This is an example, and other methods should be considered to implement it.

## Edge Lists Types and Naming
The term edge list has different forms.
- The abstract edge list is a range of values with a `target_id`, `source_id` and optional 
  `edge_value` members. They can have different names, so the library needs to be able to 
  enable the user to expose them into those names using the matching CPO functions. `edge_list`
  (with an underscore separator) will be used to refer to this.
- `edgelist` (one word) refers to an edge list view of all edges in an adjacency list. This has
  not been implemented yet.

## Goals
Create an implementation for an edge list in the `graph::edge_list` namespace, a peer to 
`graph::adj_list`. 

Future algorithms that require an edge list must be able to work with either an `edge_list`
or an `edgelist` view without caring which one it is using.

CPOs for `source_id(g,uv)`, `target_id(g,uv)` and `edge_value(g,uv)` need to work on an adjacency list
edge, a `edge_list` value, and an `edgelist` view. This will help unify the overall design. Observations 
to consider for implementation:
- Extend the `source_id(g,uv)`, `target_id(g,uv)` and `edge_value(g,uv)` CPOs to handle the `edge_list` values.
- Extend the edge descriptor to handle the `edge_list` values.

## Reference Implementation
`/mnt/d/dev_graph/graph-v2` contains an earlier version of this library (graph-v2) that uses
a reference-based define instead of the value-based descriptors. It has a similar directory
layout, including the `edge_list/` subdirectory for the edge list implementation that can be used 
as a reference design. The implementation for this library may need to be different.

## Important Details
`graph_data.hpp` contains common definitions of classes/structs that are shared between both the 
adjacency list and edge list data structures. While `edge_data` is the primary shared type, others 
could be added in the future.
