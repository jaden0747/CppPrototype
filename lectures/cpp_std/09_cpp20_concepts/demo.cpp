// ============================================================================
// Lecture 09 — Demo: C++20 Concepts
// ============================================================================
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Defining concepts
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept Numeric = std::is_arithmetic_v<T>;

template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template <typename T>
concept Printable = requires(std::ostream& os, T val) {
    { os << val } -> std::same_as<std::ostream&>;
};

template <typename T>
concept Container = requires(T c) {
    c.begin();
    c.end();
    { c.size() } -> std::convertible_to<std::size_t>;
    typename T::value_type;
};

template <typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

// ──────────────────────────────────────────────────────────────────────────
// 2. Four syntax forms
// ──────────────────────────────────────────────────────────────────────────

// Form 1: requires clause
template <typename T>
    requires Numeric<T>
T add_v1(T a, T b)
{
    return a + b;
}

// Form 2: constrained parameter
template <Numeric T>
T add_v2(T a, T b)
{
    return a + b;
}

// Form 3: trailing requires
template <typename T>
T add_v3(T a, T b)
    requires Numeric<T>
{
    return a + b;
}

// Form 4: terse (abbreviated)
auto add_v4(Numeric auto a, Numeric auto b)
{
    return a + b;
}

// ──────────────────────────────────────────────────────────────────────────
// 3. requires expressions
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept Stringifiable = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
};

template <typename T>
concept SmallTrivial = requires {
    requires sizeof(T) <= 16;
    requires std::is_trivially_copyable_v<T>;
};

struct Point
{
    double      x, y;
    std::string to_string() const
    {
        return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }
};

static_assert(Stringifiable<Point>);
static_assert(SmallTrivial<Point>);

// ──────────────────────────────────────────────────────────────────────────
// 4. Concept subsumption / overload resolution
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
std::string classify(T)
{
    return "unconstrained";
}

template <std::integral T>
std::string classify(T)
{
    return "integral";
}

template <std::signed_integral T>
std::string classify(T)
{
    return "signed_integral";
}

template <std::floating_point T>
std::string classify(T)
{
    return "floating_point";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Practical: constrained algorithms
// ──────────────────────────────────────────────────────────────────────────
template <Container C>
void print_container(const C& c, std::string_view label)
{
    std::cout << "  " << label << ": [";
    bool first = true;
    for (const auto& elem : c)
    {
        if (!first)
            std::cout << ", ";
        std::cout << elem;
        first = false;
    }
    std::cout << "] (size=" << c.size() << ")\n";
}

template <Container C>
    requires std::totally_ordered<typename C::value_type>
auto find_max(const C& c)
{
    auto it   = c.begin();
    auto best = *it;
    while (++it != c.end())
    {
        if (*it > best)
            best = *it;
    }
    return best;
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Composing concepts
// ──────────────────────────────────────────────────────────────────────────
template <typename T>
concept PrintableNumeric = Numeric<T> && Printable<T>;

void print_double(PrintableNumeric auto x)
{
    std::cout << "  doubled: " << (x + x) << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 09 — C++20 Concepts                     ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    // Demo 1: Four syntax forms
    std::cout << "=== 1. Four Syntax Forms ===\n";
    std::cout << "  add_v1(3, 4) = " << add_v1(3, 4) << "\n";
    std::cout << "  add_v2(1.5, 2.5) = " << add_v2(1.5, 2.5) << "\n";
    std::cout << "  add_v3(10, 20) = " << add_v3(10, 20) << "\n";
    std::cout << "  add_v4(100, 200) = " << add_v4(100, 200) << "\n\n";

    // Demo 2: Custom concepts
    std::cout << "=== 2. Custom Concepts ===\n";
    Point p{3.0, 4.0};
    std::cout << "  Point::to_string() = " << p.to_string() << "\n";
    std::cout << "  Stringifiable<Point>: true\n";
    std::cout << "  SmallTrivial<Point>: true\n";
    std::cout << "  Hashable<int>: " << std::boolalpha << Hashable<int> << "\n";
    std::cout << "  Hashable<Point>: " << Hashable<Point> << "\n\n";

    // Demo 3: Subsumption
    std::cout << "=== 3. Concept Subsumption ===\n";
    std::cout << "  classify(42) = " << classify(42) << "\n";
    std::cout << "  classify(42u) = " << classify(42u) << "\n";
    std::cout << "  classify(3.14) = " << classify(3.14) << "\n";
    std::cout << "  classify(\"hi\") = " << classify("hi") << "\n\n";

    // Demo 4: Constrained algorithms
    std::cout << "=== 4. Constrained Algorithms ===\n";
    std::vector<int> nums{5, 2, 8, 1, 9, 3};
    print_container(nums, "nums");
    std::cout << "  max = " << find_max(nums) << "\n";

    std::vector<std::string> words{"banana", "apple", "cherry"};
    print_container(words, "words");
    std::cout << "  max = " << find_max(words) << "\n\n";

    // Demo 5: Composed concepts
    std::cout << "=== 5. Composed Concepts ===\n";
    print_double(21);
    print_double(1.5);

    std::cout << "\nAll demos complete.\n";
    return 0;
}
