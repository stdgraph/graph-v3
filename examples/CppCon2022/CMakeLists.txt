# CppCon 2022 examples — refactored from graph-v2 to graph-v3

set(CPPCON22_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(cppcon22_germany_routes germany_routes_example.cpp)
target_link_libraries(cppcon22_germany_routes PRIVATE graph3)
target_include_directories(cppcon22_germany_routes PRIVATE ${CPPCON22_INCLUDE_DIRS})
