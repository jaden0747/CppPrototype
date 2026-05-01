// ============================================================================
// Template 01 — Exercises: Function & Class Templates
// ============================================================================
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────────────────────────────────
// Exercise 1: Write a function template `min_of(a, b)` that returns the
// smaller value. Test with int, double, and string.
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> T min_of(T a, T b) { ... }

void exercise_min_of()
{
    // assert(min_of(3, 7) == 3);
    // assert(min_of(2.5, 1.5) == 1.5);
    // assert(min_of(std::string("apple"), std::string("banana")) == "apple");
    std::cout << "  Exercise 1: min_of — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 2: Write a function template `clamp(val, lo, hi)` that clamps
// val into the range [lo, hi].
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> T clamp(T val, T lo, T hi) { ... }

void exercise_clamp()
{
    // assert(clamp(5, 1, 10) == 5);
    // assert(clamp(-3, 0, 100) == 0);
    // assert(clamp(999, 0, 100) == 100);
    // assert(clamp(3.14, 0.0, 1.0) == 1.0);
    std::cout << "  Exercise 2: clamp — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 3: Write a class template `Pair<T, U>` with:
//   - Public members `first` and `second`
//   - A constructor taking (T, U)
//   - A `swap()` method (only valid when T == U)
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T, typename U> class Pair { ... };

void exercise_pair()
{
    // Pair<int, double> p1(42, 3.14);
    // assert(p1.first == 42);
    // assert(p1.second == 3.14);
    //
    // Pair<int, int> p2(1, 2);
    // p2.swap();
    // assert(p2.first == 2 && p2.second == 1);
    std::cout << "  Exercise 3: Pair — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 4: Write a class template `Stack<T>` with:
//   push, pop, top, empty, size
// Then add a member template `contains(U val)` that returns true if
// the stack contains a value equal to val (comparing via ==).
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T> class Stack { ... };

void exercise_stack()
{
    // Stack<int> s;
    // assert(s.empty());
    // s.push(10); s.push(20); s.push(30);
    // assert(s.size() == 3);
    // assert(s.top() == 30);
    // s.pop();
    // assert(s.top() == 20);
    // assert(s.contains(10));
    // assert(!s.contains(99));
    std::cout << "  Exercise 4: Stack — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 5: Write a function template with two template params:
//   template<typename R, typename T> R narrow_cast(T val)
// that static_casts val to R and asserts the value survives the round-trip:
//   assert(static_cast<T>(static_cast<R>(val)) == val)
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename R, typename T> R narrow_cast(T val) { ... }

void exercise_narrow_cast()
{
    // assert(narrow_cast<int>(42.0) == 42);
    // assert(narrow_cast<short>(100) == 100);
    // narrow_cast<int>(3.14);  // should assert fail: 3 != 3.14
    std::cout << "  Exercise 5: narrow_cast — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
// Exercise 6 (Challenge): Write a class template `Matrix<T, Rows, Cols>`
// with default T=double, Rows=3, Cols=3. Support:
//   - operator()(row, col) for element access
//   - fill(value) to fill all elements
//   - A member template `cast<U>()` that returns Matrix<U, Rows, Cols>
// ──────────────────────────────────────────────────────────────────────────
// TODO: template<typename T = double, int Rows = 3, int Cols = 3>
//       class Matrix { ... };

void exercise_matrix()
{
    // Matrix<> m;  // double, 3x3
    // m.fill(0.0);
    // m(1, 2) = 3.14;
    // assert(m(1, 2) == 3.14);
    // assert(m(0, 0) == 0.0);
    //
    // auto mi = m.cast<int>();
    // assert(mi(1, 2) == 3);
    std::cout << "  Exercise 6: Matrix — PASSED\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Template 01 — Exercises: Function & Class       ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    exercise_min_of();
    exercise_clamp();
    exercise_pair();
    exercise_stack();
    exercise_narrow_cast();
    exercise_matrix();

    std::cout << "\nAll exercises done!\n";
    return 0;
}
