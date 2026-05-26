# Comprehensive compiler warning configuration
function(set_project_warnings target_name)
    option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

    set(MSVC_WARNINGS
        /W4     # High warning level
        /w14242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits'
        /w14263 # 'function': member function does not override any base class virtual member
        /w14265 # 'classname': class has virtual functions, but destructor is not virtual
        /w14287 # 'operator': unsigned/negative constant mismatch
        /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop
        /w14296 # 'operator': expression is always 'boolean_value'
        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545 # expression before comma evaluates to a function which is missing an argument list
        /w14546 # function call before comma missing argument list
        /w14547 # 'operator': operator before comma has no effect
        /w14549 # 'operator': operator before comma has no effect
        /w14555 # expression has no effect; expected expression with side-effect
        /w14619 # pragma warning: there is no warning number 'number'
        /w14640 # Enable warning on thread un-safe static member initialization
        /w14826 # Conversion from 'type1' to 'type2' is sign-extended
        /w14905 # wide string literal cast to 'LPSTR'
        /w14906 # string literal cast to 'LPWSTR'
        /w14928 # illegal copy-initialization; more than one user-defined conversion
        /permissive- # standards conformance mode
    )

    # The -Wno-* options are used to suppress specific warnings that are either not relevant or too noisy for this project.
    # Further adjustments may be necessary based on the specific codebase and compiler versions used.
    set(CLANG_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wshadow
        #-Wconversion
        #-Wsign-conversion
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-mismatched-tags
        -Wno-ignored-qualifiers
        -Wno-deprecated-declarations
        -Wno-unqualified-std-cast-call
        -Wno-unused-lambda-capture
        -Wno-unused-local-typedef
        -Wno-self-assign-overloaded
        -Wno-unneeded-internal-declaration
        -Wno-unused-but-set-variable
        -Wno-sign-compare
        -Wno-old-style-cast
        -Wno-double-promotion
        -Wno-conversion
        -Wno-sign-conversion
    )

    set(GCC_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wshadow
        #-Wconversion
        #-Wsign-conversion
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wlogical-op
        -Wuseless-cast
        -Wno-useless-cast
        -Wno-old-style-cast
        -Wno-unused-local-typedefs
        -Wno-unused-but-set-variable
        -Wno-sign-compare
        -Wno-comment
        -Wno-null-dereference
        -Wno-deprecated-declarations
        -Wno-conversion
        -Wno-sign-conversion
    )

    if(WARNINGS_AS_ERRORS)
        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
        set(GCC_WARNINGS ${GCC_WARNINGS} -Werror)
        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
    endif()

    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(WARNING "No compiler warnings set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
    endif()

    target_compile_options(${target_name} INTERFACE ${PROJECT_WARNINGS})
endfunction()
