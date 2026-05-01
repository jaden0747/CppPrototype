// ============================================================================
// Stdlib 08 — Exercises: Numeric, Set & Heap Algorithms
// ============================================================================
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <vector>

// ── Exercise 1: Running average ──────────────────────────────────────────
void ex1_running_average()
{
    std::cout << "Exercise 1: Running average\n";
    std::vector<double> data{10, 20, 30, 40, 50};

    // TODO: Use partial_sum to compute cumulative sums
    // TODO: Divide each cumulative sum by its 1-based index to get running average
    // TODO: Print: 10, 15, 20, 25, 30

    std::cout << "\n";
}

// ── Exercise 2: Dot product ──────────────────────────────────────────────
void ex2_dot_product()
{
    std::cout << "Exercise 2: Dot product\n";
    std::vector<double> a{1.0, 2.0, 3.0, 4.0};
    std::vector<double> b{5.0, 6.0, 7.0, 8.0};

    // TODO: Compute dot product using inner_product
    // TODO: Compute same using transform_reduce
    // TODO: Print both results (should be 70)

    std::cout << "\n";
}

// ── Exercise 3: Set operations ───────────────────────────────────────────
void ex3_set_ops()
{
    std::cout << "Exercise 3: Set operations on sorted vectors\n";

    std::vector<int> enrolled_math{1, 3, 5, 7, 9, 11};
    std::vector<int> enrolled_physics{3, 5, 8, 9, 12};

    // TODO: Find students in both classes (intersection)
    // TODO: Find students in either class (union)
    // TODO: Find students only in math (difference)
    // TODO: Print all results

    std::cout << "\n";
}

// ── Exercise 4: Priority queue with heap ─────────────────────────────────
void ex4_heap_pq()
{
    std::cout << "Exercise 4: Manual priority queue via heap\n";

    std::vector<int> pq;

    // TODO: Insert values {5, 2, 8, 1, 9, 3} one at a time using
    //        push_back + push_heap
    // TODO: Pop and print elements in descending order using
    //        pop_heap + pop_back

    std::cout << "\n";
}

// ── Exercise 5: Binary search applications ───────────────────────────────
void ex5_binary_search()
{
    std::cout << "Exercise 5: Binary search\n";

    std::vector<int> data{2, 4, 4, 4, 6, 8, 10, 12};

    // TODO: Use binary_search to check if 4 exists
    // TODO: Use equal_range to find how many 4s there are
    // TODO: Use lower_bound to find insertion point for value 5
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 6: Dice simulation with random ──────────────────────────────
void ex6_dice_sim()
{
    std::cout << "Exercise 6: Dice simulation\n";

    // TODO: Roll two dice 10000 times (mt19937 + uniform_int_distribution)
    // TODO: Count frequency of each sum (2-12) using a vector or map
    // TODO: Print histogram

    std::cout << "\n";
}

// ── Exercise 7: Bit manipulation ─────────────────────────────────────────
void ex7_bits()
{
    std::cout << "Exercise 7: Bit manipulation\n";

    // TODO: For numbers 1-16, print: value, popcount, is_power_of_2 (has_single_bit)
    // TODO: Use bit_ceil to find next power of 2 for values 5, 10, 17

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — statistical summary ───────────────────────
void ex8_stats()
{
    std::cout << "Exercise 8: Statistical summary\n";

    std::vector<double> data{23.5, 17.2, 31.8, 42.1, 15.6, 28.3, 36.7, 19.4, 25.1, 33.9};

    // TODO: Compute: sum, mean, min, max using accumulate + min/max_element
    // TODO: Compute variance using transform_reduce:
    //        variance = (1/n) * sum((x - mean)^2)
    // TODO: Compute standard deviation = sqrt(variance)
    // TODO: Compute median using nth_element
    // TODO: Print all statistics

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 08 — Exercises: Numeric, Set & Heap      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_running_average();
    ex2_dot_product();
    ex3_set_ops();
    ex4_heap_pq();
    ex5_binary_search();
    ex6_dice_sim();
    ex7_bits();
    ex8_stats();

    std::cout << "All exercises complete.\n";
    return 0;
}
