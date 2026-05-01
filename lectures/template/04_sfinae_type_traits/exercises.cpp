// ============================================================================
// Template 04 — Exercises: SFINAE & Type Traits
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write an `is_string<T>` trait that is true for std::string
// and const char*. Use it with enable_if to write a `to_upper(T)` that only
// compiles for string types.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> struct is_string : std::false_type {};
// TODO: specializations
// TODO: to_upper function

void exercise_is_string()
{
    // static_assert(is_string<std::string>::value);
    // static_assert(is_string<const char*>::value);
    // static_assert(!is_string<int>::value);
    std::cout << "  Exercise 1: is_string — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write a `has_begin_end<T>` detector using void_t.
// True if T has .begin() and .end() methods.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T, typename = void>
//       struct has_begin_end : std::false_type {};

void exercise_has_begin_end()
{
    // static_assert(has_begin_end<std::vector<int>>::value);
    // static_assert(has_begin_end<std::string>::value);
    // static_assert(!has_begin_end<int>::value);
    // static_assert(!has_begin_end<double>::value);
    std::cout << "  Exercise 2: has_begin_end — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a function `smart_print(T val)` that uses enable_if to:
//   - Print arithmetic types with std::to_string
//   - Print types with .c_str() by calling it
//   - Print types with .begin()/.end() as [a, b, c]
// ──────────────────────────────────────────────────────────────────────────
// TODO: three overloads of smart_print with enable_if

void exercise_smart_print()
{
    // smart_print(42);            // "42"
    // smart_print(3.14);          // "3.140000"
    // smart_print(std::string("hi")); // "hi"
    // smart_print(std::vector<int>{1,2,3}); // "[1, 2, 3]"
    std::cout << "  Exercise 3: smart_print — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a `promote<T>` type transformation that:
//   - int → long
//   - float → double
//   - short → int
//   - Everything else → T (unchanged)
// Use conditional_t or specializations.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> struct promote { using type = T; };

void exercise_promote()
{
    // static_assert(std::is_same_v<promote<int>::type, long>);
    // static_assert(std::is_same_v<promote<float>::type, double>);
    // static_assert(std::is_same_v<promote<short>::type, int>);
    // static_assert(std::is_same_v<promote<double>::type, double>);
    std::cout << "  Exercise 4: promote — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write `is_callable<F, Args...>` that detects
// whether F(Args...) is a valid expression. Use declval and void_t.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename, typename = void> struct is_callable : std::false_type {};

void exercise_is_callable()
{
    // auto lambda = [](int x) { return x * 2; };
    // static_assert(is_callable<decltype(lambda), int>::value);
    // static_assert(!is_callable<decltype(lambda), std::string>::value);
    // static_assert(!is_callable<int, int>::value);
    std::cout << "  Exercise 5: is_callable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 04 — Exercises: SFINAE & Type Traits   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_is_string();
    exercise_has_begin_end();
    exercise_smart_print();
    exercise_promote();
    exercise_is_callable();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
