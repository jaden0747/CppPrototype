// ============================================================================
// Template 03 — Demo: Variadic Templates
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Parameter Packs & Basic Expansion
// ──────────────────────────────────────────────────────────────────────────
template <typename... Ts>
constexpr size_t count_types()
{
    return sizeof...(Ts);
}

template <typename... Ts>
constexpr size_t count_args(Ts... /*args*/)
{
    return sizeof...(Ts);
}

void demo_sizeof()
{
    std::cout << "=== 1. sizeof... ===\n";
    std::cout << "  count_types<int,double,char>() = " << count_types<int, double, char>() << "\n";
    std::cout << "  count_args(1, 2.0, 'x') = " << count_args(1, 2.0, 'x') << "\n";
    std::cout << "  count_args() = " << count_args() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Recursive Unpacking (pre-C++17 style)
// ──────────────────────────────────────────────────────────────────────────
void print_recursive()
{
    std::cout << "\n";
}

template <typename T, typename... Rest>
void print_recursive(T first, Rest... rest)
{
    std::cout << first;
    if constexpr (sizeof...(rest) > 0)
        std::cout << ", ";
    print_recursive(rest...);
}

void demo_recursive()
{
    std::cout << "=== 2. Recursive Unpacking ===\n  ";
    print_recursive(1, 2.5, "hello", 'x');
    std::cout << "  ";
    print_recursive(42);
    std::cout << "  ";
    print_recursive();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Fold Expressions (C++17)
// ──────────────────────────────────────────────────────────────────────────
template <typename... Ts>
auto sum(Ts... args)
{
    return (args + ...);
}

template <typename... Ts>
auto product(Ts... args)
{
    return (args * ...);
}

template <typename... Ts>
bool all_true(Ts... args)
{
    return (args && ...);
}

template <typename... Ts>
bool any_true(Ts... args)
{
    return (args || ...);
}

// Fold with comma operator for side effects
template <typename... Ts>
void print_fold(Ts... args)
{
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}

// Left fold with initial value
template <typename... Ts>
auto sum_from_zero(Ts... args)
{
    return (0 + ... + args);
}

void demo_folds()
{
    std::cout << "=== 3. Fold Expressions ===\n";
    std::cout << "  sum(1,2,3,4) = " << sum(1, 2, 3, 4) << "\n";
    std::cout << "  product(1,2,3,4) = " << product(1, 2, 3, 4) << "\n";
    std::cout << "  all_true(true,true,false) = " << std::boolalpha << all_true(true, true, false) << "\n";
    std::cout << "  any_true(false,false,true) = " << any_true(false, false, true) << "\n";
    std::cout << "  print_fold: ";
    print_fold(1, "hello", 3.14, 'z');
    std::cout << "  sum_from_zero() = " << sum_from_zero() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Pack Expansion in Various Contexts
// ──────────────────────────────────────────────────────────────────────────

// Expand in template arguments
template <typename... Ts>
auto make_tuple_val(Ts... args)
{
    return std::tuple<Ts...>(args...);
}

// Expand with transformation
template <typename... Ts>
auto double_all(Ts... args)
{
    return std::make_tuple((args * 2)...);
}

// Expand in base class list
template <typename... Bases>
struct MultiDerived : Bases...
{
    using Bases::operator()...; // bring all call operators (C++17)
};

struct PrintInt
{
    void operator()(int x) const
    {
        std::cout << "int: " << x << "\n";
    }
};
struct PrintStr
{
    void operator()(const std::string& s) const
    {
        std::cout << "str: " << s << "\n";
    }
};

void demo_expansion()
{
    std::cout << "=== 4. Pack Expansion Contexts ===\n";

    auto t = make_tuple_val(1, 2.5, std::string("hi"));
    std::cout << "  tuple: (" << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << ")\n";

    auto d = double_all(1, 2, 3);
    std::cout << "  doubled: (" << std::get<0>(d) << ", " << std::get<1>(d) << ", " << std::get<2>(d) << ")\n";

    // Overloaded pattern (like std::visit helper)
    MultiDerived<PrintInt, PrintStr> printer;
    std::cout << "  ";
    printer(42);
    std::cout << "  ";
    printer(std::string("hello"));
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Index Sequences
// ──────────────────────────────────────────────────────────────────────────
template <typename Tuple, size_t... Is>
void print_tuple_impl(const Tuple& t, std::index_sequence<Is...>)
{
    ((std::cout << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
}

template <typename... Ts>
void print_tuple(const std::tuple<Ts...>& t)
{
    std::cout << "(";
    print_tuple_impl(t, std::index_sequence_for<Ts...>{});
    std::cout << ")\n";
}

void demo_index_sequence()
{
    std::cout << "=== 5. Index Sequences ===\n";
    auto t = std::make_tuple(1, "hello", 3.14, true);
    std::cout << "  ";
    print_tuple(t);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 03 — Variadic Templates                ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_sizeof();
    demo_recursive();
    demo_folds();
    demo_expansion();
    demo_index_sequence();

    std::cout << "All demos complete.\n";
    return 0;
}
