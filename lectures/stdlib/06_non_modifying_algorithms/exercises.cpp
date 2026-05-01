// ============================================================================
// Stdlib 06 — Exercises: Non-Modifying Algorithms
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

// ── Exercise 1: Find first negative ──────────────────────────────────────
void ex1_find_negative()
{
    std::cout << "Exercise 1: Find first negative\n";
    std::vector<int> v{5, 3, 8, -2, 7, -5, 1};

    // TODO: Use find_if to locate the first negative number
    // TODO: Print its value and index

    std::cout << "\n";
}

// ── Exercise 2: Count words longer than 4 ────────────────────────────────
void ex2_count_long_words()
{
    std::cout << "Exercise 2: Count long words\n";
    std::vector<std::string> words{"the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog"};

    // TODO: Use count_if (ranges version) to count words with length > 4
    // TODO: Print the count

    std::cout << "\n";
}

// ── Exercise 3: Validate data with all_of / any_of ──────────────────────
void ex3_validation()
{
    std::cout << "Exercise 3: Validate data\n";
    std::vector<int> ages{25, 30, 17, 45, 22};

    // TODO: Check if all ages are >= 18 (all_of)
    // TODO: Check if any age is >= 40 (any_of)
    // TODO: Check if none are negative (none_of)
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 4: Find the longest string ──────────────────────────────────
void ex4_longest()
{
    std::cout << "Exercise 4: Longest string\n";
    std::vector<std::string> words{"apple", "banana", "cherry", "date", "elderberry", "fig"};

    // TODO: Use ranges::max_element with a projection on string length
    // TODO: Print the longest word and its length

    std::cout << "\n";
}

// ── Exercise 5: Find subsequence ─────────────────────────────────────────
void ex5_search()
{
    std::cout << "Exercise 5: Subsequence search\n";
    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> pattern{5, 6, 7};

    // TODO: Use std::search to find 'pattern' within 'data'
    // TODO: Print the starting index

    std::cout << "\n";
}

// ── Exercise 6: Compare two containers ───────────────────────────────────
void ex6_compare()
{
    std::cout << "Exercise 6: Compare containers\n";
    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<int> b{1, 2, 4, 4, 5};

    // TODO: Use mismatch to find where a and b first differ
    // TODO: Use equal to check if they are identical
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 7: for_each accumulation ────────────────────────────────────
void ex7_for_each_stats()
{
    std::cout << "Exercise 7: Statistics with for_each\n";
    std::vector<double> data{2.5, 3.7, 1.2, 8.4, 5.1, 6.3};

    // TODO: Use for_each to compute sum, min, max in a single pass
    //        (capture sum/min/max by reference in the lambda)
    // TODO: Compute and print mean, min, max

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — text analysis ─────────────────────────────
void ex8_text_analysis()
{
    std::cout << "Exercise 8: Text analysis\n";
    std::string text = "To be or not to be that is the question";

    // TODO: Split text into words
    // TODO: Count total words
    // TODO: Find the longest word (max_element)
    // TODO: Find the shortest word (min_element)
    // TODO: Check if any word is longer than 10 characters (any_of)
    // TODO: Count words that start with 't' or 'T'
    // TODO: Print all statistics

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 06 — Exercises: Non-Modifying Algorithms  ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_find_negative();
    ex2_count_long_words();
    ex3_validation();
    ex4_longest();
    ex5_search();
    ex6_compare();
    ex7_for_each_stats();
    ex8_text_analysis();

    std::cout << "All exercises complete.\n";
    return 0;
}
