# BGL Workshop Outline

I (Phil) am scheduled to do a presentation to BGL users with Andrew on May 6, 2026. 
We have 90 minutes with the following agenda:
- Retrospective on BGL and lessons learned over 26 years (Andrew)
- The nwgraph library using standard C++ containers (Andrew)
- graph-v3 and standardization (Phil)
- Graphs and agentic coding (Phil)
  - Creating graph-v3 using vibe coding 
  - Observations on using an AI agent to convert BGL to use C++20
- Q&A (15 minutes)

Definitions and People
- BGL: Boost Graph Library written circa 2000 by Andrew Lumsdaine, et al
- graph-v3: A modern graph library using C++20 by Phil Ratzloff with contributors Andrew Lumsdaine
  Kevin Deweese, and Scott McMillan. It was started in 2021 with the goal of adding it to the
  C++ standard library.

The following sections describe the organization of the presentation, but it may change as
it is refined.

## Introduction to graph-v3
This section compares graph-v3 with BGL

### Kevin Bacon

General Syntax differences
- Graph data: actors, directors, and movies
  - Show graph image
- Problem(s): six degrees of Kevin Bacon
  - Bacon number
- Example 1
  - graph construction
  - edge iteration, views
  - Bacon number
- Example 2
  - non-integral & sparse vertex ids

### French Roads
  - properties


  - Dijkstra

### Performance comparison
- P3337 

### Agentic Coding
  - graph-v3
  - bgl modernization
  - algorithm demo (optional): Adamic-Adar Index, Transitive Closure

### Parallel Narrative

- goals: 
  - easy to use, natural C++20 style when used
  - expressive
  - performant
- SG19
  - getting started
  - learn, grow (be open to critique, set aside ego)
  - play with the big boys
- Discoveries
  - ranges
  - CPOs
  - Descriptors
- Current State
  
## Design
- Guiding Principles: simplicity, expressiveness, flexibility, performance
  - use features & idioms in the C++20 std: ranges, concepts, r-value references, free functions (e.g. begin(r), end(r)), structured bindings, fnc objs
- Architecture
  - Layers: algorithms, views, GCI, graph containers
  - Data model: range-of-ranges as key abstraction supported & enforced by concepts
    - algorithms writes to
    - graphs are adapted to
  - primitive functions (CPOs) and default impl
    - values (std) vs. properties
    - descriptors
- Views
- Algorithms
  - visitors
- Graph Containers
  - std-based container patterns
  - library graphs: compressed_graph, dynamic_graph, undirected_graph
  - BYOG; patterns supported
    - typical integration - 4-7 fnc overloads

## Feature Comparison		BGL		graph-v3
- Create & init graph
- vertex & edge iteration
- concepts, traits, types
  - graph_traits                required        only on dynamic_graph
- properties

- Other BGL
  - BGL wrapper


## Miscellaneous Notes (agent must ignore)

            			BGL		      graph-v3
Graph Defintion		graph_traits	

properties: property maps vs. fnc objs, internal vs. external properties
concepts
parallel algorithms
distributed algorithms
features not supported (yet)
function creation
Examples: Kevin Bacon, social network, road network
iteration
key C++ features: ranges, concepts, structured bindings, constexpr
BYOG; patterns supported

