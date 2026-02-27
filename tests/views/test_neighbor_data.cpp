#include <catch2/catch_test_macros.hpp>
#include <graph/graph_data.hpp>
#include <type_traits>

using namespace graph;

// Mock types for testing
struct mock_vertex_descriptor {
  int id;
};

struct mock_value {
  double data;
};

TEST_CASE("neighbor_data: all 16 specializations compile", "[neighbor_data]") {
  SECTION("VId, Sourced=true, V, VV all present") {
    neighbor_data<int, true, mock_vertex_descriptor, mock_value> ni{1, 2, mock_vertex_descriptor{10}, mock_value{42.0}};
    REQUIRE(ni.source_id == 1);
    REQUIRE(ni.target_id == 2);
    REQUIRE(ni.target.id == 10);
    REQUIRE(ni.value.data == 42.0);
  }

  SECTION("VId, Sourced=true, V present; VV=void") {
    neighbor_data<int, true, mock_vertex_descriptor, void> ni{2, 3, mock_vertex_descriptor{20}};
    REQUIRE(ni.source_id == 2);
    REQUIRE(ni.target_id == 3);
    REQUIRE(ni.target.id == 20);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId, Sourced=true, VV present; V=void") {
    neighbor_data<int, true, void, mock_value> ni{3, 4, mock_value{99.9}};
    REQUIRE(ni.source_id == 3);
    REQUIRE(ni.target_id == 4);
    REQUIRE(ni.value.data == 99.9);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
  }

  SECTION("VId, Sourced=true; V=void, VV=void") {
    neighbor_data<int, true, void, void> ni{4, 5};
    REQUIRE(ni.source_id == 4);
    REQUIRE(ni.target_id == 5);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId, Sourced=false, V, VV all present") {
    neighbor_data<int, false, mock_vertex_descriptor, mock_value> ni{5, mock_vertex_descriptor{30}, mock_value{123.4}};
    REQUIRE(ni.target_id == 5);
    REQUIRE(ni.target.id == 30);
    REQUIRE(ni.value.data == 123.4);
  }

  SECTION("VId, Sourced=false, V present; VV=void") {
    neighbor_data<int, false, mock_vertex_descriptor, void> ni{6, mock_vertex_descriptor{40}};
    REQUIRE(ni.target_id == 6);
    REQUIRE(ni.target.id == 40);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId, Sourced=false, VV present; V=void") {
    neighbor_data<int, false, void, mock_value> ni{7, mock_value{77.7}};
    REQUIRE(ni.target_id == 7);
    REQUIRE(ni.value.data == 77.7);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
  }

  SECTION("VId, Sourced=false; V=void, VV=void") {
    neighbor_data<int, false, void, void> ni{8};
    REQUIRE(ni.target_id == 8);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId=void, Sourced=true; V, VV present (descriptor-based)") {
    neighbor_data<void, true, mock_vertex_descriptor, mock_value> ni{mock_vertex_descriptor{50}, mock_value{200.0}};
    REQUIRE(ni.vertex.id == 50);
    REQUIRE(ni.value.data == 200.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
  }

  SECTION("VId=void, Sourced=true, VV=void; V present") {
    neighbor_data<void, true, mock_vertex_descriptor, void> ni{mock_vertex_descriptor{60}};
    REQUIRE(ni.vertex.id == 60);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId=void, Sourced=true, V=void; VV present") {
    neighbor_data<void, true, void, mock_value> ni{mock_value{300.0}};
    REQUIRE(ni.value.data == 300.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
  }

  SECTION("VId=void, Sourced=true; V=void, VV=void (empty)") {
    neighbor_data<void, true, void, void> ni{};
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId=void, Sourced=false; V, VV present (primary pattern)") {
    neighbor_data<void, false, mock_vertex_descriptor, mock_value> ni{mock_vertex_descriptor{70}, mock_value{400.0}};
    REQUIRE(ni.vertex.id == 70);
    REQUIRE(ni.value.data == 400.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
  }

  SECTION("VId=void, Sourced=false, VV=void; V present") {
    neighbor_data<void, false, mock_vertex_descriptor, void> ni{mock_vertex_descriptor{80}};
    REQUIRE(ni.vertex.id == 80);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("VId=void, Sourced=false, V=void; VV present") {
    neighbor_data<void, false, void, mock_value> ni{mock_value{500.0}};
    REQUIRE(ni.value.data == 500.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
  }

  SECTION("VId=void, Sourced=false; V=void, VV=void (empty)") {
    neighbor_data<void, false, void, void> ni{};
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }
}

TEST_CASE("neighbor_data: structured bindings work correctly", "[neighbor_data]") {
  SECTION("Sourced=true, all four members") {
    neighbor_data<int, true, mock_vertex_descriptor, mock_value> ni{1, 2, mock_vertex_descriptor{10}, mock_value{42.0}};
    auto [sid, tid, t, val] = ni;
    REQUIRE(sid == 1);
    REQUIRE(tid == 2);
    REQUIRE(t.id == 10);
    REQUIRE(val.data == 42.0);
  }

  SECTION("Sourced=false, three members") {
    neighbor_data<int, false, mock_vertex_descriptor, mock_value> ni{5, mock_vertex_descriptor{30}, mock_value{123.4}};
    auto [tid, t, val] = ni;
    REQUIRE(tid == 5);
    REQUIRE(t.id == 30);
    REQUIRE(val.data == 123.4);
  }

  SECTION("Three members: source_id, target_id and target") {
    neighbor_data<int, true, mock_vertex_descriptor, void> ni{2, 3, mock_vertex_descriptor{20}};
    auto [sid, tid, t] = ni;
    REQUIRE(sid == 2);
    REQUIRE(tid == 3);
    REQUIRE(t.id == 20);
  }

  SECTION("Two members: target_id and value") {
    neighbor_data<int, false, void, mock_value> ni{7, mock_value{77.7}};
    auto [tid, val] = ni;
    REQUIRE(tid == 7);
    REQUIRE(val.data == 77.7);
  }

  SECTION("Two members: source_id and target_id only") {
    neighbor_data<int, true, void, void> ni{4, 5};
    auto [sid, tid] = ni;
    REQUIRE(sid == 4);
    REQUIRE(tid == 5);
  }

  SECTION("Primary pattern: vertex and value (descriptor-based)") {
    neighbor_data<void, false, mock_vertex_descriptor, mock_value> ni{mock_vertex_descriptor{70}, mock_value{400.0}};
    auto [v, val] = ni;
    REQUIRE(v.id == 70);
    REQUIRE(val.data == 400.0);
  }

  SECTION("Descriptor-based: vertex only") {
    neighbor_data<void, true, mock_vertex_descriptor, void> ni{mock_vertex_descriptor{60}};
    auto [v] = ni;
    REQUIRE(v.id == 60);
  }

  SECTION("Descriptor-based: value only") {
    neighbor_data<void, false, void, mock_value> ni{mock_value{500.0}};
    auto [val] = ni;
    REQUIRE(val.data == 500.0);
  }
}

TEST_CASE("neighbor_data: sizeof verifies physical absence of void members", "[neighbor_data]") {
  SECTION("Full struct vs VId=void reduces size") {
    using full_t  = neighbor_data<int, true, mock_vertex_descriptor, mock_value>;
    using no_id_t = neighbor_data<void, true, mock_vertex_descriptor, mock_value>;

    // No source_id/target_id members should be smaller or equal (padding may prevent strict reduction)
    REQUIRE(sizeof(no_id_t) <= sizeof(full_t));
    // Should be at most the size of the two members plus padding
    REQUIRE(sizeof(no_id_t) <= sizeof(mock_vertex_descriptor) + sizeof(mock_value) + 2 * sizeof(int));
  }

  SECTION("IDs only struct (Sourced=true)") {
    using ids_only_t = neighbor_data<int, true, void, void>;
    REQUIRE(sizeof(ids_only_t) == 2 * sizeof(int)); // source_id + target_id
  }

  SECTION("target_id only struct (Sourced=false)") {
    using id_only_t = neighbor_data<size_t, false, void, void>;
    REQUIRE(sizeof(id_only_t) == sizeof(size_t)); // Only target_id
  }

  SECTION("Empty structs") {
    using empty_sourced_t   = neighbor_data<void, true, void, void>;
    using empty_unsourced_t = neighbor_data<void, false, void, void>;

    // Empty struct has size 1 in C++ (must be distinct)
    REQUIRE(sizeof(empty_sourced_t) >= 1);
    REQUIRE(sizeof(empty_unsourced_t) >= 1);
  }
}

TEST_CASE("neighbor_data: Sourced parameter affects member presence", "[neighbor_data]") {
  SECTION("Sourced=true has source_id and target_id") {
    neighbor_data<int, true, void, void> ni{42, 99};
    REQUIRE(ni.source_id == 42);
    REQUIRE(ni.target_id == 99);
    STATIC_REQUIRE(std::is_same_v<decltype(ni.source_id), int>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni.target_id), int>);
  }

  SECTION("Sourced=false has only target_id") {
    neighbor_data<int, false, void, void> ni{99};
    REQUIRE(ni.target_id == 99);
    STATIC_REQUIRE(std::is_same_v<decltype(ni.target_id), int>);
  }
}

TEST_CASE("neighbor_data: copyable and movable", "[neighbor_data]") {
  SECTION("Copy construction - Sourced=true") {
    neighbor_data<int, true, mock_vertex_descriptor, mock_value> ni1{1, 2, mock_vertex_descriptor{10},
                                                                     mock_value{42.0}};
    neighbor_data<int, true, mock_vertex_descriptor, mock_value> ni2 = ni1;
    REQUIRE(ni2.source_id == ni1.source_id);
    REQUIRE(ni2.target_id == ni1.target_id);
    REQUIRE(ni2.target.id == ni1.target.id);
    REQUIRE(ni2.value.data == ni1.value.data);
  }

  SECTION("Move construction - Sourced=false") {
    neighbor_data<int, false, mock_vertex_descriptor, mock_value> ni1{5, mock_vertex_descriptor{30}, mock_value{123.4}};
    neighbor_data<int, false, mock_vertex_descriptor, mock_value> ni2 = std::move(ni1);
    REQUIRE(ni2.target_id == 5);
    REQUIRE(ni2.target.id == 30);
    REQUIRE(ni2.value.data == 123.4);
  }
}

TEST_CASE("neighbor_data: descriptor-based pattern primary use case", "[neighbor_data]") {
  SECTION("Primary pattern: neighbor_data<void, false, vertex_descriptor, VV>") {
    // This is THE primary pattern for neighbor views as per Section 8.3 of view_strategy
    neighbor_data<void, false, mock_vertex_descriptor, double> ni{mock_vertex_descriptor{100}, 3.14};

    auto [v, val] = ni;
    REQUIRE(v.id == 100);
    REQUIRE(val == 3.14);

    // Verify no id members present
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::value_type, double>);
  }

  SECTION("Descriptor without value function") {
    neighbor_data<void, false, mock_vertex_descriptor, void> ni{mock_vertex_descriptor{200}};

    auto [v] = ni;
    REQUIRE(v.id == 200);

    // Verify only target member present
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }

  SECTION("Sourced pattern: neighbor_data<void, true, vertex_descriptor, VV>") {
    // Used when iterating from known source
    neighbor_data<void, true, mock_vertex_descriptor, std::string> ni{mock_vertex_descriptor{300},
                                                                      std::string("neighbor_data")};

    auto [v, val] = ni;
    REQUIRE(v.id == 300);
    REQUIRE(val == "neighbor_data");

    // Verify no ID members present
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::target_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::value_type, std::string>);
  }
}

TEST_CASE("neighbor_data: external data pattern use case", "[neighbor_data]") {
  SECTION("Sourced external data: source_id, target_id and value") {
    neighbor_data<size_t, true, void, double> ni{100, 200, 12.34};

    auto [sid, tid, val] = ni;
    REQUIRE(sid == 100);
    REQUIRE(tid == 200);
    REQUIRE(val == 12.34);

    // Verify source_id, target_id and value present, target absent
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::source_id_type, size_t>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::target_id_type, size_t>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::value_type, double>);
  }

  SECTION("Unsourced external data: target_id and value") {
    neighbor_data<int, false, void, std::string> ni{42, std::string("data")};

    auto [tid, val] = ni;
    REQUIRE(tid == 42);
    REQUIRE(val == "data");

    // Verify target_id and value present, source_id and target absent
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::target_id_type, int>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::value_type, std::string>);
  }

  SECTION("ID with target descriptor (external construction)") {
    neighbor_data<size_t, false, mock_vertex_descriptor, void> ni{999, mock_vertex_descriptor{400}};

    auto [tid, tgt] = ni;
    REQUIRE(tid == 999);
    REQUIRE(tgt.id == 400);

    // Both target_id and target present
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::source_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::target_id_type, size_t>);
    STATIC_REQUIRE(std::is_same_v<decltype(ni)::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_void_v<decltype(ni)::value_type>);
  }
}

