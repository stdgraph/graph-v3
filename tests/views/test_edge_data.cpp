#include <catch2/catch_test_macros.hpp>
#include <graph/graph_data.hpp>
#include <type_traits>

using namespace graph;

// Mock types for testing
struct mock_edge_descriptor {
  int src_id;
  int tgt_id;
};

struct mock_value {
  double weight;
};

TEST_CASE("edge_data: all 16 specializations compile", "[edge_data]") {
  SECTION("VId, Sourced=true, E, EV all present") {
    edge_data<int, true, mock_edge_descriptor, mock_value> ei{1, 2, mock_edge_descriptor{0, 1}, mock_value{10.5}};
    REQUIRE(ei.source_id == 1);
    REQUIRE(ei.target_id == 2);
    REQUIRE(ei.edge.src_id == 0);
    REQUIRE(ei.edge.tgt_id == 1);
    REQUIRE(ei.value.weight == 10.5);
  }

  SECTION("VId, Sourced=true, E present; EV=void") {
    edge_data<int, true, mock_edge_descriptor, void> ei{2, 3, mock_edge_descriptor{1, 2}};
    REQUIRE(ei.source_id == 2);
    REQUIRE(ei.target_id == 3);
    REQUIRE(ei.edge.src_id == 1);
    REQUIRE(ei.edge.tgt_id == 2);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId, Sourced=true, EV present; E=void") {
    edge_data<int, true, void, mock_value> ei{3, 4, mock_value{20.0}};
    REQUIRE(ei.source_id == 3);
    REQUIRE(ei.target_id == 4);
    REQUIRE(ei.value.weight == 20.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
  }

  SECTION("VId, Sourced=true; E=void, EV=void") {
    edge_data<int, true, void, void> ei{4, 5};
    REQUIRE(ei.source_id == 4);
    REQUIRE(ei.target_id == 5);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId, Sourced=false, E, EV all present") {
    edge_data<int, false, mock_edge_descriptor, mock_value> ei{5, mock_edge_descriptor{2, 3}, mock_value{15.5}};
    REQUIRE(ei.target_id == 5);
    REQUIRE(ei.edge.src_id == 2);
    REQUIRE(ei.edge.tgt_id == 3);
    REQUIRE(ei.value.weight == 15.5);
  }

  SECTION("VId, Sourced=false, E present; EV=void") {
    edge_data<int, false, mock_edge_descriptor, void> ei{6, mock_edge_descriptor{3, 4}};
    REQUIRE(ei.target_id == 6);
    REQUIRE(ei.edge.src_id == 3);
    REQUIRE(ei.edge.tgt_id == 4);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId, Sourced=false, EV present; E=void") {
    edge_data<int, false, void, mock_value> ei{7, mock_value{25.0}};
    REQUIRE(ei.target_id == 7);
    REQUIRE(ei.value.weight == 25.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
  }

  SECTION("VId, Sourced=false; E=void, EV=void") {
    edge_data<int, false, void, void> ei{8};
    REQUIRE(ei.target_id == 8);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId=void, Sourced=true; E, EV present (descriptor-based)") {
    edge_data<void, true, mock_edge_descriptor, mock_value> ei{mock_edge_descriptor{4, 5}, mock_value{30.0}};
    REQUIRE(ei.edge.src_id == 4);
    REQUIRE(ei.edge.tgt_id == 5);
    REQUIRE(ei.value.weight == 30.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
  }

  SECTION("VId=void, Sourced=true, EV=void; E present") {
    edge_data<void, true, mock_edge_descriptor, void> ei{mock_edge_descriptor{5, 6}};
    REQUIRE(ei.edge.src_id == 5);
    REQUIRE(ei.edge.tgt_id == 6);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId=void, Sourced=true, E=void; EV present") {
    edge_data<void, true, void, mock_value> ei{mock_value{35.0}};
    REQUIRE(ei.value.weight == 35.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
  }

  SECTION("VId=void, Sourced=true; E=void, EV=void (empty)") {
    edge_data<void, true, void, void> ei{};
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId=void, Sourced=false; E, EV present (descriptor-based)") {
    edge_data<void, false, mock_edge_descriptor, mock_value> ei{mock_edge_descriptor{6, 7}, mock_value{40.0}};
    REQUIRE(ei.edge.src_id == 6);
    REQUIRE(ei.edge.tgt_id == 7);
    REQUIRE(ei.value.weight == 40.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
  }

  SECTION("VId=void, Sourced=false, EV=void; E present") {
    edge_data<void, false, mock_edge_descriptor, void> ei{mock_edge_descriptor{7, 8}};
    REQUIRE(ei.edge.src_id == 7);
    REQUIRE(ei.edge.tgt_id == 8);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }

  SECTION("VId=void, Sourced=false, E=void; EV present") {
    edge_data<void, false, void, mock_value> ei{mock_value{45.0}};
    REQUIRE(ei.value.weight == 45.0);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
  }

  SECTION("VId=void, Sourced=false; E=void, EV=void (empty)") {
    edge_data<void, false, void, void> ei{};
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }
}

TEST_CASE("edge_data: structured bindings work correctly", "[edge_data]") {
  SECTION("Sourced=true, all four members") {
    edge_data<int, true, mock_edge_descriptor, mock_value> ei{1, 2, mock_edge_descriptor{0, 1}, mock_value{10.5}};
    auto [sid, tid, e, val] = ei;
    REQUIRE(sid == 1);
    REQUIRE(tid == 2);
    REQUIRE(e.src_id == 0);
    REQUIRE(e.tgt_id == 1);
    REQUIRE(val.weight == 10.5);
  }

  SECTION("Sourced=false, three members") {
    edge_data<int, false, mock_edge_descriptor, mock_value> ei{5, mock_edge_descriptor{2, 3}, mock_value{15.5}};
    auto [tid, e, val] = ei;
    REQUIRE(tid == 5);
    REQUIRE(e.src_id == 2);
    REQUIRE(e.tgt_id == 3);
    REQUIRE(val.weight == 15.5);
  }

  SECTION("Three members: source_id, target_id and edge") {
    edge_data<int, true, mock_edge_descriptor, void> ei{2, 3, mock_edge_descriptor{1, 2}};
    auto [sid, tid, e] = ei;
    REQUIRE(sid == 2);
    REQUIRE(tid == 3);
    REQUIRE(e.src_id == 1);
    REQUIRE(e.tgt_id == 2);
  }

  SECTION("Two members: target_id and value") {
    edge_data<int, false, void, mock_value> ei{7, mock_value{25.0}};
    auto [tid, val] = ei;
    REQUIRE(tid == 7);
    REQUIRE(val.weight == 25.0);
  }

  SECTION("Two members: source_id and target_id only") {
    edge_data<int, true, void, void> ei{4, 5};
    auto [sid, tid] = ei;
    REQUIRE(sid == 4);
    REQUIRE(tid == 5);
  }

  SECTION("Descriptor-based: edge and value") {
    edge_data<void, false, mock_edge_descriptor, mock_value> ei{mock_edge_descriptor{6, 7}, mock_value{40.0}};
    auto [e, val] = ei;
    REQUIRE(e.src_id == 6);
    REQUIRE(e.tgt_id == 7);
    REQUIRE(val.weight == 40.0);
  }

  SECTION("Descriptor-based: edge only") {
    edge_data<void, true, mock_edge_descriptor, void> ei{mock_edge_descriptor{5, 6}};
    auto [e] = ei;
    REQUIRE(e.src_id == 5);
    REQUIRE(e.tgt_id == 6);
  }

  SECTION("Descriptor-based: value only") {
    edge_data<void, false, void, mock_value> ei{mock_value{45.0}};
    auto [val] = ei;
    REQUIRE(val.weight == 45.0);
  }
}

TEST_CASE("edge_data: sizeof verifies physical absence of void members", "[edge_data]") {
  SECTION("Full struct vs VId=void reduces size") {
    using full_t  = edge_data<int, true, mock_edge_descriptor, mock_value>;
    using no_id_t = edge_data<void, true, mock_edge_descriptor, mock_value>;

    // No source_id/target_id members should be smaller or equal (padding may prevent strict reduction)
    REQUIRE(sizeof(no_id_t) <= sizeof(full_t));
    // Should be at most the size of the two members plus padding
    REQUIRE(sizeof(no_id_t) <= sizeof(mock_edge_descriptor) + sizeof(mock_value) + 2 * sizeof(int));
  }

  SECTION("IDs only struct (Sourced=true)") {
    using ids_only_t = edge_data<int, true, void, void>;
    REQUIRE(sizeof(ids_only_t) == 2 * sizeof(int)); // source_id + target_id
  }

  SECTION("target_id only struct (Sourced=false)") {
    using id_only_t = edge_data<size_t, false, void, void>;
    REQUIRE(sizeof(id_only_t) == sizeof(size_t)); // Only target_id
  }

  SECTION("Empty structs") {
    using empty_sourced_t   = edge_data<void, true, void, void>;
    using empty_unsourced_t = edge_data<void, false, void, void>;

    // Empty struct has size 1 in C++ (must be distinct)
    REQUIRE(sizeof(empty_sourced_t) >= 1);
    REQUIRE(sizeof(empty_unsourced_t) >= 1);
  }
}

TEST_CASE("edge_data: Sourced parameter affects member presence", "[edge_data]") {
  SECTION("Sourced=true has source_id and target_id") {
    edge_data<int, true, void, void> ei{42, 99};
    REQUIRE(ei.source_id == 42);
    REQUIRE(ei.target_id == 99);
    STATIC_REQUIRE(std::is_same_v<decltype(ei.source_id), int>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei.target_id), int>);
  }

  SECTION("Sourced=false has only target_id") {
    edge_data<int, false, void, void> ei{99};
    REQUIRE(ei.target_id == 99);
    STATIC_REQUIRE(std::is_same_v<decltype(ei.target_id), int>);
  }
}

TEST_CASE("edge_data: copyable and movable", "[edge_data]") {
  SECTION("Copy construction - Sourced=true") {
    edge_data<int, true, mock_edge_descriptor, mock_value> ei1{1, 2, mock_edge_descriptor{0, 1}, mock_value{10.5}};
    edge_data<int, true, mock_edge_descriptor, mock_value> ei2 = ei1;
    REQUIRE(ei2.source_id == ei1.source_id);
    REQUIRE(ei2.target_id == ei1.target_id);
    REQUIRE(ei2.edge.src_id == ei1.edge.src_id);
    REQUIRE(ei2.value.weight == ei1.value.weight);
  }

  SECTION("Move construction - Sourced=false") {
    edge_data<int, false, mock_edge_descriptor, mock_value> ei1{5, mock_edge_descriptor{2, 3}, mock_value{15.5}};
    edge_data<int, false, mock_edge_descriptor, mock_value> ei2 = std::move(ei1);
    REQUIRE(ei2.target_id == 5);
    REQUIRE(ei2.edge.src_id == 2);
    REQUIRE(ei2.value.weight == 15.5);
  }
}

TEST_CASE("edge_data: descriptor-based pattern primary use cases", "[edge_data]") {
  SECTION("Incidence view pattern: edge_data<void, true, edge_descriptor, EV>") {
    // Primary pattern for incidence views (sourced iteration)
    edge_data<void, true, mock_edge_descriptor, double> ei{mock_edge_descriptor{10, 20}, 3.14};

    auto [e, val] = ei;
    REQUIRE(e.src_id == 10);
    REQUIRE(e.tgt_id == 20);
    REQUIRE(val == 3.14);

    // Verify no id members present
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::edge_type, mock_edge_descriptor>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::value_type, double>);
  }

  SECTION("Edgelist view pattern: edge_data<void, false, edge_descriptor, EV>") {
    // Primary pattern for edgelist views (unsourced iteration)
    edge_data<void, false, mock_edge_descriptor, std::string> ei{mock_edge_descriptor{5, 8}, std::string("road")};

    auto [e, val] = ei;
    REQUIRE(e.src_id == 5);
    REQUIRE(e.tgt_id == 8);
    REQUIRE(val == "road");

    // Verify no id members present
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::edge_type, mock_edge_descriptor>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::value_type, std::string>);
  }

  SECTION("Descriptor without value function") {
    edge_data<void, true, mock_edge_descriptor, void> ei{mock_edge_descriptor{15, 25}};

    auto [e] = ei;
    REQUIRE(e.src_id == 15);
    REQUIRE(e.tgt_id == 25);

    // Verify only edge member present
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::target_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::edge_type, mock_edge_descriptor>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::value_type>);
  }
}

TEST_CASE("edge_data: external data pattern use case", "[edge_data]") {
  SECTION("Sourced external data: source_id, target_id and value") {
    edge_data<size_t, true, void, double> ei{100, 200, 12.34};

    auto [sid, tid, val] = ei;
    REQUIRE(sid == 100);
    REQUIRE(tid == 200);
    REQUIRE(val == 12.34);

    // Verify source_id, target_id and value present, edge absent
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::source_id_type, size_t>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::target_id_type, size_t>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::value_type, double>);
  }

  SECTION("Unsourced external data: target_id and value") {
    edge_data<int, false, void, std::string> ei{42, std::string("highway")};

    auto [tid, val] = ei;
    REQUIRE(tid == 42);
    REQUIRE(val == "highway");

    // Verify target_id and value present, source_id and edge absent
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::source_id_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::target_id_type, int>);
    STATIC_REQUIRE(std::is_void_v<decltype(ei)::edge_type>);
    STATIC_REQUIRE(std::is_same_v<decltype(ei)::value_type, std::string>);
  }
}

