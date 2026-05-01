# Stdlib 02 — Associative Containers

> **Goal:** Master ordered key-based containers: `map`, `set`, `multimap`,
> `multiset`. Understand how red-black trees work, custom comparators, and
> modern C++17/20 enhancements. Know when to pick ordered vs unordered.

---

## Table of Contents

1. [What Are Associative Containers?](#1-what-are-associative-containers)
2. [`std::set` / `std::multiset`](#2-stdset--stdmultiset)
3. [`std::map` / `std::multimap`](#3-stdmap--stdmultimap)
4. [How Ordered Containers Work Internally](#4-how-they-work-internally)
5. [Custom Comparators](#5-custom-comparators)
6. [Modern Enhancements (C++17/20)](#6-modern-enhancements)
7. [Performance Characteristics](#7-performance-characteristics)
8. [Common Patterns and Idioms](#8-common-patterns-and-idioms)
9. [Exercises](#9-exercises)

---

## 1. What Are Associative Containers?

### The "What"

Associative containers organize elements by **key**, not by position. Unlike
`vector` (where element 0 is always first), associative containers **sort**
elements automatically and provide **fast lookup by key**.

### The two families

| Family | Containers | Internal structure |
|--------|-----------|-------------------|
| **Ordered** | `set`, `map`, `multiset`, `multimap` | Red-black tree (balanced BST) |
| **Unordered** | `unordered_set`, `unordered_map`, etc. | Hash table (lecture 03) |

### The "Why"

When you need to:
- Quickly check if a value exists → `set`
- Quickly look up a value by key → `map`
- Keep elements automatically sorted → ordered containers
- Find elements in a range (`lower_bound`, `upper_bound`) → ordered containers

---

## 2. `std::set` / `std::multiset`

### What is `std::set`?

A collection of **unique, sorted keys**. Think of it as a mathematical set
that's always in order.

### Basic usage

```cpp
#include <set>

std::set<int> s{5, 3, 1, 4, 2, 2, 3};
// s contains {1, 2, 3, 4, 5} — duplicates removed, sorted

s.insert(6);             // O(log n) — {1, 2, 3, 4, 5, 6}
s.erase(3);              // O(log n) — {1, 2, 4, 5, 6}
s.contains(4);           // C++20 — true
s.find(4) != s.end();    // pre-C++20 way to check
s.count(4);              // 0 or 1 (for set; 0..n for multiset)
s.size();                // 5
s.empty();               // false
```

### Iteration is always sorted

```cpp
for (int x : s)
    std::cout << x << " ";  // 1 2 4 5 6 — always sorted!
```

### Range queries with `lower_bound` / `upper_bound`

This is the **killer feature** of ordered containers:

```cpp
std::set<int> s{10, 20, 30, 40, 50};

auto lo = s.lower_bound(25);  // points to 30 (first element >= 25)
auto hi = s.upper_bound(40);  // points to 50 (first element > 40)

// Iterate elements in range [25, 40]:
for (auto it = lo; it != hi; ++it)
    std::cout << *it << " ";  // 30 40
```

### `std::multiset` — allows duplicates

```cpp
std::multiset<int> ms{1, 2, 2, 3, 3, 3};
ms.count(3);   // 3 — three copies of 3
ms.erase(3);   // removes ALL 3's — ms = {1, 2, 2}

// Erase just one:
auto it = ms.find(2);
ms.erase(it);  // removes ONE 2 — ms = {1, 2}
```

---

## 3. `std::map` / `std::multimap`

### What is `std::map`?

A sorted collection of **key-value pairs** with unique keys. Think of it as
a dictionary that's always sorted by key.

### Basic usage

```cpp
#include <map>

std::map<std::string, int> ages;
ages["Alice"] = 25;          // insert or overwrite
ages["Bob"] = 30;
ages.emplace("Charlie", 35); // construct in-place

// Access:
int a = ages["Alice"];        // 25
int b = ages.at("Bob");       // 30 — throws if key missing!
```

### The `operator[]` trap

```cpp
std::map<std::string, int> m;
std::cout << m["missing"];  // prints 0 — BUT ALSO INSERTS {"missing", 0}!
std::cout << m.size();       // 1, not 0!
```

**`operator[]` inserts a default value if the key doesn't exist!**
Use `find()`, `contains()`, or `at()` for read-only lookup.

### Iterating a map

```cpp
for (const auto& [key, value] : ages) {  // structured bindings (C++17)
    std::cout << key << ": " << value << "\n";
}
// Output is sorted by key:
// Alice: 25
// Bob: 30
// Charlie: 35
```

### `std::multimap` — multiple values per key

```cpp
std::multimap<std::string, int> scores;
scores.emplace("Alice", 90);
scores.emplace("Alice", 85);
scores.emplace("Bob", 95);

// No operator[] — ambiguous (which Alice?)
auto range = scores.equal_range("Alice");
for (auto it = range.first; it != range.second; ++it)
    std::cout << it->second << " ";  // 85 90
```

---

## 4. How They Work Internally

### Red-black tree

Ordered containers use a **self-balancing binary search tree** (red-black tree):

```
        30 (B)
       /      \
    20 (R)    40 (R)
   /    \     /    \
 10(B) 25(B) 35(B) 50(B)
```

**Properties:**
- Every path from root to leaf has the same number of black nodes
- No two consecutive red nodes on any path
- This guarantees the tree height is O(log n) → all operations are O(log n)

### Why this matters

- **Lookup**: Walk from root to leaf, going left if key < node, right if key > node
- **Insert**: Walk to correct position, insert, rebalance (recolor/rotate)
- **Iteration**: In-order traversal of the tree → sorted output

---

## 5. Custom Comparators

### Default: `std::less<Key>`

By default, `set` and `map` sort using `operator<`. To change the sort order
or criteria, provide a **custom comparator**.

### Method 1: Function object (struct)

```cpp
struct CaseInsensitiveLess {
    bool operator()(const std::string& a, const std::string& b) const {
        return std::lexicographical_compare(
            a.begin(), a.end(), b.begin(), b.end(),
            [](char c1, char c2) { return std::tolower(c1) < std::tolower(c2); }
        );
    }
};

std::set<std::string, CaseInsensitiveLess> s;
s.insert("Hello");
s.insert("hello");  // NOT inserted — "Hello" == "hello" with our comparator
```

### Method 2: Lambda (C++20)

```cpp
auto cmp = [](int a, int b) { return a > b; };  // reverse order
std::set<int, decltype(cmp)> s(cmp);
s.insert(1); s.insert(3); s.insert(2);
// Iteration order: 3, 2, 1
```

### Method 3: `std::greater<>` for reverse order

```cpp
std::set<int, std::greater<>> s{1, 2, 3, 4, 5};
// Iteration order: 5, 4, 3, 2, 1
```

---

## 6. Modern Enhancements

### `contains` (C++20) — check if key exists

```cpp
if (m.contains("key")) { ... }
// Replaces the ugly:
// if (m.find("key") != m.end()) { ... }
// and the misleading:
// if (m.count("key") > 0) { ... }
```

### `try_emplace` (C++17) — only construct if key is new

```cpp
auto [it, inserted] = m.try_emplace("key", expensive_args...);
// If "key" already exists: does NOT construct the value (saves work)
// If "key" is new: constructs and inserts
```

### `insert_or_assign` (C++17) — upsert

```cpp
auto [it, inserted] = m.insert_or_assign("key", value);
// If "key" exists: overwrites the value, returns inserted=false
// If "key" is new: inserts, returns inserted=true
```

### Node extraction and merging (C++17)

```cpp
// Extract a node (key-value pair) without copying:
auto node = m.extract("key");
if (!node.empty()) {
    node.key() = "new_key";      // modify the key!
    m.insert(std::move(node));   // re-insert
}

// Merge all elements from another map:
m.merge(other_map);  // moves nodes, no copies
```

**Why this is powerful:** Normally, you can't change a key in a map/set
because it would break the sorted order. Node extraction lets you remove
the node, modify the key, and re-insert it — all without copying the value.

---

## 7. Performance Characteristics

| Operation | `set`/`map` | Notes |
|-----------|------------|-------|
| Insert | O(log n) | Tree walk + possible rebalance |
| Find / contains | O(log n) | Tree walk |
| Erase | O(log n) | Tree walk + possible rebalance |
| lower_bound / upper_bound | O(log n) | Tree walk |
| Iteration (full) | O(n) | In-order traversal, always sorted |
| Memory per element | ~3 pointers + data | Left, right, parent + color bit |

### Ordered vs Unordered — quick comparison

| | Ordered (`map`/`set`) | Unordered |
|-|----------------------|-----------|
| Lookup | O(log n) | O(1) average |
| Sorted iteration? | ✅ | ❌ |
| Range queries? | ✅ | ❌ |
| Custom type key? | Need `operator<` | Need `hash` + `operator==` |

**Rule of thumb:** Use ordered containers when you need sorted iteration
or range queries. Otherwise, unordered containers are faster.

---

## 8. Common Patterns and Idioms

### Word frequency counter

```cpp
std::map<std::string, int> freq;
for (const auto& word : words)
    freq[word]++;  // operator[] default-constructs (0) then increments
```

### Finding the N-th smallest element

```cpp
std::set<int> s{50, 20, 40, 10, 30};
auto it = s.begin();
std::advance(it, 2);  // O(n) — 3rd element
// *it = 30 (elements are: 10, 20, 30, 40, 50)
```

### Removing elements while iterating

```cpp
// Pre-C++20:
for (auto it = m.begin(); it != m.end(); ) {
    if (should_remove(*it))
        it = m.erase(it);  // erase returns next iterator
    else
        ++it;
}

// C++20:
std::erase_if(m, [](const auto& pair) { return should_remove(pair); });
```

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** Unordered Containers & Adaptors.
