// ============================================================================
// Stdlib 11 — Exercises: Utilities & Type Support
// ============================================================================
#include <any>
#include <cassert>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

// ── Exercise 1: Safe dictionary lookup with optional ─────────────────────
void ex1_safe_lookup()
{
    std::cout << "Exercise 1: Safe lookup with optional\n";

    // TODO: Write a function lookup(map, key) -> optional<value>
    //        that returns nullopt if key not found
    // TODO: Test with a map<string, int> of word frequencies
    // TODO: Use value_or to provide a default

    std::cout << "\n";
}

// ── Exercise 2: Variant-based calculator ─────────────────────────────────
// TODO: Define Expr = variant<int, Add, Mul> where Add and Mul hold two ints

void ex2_variant_calc()
{
    std::cout << "Exercise 2: Variant calculator\n";

    // TODO: Create expressions: literal 5, Add(3,4), Mul(2,6)
    // TODO: Write an evaluate function using std::visit
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 3: Config value with variant ────────────────────────────────
void ex3_config()
{
    std::cout << "Exercise 3: Config with variant\n";

    using ConfigValue = std::variant<int, double, bool, std::string>;

    // TODO: Create a map<string, ConfigValue> for application config
    // TODO: Add entries: "width"=1920, "ratio"=1.5, "fullscreen"=true, "title"="App"
    // TODO: Print all config values using visit

    std::cout << "\n";
}

// ── Exercise 4: Type-erased event system with any ────────────────────────
void ex4_event_system()
{
    std::cout << "Exercise 4: Event system with any\n";

    // TODO: Create a simple event system:
    //   - map<string, vector<any>> stores event data
    //   - emit(name, any) adds data to event
    //   - get_events<T>(name) returns the events as vector<T>
    // TODO: Emit some events, retrieve and print them

    std::cout << "\n";
}

// ── Exercise 5: Tuple operations ─────────────────────────────────────────
void ex5_tuple_ops()
{
    std::cout << "Exercise 5: Tuple operations\n";

    // TODO: Create a tuple<string, int, double> representing (name, age, gpa)
    // TODO: Use structured bindings to decompose
    // TODO: Use std::apply to pass tuple elements to a print function
    // TODO: Use tuple_cat to combine two tuples

    std::cout << "\n";
}

// ── Exercise 6: Compile-time dispatch with type_traits ───────────────────
// TODO: Write a template function stringify(T value) -> string that:
//   - For integral types: uses std::to_string
//   - For floating point: uses std::format or to_string with precision
//   - For string types: returns as-is
// Use if constexpr + type_traits

void ex6_traits_dispatch()
{
    std::cout << "Exercise 6: Type traits dispatch\n";

    // TODO: Test stringify with int, double, and string

    std::cout << "\n";
}

// ── Exercise 7: Callback registry with std::function ─────────────────────
void ex7_callbacks()
{
    std::cout << "Exercise 7: Callback registry\n";

    // TODO: Create a class EventBus with:
    //   - on(string event, function<void(int)> callback)
    //   - emit(string event, int data)
    // TODO: Register callbacks for "click" and "keypress"
    // TODO: Emit events and verify callbacks fire

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — Result<T,E> type ─────────────────────────
// TODO: Implement Result<T,E> using std::variant<T,E> with:
//   - is_ok(), is_err()
//   - value(), error()
//   - map(fn) — transform the value
//   - or_else(fn) — handle error

void ex8_result_type()
{
    std::cout << "Exercise 8: Result type\n";

    // TODO: Write a divide(int, int) -> Result<double, string>
    //        that returns error on division by zero
    // TODO: Chain with map() to double the result
    // TODO: Print results

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 11 — Exercises: Utilities & Type Support  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_safe_lookup();
    ex2_variant_calc();
    ex3_config();
    ex4_event_system();
    ex5_tuple_ops();
    ex6_traits_dispatch();
    ex7_callbacks();
    ex8_result_type();

    std::cout << "All exercises complete.\n";
    return 0;
}
