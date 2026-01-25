#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <sstream>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// Graph type aliases
using vov_uint = dynamic_graph<void, void, void, uint64_t, false,
                               vov_graph_traits<void, void, void, uint64_t, false>>;
using mos_string = dynamic_graph<void, void, void, std::string, false,
                                 mos_graph_traits<void, void, void, std::string, false>>;
using vov_int = dynamic_graph<void, void, void, int, false,
                              vov_graph_traits<void, void, void, int, false>>;
using dofl_uint = dynamic_graph<void, void, void, uint64_t, false,
                                dofl_graph_traits<void, void, void, uint64_t, false>>;

// ID mapper class: converts IDs from one type to another
template<typename FromId, typename ToId>
class id_mapper {
public:
    using from_id_type = FromId;
    using to_id_type = ToId;
    
    // Add a mapping
    void add_mapping(const FromId& from, const ToId& to) {
        forward_map_[from] = to;
        reverse_map_[to] = from;
    }
    
    // Convert from FromId to ToId
    ToId to(const FromId& from) const {
        auto it = forward_map_.find(from);
        if (it != forward_map_.end()) {
            return it->second;
        }
        throw std::out_of_range("ID not found in mapper");
    }
    
    // Convert from ToId to FromId
    FromId from(const ToId& to) const {
        auto it = reverse_map_.find(to);
        if (it != reverse_map_.end()) {
            return it->second;
        }
        throw std::out_of_range("ID not found in mapper");
    }
    
    // Check if mapping exists
    bool has_forward(const FromId& from) const {
        return forward_map_.find(from) != forward_map_.end();
    }
    
    bool has_reverse(const ToId& to) const {
        return reverse_map_.find(to) != reverse_map_.end();
    }
    
    size_t size() const { return forward_map_.size(); }
    
private:
    std::map<FromId, ToId> forward_map_;
    std::map<ToId, FromId> reverse_map_;
};

// Helper: Convert graph with ID mapping
template<typename SourceGraph, typename TargetGraph, typename FromId, typename ToId>
TargetGraph convert_graph(const SourceGraph& source, const id_mapper<FromId, ToId>& mapper) {
    std::vector<copyable_edge_t<ToId, void>> edge_list;
    std::vector<copyable_vertex_t<ToId, void>> vertex_list;
    
    // Add all vertices (even isolated ones)
    for (auto&& u : vertices(source)) {
        auto uid = vertex_id(source, u);
        vertex_list.push_back({.id = mapper.to(uid)});
    }
    
    // Add all edges
    for (auto&& u : vertices(source)) {
        auto uid = vertex_id(source, u);
        for (auto&& e : edges(source, u)) {
            auto vid = target_id(source, e);
            edge_list.push_back({.source_id = mapper.to(uid), .target_id = mapper.to(vid)});
        }
    }
    
    std::vector<ToId> partitions;
    return TargetGraph(edge_list, vertex_list, identity{}, identity{}, partitions);
}

// Helper: Count edges
template<typename G>
size_t count_edges(const G& g) {
    size_t count = 0;
    for (auto&& u : vertices(g)) {
        count += static_cast<size_t>(std::ranges::distance(edges(g, u)));
    }
    return count;
}

// Helper: Count vertices
template<typename G>
size_t count_vertices(const G& g) {
    return static_cast<size_t>(std::ranges::distance(vertices(g)));
}

