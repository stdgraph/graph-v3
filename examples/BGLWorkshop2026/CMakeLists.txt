# BGL Workshop 2026 examples

set(BGLWS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/graphs)

add_executable(bglws_bacon bacon.cpp)
target_link_libraries(bglws_bacon PRIVATE graph3)
target_include_directories(bglws_bacon PRIVATE ${BGLWS_INCLUDE_DIRS})
target_compile_features(bglws_bacon PRIVATE cxx_std_20)
target_compile_definitions(bglws_bacon PRIVATE
    BGLWS_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/output")
