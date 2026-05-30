# CppCon 2021 examples — refactored from graph-v2 to graph-v3

set(CPPCON21_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/graphs)

set(CPPCON21_WARNING_SUPPRESSIONS)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND CPPCON21_WARNING_SUPPRESSIONS
             -Wno-old-style-cast
             -Wno-useless-cast)
elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    list(APPEND CPPCON21_WARNING_SUPPRESSIONS
             -Wno-old-style-cast)
endif()

add_executable(cppcon21_graphs  examples/graphs.cpp)
target_link_libraries(cppcon21_graphs  PRIVATE graph3)
target_include_directories(cppcon21_graphs  PRIVATE ${CPPCON21_INCLUDE_DIRS})
target_compile_options(cppcon21_graphs PRIVATE ${CPPCON21_WARNING_SUPPRESSIONS})

add_executable(cppcon21_bacon   examples/bacon.cpp)
target_link_libraries(cppcon21_bacon   PRIVATE graph3)
target_include_directories(cppcon21_bacon   PRIVATE ${CPPCON21_INCLUDE_DIRS})
target_compile_options(cppcon21_bacon PRIVATE ${CPPCON21_WARNING_SUPPRESSIONS})

add_executable(cppcon21_ospf    examples/ospf.cpp)
target_link_libraries(cppcon21_ospf    PRIVATE graph3)
target_include_directories(cppcon21_ospf    PRIVATE ${CPPCON21_INCLUDE_DIRS})
target_compile_options(cppcon21_ospf PRIVATE ${CPPCON21_WARNING_SUPPRESSIONS})

add_executable(cppcon21_imdb    examples/imdb.cpp)
target_link_libraries(cppcon21_imdb    PRIVATE graph3)
target_include_directories(cppcon21_imdb    PRIVATE ${CPPCON21_INCLUDE_DIRS})
target_compile_options(cppcon21_imdb PRIVATE ${CPPCON21_WARNING_SUPPRESSIONS})