// Helper: Check if edge exists
template<typename G, typename VId>
bool has_edge_helper(const G& g, const VId& uid, const VId& vid) {
    for (auto&& u : vertices(g)) {
        if (vertex_id(g, u) == uid) {
            for (auto&& e : edges(g, u)) {
                if (target_id(g, e) == vid) {
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

TEST_CASE("Convert string graph to integral graph with ID mapper", "[6.4.2][conversion][string-to-int]") {
    // Create string ID graph
    mos_string g_str({{.source_id = "A", .target_id = "B"}, 
                      {.source_id = "B", .target_id = "C"}, 
                      {.source_id = "A", .target_id = "C"}});
    
    // Create ID mapper
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("A", 0);
    mapper.add_mapping("B", 1);
    mapper.add_mapping("C", 2);
    
    // Convert to integral ID graph
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 3);
    REQUIRE(count_edges(g_int) == 3);
    REQUIRE(has_edge_helper(g_int, 0ul, 1ul));
    REQUIRE(has_edge_helper(g_int, 1ul, 2ul));
    REQUIRE(has_edge_helper(g_int, 0ul, 2ul));
}

TEST_CASE("Convert integral graph to string graph with ID mapper", "[6.4.2][conversion][int-to-string]") {
    // Create integral ID graph
    vov_uint g_int({{.source_id = 0, .target_id = 1}, 
                    {.source_id = 1, .target_id = 2}, 
                    {.source_id = 0, .target_id = 2}});
    
    // Create ID mapper
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "Alice");
    mapper.add_mapping(1, "Bob");
    mapper.add_mapping(2, "Charlie");
    
    // Convert to string ID graph
    auto g_str = convert_graph<vov_uint, mos_string>(g_int, mapper);
    
    REQUIRE(count_vertices(g_str) == 3);
    REQUIRE(count_edges(g_str) == 3);
    REQUIRE(has_edge_helper(g_str, std::string("Alice"), std::string("Bob")));
    REQUIRE(has_edge_helper(g_str, std::string("Bob"), std::string("Charlie")));
    REQUIRE(has_edge_helper(g_str, std::string("Alice"), std::string("Charlie")));
}

TEST_CASE("Bijective ID mapping", "[6.4.2][conversion][bijective]") {
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "zero");
    mapper.add_mapping(1, "one");
    mapper.add_mapping(2, "two");
    
    // Forward mapping
    REQUIRE(mapper.to(0) == "zero");
    REQUIRE(mapper.to(1) == "one");
    REQUIRE(mapper.to(2) == "two");
    
    // Reverse mapping
    REQUIRE(mapper.from("zero") == 0);
    REQUIRE(mapper.from("one") == 1);
    REQUIRE(mapper.from("two") == 2);
    
    // Bidirectional consistency
    for (uint64_t i = 0; i < 3; ++i) {
        REQUIRE(mapper.from(mapper.to(i)) == i);
    }
}

TEST_CASE("Sparse ID mapping", "[6.4.2][conversion][sparse]") {
    // Create graph with sparse IDs (non-contiguous)
    mos_string g_str({{.source_id = "node_10", .target_id = "node_20"}, 
                      {.source_id = "node_20", .target_id = "node_50"}});
    
    // Map sparse string IDs to compact integral IDs
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("node_10", 0);
    mapper.add_mapping("node_20", 1);
    mapper.add_mapping("node_50", 2);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 3);
    REQUIRE(count_edges(g_int) == 2);
    REQUIRE(has_edge_helper(g_int, 0ul, 1ul));
    REQUIRE(has_edge_helper(g_int, 1ul, 2ul));
}

TEST_CASE("Empty graph conversion", "[6.4.2][conversion][empty]") {
    mos_string g_str;
    id_mapper<std::string, uint64_t> mapper;
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 0);
    REQUIRE(count_edges(g_int) == 0);
}

TEST_CASE("Single vertex graph conversion", "[6.4.2][conversion][single-vertex]") {
    mos_string g_str({{.source_id = "A", .target_id = "A"}});  // Self-loop
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("A", 0);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 1);
    REQUIRE(count_edges(g_int) == 1);
    REQUIRE(has_edge_helper(g_int, 0ul, 0ul));
}

TEST_CASE("Cycle graph conversion", "[6.4.2][conversion][cycle]") {
    mos_string g_str({{.source_id = "A", .target_id = "B"}, 
                      {.source_id = "B", .target_id = "C"}, 
                      {.source_id = "C", .target_id = "A"}});
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("A", 0);
    mapper.add_mapping("B", 1);
    mapper.add_mapping("C", 2);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 3);
    REQUIRE(count_edges(g_int) == 3);
    REQUIRE(has_edge_helper(g_int, 0ul, 1ul));
    REQUIRE(has_edge_helper(g_int, 1ul, 2ul));
    REQUIRE(has_edge_helper(g_int, 2ul, 0ul));
}

