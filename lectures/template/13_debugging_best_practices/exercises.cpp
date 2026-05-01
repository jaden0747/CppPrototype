// ============================================================================
// Template 13 — Exercises: Debugging & Best Practices
// ============================================================================
#include <cassert>
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Add static_assert guards to this template class:
//   template<typename T, int N>
//   class FixedBuffer { T data[N]; ... };
// Assert: N > 0, N <= 1024, T is trivially copyable.
// ──────────────────────────────────────────────────────────────────────────
// TODO: add static_asserts inside FixedBuffer

void exercise_static_assert()
{
    // FixedBuffer<int, 10> buf;  // OK
    // FixedBuffer<int, 0> bad1;  // static_assert: N must be > 0
    // FixedBuffer<int, 2000> bad2;  // static_assert: N must be <= 1024
    // FixedBuffer<std::string, 10> bad3;  // static_assert: T must be trivially copyable
    std::cout << "  Exercise 1: static_assert guards — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Rewrite this SFINAE-heavy code using concepts and if constexpr:
//
// template<typename T, enable_if_t<is_integral_v<T>, int> = 0>
// string format(T val) { return to_string(val); }
// template<typename T, enable_if_t<is_floating_point_v<T>, int> = 0>
// string format(T val) { return to_string(val); }
// template<typename T, enable_if_t<is_same_v<T, string>, int> = 0>
// string format(T val) { return "\"" + val + "\""; }
// template<typename T, enable_if_t<is_same_v<T, bool>, int> = 0>
// string format(T val) { return val ? "true" : "false"; }
// ──────────────────────────────────────────────────────────────────────────
// TODO: rewrite using modern C++20 style

void exercise_modernize()
{
    // assert(format(42) == "42");
    // assert(format(3.14) == "3.140000");
    // assert(format(std::string("hi")) == "\"hi\"");
    // assert(format(true) == "true");
    std::cout << "  Exercise 2: modernize SFINAE — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a test suite for a template `Map<K, V>` (simplified).
// Test with at least 3 different type combinations:
//   - Map<int, string>
//   - Map<string, int>
//   - Map<string, vector<int>>
// Each test should verify: insert, find, size, remove.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename K, typename V> class Map { ... };
// TODO: template<typename K, typename V> void test_map() { ... }

void exercise_test_suite()
{
    // test_map<int, std::string>();
    // test_map<std::string, int>();
    // test_map<std::string, std::vector<int>>();
    std::cout << "  Exercise 3: test suite — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Measure and reduce template instantiations.
// Given a utility library that uses templates heavily:
//   template<typename T> T parse(const string& s);
//   template<typename T> string stringify(const T& val);
// Write explicit instantiations for common types (int, double, string)
// and demonstrate the extern template pattern.
// ──────────────────────────────────────────────────────────────────────────
// TODO: extern template declarations + definitions

void exercise_extern_template()
{
    // auto x = parse<int>("42");
    // auto s = stringify(3.14);
    std::cout << "  Exercise 4: extern template — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Refactor this over-templated code into simpler code.
// Identify which templates are unnecessary and replace with:
//   - Regular functions
//   - Runtime polymorphism (where appropriate)
//   - constexpr functions
//
// template<typename T>
// struct Adder { T operator()(T a, T b) { return a + b; } };
// template<typename T>
// struct Multiplier { T operator()(T a, T b) { return a * b; } };
// template<typename Op, typename T>
// T apply_op(Op op, T a, T b) { return op(a, b); }
// template<typename T>
// T compute(T a, T b, bool do_add) {
//     if (do_add) return apply_op(Adder<T>{}, a, b);
//     else return apply_op(Multiplier<T>{}, a, b);
// }
// ──────────────────────────────────────────────────────────────────────────
// TODO: simplify the above

void exercise_simplify()
{
    // auto r1 = compute(3, 4, true);   // 7
    // auto r2 = compute(3, 4, false);  // 12
    // assert(r1 == 7 && r2 == 12);
    std::cout << "  Exercise 5: simplify — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 13 — Exercises: Debugging & Best       ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_static_assert();
    exercise_modernize();
    exercise_test_suite();
    exercise_extern_template();
    exercise_simplify();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
