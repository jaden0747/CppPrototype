# ============================================================================
# Template Lectures — C++ Templates Mastery Series (01–13)
# ============================================================================
# Each lecture has a demo and exercises target.
# Requires C++20.
# ============================================================================

function(add_template_lecture NUM NAME)
    set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/${NUM}_${NAME})

    if(EXISTS ${DIR}/demo.cpp)
        add_executable(template${NUM}_demo ${DIR}/demo.cpp)
        set_target_properties(template${NUM}_demo PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
    endif()

    if(EXISTS ${DIR}/exercises.cpp)
        add_executable(template${NUM}_exercises ${DIR}/exercises.cpp)
        set_target_properties(template${NUM}_exercises PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
    endif()
endfunction()

add_template_lecture("01" "function_class_templates")
add_template_lecture("02" "nontype_specialization")
add_template_lecture("03" "variadic_templates")
add_template_lecture("04" "sfinae_type_traits")
add_template_lecture("05" "aliases_variables_lambdas")
add_template_lecture("06" "concepts_basics")
add_template_lecture("07" "standard_concepts")
add_template_lecture("08" "crtp_static_polymorphism")
add_template_lecture("09" "template_metaprogramming")
add_template_lecture("10" "expression_templates")
add_template_lecture("11" "perfect_forwarding")
add_template_lecture("12" "library_design")
add_template_lecture("13" "debugging_best_practices")