TEST_CASE("Disconnected graph conversion", "[6.4.2][conversion][disconnected]") {
    mos_string g_str({{.source_id = "A", .target_id = "B"}, 
                      {.source_id = "C", .target_id = "D"}});
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("A", 0);
    mapper.add_mapping("B", 1);
    mapper.add_mapping("C", 2);
    mapper.add_mapping("D", 3);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 4);
    REQUIRE(count_edges(g_int) == 2);
    REQUIRE(has_edge_helper(g_int, 0ul, 1ul));
    REQUIRE(has_edge_helper(g_int, 2ul, 3ul));
}

TEST_CASE("Star graph conversion", "[6.4.2][conversion][star]") {
    mos_string g_str({{.source_id = "center", .target_id = "A"}, 
                      {.source_id = "center", .target_id = "B"}, 
                      {.source_id = "center", .target_id = "C"}});
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("center", 0);
    mapper.add_mapping("A", 1);
    mapper.add_mapping("B", 2);
    mapper.add_mapping("C", 3);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 4);
    REQUIRE(count_edges(g_int) == 3);
    REQUIRE(has_edge_helper(g_int, 0ul, 1ul));
    REQUIRE(has_edge_helper(g_int, 0ul, 2ul));
    REQUIRE(has_edge_helper(g_int, 0ul, 3ul));
}