TEST_CASE("neighbor_data: type traits are correct", "[neighbor_data]") {
  SECTION("All type aliases match - Sourced=true") {
    using ni_t = neighbor_data<int, true, mock_vertex_descriptor, mock_value>;
    STATIC_REQUIRE(std::is_same_v<ni_t::source_id_type, int>);
    STATIC_REQUIRE(std::is_same_v<ni_t::target_id_type, int>);
    STATIC_REQUIRE(std::is_same_v<ni_t::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_same_v<ni_t::value_type, mock_value>);
  }

  SECTION("All type aliases match - Sourced=false") {
    using ni_t = neighbor_data<size_t, false, mock_vertex_descriptor, mock_value>;
    STATIC_REQUIRE(std::is_void_v<ni_t::source_id_type>);
    STATIC_REQUIRE(std::is_same_v<ni_t::target_id_type, size_t>);
    STATIC_REQUIRE(std::is_same_v<ni_t::vertex_type, mock_vertex_descriptor>);
    STATIC_REQUIRE(std::is_same_v<ni_t::value_type, mock_value>);
  }

  SECTION("Void type aliases when void") {
    using ni_t = neighbor_data<void, true, void, mock_value>;
    STATIC_REQUIRE(std::is_void_v<ni_t::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<ni_t::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<ni_t::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<ni_t::value_type, mock_value>);
  }
}

