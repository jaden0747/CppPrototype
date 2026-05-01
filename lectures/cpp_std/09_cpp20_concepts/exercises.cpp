// ============================================================================
// Lecture 09 — Exercises: C++20 Concepts
// ============================================================================
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Define a Container concept
// Must have: begin(), end(), size(), value_type
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename T>
// concept Container = requires(T c) { ... };

void exercise_container_concept()
{
    // static_assert(Container<std::vector<int>>);
    // static_assert(Container<std::string>);
    // static_assert(!Container<int>);
    // static_assert(!Container<double>);
    std::cout << "  Exercise 1: Container concept — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Serializable concept + function
// Concept: type must have a `.serialize() -> std::string` method
// Function: serialize_all(Container auto& c) where elements are Serializable
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename T>
// concept Serializable = requires(const T& t) {
//     { t.serialize() } -> std::convertible_to<std::string>;
// };
//
// std::string serialize_all(const auto& container)
//     requires Container<...> && Serializable<...>
// { ... }

struct Record
{
    int         id;
    std::string name;
    std::string serialize() const
    {
        return std::to_string(id) + ":" + name;
    }
};

void exercise_serializable()
{
    // std::vector<Record> records{{1, "Alice"}, {2, "Bob"}};
    // auto result = serialize_all(records);
    // assert(result.find("1:Alice") != std::string::npos);
    // assert(result.find("2:Bob") != std::string::npos);
    std::cout << "  Exercise 2: Serializable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Terse syntax algorithms
// Use abbreviated function templates (Concept auto):
//   a) auto double_it(Numeric auto x) -> returns x*2
//   b) void print_if(Container auto& c, std::predicate<???> auto pred)
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement using terse syntax

void exercise_terse()
{
    // assert(double_it(21) == 42);
    // assert(double_it(1.5) == 3.0);
    // std::vector<int> v{1,2,3,4,5,6};
    // print_if(v, [](int x) { return x % 2 == 0; }); // prints 2 4 6
    std::cout << "  Exercise 3: terse syntax — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Concept-based overloading
// Write process(T) with three overloads:
//   - std::integral T → "integer: N"
//   - std::floating_point T → "float: N"  (with 2 decimal places)
//   - Printable T (anything streamable) → "other: ..."
// Most constrained should win.
// ──────────────────────────────────────────────────────────────────────────
// TODO: three overloads of process()

void exercise_overloading()
{
    // assert(process(42) == "integer: 42");
    // assert(process(3.14).substr(0, 6) == "float:");
    // assert(process(std::string("hi")) == "other: hi");
    std::cout << "  Exercise 4: concept overloading — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Concept hierarchy
// Build a hierarchy:
//   Number (arithmetic) → Integer (integral) → SignedInt (signed_integral)
// Write a function that accepts only SignedInt.
// Verify subsumption works (most constrained selected).
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename T> concept Number = ...;
// template<typename T> concept Integer = Number<T> && ...;
// template<typename T> concept SignedInt = Integer<T> && ...;

void exercise_hierarchy()
{
    // auto x = negate(42);  // uses SignedInt overload
    // assert(x == -42);
    // negate(42u); // should fail to compile (unsigned not SignedInt)
    std::cout << "  Exercise 5: concept hierarchy — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Concept-constrained pipeline
// Build Pipeline<F1, F2, ...> where each Fi is a callable.
// Pipeline::operator()(x) applies f1, then f2, ... in sequence.
// Constraint: each function's return type must be valid input to the next.
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename... Fs>
// struct Pipeline { ... };
// Deduction guide: Pipeline(Fs...) -> Pipeline<Fs...>;

void exercise_pipeline()
{
    // auto p = Pipeline{
    //     [](int x) { return x * 2; },
    //     [](int x) { return x + 1; },
    //     [](int x) { return std::to_string(x); }
    // };
    // assert(p(20) == "41");  // 20*2=40, 40+1=41, to_string
    std::cout << "  Exercise 6: pipeline — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 09 — Exercises: C++20 Concepts          ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_container_concept();
    exercise_serializable();
    exercise_terse();
    exercise_overloading();
    exercise_hierarchy();
    exercise_pipeline();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