TEST_CASE("ID mapper with numeric strings", "[6.4.2][conversion][numeric-strings]") {
    vov_uint g_int({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
    
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "0");
    mapper.add_mapping(1, "1");
    mapper.add_mapping(2, "2");
    
    auto g_str = convert_graph<vov_uint, mos_string>(g_int, mapper);
    
    REQUIRE(count_vertices(g_str) == 3);
    REQUIRE(count_edges(g_str) == 2);
    REQUIRE(has_edge_helper(g_str, std::string("0"), std::string("1")));
    REQUIRE(has_edge_helper(g_str, std::string("1"), std::string("2")));
}

TEST_CASE("Round-trip conversion preserves structure", "[6.4.2][conversion][round-trip]") {
    // Start with string graph
    mos_string g1({{.source_id = "A", .target_id = "B"}, {.source_id = "B", .target_id = "C"}});
    
    // Convert to integral
    id_mapper<std::string, uint64_t> mapper1;
    mapper1.add_mapping("A", 0);
    mapper1.add_mapping("B", 1);
    mapper1.add_mapping("C", 2);
    auto g2 = convert_graph<mos_string, vov_uint>(g1, mapper1);
    
    // Convert back to string
    id_mapper<uint64_t, std::string> mapper2;
    mapper2.add_mapping(0, "A");
    mapper2.add_mapping(1, "B");
    mapper2.add_mapping(2, "C");
    auto g3 = convert_graph<vov_uint, mos_string>(g2, mapper2);
    
    // Verify structure preserved
    REQUIRE(count_vertices(g3) == count_vertices(g1));
    REQUIRE(count_edges(g3) == count_edges(g1));
    REQUIRE(has_edge_helper(g3, std::string("A"), std::string("B")));
    REQUIRE(has_edge_helper(g3, std::string("B"), std::string("C")));
}

TEST_CASE("Convert between different integral types", "[6.4.2][conversion][int-types]") {
    vov_uint g_uint({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
    
    id_mapper<uint64_t, int> mapper;
    mapper.add_mapping(0, 0);
    mapper.add_mapping(1, 1);
    mapper.add_mapping(2, 2);
    
    auto g_int = convert_graph<vov_uint, vov_int>(g_uint, mapper);
    
    REQUIRE(count_vertices(g_int) == 3);
    REQUIRE(count_edges(g_int) == 2);
    REQUIRE(has_edge_helper(g_int, 0, 1));
    REQUIRE(has_edge_helper(g_int, 1, 2));
}

TEST_CASE("Convert to different container type", "[6.4.2][conversion][container-types]") {
    vov_uint g_vov({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
    
    id_mapper<uint64_t, uint64_t> mapper;
    mapper.add_mapping(0, 0);
    mapper.add_mapping(1, 1);
    mapper.add_mapping(2, 2);
    
    auto g_dofl = convert_graph<vov_uint, dofl_uint>(g_vov, mapper);
    
    REQUIRE(count_vertices(g_dofl) == 3);
    REQUIRE(count_edges(g_dofl) == 2);
    REQUIRE(has_edge_helper(g_dofl, 0ul, 1ul));
    REQUIRE(has_edge_helper(g_dofl, 1ul, 2ul));
}

TEST_CASE("ID mapper missing forward mapping throws", "[6.4.2][conversion][error-handling]") {
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "zero");
    
    REQUIRE_THROWS_AS(mapper.to(1), std::out_of_range);
}

TEST_CASE("ID mapper missing reverse mapping throws", "[6.4.2][conversion][error-handling]") {
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "zero");
    
    REQUIRE_THROWS_AS(mapper.from("one"), std::out_of_range);
}

TEST_CASE("ID mapper has_forward check", "[6.4.2][conversion][existence-check]") {
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "zero");
    mapper.add_mapping(1, "one");
    
    REQUIRE(mapper.has_forward(0));
    REQUIRE(mapper.has_forward(1));
    REQUIRE_FALSE(mapper.has_forward(2));
}

TEST_CASE("ID mapper has_reverse check", "[6.4.2][conversion][existence-check]") {
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "zero");
    mapper.add_mapping(1, "one");
    
    REQUIRE(mapper.has_reverse("zero"));
    REQUIRE(mapper.has_reverse("one"));
    REQUIRE_FALSE(mapper.has_reverse("two"));
}

TEST_CASE("ID mapper size", "[6.4.2][conversion][size]") {
    id_mapper<uint64_t, std::string> mapper;
    REQUIRE(mapper.size() == 0);
    
    mapper.add_mapping(0, "zero");
    REQUIRE(mapper.size() == 1);
    
    mapper.add_mapping(1, "one");
    REQUIRE(mapper.size() == 2);
}

TEST_CASE("Large graph conversion", "[6.4.2][conversion][large]") {
    // Create large integral graph
    std::vector<copyable_edge_t<uint64_t, void>> edge_list;
    for (uint64_t i = 0; i < 100; ++i) {
        edge_list.push_back({.source_id = i, .target_id = (i + 1) % 100});
    }
    std::vector<copyable_vertex_t<uint64_t, void>> vertex_list;
    std::vector<uint64_t> partitions;
    vov_uint g_int(edge_list, vertex_list, identity{}, identity{}, partitions);
    
    // Create mapper
    id_mapper<uint64_t, std::string> mapper;
    for (uint64_t i = 0; i < 100; ++i) {
        std::ostringstream oss;
        oss << "node_" << i;
        mapper.add_mapping(i, oss.str());
    }
    
    // Convert
    auto g_str = convert_graph<vov_uint, mos_string>(g_int, mapper);
    
    REQUIRE(count_vertices(g_str) == 100);
    REQUIRE(count_edges(g_str) == 100);
}

TEST_CASE("Conversion preserves self-loops", "[6.4.2][conversion][self-loops]") {
    mos_string g_str({{.source_id = "A", .target_id = "A"}, 
                      {.source_id = "B", .target_id = "B"}});
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("A", 0);
    mapper.add_mapping("B", 1);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_edges(g_int) == 2);
    REQUIRE(has_edge_helper(g_int, 0ul, 0ul));
    REQUIRE(has_edge_helper(g_int, 1ul, 1ul));
}

TEST_CASE("Conversion with UUID-like string IDs", "[6.4.2][conversion][uuid]") {
    mos_string g_str({{.source_id = "550e8400-e29b-41d4-a716-446655440000", 
                       .target_id = "550e8400-e29b-41d4-a716-446655440001"}});
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("550e8400-e29b-41d4-a716-446655440000", 0);
    mapper.add_mapping("550e8400-e29b-41d4-a716-446655440001", 1);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 2);
    REQUIRE(count_edges(g_int) == 1);
    REQUIRE(has_edge_helper(g_int, 0ul, 1ul));
}

