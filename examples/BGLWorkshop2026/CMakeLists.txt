# BGL Workshop 2026 examples

set(BGLWS_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/graphs)

add_executable(bglws_bacon bacon_gv3.cpp)
target_link_libraries(bglws_bacon PRIVATE graph3)
target_include_directories(bglws_bacon PRIVATE ${BGLWS_INCLUDE_DIRS})
target_compile_features(bglws_bacon PRIVATE cxx_std_20)
target_compile_definitions(bglws_bacon PRIVATE
    BGLWS_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/output")

add_executable(bglws_france_routes france_routes_gv3.cpp)
target_link_libraries(bglws_france_routes PRIVATE graph3)
target_include_directories(bglws_france_routes PRIVATE ${BGLWS_INCLUDE_DIRS})
target_compile_features(bglws_france_routes PRIVATE cxx_std_20)
target_compile_definitions(bglws_france_routes PRIVATE
    BGLWS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/graphs"
    BGLWS_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/output")

# BGL Bacon example — requires Boost headers (header-only, no linking needed).
# Resolution order: BGL_INCLUDE_DIR cache var → $BGL_INCLUDE_DIR env → $BOOST_ROOT env → ~/dev_graph/boost.
if(NOT DEFINED _bglws_bgl_inc)
    set(_bglws_bgl_inc "")
    foreach(_c "${BGL_INCLUDE_DIR}" "$ENV{BGL_INCLUDE_DIR}" "$ENV{BOOST_ROOT}"
                "$ENV{HOME}/dev_graph/boost" "/usr/local/include" "/usr/include")
        if(_c AND EXISTS "${_c}/boost/graph/adjacency_list.hpp")
            set(_bglws_bgl_inc "${_c}")
            break()
        endif()
    endforeach()
endif()

if(_bglws_bgl_inc)
    message(STATUS "BGLWorkshop2026: BGL headers at ${_bglws_bgl_inc}")
    add_executable(bglws_bacon_bgl bacon_bgl.cpp)
    target_link_libraries(bglws_bacon_bgl PRIVATE graph3)
    target_include_directories(bglws_bacon_bgl PRIVATE ${BGLWS_INCLUDE_DIRS} ${_bglws_bgl_inc})
    target_compile_features(bglws_bacon_bgl PRIVATE cxx_std_20)
    target_compile_definitions(bglws_bacon_bgl PRIVATE
        BGLWS_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/output")

    add_executable(bglws_france_routes_bgl france_routes_bgl.cpp)
    target_link_libraries(bglws_france_routes_bgl PRIVATE graph3)
    target_include_directories(bglws_france_routes_bgl PRIVATE ${BGLWS_INCLUDE_DIRS} ${_bglws_bgl_inc})
    target_compile_features(bglws_france_routes_bgl PRIVATE cxx_std_20)
    target_compile_definitions(bglws_france_routes_bgl PRIVATE
        BGLWS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/graphs"
        BGLWS_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/output")
else()
    message(STATUS "BGLWorkshop2026: Boost not found — skipping bglws_bacon_bgl (set BGL_INCLUDE_DIR)")
endif()
