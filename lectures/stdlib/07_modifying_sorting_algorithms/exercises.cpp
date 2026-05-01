// ============================================================================
// Stdlib 07 — Exercises: Modifying & Sorting Algorithms
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <string>
#include <vector>

// ── Exercise 1: Transform strings to uppercase ───────────────────────────
void ex1_uppercase()
{
    std::cout << "Exercise 1: Uppercase transform\n";
    std::vector<std::string> words{"hello", "world", "foo", "bar"};

    // TODO: Use std::transform to convert each word to uppercase (in-place)
    // TODO: Print result

    std::cout << "\n";
}

// ── Exercise 2: Generate Fibonacci sequence ──────────────────────────────
void ex2_generate_fib()
{
    std::cout << "Exercise 2: Generate Fibonacci\n";

    // TODO: Use std::generate to fill a vector<int> of size 15 with Fibonacci numbers
    // Hint: capture two variables (a, b) in the lambda and update them
    // TODO: Print the sequence

    std::cout << "\n";
}

// ── Exercise 3: Remove and compress ──────────────────────────────────────
void ex3_remove_compress()
{
    std::cout << "Exercise 3: Remove vowels\n";
    std::string s = "the quick brown fox jumps over the lazy dog";

    // TODO: Remove all vowels (a,e,i,o,u) using erase-remove or C++20 erase_if
    // TODO: Print result

    std::cout << "\n";
}

// ── Exercise 4: Stable partition by predicate ────────────────────────────
void ex4_stable_partition()
{
    std::cout << "Exercise 4: Stable partition\n";

    struct Student
    {
        std::string name;
        int         grade;
    };

    std::vector<Student> students{
        {"Alice", 92}, {"Bob", 65}, {"Charlie", 88}, {"Diana", 55}, {"Eve", 95}, {"Frank", 70}};

    // TODO: Stable partition: passing students (grade >= 70) first
    // TODO: Print the partitioned list preserving original order within groups

    std::cout << "\n";
}

// ── Exercise 5: Top-K via partial_sort ───────────────────────────────────
void ex5_top_k()
{
    std::cout << "Exercise 5: Top-K scores\n";
    std::vector<int> scores{72, 95, 88, 67, 91, 84, 99, 76, 63, 87};
    int              k = 3;

    // TODO: Use partial_sort to find the top-k scores
    // TODO: Print the top-k

    std::cout << "\n";
}

// ── Exercise 6: Find median with nth_element ─────────────────────────────
void ex6_median()
{
    std::cout << "Exercise 6: Find median\n";
    std::vector<int> data{42, 17, 8, 99, 3, 55, 21, 67, 33};

    // TODO: Use nth_element to find the median in O(n)
    // TODO: Print the median

    std::cout << "\n";
}

// ── Exercise 7: Rotate array by k positions ──────────────────────────────
void ex7_rotate()
{
    std::cout << "Exercise 7: Rotate array\n";
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7};
    int              k = 3;

    // TODO: Rotate the array left by k positions using std::rotate
    // TODO: Print result (should be: 4 5 6 7 1 2 3)

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — card deck shuffle & sort ──────────────────
void ex8_card_deck()
{
    std::cout << "Exercise 8: Card deck\n";

    // TODO: Represent a standard 52-card deck as vector<pair<string, int>>
    //        where first = suit ("Hearts","Diamonds","Clubs","Spades")
    //        and second = rank (1-13)
    // TODO: Shuffle the deck using std::shuffle + mt19937
    // TODO: Deal 5 cards (use copy_n)
    // TODO: Sort the hand by suit then rank
    // TODO: Print the hand

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 07 — Exercises: Modifying & Sorting      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_uppercase();
    ex2_generate_fib();
    ex3_remove_compress();
    ex4_stable_partition();
    ex5_top_k();
    ex6_median();
    ex7_rotate();
    ex8_card_deck();

    std::cout << "All exercises complete.\n";
    return 0;
}