TEST_CASE("neighbor_data: copyable_neighbor_t alias works", "[neighbor_data]") {
  SECTION("copyable_neighbor_t alias (Sourced=true)") {
    using alias_t    = copyable_neighbor_t<int, double>;
    using explicit_t = neighbor_data<int, true, void, double>;

    STATIC_REQUIRE(std::is_same_v<alias_t, explicit_t>);
  }

  SECTION("Alias used for sourced external data") {
    copyable_neighbor_t<int, double> cn{99, 100, 3.14};
    auto [sid, tid, val] = cn;
    REQUIRE(sid == 99);
    REQUIRE(tid == 100);
    REQUIRE(val == 3.14);
  }
}

TEST_CASE("neighbor_data: relationship to view_strategy.md Section 8.3", "[neighbor_data]") {
  SECTION("Section 8.3 specifies: neighbor_data<void, false, vertex_descriptor, VV>") {
    // Verify this is the exact pattern described in the strategy document
    using strategy_pattern = neighbor_data<void, false, mock_vertex_descriptor, double>;

    strategy_pattern ni{mock_vertex_descriptor{42}, 3.14159};

    // Should yield {vertex descriptor, value} as per Section 8.3
    auto [v, val] = ni;
    REQUIRE(v.id == 42);
    REQUIRE(val == 3.14159);

    // The descriptor contains the vertex ID, so VId=void
    STATIC_REQUIRE(std::is_void_v<strategy_pattern::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<strategy_pattern::target_id_type>);

    // Must have target member (the descriptor)
    STATIC_REQUIRE(!std::is_void_v<strategy_pattern::vertex_type>);

    // May or may not have value (VV can be void if no value function)
    // In this case we have a value
    STATIC_REQUIRE(!std::is_void_v<strategy_pattern::value_type>);
  }

  SECTION("Without value function: neighbor_data<void, false, vertex_descriptor, void>") {
    using no_value_pattern = neighbor_data<void, false, mock_vertex_descriptor, void>;

    no_value_pattern ni{mock_vertex_descriptor{99}};

    // Should yield {vertex descriptor} only
    auto [v] = ni;
    REQUIRE(v.id == 99);

    STATIC_REQUIRE(std::is_void_v<no_value_pattern::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<no_value_pattern::target_id_type>);
    STATIC_REQUIRE(!std::is_void_v<no_value_pattern::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<no_value_pattern::value_type>);
  }
}
