// ============================================================================
// Template 05 — Exercises: Aliases, Variable Templates & Lambdas
// ============================================================================
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Create alias templates:
//   - Matrix<T> = vector<vector<T>>
//   - Dict<V> = map<string, V>
//   - Callback<R, Args...> = function<R(Args...)>
// Test them by creating instances.
// ──────────────────────────────────────────────────────────────────────────
// TODO: define the aliases

void exercise_aliases()
{
    // Matrix<int> m = {{1,2},{3,4}};
    // assert(m[0][0] == 1 && m[1][1] == 4);
    // Dict<int> d; d["key"] = 42;
    // assert(d["key"] == 42);
    std::cout << "  Exercise 1: aliases — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Create variable templates:
//   - golden_ratio<T> ≈ 1.6180339887...
//   - epsilon<T> = smallest T > 0 different from 0
//   - max_val<T> using std::numeric_limits
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> constexpr T golden_ratio = ...;

void exercise_var_templates()
{
    // static_assert(golden_ratio<float> > 1.618f);
    // static_assert(golden_ratio<float> < 1.619f);
    // std::cout << "  golden_ratio<double> = " << golden_ratio<double> << "\n";
    // std::cout << "  max_val<int> = " << max_val<int> << "\n";
    std::cout << "  Exercise 2: variable templates — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a generic lambda `transform_all` that takes a
// container and a function, returns a new vector with transformed elements.
// ──────────────────────────────────────────────────────────────────────────
// TODO: auto transform_all = [](auto& container, auto func) { ... };

void exercise_transform()
{
    // std::vector<int> v{1, 2, 3, 4, 5};
    // auto doubled = transform_all(v, [](int x) { return x * 2; });
    // assert(doubled == (std::vector<int>{2, 4, 6, 8, 10}));
    //
    // auto strs = transform_all(v, [](int x) { return std::to_string(x); });
    // assert(strs[0] == "1");
    std::cout << "  Exercise 3: transform_all — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a function `serialize(T val)` using if constexpr:
//   - bool → "true" / "false"
//   - integral → std::to_string(val)
//   - floating → std::to_string(val)
//   - string → "\"" + val + "\""
//   - vector<T> → "[elem1, elem2, ...]" (recursive)
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> std::string serialize(const T& val) { ... }

void exercise_serialize()
{
    // assert(serialize(true) == "true");
    // assert(serialize(42) == "42");
    // assert(serialize(std::string("hi")) == "\"hi\"");
    // assert(serialize(std::vector<int>{1,2}) == "[1, 2]");
    std::cout << "  Exercise 4: serialize — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write a C++20 template lambda `filter` that:
//   []<typename T>(const vector<T>& v, Predicate<T> pred) -> vector<T>
// Returns elements for which pred returns true.
// ──────────────────────────────────────────────────────────────────────────
// TODO: auto filter = []<typename T>(...) { ... };

void exercise_filter()
{
    // std::vector<int> v{1,2,3,4,5,6,7,8,9,10};
    // auto evens = filter(v, [](const int& x) { return x % 2 == 0; });
    // assert(evens == (std::vector<int>{2,4,6,8,10}));
    std::cout << "  Exercise 5: filter — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 05 — Exercises: Aliases & Lambdas      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_aliases();
    exercise_var_templates();
    exercise_transform();
    exercise_serialize();
    exercise_filter();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
