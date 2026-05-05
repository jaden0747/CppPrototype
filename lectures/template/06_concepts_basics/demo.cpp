// ============================================================================
// Template 06 — Demo: Concepts — Defining & Using (C++20)
// ============================================================================
#include <cassert>
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Defining Concepts
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template <typename T>
concept Printable = requires(T a) {
    { std::cout << a } -> std::same_as<std::ostream&>;
};

template <typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
};

void demo_define_concepts()
{
    std::cout << "=== 1. Defining Concepts ===\n";
    std::cout << "  Numeric<int> = " << std::boolalpha << Numeric<int> << "\n";
    std::cout << "  Numeric<string> = " << Numeric<std::string> << "\n";
    std::cout << "  Addable<int> = " << Addable<int> << "\n";
    std::cout << "  Addable<string> = " << Addable<std::string> << "\n";
    std::cout << "  Printable<int> = " << Printable<int> << "\n";
    std::cout << "  Hashable<int> = " << Hashable<int> << "\n";
    std::cout << "  Hashable<vector<int>> = " << Hashable<std::vector<int>> << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Three Ways to Use Concepts
// ──────────────────────────────────────────────────────────────────────────

// Way 1: Constrained template parameter
template <Numeric T>
T square(T x)
{
    return x * x;
}

// Way 2: requires clause
template <typename T>
    requires Numeric<T>
T cube(T x)
{
    return x * x * x;
}

// Way 3: Trailing requires
template <typename T>
T half(T x)
    requires Numeric<T>
{
    return x / 2;
}

// Way 4: Abbreviated function template (terse syntax)
void print_value(Printable auto const& val)
{
    std::cout << "  " << val << "\n";
}

void demo_using_concepts()
{
    std::cout << "=== 2. Using Concepts ===\n";
    std::cout << "  square(5) = " << square(5) << "\n";
    std::cout << "  cube(3) = " << cube(3) << "\n";
    std::cout << "  half(10) = " << half(10) << "\n";
    std::cout << "  half(7.0) = " << half(7.0) << "\n";
    print_value(42);
    print_value("hello");
    print_value(3.14);
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. requires Expressions
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept Container = requires(T c) {
    typename T::value_type;
    typename T::iterator;
    { c.begin() } -> std::same_as<typename T::iterator>;
    { c.end() } -> std::same_as<typename T::iterator>;
    { c.size() } -> std::convertible_to<size_t>;
    { c.empty() } -> std::convertible_to<bool>;
};

template <typename T>
concept Indexable = Container<T> && requires(T c, size_t i) {
    { c[i] } -> std::same_as<typename T::reference>;
};

void demo_requires_expression()
{
    std::cout << "=== 3. requires Expressions ===\n";
    std::cout << "  Container<vector<int>> = " << std::boolalpha << Container<std::vector<int>> << "\n";
    std::cout << "  Container<int> = " << Container<int> << "\n";
    std::cout << "  Container<string> = " << Container<std::string> << "\n";
    std::cout << "  Indexable<vector<int>> = " << Indexable<std::vector<int>> << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Compound & Nested Requirements
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept Comparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
    { a <= b } -> std::convertible_to<bool>;
    { a >= b } -> std::convertible_to<bool>;
};

template <typename T>
concept SmallComparable = Comparable<T> && requires {
    requires sizeof(T) <= 16;
    requires std::is_copy_constructible_v<T>;
};

// Use in a function
template <SmallComparable T>
T clamp(T val, T lo, T hi)
{
    return val < lo ? lo : (val > hi ? hi : val);
}

void demo_compound()
{
    std::cout << "=== 4. Compound & Nested Requirements ===\n";
    std::cout << "  Comparable<int> = " << std::boolalpha << Comparable<int> << "\n";
    std::cout << "  SmallComparable<int> = " << SmallComparable<int> << "\n";
    std::cout << "  clamp(15, 0, 10) = " << clamp(15, 0, 10) << "\n";
    std::cout << "  clamp(5, 0, 10) = " << clamp(5, 0, 10) << "\n";
    std::cout << "  clamp(-3, 0, 10) = " << clamp(-3, 0, 10) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Combining Concepts
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept NumericContainer = Container<T> && Numeric<typename T::value_type>;

template <NumericContainer C>
auto sum(const C& c)
{
    typename C::value_type total{};
    for (const auto& elem : c)
        total += elem;
    return total;
}

template <typename T>
concept StringLike = std::same_as<std::decay_t<T>, std::string> || std::same_as<std::decay_t<T>, std::string_view> ||
                     std::same_as<std::decay_t<T>, const char*>;

void greet(StringLike auto name)
{
    std::cout << "  Hello, " << name << "!\n";
}

void demo_combining()
{
    std::cout << "=== 5. Combining Concepts ===\n";
    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "  sum({1,2,3,4,5}) = " << sum(v) << "\n";

    std::vector<double> vd{1.1, 2.2, 3.3};
    std::cout << "  sum({1.1,2.2,3.3}) = " << sum(vd) << "\n";

    greet(std::string("Alice"));
    greet("Bob");
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 06 — Concepts: Defining & Using        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_define_concepts();
    demo_using_concepts();
    demo_requires_expression();
    demo_compound();
    demo_combining();

    std::cout << "All demos complete.\n";
    return 0;
}
