// ============================================================================
// Lecture 12 — Exercises: C++20 Utilities
// ============================================================================
#include <algorithm>
#include <cassert>
#include <compare>
#include <iostream>
#include <source_location>
#include <span>
#include <string>
#include <thread>
#include <vector>

#if __has_include(<format>)
#include <format>
#define HAS_FORMAT 1
#else
#define HAS_FORMAT 0
#endif

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Spaceship operator
// Create a `Fraction` struct (numerator, denominator) with <=>
// that compares by actual numeric value (cross-multiply).
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// struct Fraction {
//     int num, den;
//     std::strong_ordering operator<=>(const Fraction& other) const { ??? }
//     bool operator==(const Fraction& other) const { ??? }
// };

void exercise_spaceship()
{
    // Fraction a{1, 2}, b{2, 4}, c{3, 4};
    // assert(a == b);       // 1/2 == 2/4
    // assert(a < c);        // 1/2 < 3/4
    // assert(c > a);
    // assert(!(a != b));

    // std::vector<Fraction> fracs{{3,4}, {1,2}, {2,3}, {1,4}};
    // std::ranges::sort(fracs);
    // assert(fracs[0] == Fraction{1,4});
    // assert(fracs[3] == Fraction{3,4});
    std::cout << "  Exercise 1: spaceship — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: std::format — build a table
// Format a vector of {name, age, gpa} into an aligned table.
// Each row: "| %-15s | %3d | %5.2f |"
// ──────────────────────────────────────────────────────────────────────────
// TODO: std::string format_table(const std::vector<std::tuple<std::string,int,double>>& data)

void exercise_format()
{
#if HAS_FORMAT
    // std::vector<std::tuple<std::string,int,double>> data{
    //     {"Alice", 22, 3.95},
    //     {"Bob", 25, 3.72},
    //     {"Carol", 21, 3.88}
    // };
    // auto table = format_table(data);
    // assert(table.find("Alice") != std::string::npos);
    // assert(table.find("3.95") != std::string::npos);
#endif
    std::cout << "  Exercise 2: format table — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: std::span — statistics functions
// Write:
//   double mean(std::span<const double> data)
//   double variance(std::span<const double> data)
//   double stdev(std::span<const double> data)
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement mean, variance, stdev

void exercise_span()
{
    // std::vector<double> data{2, 4, 4, 4, 5, 5, 7, 9};
    // assert(std::abs(mean(data) - 5.0) < 0.01);
    // assert(std::abs(variance(data) - 4.0) < 0.01);
    // assert(std::abs(stdev(data) - 2.0) < 0.01);
    //
    // // Works with subspan too:
    // std::span<const double> s(data);
    // assert(std::abs(mean(s.first(4)) - 3.5) < 0.01);
    std::cout << "  Exercise 3: span statistics — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: jthread — producer/consumer with stop token
// Create a producer jthread that pushes numbers 1,2,3,... into a
// shared vector, and a consumer that processes them.
// Use stop_token to gracefully shut down both.
// ──────────────────────────────────────────────────────────────────────────
// TODO: implement producer/consumer with stop tokens

void exercise_jthread()
{
    // std::vector<int> buffer;
    // std::mutex mtx;
    // std::jthread producer([&](std::stop_token st) {
    //     int i = 0;
    //     while (!st.stop_requested()) {
    //         std::lock_guard lock(mtx);
    //         buffer.push_back(++i);
    //     }
    // });
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // producer.request_stop();
    // assert(!buffer.empty());
    std::cout << "  Exercise 4: jthread — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: source_location — build a simple assert macro replacement
// Write assert_eq(a, b) as a function that:
//   - Compares a == b
//   - If not equal, prints file:line, function, and values
//   - Uses source_location (no macros!)
// ──────────────────────────────────────────────────────────────────────────
// TODO:
// template<typename T>
// void assert_eq(const T& a, const T& b,
//                std::source_location loc = std::source_location::current())

void exercise_source_location()
{
    // assert_eq(1 + 1, 2);   // passes silently
    // assert_eq(3, 3);       // passes silently
    // assert_eq(2, 3);       // prints error with location
    std::cout << "  Exercise 5: source_location — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Custom formatter
// Implement std::formatter<Matrix> for a simple 2x2 matrix:
//   struct Matrix { double data[2][2]; };
// Format: "[[a, b], [c, d]]"
// Support width specifier for element formatting.
// ──────────────────────────────────────────────────────────────────────────
struct Matrix
{
    double data[2][2];
};

// TODO: template<> struct std::formatter<Matrix> { ... };

void exercise_custom_formatter()
{
#if HAS_FORMAT
    // Matrix m{{{1.5, 2.0}, {3.0, 4.5}}};
    // auto s = std::format("{}", m);
    // assert(s.find("1.5") != std::string::npos);
    // assert(s.find("4.5") != std::string::npos);
#endif
    std::cout << "  Exercise 6: custom formatter — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 12 — Exercises: C++20 Utilities         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_spaceship();
    exercise_format();
    exercise_span();
    exercise_jthread();
    exercise_source_location();
    exercise_custom_formatter();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
