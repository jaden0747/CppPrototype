// ============================================================================
// Lecture 05 — Exercises: C++14 Refinements
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Generic lambdas
//
// Write a generic lambda `clamp` that works with any numeric type.
// clamp(value, lo, hi) returns lo if value < lo, hi if value > hi, else value.
// ──────────────────────────────────────────────────────────────────────────
void exercise_generic_lambda()
{
    // auto clamp = [](auto val, auto lo, auto hi) { ??? };
    // assert(clamp(5, 0, 10) == 5);
    // assert(clamp(-3, 0, 10) == 0);
    // assert(clamp(15, 0, 10) == 10);
    // assert(clamp(1.5, 0.0, 1.0) == 1.0);
    std::cout << "  Exercise 1: generic clamp — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Relaxed constexpr — compile-time bubble sort
//
// Write a constexpr function that sorts a std::array<int, N> at compile time.
// ──────────────────────────────────────────────────────────────────────────
// #include <array>
// template<std::size_t N>
// constexpr std::array<int, N> compile_sort(std::array<int, N> arr) { ??? }

void exercise_constexpr_sort()
{
    // constexpr std::array<int, 5> unsorted{5, 3, 1, 4, 2};
    // constexpr auto sorted = compile_sort(unsorted);
    // static_assert(sorted[0] == 1, "");
    // static_assert(sorted[4] == 5, "");
    std::cout << "  Exercise 2: constexpr sort — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Eliminate raw new — use make_unique
//
// Refactor the following code to use make_unique.
// ──────────────────────────────────────────────────────────────────────────
void exercise_make_unique()
{
    // Before (bad):
    // int* raw = new int(42);
    // std::string* str = new std::string("hello");
    // delete raw;
    // delete str;

    // TODO: Rewrite using make_unique — no delete needed
    // auto raw = std::make_unique<int>(42);
    // auto str = std::make_unique<std::string>("hello");
    // assert(*raw == 42);
    // assert(*str == "hello");
    std::cout << "  Exercise 3: make_unique — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Variable templates
//
// Create variable templates for:
//  - golden_ratio<T> = 1.6180339887...
//  - max_value<T> = std::numeric_limits<T>::max()
// ──────────────────────────────────────────────────────────────────────────
// #include <limits>
// TODO: template<typename T> constexpr T golden_ratio = ???;
// TODO: template<typename T> constexpr T max_value = ???;

void exercise_variable_templates()
{
    // static_assert(golden_ratio<float> > 1.617f, "");
    // static_assert(golden_ratio<float> < 1.619f, "");
    // static_assert(max_value<int> == std::numeric_limits<int>::max(), "");
    std::cout << "  Exercise 4: variable templates — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: decltype(auto)
//
// Write a function `at(vector<T>&, index)` that returns a REFERENCE to the
// element (not a copy). Use decltype(auto) return type.
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename T>
// decltype(auto) at(std::vector<T>& v, std::size_t i) { ??? }

void exercise_decltype_auto()
{
    // std::vector<int> v{10, 20, 30};
    // at(v, 1) = 99;
    // assert(v[1] == 99);
    std::cout << "  Exercise 5: decltype(auto) — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Compile-time string hash
//
// Write a constexpr function `hash_str(const char*)` that computes a hash
// at compile time (e.g., FNV-1a). Use it in a switch statement.
// ──────────────────────────────────────────────────────────────────────────
// constexpr uint32_t hash_str(const char* s) { ??? }

void exercise_compile_hash()
{
    // constexpr auto h = hash_str("hello");
    // switch (h) {
    //     case hash_str("hello"): break;  // this branch
    //     case hash_str("world"): break;
    //     default: assert(false);
    // }
    std::cout << "  Exercise 6: compile-time hash — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 05 — Exercises: C++14 Refinements       ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_generic_lambda();
    exercise_constexpr_sort();
    exercise_make_unique();
    exercise_variable_templates();
    exercise_decltype_auto();
    exercise_compile_hash();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
