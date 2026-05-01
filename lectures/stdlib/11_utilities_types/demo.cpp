// ============================================================================
// Stdlib 11 — Demo: Utilities & Type Support
// ============================================================================
#include <any>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. std::optional
// ──────────────────────────────────────────────────────────────────────────
std::optional<int> find_index(const std::vector<int>& v, int target)
{
    for (std::size_t i = 0; i < v.size(); ++i)
        if (v[i] == target)
            return static_cast<int>(i);
    return std::nullopt;
}

void demo_optional()
{
    std::cout << "=== 1. std::optional ===\n";

    std::vector<int> v{10, 20, 30, 40, 50};

    auto idx = find_index(v, 30);
    if (idx)
    {
        std::cout << "  found 30 at index " << *idx << "\n";
    }

    auto missing = find_index(v, 99);
    std::cout << "  found 99? " << std::boolalpha << missing.has_value() << "\n";

    // value_or
    std::cout << "  value_or(-1) = " << missing.value_or(-1) << "\n";

    // Monadic operations (C++23)
    // auto result = find_index(v, 30)
    //     .transform([](int i) { return i * 2; })
    //     .value_or(0);

    // Optional with string
    std::optional<std::string> name;
    std::cout << "  empty optional: " << name.value_or("(none)") << "\n";
    name = "Alice";
    std::cout << "  after assign: " << *name << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::variant
// ──────────────────────────────────────────────────────────────────────────
using Value = std::variant<int, double, std::string>;

// Visitor pattern with overloaded lambdas
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

void demo_variant()
{
    std::cout << "=== 2. std::variant ===\n";

    Value v = 42;
    std::cout << "  holds int? " << std::boolalpha << std::holds_alternative<int>(v) << "\n";
    std::cout << "  get<int> = " << std::get<int>(v) << "\n";

    v = 3.14;
    std::cout << "  now holds double: " << std::get<double>(v) << "\n";

    v = std::string("hello");
    std::cout << "  now holds string: " << std::get<std::string>(v) << "\n";

    // Visit with overloaded
    std::vector<Value> values{42, 3.14, std::string("world")};
    for (const auto& val : values)
    {
        std::visit(
            overloaded{
                [](int i) { std::cout << "  int: " << i << "\n"; },
                [](double d) { std::cout << "  double: " << d << "\n"; },
                [](const std::string& s) { std::cout << "  string: \"" << s << "\"\n"; }},
            val);
    }

    // get_if — returns pointer or null
    if (auto* p = std::get_if<int>(&values[0]))
    {
        std::cout << "  get_if<int>: " << *p << "\n";
    }

    // index()
    std::cout << "  values[0].index() = " << values[0].index() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::any
// ──────────────────────────────────────────────────────────────────────────
void demo_any()
{
    std::cout << "=== 3. std::any ===\n";

    std::any a = 42;
    std::cout << "  type: " << a.type().name() << "  val: " << std::any_cast<int>(a) << "\n";

    a = std::string("hello");
    std::cout << "  type: " << a.type().name() << "  val: " << std::any_cast<std::string>(a) << "\n";

    // Bad cast throws
    try
    {
        [[maybe_unused]] auto x = std::any_cast<double>(a);
    }
    catch (const std::bad_any_cast& e)
    {
        std::cout << "  bad_any_cast: " << e.what() << "\n";
    }

    // has_value / reset
    std::cout << "  has_value? " << std::boolalpha << a.has_value() << "\n";
    a.reset();
    std::cout << "  after reset: " << a.has_value() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::pair and std::tuple
// ──────────────────────────────────────────────────────────────────────────
void demo_pair_tuple()
{
    std::cout << "=== 4. pair & tuple ===\n";

    // Pair
    auto p = std::make_pair("key", 42);
    std::cout << "  pair: (" << p.first << ", " << p.second << ")\n";

    // Structured bindings
    auto [k, v] = p;
    std::cout << "  destructured: k=\"" << k << "\" v=" << v << "\n";

    // Tuple
    auto t = std::make_tuple(1, "hello", 3.14);
    std::cout << "  tuple: (" << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << ")\n";

    // Structured bindings for tuple
    auto [a, b, c] = t;
    std::cout << "  destructured: " << a << " " << b << " " << c << "\n";

    // tuple_size
    std::cout << "  tuple_size = " << std::tuple_size_v<decltype(t)> << "\n";

    // tie for assignment
    int         x;
    std::string y;
    double      z;
    std::tie(x, y, z) = t;
    std::cout << "  tie: x=" << x << " y=" << y << " z=" << z << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Type traits
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
void describe_type()
{
    std::cout << "  is_integral:       " << std::is_integral_v<T> << "\n";
    std::cout << "  is_floating_point: " << std::is_floating_point_v<T> << "\n";
    std::cout << "  is_arithmetic:     " << std::is_arithmetic_v<T> << "\n";
    std::cout << "  is_class:          " << std::is_class_v<T> << "\n";
}

void demo_type_traits()
{
    std::cout << "=== 5. Type Traits ===\n";

    std::cout << "  -- int --\n";
    describe_type<int>();

    std::cout << "  -- double --\n";
    describe_type<double>();

    std::cout << "  -- std::string --\n";
    describe_type<std::string>();

    // is_same
    std::cout << "\n  is_same<int, int>: " << std::is_same_v<int, int> << "\n";
    std::cout << "  is_same<int, long>: " << std::is_same_v<int, long> << "\n";

    // conditional
    using T = std::conditional_t<sizeof(int) >= 4, int, long>;
    std::cout << "  conditional_t: sizeof=" << sizeof(T) << "\n";

    // decay
    std::cout << "  is_same<decay_t<const int&>, int>: " << std::is_same_v<std::decay_t<const int&>, int> << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. std::function, std::invoke, std::bind_front
// ──────────────────────────────────────────────────────────────────────────
int add(int a, int b)
{
    return a + b;
}

struct Multiplier
{
    int factor;
    int operator()(int x) const
    {
        return x * factor;
    }
    int multiply(int x) const
    {
        return x * factor;
    }
};

void demo_functional()
{
    std::cout << "=== 6. function / invoke / bind_front ===\n";

    // std::function — type-erased callable
    std::function<int(int, int)> fn = add;
    std::cout << "  function(add)(3,4) = " << fn(3, 4) << "\n";

    fn = [](int a, int b) { return a * b; };
    std::cout << "  function(lambda)(3,4) = " << fn(3, 4) << "\n";

    // std::invoke — call anything uniformly
    std::cout << "  invoke(add, 5, 6) = " << std::invoke(add, 5, 6) << "\n";

    Multiplier m{3};
    std::cout << "  invoke(functor, 7) = " << std::invoke(m, 7) << "\n";
    std::cout << "  invoke(&multiply, m, 7) = " << std::invoke(&Multiplier::multiply, m, 7) << "\n";

    // std::bind_front (C++20) — partial application
    auto add5 = std::bind_front(add, 5);
    std::cout << "  bind_front(add, 5)(10) = " << add5(10) << "\n";

    // apply — unpack tuple as function args
    auto args = std::make_tuple(3, 4);
    std::cout << "  apply(add, {3,4}) = " << std::apply(add, args) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 11 — Utilities & Type Support             ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_optional();
    demo_variant();
    demo_any();
    demo_pair_tuple();
    demo_type_traits();
    demo_functional();

    std::cout << "All demos complete.\n";
    return 0;
}
