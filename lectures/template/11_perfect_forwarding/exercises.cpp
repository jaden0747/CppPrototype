// ============================================================================
// Template 11 — Exercises: Perfect Forwarding & Reference Collapsing
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write a `log_and_call(func, args...)` that:
//   - Prints "Calling function with N args"
//   - Perfect-forwards all args to func
//   - Returns the result of func
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename F, typename... Args>
//       auto log_and_call(F&& func, Args&&... args)

void exercise_log_and_call()
{
    // auto result = log_and_call([](int a, int b) { return a + b; }, 1, 2);
    // assert(result == 3);
    //
    // std::string s = "hello";
    // log_and_call([](std::string& s) { s += " world"; }, s);
    // assert(s == "hello world");
    std::cout << "  Exercise 1: log_and_call — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write a `make_from_tuple<T>(tuple)` that constructs T
// from a tuple's elements using perfect forwarding.
// Hint: use std::index_sequence.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T, typename Tuple>
//       T make_from_tuple(Tuple&& t)

void exercise_make_from_tuple()
{
    // struct Widget {
    //     std::string name; int value;
    //     Widget(std::string n, int v) : name(std::move(n)), value(v) {}
    // };
    // auto t = std::make_tuple(std::string("foo"), 42);
    // auto w = make_from_tuple<Widget>(std::move(t));
    // assert(w.name == "foo" && w.value == 42);
    std::cout << "  Exercise 2: make_from_tuple — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write an `emplace_back` wrapper that perfectly forwards
// arguments to construct an element in-place:
//   template<typename Container, typename... Args>
//   auto& emplace_back(Container& c, Args&&... args)
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement emplace_back

void exercise_emplace()
{
    // std::vector<std::pair<std::string, int>> v;
    // emplace_back(v, "hello", 42);
    // assert(v.back().first == "hello" && v.back().second == 42);
    std::cout << "  Exercise 3: emplace_back — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a `Wrapper<T>` class that:
//   - Takes a T&& in constructor (perfect forwarding)
//   - Has apply(F&& f) that calls f(value_)
//   - Correctly handles lvalue and rvalue wrappers
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> class Wrapper { ... };

void exercise_wrapper()
{
    // std::string s = "hello";
    // auto w = Wrapper(s);           // stores reference/copy?
    // w.apply([](auto& x) { x += " world"; });
    //
    // auto w2 = Wrapper(std::string("temp"));
    // w2.apply([](auto& x) { assert(x == "temp"); });
    std::cout << "  Exercise 4: Wrapper — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write a thread-safe `lazy<T>` that:
//   - Stores a factory function F
//   - On first access, calls F() with forwarded args
//   - Returns const T& on subsequent accesses
//   - Uses std::call_once for thread safety
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> class lazy { ... };

void exercise_lazy()
{
    // int call_count = 0;
    // lazy<std::string> val([&call_count] {
    //     ++call_count;
    //     return std::string("computed");
    // });
    // assert(val.get() == "computed");
    // assert(val.get() == "computed");
    // assert(call_count == 1);  // factory only called once
    std::cout << "  Exercise 5: lazy — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 11 — Exercises: Perfect Forwarding     ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_log_and_call();
    exercise_make_from_tuple();
    exercise_emplace();
    exercise_wrapper();
    exercise_lazy();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
