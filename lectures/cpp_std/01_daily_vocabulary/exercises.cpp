// ============================================================================
// Lecture 01 — Exercises: Daily Vocabulary (C++11)
// ============================================================================
// Build:  cmake --build build/Debug --target lecture01_exercises
// Run:    ./build/Debug/lecture01_exercises
//
// Instructions:
//   1. Each exercise has a TODO comment. Implement the code.
//   2. Compile and run. The asserts will tell you if you got it right.
//   3. Do NOT peek at demo.cpp until you've tried on your own!
// ============================================================================

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: auto deduction
//
// Fill in the blanks so the static_asserts pass.
// ──────────────────────────────────────────────────────────────────────────
void exercise_auto()
{
    // TODO: Declare 'a' using auto, initialized to 3.14f
    // auto a = ???;
    // static_assert(std::is_same<decltype(a), float>::value, "");

    // TODO: Declare 'b' using auto, initialized to a std::string literal
    // auto b = ???;
    // static_assert(std::is_same<decltype(b), std::string>::value, "");

    // TODO: Given the following, what type does 'c' have?
    // const double pi = 3.14159;
    // auto c = pi;
    // static_assert(std::is_same<decltype(c), ???>::value, "auto drops const");

    std::cout << "  Exercise 1: auto — PASSED (uncomment and fill in)\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Range-based for
//
// Rewrite the loop using range-based for to double every element.
// ──────────────────────────────────────────────────────────────────────────
void exercise_range_for()
{
    std::vector<int> v{1, 2, 3, 4, 5};

    // TODO: Use a range-based for loop (by reference) to multiply each element by 2
    // for (???) { ... }

    // Verify:
    // assert(v[0] == 2 && v[1] == 4 && v[2] == 6 && v[3] == 8 && v[4] == 10);
    std::cout << "  Exercise 2: range-for — PASSED (uncomment and implement)\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: nullptr
//
// Write a function that searches a vector for a value and returns a pointer
// to the element, or nullptr if not found.
// ──────────────────────────────────────────────────────────────────────────

// TODO: Implement find_ptr
// int* find_ptr(std::vector<int>& vec, int target)
// {
//     ???
// }

void exercise_nullptr()
{
    // std::vector<int> data{10, 20, 30, 40, 50};
    //
    // int* p = find_ptr(data, 30);
    // assert(p != nullptr && *p == 30);
    //
    // int* q = find_ptr(data, 99);
    // assert(q == nullptr);

    std::cout << "  Exercise 3: nullptr — PASSED (uncomment and implement)\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Brace initialization
//
// Fix the following code so it compiles without warnings or errors.
// The intent is described in comments.
// ──────────────────────────────────────────────────────────────────────────
void exercise_brace_init()
{
    // TODO: Fix these — some will fail due to narrowing prevention.
    // int a{3.14};         // Intent: store 3 (truncate)
    // uint8_t b{1000};     // Intent: store a small number (fix the value)
    // std::vector<int> v(5, 1);  // This is correct: 5 ones. Rewrite using {} syntax
    //                            // to create a vector containing exactly {5, 1}

    std::cout << "  Exercise 4: brace init — PASSED (uncomment and fix)\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: enum class
//
// Design a Direction enum class with values: North, South, East, West.
// Write a function that returns the opposite direction.
// ──────────────────────────────────────────────────────────────────────────

// TODO: Define enum class Direction
// enum class Direction { ??? };

// TODO: Implement opposite()
// Direction opposite(Direction d) { ??? }

void exercise_enum_class()
{
    // assert(opposite(Direction::North) == Direction::South);
    // assert(opposite(Direction::East) == Direction::West);
    // assert(opposite(Direction::South) == Direction::North);
    // assert(opposite(Direction::West) == Direction::East);

    std::cout << "  Exercise 5: enum class — PASSED (uncomment and implement)\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Combine everything
//
// Write a function `summarize` that takes a vector<string> and returns a
// map<char, int> counting first letters. Use:
//   - auto for all variable types
//   - range-for to iterate
//   - brace initialization for the map
//   - nullptr is not needed here, but scoped enums could be added for fun
// ──────────────────────────────────────────────────────────────────────────

// TODO: Implement summarize
// auto summarize(const std::vector<std::string>& words) -> std::map<char, int>
// {
//     ???
// }

void exercise_challenge()
{
    // std::vector<std::string> words{"apple", "avocado", "banana", "cherry", "cranberry"};
    // auto result = summarize(words);
    // assert(result['a'] == 2);
    // assert(result['b'] == 1);
    // assert(result['c'] == 2);

    std::cout << "  Exercise 6: challenge — PASSED (uncomment and implement)\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔════════════════════════════════════════════════╗\n"
              << "║  Lecture 01 — Exercises: Daily Vocabulary      ║\n"
              << "╚════════════════════════════════════════════════╝\n\n";

    exercise_auto();
    exercise_range_for();
    exercise_nullptr();
    exercise_brace_init();
    exercise_enum_class();
    exercise_challenge();

    std::cout << "\nAll exercises done. Great work!\n";
    return 0;
}
