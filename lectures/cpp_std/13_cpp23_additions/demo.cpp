// ============================================================================
// Lecture 13 — Demo: C++23 Additions
// ============================================================================
// Note: C++23 features require very recent compilers (GCC 13+, Clang 17+,
// MSVC 2022 17.7+). Some features may not be available on all platforms.
// Compile with: -std=c++23
// ============================================================================
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// Check for C++23 features
#if __cplusplus >= 202302L
#define HAS_CPP23 1
#else
#define HAS_CPP23 0
#endif

#if __has_include(<expected>)
#include <expected>
#define HAS_EXPECTED 1
#else
#define HAS_EXPECTED 0
#endif

#if __has_include(<print>)
#include <print>
#define HAS_PRINT 1
#else
#define HAS_PRINT 0
#endif

// ──────────────────────────────────────────────────────────────────────────
// 1. Deducing this
// ──────────────────────────────────────────────────────────────────────────

// Counter moved to file scope — template members in local classes are not
// permitted by the standard (and rejected by most compilers).
#if HAS_CPP23
struct Counter
{
    int count = 0;
    template <typename Self>
    auto& increment(this Self& self)
    {
        ++self.count;
        return self;
    }
};

// Recursive lambda
void demo_deducing_this()
{
    std::cout << "=== 1. Deducing this ===\n";

    // Recursive lambda without Y-combinator!
    auto fib = [](this auto self, int n) -> int
    {
        if (n <= 1)
            return n;
        return self(n - 1) + self(n - 2);
    };
    std::cout << "  fib(10) = " << fib(10) << "\n";

    Counter c;
    c.increment().increment().increment();
    std::cout << "  Counter after 3 increments: " << c.count << "\n\n";
}

#else

void demo_deducing_this()
{
    std::cout << "=== 1. Deducing this ===\n";
    std::cout << "  (Requires C++23 compiler support)\n";

    // Conceptual example:
    // auto factorial = [](this auto self, int n) -> int {
    //     return n <= 1 ? 1 : n * self(n - 1);
    // };
    std::cout << "  Conceptual: recursive lambda factorial(5) = 120\n\n";
}

#endif

// ──────────────────────────────────────────────────────────────────────────
// 2. std::expected
// ──────────────────────────────────────────────────────────────────────────
#if HAS_EXPECTED

enum class ParseError
{
    EmptyInput,
    InvalidFormat,
    Overflow
};

std::string to_string(ParseError e)
{
    switch (e)
    {
    case ParseError::EmptyInput:
        return "empty input";
    case ParseError::InvalidFormat:
        return "invalid format";
    case ParseError::Overflow:
        return "overflow";
    }
    return "unknown";
}

std::expected<int, ParseError> parse_int(std::string_view sv)
{
    if (sv.empty())
        return std::unexpected(ParseError::EmptyInput);
    try
    {
        size_t pos;
        long   val = std::stol(std::string(sv), &pos);
        if (pos != sv.size())
            return std::unexpected(ParseError::InvalidFormat);
        if (val > INT_MAX || val < INT_MIN)
            return std::unexpected(ParseError::Overflow);
        return static_cast<int>(val);
    }
    catch (...)
    {
        return std::unexpected(ParseError::InvalidFormat);
    }
}

void demo_expected()
{
    std::cout << "=== 2. std::expected ===\n";

    auto r1 = parse_int("42");
    std::cout << "  parse_int(\"42\"): " << (r1 ? std::to_string(*r1) : "error") << "\n";

    auto r2 = parse_int("abc");
    std::cout << "  parse_int(\"abc\"): " << (r2 ? "ok" : to_string(r2.error())) << "\n";

    auto r3 = parse_int("");
    std::cout << "  parse_int(\"\"): " << (r3 ? "ok" : to_string(r3.error())) << "\n";

    // Monadic operations
    auto doubled = parse_int("21").transform([](int x) { return x * 2; });
    std::cout << "  parse_int(\"21\").transform(*2): " << *doubled << "\n\n";
}

#else

