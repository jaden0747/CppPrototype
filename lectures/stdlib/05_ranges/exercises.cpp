// ============================================================================
// Stdlib 05 — Exercises: Ranges (C++20)
// ============================================================================
#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

// ── Exercise 1: Filter and transform pipeline ────────────────────────────
void ex1_pipeline()
{
    std::cout << "Exercise 1: Filter + transform pipeline\n";
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    // TODO: Create a pipeline that:
    //   1. Filters to multiples of 3
    //   2. Transforms by doubling
    //   3. Takes only the first 3 results
    // TODO: Print result (should be: 6 12 18)

    std::cout << "\n";
}

// ── Exercise 2: Sort with projection ─────────────────────────────────────
struct Product
{
    std::string name;
    double      price;
    int         quantity;
};

void ex2_projection_sort()
{
    std::cout << "Exercise 2: Sort with projection\n";

    std::vector<Product> products{
        {"Laptop", 999.99, 5}, {"Mouse", 29.99, 50}, {"Keyboard", 79.99, 30}, {"Monitor", 499.99, 10}};

    // TODO: Sort by price ascending using projection
    // TODO: Print sorted products
    // TODO: Sort by quantity descending using projection
    // TODO: Print again

    std::cout << "\n";
}

// ── Exercise 3: iota + fizzbuzz ──────────────────────────────────────────
void ex3_fizzbuzz()
{
    std::cout << "Exercise 3: FizzBuzz with iota\n";

    // TODO: Use views::iota(1, 31) and views::transform to create
    //        a FizzBuzz output (string for each number)
    // TODO: Print each result

    std::cout << "\n";
}

// ── Exercise 4: Split and process CSV ────────────────────────────────────
void ex4_csv_split()
{
    std::cout << "Exercise 4: CSV split\n";
    std::string line = "John,28,Engineer,New York";

    // TODO: Use views::split(',') to extract each field
    // TODO: Print fields with indices: [0]="John", [1]="28", etc.

    std::cout << "\n";
}

// ── Exercise 5: Compose multiple views ───────────────────────────────────
void ex5_composition()
{
    std::cout << "Exercise 5: View composition\n";

    std::vector<std::string> words{"hello", "world", "foo", "bar", "baz", "quux", "corge", "grault"};

    // TODO: Create a pipeline that:
    //   1. Filters words longer than 3 characters
    //   2. Takes the first 4
    //   3. Reverses the order
    // TODO: Print result

    std::cout << "\n";
}

// ── Exercise 6: Range algorithm — partition ──────────────────────────────
void ex6_partition()
{
    std::cout << "Exercise 6: ranges::partition\n";
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // TODO: Use ranges::partition to put even numbers first
    // TODO: Print the partitioned vector
    // TODO: Print where the partition point is

    std::cout << "\n";
}

// ── Exercise 7: Enumerate simulation ─────────────────────────────────────
void ex7_enumerate()
{
    std::cout << "Exercise 7: Enumerate\n";
    std::vector<std::string> fruits{"apple", "banana", "cherry", "date"};

    // TODO: Use views::iota + views::transform (or zip if available)
    //        to print each fruit with its index: "0: apple", "1: banana", etc.

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — lazy prime sieve ──────────────────────────
void ex8_primes()
{
    std::cout << "Exercise 8: Lazy prime finder\n";

    // TODO: Use views::iota(2) | views::filter(is_prime) | views::take(20)
    //        to lazily generate the first 20 prime numbers
    // Write is_prime as a lambda or function
    // TODO: Print them

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 05 — Exercises: Ranges (C++20)           ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_pipeline();
    ex2_projection_sort();
    ex3_fizzbuzz();
    ex4_csv_split();
    ex5_composition();
    ex6_partition();
    ex7_enumerate();
    ex8_primes();

    std::cout << "All exercises complete.\n";
    return 0;
}
