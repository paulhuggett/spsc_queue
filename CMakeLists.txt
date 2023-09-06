cmake_minimum_required (VERSION 3.10)
project (spsc_queue CXX)

option (COVERAGE_ENABLED "Capture coverage data")

function (configure_target target)
    set_target_properties (${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED Yes
        CXX_EXTENSIONS No
    )

    # Tweak options for each compiler.
    if (MSVC)
        # Enable more warnings.
        target_compile_options (${target} PRIVATE /W4)
        if (COVERAGE_ENABLED)
            message (WARNING "COVERAGE_ENABLED is not yet implemented for MSVC")
        endif (COVERAGE_ENABLED)
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        # Enable more warnings.
        target_compile_options (${target} PRIVATE
            -Wall
            -Wextra
            -pedantic
        )
        if (COVERAGE_ENABLED)
            target_compile_options (${target} PRIVATE -fprofile-arcs -ftest-coverage)
            target_link_libraries (${target} PUBLIC --coverage)
        endif (COVERAGE_ENABLED)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Enable more warnings and request libc++
        target_compile_options (${target} PRIVATE
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-padded
        )
        if (COVERAGE_ENABLED)
            target_compile_options (${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
            target_compile_options (${target} PRIVATE -fno-inline-functions)
            target_link_libraries (${target} PUBLIC -fprofile-instr-generate -fcoverage-mapping)
        endif (COVERAGE_ENABLED)
    endif ()

endfunction()


find_package(Threads REQUIRED)
add_executable (spsc_queue
    main.cpp
    spsc_queue.hpp
)
target_link_libraries (spsc_queue ${CMAKE_THREAD_LIBS_INIT})
configure_target (spsc_queue)
