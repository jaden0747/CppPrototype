# Stdlib 06 — Non-Modifying Algorithms

> **Goal:** Master the non-modifying algorithms — functions that **inspect**
> elements without changing them. Learn the classic iterator-pair versions
> and their modern `std::ranges::` counterparts.

---

## Table of Contents

1. [What Are Non-Modifying Algorithms?](#1-what-are-non-modifying-algorithms)
2. [Finding Elements: `find`, `find_if`, `find_if_not`](#2-finding-elements)
3. [Counting: `count`, `count_if`](#3-counting)
4. [Predicates: `all_of`, `any_of`, `none_of`](#4-predicates)
5. [`for_each` and `for_each_n`](#5-for_each-and-for_each_n)
6. [Comparing Ranges: `equal`, `mismatch`](#6-comparing-ranges)
7. [Searching: `search`, `adjacent_find`](#7-searching)
8. [Min/Max: `min_element`, `max_element`, `minmax_element`](#8-minmax)
9. [Classic vs Ranges — Side by Side](#9-classic-vs-ranges--side-by-side)
10. [Exercises](#10-exercises)

---

## 1. What Are Non-Modifying Algorithms?

### The "What"

Non-modifying algorithms **read** from a range but never **write** to it.
They answer questions like:
- "Is this element in the collection?" → `find`
- "How many elements satisfy this condition?" → `count_if`
- "Do all elements satisfy this predicate?" → `all_of`

### The "Why"

These are the **most commonly used** STL algorithms. Instead of writing
manual `for` loops with `if` statements, you express **intent** directly:

```cpp
// Manual loop — what does this do? You have to read every line.
bool found = false;
for (size_t i = 0; i < v.size(); ++i) {
    if (v[i] == target) { found = true; break; }
}

// Algorithm — intent is clear from the name
bool found = std::ranges::find(v, target) != v.end();
```

### The "How" (common pattern)

All non-modifying algorithms follow this pattern:
1. Accept a range (or iterator pair)
2. Apply some check to each element
3. Return an iterator, a count, or a boolean

---

## 2. Finding Elements

### `std::find` — find by value

```cpp
std::vector<int> v{10, 20, 30, 40, 50};

auto it = std::find(v.begin(), v.end(), 30);
if (it != v.end())
    std::cout << "Found: " << *it << "\n";  // Found: 30

// Ranges version (cleaner):
auto it2 = std::ranges::find(v, 30);
```

**Returns:** Iterator to the first matching element, or `end()` if not found.

### `std::find_if` — find by predicate

```cpp
// Find first even number
auto it = std::find_if(v.begin(), v.end(),
                       [](int x) { return x % 2 == 0; });

// Ranges version with projection:
struct Student { std::string name; int grade; };
std::vector<Student> students = ...;
auto it = std::ranges::find_if(students,
                               [](int g) { return g >= 90; },
                               &Student::grade);
// "Find a student whose grade is >= 90"
```

### `std::find_if_not` — find first that does NOT match

```cpp
// Find first odd number
auto it = std::ranges::find_if_not(v, [](int x) { return x % 2 == 0; });
```

### When to use which

| Function | Use when... |
|----------|------------|
| `find(range, value)` | Looking for a specific value |
| `find_if(range, pred)` | Looking for first element matching a condition |
| `find_if_not(range, pred)` | Looking for first element NOT matching a condition |

---

## 3. Counting

### `std::count` — count occurrences of a value

```cpp
std::vector<int> v{1, 2, 3, 2, 1, 2, 3};

auto n = std::count(v.begin(), v.end(), 2);       // n = 3
auto n2 = std::ranges::count(v, 2);               // n2 = 3 (ranges)
```

### `std::count_if` — count elements matching a predicate

```cpp
// How many even numbers?
auto n = std::ranges::count_if(v, [](int x) { return x % 2 == 0; });
// n = 3 (the three 2's)

// How many students passed?
auto passed = std::ranges::count_if(students,
    [](int g) { return g >= 60; }, &Student::grade);
```

### Performance note

`count` and `count_if` always scan the **entire** range (O(n)).
If you only need to know if **at least one** element exists, use
`find` or `any_of` instead — they short-circuit.

---

## 4. Predicates: `all_of`, `any_of`, `none_of`

These answer yes/no questions about a range:

### `std::all_of` — "do ALL elements satisfy the condition?"

```cpp
std::vector<int> v{2, 4, 6, 8, 10};

bool all_even = std::ranges::all_of(v, [](int x) { return x % 2 == 0; });
// true — every element is even
```

### `std::any_of` — "does AT LEAST ONE element satisfy the condition?"

```cpp
std::vector<int> v{1, 3, 5, 6, 7};

bool has_even = std::ranges::any_of(v, [](int x) { return x % 2 == 0; });
// true — 6 is even
```

### `std::none_of` — "do NO elements satisfy the condition?"

```cpp
std::vector<int> v{1, 3, 5, 7, 9};

bool no_even = std::ranges::none_of(v, [](int x) { return x % 2 == 0; });
// true — no element is even
```

### Short-circuit behavior

These algorithms stop as soon as the answer is determined:

| Algorithm | Stops when... |
|-----------|--------------|
| `all_of` | Finds first element that is `false` |
| `any_of` | Finds first element that is `true` |
| `none_of` | Finds first element that is `true` |

### Edge case: empty ranges

```cpp
std::vector<int> empty;
std::ranges::all_of(empty, pred);  // true  (vacuously true)
std::ranges::any_of(empty, pred);  // false (no elements to check)
std::ranges::none_of(empty, pred); // true  (no elements violate)
```

---

## 5. `for_each` and `for_each_n`

### `std::for_each` — apply a function to every element

```cpp
std::vector<int> v{1, 2, 3, 4, 5};

std::ranges::for_each(v, [](int x) {
    std::cout << x << " ";
});
// Output: 1 2 3 4 5
```

### When to use `for_each` vs range-based `for`

In most cases, **range-based `for` is simpler**:

```cpp
for (int x : v) std::cout << x << " ";  // Prefer this
```

Use `for_each` when:
- You need the **return value** (the function object, with accumulated state)
- You're working in an algorithm pipeline and need functional style

### `std::for_each_n` — apply to first N elements

```cpp
std::for_each_n(v.begin(), 3, [](int x) { std::cout << x << " "; });
// Output: 1 2 3
```

---

## 6. Comparing Ranges: `equal`, `mismatch`

### `std::equal` — are two ranges identical?

```cpp
std::vector<int> a{1, 2, 3, 4};
std::vector<int> b{1, 2, 3, 4};
std::vector<int> c{1, 2, 5, 4};

std::ranges::equal(a, b);  // true
std::ranges::equal(a, c);  // false

// With custom comparison:
std::ranges::equal(a, c, [](int x, int y) {
    return std::abs(x - y) <= 2;
}); // true — all elements are within 2 of each other
```

### `std::mismatch` — find where two ranges diverge

```cpp
auto [it1, it2] = std::ranges::mismatch(a, c);
// *it1 = 3, *it2 = 5 — first position where they differ
```

**Use case:** Finding the common prefix of two sequences.

---

## 7. Searching: `search`, `adjacent_find`

### `std::search` — find a subsequence within a range

```cpp
std::vector<int> haystack{1, 2, 3, 4, 5, 6, 7};
std::vector<int> needle{3, 4, 5};

auto it = std::ranges::search(haystack, needle);
// it.begin() points to the 3 in haystack
```

### `std::adjacent_find` — find first pair of equal neighbors

```cpp
std::vector<int> v{1, 2, 3, 3, 4, 5};

auto it = std::ranges::adjacent_find(v);
// *it = 3 — the first of the adjacent duplicate pair
```

**Use case:** Detecting duplicates in sorted data, finding repeated patterns.

---

## 8. Min/Max

### `std::min_element` / `std::max_element`

```cpp
std::vector<int> v{5, 2, 8, 1, 9, 3};

auto min_it = std::ranges::min_element(v);  // points to 1
auto max_it = std::ranges::max_element(v);  // points to 9

std::cout << "Min: " << *min_it << "\n";    // Min: 1
std::cout << "Max: " << *max_it << "\n";    // Max: 9
```

### `std::minmax_element` — find both in one pass

```cpp
auto [min_it, max_it] = std::ranges::minmax_element(v);
// More efficient than calling min_element + max_element separately
// Only ~1.5n comparisons vs 2n
```

### With projections

```cpp
struct Product { std::string name; double price; };
std::vector<Product> products = ...;

auto cheapest = std::ranges::min_element(products, {}, &Product::price);
auto most_expensive = std::ranges::max_element(products, {}, &Product::price);
```

---

## 9. Classic vs Ranges — Side by Side

| Task | Classic | Ranges |
|------|---------|--------|
| Find value | `find(v.begin(), v.end(), x)` | `ranges::find(v, x)` |
| Find by predicate | `find_if(b, e, pred)` | `ranges::find_if(v, pred)` |
| Count | `count(b, e, x)` | `ranges::count(v, x)` |
| All match? | `all_of(b, e, pred)` | `ranges::all_of(v, pred)` |
| Equal? | `equal(b1, e1, b2)` | `ranges::equal(v1, v2)` |
| Min element | `min_element(b, e)` | `ranges::min_element(v)` |

**Recommendation:** Use `std::ranges::` versions in new code. They are:
- Shorter (pass the whole container)
- Safer (detect mismatched ranges at compile time)
- More powerful (support projections)

---

## 10. Exercises

See `exercises.cpp`.

---

**Next lecture:** Modifying & Sorting Algorithms.
