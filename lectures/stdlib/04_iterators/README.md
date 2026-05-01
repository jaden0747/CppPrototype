# Stdlib 04 — Iterators

> **Goal:** Understand what iterators are, why they exist, and how they connect
> containers to algorithms. Master iterator categories, adaptors, traits, and
> learn to write your own custom iterator.

---

## Table of Contents

1. [What Are Iterators and Why Do They Exist?](#1-what-are-iterators-and-why-do-they-exist)
2. [Iterator Categories (The Hierarchy)](#2-iterator-categories-the-hierarchy)
3. [Range Access Functions](#3-range-access-functions)
4. [Iterator Navigation: `advance`, `distance`, `next`, `prev`](#4-iterator-navigation)
5. [Insert Iterators (Output Adaptors)](#5-insert-iterators-output-adaptors)
6. [Stream Iterators](#6-stream-iterators)
7. [`std::iterator_traits`](#7-stditerator_traits)
8. [Writing a Custom Iterator](#8-writing-a-custom-iterator)
9. [Common Pitfalls](#9-common-pitfalls)
10. [Exercises](#10-exercises)

---

## 1. What Are Iterators and Why Do They Exist?

### The Problem

Imagine you want to write a `find` function. Without iterators, you'd need
a different version for every container:

```cpp
int* find_in_vector(std::vector<int>& v, int target);
int* find_in_list(std::list<int>& l, int target);
int* find_in_array(int arr[], int n, int target);
```

That's 3 functions doing the same thing. With 10 containers × 50 algorithms,
you'd need 500 functions.

### The Solution: Iterators

Iterators are **the glue** between containers and algorithms. They provide a
uniform way to traverse any container:

```
Container ←→ Iterator ←→ Algorithm

vector  → random_access_iterator → sort, find, binary_search
list    → bidirectional_iterator → find, reverse, merge
forward_list → forward_iterator  → find, remove, unique
```

With iterators: 10 containers + 50 algorithms = **60 things to write**
(not 500). This is the "separation of concerns" that makes the STL powerful.

### The mental model

An iterator is like a **cursor** or **finger pointing at an element**:

```
vector: [10, 20, 30, 40, 50]
              ^
              iterator pointing at 20

*it    → 20         (dereference: read the value)
++it   → moves to 30 (advance to next)
it == v.end()  → false (not past the end yet)
```

---

## 2. Iterator Categories (The Hierarchy)

Not all iterators are equally powerful. They form a hierarchy of **capabilities**:

```
Output Iterator (write-only)
Input Iterator (read-only, single-pass)
  ↓
Forward Iterator (multi-pass, read/write)
  ↓
Bidirectional Iterator (can go backwards: --)
  ↓
Random Access Iterator (can jump: +n, -n, [n])
  ↓
Contiguous Iterator (C++17: elements are adjacent in memory)
```

### What each category supports

| Category | Operations | Example containers |
|----------|-----------|-------------------|
| **Input** | `*it` (read), `++it`, `==` | `istream_iterator` |
| **Output** | `*it = val` (write), `++it` | `ostream_iterator`, `back_inserter` |
| **Forward** | All of input + multi-pass | `forward_list`, `unordered_set` |
| **Bidirectional** | Forward + `--it` | `list`, `set`, `map` |
| **Random Access** | Bidirectional + `it+n`, `it-n`, `it[n]`, `<` | `vector`, `deque`, `array` |
| **Contiguous** | Random access + adjacent memory guarantee | `vector`, `array`, `string` |

### Why this matters

Algorithms declare their **minimum** iterator requirement:

```cpp
std::sort(first, last);      // requires RandomAccess (can't sort a list!)
std::find(first, last, val); // requires Input (works with anything)
std::reverse(first, last);   // requires Bidirectional (needs --)
```

If you pass the wrong iterator type, you get a **compile error**.

---

## 3. Range Access Functions

### The problem with member `begin()/end()`

C arrays don't have `.begin()` and `.end()`. Free functions solve this:

```cpp
int arr[] = {1, 2, 3, 4, 5};

// These work with arrays AND containers:
std::begin(arr);   // &arr[0]
std::end(arr);     // &arr[5] (past-the-end)
std::size(arr);    // 5

std::vector<int> v{1, 2, 3};
std::begin(v);     // v.begin()
std::end(v);       // v.end()
```

### Full list of range access functions

| Function | What it returns |
|----------|----------------|
| `std::begin(c)` / `std::end(c)` | Iterator to first / past-the-end |
| `std::cbegin(c)` / `std::cend(c)` | `const` iterator (read-only) |
| `std::rbegin(c)` / `std::rend(c)` | Reverse iterator (last → first) |
| `std::size(c)` | Number of elements |
| `std::ssize(c)` (C++20) | Signed size (avoids signed/unsigned warnings) |
| `std::empty(c)` | `true` if container is empty |
| `std::data(c)` | Pointer to underlying contiguous storage |

---

## 4. Iterator Navigation

### `std::advance(it, n)` — move iterator by n steps

```cpp
std::list<int> l{10, 20, 30, 40, 50};
auto it = l.begin();
std::advance(it, 3);   // it now points to 40
// For list: O(n) — walks step by step
// For vector: O(1) — uses pointer arithmetic
```

**Note:** `advance` modifies the iterator **in-place**. It returns void.

### `std::next(it, n)` / `std::prev(it, n)` — return a new iterator

```cpp
auto it = std::next(l.begin(), 2);   // points to 30 (returns new iterator)
auto it2 = std::prev(l.end());       // points to 50
```

**Prefer `next`/`prev` over `advance`** when you want to keep the original
iterator unchanged.

### `std::distance(first, last)` — count elements between iterators

```cpp
auto d = std::distance(l.begin(), l.end());  // 5
```

---

## 5. Insert Iterators (Output Adaptors)

### The problem

Many algorithms write to an output range. But what if the destination is empty?

```cpp
std::vector<int> dest;
std::copy(src.begin(), src.end(), dest.begin()); // 💥 UB! dest has no elements!
```

### The solution: insert iterators

These automatically call `push_back`, `push_front`, or `insert` on the container:

```cpp
// back_inserter — calls push_back
std::vector<int> dest;
std::copy(src.begin(), src.end(), std::back_inserter(dest));

// front_inserter — calls push_front (deque, list only)
std::deque<int> dq;
std::copy(src.begin(), src.end(), std::front_inserter(dq));

// inserter — calls insert at a given position
std::list<int> lst{100, 200};
std::copy(src.begin(), src.end(), std::inserter(lst, std::next(lst.begin())));
// result: {100, 1, 2, 3, ..., 200}
```

---

## 6. Stream Iterators

### Reading from a stream

```cpp
std::istringstream iss("10 20 30 40 50");
std::vector<int> v{std::istream_iterator<int>(iss),
                    std::istream_iterator<int>()};
// v = {10, 20, 30, 40, 50}
```

The default-constructed `istream_iterator` is the **end sentinel** (like EOF).

### Writing to a stream

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, ", "));
// prints: 1, 2, 3, 4, 5,
```

---

## 7. `std::iterator_traits`

### What is it?

`iterator_traits<It>` is a struct that tells you about an iterator's properties:

```cpp
using traits = std::iterator_traits<std::vector<int>::iterator>;

traits::value_type;        // int
traits::difference_type;   // ptrdiff_t
traits::pointer;           // int*
traits::reference;         // int&
traits::iterator_category; // std::random_access_iterator_tag
```

### Why use it?

To write **generic algorithms** that adapt behavior based on iterator category:

```cpp
template<typename It>
void my_advance(It& it, int n) {
    using category = typename std::iterator_traits<It>::iterator_category;

    if constexpr (std::is_same_v<category, std::random_access_iterator_tag>)
        it += n;                    // O(1)
    else
        for (int i = 0; i < n; ++i) ++it;  // O(n)
}
```

---

## 8. Writing a Custom Iterator

### When do you need one?

When you have a custom data structure and want it to work with STL algorithms
and range-based for loops.

### Minimum requirements for a forward iterator

Your iterator struct needs 5 type aliases + these operators:

```cpp
struct MyIterator {
    // Required type aliases
    using iterator_category = std::forward_iterator_tag;
    using value_type        = int;
    using difference_type   = std::ptrdiff_t;
    using pointer           = int*;
    using reference         = int&;

    // Dereference
    int& operator*() const;

    // Advance
    MyIterator& operator++();       // pre-increment
    MyIterator  operator++(int);    // post-increment

    // Comparison
    bool operator==(const MyIterator&) const;
    bool operator!=(const MyIterator&) const;
};
```

### Example: IntRange — generates integers lazily

```cpp
class IntRange {
    int start_, end_;
public:
    IntRange(int s, int e) : start_(s), end_(e) {}

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using difference_type = std::ptrdiff_t;
        using pointer = const int*;
        using reference = int;

        int current;
        int  operator*() const { return current; }
        Iterator& operator++() { ++current; return *this; }
        Iterator  operator++(int) { auto t = *this; ++current; return t; }
        bool operator==(const Iterator& o) const { return current == o.current; }
        bool operator!=(const Iterator& o) const { return current != o.current; }
    };

    Iterator begin() const { return {start_}; }
    Iterator end()   const { return {end_}; }
};

// Usage:
for (int x : IntRange(1, 11))
    std::cout << x << " ";  // prints 1 2 3 4 5 6 7 8 9 10
```

---

## 9. Common Pitfalls

### 1. Iterator invalidation

Modifying a container can invalidate existing iterators:

```cpp
std::vector<int> v{1, 2, 3};
auto it = v.begin();
v.push_back(4);    // ⚠️ may reallocate — it is now DANGLING!
*it;               // 💥 undefined behavior
```

**Fix:** Re-acquire iterators after modifications, or use `reserve()`.

### 2. Most Vexing Parse with `istream_iterator`

```cpp
// This declares a FUNCTION, not a vector! (most vexing parse)
std::vector<int> v(std::istream_iterator<int>(iss),
                   std::istream_iterator<int>());

// Fix: use braces
std::vector<int> v{std::istream_iterator<int>(iss),
                   std::istream_iterator<int>()};
```

### 3. Off-by-one with `end()`

`end()` points **past the last element**. Dereferencing it is undefined behavior.

---

## 10. Exercises

See `exercises.cpp`.

---

**Next lecture:** Ranges (C++20).
