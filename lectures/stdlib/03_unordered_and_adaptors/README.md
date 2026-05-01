# Stdlib 03 — Unordered Containers & Adaptors

> **Goal:** Master hash-based containers (`unordered_map`, `unordered_set`) and
> container adaptors (`stack`, `queue`, `priority_queue`, `span`).
> Understand how hash tables work internally, how to write custom hash functions,
> and when to pick an adaptor over a raw container.

---

## Table of Contents

1. [What Are Unordered Containers?](#1-what-are-unordered-containers)
2. [`std::unordered_set` / `std::unordered_multiset`](#2-stdunordered_set--stdunordered_multiset)
3. [`std::unordered_map` / `std::unordered_multimap`](#3-stdunordered_map--stdunordered_multimap)
4. [How Hash Tables Work Internally](#4-how-hash-tables-work-internally)
5. [Writing Custom Hash Functions](#5-writing-custom-hash-functions)
6. [Container Adaptors: Stack, Queue, Priority Queue](#6-container-adaptors-stack-queue-priority-queue)
7. [`std::span` (C++20) — Non-Owning View](#7-stdspan-c20--non-owning-view)
8. [Ordered vs Unordered — When To Use Which](#8-ordered-vs-unordered--when-to-use-which)
9. [Exercises](#9-exercises)

---

## 1. What Are Unordered Containers?

### The "What"

Unordered containers are **hash-table-based** data structures. Unlike `std::map`
and `std::set` (which use balanced BSTs and keep elements sorted), unordered
containers store elements in **buckets** determined by a hash function. This
gives them **O(1) average** lookup, insert, and erase — compared to O(log n)
for ordered containers.

### The "Why"

If you don't need sorted iteration, unordered containers are almost always
faster for lookups. Consider:

| Operation   | `std::map` | `std::unordered_map` |
|-------------|-----------|---------------------|
| Find        | O(log n)  | O(1) average        |
| Insert      | O(log n)  | O(1) average        |
| Erase       | O(log n)  | O(1) average        |
| Iteration   | Sorted    | Arbitrary order     |
| Worst case  | O(log n)  | O(n) (all collide)  |

### The "When"

- **Use unordered** when you need fast lookup/insert and don't care about order
- **Use ordered** when you need sorted iteration, range queries (`lower_bound`),
  or have types without a good hash function

---

## 2. `std::unordered_set` / `std::unordered_multiset`

### What is it?

A hash-table-based collection of **unique keys** (no duplicates for `unordered_set`;
duplicates allowed for `unordered_multiset`).

### Basic usage

```cpp
#include <unordered_set>

std::unordered_set<int> s{5, 3, 1, 4, 2, 2, 3};
// s contains {1, 2, 3, 4, 5} — duplicates silently ignored
// Iteration order is NOT sorted — it's bucket order

s.insert(6);              // O(1) average
s.erase(3);               // O(1) average
bool has = s.contains(4); // C++20 — replaces s.find(4) != s.end()
auto count = s.count(4);  // 0 or 1 for set, 0..n for multiset
```

### How `insert` return value works

```cpp
auto [iterator, was_inserted] = s.insert(42);
// was_inserted is true if 42 was new, false if already present
// iterator points to the element (new or existing)
```

### `unordered_multiset` — allowing duplicates

```cpp
std::unordered_multiset<int> ms{1, 1, 2, 2, 3};
ms.count(1);  // returns 2
```

---

## 3. `std::unordered_map` / `std::unordered_multimap`

### What is it?

A hash table mapping **keys to values**. `unordered_map` enforces unique keys;
`unordered_multimap` allows multiple values per key.

### Basic usage

```cpp
#include <unordered_map>

std::unordered_map<std::string, int> m;
m["apple"]  = 3;        // insert or overwrite
m.emplace("banana", 5); // construct in-place

// Access
int val = m["apple"];     // 3
int val2 = m.at("banana"); // 5 — throws if missing!
```

### The `operator[]` trap

**Warning:** `m["missing_key"]` **inserts** a default-constructed value (0 for int)!
This is a common source of bugs.

```cpp
std::unordered_map<std::string, int> m;
std::cout << m["oops"]; // prints 0, AND inserts {"oops", 0} into the map!
std::cout << m.size();   // now 1, not 0!
```

**Best practice:** Use `find()`, `contains()`, or `at()` for read-only access.

### `try_emplace` (C++17) — only construct if key is new

```cpp
auto [it, ok] = m.try_emplace("key", expensive_value);
// If "key" exists: does NOT construct expensive_value (unlike emplace!)
// If "key" is new: constructs and inserts
```

**Why it matters:** `emplace` may construct the value even if the key exists
(then throw it away). `try_emplace` avoids this waste.

---

## 4. How Hash Tables Work Internally

Understanding the internals helps you write better code and debug performance
issues.

### The bucket model

```
Hash function: hash("apple") = 5

Bucket array:
[0] -> {}
[1] -> {"banana" -> 5}
[2] -> {}
[3] -> {}
[4] -> {}
[5] -> {"apple" -> 3, "grape" -> 7}   ← collision! chained
[6] -> {}
[7] -> {"cherry" -> 2}
```

1. **Hash the key** → get a large number
2. **Modulo by bucket count** → get bucket index
3. **Search the bucket chain** → find or insert the element

### Load factor

```
load_factor = size / bucket_count
```

- When `load_factor` exceeds `max_load_factor` (default 1.0), the table
  **rehashes**: allocates more buckets, re-inserts everything
- Rehashing is **O(n)** — it's the reason insert is "amortised" O(1)
- You can prevent surprise rehashes with `reserve(expected_size)`

### The bucket interface

```cpp
s.bucket_count();          // current number of buckets
s.load_factor();           // current load factor
s.max_load_factor();       // threshold for rehash (default 1.0)
s.reserve(1000);           // pre-allocate buckets for 1000 elements
s.rehash(2000);            // set minimum bucket count to 2000
```

**Performance tip:** If you know the approximate size, call `reserve()` upfront
to avoid multiple rehashes.

---

## 5. Writing Custom Hash Functions

### Why you need them

`std::hash<T>` is provided for built-in types and `std::string`. For your own
types, you must supply a hash function — otherwise you get a compile error.

### Method 1: Function object (struct with `operator()`)

```cpp
struct Point {
    int x, y;
    bool operator==(const Point&) const = default; // also required!
};

struct PointHash {
    std::size_t operator()(const Point& p) const {
        auto h1 = std::hash<int>{}(p.x);
        auto h2 = std::hash<int>{}(p.y);
        // Combine hashes — XOR + shift is a simple approach
        return h1 ^ (h2 << 1);
    }
};

std::unordered_set<Point, PointHash> points;
```

### Method 2: Specialize `std::hash` (makes it the default)

```cpp
template<>
struct std::hash<Point> {
    std::size_t operator()(const Point& p) const {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

// Now you don't need to specify hash explicitly:
std::unordered_set<Point> points;
```

### Hash combination best practice

Simple XOR is prone to collisions (e.g., `{1,2}` and `{2,1}` may collide).
A better combiner (from Boost):

```cpp
template<typename T>
void hash_combine(std::size_t& seed, const T& val) {
    seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
```

### Remember: `operator==` is also required!

The hash table uses `==` to handle collisions. If two elements hash to the
same bucket, it calls `==` to distinguish them.

---

## 6. Container Adaptors: Stack, Queue, Priority Queue

### What are adaptors?

Container adaptors are **wrappers** that provide a restricted interface on
top of an underlying container. They don't expose iterators — they enforce
a specific access pattern.

### `std::stack` — Last In, First Out (LIFO)

```cpp
#include <stack>

std::stack<int> s;           // default: backed by deque
std::stack<int, std::vector<int>> sv; // backed by vector

s.push(1); s.push(2); s.push(3);
s.top();    // 3 — peek at top (does NOT remove)
s.pop();    // removes top (returns void!)
s.size();   // 2
s.empty();  // false
```

**When to use:** Expression parsing, undo stacks, DFS traversal, bracket matching.

### `std::queue` — First In, First Out (FIFO)

```cpp
#include <queue>

std::queue<std::string> q;
q.push("first");
q.push("second");
q.front();  // "first" — next to be removed
q.back();   // "second" — last added
q.pop();    // removes front
```

**When to use:** BFS traversal, task scheduling, producer-consumer buffers.

### `std::priority_queue` — highest priority first

```cpp
#include <queue>

// Max-heap by default (largest element on top)
std::priority_queue<int> pq;
pq.push(3); pq.push(1); pq.push(4);
pq.top();   // 4 — largest
pq.pop();   // removes 4

// Min-heap: use std::greater
std::priority_queue<int, std::vector<int>, std::greater<>> min_pq;
min_pq.push(3); min_pq.push(1); min_pq.push(4);
min_pq.top();   // 1 — smallest
```

**When to use:** Dijkstra's algorithm, top-K problems, event scheduling,
merge K sorted lists.

**How it works internally:** A binary heap stored in a `vector`. `push` is
O(log n), `pop` is O(log n), `top` is O(1).

---

## 7. `std::span` (C++20) — Non-Owning View

### What is it?

`std::span<T>` is a **non-owning, lightweight view** over a contiguous
sequence of elements. Think of it as a safe replacement for the old C pattern
of `(T* ptr, size_t count)`.

### Why use it?

```cpp
// BAD old way — error-prone, no bounds info
void process(int* data, size_t count);

// GOOD modern way — carries pointer AND size, no copy
void process(std::span<const int> data);
```

### Basic usage

```cpp
#include <span>

void print_span(std::span<const int> data) {
    for (int x : data) std::cout << x << " ";
}

std::vector<int> v{1, 2, 3, 4, 5};
print_span(v);           // works with vector
int arr[] = {10, 20, 30};
print_span(arr);         // works with C array
print_span({v.data(), 3}); // works with pointer + count

// Subspan operations
std::span s(v);
auto first3 = s.first(3);     // first 3 elements
auto last2  = s.last(2);      // last 2 elements
auto mid    = s.subspan(1, 3); // elements [1..4)
```

### Static extent vs dynamic extent

```cpp
std::span<int>     dynamic_span; // size known at runtime
std::span<int, 5>  static_span;  // size fixed at compile time (like array)
```

---

## 8. Ordered vs Unordered — When To Use Which

| Criterion | Ordered (`map`/`set`) | Unordered (`unordered_map`/`unordered_set`) |
|-----------|----------------------|---------------------------------------------|
| Lookup speed | O(log n) | O(1) average |
| Need sorted iteration? | ✅ Yes | ❌ No |
| Need `lower_bound`? | ✅ Yes | ❌ No |
| Custom type as key? | Need `operator<` | Need `hash` + `operator==` |
| Memory overhead | Less (tree nodes) | More (bucket array + chains) |
| Worst case | O(log n) always | O(n) if bad hash |
| Cache friendliness | Poor (pointer chasing) | Better (but not great) |

**Rule of thumb:** Default to `unordered_map`/`unordered_set` unless you need
sorted order or range queries.

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** Iterators.
