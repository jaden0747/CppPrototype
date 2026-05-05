// ============================================================================
// Stdlib 02 — Exercises: Associative Containers
// ============================================================================
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

// ── Exercise 1: Word frequency counter ───────────────────────────────────
void ex1_word_freq()
{
    std::cout << "Exercise 1: Word frequency counter\n";
    std::string text = "the quick brown fox jumps over the lazy dog the fox";

    // TODO: Split text into words (use istringstream or manual split)
    // TODO: Count frequency of each word using std::map<string, int>
    // TODO: Print words sorted by frequency (descending)

    std::cout << "\n";
}

// ── Exercise 2: Set operations ───────────────────────────────────────────
void ex2_set_ops()
{
    std::cout << "Exercise 2: Set operations\n";
    std::set<int> a{1, 2, 3, 4, 5, 6};
    std::set<int> b{4, 5, 6, 7, 8, 9};

    // TODO: Compute and print the union of a and b (use std::set_union)
    // TODO: Compute and print the intersection
    // TODO: Compute and print the difference (a - b)
    // TODO: Compute and print the symmetric difference

    std::cout << "\n";
}

// ── Exercise 3: Student grades (multimap) ────────────────────────────────
void ex3_grades()
{
    std::cout << "Exercise 3: Student grades\n";

    // TODO: Create a multimap<string, int> mapping student names to grades
    // TODO: Add several grades for each student
    // TODO: For each student, print their average grade
    //        (use equal_range to find all grades for a student)

    std::cout << "\n";
}

// ── Exercise 4: LRU-style ordered set ────────────────────────────────────
void ex4_ordered_unique()
{
    std::cout << "Exercise 4: First-seen order with uniqueness\n";

    std::vector<std::string> input{"banana", "apple", "cherry", "apple", "banana", "date"};

    // TODO: Use a set to track seen items, and a vector to maintain insertion order
    // TODO: Print unique items in first-seen order: banana, apple, cherry, date

    std::cout << "\n";
}

// ── Exercise 5: Node extraction — rename keys ────────────────────────────
void ex5_rename_keys()
{
    std::cout << "Exercise 5: Rename keys via node extraction\n";

    std::map<std::string, int> inventory{{"apples", 10}, {"bananas", 5}, {"cherries", 20}};

    // TODO: Use extract() to rename "apples" -> "green_apples"
    //        without copying the value
    // TODO: Print the resulting map

    std::cout << "\n";
}

// ── Exercise 6: Custom comparator — reverse sorted map ───────────────────
void ex6_custom_comp()
{
    std::cout << "Exercise 6: Custom comparator\n";

    // TODO: Create a map<int, string> that stores keys in descending order
    //        using std::greater<>
    // TODO: Insert {1,"one"}, {2,"two"}, {3,"three"}, {4,"four"}
    // TODO: Print — should show 4, 3, 2, 1

    std::cout << "\n";
}

// ── Exercise 7: Phone book with contains & try_emplace ───────────────────
void ex7_phonebook()
{
    std::cout << "Exercise 7: Phone book\n";

    // TODO: Create a map<string, string> (name -> phone)
    // TODO: Use try_emplace to add entries (don't overwrite existing)
    // TODO: Use contains() (C++20) to check if a name exists
    // TODO: Use insert_or_assign to update an existing entry
    // TODO: Print the phone book

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — Concordance ───────────────────────────────
void ex8_concordance()
{
    std::cout << "Exercise 8: Concordance mini-project\n";

    // A concordance maps each word to the set of line numbers where it appears.
    // TODO: Given this multi-line text:
    std::string text = R"(the quick brown fox
jumps over the lazy dog
the fox and the dog are friends)";

    // TODO: Build a map<string, set<int>> where key=word, value=set of line numbers
    // TODO: Print each word and its line numbers

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 02 — Exercises: Associative Containers   ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_word_freq();
    ex2_set_ops();
    ex3_grades();
    ex4_ordered_unique();
    ex5_rename_keys();
    ex6_custom_comp();
    ex7_phonebook();
    ex8_concordance();

    std::cout << "All exercises complete.\n";
    return 0;
}