TEST_CASE("edge_data: type traits are correct", "[edge_data]") {
  SECTION("All type aliases match - Sourced=true") {
    using ei_t = edge_data<int, true, mock_edge_descriptor, mock_value>;
    STATIC_REQUIRE(std::is_same_v<ei_t::source_id_type, int>);
    STATIC_REQUIRE(std::is_same_v<ei_t::target_id_type, int>);
    STATIC_REQUIRE(std::is_same_v<ei_t::edge_type, mock_edge_descriptor>);
    STATIC_REQUIRE(std::is_same_v<ei_t::value_type, mock_value>);
  }

  SECTION("All type aliases match - Sourced=false") {
    using ei_t = edge_data<size_t, false, mock_edge_descriptor, mock_value>;
    STATIC_REQUIRE(std::is_void_v<ei_t::source_id_type>);
    STATIC_REQUIRE(std::is_same_v<ei_t::target_id_type, size_t>);
    STATIC_REQUIRE(std::is_same_v<ei_t::edge_type, mock_edge_descriptor>);
    STATIC_REQUIRE(std::is_same_v<ei_t::value_type, mock_value>);
  }

  SECTION("Void type aliases when void") {
    using ei_t = edge_data<void, true, void, mock_value>;
    STATIC_REQUIRE(std::is_void_v<ei_t::source_id_type>);
    STATIC_REQUIRE(std::is_void_v<ei_t::target_id_type>);
    STATIC_REQUIRE(std::is_void_v<ei_t::edge_type>);
    STATIC_REQUIRE(std::is_same_v<ei_t::value_type, mock_value>);
  }
}

TEST_CASE("edge_data: copyable_edge_t alias works", "[edge_data]") {
  SECTION("copyable_edge_t alias (Sourced=true)") {
    using alias_t    = copyable_edge_t<int, double>;
    using explicit_t = edge_data<int, true, void, double>;

    STATIC_REQUIRE(std::is_same_v<alias_t, explicit_t>);
  }

  SECTION("Alias used for sourced external data") {
    copyable_edge_t<int, double> ce{99, 100, 3.14};
    auto [sid, tid, val] = ce;
    REQUIRE(sid == 99);
    REQUIRE(tid == 100);
    REQUIRE(val == 3.14);
  }
}
