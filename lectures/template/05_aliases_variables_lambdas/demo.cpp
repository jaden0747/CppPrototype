// ============================================================================
// Template 05 — Demo: Aliases, Variable Templates & Lambdas
// ============================================================================
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Template Type Aliases
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
using Vec = std::vector<T>;

template <typename T>
using StringMap = std::map<std::string, T>;

// Clean up trait usage
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

// Alias for function signatures
template <typename T>
using Predicate = std::function<bool(const T&)>;

void demo_aliases()
{
    std::cout << "=== 1. Template Type Aliases ===\n";

    Vec<int> numbers{1, 2, 3, 4, 5};
    std::cout << "  Vec<int> size: " << numbers.size() << "\n";

    StringMap<int> ages;
    ages["Alice"] = 30;
    ages["Bob"]   = 25;
    std::cout << "  StringMap: Alice=" << ages["Alice"] << "\n";

    // remove_cvref_t
    static_assert(std::is_same_v<remove_cvref_t<const int&>, int>);
    static_assert(std::is_same_v<remove_cvref_t<volatile double&&>, double>);
    std::cout << "  remove_cvref_t<const int&> → int ✓\n";

    // Predicate alias
    Predicate<int> is_even = [](const int& x) { return x % 2 == 0; };
    std::cout << "  is_even(4) = " << std::boolalpha << is_even(4) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Variable Templates (C++14)
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
constexpr T pi = T(3.14159265358979323846L);

template <typename T>
constexpr T e_val = T(2.71828182845904523536L);

template <typename T>
constexpr T tau = pi<T> * T(2);

// Bool variable templates (like the standard library)
template <typename T>
constexpr bool is_numeric_v = std::is_arithmetic_v<T>;

template <typename T>
constexpr bool is_text_v = std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, const char*>;

void demo_variable_templates()
{
    std::cout << "=== 2. Variable Templates ===\n";
    std::cout << "  pi<float>  = " << pi<float> << "\n";
    std::cout << "  pi<double> = " << pi<double> << "\n";
    std::cout << "  e<double>  = " << e_val<double> << "\n";
    std::cout << "  tau<double>= " << tau<double> << "\n";

    std::cout << "  is_numeric_v<int> = " << std::boolalpha << is_numeric_v<int> << "\n";
    std::cout << "  is_numeric_v<string> = " << is_numeric_v<std::string> << "\n";
    std::cout << "  is_text_v<string> = " << is_text_v<std::string> << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Generic Lambdas (C++14)
// ──────────────────────────────────────────────────────────────────────────
void demo_generic_lambdas()
{
    std::cout << "=== 3. Generic Lambdas ===\n";

    // Basic generic lambda
    auto add = [](auto a, auto b) { return a + b; };
    std::cout << "  add(1, 2) = " << add(1, 2) << "\n";
    std::cout << "  add(1.5, 2.3) = " << add(1.5, 2.3) << "\n";
    std::cout << "  add(\"hello\", \" world\") = " << add(std::string("hello"), std::string(" world")) << "\n";

    // Generic lambda with auto&& (perfect forwarding)
    auto print = [](auto&& val) { std::cout << "  value: " << val << "\n"; };
    print(42);
    print("hello");

    // Recursive generic lambda (C++14 pattern)
    auto factorial = [](auto self, int n) -> int { return n <= 1 ? 1 : n * self(self, n - 1); };
    std::cout << "  factorial(5) = " << factorial(factorial, 5) << "\n";

    // Higher-order generic lambda
    auto apply_twice = [](auto f, auto x) { return f(f(x)); };
    auto increment   = [](int x) { return x + 1; };
    std::cout << "  apply_twice(inc, 5) = " << apply_twice(increment, 5) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. if constexpr (C++17)
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
std::string to_string_smart(const T& val)
{
    if constexpr (std::is_arithmetic_v<T>)
        return std::to_string(val);
    else if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
        return val;
    else if constexpr (std::is_same_v<std::decay_t<T>, const char*>)
        return std::string(val);
    else
        return "[unprintable]";
}

// Replace SFINAE with if constexpr
template <typename T>
auto smart_abs(T val)
{
    if constexpr (std::is_unsigned_v<T>)
        return val; // unsigned is always non-negative
    else if constexpr (std::is_floating_point_v<T>)
        return std::fabs(val);
    else
        return val < 0 ? -val : val;
}

void demo_if_constexpr()
{
    std::cout << "=== 4. if constexpr ===\n";
    std::cout << "  to_string(42) = " << to_string_smart(42) << "\n";
    std::cout << "  to_string(3.14) = " << to_string_smart(3.14) << "\n";
    std::cout << "  to_string(\"hi\") = " << to_string_smart(std::string("hi")) << "\n";

    std::cout << "  smart_abs(-5) = " << smart_abs(-5) << "\n";
    std::cout << "  smart_abs(-3.14) = " << smart_abs(-3.14) << "\n";
    std::cout << "  smart_abs(5u) = " << smart_abs(5u) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Template Lambdas (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_template_lambdas()
{
    std::cout << "=== 5. Template Lambdas (C++20) ===\n";

    // Explicit template parameter on lambda
    auto get_size = []<typename T>(const std::vector<T>& v) -> size_t { return v.size(); };
    std::cout << "  get_size({1,2,3}) = " << get_size(std::vector{1, 2, 3}) << "\n";

    // Access the template parameter for type info
    auto type_name = []<typename T>(T) -> const char*
    {
        if constexpr (std::is_integral_v<T>)
            return "integral";
        else if constexpr (std::is_floating_point_v<T>)
            return "floating";
        else
            return "other";
    };
    std::cout << "  type_name(42) = " << type_name(42) << "\n";
    std::cout << "  type_name(3.14) = " << type_name(3.14) << "\n";

    // Template lambda for creating containers
    auto make_filled = []<typename T>(T val, size_t count) { return std::vector<T>(count, val); };
    auto v           = make_filled(3.14, 3);
    std::cout << "  make_filled(3.14, 3) = [";
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i)
            std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 05 — Aliases, Variables & Lambdas      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_aliases();
    demo_variable_templates();
    demo_generic_lambdas();
    demo_if_constexpr();
    demo_template_lambdas();

    std::cout << "All demos complete.\n";
    return 0;
}
