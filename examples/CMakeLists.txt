# Examples will be added here
add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE graph3)

add_executable(dijkstra_example dijkstra_clrs_example.cpp)
target_link_libraries(dijkstra_example PRIVATE graph3)
target_include_directories(dijkstra_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
