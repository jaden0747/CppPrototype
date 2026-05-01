# Stdlib 07 — Modifying & Sorting Algorithms

> **Goal:** Master algorithms that **change** sequences: copying, transforming,
> filling, removing, replacing, reordering, and sorting. Understand the
> erase-remove idiom and when to use each sorting variant.

---

## Table of Contents

1. [Modifying vs Non-Modifying — What's the Difference?](#1-modifying-vs-non-modifying)
2. [Copying: `copy`, `copy_if`, `copy_n`](#2-copying)
3. [Transforming: `transform`](#3-transforming)
4. [Filling: `fill`, `generate`, `iota`](#4-filling)
5. [Removing: `remove`, `remove_if`, Erase-Remove Idiom](#5-removing)
6. [Replacing: `replace`, `replace_if`](#6-replacing)
7. [Reordering: `reverse`, `rotate`, `shuffle`](#7-reordering)
8. [`unique` — Remove Consecutive Duplicates](#8-unique)
9. [Sorting: `sort`, `stable_sort`, `partial_sort`, `nth_element`](#9-sorting)
10. [Partitioning: `partition`, `stable_partition`](#10-partitioning)
11. [Exercises](#11-exercises)

---

## 1. Modifying vs Non-Modifying

Non-modifying algorithms (lecture 06) only **read** elements. Modifying
algorithms **write** to the range — they copy elements, move them, fill
positions, or reorder them.

**Key rule:** Modifying algorithms **never change the size** of a container.
They can shuffle elements around, overwrite values, or move "unwanted"
elements to the end — but they can't call `push_back` or `erase`.

---

## 2. Copying

### `std::copy` — copy a range to a destination

```cpp
std::vector<int> src{1, 2, 3, 4, 5};
std::vector<int> dest(5); // must be pre-allocated!

std::copy(src.begin(), src.end(), dest.begin());
// dest = {1, 2, 3, 4, 5}

// Ranges version:
std::ranges::copy(src, dest.begin());
```

### `std::copy_if` — copy only elements matching a predicate

```cpp
std::vector<int> src{1, 2, 3, 4, 5, 6};
std::vector<int> evens;

std::ranges::copy_if(src, std::back_inserter(evens),
                     [](int x) { return x % 2 == 0; });
// evens = {2, 4, 6}
```

### `std::copy_n` — copy exactly N elements

```cpp
std::copy_n(src.begin(), 3, dest.begin());
// copies first 3 elements
```

### `std::copy_backward` — copy in reverse order

```cpp
// Useful when source and destination overlap and dest is AFTER source
std::vector<int> v{1, 2, 3, 4, 5, 0, 0};
std::copy_backward(v.begin(), v.begin() + 5, v.end());
// v = {1, 2, 1, 2, 3, 4, 5}
```

---

## 3. Transforming

### `std::transform` — apply a function to each element

This is the STL equivalent of "map" in functional programming.

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
std::vector<int> result(v.size());

// Unary transform: one input range
std::ranges::transform(v, result.begin(),
                       [](int x) { return x * x; });
// result = {1, 4, 9, 16, 25}

// In-place transform:
std::ranges::transform(v, v.begin(), [](int x) { return x * 2; });
// v = {2, 4, 6, 8, 10}
```

### Binary transform — combine two ranges

```cpp
std::vector<int> a{1, 2, 3};
std::vector<int> b{10, 20, 30};
std::vector<int> sum(3);

std::ranges::transform(a, b, sum.begin(), std::plus<>{});
// sum = {11, 22, 33}
```

### `transform` vs `for_each`

| Feature | `transform` | `for_each` |
|---------|------------|-----------|
| Writes output? | Yes, to output iterator | No (side effects only) |
| Returns? | Output iterator | The function object |
| Use for? | Creating new data | Printing, logging, etc. |

---

## 4. Filling

### `std::fill` / `std::fill_n` — set elements to a value

```cpp
std::vector<int> v(10);
std::ranges::fill(v, 42);
// v = {42, 42, 42, 42, 42, 42, 42, 42, 42, 42}

std::fill_n(v.begin(), 5, 0);
// v = {0, 0, 0, 0, 0, 42, 42, 42, 42, 42}
```

### `std::generate` — fill with function results

```cpp
int counter = 0;
std::ranges::generate(v, [&counter]() { return counter++; });
// v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

// Random values:
std::mt19937 gen(42);
std::uniform_int_distribution<> dist(1, 100);
std::ranges::generate(v, [&]() { return dist(gen); });
```

### `std::iota` — fill with incrementing values

```cpp
#include <numeric>
std::vector<int> v(5);
std::iota(v.begin(), v.end(), 10);
// v = {10, 11, 12, 13, 14}
```

---

## 5. Removing

### The erase-remove idiom (before C++20)

**Critical concept:** `std::remove` does NOT actually delete elements from the
container. It **moves** unwanted elements to the end and returns an iterator
to the new "logical end":

```cpp
std::vector<int> v{1, 2, 3, 2, 4, 2, 5};

auto new_end = std::remove(v.begin(), v.end(), 2);
// v is now: {1, 3, 4, 5, ?, ?, ?}
//                        ^ new_end
// The container still has 7 elements! The ? values are unspecified.

// You MUST erase the tail:
v.erase(new_end, v.end());
// v is now: {1, 3, 4, 5}
```

### Why does it work this way?

Algorithms don't know about containers — they only see iterators. An iterator
can't call `vector::erase()`. So `remove` does what it can: shifts good
elements forward and tells you where the junk starts.

### The one-liner (erase-remove idiom):

```cpp
v.erase(std::remove(v.begin(), v.end(), 2), v.end());
```

### C++20 `std::erase` / `std::erase_if` — the modern way

```cpp
// C++20: one function, no idiom needed!
std::erase(v, 2);           // remove all 2's
std::erase_if(v, [](int x) { return x % 2 == 0; }); // remove all evens
```

**Always prefer `std::erase`/`std::erase_if` in C++20 code.** They work for
`vector`, `deque`, `list`, `string`, `set`, `map`, and all other containers.

---

## 6. Replacing

### `std::replace` — replace all occurrences of a value

```cpp
std::vector<int> v{1, 2, 3, 2, 4, 2};
std::ranges::replace(v, 2, 99);
// v = {1, 99, 3, 99, 4, 99}
```

### `std::replace_if` — replace elements matching a predicate

```cpp
std::ranges::replace_if(v, [](int x) { return x < 3; }, 0);
// replaces all elements < 3 with 0
```

---

## 7. Reordering

### `std::reverse` — reverse element order

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
std::ranges::reverse(v);
// v = {5, 4, 3, 2, 1}
```

### `std::rotate` — rotate elements left

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
std::ranges::rotate(v, v.begin() + 2); // "2" becomes the new front
// v = {3, 4, 5, 1, 2}
```

**Use case:** Implementing a left-shift, moving an element to the front,
or circular buffer operations.

### `std::shuffle` — random reorder

```cpp
#include <random>
std::mt19937 gen(std::random_device{}());
std::ranges::shuffle(v, gen);
// v in random order
```

**Note:** Never use the old `std::random_shuffle` (removed in C++17).
Always use `std::shuffle` with a proper random engine.

---

## 8. `unique` — Remove Consecutive Duplicates

### How it works

`unique` removes **consecutive** equal elements (keeps the first of each run).
The range must be **sorted first** to remove all duplicates.

```cpp
std::vector<int> v{1, 1, 2, 2, 2, 3, 3, 4};

auto new_end = std::unique(v.begin(), v.end());
v.erase(new_end, v.end());
// v = {1, 2, 3, 4}
```

### The common pattern: sort + unique + erase

```cpp
std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
std::ranges::sort(v);          // {1, 1, 2, 3, 3, 4, 5, 5, 6, 9}
auto [new_end, _] = std::ranges::unique(v);
v.erase(new_end, v.end());    // {1, 2, 3, 4, 5, 6, 9}
```

---

## 9. Sorting

### `std::sort` — fastest general-purpose sort

```cpp
std::vector<int> v{5, 3, 1, 4, 2};
std::ranges::sort(v);
// v = {1, 2, 3, 4, 5}

// Custom comparator:
std::ranges::sort(v, std::greater<>{});
// v = {5, 4, 3, 2, 1}

// Sort structs by member (projection):
std::ranges::sort(students, {}, &Student::grade);
```

**Complexity:** O(n log n) worst-case (introsort = quicksort + heapsort fallback).
**Stability:** NOT stable — equal elements may be reordered.

### `std::stable_sort` — preserves relative order of equal elements

```cpp
struct Student { std::string name; int grade; };
// Sort by grade, but keep original name order for same grade
std::ranges::stable_sort(students, {}, &Student::grade);
```

**Complexity:** O(n log² n) without extra memory, O(n log n) with.

### `std::partial_sort` — sort only the first K elements

```cpp
std::vector<int> v{5, 3, 1, 4, 2, 8, 7, 6};
std::partial_sort(v.begin(), v.begin() + 3, v.end());
// v = {1, 2, 3, ?, ?, ?, ?, ?}
// First 3 are correct and sorted; rest is unspecified
```

**When to use:** "Top 10 results", "3 smallest values". Faster than sorting
everything when you only need a prefix.

### `std::nth_element` — find the k-th element

```cpp
std::vector<int> v{5, 3, 1, 4, 2, 8, 7, 6};
std::nth_element(v.begin(), v.begin() + 3, v.end());
// v[3] is the element that WOULD be at index 3 if sorted
// Everything before it is <= v[3], everything after is >= v[3]
// But neither half is sorted!
```

**Complexity:** O(n) average — much faster than sorting!
**When to use:** Finding the median, percentiles, or the k-th largest/smallest.

### Sorting cheat sheet

| Algorithm | Does what | Complexity | Use when |
|-----------|----------|-----------|----------|
| `sort` | Full sort | O(n log n) | Need everything sorted |
| `stable_sort` | Sort preserving equal order | O(n log n) | Need stable ordering |
| `partial_sort` | Sort first k elements | O(n log k) | Need top-k |
| `nth_element` | Place k-th element correctly | O(n) | Need median/percentile |

---

## 10. Partitioning

### `std::partition` — split range by predicate

Moves all elements satisfying the predicate to the front:

```cpp
std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
auto pivot = std::ranges::partition(v, [](int x) { return x % 2 == 0; });
// v = {8, 2, 6, 4, | 5, 3, 7, 1}
//                   ^ pivot (first element of "false" group)
// Even numbers are before pivot, odd after
// Order within each group is NOT preserved
```

### `std::stable_partition` — preserves relative order

```cpp
auto pivot = std::ranges::stable_partition(v, [](int x) { return x % 2 == 0; });
// v = {2, 4, 6, 8, | 1, 3, 5, 7}
// Order within each group IS preserved
```

### `std::partition_point` — find the split point

If a range is already partitioned, find where the partition boundary is:

```cpp
auto pp = std::ranges::partition_point(v, [](int x) { return x % 2 == 0; });
// pp points to the first element where the predicate is false
```

---

## 11. Exercises

See `exercises.cpp`.

---

**Next lecture:** Numeric & Set Algorithms.
