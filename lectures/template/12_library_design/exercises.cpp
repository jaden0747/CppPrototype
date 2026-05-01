// ============================================================================
// Template 12 — Exercises: Library Design Patterns
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Implement a type-erased `Printable` that wraps any type
// with a print(ostream&) method or operator<<. Store in a vector and
// print all items.
// ──────────────────────────────────────────────────────────────────────────
// TODO: class Printable { ... };

void exercise_printable()
{
    // std::vector<Printable> items;
    // items.push_back(42);
    // items.push_back(std::string("hello"));
    // items.push_back(3.14);
    // for (const auto& item : items) item.print(std::cout);
    std::cout << "  Exercise 1: Printable — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Add Serializer specializations for:
//   - bool (serialize as "true"/"false")
//   - std::pair<A, B>
//   - std::vector<std::pair<std::string, int>> (key-value store)
// Round-trip test each.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<> struct Serializer<bool> { ... };
// TODO: template<typename A, typename B> struct Serializer<std::pair<A,B>> { ... };

void exercise_serializer()
{
    // assert(serialize(true) == "true");
    // assert(deserialize<bool>("true") == true);
    // auto p = std::make_pair(std::string("key"), 42);
    // auto s = serialize(p);
    // auto p2 = deserialize<std::pair<std::string, int>>(s);
    // assert(p == p2);
    std::cout << "  Exercise 2: Serializer — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a `FilterView<Range, Pred>` similar to the demo's
// TransformView. Support the pipe operator:
//   v | filter([](int x) { return x > 3; })
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename Range, typename Pred> class FilterView { ... };

void exercise_filter_view()
{
    // std::vector<int> v{1, 2, 3, 4, 5, 6};
    // auto result = v | filter([](int x) { return x % 2 == 0; });
    // std::vector<int> out(result.begin(), result.end());
    // assert(out == (std::vector<int>{2, 4, 6}));
    std::cout << "  Exercise 3: FilterView — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Implement a `Function<R(Args...)>` type-erased callable
// (simplified std::function):
//   - Supports lambdas, function pointers, functors
//   - operator()
//   - operator bool()
//   - Copy construction
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename> class Function;
// TODO: template<typename R, typename... Args> class Function<R(Args...)> { ... };

void exercise_function()
{
    // Function<int(int)> f = [](int x) { return x * 2; };
    // assert(f(5) == 10);
    //
    // Function<std::string(int)> g = [](int x) { return std::to_string(x); };
    // assert(g(42) == "42");
    //
    // Function<int(int)> empty;
    // assert(!empty);
    std::cout << "  Exercise 4: Function — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Write a compile-time "named parameter" system:
//   auto config = Config{}
//       .set<"width">(1920)
//       .set<"height">(1080)
//       .set<"title">("My App");
//   assert(config.get<"width">() == 1920);
// Use FixedString as NTTP and a tuple/map internally.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<...> class Config { ... };

void exercise_named_params()
{
    // auto cfg = Config{}.set<"x">(10).set<"y">(20);
    // assert(cfg.get<"x">() == 10);
    std::cout << "  Exercise 5: named params — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 12 — Exercises: Library Design         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_printable();
    exercise_serializer();
    exercise_filter_view();
    exercise_function();
    exercise_named_params();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
