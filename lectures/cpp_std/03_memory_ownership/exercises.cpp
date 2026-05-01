// ============================================================================
// Lecture 03 — Exercises: Memory & Ownership (C++11)
// ============================================================================
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Implement a move-only String class
//
// Create a SimpleString that:
//  - Allocates a char[] on the heap
//  - Deletes copy ctor/assign
//  - Implements move ctor/assign (steal the pointer)
//  - Has size() and c_str() accessors
// ──────────────────────────────────────────────────────────────────────────
// TODO: class SimpleString { ... };

void exercise_move_string()
{
    // SimpleString a("hello");
    // assert(a.size() == 5);
    // SimpleString b = std::move(a);
    // assert(b.size() == 5);
    // assert(a.size() == 0);
    // assert(a.c_str() == nullptr);
    std::cout << "  Exercise 1: move-only string — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Factory with unique_ptr
//
// Create a Shape hierarchy (Circle, Square, Triangle) and a factory
// function that returns unique_ptr<Shape>.
// ──────────────────────────────────────────────────────────────────────────
// TODO: Define hierarchy and factory

void exercise_factory()
{
    // auto c = makeShape("circle", 5.0);
    // auto s = makeShape("square", 3.0);
    // assert(c != nullptr);
    // assert(s != nullptr);
    // assert(c->area() > 78.0);  // pi * 25
    // assert(s->area() == 9.0);
    std::cout << "  Exercise 2: factory — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: shared_ptr reference counting
//
// Create a shared_ptr, copy it several times, check use_count at each step.
// ──────────────────────────────────────────────────────────────────────────
void exercise_refcount()
{
    // auto p1 = std::make_shared<int>(42);
    // assert(p1.use_count() == 1);
    // auto p2 = p1;
    // assert(p1.use_count() == 2);
    // {
    //     auto p3 = p2;
    //     assert(p1.use_count() == 3);
    // }
    // assert(p1.use_count() == 2);
    // p2.reset();
    // assert(p1.use_count() == 1);
    std::cout << "  Exercise 3: ref counting — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Fix the circular reference
//
// The following code LEAKS. Fix it using weak_ptr.
// ──────────────────────────────────────────────────────────────────────────
// struct Person {
//     std::string name;
//     std::shared_ptr<Person> best_friend;  // ← causes cycle
//     Person(std::string n) : name(std::move(n)) {}
//     ~Person() { std::cout << "  ~Person(" << name << ")\n"; }
// };

void exercise_fix_cycle()
{
    // {
    //     auto alice = std::make_shared<Person>("Alice");
    //     auto bob   = std::make_shared<Person>("Bob");
    //     alice->best_friend = bob;
    //     bob->best_friend = alice;  // cycle! Change one to weak_ptr
    // }
    // // Both destructors should have printed by now
    std::cout << "  Exercise 4: fix cycle — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5 (Challenge): Implement a minimal unique_ptr
//
// Write MyUniquePtr<T> that:
//  - Takes ownership of a raw T* in constructor
//  - Deletes in destructor
//  - Deleted copy, implemented move
//  - Has operator*, operator->, get(), release(), reset()
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> class MyUniquePtr { ... };

void exercise_my_unique_ptr()
{
    // MyUniquePtr<int> p(new int(42));
    // assert(*p == 42);
    // MyUniquePtr<int> q = std::move(p);
    // assert(p.get() == nullptr);
    // assert(*q == 42);
    // int* raw = q.release();
    // assert(q.get() == nullptr);
    // assert(*raw == 42);
    // delete raw;
    std::cout << "  Exercise 5: MyUniquePtr — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Lecture 03 — Exercises: Memory & Ownership      ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_move_string();
    exercise_factory();
    exercise_refcount();
    exercise_fix_cycle();
    exercise_my_unique_ptr();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
