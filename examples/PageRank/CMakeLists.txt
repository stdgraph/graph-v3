add_library(pagerank_example INTERFACE)
target_link_libraries(pagerank_example INTERFACE graph3)
target_sources(pagerank_example INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/pagerank.hpp
)
