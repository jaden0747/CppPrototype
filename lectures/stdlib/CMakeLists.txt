# ============================================================================
# Stdlib Lectures — C++ Standard Library Mastery Series (01–13)
# ============================================================================
# Each lecture has a demo and exercises target.
# Requires C++20.
# ============================================================================

function(add_stdlib_lecture NUM NAME)
    set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/${NUM}_${NAME})

    if(EXISTS ${DIR}/demo.cpp)
        add_executable(stdlib${NUM}_demo ${DIR}/demo.cpp)
        set_target_properties(stdlib${NUM}_demo PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
    endif()

    if(EXISTS ${DIR}/exercises.cpp)
        add_executable(stdlib${NUM}_exercises ${DIR}/exercises.cpp)
        set_target_properties(stdlib${NUM}_exercises PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
    endif()
endfunction()

add_stdlib_lecture("01" "sequence_containers")
add_stdlib_lecture("02" "associative_containers")
add_stdlib_lecture("03" "unordered_and_adaptors")
add_stdlib_lecture("04" "iterators")
add_stdlib_lecture("05" "ranges")
add_stdlib_lecture("06" "non_modifying_algorithms")
add_stdlib_lecture("07" "modifying_sorting_algorithms")
add_stdlib_lecture("08" "numeric_set_algorithms")
add_stdlib_lecture("09" "strings_text")
add_stdlib_lecture("10" "memory_management")
add_stdlib_lecture("11" "utilities_types")
add_stdlib_lecture("12" "io_filesystem_chrono")
add_stdlib_lecture("13" "concurrency")
