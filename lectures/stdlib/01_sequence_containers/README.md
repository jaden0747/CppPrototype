# Stdlib 01 — Sequence Containers

> **Goal:** Master the everyday linear containers — `vector`, `array`, `deque`,
> `list`, and `forward_list`. Know when to pick each, understand their internal
> memory layout, iterator invalidation, and write efficient insertion code.

---

## Table of Contents

1. [What Are Sequence Containers?](#1-what-are-sequence-containers)
2. [`std::vector` — The Default Container](#2-stdvector--the-default-container)
3. [`std::array` — Fixed-Size, Stack-Allocated](#3-stdarray--fixed-size-stack-allocated)
4. [`std::deque` — Double-Ended Queue](#4-stddeque--double-ended-queue)
5. [`std::list` — Doubly Linked List](#5-stdlist--doubly-linked-list)
6. [`std::forward_list` — Singly Linked List](#6-stdforward_list--singly-linked-list)
7. [Iterator Invalidation Rules](#7-iterator-invalidation-rules)
8. [Choosing the Right Container](#8-choosing-the-right-container)
9. [Exercises](#9-exercises)

---

## 1. What Are Sequence Containers?

### The "What"

Sequence containers store elements in a **linear order** — element 0, element 1,
element 2, etc. The order you insert is the order you get back (unlike sets/maps
which sort automatically).

### The "Why"

They're the most fundamental data structures in C++. Almost every program
needs to store a collection of things:
- A list of users
- Pixel data for an image
- A buffer of network messages
- A history of commands

### The five sequence containers

| Container | Memory layout | Key trait |
|-----------|--------------|-----------|
| `vector` | Contiguous array (heap) | Best default, fast random access |
| `array` | Contiguous array (stack) | Fixed size, zero overhead |
| `deque` | Chunks of arrays | Fast front and back insertion |
| `list` | Doubly-linked nodes | O(1) insert/erase anywhere |
| `forward_list` | Singly-linked nodes | Minimal memory, forward only |

---

## 2. `std::vector` — The Default Container

### What is it?

A **dynamically-sized, contiguous array**. Elements are stored one after another
in a single block of heap memory, like a resizable C array.

```
Memory: [1][2][3][4][5][...unused capacity...]
         ^                ^
         data()           data() + size()
```

### Basic usage

```cpp
#include <vector>

std::vector<int> v;              // empty vector
std::vector<int> v2{1, 2, 3};   // initializer list
std::vector<int> v3(10, 0);     // 10 zeros
std::vector<std::string> vs(5); // 5 empty strings
```

### Adding elements

```cpp
v.push_back(42);       // copy/move to the end — O(1) amortised
v.emplace_back(42);    // construct in-place (avoids copy) — O(1) amortised
v.insert(v.begin(), 0); // insert at front — O(n)! shifts everything
```

### `push_back` vs `emplace_back` — when does it matter?

```cpp
struct Widget {
    Widget(int a, double b);  // constructor
};

std::vector<Widget> v;
v.push_back(Widget(1, 2.0));   // constructs Widget, THEN moves into vector
v.emplace_back(1, 2.0);        // constructs Widget directly inside vector
// emplace_back avoids one move/copy — matters for expensive-to-move types
```

**Rule of thumb:** Use `emplace_back` when constructing from arguments.
Use `push_back` when you already have an object.

### Capacity vs Size

```cpp
v.size();        // number of elements currently stored
v.capacity();    // number of elements that can fit before reallocation
v.reserve(100);  // pre-allocate space for 100 elements (no resize!)
v.resize(10, 0); // change SIZE to 10, fill new elements with 0
v.shrink_to_fit(); // request to reduce capacity to match size
```

**How growth works:**
When `push_back` exceeds capacity, vector **allocates a new, larger block**
(typically 2x the old capacity), copies/moves all elements, and frees the old
block. This is why `push_back` is "amortised O(1)" — most calls are O(1),
but occasionally one is O(n).

```
Before push_back(6):
[1][2][3][4][5]          capacity=5, size=5

After push_back(6):
[1][2][3][4][5][6][.][.][.][.]  capacity=10, size=6
                                 (new allocation, old memory freed)
```

**Performance tip:** If you know the final size, call `reserve()` upfront
to avoid multiple reallocations.

### Accessing elements

```cpp
v[0];            // no bounds check — undefined behavior if out of range!
v.at(0);         // bounds check — throws std::out_of_range
v.front();       // first element
v.back();        // last element
v.data();        // raw T* pointer (for C API interop)
```

### Removing elements

```cpp
v.pop_back();                   // remove last — O(1)
v.erase(v.begin() + 2);        // remove element at index 2 — O(n)
v.erase(v.begin(), v.begin()+3); // remove first 3 elements — O(n)
v.clear();                      // remove all — O(n) for destructors

// C++20: remove all elements equal to a value
std::erase(v, 42);              // remove all 42's
std::erase_if(v, [](int x) { return x < 0; }); // remove negatives
```

---

## 3. `std::array` — Fixed-Size, Stack-Allocated

### What is it?

A **fixed-size array** that lives on the stack. It's a thin wrapper around a
C array (`T[N]`) that adds `.size()`, `.begin()/.end()`, bounds checking, etc.

### Why use it instead of C arrays?

```cpp
// C array: no size info, decays to pointer, no bounds check
int arr[5] = {1, 2, 3, 4, 5};
// sizeof(arr) is 20 bytes, but pass to function → just a pointer!

// std::array: knows its size, doesn't decay, has iterators
std::array<int, 5> a{1, 2, 3, 4, 5};
a.size();     // 5 — always knows its size
a.at(10);     // throws! — bounds checked
```

### Basic usage

```cpp
#include <array>

std::array<int, 5> a{1, 2, 3, 4, 5};
a[0] = 10;
a.fill(0);              // set all elements to 0
a.size();               // 5 (constexpr)

// Size is part of the type!
std::array<int, 3> x;   // different type than std::array<int, 5>
```

### When to use `array`

- Size known at **compile time**
- Want **stack allocation** (no heap)
- Need **zero overhead** over C arrays
- Want it to work with `constexpr`

---

## 4. `std::deque` — Double-Ended Queue

### What is it?

A deque (pronounced "deck") provides O(1) insertion and removal at **both
front and back**. Internally, it's a collection of fixed-size arrays (chunks)
managed by a pointer array.

```
Chunks: [chunk0: a,b,c] [chunk1: d,e,f] [chunk2: g,h,i]
Map:    [ptr0] [ptr1] [ptr2]
```

### Why use deque over vector?

- `push_front()` is O(1) — vector doesn't even have `push_front()`
- Never invalidates pointers/references to existing elements on push_back/push_front
  (only iterators may be invalidated)

### Basic usage

```cpp
#include <deque>

std::deque<int> d;
d.push_back(1);       // O(1) at back
d.push_front(0);      // O(1) at front — vector can't do this!
d.pop_front();         // O(1) remove from front
d.pop_back();          // O(1) remove from back
d[0];                  // random access O(1)
```

### When to use deque

- Need fast insertion/removal at **both ends** (e.g., sliding window)
- Default underlying container for `std::stack` and `std::queue`
- **Don't use if:** you need contiguous memory or `data()` pointer

---

## 5. `std::list` — Doubly Linked List

### What is it?

A doubly-linked list where each element is a separate heap allocation
connected by forward and backward pointers.

```
[prev|data|next] ↔ [prev|data|next] ↔ [prev|data|next]
```

### When to use list

- Need **O(1) insert/erase** at any position (given an iterator)
- Need **splice**: move elements between lists in O(1) without copying
- Elements must **never** be moved in memory (stable addresses)

### Basic usage

```cpp
#include <list>

std::list<int> l{3, 1, 4, 1, 5};

// Insert/erase anywhere in O(1) (given iterator):
auto it = std::next(l.begin(), 2); // points to 4
l.insert(it, 99);  // l = {3, 1, 99, 4, 1, 5}
l.erase(it);        // l = {3, 1, 99, 1, 5} — erases the 4

// List has its own sort (can't use std::sort — needs random access)
l.sort();           // {1, 1, 3, 5, 99}
l.unique();         // {1, 3, 5, 99} — remove consecutive duplicates
l.reverse();        // {99, 5, 3, 1}
```

### Splice — the superpower of list

```cpp
std::list<int> a{1, 2, 3};
std::list<int> b{10, 20, 30};

a.splice(std::next(a.begin()), b); // move ALL of b into a after first element
// a = {1, 10, 20, 30, 2, 3}
// b = {} (empty!)
// This is O(1)! No copies, no allocations.
```

### Why list is usually slower than vector

Even though list has O(1) insert, it's **cache-unfriendly**. Each node
is a separate heap allocation scattered in memory. Traversing a list
causes many **cache misses**. In practice, `vector` (with O(n) insert)
is often faster than `list` for small to medium sizes because of cache
locality.

**Rule:** Only use `list` when you need splice or stable iterators/pointers.

---

## 6. `std::forward_list` — Singly Linked List

### What is it?

A singly-linked list — each node only has a `next` pointer (no `prev`).
Minimal memory overhead.

```cpp
#include <forward_list>

std::forward_list<int> fl{1, 2, 3, 4};
fl.push_front(0);          // O(1) — only front insertion
fl.insert_after(fl.begin(), 99); // insert AFTER an element
fl.erase_after(fl.begin()); // erase element AFTER an element
```

### Limitations

- **No `size()`** — would require O(n) traversal or extra storage
- **No `push_back()`** — no backward pointer
- **Insert/erase after**, not before (singly linked)
- Use `before_begin()` to insert at the front

### When to use

- Need a linked list with **minimal memory** per node
- Only need **forward traversal**
- Replacing C-style singly-linked list

---

## 7. Iterator Invalidation Rules

**Iterator invalidation** means an iterator (or pointer/reference) becomes
dangling after a container modification. Using an invalidated iterator is
**undefined behavior**.

### Vector

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
auto it = v.begin() + 2;  // points to 3

v.push_back(6);  // ⚠️ MAY invalidate ALL iterators (if reallocation!)
v.insert(v.begin(), 0); // Invalidates all iterators at/after insertion point
v.erase(v.begin());     // Invalidates all iterators at/after erasure point
```

**Safe pattern:** Re-acquire iterators after modifications, or `reserve()` first.

### Summary table

| Container | Operation | What's invalidated |
|-----------|-----------|-------------------|
| `vector` | `push_back` (no realloc) | Only `end()` |
| `vector` | `push_back` (realloc) | **ALL** iterators |
| `vector` | `insert`/`erase` | At and after the point |
| `deque` | `push_back`/`push_front` | **ALL** iterators (but not references!) |
| `deque` | `insert`/`erase` (middle) | **ALL** iterators and references |
| `list` | Any insert/erase | **Only** the erased element |
| `array` | (fixed size) | Never invalidated |
| `forward_list` | Any insert/erase | **Only** the erased element |

---

## 8. Choosing the Right Container

### Decision flowchart

```
Need a collection of elements?
├── Size known at compile time? → std::array
├── Need fast front + back insertion? → std::deque
├── Need O(1) insert/erase anywhere? → std::list
├── Need minimal memory linked list? → std::forward_list
└── Otherwise → std::vector (the default!)
```

### Performance comparison

| Operation | `vector` | `array` | `deque` | `list` | `forward_list` |
|-----------|---------|---------|---------|--------|----------------|
| Random access | O(1) | O(1) | O(1) | O(n) | O(n) |
| Push back | O(1)* | N/A | O(1) | O(1) | N/A |
| Push front | O(n) | N/A | O(1) | O(1) | O(1) |
| Insert middle | O(n) | N/A | O(n) | O(1)† | O(1)† |
| Cache friendly | ✅✅ | ✅✅ | ✅ | ❌ | ❌ |

*amortised  †given an iterator

**The golden rule:** Default to `std::vector`. It's almost always the best
choice due to cache locality. Only switch when you have a measured performance
problem that another container solves.

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** Associative Containers.
