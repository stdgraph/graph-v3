# Design and Implementation of Uniform Properties

The current implementation treats properties in different ways. This can be seen in 
dijkstra_shortest_paths.hpp, where the edge weight is a property on the edge, and the 
distance and predecessor values are external containers.

Function objects should be used to access properties that are used in the algorithms to 
provide a uniform access method. For instance, the function object may refer to a value 
on a materialized vertex or edge object, or an external container, given the appropriate 
vertex or edge descriptor.

## Requirements
- Define `vertex_property_function` and `vertex_arithmetic_property_function` concepts. They should
  be similar to `basic_edge_weight_function` and `edge_weight_function` concepts respectfully.
- Update functions in dijkstra_shortest_paths.hpp to accept function objects for `distances`
  and `predecessor`.
- Create test examples using external containers and vertex/edge properties.

## Other
- Consider helper objects/functions to initialize values (e.g. predecessors and distances)
- Layered/tiered architecture to enable common default behavior with the ability to override for more complex situations. For instance, a distances object that stores a vector of distances that is initialized (common), but also allows the user to define their own vector.
- The user may find it easier to specify a property using an enum value. Define a way to associate an enum
  value with a accessor function.

