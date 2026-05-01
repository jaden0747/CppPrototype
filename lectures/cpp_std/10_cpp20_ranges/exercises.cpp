// ============================================================================
// Lecture 10 — Exercises: C++20 Ranges
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Rewrite with views
// Convert the imperative loop to a range pipeline.
// ──────────────────────────────────────────────────────────────────────────
void exercise_rewrite()
{
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // IMPERATIVE (rewrite this):
    // std::vector<int> result;
    // for (int x : data) {
    //     if (x % 3 == 0) {
    //         result.push_back(x * x);
    //         if (result.size() == 4) break;
    //     }
    // }

    // TODO: Use views::filter | views::transform | views::take
    // auto pipeline = data | ...;
    // Verify: result should be {9, 36, 81, 144}

    std::cout << "  Exercise 1: rewrite with views — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Generate primes with views
// Use views::iota + filter to generate first N primes.
// ──────────────────────────────────────────────────────────────────────────
// TODO: auto is_prime(int n) -> bool { ... }

void exercise_primes()
{
    // auto primes = std::views::iota(2)
    //     | std::views::filter(is_prime)
    //     | std::views::take(10);
    //
    // std::vector<int> result(primes.begin(), primes.end());
    // assert(result.size() == 10);
    // assert(result[0] == 2);
    // assert(result[3] == 7);
    // assert(result[9] == 29);
    std::cout << "  Exercise 2: primes — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Split and process strings
// Split "one:two:three:four" by ':' and collect into vector<string>.
// ──────────────────────────────────────────────────────────────────────────
void exercise_split()
{
    // std::string input = "one:two:three:four";
    // TODO: Use std::views::split to break by ':'
    // auto words = input | std::views::split(':') | ...;
    // Convert sub-ranges to strings
    // assert(words.size() == 4);
    // assert(words[0] == "one");
    // assert(words[3] == "four");
    std::cout << "  Exercise 3: split — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Projections
// Sort a vector of {name, score} by score descending,
// then find the person with name "Carol".
// ──────────────────────────────────────────────────────────────────────────
struct Student
{
    std::string name;
    int         score;
};

void exercise_projections()
{
    std::vector<Student> students{{"Alice", 85}, {"Bob", 92}, {"Carol", 78}, {"Dave", 95}, {"Eve", 88}};

    // TODO: Sort by score descending using std::ranges::sort with projection
    // TODO: Find "Carol" using std::ranges::find with projection
    // assert(students[0].name == "Dave");  // highest score first
    // assert(found->score == 78);
    std::cout << "  Exercise 4: projections — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Multi-stage pipeline
// Given a vector of ints, build a pipeline that:
//   1. Drops the first 2 elements
//   2. Takes elements while < 100
//   3. Filters out odds
//   4. Doubles each value
// Test with: {5, 3, 10, 20, 30, 50, 150, 200}
// Expected: {20, 40, 60, 100} (dropped 5,3; took until 150; kept evens; doubled)
// ──────────────────────────────────────────────────────────────────────────
void exercise_pipeline()
{
    // std::vector<int> input{5, 3, 10, 20, 30, 50, 150, 200};
    // TODO: Build the pipeline
    // auto result = input | views::drop(2) | ...;
    std::cout << "  Exercise 5: multi-stage pipeline — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Sliding window average
// Implement a function that computes a sliding window average of size N
// over a range. Use views and range adaptors.
// Example: sliding_avg({1,2,3,4,5}, 3) = {2.0, 3.0, 4.0}
// ──────────────────────────────────────────────────────────────────────────
// TODO: std::vector<double> sliding_avg(const std::vector<int>& data, int window)

void exercise_sliding_window()
{
    // auto result = sliding_avg({1, 2, 3, 4, 5, 6, 7}, 3);
    // assert(result.size() == 5);
    // assert(result[0] == 2.0);  // (1+2+3)/3
    // assert(result[1] == 3.0);  // (2+3+4)/3
    // assert(result[4] == 6.0);  // (5+6+7)/3
    std::cout << "  Exercise 6: sliding window — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 10 — Exercises: C++20 Ranges            ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_rewrite();
    exercise_primes();
    exercise_split();
    exercise_projections();
    exercise_pipeline();
    exercise_sliding_window();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
