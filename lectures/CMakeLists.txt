# ============================================================================
# Lectures — C++ Mastery Series (01–13)
# ============================================================================
# Each lecture has a demo and exercises target.
# Compile with C++20 minimum (C++23 for lecture 13).
# ============================================================================

set(LECTURE_CXX_STANDARD 20)

# Helper function to add a lecture target
function(add_lecture NUM NAME)
    set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/${NUM}_${NAME})

    if(EXISTS ${DIR}/demo.cpp)
        add_executable(lecture${NUM}_demo ${DIR}/demo.cpp)
        set_target_properties(lecture${NUM}_demo PROPERTIES
            CXX_STANDARD ${LECTURE_CXX_STANDARD}
            CXX_STANDARD_REQUIRED ON)
        # Filesystem linking for lecture 08
        if(NUM STREQUAL "08")
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
                target_link_libraries(lecture${NUM}_demo PRIVATE stdc++fs)
            endif()
        endif()
    endif()

    if(EXISTS ${DIR}/exercises.cpp)
        add_executable(lecture${NUM}_exercises ${DIR}/exercises.cpp)
        set_target_properties(lecture${NUM}_exercises PROPERTIES
            CXX_STANDARD ${LECTURE_CXX_STANDARD}
            CXX_STANDARD_REQUIRED ON)
        if(NUM STREQUAL "08")
            if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
                target_link_libraries(lecture${NUM}_exercises PRIVATE stdc++fs)
            endif()
        endif()
    endif()
endfunction()

add_lecture("01" "daily_vocabulary")
add_lecture("02" "functions_lambdas")
add_lecture("03" "memory_ownership")
add_lecture("04" "compile_time_templates")
add_lecture("05" "cpp14_refinements")
add_lecture("06" "cpp17_ergonomics")
add_lecture("07" "cpp17_vocabulary_types")
add_lecture("08" "cpp17_templates_filesystem")
add_lecture("09" "cpp20_concepts")
add_lecture("10" "cpp20_ranges")
add_lecture("11" "cpp20_coroutines_modules")
add_lecture("12" "cpp20_utilities")

# Lecture 13 needs C++23
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/13_cpp23_additions/demo.cpp)
    add_executable(lecture13_demo ${CMAKE_CURRENT_SOURCE_DIR}/13_cpp23_additions/demo.cpp)
    set_target_properties(lecture13_demo PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/13_cpp23_additions/exercises.cpp)
    add_executable(lecture13_exercises ${CMAKE_CURRENT_SOURCE_DIR}/13_cpp23_additions/exercises.cpp)
    set_target_properties(lecture13_exercises PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)
endif()
