# Stdlib 05 — Ranges (C++20/23)

> **Goal:** Master the C++20 Ranges library — views, adaptors, projections,
> and lazy pipelines. Understand how ranges supersede iterator pairs and
> enable a functional programming style in C++.

---

## Table of Contents

1. [What Are Ranges and Why Were They Added?](#1-what-are-ranges-and-why-were-they-added)
2. [Range Concepts](#2-range-concepts)
3. [Range-Based Algorithms](#3-range-based-algorithms)
4. [Views — Lazy, Composable Adaptors](#4-views--lazy-composable-adaptors)
5. [View Composition with the Pipe Operator](#5-view-composition-with-the-pipe-operator)
6. [Common Views Reference](#6-common-views-reference)
7. [Projections — Sort/Find by Member](#7-projections--sort-find-by-member)
8. [Borrowed Ranges and Dangling](#8-borrowed-ranges-and-dangling)
9. [Practical Examples](#9-practical-examples)
10. [Exercises](#10-exercises)

---

## 1. What Are Ranges and Why Were They Added?

### The Problem with Iterator Pairs

Traditional STL algorithms require **two iterators** (begin and end).
This is verbose, error-prone, and makes composition awkward:

```cpp
// Old way: verbose, easy to mismatch begin/end
std::vector<int> v{5, 3, 1, 4, 2};
std::sort(v.begin(), v.end());
auto it = std::find(v.begin(), v.end(), 3);

// Composing operations is ugly
std::vector<int> temp;
std::copy_if(v.begin(), v.end(), std::back_inserter(temp),
             [](int x) { return x > 2; });
std::transform(temp.begin(), temp.end(), temp.begin(),
               [](int x) { return x * x; });
```

### The Solution: Ranges

A **range** is anything with `begin()` and `end()`. The Ranges library lets
you pass the container directly and compose operations with pipes:

```cpp
// New way: clean, readable, composable
std::ranges::sort(v);
auto it = std::ranges::find(v, 3);

// Composition with pipes — no temporary containers!
auto result = v | std::views::filter([](int x) { return x > 2; })
                | std::views::transform([](int x) { return x * x; });
// result is a lazy VIEW — no work done until iterated!
```

### The Key Insight: Laziness

Views don't compute anything until you iterate them. They just describe
a transformation pipeline. This means:
- **No intermediate containers** (saves memory)
- **Work is done element-by-element** as you iterate
- **You can compose infinitely** and only pull what you need

---

## 2. Range Concepts

C++20 defines range concepts in a hierarchy, similar to iterator categories:

```
range              ← has begin() and end()
  ↓
input_range        ← can read elements
  ↓
forward_range      ← can iterate multiple times
  ↓
bidirectional_range← can go backwards
  ↓
random_access_range← can jump by index
  ↓
contiguous_range   ← elements are adjacent in memory
```

### How to check

```cpp
static_assert(std::ranges::range<std::vector<int>>);           // ✅
static_assert(std::ranges::random_access_range<std::vector<int>>);  // ✅
static_assert(std::ranges::contiguous_range<std::vector<int>>);     // ✅
static_assert(std::ranges::bidirectional_range<std::list<int>>);    // ✅
static_assert(!std::ranges::random_access_range<std::list<int>>);   // ✅ list can't jump
```

### `sized_range` and `common_range`

```cpp
// sized_range: knows its size in O(1)
static_assert(std::ranges::sized_range<std::vector<int>>);     // ✅

// common_range: begin and end have the SAME type
// (some ranges use sentinel types for end, making them not common)
```

---

## 3. Range-Based Algorithms

Every classic algorithm in `<algorithm>` has a `std::ranges::` version that
accepts a whole range instead of iterator pairs:

```cpp
std::vector<int> v{5, 3, 1, 4, 2};

// Classic (iterator pair)           →  Ranges (whole container)
std::sort(v.begin(), v.end());       →  std::ranges::sort(v);
std::find(v.begin(), v.end(), 3);    →  std::ranges::find(v, 3);
std::reverse(v.begin(), v.end());    →  std::ranges::reverse(v);
std::count(v.begin(), v.end(), 3);   →  std::ranges::count(v, 3);
```

### Advantages of range algorithms

1. **Shorter code** — pass the container, not `.begin()/.end()`
2. **Safer** — can't accidentally mismatch begin/end from different containers
3. **Projections** — built-in support (see section 7)
4. **Better error messages** — constrained with concepts

---

## 4. Views — Lazy, Composable Adaptors

### What is a view?

A view is a **lightweight, non-owning, lazy** adaptor over a range. It:
- Does **not own** the data (just holds a reference)
- Is **lazy** — no work until you iterate
- Is **cheap to copy** (O(1) — it's just a reference + state)

### Basic view examples

```cpp
std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// filter: keep only elements matching predicate
auto evens = v | std::views::filter([](int x) { return x % 2 == 0; });
// evens "contains" {2, 4, 6, 8, 10} — but nothing is computed yet!

// transform: apply function to each element
auto squared = v | std::views::transform([](int x) { return x * x; });
// squared "contains" {1, 4, 9, 16, ..., 100}

// take: first N elements
auto first3 = v | std::views::take(3);
// first3 = {1, 2, 3}

// drop: skip first N elements
auto after3 = v | std::views::drop(3);
// after3 = {4, 5, 6, 7, 8, 9, 10}
```

### Lazy evaluation demonstrated

```cpp
auto pipeline = v | std::views::filter([](int x) {
                        std::cout << "filter " << x << "\n";
                        return x % 2 == 0;
                    })
                  | std::views::transform([](int x) {
                        std::cout << "transform " << x << "\n";
                        return x * 10;
                    })
                  | std::views::take(2);

// Nothing printed yet! The pipeline is just a description.

for (int x : pipeline) {
    std::cout << "got " << x << "\n";
}
// Output:
// filter 1          ← rejected
// filter 2          ← passed
// transform 2       ← transformed
// got 20            ← first result
// filter 3          ← rejected
// filter 4          ← passed
// transform 4       ← transformed
// got 40            ← second result (take(2) stops here!)
// Remaining elements (5-10) are NEVER touched!
```

---

## 5. View Composition with the Pipe Operator

### The `|` operator

Views compose left-to-right using `|`, like Unix pipes:

```cpp
auto result = data
    | std::views::filter(predicate)    // step 1: keep matching
    | std::views::transform(func)      // step 2: transform each
    | std::views::take(10);            // step 3: only first 10
```

This is equivalent to:

```cpp
auto result = std::views::take(
    std::views::transform(
        std::views::filter(data, predicate),
        func),
    10);
```

The pipe version is **far more readable**.

### Creating reusable view adaptors

```cpp
// You can store partial views for reuse:
auto top5_even = std::views::filter([](int x) { return x % 2 == 0; })
               | std::views::take(5);

// Apply to any range:
for (int x : some_vector | top5_even) { ... }
for (int x : some_array  | top5_even) { ... }
```

---

## 6. Common Views Reference

| View | What it does | Example |
|------|-------------|---------|
| `filter(pred)` | Keep elements where pred is true | `v \| filter(is_even)` |
| `transform(f)` | Apply f to each element | `v \| transform(square)` |
| `take(n)` | First n elements | `v \| take(5)` |
| `drop(n)` | Skip first n elements | `v \| drop(3)` |
| `reverse` | Reverse order | `v \| reverse` |
| `take_while(pred)` | Take while pred is true | `v \| take_while(positive)` |
| `drop_while(pred)` | Drop while pred is true | `v \| drop_while(negative)` |
| `split(delim)` | Split into sub-ranges | `str \| split(',')` |
| `join` | Flatten nested ranges | `vv \| join` |
| `keys` / `values` | Get keys/values from pairs | `map \| values` |
| `iota(start)` | Infinite sequence: start, start+1, ... | `iota(0) \| take(10)` |
| `iota(start, end)` | Bounded sequence | `iota(1, 11)` |
| `zip` (C++23) | Pair up elements from multiple ranges | `zip(names, scores)` |
| `enumerate` (C++23) | Add index to each element | `v \| enumerate` |

### `std::views::iota` — lazy number generator

```cpp
// Infinite sequence (use take to limit!)
for (int x : std::views::iota(1) | std::views::take(5))
    std::cout << x << " ";  // 1 2 3 4 5

// Bounded sequence
for (int x : std::views::iota(1, 11))
    std::cout << x << " ";  // 1 2 3 4 5 6 7 8 9 10

// Replace manual loops:
// Old: for (int i = 0; i < n; ++i)
// New: for (int i : std::views::iota(0, n))
```

---

## 7. Projections — Sort/Find by Member

### The problem

Sorting structs by a member used to require verbose lambdas:

```cpp
struct Student { std::string name; double gpa; };
std::vector<Student> students = ...;

// Old way: need a custom comparator
std::sort(students.begin(), students.end(),
          [](const Student& a, const Student& b) { return a.gpa < b.gpa; });
```

### The solution: projections

Range algorithms accept a **projection** — a function applied to each element
before comparison:

```cpp
// Sort by GPA:
std::ranges::sort(students, {}, &Student::gpa);
//                           ^^  ^^^^^^^^^^^^^^^^
//                      comparator  projection
//                      (default <)  (which member)

// Sort by name descending:
std::ranges::sort(students, std::ranges::greater{}, &Student::name);

// Find by name:
auto it = std::ranges::find(students, "Alice", &Student::name);

// Max element by GPA:
auto best = std::ranges::max_element(students, {}, &Student::gpa);
```

**Why projections are better than lambdas:**
- Less boilerplate
- Clear intent (you see the member name directly)
- Composable with other range features

---

## 8. Borrowed Ranges and Dangling

### The problem: dangling iterators

What if you pass a temporary range to an algorithm?

```cpp
auto it = std::ranges::find(get_vector(), 42);
// The temporary vector is destroyed! it is a dangling iterator!
```

### How ranges protect you

Range algorithms return `std::ranges::dangling` instead of an iterator
when the input is a temporary that doesn't model `borrowed_range`:

```cpp
auto it = std::ranges::find(get_vector(), 42);
// it is std::ranges::dangling — can't dereference it!
// This is a COMPILE ERROR if you try to use it as an iterator.
```

### Borrowed ranges

Some ranges are safe to return iterators from even as temporaries:

```cpp
// span is borrowed (doesn't own data)
auto it = std::ranges::find(std::span(ptr, n), 42); // OK!

// subrange is borrowed
auto it2 = std::ranges::find(std::ranges::subrange(b, e), 42); // OK!
```

---

## 9. Practical Examples

### Example 1: Top 3 highest scores from CSV data

```cpp
struct Record { std::string name; int score; };
std::vector<Record> data = read_csv();

auto top3 = data
    | std::views::filter([](const Record& r) { return r.score > 0; })
    | std::views::transform([](const Record& r) { return r.score; });

// Note: to sort and take top 3, you'd need to materialize
// since sort modifies the range
std::ranges::sort(data, std::ranges::greater{}, &Record::score);
for (auto& r : data | std::views::take(3))
    std::cout << r.name << ": " << r.score << "\n";
```

### Example 2: FizzBuzz with ranges

```cpp
for (int i : std::views::iota(1, 101)) {
    if (i % 15 == 0) std::cout << "FizzBuzz\n";
    else if (i % 3 == 0) std::cout << "Fizz\n";
    else if (i % 5 == 0) std::cout << "Buzz\n";
    else std::cout << i << "\n";
}
```

### Example 3: Extract even numbers, square them, take first 5

```cpp
std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

auto result = v
    | std::views::filter([](int x) { return x % 2 == 0; })
    | std::views::transform([](int x) { return x * x; })
    | std::views::take(5);

for (int x : result)
    std::cout << x << " ";  // 4 16 36 64 100
```

---

## 10. Exercises

See `exercises.cpp`.

---

**Next lecture:** Non-Modifying Algorithms.
