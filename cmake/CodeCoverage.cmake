# Code coverage configuration
function(enable_coverage target_name)
    option(ENABLE_COVERAGE "Enable code coverage" OFF)

    if(ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target_name} INTERFACE
                --coverage
                -O0
                -g
            )
            target_link_options(${target_name} INTERFACE --coverage)
            
            message(STATUS "Code coverage enabled for target: ${target_name}")
            
            # Add custom target for generating coverage report
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
                find_program(GCOV gcov)
                find_program(LCOV lcov)
                find_program(GENHTML genhtml)
                
                if(LCOV AND GENHTML)
                    add_custom_target(coverage
                        COMMAND ${LCOV} --directory . --zerocounters
                        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                        COMMAND ${LCOV} --directory . --capture --output-file coverage.info --ignore-errors mismatch
                        COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info --ignore-errors mismatch
                        COMMAND ${GENHTML} coverage.info --output-directory coverage_html --ignore-errors mismatch
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                        COMMENT "Generating code coverage report"
                    )
                    message(STATUS "Coverage report can be generated with: make coverage")
                endif()
            elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                find_program(LLVM_COV llvm-cov)
                find_program(LLVM_PROFDATA llvm-profdata)
                
                if(LLVM_COV AND LLVM_PROFDATA)
                    message(STATUS "For coverage with Clang, run tests and then:")
                    message(STATUS "  llvm-profdata merge -sparse default.profraw -o default.profdata")
                    message(STATUS "  llvm-cov show <test-executable> -instr-profile=default.profdata")
                endif()
            endif()
        else()
            message(WARNING "Code coverage is not supported for ${CMAKE_CXX_COMPILER_ID}")
        endif()
    endif()
endfunction()
