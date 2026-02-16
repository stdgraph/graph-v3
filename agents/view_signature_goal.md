
# Refactor View Functions for Consistency and Predictability

When writing an algorithm it's assumed that the author will typically use an id-oriented style (e.g.
vertex_id_t<G>) or a descriptor-oriented style (vertex_t<G> and edge_t<G>). Using index_adjacency_list<G>
concept implies an id-oriented style.

The View functions include overloads that allow either a vertex id or vertex descriptor. The return
types should reflect the overload used. So a vertex_id_t<G> overload should have return type with vertex ids,
but a vertex_t<G> should have a return type that includes ids and a descriptor.

When working with an adjacency list or edge list as a whole, there's no option to select from an
id or descriptor. In those cases, a `basic_` prefix is used as in `basic_vertexlist` and `basic_edgelist`.

Some of the views return a struct with a single item (e.g. vertex id) which may be unexpected or appear
unusual. Input received was to keep it as-is for consistency and predictability.

The library has not been released and so backward compatibility is not a concern outside of the library.
Existing algorithms and tests may need to be updated.

Search views (DFS, BFS, topological_sort) will be addressed separately.

In the following sections, `vval` is the value returned by a `VVF` and `eval` is the value returned by a `EVF`.

The `basic_` prefix indicates that view parameters will be id's and descriptors (`vertex_t<G>` and `edge_t<G>`) will not be in the returned value. vertex ids will always be returned. The non-`basic_`
versions will use vertex descriptors in the parameters instead of vertex ids.

Algorithms and tests may need to be updated to use the views appropriately. Algorithms will typically use
the `basic_` version of views.

## vertexlist view
The views shown here increase the existing API surface. This is understood and matches the proposed views
that have gone through peer review in the past.

`vertexlist` view examples:
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&&[uid,u]:vertexlist(g)) | vertex_info<VId,V,void> ||
| for(auto&&[uid,u,vval]:vertexlist(g,vvf)) | vertex_info<VId,V,VV> || 
| for(auto&&[uid,u]:vertexlist(g,first_u,last_u)) | vertex_info<VId,V,void> || 
| for(auto&&[uid,u,vval]:vertexlist(g,first_u,last_u,vvf)) | vertex_info<VId,V,VV> || 
| for(auto&&[uid,u]:vertexlist(g,vr)) | vertex_info<VId,V,void> || 
| for(auto&&[uid,u,vval]:vertexlist(g,vr,vvf)) | vertex_info<VId,V,VV> || 

`basic_vertexlist` view examples don't include the vertex descriptor in the return type:
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&&[uid]:basic_vertexlist(g)) | vertex_info<VId,void,void> | | 
| for(auto&&[uid,vval]:basic_vertexlist(g,vvf)) | vertex_info<VId,void,VV> | | 
| for(auto&&[uid]:basic_vertexlist(g,first_uid,last_uid)) | vertex_info<VId,void,void> | | 
| for(auto&&[uid,vval]:basic_vertexlist(g,first_uid,last_uid,vvf)) | vertex_info<VId,void,VV> | | 
| for(auto&&[uid]:basic_vertexlist(g,vr)) | vertex_info<VId,void,void> | | 
| for(auto&&[uid,vval]:basic_vertexlist(g,vr,vvf)) | vertex_info<VId,void,VV> | | 

## incidence view

`incidence` view examples
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&& [vid,uv]:incidence(g,u)) | edge_info<VId,false,E,void> | | 
| for(auto&& [vid,uv,eval]:incidence(g,u,evf)) | edge_info<VId,false,E,EV> | | 

`basic_incidence` view examples
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&& [vid]:basic_incidence(g,uid)) | edge_info<VId,false,void,void> | | 
| for(auto&& [vid,eval]:basic_incidence(g,uid,evf)) | edge_info<VId,false,void,EV> | | 

## neighbors view
`neighbors` view examples
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&&[vid,v]: neighbors(g,u)) | neighbor_info<VId,false,V,void> | | 
| for(auto&&[vid,v,vval]: neighbors(g,u,vvf)) | neighbor_info<VId,false,V,VV> | | 

`basic_neighbors` view examples
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&&[vid]: basic_neighbors(g,uid)) | neighbor_info<VId,false,void,void> | | 
| for(auto&&[vid,vval]:basic_neighbors(g,uid,vvf)) | neighbor_info<VId,false,void,VV> | | 

## edgelist view

`edgelist` view examples
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&&[uid,vid,uv]:edgelist(g)) | edge_info<VId,true,E,void> | | 
| for(auto&&[uid,vid,uv,eval]:edgelist(g,evf)) | edge_info<VId,true,E,EV> | | 

`basic_edgelist` view examples don't include the edge descriptor in the return type.
| Usage | Return | Notes |
|-------|--------|-------|
| for(auto&&[uid,vid]:basic_edgelist(g)) | edge_info<VId,true,void,void> | | 
| for(auto&&[uid,vid,eval]:basic_edgelist(g,evf)) | edge_info<VId,true,void,EV> | | 

---

## Pros and Cons

### Pros

1. **Return type reflects the caller's intent.** When a caller passes a `vertex_id_t<G>`, they're working
   in an id-oriented style and don't need descriptors in the return value. Omitting them reduces cognitive
   overhead and simplifies structured bindings.

2. **Lighter-weight id-only returns for algorithms.** Algorithms typically use `index_adjacency_list<G>` and
   only need ids. Stripping descriptors from the return avoids carrying data the algorithm won't use, which
   can reduce object size and improve cache behavior in tight loops.

3. **`basic_` variants give users an explicit opt-out.** For whole-graph views (`vertexlist`, `edgelist`)
   where there's no overload parameter to signal intent, the `basic_` prefix is a clear, self-documenting
   way to request the lighter return type.

4. **Consistency across view families.** Applying the same id-vs-descriptor rule to `vertexlist`, `incidence`,
   `neighbors`, and `edgelist` makes the API predictable. A user who learns the pattern for one view can
   apply it to all others.

5. **Subrange overloads (`first,last` and `vr`) add flexibility.** They let users iterate a subset of
   vertices without building temporary containers, which is useful for partitioned graphs and
   parallel algorithms.

6. **Keeping single-item structured bindings for consistency.** The decision to keep single-member returns
   (e.g. `for(auto&&[vid]:basic_incidence(g,uid))`) rather than special-casing them means the pattern is uniform:
   every view always returns an info struct, regardless of how many fields it has. This predictability
   outweighs the minor awkwardness.

### Cons

1. **`basic_` naming may not be intuitive.** The `basic_` prefix conventionally suggests "simpler
   implementation" (cf. `std::basic_string`), not "fewer fields in the return type." Users may not
   immediately guess that `basic_incidence` means "id-only return without edge descriptor." An alternative
   like `id_incidence` or a documented naming rationale could help.

2. **`vertexlist`/`edgelist` break the parameter-type pattern.** For `incidence` and `neighbors`, `basic_`
   correlates with a different parameter type (`uid` vs `u`), making the two variants distinguishable by
   signature alone. For `vertexlist` and `edgelist`, both the full and `basic_` variants can take the same
   graph-level parameters (e.g. `basic_vertexlist(g)` vs `vertexlist(g)`). The distinction is purely by
   name. The subrange overloads *do* differentiate via parameter type (`first_u`/`last_u` vs
   `first_uid`/`last_uid`), but the simplest overloads (`g` and `g,vvf`/`g,evf`) do not.
