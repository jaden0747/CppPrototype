// ============================================================================
// Template 11 — Demo: Perfect Forwarding & Reference Collapsing
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. Forwarding References
// ──────────────────────────────────────────────────────────────────────────

// T&& is a forwarding reference when T is deduced
template <typename T>
void show_category(T&&)
{
    if constexpr (std::is_lvalue_reference_v<T>)
        std::cout << "  lvalue reference (T = " << "T&" << ")\n";
    else
        std::cout << "  rvalue reference (T = " << "T" << ")\n";
}

void demo_forwarding_refs()
{
    std::cout << "=== 1. Forwarding References ===\n";

    int x = 42;
    show_category(x);            // lvalue → T = int&
    show_category(42);           // rvalue → T = int
    show_category(std::move(x)); // rvalue → T = int

    std::string s = "hello";
    show_category(s);                 // lvalue → T = string&
    show_category(std::string("hi")); // rvalue → T = string
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Reference Collapsing
// ──────────────────────────────────────────────────────────────────────────
void demo_ref_collapsing()
{
    std::cout << "=== 2. Reference Collapsing ===\n";

    // T& & → T&
    using R1 = std::add_lvalue_reference_t<int&>;
    static_assert(std::is_same_v<R1, int&>);
    std::cout << "  T& & → T& ✓\n";

    // T& && → T&
    using R2 = std::add_rvalue_reference_t<int&>;
    static_assert(std::is_same_v<R2, int&>);
    std::cout << "  T& && → T& ✓\n";

    // T&& & → T&
    using R3 = std::add_lvalue_reference_t<int&&>;
    static_assert(std::is_same_v<R3, int&>);
    std::cout << "  T&& & → T& ✓\n";

    // T&& && → T&&
    using R4 = std::add_rvalue_reference_t<int&&>;
    static_assert(std::is_same_v<R4, int&&>);
    std::cout << "  T&& && → T&& ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. std::forward
// ──────────────────────────────────────────────────────────────────────────

struct Heavy
{
    std::string data;
    Heavy(const std::string& s)
        : data(s)
    {
        std::cout << "  Heavy COPIED: " << data << "\n";
    }
    Heavy(std::string&& s)
        : data(std::move(s))
    {
        std::cout << "  Heavy MOVED: " << data << "\n";
    }
};

// Without forward — always copies
template <typename T>
Heavy make_heavy_bad(T arg)
{
    return Heavy(arg);
}

// With forward — preserves value category
template <typename T>
Heavy make_heavy_good(T&& arg)
{
    return Heavy(std::forward<T>(arg));
}

void demo_forward()
{
    std::cout << "=== 3. std::forward ===\n";

    std::string s = "hello";

    std::cout << "  — Without forward:\n";
    auto h1 = make_heavy_bad(s);                    // copies
    auto h2 = make_heavy_bad(std::string("world")); // copies (arg is lvalue in func!)

    std::cout << "  — With forward:\n";
    auto h3 = make_heavy_good(s);                    // copies (s is lvalue)
    auto h4 = make_heavy_good(std::string("world")); // moves! (rvalue preserved)
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Factory Functions
// ──────────────────────────────────────────────────────────────────────────

// Emplace-style factory
template <typename T, typename... Args>
T construct(Args&&... args)
{
    return T(std::forward<Args>(args)...);
}

// make_unique equivalent
template <typename T, typename... Args>
std::unique_ptr<T> my_make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct Point
{
    double x, y;
    Point(double x, double y)
        : x(x)
        , y(y)
    {
        std::cout << "  Point(" << x << ", " << y << ")\n";
    }
};

// Emplace-back wrapper
template <typename Container, typename... Args>
void emplace(Container& c, Args&&... args)
{
    c.emplace_back(std::forward<Args>(args)...);
}

void demo_factory()
{
    std::cout << "=== 4. Factory Functions ===\n";

    auto p1 = construct<Point>(1.0, 2.0);
    auto p2 = my_make_unique<Point>(3.0, 4.0);

    std::vector<std::string> v;
    emplace(v, "hello");
    emplace(v, 5, 'x'); // string(5, 'x') = "xxxxx"
    std::cout << "  v = [" << v[0] << ", " << v[1] << "]\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Pitfalls
// ──────────────────────────────────────────────────────────────────────────

// DON'T: forward the same argument twice
template <typename T>
void bad_double_forward(T&& arg)
{
    // auto a = std::forward<T>(arg);  // arg might be moved
    // auto b = std::forward<T>(arg);  // UB if arg was moved!
    (void)arg;
}

// DO: use separate arguments or copy first
template <typename T>
void good_multi_use(T&& arg)
{
    auto copy  = arg; // copy first if needed
    auto moved = std::forward<T>(arg);
    (void)copy;
    (void)moved;
}

// const T&& is NOT a forwarding reference
template <typename T>
void not_forwarding(const T&& /*arg*/)
{
    // This ONLY matches rvalues, not lvalues
}

void demo_pitfalls()
{
    std::cout << "=== 5. Pitfalls ===\n";
    std::cout << "  - Don't forward the same arg twice\n";
    std::cout << "  - const T&& is NOT a forwarding reference\n";
    std::cout << "  - Class member access is always an lvalue\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 11 — Perfect Forwarding                ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_forwarding_refs();
    demo_ref_collapsing();
    demo_forward();
    demo_factory();
    demo_pitfalls();

    std::cout << "All demos complete.\n";
    return 0;
}
