// ============================================================================
// Lecture 01 — Demo: Daily Vocabulary (C++11)
// ============================================================================
// Build:  cmake --build build/Debug --target lecture01_demo
// Run:    ./build/Debug/lecture01_demo
// ============================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// 1. auto — the compiler figures out the type for you
// ──────────────────────────────────────────────────────────────────────────
void demo_auto()
{
    std::cout << "=== 1. auto type deduction ===\n";

    auto i = 42;                   // int
    auto d = 3.14;                 // double
    auto s = std::string{"hello"}; // std::string (brace-init!)
    auto p = &i;                   // int*

    // auto drops top-level const:
    const int ci = 100;
    auto      x  = ci; // int (const dropped!)
    // const auto& preserves it:
    const auto& y = ci; // const int&

    std::cout << "  i = " << i << " (int)\n"
              << "  d = " << d << " (double)\n"
              << "  s = " << s << " (std::string)\n"
              << "  *p = " << *p << " (int*)\n"
              << "  x = " << x << " (auto drops const: int)\n"
              << "  y = " << y << " (const auto&: const int&)\n\n";

    // Verify at compile time:
    static_assert(std::is_same<decltype(i), int>::value, "i should be int");
    static_assert(std::is_same<decltype(d), double>::value, "d should be double");
    static_assert(std::is_same<decltype(x), int>::value, "x loses const");
    static_assert(std::is_same<decltype(y), const int&>::value, "y keeps const&");
}

// ──────────────────────────────────────────────────────────────────────────
// 2. Range-based for loop
// ──────────────────────────────────────────────────────────────────────────
void demo_range_for()
{
    std::cout << "=== 2. Range-based for loop ===\n";

    // By value — copies each element (fine for cheap types)
    std::vector<int> nums{10, 20, 30, 40, 50};
    std::cout << "  By value:     ";
    for (auto n : nums)
    {
        std::cout << n << " ";
    }
    std::cout << "\n";

    // By const reference — no copy, read-only
    std::vector<std::string> names{"Alice", "Bob", "Carol"};
    std::cout << "  By const ref: ";
    for (const auto& name : names)
    {
        std::cout << name << " ";
    }
    std::cout << "\n";

    // By reference — modify in-place
    for (auto& n : nums)
    {
        n *= 2;
    }
    std::cout << "  After x2:     ";
    for (const auto& n : nums)
    {
        std::cout << n << " ";
    }
    std::cout << "\n";

    // Works on raw arrays too!
    int arr[] = {100, 200, 300};
    std::cout << "  Raw array:    ";
    for (auto val : arr)
    {
        std::cout << val << " ";
    }
    std::cout << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. nullptr
// ──────────────────────────────────────────────────────────────────────────
void overloaded(int x)
{
    std::cout << "  overloaded(int): " << x << "\n";
}
void overloaded(int* p)
{
    std::cout << "  overloaded(int*): " << p << "\n";
}

void demo_nullptr()
{
    std::cout << "=== 3. nullptr ===\n";

    int* p = nullptr; // Always use nullptr, never NULL or 0

    if (p == nullptr)
    {
        std::cout << "  p is null\n";
    }

    // Overload resolution:
    // overloaded(NULL);    // Would call overloaded(int) — wrong!
    overloaded(nullptr); // Correctly calls overloaded(int*)
    overloaded(42);      // Correctly calls overloaded(int)

    // nullptr as a sentinel
    int  values[] = {1, 2, 3, 4, 5};
    int* found    = nullptr;
    for (auto& v : values)
    {
        if (v == 3)
        {
            found = &v;
            break;
        }
    }
    if (found != nullptr)
    {
        std::cout << "  Found: " << *found << "\n";
    }
    else
    {
        std::cout << "  Not found\n";
    }
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Brace Initialization (Uniform Init)
// ──────────────────────────────────────────────────────────────────────────
struct Point
{
    double x;
    double y;
};

void demo_brace_init()
{
    std::cout << "=== 4. Brace Initialization ===\n";

    // Scalars
    int    a{42};
    double b{3.14};

    // Aggregates
    Point p{1.0, 2.5};

    // Containers with initializer_list
    std::vector<int>           v{1, 2, 3, 4, 5};
    std::map<std::string, int> m{{"Alice", 90}, {"Bob", 85}};

    // Narrowing prevention — these would FAIL to compile:
    // int bad{3.14};       // ERROR: narrowing from double to int
    // uint8_t tiny{256};   // ERROR: narrowing, 256 doesn't fit

    std::cout << "  a = " << a << "\n"
              << "  b = " << b << "\n"
              << "  p = (" << p.x << ", " << p.y << ")\n"
              << "  v = ";
    for (const auto& val : v)
    {
        std::cout << val << " ";
    }
    std::cout << "\n  m = ";
    for (const auto& kv : m)
    {
        std::cout << kv.first << ":" << kv.second << " ";
    }
    std::cout << "\n";

    // GOTCHA: initializer_list vs count constructor
    std::vector<int> ten_zeros(10); // 10 elements, all 0
    std::vector<int> one_ten{10};   // 1 element: value 10
    std::cout << "  vector(10) size = " << ten_zeros.size() << ", vector{10} size = " << one_ten.size() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. Scoped Enums (enum class)
// ──────────────────────────────────────────────────────────────────────────
enum class Color : uint8_t
{
    Red   = 0,
    Green = 1,
    Blue  = 2
};
enum class Fruit
{
    Apple,
    Banana,
    Cherry
};

std::string colorToString(Color c)
{
    switch (c)
    {
    case Color::Red:
        return "Red";
    case Color::Green:
        return "Green";
    case Color::Blue:
        return "Blue";
    }
    return "Unknown";
}

void demo_enum_class()
{
    std::cout << "=== 5. Scoped Enums (enum class) ===\n";

    auto c = Color::Green;
    std::cout << "  Color: " << colorToString(c) << "\n";

    // No implicit conversion to int:
    // int x = Color::Red;  // ERROR — won't compile

    // Explicit conversion is fine:
    int x = static_cast<int>(Color::Red);
    std::cout << "  Color::Red as int: " << x << "\n";

    // Scoped — Color::Red and Fruit::Apple don't collide
    // (Compare with old enum where both would be in global scope)

    // Size control:
    std::cout << "  sizeof(Color) = " << sizeof(Color) << " byte(s)\n";
    std::cout << "  sizeof(Fruit) = " << sizeof(Fruit) << " byte(s)\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Main
// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  Lecture 01 — Daily Vocabulary (C++11)   ║\n"
              << "╚══════════════════════════════════════════╝\n\n";

    demo_auto();
    demo_range_for();
    demo_nullptr();
    demo_brace_init();
    demo_enum_class();

    std::cout << "All demos complete.\n";
    return 0;
}
