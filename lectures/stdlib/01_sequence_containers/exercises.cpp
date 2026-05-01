// ============================================================================
// Stdlib 01 — Exercises: Sequence Containers
// ============================================================================
// Instructions: Replace every TODO with working code, then compile and run.
// ============================================================================
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <vector>

// ── Exercise 1: vector basics ────────────────────────────────────────────
// Create a vector of ints 1..10, remove all even numbers, print the rest.
void ex1_vector_basics()
{
    std::cout << "Exercise 1: vector basics\n";
    // TODO: Create vector<int> with values 1 through 10
    // TODO: Use std::erase_if (C++20) to remove even numbers
    // TODO: Print remaining elements

    std::cout << "\n";
}

// ── Exercise 2: vector capacity management ───────────────────────────────
// Demonstrate the difference between reserve() and resize().
void ex2_capacity()
{
    std::cout << "Exercise 2: capacity management\n";
    std::vector<int> v;

    // TODO: reserve 50 elements — what are size() and capacity()?
    // TODO: resize to 20 with default value 0 — what are size() and capacity()?
    // TODO: shrink_to_fit — what is capacity now?

    std::cout << "\n";
}

// ── Exercise 3: std::array operations ────────────────────────────────────
// Sort an array, find the median, and compute the sum.
void ex3_array()
{
    std::cout << "Exercise 3: array operations\n";
    std::array<int, 7> a{42, 17, 8, 99, 3, 55, 21};

    // TODO: Sort the array
    // TODO: Print the median (middle element)
    // TODO: Compute and print the sum using std::accumulate

    std::cout << "\n";
}

// ── Exercise 4: deque as a sliding window ────────────────────────────────
// Maintain a sliding window of the last 5 elements from a stream.
void ex4_deque_window()
{
    std::cout << "Exercise 4: deque sliding window\n";
    std::vector<int> stream{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::deque<int>  window;

    // TODO: For each element in stream:
    //   - push_back to window
    //   - if window.size() > 5, pop_front
    //   - print the current window contents

    std::cout << "\n";
}

// ── Exercise 5: list splice & merge ──────────────────────────────────────
// Merge two sorted lists and splice a subrange from one list to another.
void ex5_list_ops()
{
    std::cout << "Exercise 5: list splice & merge\n";
    std::list<int> a{1, 3, 5, 7, 9};
    std::list<int> b{2, 4, 6, 8, 10};

    // TODO: Merge b into a (both are sorted) — use a.merge(b)
    // TODO: Print merged list
    // TODO: Splice the first 3 elements of a into a new list c
    // TODO: Print both a and c

    std::cout << "\n";
}

// ── Exercise 6: forward_list operations ──────────────────────────────────
// Reverse a forward_list and remove duplicates.
void ex6_forward_list()
{
    std::cout << "Exercise 6: forward_list\n";
    std::forward_list<int> fl{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};

    // TODO: Sort the forward_list
    // TODO: Remove consecutive duplicates with unique()
    // TODO: Reverse it
    // TODO: Print the result

    std::cout << "\n";
}

// ── Exercise 7: Iterator invalidation test ───────────────────────────────
// Demonstrate safe iteration with erase.
void ex7_safe_erase()
{
    std::cout << "Exercise 7: safe erase during iteration\n";
    std::list<int> l{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // TODO: Iterate through the list and erase all multiples of 3.
    // Use the return value of erase() to keep a valid iterator.
    // Print the result: should be {1, 2, 4, 5, 7, 8, 10}

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — TaskQueue ─────────────────────────────────
struct Task
{
    int         id;
    std::string name;
    bool        priority;
};

class TaskQueue
{
    // TODO: Use std::deque<Task> as the underlying container
public:
    // TODO: void add(Task t) — push_back for normal, push_front for priority
    // TODO: Task pop_next() — pop from front
    // TODO: void remove_by_id(int id) — remove task with given id
    // TODO: size_t size() const
    // TODO: bool empty() const
    // TODO: void print() const — display all tasks
};

void ex8_task_queue()
{
    std::cout << "Exercise 8: TaskQueue mini-project\n";

    // TODO: Create a TaskQueue, add some tasks (mix of priority and normal)
    // TODO: Pop a few, remove one by id, print state after each operation

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 01 — Exercises: Sequence Containers      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_vector_basics();
    ex2_capacity();
    ex3_array();
    ex4_deque_window();
    ex5_list_ops();
    ex6_forward_list();
    ex7_safe_erase();
    ex8_task_queue();

    std::cout << "All exercises complete.\n";
    return 0;
}
