# ── Filtered graph adaptor tests (no external dependencies) ──────────────────

add_executable(graph3_filtered_graph_tests
  test_filtered_graph.cpp
)

target_link_libraries(graph3_filtered_graph_tests
  PRIVATE graph3 Catch2::Catch2WithMain)

add_test(NAME graph3_filtered_graph_tests COMMAND graph3_filtered_graph_tests)

# ── BGL adaptor tests ────────────────────────────────────────────────────────
# Requires Boost headers. Reuse BGL_INCLUDE_DIR resolution from benchmark/.
option(TEST_BGL_ADAPTOR "Build BGL adaptor tests (requires Boost headers)" OFF)

if(TEST_BGL_ADAPTOR)
  # Resolve Boost include directory (same logic as benchmark/algorithms/)
  if(NOT BGL_INCLUDE_DIR)
    if(DEFINED ENV{BGL_INCLUDE_DIR})
      set(BGL_INCLUDE_DIR "$ENV{BGL_INCLUDE_DIR}")
    elseif(DEFINED ENV{BOOST_ROOT})
      set(BGL_INCLUDE_DIR "$ENV{BOOST_ROOT}")
    else()
      # Platform defaults
      foreach(_path "$ENV{HOME}/dev_graph/boost" "/usr/local/include" "/usr/include")
        if(EXISTS "${_path}/boost/graph/adjacency_list.hpp")
          set(BGL_INCLUDE_DIR "${_path}")
          break()
        endif()
      endforeach()
    endif()
  endif()

  if(NOT BGL_INCLUDE_DIR OR NOT EXISTS "${BGL_INCLUDE_DIR}/boost/graph/adjacency_list.hpp")
    message(WARNING "BGL headers not found — skipping BGL adaptor tests. "
                    "Set BGL_INCLUDE_DIR or BOOST_ROOT.")
    return()
  endif()

  message(STATUS "BGL adaptor tests: using Boost headers from ${BGL_INCLUDE_DIR}")

  add_executable(graph3_bgl_adaptor_tests
    test_bgl_edge_iterator.cpp
    test_bgl_cpo.cpp
    test_bgl_concepts.cpp
    test_bgl_views.cpp
    test_bgl_property_bridge.cpp
    test_bgl_dijkstra.cpp
    test_bgl_bidirectional.cpp
    test_bgl_csr.cpp
  )

  target_link_libraries(graph3_bgl_adaptor_tests
    PRIVATE graph3 Catch2::Catch2WithMain)

  target_include_directories(graph3_bgl_adaptor_tests
    PRIVATE ${BGL_INCLUDE_DIR})

  # Keep the test runner simple: run the whole adaptor suite as one CTest test.
  # The phases in this plan are expected to remain short-running, so per-case
  # Catch discovery is not required.
  add_test(NAME graph3_bgl_adaptor_tests COMMAND graph3_bgl_adaptor_tests)
endif()
