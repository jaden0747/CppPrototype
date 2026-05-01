// ============================================================================
// Lecture 08 — Demo: C++17 Templates & Filesystem
// ============================================================================
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace fs = std::filesystem;

// ──────────────────────────────────────────────────────────────────────────
// 1. if constexpr — deep dive
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
std::string to_debug_string(const T& value)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        return "[num:" + std::to_string(value) + "]";
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return "[str:\"" + value + "\"]";
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        if (value)
            return "[ptr:" + to_debug_string(*value) + "]";
        return "[ptr:null]";
    }
    else
    {
        return "[unknown]";
    }
}

// Recursive variadic with if constexpr
template <typename T, typename... Rest>
void print_all(T first, Rest... rest)
{
    std::cout << first;
    if constexpr (sizeof...(rest) > 0)
    {
        std::cout << ", ";
        print_all(rest...);
    }
}

void demo_if_constexpr()
{
    std::cout << "=== 1. if constexpr ===\n";
    std::cout << "  " << to_debug_string(42) << "\n";
    std::cout << "  " << to_debug_string(3.14) << "\n";
    std::cout << "  " << to_debug_string(std::string("hello")) << "\n";
    int x = 7;
    std::cout << "  " << to_debug_string(&x) << "\n";

    std::cout << "  print_all: ";
    print_all(1, "two", 3.0, 'x');
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. template<auto>
// ──────────────────────────────────────────────────────────────────────────
template <auto Value>
struct Constant
{
    static constexpr auto value = Value;
    using type                  = decltype(Value);
};

template <auto N>
constexpr auto doubled = N * 2;

template <auto Func>
decltype(auto) invoke_static()
{
    return Func();
}

int get_answer()
{
    return 42;
}

void demo_template_auto()
{
    std::cout << "=== 2. template<auto> ===\n";
    std::cout << "  Constant<42>::value = " << Constant<42>::value << "\n";
    std::cout << "  Constant<'A'>::value = " << Constant<'A'>::value << "\n";
    std::cout << "  doubled<21> = " << doubled<21> << "\n";
    std::cout << "  invoke_static<get_answer>() = " << invoke_static<get_answer>() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Type traits upgrades
// ──────────────────────────────────────────────────────────────────────────
template <typename... Args>
constexpr bool all_integral = std::conjunction_v<std::is_integral<Args>...>;

template <typename... Args>
constexpr bool any_floating = std::disjunction_v<std::is_floating_point<Args>...>;

void demo_traits()
{
    std::cout << "=== 3. Type Traits Upgrades ===\n";
    std::cout << "  all_integral<int,long,char> = " << std::boolalpha << all_integral<int, long, char> << "\n";
    std::cout << "  all_integral<int,double> = " << all_integral<int, double> << "\n";
    std::cout << "  any_floating<int,double,char> = " << any_floating<int, double, char> << "\n";

    // is_invocable
    auto lambda = [](int x) { return x * 2; };
    std::cout << "  lambda invocable with int: " << std::is_invocable_v<decltype(lambda), int> << "\n";
    std::cout << "  lambda invocable with string: " << std::is_invocable_v<decltype(lambda), std::string> << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. std::filesystem
// ──────────────────────────────────────────────────────────────────────────
void demo_filesystem()
{
    std::cout << "=== 4. std::filesystem ===\n";

    // Path operations
    fs::path p = "/usr/local/bin/program.sh";
    std::cout << "  path: " << p << "\n";
    std::cout << "  filename: " << p.filename() << "\n";
    std::cout << "  stem: " << p.stem() << "\n";
    std::cout << "  extension: " << p.extension() << "\n";
    std::cout << "  parent: " << p.parent_path() << "\n";

    // Path concatenation
    fs::path base = "/home/user";
    auto     full = base / "docs" / "file.txt";
    std::cout << "  concatenated: " << full << "\n";

    // Current directory info
    auto cwd = fs::current_path();
    std::cout << "  cwd: " << cwd << "\n";

    // List current directory (first 5 entries)
    std::cout << "  First files in cwd:\n";
    int count = 0;
    for (auto& entry : fs::directory_iterator(cwd))
    {
        if (++count > 5)
            break;
        std::cout << "    " << entry.path().filename() << (entry.is_directory() ? "/" : "") << "\n";
    }

    // Create temp structure and clean up
    fs::path tmp = fs::temp_directory_path() / "lecture08_demo";
    fs::create_directories(tmp / "sub1" / "sub2");
    {
        std::ofstream(tmp / "test.txt") << "hello filesystem";
    }
    std::cout << "  Created: " << tmp << "\n";
    std::cout << "  test.txt size: " << fs::file_size(tmp / "test.txt") << " bytes\n";

    // Cleanup
    fs::remove_all(tmp);
    std::cout << "  Cleaned up temp dir\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. std::apply and std::invoke
// ──────────────────────────────────────────────────────────────────────────
struct Calculator
{
    int base;
    int add(int x) const
    {
        return base + x;
    }
};

void demo_apply_invoke()
{
    std::cout << "=== 5. std::apply & std::invoke ===\n";

    // std::apply: unpack tuple into function call
    auto args = std::make_tuple(3, 4);
    auto sum  = std::apply([](int a, int b) { return a + b; }, args);
    std::cout << "  apply(add, {3,4}) = " << sum << "\n";

    // std::invoke: uniform callable
    Calculator calc{100};
    auto       result = std::invoke(&Calculator::add, calc, 5);
    std::cout << "  invoke(Calculator::add, {100}, 5) = " << result << "\n";

    // invoke with member variable
    auto base = std::invoke(&Calculator::base, calc);
    std::cout << "  invoke(Calculator::base) = " << base << "\n";

    // invoke with lambda
    auto doubled = std::invoke([](int x) { return x * 2; }, 21);
    std::cout << "  invoke(lambda, 21) = " << doubled << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 08 — C++17 Templates & Filesystem       ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_if_constexpr();
    demo_template_auto();
    demo_traits();
    demo_filesystem();
    demo_apply_invoke();

    std::cout << "All demos complete.\n";
    return 0;
}
