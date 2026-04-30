# CppCon 2021 examples — refactored from graph-v2 to graph-v3

set(CPPCON21_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/graphs)

add_executable(cppcon21_graphs  examples/graphs.cpp)
target_link_libraries(cppcon21_graphs  PRIVATE graph3)
target_include_directories(cppcon21_graphs  PRIVATE ${CPPCON21_INCLUDE_DIRS})

add_executable(cppcon21_bacon   examples/bacon.cpp)
target_link_libraries(cppcon21_bacon   PRIVATE graph3)
target_include_directories(cppcon21_bacon   PRIVATE ${CPPCON21_INCLUDE_DIRS})

add_executable(cppcon21_ospf    examples/ospf.cpp)
target_link_libraries(cppcon21_ospf    PRIVATE graph3)
target_include_directories(cppcon21_ospf    PRIVATE ${CPPCON21_INCLUDE_DIRS})

add_executable(cppcon21_imdb    examples/imdb.cpp)
target_link_libraries(cppcon21_imdb    PRIVATE graph3)
target_include_directories(cppcon21_imdb    PRIVATE ${CPPCON21_INCLUDE_DIRS})
