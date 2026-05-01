// ============================================================================
// Stdlib 10 — Exercises: Memory Management
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <string>
#include <vector>

// ── Exercise 1: unique_ptr ownership transfer ────────────────────────────
void ex1_unique_ownership()
{
    std::cout << "Exercise 1: unique_ptr ownership\n";

    // TODO: Create a unique_ptr<string> with make_unique
    // TODO: Pass it to a function via std::move, which prints and returns it
    // TODO: Verify the pointer is valid after getting it back

    std::cout << "\n";
}

// ── Exercise 2: unique_ptr with polymorphism ─────────────────────────────
// TODO: Define a Shape base class with virtual area() and virtual destructor
// TODO: Define Circle and Rectangle derived classes

void ex2_polymorphic_unique()
{
    std::cout << "Exercise 2: Polymorphic unique_ptr\n";

    // TODO: Create vector<unique_ptr<Shape>>
    // TODO: Add a Circle and Rectangle
    // TODO: Print the area of each shape via base pointer

    std::cout << "\n";
}

// ── Exercise 3: shared_ptr reference counting ────────────────────────────
void ex3_shared_counting()
{
    std::cout << "Exercise 3: shared_ptr counting\n";

    // TODO: Create a shared_ptr<int>
    // TODO: Copy it into a vector of shared_ptrs (3 copies)
    // TODO: Print use_count after each operation
    // TODO: Clear the vector and verify use_count drops back to 1

    std::cout << "\n";
}

// ── Exercise 4: Break a shared_ptr cycle ─────────────────────────────────
struct Person
{
    std::string name;
    // TODO: One of these should be weak_ptr to break the cycle
    std::shared_ptr<Person> best_friend;

    Person(std::string n)
        : name(std::move(n))
    {
        std::cout << "  Person(" << name << ") created\n";
    }
    ~Person()
    {
        std::cout << "  Person(" << name << ") destroyed\n";
    }
};

void ex4_break_cycle()
{
    std::cout << "Exercise 4: Break shared_ptr cycle\n";

    // TODO: Create two Person shared_ptrs
    // TODO: Make them each other's best_friend
    // TODO: Observe that they LEAK (destructors not called) with shared_ptr cycle
    // TODO: Fix by changing one member to weak_ptr

    std::cout << "\n";
}

// ── Exercise 5: Custom deleter ───────────────────────────────────────────
void ex5_custom_deleter()
{
    std::cout << "Exercise 5: Custom deleter\n";

    // TODO: Create a unique_ptr<int[]> that uses a custom deleter
    //        which prints "Array deleted" before freeing
    // TODO: Fill and print the array

    std::cout << "\n";
}

// ── Exercise 6: PMR arena allocator ──────────────────────────────────────
void ex6_pmr_arena()
{
    std::cout << "Exercise 6: PMR arena allocator\n";

    // TODO: Create a stack buffer of 8KB
    // TODO: Create a monotonic_buffer_resource using that buffer
    // TODO: Allocate a pmr::vector<int> with 1000 elements from the pool
    // TODO: Verify it works without heap allocation
    // TODO: Print size

    std::cout << "\n";
}

// ── Exercise 7: Factory function returning unique_ptr ────────────────────
// TODO: Write a factory function create_widget(int type) -> unique_ptr<Widget>
//        that returns different widget types based on the input

void ex7_factory()
{
    std::cout << "Exercise 7: Factory with unique_ptr\n";

    // TODO: Call factory, use the returned unique_ptr

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — object pool ───────────────────────────────
// TODO: Implement a simple ObjectPool<T> that:
//   - Pre-allocates N objects
//   - acquire() returns a unique_ptr with custom deleter that returns to pool
//   - The pool reuses objects instead of allocating new ones

void ex8_object_pool()
{
    std::cout << "Exercise 8: Object pool\n";

    // TODO: Create an ObjectPool<Widget>(5)
    // TODO: Acquire 3 objects, use them
    // TODO: Release (let unique_ptrs go out of scope)
    // TODO: Acquire again — should reuse, not allocate

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 10 — Exercises: Memory Management        ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_unique_ownership();
    ex2_polymorphic_unique();
    ex3_shared_counting();
    ex4_break_cycle();
    ex5_custom_deleter();
    ex6_pmr_arena();
    ex7_factory();
    ex8_object_pool();

    std::cout << "All exercises complete.\n";
    return 0;
}
