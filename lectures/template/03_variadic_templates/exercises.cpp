// ============================================================================
// Template 03 — Exercises: Variadic Templates
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write a variadic `max_of(args...)` that returns the largest
// value. Use fold expressions or recursive unpacking.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T, typename... Rest> auto max_of(T first, Rest... rest)

void exercise_max_of()
{
    // assert(max_of(1) == 1);
    // assert(max_of(3, 7, 2, 9, 1) == 9);
    // assert(max_of(1.5, 2.3, 0.1) == 2.3);
    std::cout << "  Exercise 1: max_of — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write `count_if(pred, args...)` that returns how many
// args satisfy the predicate. Use fold expressions with the comma operator.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Pred, typename... Ts>
//       int count_if(Pred pred, Ts... args)

void exercise_count_if()
{
    // auto is_even = [](int x) { return x % 2 == 0; };
    // assert(count_if(is_even, 1, 2, 3, 4, 5, 6) == 3);
    //
    // auto is_positive = [](double x) { return x > 0; };
    // assert(count_if(is_positive, -1.0, 2.0, -3.0, 4.0) == 2);
    std::cout << "  Exercise 2: count_if — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write `apply_all(val, funcs...)` that applies each function
// to val and returns a tuple of results.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Val, typename... Funcs>
//       auto apply_all(Val val, Funcs... funcs)

void exercise_apply_all()
{
    // auto dbl = [](int x) { return x * 2; };
    // auto neg = [](int x) { return -x; };
    // auto str = [](int x) { return std::to_string(x); };
    // auto result = apply_all(5, dbl, neg, str);
    // assert(std::get<0>(result) == 10);
    // assert(std::get<1>(result) == -5);
    // assert(std::get<2>(result) == "5");
    std::cout << "  Exercise 3: apply_all — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a variadic `concat(strings...)` that concatenates
// all arguments into a single std::string using a fold expression.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename... Ts> std::string concat(Ts... args)

void exercise_concat()
{
    // using namespace std::string_literals;
    // assert(concat("hello"s, " "s, "world"s) == "hello world");
    // assert(concat("a"s) == "a");
    std::cout << "  Exercise 4: concat — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write `for_each_arg(func, args...)` that
// applies func to each argument. Then use it to implement `print_csv`
// that prints comma-separated values.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename F, typename... Ts>
//       void for_each_arg(F func, Ts... args)
// TODO: template<typename... Ts> void print_csv(Ts... args)

void exercise_for_each()
{
    // for_each_arg([](auto x) { std::cout << x; }, 1, 2, 3);
    // print_csv(1, "hello", 3.14);  // output: 1,hello,3.14
    std::cout << "  Exercise 5: for_each_arg — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 03 — Exercises: Variadic Templates     ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_max_of();
    exercise_count_if();
    exercise_apply_all();
    exercise_concat();
    exercise_for_each();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
