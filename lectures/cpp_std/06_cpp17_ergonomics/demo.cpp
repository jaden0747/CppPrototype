// ============================================================================
// Lecture 06 — Demo: C++17 Ergonomics
// ============================================================================
#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Structured Bindings
// ──────────────────────────────────────────────────────────────────────────
void demo_structured_bindings()
{
    std::cout << "=== 1. Structured Bindings ===\n";

    // With std::map
    std::map<std::string, int> scores{{"Alice", 95}, {"Bob", 87}, {"Carol", 92}};
    std::cout << "  Scores:\n";
    for (const auto& [name, score] : scores)
    {
        std::cout << "    " << name << " → " << score << "\n";
    }

    // With tuple
    auto [x, y, z] = std::make_tuple(1.0, 2.5, 3.7);
    std::cout << "  Tuple: (" << x << ", " << y << ", " << z << ")\n";

    // With struct
    struct RGB
    {
        int r, g, b;
    };
    RGB color{255, 128, 0};
    auto [r, g, b] = color;
    std::cout << "  RGB: (" << r << ", " << g << ", " << b << ")\n";

    // With array
    int arr[]       = {10, 20, 30};
    auto [a, b2, c] = arr;
    std::cout << "  Array: " << a << " " << b2 << " " << c << "\n";

    // map::insert returns pair<iterator, bool>
    auto [it, inserted] = scores.insert({"Dave", 88});
    std::cout << "  Insert Dave: " << (inserted ? "new" : "exists") << " → " << it->second << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. If/switch with initializer
// ──────────────────────────────────────────────────────────────────────────
void demo_if_init()
{
    std::cout << "=== 2. If with Initializer ===\n";

    std::map<std::string, int> m{{"key", 42}};

    // Pattern: scope the iterator
    if (auto it = m.find("key"); it != m.end())
    {
        std::cout << "  Found: " << it->second << "\n";
    }

    if (auto it = m.find("missing"); it == m.end())
    {
        std::cout << "  'missing' not found\n";
    }

    // With insert
    if (auto [it, ok] = m.insert({"key", 99}); !ok)
    {
        std::cout << "  'key' already exists with value " << it->second << "\n";
    }

    // With mutex
    std::mutex mtx;
    if (std::lock_guard lock(mtx); true)
    {
        std::cout << "  Locked scope\n";
    }
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Class Template Argument Deduction (CTAD)
// ──────────────────────────────────────────────────────────────────────────
void demo_ctad()
{
    std::cout << "=== 3. CTAD ===\n";

    std::pair p{42, std::string("hello")}; // pair<int, string>
    std::cout << "  pair: (" << p.first << ", " << p.second << ")\n";

    std::vector v{1, 2, 3, 4, 5}; // vector<int>
    std::cout << "  vector size: " << v.size() << "\n";

    std::tuple t{1, 2.0, 'x'}; // tuple<int, double, char>
    std::cout << "  tuple: (" << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << ")\n";

    // lock_guard with CTAD
    std::mutex      mtx;
    std::lock_guard lock(mtx); // deduces lock_guard<mutex>
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Fold Expressions
// ──────────────────────────────────────────────────────────────────────────
template <typename... Args>
auto fold_sum(Args... args)
{
    return (args + ...);
}

template <typename... Args>
bool fold_all(Args... args)
{
    return (args && ...);
}

template <typename... Args>
bool fold_any(Args... args)
{
    return (args || ...);
}

template <typename... Args>
void fold_print(Args... args)
{
    ((std::cout << args << " "), ...) << "\n";
}

void demo_fold_expressions()
{
    std::cout << "=== 4. Fold Expressions ===\n";

    std::cout << "  sum(1,2,3,4,5) = " << fold_sum(1, 2, 3, 4, 5) << "\n";
    std::cout << "  all(true, true, false) = " << std::boolalpha << fold_all(true, true, false) << "\n";
    std::cout << "  any(false, false, true) = " << fold_any(false, false, true) << "\n";
    std::cout << "  print: ";
    fold_print("hello", 42, 3.14, 'x');
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. inline variables
// ──────────────────────────────────────────────────────────────────────────
inline int               global_counter = 0; // safe in headers!
inline const std::string VERSION        = "1.0.0";

void demo_inline_vars()
{
    std::cout << "=== 5. Inline Variables ===\n";
    ++global_counter;
    std::cout << "  counter = " << global_counter << "\n";
    std::cout << "  VERSION = " << VERSION << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. constexpr if
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
std::string stringify(T value)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        return std::to_string(value);
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return value;
    }
    else
    {
        return std::string(value); // assumes convertible to string
    }
}

void demo_constexpr_if()
{
    std::cout << "=== 6. constexpr if ===\n";
    std::cout << "  stringify(42) = \"" << stringify(42) << "\"\n";
    std::cout << "  stringify(3.14) = \"" << stringify(3.14) << "\"\n";
    std::cout << "  stringify(\"hi\") = \"" << stringify("hi") << "\"\n";
    std::cout << "  stringify(string) = \"" << stringify(std::string("yo")) << "\"\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. Nested namespaces
// ──────────────────────────────────────────────────────────────────────────
namespace my::lib::detail
{
inline int magicNumber()
{
    return 42;
}
} // namespace my::lib::detail

void demo_nested_ns()
{
    std::cout << "=== 7. Nested Namespaces ===\n";
    std::cout << "  my::lib::detail::magicNumber() = " << my::lib::detail::magicNumber() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 06 — C++17 Ergonomics                   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_structured_bindings();
    demo_if_init();
    demo_ctad();
    demo_fold_expressions();
    demo_inline_vars();
    demo_constexpr_if();
    demo_nested_ns();

    std::cout << "All demos complete.\n";
    return 0;
}
