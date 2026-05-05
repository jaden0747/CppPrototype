// ============================================================================
// Template 07 — Demo: Standard Concepts & Constrained Templates
// ============================================================================
#include <algorithm>
#include <cassert>
#include <concepts>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Standard Library Concepts
// ──────────────────────────────────────────────────────────────────────────
void demo_std_concepts()
{
    std::cout << "=== 1. Standard Library Concepts ===\n";

    // Core language concepts
    static_assert(std::integral<int>);
    static_assert(std::integral<char>);
    static_assert(!std::integral<double>);
    static_assert(std::floating_point<double>);
    static_assert(std::same_as<int, int>);
    static_assert(!std::same_as<int, long>);
    static_assert(std::convertible_to<int, double>);

    std::cout << "  integral<int> = true ✓\n";
    std::cout << "  floating_point<double> = true ✓\n";

    // Comparison concepts
    static_assert(std::equality_comparable<int>);
    static_assert(std::totally_ordered<int>);
    static_assert(std::totally_ordered<std::string>);

    std::cout << "  totally_ordered<int> = true ✓\n";

    // Object concepts
    static_assert(std::movable<std::string>);
    static_assert(std::copyable<int>);
    static_assert(std::semiregular<int>);
    static_assert(std::regular<int>);

    std::cout << "  regular<int> = true ✓\n";

    // Callable concepts
    auto lambda = [](int x) { return x * 2; };
    static_assert(std::invocable<decltype(lambda), int>);

    std::cout << "  invocable<lambda, int> = true ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Concept Subsumption
// ──────────────────────────────────────────────────────────────────────────

// Overloads from least to most constrained
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

template <std::unsigned_integral T>
std::string classify(T)
{
    return "unsigned_integral";
}

void demo_subsumption()
{
    std::cout << "=== 2. Concept Subsumption ===\n";
    std::cout << "  classify(42) = " << classify(42) << "\n";       // signed_integral
    std::cout << "  classify(42u) = " << classify(42u) << "\n";     // unsigned_integral
    std::cout << "  classify(3.14) = " << classify(3.14) << "\n";   // unconstrained
    std::cout << "  classify('x') = " << classify('x') << "\n";     // signed_integral (char)
    std::cout << "  classify(true) = " << classify(true) << "\n\n"; // unsigned_integral
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Abbreviated Function Templates
// ──────────────────────────────────────────────────────────────────────────

// Terse syntax — each auto is a separate template param
void show(auto val)
{
    std::cout << "  show(auto): " << val << "\n";
}

auto add_num(std::integral auto a, std::floating_point auto b)
{
    return a + b;
}

// Constrained with standard concepts
void print_range(std::ranges::input_range auto&& r)
{
    std::cout << "  [";
    bool first = true;
    for (auto&& elem : r)
    {
        if (!first)
            std::cout << ", ";
        std::cout << elem;
        first = false;
    }
    std::cout << "]\n";
}

void demo_abbreviated()
{
    std::cout << "=== 3. Abbreviated Function Templates ===\n";
    show(42);
    show("hello");
    show(3.14);

    std::cout << "  add_num(1, 2.5) = " << add_num(1, 2.5) << "\n";

    print_range(std::vector<int>{1, 2, 3, 4, 5});
    print_range(std::vector<std::string>{"hello", "world"});
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Constraining Class Templates
// ──────────────────────────────────────────────────────────────────────────

template <std::totally_ordered T>
class SortedList
{
    std::vector<T> data_;

public:
    void insert(const T& val)
    {
        auto it = std::lower_bound(data_.begin(), data_.end(), val);
        data_.insert(it, val);
    }

    size_t size() const
    {
        return data_.size();
    }
    const T& operator[](size_t i) const
    {
        return data_[i];
    }

    // Conditionally enabled member
    T sum() const
        requires std::integral<T> || std::floating_point<T>
    {
        return std::accumulate(data_.begin(), data_.end(), T{});
    }

    void print() const
    {
        std::cout << "  SortedList[";
        for (size_t i = 0; i < data_.size(); ++i)
        {
            if (i)
                std::cout << ", ";
            std::cout << data_[i];
        }
        std::cout << "]\n";
    }
};

void demo_constrained_class()
{
    std::cout << "=== 4. Constraining Class Templates ===\n";

    SortedList<int> si;
    si.insert(5);
    si.insert(1);
    si.insert(3);
    si.insert(2);
    si.insert(4);
    si.print();
    std::cout << "  sum = " << si.sum() << "\n";

    SortedList<std::string> ss;
    ss.insert("banana");
    ss.insert("apple");
    ss.insert("cherry");
    ss.print();
    // ss.sum();  // would not compile — string is not integral or floating_point
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. SFINAE → Concepts migration
// ──────────────────────────────────────────────────────────────────────────

// OLD: SFINAE style
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string old_describe(T val)
{
    return "old integral: " + std::to_string(val);
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
std::string old_describe(T val)
{
    return "old floating: " + std::to_string(val);
}

// NEW: Concepts style
std::string new_describe(std::integral auto val)
{
    return "new integral: " + std::to_string(val);
}

std::string new_describe(std::floating_point auto val)
{
    return "new floating: " + std::to_string(val);
}

void demo_migration()
{
    std::cout << "=== 5. SFINAE → Concepts Migration ===\n";
    std::cout << "  " << old_describe(42) << "\n";
    std::cout << "  " << old_describe(3.14) << "\n";
    std::cout << "  " << new_describe(42) << "\n";
    std::cout << "  " << new_describe(3.14) << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 07 — Standard Concepts & Constraints   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_std_concepts();
    demo_subsumption();
    demo_abbreviated();
    demo_constrained_class();
    demo_migration();

    std::cout << "All demos complete.\n";
    return 0;
}
