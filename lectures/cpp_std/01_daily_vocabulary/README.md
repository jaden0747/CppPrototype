# Lecture 01 — Daily Vocabulary (C++11)

> **Goal:** Master the five features you'll use in *every single* C++ file you write.
> After this lecture you should be able to write clean, modern C++11 code without
> ever reaching for old-style casts, raw loops, or `NULL`.

---

## Table of Contents

1. [auto Type Deduction](#1-auto-type-deduction)
2. [Range-Based for Loop](#2-range-based-for-loop)
3. [nullptr](#3-nullptr)
4. [Brace Initialization (Uniform Init)](#4-brace-initialization-uniform-init)
5. [Scoped Enums (enum class)](#5-scoped-enums-enum-class)
6. [Exercises](#6-exercises)
7. [Capstone Mini-Project](#7-capstone-mini-project)
8. [Further Reading & Suggestions](#8-further-reading--suggestions)

---

## 1. `auto` Type Deduction

### What is it?

`auto` tells the compiler: *"Figure out the type for me."* You still get a
concrete, static type — it's just deduced from the initializer expression
instead of being spelled out by you.

### Why does it matter?

Before C++11, you wrote monstrosities like:

```cpp
std::map<std::string, std::vector<int>>::const_iterator it = m.begin();
```

Now you write:

```cpp
auto it = m.begin();   // same type, zero clutter
```

### The mental model

Think of `auto` as a *copy machine for types*. It looks at the right-hand side
of `=`, copies the type, and stamps it onto the variable. The compiler still
checks everything at compile time — there is **no** runtime cost, no dynamic
typing, no boxing.

### Rules you need to know

| Declaration          | Deduced type | Why                         |
| -------------------- | ------------ | --------------------------- |
| `auto x = 42;`       | `int`        | Integer literal → `int`     |
| `auto x = 42.0;`     | `double`     | Floating literal → `double` |
| `auto x = 42.0f;`    | `float`      | `f` suffix → `float`        |
| `const auto& r = x;` | `const int&` | You can add qualifiers      |
| `auto& r = x;`       | `int&`       | Reference to `x`            |
| `auto* p = &x;`      | `int*`       | Pointer to `x`              |

### Pitfall: `auto` drops top-level `const` and references

```cpp
const int ci = 10;
auto x = ci;          // x is `int`, NOT `const int` — the const is dropped
auto& y = ci;         // y is `const int&` — reference preserves const
```

**Rule of thumb:** If you want a reference, say `auto&`. If you want const,
say `const auto&`. Don't rely on `auto` alone to preserve qualifiers.

### When NOT to use `auto`

- When the type isn't obvious from context and readability suffers:
  ```cpp
  auto result = computeSomething();  // What type is result? Reader has to dig.
  double result = computeSomething(); // Now it's obvious.
  ```
- When you explicitly want a different type than what the initializer gives:
  ```cpp
  auto size = vec.size();     // std::size_t (unsigned)
  int  size = vec.size();     // narrowing — but intentional truncation
  ```

### Demo code

See `demo.cpp` in this folder — it demonstrates all the above.

---

## 2. Range-Based `for` Loop

### What is it?

A cleaner syntax for iterating over *anything* that has `begin()` and `end()`:

```cpp
for (auto& element : container) {
    // use element
}
```

### Why does it matter?

The old-style indexed loop is error-prone (off-by-one, wrong end condition)
and verbose. Range-for eliminates an entire class of bugs.

### The mental model

Think of it as *"for each thing in the collection, do this."* If you've used
Python's `for x in list:`, it's the same idea.

### The three forms

```cpp
std::vector<std::string> names = {"Alice", "Bob", "Carol"};

// 1. By value (copies each element — use for cheap-to-copy types)
for (auto name : names) { ... }

// 2. By const reference (read-only, no copies)
for (const auto& name : names) { ... }

// 3. By reference (modify elements in-place)
for (auto& name : names) { name += "!"; }
```

**Best practice:** Default to `const auto&` unless you need to modify.

### What can you iterate?

- `std::vector`, `std::array`, `std::list`, `std::map`, `std::set`, …
- Raw C arrays: `int arr[] = {1,2,3}; for (auto x : arr) { ... }`
- `std::string` (iterates characters)
- Any class with `begin()` and `end()` returning iterators
- `std::initializer_list<T>`

### Pitfall: don't modify the container during iteration

```cpp
for (auto& x : vec) {
    vec.push_back(x);  // UNDEFINED BEHAVIOUR — iterator invalidation!
}
```

---

## 3. `nullptr`

### What is it?

A keyword that represents a null pointer. It replaces the old `NULL` macro
(which was just `0` or `(void*)0`).

### Why does it matter?

`NULL` is an *integer*, not a pointer. This causes real bugs:

```cpp
void foo(int x);
void foo(int* p);

foo(NULL);      // Calls foo(int) — surprise! NULL is 0.
foo(nullptr);   // Calls foo(int*) — correct.
```

### The mental model

`nullptr` has its own type: `std::nullptr_t`. It implicitly converts to any
pointer type, but it does NOT convert to `int`. This makes overload resolution
work correctly.

### Rules

```cpp
int* p = nullptr;       // OK
int  n = nullptr;       // ERROR — nullptr is not an integer
if (p == nullptr) {}    // OK — the idiomatic null check
if (!p) {}              // Also OK — pointer converts to bool
```

**Rule of thumb:** Never write `NULL` or `0` for pointers in C++11 or later.
Always use `nullptr`.

---

## 4. Brace Initialization (Uniform Init)

### What is it?

C++11 lets you use `{}` to initialize *anything*:

```cpp
int x{42};                               // scalar
std::vector<int> v{1, 2, 3};             // container
std::pair<int, double> p{1, 2.5};        // aggregate
MyClass obj{arg1, arg2};                 // class with constructor
```

### Why does it matter?

Before C++11, initialization syntax was inconsistent:
- `int x = 42;` (copy-init)
- `int x(42);` (direct-init — but also the Most Vexing Parse!)
- `int arr[] = {1, 2, 3};` (aggregate-init, only for arrays)

Braces unify all of these. And they have one huge advantage:

### Narrowing prevention

```cpp
int x = 7.5;    // Compiles! Silently truncates to 7.
int x{7.5};     // ERROR — narrowing conversion not allowed.
```

This catches bugs at compile time that would otherwise silently corrupt data.

### The Most Vexing Parse — solved

```cpp
// Old C++ — this declares a function, not an object!
Widget w();        // Function returning Widget (Most Vexing Parse)

// Braces — always an object
Widget w{};        // Default-constructed Widget, guaranteed.
```

### `std::initializer_list`

When a class has a constructor that accepts `std::initializer_list<T>`, braces
will prefer that constructor:

```cpp
std::vector<int> v{10};     // vector with ONE element: 10
std::vector<int> v(10);     // vector with TEN default elements
```

This is the **one gotcha** with braces. Know it, respect it.

### When to use `()` vs `{}`

| Situation                                          | Use         |
| -------------------------------------------------- | ----------- |
| Prevent narrowing                                  | `{}` always |
| Construct with `initializer_list` semantics        | `{}`        |
| Construct with count/value (e.g., `vector(10, 0)`) | `()`        |
| Default-construct to avoid Most Vexing Parse       | `{}`        |

---

## 5. Scoped Enums (`enum class`)

### What is it?

A strongly-typed, scoped enumeration:

```cpp
enum class Color { Red, Green, Blue };
Color c = Color::Red;
```

### Why does it matter?

Old-style `enum` pollutes the enclosing scope and implicitly converts to `int`:

```cpp
enum Color { Red, Green, Blue };
enum TrafficLight { Red, Yellow, Green };  // ERROR — Red, Green already defined!

int x = Red;    // Compiles — implicit int conversion. Dangerous.
```

`enum class` fixes both problems:

```cpp
enum class Color { Red, Green, Blue };
enum class TrafficLight { Red, Yellow, Green };  // Fine — separate scopes

int x = Color::Red;   // ERROR — no implicit conversion to int
int x = static_cast<int>(Color::Red);  // OK — explicit conversion
```

### Choosing the underlying type

```cpp
enum class ErrorCode : uint8_t {
    OK = 0,
    NotFound = 1,
    Timeout = 2
};
// sizeof(ErrorCode) == 1
```

### Forward declaration

```cpp
enum class Status : int;  // forward-declare with underlying type
// ... later ...
enum class Status : int { Active, Inactive };
```

This is impossible with old `enum` (the compiler doesn't know the size).

---

## 6. Exercises

Each exercise has a **goal**, **hints**, and an **expected output**. Try to
solve it yourself before looking at the hints. The exercise files are in this
directory with `_exercises` suffix.

### Exercise 1 — Type Detective

**Goal:** Predict the type deduced by `auto` for each variable, then verify
using `typeid().name()` or `static_assert`.

```
Difficulty: ★☆☆☆☆ (Warm-up)
```

**Hints:**
1. Remember that `auto` drops top-level `const` and references.
2. Use `std::is_same<decltype(var), expected_type>::value` in a `static_assert`
   to check at compile time.
3. Integer literals are `int`, floating literals are `double`, string literals
   are `const char*` (NOT `std::string`).

---

### Exercise 2 — Container Stats

**Goal:** Read a `std::vector<double>` of grades, then compute and print:
- The number of grades
- The average
- The highest grade
- The number of passing grades (≥ 60.0)

Use `auto`, range-for, and `nullptr` for an optional "not found" message.

```
Difficulty: ★★☆☆☆
```

**Hints:**
1. Use `const auto&` in range-for to avoid copies.
2. Track the max with a pointer (`double* best = nullptr;`). After the loop,
   check `if (best != nullptr)` before printing.
3. The average is `sum / static_cast<double>(count)`.

---

### Exercise 3 — Enum-Based State Machine

**Goal:** Implement a simple traffic light state machine using `enum class`.
States: `Red`, `Green`, `Yellow`. Transitions: Red→Green→Yellow→Red.
Print each state and transition 2 full cycles.

```
Difficulty: ★★☆☆☆
```

**Hints:**
1. Write a `nextState(TrafficLight)` function that returns the next state.
2. Write a `toString(TrafficLight)` function using a `switch`.
3. Use a loop that calls `nextState` repeatedly.

---

### Exercise 4 — Safe Initialization

**Goal:** Demonstrate narrowing prevention. Create a struct `Pixel` with
`uint8_t r, g, b` fields. Try to initialize it with values that would narrow
(e.g., `int` values > 255 or negative). Observe the compiler errors when using
`{}` vs `()`.

```
Difficulty: ★★☆☆☆
```

**Hints:**
1. Aggregate initialization: `Pixel p{r, g, b};`
2. Try `Pixel p{-1, 256, 0};` — both should fail with braces.
3. Try the same with `=` — it might silently truncate.

---

### Exercise 5 — Word Frequency Counter

**Goal:** Read words from a hardcoded string, count occurrences using
`std::map<std::string, int>`, and print the top 3 most frequent words.

Use ALL five features from this lecture.

```
Difficulty: ★★★☆☆
```

**Hints:**
1. Use `std::istringstream` to split the string into words.
2. Use `auto&` in range-for over the map.
3. Use brace init for the map and vector.
4. Use `enum class SortOrder { Ascending, Descending }` to control output order.
5. Move results into a `std::vector<std::pair<std::string, int>>` and sort with
   a lambda (preview of Lecture 02!).

---

### Exercise 6 — Matrix Printer

**Goal:** Create a 2D matrix (vector of vectors), fill it with values, and
print it as a formatted grid. Use `auto` for type deduction, range-for for
iteration, and brace-init for construction.

```
Difficulty: ★★★☆☆
```

**Hints:**
1. `std::vector<std::vector<int>> matrix{{1,2,3},{4,5,6},{7,8,9}};`
2. Nested range-for: `for (const auto& row : matrix) for (const auto& val : row)`
3. Use `std::setw()` from `<iomanip>` for alignment.

---

### Exercise 7 — Null-Safe Linked List

**Goal:** Implement a simple singly-linked list using `struct Node` with a raw
`Node* next` pointer. Use `nullptr` throughout. Write `push_front`, `print`,
and `find(int value)` that returns `Node*` (or `nullptr` if not found).

```
Difficulty: ★★★★☆
```

**Hints:**
1. `struct Node { int data; Node* next = nullptr; };` — brace-init the default!
2. In `push_front`: allocate with `new`, set `next` to old head.
3. In `find`: walk the list with `while (current != nullptr)`.
4. In `main`: check `if (auto* found = find(head, 42); found != nullptr)` —
   wait, that's C++17! For now: `auto* found = find(head, 42); if (found) ...`
5. **Don't forget to free the list at the end** (we'll fix this with smart
   pointers in Lecture 03).

---

### Exercise 8 — Compile-Time Enum Map

**Goal:** Create an `enum class Direction { North, South, East, West }`.
Write a function `opposite(Direction)` that returns the opposite direction.
Write a function `toString(Direction)`. Demonstrate that `Direction` does NOT
implicitly convert to `int`.

```
Difficulty: ★★☆☆☆
```

**Hints:**
1. Use `switch` with no `default` — the compiler warns about unhandled cases.
2. `static_cast<int>(Direction::North)` for explicit conversion.
3. Show that `int x = Direction::North;` fails to compile.

---

## 7. Capstone Mini-Project

### Student Grade Analyzer

Build a complete program that:

1. Stores student records in a `std::vector<Student>` where `Student` is:
   ```cpp
   enum class Grade : uint8_t { A, B, C, D, F };
   struct Student { std::string name; double score; Grade grade; };
   ```
2. Uses brace-initialization to populate the list.
3. Prints all students using range-for with `const auto&`.
4. Finds the top student (use `nullptr` for "empty list" case).
5. Groups students by `Grade` using `std::map<Grade, std::vector<Student*>>`.
6. Prints a summary table.

**This project exercises ALL five features from this lecture.**

Build it as `lecture01_capstone` (see the CMakeLists.txt).

---

## 8. Further Reading & Suggestions

- **cppreference:** [auto](https://en.cppreference.com/w/cpp/language/auto),
  [range-for](https://en.cppreference.com/w/cpp/language/range-for),
  [nullptr](https://en.cppreference.com/w/cpp/language/nullptr),
  [list initialization](https://en.cppreference.com/w/cpp/language/list_initialization),
  [enum](https://en.cppreference.com/w/cpp/language/enum)
- **Book:** *A Tour of C++* by Bjarne Stroustrup, Chapters 1–3
- **Video:** Jason Turner — "C++ Weekly Ep 1: auto" (YouTube)
- **Next lecture:** Lecture 02 builds on these foundations with lambdas,
  `default`/`delete`, delegating constructors, and type aliases.

> **Tip:** Before moving on, make sure you can write a 50-line program using
> all five features without looking anything up. That's your graduation test
> for this lecture.