void demo_expected()
{
    std::cout << "=== 2. std::expected ===\n";
    std::cout << "  (Requires <expected> header — GCC 12+, Clang 16+, MSVC 2022)\n\n";
}

#endif

// ──────────────────────────────────────────────────────────────────────────
// 3. std::print / std::println
// ──────────────────────────────────────────────────────────────────────────
void demo_print()
{
    std::cout << "=== 3. std::print / std::println ===\n";

#if HAS_PRINT
    std::println("  Hello from std::println!");
    std::println("  x = {}, y = {}", 3, 4);
    std::println("  {:>10.2f}", 3.14159);
    std::print("  print without newline → ");
    std::println("done!");
#else
    std::cout << "  (Requires <print> header — GCC 14+, MSVC 2022 17.7+)\n";
    std::cout << "  Conceptual: std::println(\"Hello, {}!\", name);\n";
#endif
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. Multidimensional subscript operator
// ──────────────────────────────────────────────────────────────────────────
struct Matrix3x3
{
    double data[3][3] = {};

#if HAS_CPP23
    double& operator[](size_t row, size_t col)
    {
        return data[row][col];
    }
    const double& operator[](size_t row, size_t col) const
    {
        return data[row][col];
    }
#endif

    // Fallback for older compilers
    double& at(size_t row, size_t col)
    {
        return data[row][col];
    }
};

void demo_multidim_subscript()
{
    std::cout << "=== 4. Multidimensional operator[] ===\n";

    Matrix3x3 m;
#if HAS_CPP23
    m[0, 0] = 1.0;
    m[1, 1] = 2.0;
    m[2, 2] = 3.0;
    std::cout << "  m[0,0] = " << m[0, 0] << "\n";
    std::cout << "  m[1,1] = " << m[1, 1] << "\n";
    std::cout << "  m[2,2] = " << m[2, 2] << "\n";
#else
    m.at(0, 0) = 1.0;
    m.at(1, 1) = 2.0;
    m.at(2, 2) = 3.0;
    std::cout << "  (Using .at() fallback — m[i,j] requires C++23)\n";
    std::cout << "  m.at(0,0) = " << m.at(0, 0) << "\n";
    std::cout << "  m.at(1,1) = " << m.at(1, 1) << "\n";
    std::cout << "  m.at(2,2) = " << m.at(2, 2) << "\n";
#endif
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. ranges::to (conceptual if not available)
// ──────────────────────────────────────────────────────────────────────────
void demo_ranges_to()
{
    std::cout << "=== 5. std::ranges::to ===\n";

#if __cpp_lib_ranges_to_container >= 202202L
    auto vec =
        std::views::iota(1, 11) | std::views::filter([](int x) { return x % 2 == 0; }) | std::ranges::to<std::vector>();
    std::cout << "  evens: ";
    for (int x : vec)
        std::cout << x << " ";
    std::cout << "\n";
#else
    std::cout << "  (Requires C++23 ranges::to support)\n";
    std::cout << "  Conceptual: views::iota(1,11) | filter(even) | ranges::to<vector>()\n";
    std::cout << "  Result would be: 2 4 6 8 10\n";
#endif
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. Other C++23 features
// ──────────────────────────────────────────────────────────────────────────
void demo_other_features()
{
    std::cout << "=== 6. Other C++23 Features ===\n";

    // if consteval (conceptual)
    std::cout << "  if consteval: compile-time vs runtime branching\n";
    std::cout << "  std::flat_map: cache-friendly sorted container\n";
    std::cout << "  std::generator: standard coroutine generator\n";
    std::cout << "  std::views::zip: iterate multiple ranges together\n";
    std::cout << "  std::views::chunk: split range into chunks\n";
    std::cout << "  std::views::slide: sliding window view\n";
    std::cout << "  std::stacktrace: programmatic stack traces\n";
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 13 — C++23 Additions                    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_deducing_this();
    demo_expected();
    demo_print();
    demo_multidim_subscript();
    demo_ranges_to();
    demo_other_features();

    std::cout << "All demos complete.\n";
    std::cout << "\n🎉 Congratulations! You've completed all 13 lectures!\n";
    return 0;
}
