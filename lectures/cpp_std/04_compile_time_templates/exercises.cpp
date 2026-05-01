// ============================================================================
// Lecture 04 — Exercises: Compile-Time Power & Threads
// ============================================================================
#include <cassert>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: constexpr Fibonacci + static_assert
//
// Write a constexpr fib(n) and verify:
//   fib(0)=0, fib(1)=1, fib(10)=55, fib(15)=610
// ──────────────────────────────────────────────────────────────────────────
// TODO: constexpr int fib(int n) { ... }
// static_assert(fib(0) == 0, "");
// static_assert(fib(10) == 55, "");

void exercise_constexpr()
{
    std::cout << "  Exercise 1: constexpr fib — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: static_assert for template validation
//
// Write a template class `NumericArray<T, N>` that:
//  - static_asserts T is arithmetic (std::is_arithmetic)
//  - static_asserts N > 0 and N <= 1024
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T, int N> class NumericArray { ... };

void exercise_static_assert()
{
    // NumericArray<double, 100> arr;    // OK
    // NumericArray<int, 1024> big;      // OK
    // NumericArray<std::string, 10> s;  // Should FAIL at compile time
    // NumericArray<int, 0> zero;        // Should FAIL at compile time
    std::cout << "  Exercise 2: static_assert validation — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Variadic min()
//
// Implement a variadic function template `minimum(args...)` that returns
// the smallest of all arguments. Works with any comparable type.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> T minimum(T a) { ... }
// TODO: template<typename T, typename... Args> T minimum(T first, Args... rest) { ... }

void exercise_variadic_min()
{
    // assert(minimum(5) == 5);
    // assert(minimum(3, 1, 4, 1, 5) == 1);
    // assert(minimum(10, 20, 3, 40) == 3);
    // assert(minimum(3.14, 2.71, 1.41) == 1.41);
    std::cout << "  Exercise 3: variadic min — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Type-safe printf
//
// Implement `safe_printf(format, args...)` that processes %d, %s, %f
// placeholders using variadic templates (no va_list!).
// Print to stdout. Throw if mismatch between format and args count.
// ──────────────────────────────────────────────────────────────────────────
// TODO: void safe_printf(const std::string& fmt) { ... }
// TODO: template<typename T, typename... Args>
//       void safe_printf(const std::string& fmt, T first, Args... rest) { ... }

void exercise_safe_printf()
{
    // safe_printf("Hello %s, you are %d years old\n", "Alice", 30);
    // safe_printf("Pi is approximately %f\n", 3.14159);
    // safe_printf("No args here\n");
    std::cout << "  Exercise 4: safe_printf — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Thread-safe counter
//
// Write a Counter class with increment() and get().
// Launch 8 threads, each incrementing 100000 times.
// Verify final count == 800000.
// ──────────────────────────────────────────────────────────────────────────
// TODO: class Counter { ... };

void exercise_threaded_counter()
{
    // Counter c;
    // std::vector<std::thread> threads;
    // for (int i = 0; i < 8; ++i)
    //     threads.emplace_back([&c]{ for (int j = 0; j < 100000; ++j) c.increment(); });
    // for (auto& t : threads) t.join();
    // assert(c.get() == 800000);
    std::cout << "  Exercise 5: threaded counter — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Parallel map-reduce
//
// Implement `parallel_sum(vector<int>, num_threads)` that:
//  - Splits the vector into chunks
//  - Sums each chunk in a separate thread (use std::async)
//  - Combines partial results
// ──────────────────────────────────────────────────────────────────────────
// TODO: long long parallel_sum(const std::vector<int>& data, int num_threads) { ... }

void exercise_parallel_sum()
{
    // std::vector<int> data(1000000);
    // std::iota(data.begin(), data.end(), 1);
    // long long expected = (long long)1000000 * 1000001 / 2;
    // long long result = parallel_sum(data, 4);
    // assert(result == expected);
    std::cout << "  Exercise 6: parallel sum — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 04 — Exercises                          ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_constexpr();
    exercise_static_assert();
    exercise_variadic_min();
    exercise_safe_printf();
    exercise_threaded_counter();
    exercise_parallel_sum();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
