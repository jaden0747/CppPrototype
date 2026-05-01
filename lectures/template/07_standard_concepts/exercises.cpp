// ============================================================================
// Template 07 — Exercises: Standard Concepts & Constrained Templates
// ============================================================================
#include <cassert>
#include <concepts>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write three overloads of `convert_to_string` using concept
// subsumption:
//   - unconstrained → returns "[unknown]"
//   - std::integral → returns std::to_string(val)
//   - std::signed_integral → returns "signed: " + to_string
//   Verify the most constrained overload wins for int.
// ──────────────────────────────────────────────────────────────────────────
// TODO: three overloads

void exercise_subsumption()
{
    // assert(convert_to_string(42) == "signed: 42");      // signed_integral wins
    // assert(convert_to_string(42u) == "42");              // integral wins
    // assert(convert_to_string(3.14) == "[unknown]");      // unconstrained
    std::cout << "  Exercise 1: subsumption — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write a `SortedSet<T>` class constrained to std::totally_ordered.
// It should:
//   - insert(T) — keep sorted, no duplicates
//   - contains(T) → bool
//   - size() → size_t
//   - Conditionally enable `min()` and `max()` that require !empty()
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<std::totally_ordered T> class SortedSet { ... };

void exercise_sorted_set()
{
    // SortedSet<int> s;
    // s.insert(3); s.insert(1); s.insert(2); s.insert(1);
    // assert(s.size() == 3);
    // assert(s.contains(2));
    // assert(!s.contains(4));
    // assert(s.min() == 1);
    // assert(s.max() == 3);
    std::cout << "  Exercise 2: SortedSet — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Rewrite these SFINAE functions using concepts:
//   template<typename T, enable_if_t<is_arithmetic_v<T>, int> = 0>
//   T safe_add(T a, T b) { return a + b; }
//
//   template<typename C, enable_if_t<..has begin/end..>, int> = 0>
//   size_t count(const C& c) { return distance(begin, end); }
// ──────────────────────────────────────────────────────────────────────────
// TODO: rewrite using abbreviated syntax or requires clause

void exercise_migration()
{
    // assert(safe_add(1, 2) == 3);
    // assert(safe_add(1.5, 2.5) == 4.0);
    // assert(count(std::vector<int>{1,2,3}) == 3);
    std::cout << "  Exercise 3: SFINAE migration — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a `Pipeline` class that chains functions:
//   Pipeline p;
//   auto result = p.then([](int x) { return x * 2; })
//                  .then([](int x) { return x + 1; })
//                  .run(5);  // returns 11
// Constrain `.then(f)` to require std::invocable<F, T>.
// ──────────────────────────────────────────────────────────────────────────
// TODO: Pipeline class

void exercise_pipeline()
{
    // auto result = Pipeline<int>{}
    //     .then([](int x) { return x * 2; })
    //     .then([](int x) { return x + 1; })
    //     .run(5);
    // assert(result == 11);
    std::cout << "  Exercise 4: Pipeline — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write abbreviated function templates for:
//   - map(range, func) → vector of results
//   - filter(range, pred) → vector of matching elements
//   - reduce(range, init, func) → single value
// Use std::ranges concepts for the range parameter.
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement map, filter, reduce

void exercise_functional()
{
    // std::vector<int> v{1, 2, 3, 4, 5};
    // auto doubled = map(v, [](int x) { return x * 2; });
    // assert(doubled == (std::vector<int>{2,4,6,8,10}));
    //
    // auto evens = filter(v, [](int x) { return x % 2 == 0; });
    // assert(evens == (std::vector<int>{2, 4}));
    //
    // auto total = reduce(v, 0, [](int a, int b) { return a + b; });
    // assert(total == 15);
    std::cout << "  Exercise 5: map/filter/reduce — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 07 — Exercises: Standard Concepts      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_subsumption();
    exercise_sorted_set();
    exercise_migration();
    exercise_pipeline();
    exercise_functional();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
