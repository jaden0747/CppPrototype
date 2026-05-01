// ============================================================================
// Lecture 07 — Exercises: C++17 Vocabulary Types
// ============================================================================
#include <any>
#include <cassert>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: std::optional — Safe division
// Return std::nullopt if dividing by zero.
// ──────────────────────────────────────────────────────────────────────────
// TODO: std::optional<double> safe_divide(double a, double b) { ... }

void exercise_optional()
{
    // assert(safe_divide(10.0, 2.0) == 5.0);
    // assert(!safe_divide(1.0, 0.0).has_value());
    // assert(safe_divide(10.0, 0.0).value_or(-1.0) == -1.0);
    std::cout << "  Exercise 1: optional — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: std::variant — Expression evaluator
// Define: using Expr = std::variant<int, double, std::string>;
// Write evaluate(Expr) that:
//   int → return as double
//   double → return as-is
//   string → parse with stod, return as double, or 0.0 on failure
// ──────────────────────────────────────────────────────────────────────────
// using Expr = std::variant<int, double, std::string>;
// TODO: double evaluate(const Expr& e) { return std::visit(...); }

void exercise_variant()
{
    // assert(evaluate(Expr{42}) == 42.0);
    // assert(evaluate(Expr{3.14}) == 3.14);
    // assert(evaluate(Expr{std::string("2.5")}) == 2.5);
    // assert(evaluate(Expr{std::string("bad")}) == 0.0);
    std::cout << "  Exercise 2: variant evaluator — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Overloaded visitor pattern
// Implement the overloaded helper struct from scratch.
// Then use it to pretty-print a variant<int, double, bool, string>.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<class... Ts> struct overloaded : Ts... { ... };
// TODO: deduction guide

void exercise_overloaded()
{
    // using Val = std::variant<int, double, bool, std::string>;
    // Val v = true;
    // std::string result;
    // std::visit(overloaded{
    //     [&](int i)    { result = "int:" + std::to_string(i); },
    //     [&](double d) { result = "dbl:" + std::to_string(d); },
    //     [&](bool b)   { result = std::string("bool:") + (b?"T":"F"); },
    //     [&](const std::string& s) { result = "str:" + s; },
    // }, v);
    // assert(result == "bool:T");
    std::cout << "  Exercise 3: overloaded visitor — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: std::any property bag
// Implement a PropertyBag class with:
//   void set(string key, any value)
//   template<typename T> std::optional<T> get(string key)
// ──────────────────────────────────────────────────────────────────────────
// TODO: class PropertyBag { ... };

void exercise_any()
{
    // PropertyBag bag;
    // bag.set("width", 1920);
    // bag.set("title", std::string("Hello"));
    // assert(bag.get<int>("width") == 1920);
    // assert(bag.get<std::string>("title") == "Hello");
    // assert(!bag.get<int>("missing").has_value());
    // assert(!bag.get<double>("width").has_value());  // wrong type
    std::cout << "  Exercise 4: any property bag — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: string_view — CSV parser
// Write: std::vector<std::string_view> split(std::string_view input, char delim)
// that splits without any string allocation.
// ──────────────────────────────────────────────────────────────────────────
// TODO: std::vector<std::string_view> split(std::string_view input, char delim) { ... }

void exercise_string_view()
{
    // std::string_view csv = "one,two,three,four";
    // auto parts = split(csv, ',');
    // assert(parts.size() == 4);
    // assert(parts[0] == "one");
    // assert(parts[3] == "four");
    //
    // auto empty = split("", ',');
    // assert(empty.size() == 1);  // one empty element
    std::cout << "  Exercise 5: string_view split — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Type-safe event system
// Build a mini event system where:
//   - Events are std::variant<MouseEvent, KeyEvent, ResizeEvent>
//   - Each event type is a struct with relevant data
//   - A dispatch(event, handler...) function uses visit + overloaded
//   - Handlers return std::optional<std::string> (action taken or nullopt)
// ──────────────────────────────────────────────────────────────────────────

struct MouseEvent
{
    int  x, y;
    bool clicked;
};
struct KeyEvent
{
    char key;
    bool ctrl;
};
struct ResizeEvent
{
    int w, h;
};
using Event = std::variant<MouseEvent, KeyEvent, ResizeEvent>;

// TODO: std::optional<std::string> dispatch(const Event& e) { ... }

void exercise_event_system()
{
    // Event e1 = MouseEvent{100, 200, true};
    // Event e2 = KeyEvent{'q', true};
    // Event e3 = ResizeEvent{1920, 1080};
    // auto r1 = dispatch(e1);
    // auto r2 = dispatch(e2);
    // auto r3 = dispatch(e3);
    // assert(r1.has_value());  // mouse click generates action
    // assert(r2.has_value());  // ctrl+q generates "quit"
    std::cout << "  Exercise 6: event system — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 07 — Exercises: Vocabulary Types         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_optional();
    exercise_variant();
    exercise_overloaded();
    exercise_any();
    exercise_string_view();
    exercise_event_system();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
