// ============================================================================
// Template 06 — Exercises: Concepts — Defining & Using
// ============================================================================
#include <cassert>
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Define a concept `Incrementable<T>` that requires:
//   - ++a (pre-increment)
//   - a++ (post-increment)
//   - a += 1
// Test it with int, double, and std::string.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> concept Incrementable = requires(T a) { ... };

void exercise_incrementable()
{
    // static_assert(Incrementable<int>);
    // static_assert(Incrementable<double>);
    // static_assert(!Incrementable<std::string>);
    std::cout << "  Exercise 1: Incrementable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Define a concept `Serializable<T>` that requires T to have:
//   - std::string serialize() const
//   - static T deserialize(const std::string&)
// Write a struct `Point` that satisfies Serializable.
// ──────────────────────────────────────────────────────────────────────────
// TODO: concept + struct

void exercise_serializable()
{
    // static_assert(Serializable<Point>);
    // Point p{1.0, 2.0};
    // auto s = p.serialize();
    // auto p2 = Point::deserialize(s);
    std::cout << "  Exercise 2: Serializable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write `constrained_sort(auto& container)` that requires:
//   - container is a Container (has begin/end/size)
//   - container's value_type satisfies std::totally_ordered
// Use std::sort internally.
// ──────────────────────────────────────────────────────────────────────────
// TODO: void constrained_sort(auto& c) requires ...

void exercise_sort()
{
    // std::vector<int> v{3, 1, 4, 1, 5};
    // constrained_sort(v);
    // assert(v == (std::vector<int>{1, 1, 3, 4, 5}));
    std::cout << "  Exercise 3: constrained_sort — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Define a concept `Callable<F, Args...>` that checks:
//   - F can be called with Args...
//   - The result is not void
// Use it to constrain a `transform` function.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename F, typename... Args>
//       concept Callable = requires(F f, Args... args) { ... };

void exercise_callable()
{
    // auto dbl = [](int x) { return x * 2; };
    // static_assert(Callable<decltype(dbl), int>);
    // static_assert(!Callable<decltype(dbl), std::string>);
    std::cout << "  Exercise 4: Callable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Create a concept hierarchy:
//   - Readable<T>: has .read() -> string
//   - Writable<T>: has .write(string) -> void
//   - ReadWritable<T>: both Readable and Writable
// Then write a `copy(Readable auto& src, Writable auto& dst)` function.
// ──────────────────────────────────────────────────────────────────────────
// TODO: concepts + copy function

void exercise_rw()
{
    // struct Source { string read() const { return "data"; } };
    // struct Sink   { void write(string s) { stored = s; } string stored; };
    // Source src; Sink dst;
    // copy(src, dst);
    // assert(dst.stored == "data");
    std::cout << "  Exercise 5: Readable/Writable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 06 — Exercises: Concepts Basics        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_incrementable();
    exercise_serializable();
    exercise_sort();
    exercise_callable();
    exercise_rw();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
