# Examples will be added here
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE graph3)

add_executable(adapting_third_party_graph
    AdaptingThirdPartyGraph/adapting_a_third_party_graph.cpp)
target_link_libraries(adapting_third_party_graph PRIVATE graph3)

add_executable(dijkstra_example dijkstra_clrs_example.cpp)
target_link_libraries(dijkstra_example PRIVATE graph3)
target_include_directories(dijkstra_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(CppCon2021)

# BGL adaptor example (requires Boost headers)
option(BUILD_BGL_EXAMPLES "Build BGL adaptor examples (requires Boost headers)" OFF)

if(BUILD_BGL_EXAMPLES)
  if(NOT BGL_INCLUDE_DIR)
    if(DEFINED ENV{BGL_INCLUDE_DIR})
      set(BGL_INCLUDE_DIR "$ENV{BGL_INCLUDE_DIR}")
    elseif(DEFINED ENV{BOOST_ROOT})
      set(BGL_INCLUDE_DIR "$ENV{BOOST_ROOT}")
    else()
      foreach(_path "$ENV{HOME}/dev_graph/boost" "/usr/local/include" "/usr/include")
        if(EXISTS "${_path}/boost/graph/adjacency_list.hpp")
          set(BGL_INCLUDE_DIR "${_path}")
          break()
        endif()
      endforeach()
    endif()
  endif()

  if(BGL_INCLUDE_DIR AND EXISTS "${BGL_INCLUDE_DIR}/boost/graph/adjacency_list.hpp")
    message(STATUS "BGL examples: using Boost headers from ${BGL_INCLUDE_DIR}")

    add_executable(bgl_adaptor_example bgl_adaptor_example.cpp)
    target_link_libraries(bgl_adaptor_example PRIVATE graph3)
    target_include_directories(bgl_adaptor_example PRIVATE ${BGL_INCLUDE_DIR})
  else()
    message(WARNING "BGL headers not found — skipping BGL examples. Set BGL_INCLUDE_DIR or BOOST_ROOT.")
  endif()
endif()