TEST_CASE("Conversion with path-like string IDs", "[6.4.2][conversion][paths]") {
    mos_string g_str({{.source_id = "/root/dir1", .target_id = "/root/dir2"}, 
                      {.source_id = "/root/dir2", .target_id = "/root/dir3"}});
    
    id_mapper<std::string, uint64_t> mapper;
    mapper.add_mapping("/root/dir1", 0);
    mapper.add_mapping("/root/dir2", 1);
    mapper.add_mapping("/root/dir3", 2);
    
    auto g_int = convert_graph<mos_string, vov_uint>(g_str, mapper);
    
    REQUIRE(count_vertices(g_int) == 3);
    REQUIRE(count_edges(g_int) == 2);
}

TEST_CASE("Multiple conversions in sequence", "[6.4.2][conversion][sequence]") {
    // Start with string graph
    mos_string g1({{.source_id = "A", .target_id = "B"}});
    
    // Convert to uint
    id_mapper<std::string, uint64_t> mapper1;
    mapper1.add_mapping("A", 0);
    mapper1.add_mapping("B", 1);
    auto g2 = convert_graph<mos_string, vov_uint>(g1, mapper1);
    
    // Convert to int
    id_mapper<uint64_t, int> mapper2;
    mapper2.add_mapping(0, 0);
    mapper2.add_mapping(1, 1);
    auto g3 = convert_graph<vov_uint, vov_int>(g2, mapper2);
    
    // Verify final result
    REQUIRE(count_vertices(g3) == 2);
    REQUIRE(count_edges(g3) == 1);
    REQUIRE(has_edge_helper(g3, 0, 1));
}

TEST_CASE("Convert graph with no edges (vertices only)", "[6.4.2]") {
    // Create graph with isolated vertices (no edges)
    std::vector<copyable_edge_t<uint64_t, void>> edges_uint;
    std::vector<copyable_vertex_t<uint64_t, void>> vertices_uint = {{0}, {1}, {2}, {3}, {4}};
    std::vector<uint64_t> partitions;
    
    vov_uint g_uint(edges_uint, vertices_uint, identity{}, identity{}, partitions);
    
    // Create ID mapper (contiguous for sequential container)
    id_mapper<uint64_t, int> mapper;
    mapper.add_mapping(0, 0);
    mapper.add_mapping(1, 1);
    mapper.add_mapping(2, 2);
    mapper.add_mapping(3, 3);
    mapper.add_mapping(4, 4);
    
    // Convert to int IDs (both sequential containers preserve isolated vertices)
    auto g_int = convert_graph<vov_uint, vov_int>(g_uint, mapper);
    
    // Verify vertices exist with no edges
    REQUIRE(count_vertices(g_int) == 5);
    REQUIRE(count_edges(g_int) == 0);
    
    // Verify all mapped IDs exist
    REQUIRE(mapper.has_forward(0));
    REQUIRE(mapper.has_forward(4));
}

TEST_CASE("Convert graph with parallel edges (multiple edges between same vertices)", "[6.4.2]") {
    // Create graph with parallel edges: 0->1 (multiple times)
    std::vector<copyable_edge_t<uint64_t, void>> edges_uint = {
        {0, 1},
        {0, 1},  // Duplicate edge
        {0, 1},  // Another duplicate
        {1, 2}
    };
    std::vector<copyable_vertex_t<uint64_t, void>> vertices_uint = {{0}, {1}, {2}};
    std::vector<uint64_t> partitions;
    
    vov_uint g_uint(edges_uint, vertices_uint, identity{}, identity{}, partitions);
    
    // Create ID mapper
    id_mapper<uint64_t, std::string> mapper;
    mapper.add_mapping(0, "A");
    mapper.add_mapping(1, "B");
    mapper.add_mapping(2, "C");
    
    // Convert to string IDs
    auto g_str = convert_graph<vov_uint, mos_string>(g_uint, mapper);
    
    // Note: mos uses std::set for edges, which deduplicates
    // vov uses std::vector, which preserves duplicates
    REQUIRE(count_vertices(g_str) == 3);
    
    // vov should have 4 edges, mos will deduplicate to 2 unique edges
    // This demonstrates different container behaviors
    auto edge_count_str = count_edges(g_str);
    REQUIRE(edge_count_str == 2);  // mos deduplicates: A->B and B->C
    
    // Original vov preserves all 4 edges
    REQUIRE(count_edges(g_uint) == 4);
}
