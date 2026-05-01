// ============================================================================
// Lecture 06 — Exercises: C++17 Ergonomics
// ============================================================================
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Structured bindings
// Iterate a map and find the entry with the highest value.
// ──────────────────────────────────────────────────────────────────────────
void exercise_structured_bindings()
{
    std::map<std::string, int> scores{{"Alice", 95}, {"Bob", 87}, {"Carol", 99}, {"Dave", 91}};
    // TODO: Use structured bindings + range-for to find the name with max score
    // std::string best_name;
    // int best_score = 0;
    // for (const auto& [name, score] : scores) { ??? }
    // assert(best_name == "Carol");
    // assert(best_score == 99);
    std::cout << "  Exercise 1: structured bindings — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: If with initializer
// Refactor the following to use if-with-init.
// ──────────────────────────────────────────────────────────────────────────
void exercise_if_init()
{
    std::map<int, std::string> ids{{1, "admin"}, {2, "user"}, {3, "guest"}};

    // BEFORE (refactor this):
    // auto it = ids.find(2);
    // if (it != ids.end()) {
    //     assert(it->second == "user");
    // }

    // TODO: Rewrite with if(auto it = ...; condition) { ... }

    std::cout << "  Exercise 2: if-init — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: CTAD
// Rewrite the following without explicit template arguments.
// ──────────────────────────────────────────────────────────────────────────
void exercise_ctad()
{
    // BEFORE:
    // std::pair<int, double> p1{1, 2.5};
    // std::vector<std::string> v1{"hello", "world"};
    // std::tuple<int, double, char> t1{1, 2.0, 'x'};

    // TODO: Rewrite using CTAD (no template args)
    // auto p1 = std::pair{1, 2.5};
    // ...
    std::cout << "  Exercise 3: CTAD — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Fold expressions
// Implement:
//  a) product(args...) — multiply all arguments
//  b) concat(strings...) — concatenate all strings
//  c) all_positive(args...) — true if all > 0
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename... Args> auto product(Args... args) { ??? }
// TODO: template<typename... Args> auto concat(Args... args) { ??? }
// TODO: template<typename... Args> bool all_positive(Args... args) { ??? }

void exercise_fold()
{
    // assert(product(2, 3, 4) == 24);
    // assert(product(1, 2, 3, 4, 5) == 120);
    // assert(concat(std::string("a"), std::string("b"), std::string("c")) == "abc");
    // assert(all_positive(1, 2, 3));
    // assert(!all_positive(1, -2, 3));
    std::cout << "  Exercise 4: fold expressions — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: constexpr if
// Write a single function `to_int(x)` that:
//  - If x is integral, returns it as-is
//  - If x is floating-point, returns static_cast<int>(x)
//  - If x is a string, returns stoi(x)
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> int to_int(T x) { if constexpr (...) }

void exercise_constexpr_if()
{
    // assert(to_int(42) == 42);
    // assert(to_int(3.7) == 3);
    // assert(to_int(std::string("123")) == 123);
    std::cout << "  Exercise 5: constexpr if — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Variadic config lookup
// Write get_first_of(map, key1, key2, ...) that returns the value of the
// first key found. Use fold expressions or if constexpr + recursion.
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement get_first_of

void exercise_first_of()
{
    // std::map<std::string, int> config{{"port", 8080}, {"timeout", 30}};
    // assert(get_first_of(config, "missing", "port") == 8080);
    // assert(get_first_of(config, "timeout", "port") == 30);
    std::cout << "  Exercise 6: get_first_of — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 06 — Exercises: C++17 Ergonomics        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_structured_bindings();
    exercise_if_init();
    exercise_ctad();
    exercise_fold();
    exercise_constexpr_if();
    exercise_first_of();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
