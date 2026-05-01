// ============================================================================
// Stdlib 04 — Exercises: Iterators
// ============================================================================
#include <algorithm>
#include <cassert>
#include <deque>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// ── Exercise 1: Read ints from string, output doubled ────────────────────
void ex1_stream_iterators()
{
    std::cout << "Exercise 1: Stream iterators\n";
    std::string input = "5 10 15 20 25";

    // TODO: Use istream_iterator to read ints from input
    // TODO: Use transform + ostream_iterator to print each value doubled

    std::cout << "\n";
}

// ── Exercise 2: Reverse copy with back_inserter ──────────────────────────
void ex2_reverse_copy()
{
    std::cout << "Exercise 2: Reverse copy\n";
    std::vector<int> src{1, 2, 3, 4, 5};

    // TODO: Use std::reverse_copy with back_inserter to create reversed vector
    // TODO: Print result

    std::cout << "\n";
}

// ── Exercise 3: Nth element via std::advance ─────────────────────────────
void ex3_nth_element()
{
    std::cout << "Exercise 3: Nth element in a list\n";
    std::list<std::string> l{"alpha", "beta", "gamma", "delta", "epsilon"};

    // TODO: Write a function that returns the nth element of a list
    //        using std::advance (since list doesn't support [])
    // TODO: Print the 3rd element (0-indexed)

    std::cout << "\n";
}

// ── Exercise 4: Merge sorted ranges ──────────────────────────────────────
void ex4_merge()
{
    std::cout << "Exercise 4: Merge sorted ranges\n";
    std::vector<int> a{1, 3, 5, 7, 9};
    std::vector<int> b{2, 4, 6, 8, 10};

    // TODO: Use std::merge with back_inserter to merge a and b
    // TODO: Print the merged result

    std::cout << "\n";
}

// ── Exercise 5: iterator_traits dispatch ─────────────────────────────────
// TODO: Write a template function my_distance(It first, It last) that
//        - Uses (last - first) for random access iterators
//        - Uses manual increment-and-count for other iterators
//        Dispatch using iterator_traits<It>::iterator_category

void ex5_dispatch()
{
    std::cout << "Exercise 5: iterator_traits dispatch\n";

    // TODO: Test with vector (random access) and list (bidirectional)
    // TODO: Verify both give correct distance

    std::cout << "\n";
}

// ── Exercise 6: Custom iterator — Fibonacci ──────────────────────────────
// TODO: Create a FibIterator that generates Fibonacci numbers on-the-fly.
//        It should be a forward iterator.
// TODO: Create a FibRange(n) class that yields the first n Fibonacci numbers.

void ex6_fibonacci_iter()
{
    std::cout << "Exercise 6: Fibonacci iterator\n";

    // TODO: FibRange fib(10);
    // TODO: Print first 10 Fibonacci numbers using range-for
    // TODO: Use std::accumulate to sum them

    std::cout << "\n";
}

// ── Exercise 7: Zip iterator (advanced) ──────────────────────────────────
// TODO: Create a simple ZipIterator<It1, It2> that iterates two ranges
//        in parallel, yielding pairs.

void ex7_zip()
{
    std::cout << "Exercise 7: Zip iterator\n";

    std::vector<std::string> names{"Alice", "Bob", "Charlie"};
    std::vector<int>         scores{95, 87, 92};

    // TODO: Zip names and scores together, print as "name: score"

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — CSV line parser ───────────────────────────
void ex8_csv_parser()
{
    std::cout << "Exercise 8: CSV line parser\n";

    std::string csv = "Alice,95,A\nBob,87,B+\nCharlie,92,A-";

    // TODO: Parse each line into a vector<string> of fields
    //        Use string stream + getline with ',' delimiter
    // TODO: Store all rows in vector<vector<string>>
    // TODO: Print as a formatted table

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 04 — Exercises: Iterators                ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_stream_iterators();
    ex2_reverse_copy();
    ex3_nth_element();
    ex4_merge();
    ex5_dispatch();
    ex6_fibonacci_iter();
    ex7_zip();
    ex8_csv_parser();

    std::cout << "All exercises complete.\n";
    return 0;
}
