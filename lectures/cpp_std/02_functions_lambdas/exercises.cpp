// ============================================================================
// Lecture 02 — Exercises: Functions & Lambdas (C++11)
// ============================================================================
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write lambdas for STL algorithms
//
// a) Sort a vector of strings by LENGTH (shortest first).
// b) Use find_if to locate the first string longer than 5 characters.
// c) Use count_if to count strings that start with 'a' (case-insensitive).
// ──────────────────────────────────────────────────────────────────────────
void exercise_stl_lambdas()
{
    std::vector<std::string> words{"banana", "apple", "fig", "avocado", "kiwi", "Apricot"};

    // TODO (a): sort by length
    // std::sort(words.begin(), words.end(), ???);
    // assert(words[0] == "fig");

    // TODO (b): find first string with length > 5
    // auto it = std::find_if(words.begin(), words.end(), ???);
    // assert(it != words.end() && it->size() > 5);

    // TODO (c): count strings starting with 'a' or 'A'
    // auto count = std::count_if(words.begin(), words.end(), ???);
    // assert(count == 3);  // apple, avocado, Apricot

    std::cout << "  Exercise 1: STL lambdas — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Capture semantics
//
// Predict the output. Then uncomment and verify.
// ──────────────────────────────────────────────────────────────────────────
void exercise_captures()
{
    // int x = 10;
    // auto by_val = [x]() mutable { x += 5; return x; };
    // auto r1 = by_val();   // What is r1? ___
    // auto r2 = by_val();   // What is r2? ___
    // // What is x now? ___
    //
    // assert(r1 == 15);
    // assert(r2 == 20);
    // assert(x == 10);     // original unchanged!

    std::cout << "  Exercise 2: captures — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Design a non-copyable, non-convertible class
//
// Create a `UniqueHandle` class:
//   - Holds an int "handle" (like a file descriptor)
//   - Constructor takes int
//   - Delete copy ctor and copy assign
//   - Allow move ctor and move assign
//   - Delete constructor from double (prevent implicit conversion)
// ──────────────────────────────────────────────────────────────────────────
// TODO: Define UniqueHandle
// class UniqueHandle { ... };

void exercise_delete()
{
    // UniqueHandle h1{42};
    // // UniqueHandle h2 = h1;           // Should NOT compile
    // // UniqueHandle h3{3.14};           // Should NOT compile (double deleted)
    // UniqueHandle h4 = std::move(h1);    // Should compile
    // assert(h4.get() == 42);
    // assert(h1.get() == -1);             // moved-from state

    std::cout << "  Exercise 3: = delete — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Delegating constructors
//
// Create a `Logger` class with three constructors:
//   Logger(string name, string filename, int level)  ← full
//   Logger(string name, string filename)             ← delegates, level=1
//   Logger(string name)                              ← delegates, filename="log.txt"
//
// All constructors should set a `ready_` flag to true via the full ctor.
// ──────────────────────────────────────────────────────────────────────────
// TODO: Define Logger
// class Logger { ... };

void exercise_delegating()
{
    // Logger l1("app", "app.log", 3);
    // Logger l2("db", "db.log");
    // Logger l3("net");
    // assert(l1.ready());
    // assert(l2.ready());
    // assert(l3.ready());
    // assert(l3.filename() == "log.txt");
    // assert(l2.level() == 1);

    std::cout << "  Exercise 4: delegating ctors — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Template aliases
//
// Create the following aliases:
//   a) `Pair<T>` → std::pair<T, T>
//   b) `Callback` → std::function<void(int)>
//   c) `Matrix` → std::vector<std::vector<double>>
// ──────────────────────────────────────────────────────────────────────────
// TODO: Define aliases

void exercise_aliases()
{
    // Pair<int> p{3, 4};
    // assert(p.first == 3 && p.second == 4);
    //
    // Callback cb = [](int x){ /* ... */ };
    //
    // Matrix m(3, std::vector<double>(3, 0.0));
    // assert(m.size() == 3 && m[0].size() == 3);

    std::cout << "  Exercise 5: aliases — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Mini Event System
//
// Build a simple EventBus:
//   - subscribe(event_name, callback) → returns subscription id
//   - emit(event_name, data) → calls all callbacks for that event
//   - unsubscribe(id) → removes a specific callback
//
// Use std::function, std::map, lambdas, using aliases.
// ──────────────────────────────────────────────────────────────────────────
// TODO: Define EventBus

void exercise_event_system()
{
    // EventBus bus;
    // int received = 0;
    // auto id = bus.subscribe("click", [&received](int x){ received = x; });
    // bus.emit("click", 42);
    // assert(received == 42);
    // bus.unsubscribe(id);
    // bus.emit("click", 99);
    // assert(received == 42);  // unchanged — unsubscribed

    std::cout << "  Exercise 6: event system — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔════════════════════════════════════════════════╗\n"
              << "║  Lecture 02 — Exercises: Functions & Lambdas   ║\n"
              << "╚════════════════════════════════════════════════╝\n\n";

    exercise_stl_lambdas();
    exercise_captures();
    exercise_delete();
    exercise_delegating();
    exercise_aliases();
    exercise_event_system();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
