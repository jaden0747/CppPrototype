// ============================================================================
// Template 13 — Demo: Debugging & Best Practices
// ============================================================================
#include <cassert>
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. static_assert for Better Error Messages
// ──────────────────────────────────────────────────────────────────────────

template <typename T>
class NumericContainer
{
    static_assert(std::is_arithmetic_v<T>, "NumericContainer requires an arithmetic type (int, float, double, etc.)");

    std::vector<T> data_;

public:
    void add(T val)
    {
        data_.push_back(val);
    }
    T sum() const
    {
        T total{};
        for (auto v : data_)
            total += v;
        return total;
    }
    size_t size() const
    {
        return data_.size();
    }
};

// Concept-based constraints → clear error messages
template <typename T>
concept Summable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { T{} }; // default constructible for initial value
};

template <Summable T>
T accumulate(const std::vector<T>& v)
{
    T total{};
    for (const auto& elem : v)
        total += elem;
    return total;
}

void demo_static_assert()
{
    std::cout << "=== 1. static_assert for Errors ===\n";

    NumericContainer<int> nc;
    nc.add(1);
    nc.add(2);
    nc.add(3);
    std::cout << "  sum = " << nc.sum() << "\n";

    // NumericContainer<std::string> bad;  // clear static_assert message!

    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "  accumulate = " << accumulate(v) << "\n";

    // accumulate(std::vector<???>{});  // clear concept error message
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Extern Template (Compile Time Optimization)
// ──────────────────────────────────────────────────────────────────────────

// In a real project, you'd split this across header and source:
//
// widget.h:
//   template<typename T> class Widget { ... };
//   extern template class Widget<int>;     // don't instantiate here
//   extern template class Widget<double>;
//
// widget.cpp:
//   #include "widget.h"
//   template class Widget<int>;     // instantiate here only
//   template class Widget<double>;

template <typename T>
class Widget
{
    T value_;

public:
    explicit Widget(T v)
        : value_(v)
    {
    }
    T get() const
    {
        return value_;
    }
    void set(T v)
    {
        value_ = v;
    }
    void print() const
    {
        std::cout << "  Widget<" << typeid(T).name() << ">: " << value_ << "\n";
    }
};

// Explicit instantiation (in a real project, this would be in a .cpp file)
template class Widget<int>;
template class Widget<double>;
template class Widget<std::string>;

void demo_extern_template()
{
    std::cout << "=== 2. Extern Template ===\n";
    Widget<int> wi(42);
    wi.print();
    Widget<double> wd(3.14);
    wd.print();
    Widget<std::string> ws("hello");
    ws.print();
    std::cout << "  (In production, these would be pre-instantiated in a .cpp)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Testing Templates
// ──────────────────────────────────────────────────────────────────────────

template <typename T>
class Stack
{
    std::vector<T> data_;

public:
    void push(const T& val)
    {
        data_.push_back(val);
    }
    void pop()
    {
        data_.pop_back();
    }
    const T& top() const
    {
        return data_.back();
    }
    bool empty() const
    {
        return data_.empty();
    }
    size_t size() const
    {
        return data_.size();
    }
};

// Test function template — run the same tests for any T
template <typename T>
bool test_stack_with(T val1, T val2, T val3)
{
    Stack<T> s;

    // Empty state
    assert(s.empty());
    assert(s.size() == 0);

    // Push
    s.push(val1);
    assert(!s.empty());
    assert(s.size() == 1);
    assert(s.top() == val1);

    // Push more
    s.push(val2);
    s.push(val3);
    assert(s.size() == 3);
    assert(s.top() == val3);

    // Pop
    s.pop();
    assert(s.top() == val2);
    assert(s.size() == 2);

    return true;
}

// Compile-time type property tests
template <typename T>
void verify_stack_properties()
{
    static_assert(std::is_default_constructible_v<Stack<T>>, "Stack must be default constructible");
    static_assert(std::is_copy_constructible_v<Stack<T>>, "Stack must be copy constructible");
    static_assert(std::is_move_constructible_v<Stack<T>>, "Stack must be move constructible");
}

void demo_testing()
{
    std::cout << "=== 3. Testing Templates ===\n";

    // Test with multiple types
    assert(test_stack_with<int>(1, 2, 3));
    std::cout << "  Stack<int> — PASSED\n";

    assert(test_stack_with<double>(1.1, 2.2, 3.3));
    std::cout << "  Stack<double> — PASSED\n";

    assert(test_stack_with<std::string>("a", "b", "c"));
    std::cout << "  Stack<string> — PASSED\n";

    // Compile-time property checks
    verify_stack_properties<int>();
    verify_stack_properties<double>();
    verify_stack_properties<std::string>();
    std::cout << "  All type properties verified ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. if constexpr vs SFINAE (readability comparison)
// ──────────────────────────────────────────────────────────────────────────

// BAD: SFINAE soup
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string old_to_string(T val)
{
    return "int:" + std::to_string(val);
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
std::string old_to_string(T val)
{
    return "float:" + std::to_string(val);
}

template <typename T, std::enable_if_t<std::is_same_v<T, std::string>, int> = 0>
std::string old_to_string(T val)
{
    return "str:" + val;
}

// GOOD: if constexpr
template <typename T>
std::string new_to_string(const T& val)
{
    if constexpr (std::is_integral_v<T>)
        return "int:" + std::to_string(val);
    else if constexpr (std::is_floating_point_v<T>)
        return "float:" + std::to_string(val);
    else if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
        return "str:" + val;
    else
        static_assert(!sizeof(T*), "Unsupported type");
}

// BEST: Concepts (C++20)
std::string best_to_string(std::integral auto val)
{
    return "int:" + std::to_string(val);
}
std::string best_to_string(std::floating_point auto val)
{
    return "float:" + std::to_string(val);
}
std::string best_to_string(const std::string& val)
{
    return "str:" + val;
}

void demo_readability()
{
    std::cout << "=== 4. Readability Comparison ===\n";
    std::cout << "  old (SFINAE):      " << old_to_string(42) << "\n";
    std::cout << "  new (if constexpr): " << new_to_string(42) << "\n";
    std::cout << "  best (concepts):    " << best_to_string(42) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Reducing Template Depth
// ──────────────────────────────────────────────────────────────────────────

// BAD: Deep recursion
template <int N>
struct DeepFib
{
    static constexpr long long value = DeepFib<N - 1>::value + DeepFib<N - 2>::value;
};
template <>
struct DeepFib<0>
{
    static constexpr long long value = 0;
};
template <>
struct DeepFib<1>
{
    static constexpr long long value = 1;
};

// GOOD: constexpr function (no template depth issue)
constexpr long long flat_fib(int n)
{
    if (n <= 1)
        return n;
    long long a = 0, b = 1;
    for (int i = 2; i <= n; ++i)
    {
        long long t = a + b;
        a           = b;
        b           = t;
    }
    return b;
}

void demo_depth()
{
    std::cout << "=== 5. Reducing Template Depth ===\n";
    // Both give the same result
    static_assert(DeepFib<20>::value == flat_fib(20));
    std::cout << "  DeepFib<20> = " << DeepFib<20>::value << "\n";
    std::cout << "  flat_fib(20) = " << flat_fib(20) << "\n";
    std::cout << "  flat_fib(50) = " << flat_fib(50) << "\n";
    // DeepFib<50> would hit template depth limits!
    std::cout << "  (constexpr handles n=50 easily, TMP might not)\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 13 — Debugging & Best Practices        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_static_assert();
    demo_extern_template();
    demo_testing();
    demo_readability();
    demo_depth();

    std::cout << "All demos complete.\n";
    return 0;
}
