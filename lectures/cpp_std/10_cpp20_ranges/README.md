# Lecture 10 — C++20 Ranges

> **Goal:** Master the Ranges library — lazy, composable views that replace
> hand-written loops and transform algorithms into pipeable expressions.
> Understand **why** ranges improve on classic STL, **how** laziness works,
> **when** to use views vs algorithms, and **what** projections enable.

---

## Table of Contents

1. [Why Ranges?](#1-why-ranges)
2. [Range Concepts](#2-range-concepts)
3. [Views & View Adaptors](#3-views--view-adaptors)
4. [Pipe Operator `|`](#4-pipe-operator-)
5. [Range Algorithms](#5-range-algorithms)
6. [Projections](#6-projections)
7. [Custom Views](#7-custom-views)
8. [Common Pitfalls](#8-common-pitfalls)
9. [Exercises](#9-exercises)

---

## 1. Why Ranges?

### The problem with classic STL algorithms

```cpp
std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Problem 1: Verbose — must pass begin/end iterators
std::vector<int> evens;
std::copy_if(v.begin(), v.end(), std::back_inserter(evens),
             [](int x){ return x % 2 == 0; });

// Problem 2: Can't compose — need intermediate containers
std::vector<int> squared;
std::transform(evens.begin(), evens.end(), std::back_inserter(squared),
               [](int x){ return x * x; });

// Problem 3: Easy to mix up begin/end from different containers
// std::sort(v.begin(), w.end());  // compiles but UB!
```

### The ranges solution

```cpp
auto result = v
    | std::views::filter([](int x){ return x % 2 == 0; })
    | std::views::transform([](int x){ return x * x; });
// result = [4, 16, 36, 64, 100] — computed LAZILY, no allocations!
```

### Three key improvements

| Problem | Classic STL | Ranges |
|---------|-------------|--------|
| Verbosity | `algo(v.begin(), v.end())` | `algo(v)` or `v \| view` |
| Composition | Intermediate containers | Pipe `\|` chains |
| Safety | Can mix iterators from different containers | Pass whole range |

---

## 2. Range Concepts

### What is a range?

Any type with `begin()` and `end()`. That's it. Vectors, arrays, strings,
spans — all ranges.

### The concept hierarchy

| Concept | Meaning | Example |
|---------|---------|---------|
| `std::ranges::range` | Has begin/end | Everything below |
| `std::ranges::input_range` | Single-pass readable | `istream_view` |
| `std::ranges::forward_range` | Multi-pass | `forward_list` |
| `std::ranges::bidirectional_range` | Can go backwards | `list`, `set` |
| `std::ranges::random_access_range` | O(1) indexed access | `deque` |
| `std::ranges::contiguous_range` | Elements adjacent in memory | `vector`, `array`, `span` |
| `std::ranges::sized_range` | Has O(1) `.size()` | `vector`, `array` |
| `std::ranges::view` | Lightweight, O(1) copy/move | All views |

Each level adds capabilities. A `contiguous_range` is also a
`random_access_range`, which is also `bidirectional_range`, etc.

### What makes a view special?

A view is:
- Non-owning (doesn't own the data it refers to)
- O(1) move and (if copyable) copy
- O(1) destruction

This is what enables lazy composition without allocations.

---

## 3. Views & View Adaptors

### Factory views — create ranges from nothing

```cpp
namespace v = std::views;

v::iota(1, 11)          // [1, 2, 3, ..., 10]
v::iota(0)              // [0, 1, 2, ...] (infinite!)
v::empty<int>           // empty range of int
v::single(42)           // [42]
v::repeat(7)            // [7, 7, 7, ...] (infinite, C++23)
```

### Adaptor views — transform existing ranges

| View | What it does | Example |
|------|--------------|---------|
| `filter(pred)` | Keep elements where pred is true | `v \| filter(is_even)` |
| `transform(f)` | Apply f to each element | `v \| transform(square)` |
| `take(n)` | First n elements | `v \| take(5)` |
| `drop(n)` | Skip first n elements | `v \| drop(3)` |
| `take_while(pred)` | Take while pred is true | `v \| take_while(positive)` |
| `drop_while(pred)` | Skip while pred is true | `v \| drop_while(negative)` |
| `reverse` | Reverse iteration | `v \| reverse` |
| `elements<I>` | I-th element of tuple-like | `map \| elements<0>` |
| `keys` | First element (elements<0>) | `map \| keys` |
| `values` | Second element (elements<1>) | `map \| values` |
| `join` | Flatten nested ranges | `vec_of_vecs \| join` |
| `split(delim)` | Split by delimiter | `str \| split(' ')` |
| `common` | Make begin/end same type | For legacy algorithms |
| `counted(it, n)` | n elements from iterator | `counted(it, 5)` |

### How laziness works

```cpp
auto pipeline = numbers
    | std::views::filter([](int x) { return x % 2 == 0; })
    | std::views::transform([](int x) { return x * x; })
    | std::views::take(3);

// NO WORK DONE YET! pipeline is just a description.

// Work happens element-by-element when you iterate:
for (int x : pipeline) {
    // For each element: check filter → apply transform → check take count
    std::cout << x << " ";
}
```

Each element flows through the entire pipeline before the next element starts.
No intermediate containers. No wasted work on elements beyond `take(3)`.

---

## 4. Pipe Operator `|`

### How it works

The `|` operator is syntactic sugar for nesting:

```cpp
// These are identical:
auto r1 = v | views::filter(f) | views::transform(g) | views::take(3);
auto r2 = views::take(views::transform(views::filter(v, f), g), 3);
```

The pipe reads left-to-right (data flow direction), which is much more natural.

### Building reusable pipelines

```cpp
// You can store a partial pipeline (without the range):
auto even_squares = std::views::filter([](int x) { return x % 2 == 0; })
                  | std::views::transform([](int x) { return x * x; });

// Apply to any range later:
auto result1 = vec1 | even_squares | std::views::take(5);
auto result2 = vec2 | even_squares;
```

### Materializing — converting back to a container

```cpp
// C++20: use range constructor
auto pipeline = v | std::views::filter(pred) | std::views::transform(f);
std::vector<int> result(pipeline.begin(), pipeline.end());

// Or with ranges::copy
std::vector<int> result;
std::ranges::copy(pipeline, std::back_inserter(result));

// C++23: elegant!
auto result = pipeline | std::ranges::to<std::vector>();
```

---

## 5. Range Algorithms

All `<algorithm>` functions have range-based overloads in `std::ranges::`:

```cpp
std::vector<int> v{5, 2, 8, 1, 4};

// Pass the RANGE, not begin/end:
std::ranges::sort(v);
std::ranges::reverse(v);
auto it = std::ranges::find(v, 4);
auto count = std::ranges::count_if(v, [](int x) { return x > 3; });
bool any = std::ranges::any_of(v, [](int x) { return x < 0; });
auto [mn, mx] = std::ranges::minmax(v);
```

### Why prefer `std::ranges::` over `std::`?

| Feature | `std::sort(begin, end)` | `std::ranges::sort(range)` |
|---------|-------------------------|---------------------------|
| Pass range directly | ❌ | ✅ |
| Return useful values | iterator | `subrange` (remaining range) |
| Projections | ❌ | ✅ |
| Concept-constrained | ❌ | ✅ (clear errors) |
| ADL-proof | ❌ (Niebloid issue) | ✅ |

---

## 6. Projections

### What are projections?

A projection is a function applied to each element **before** the algorithm
operates on it. It's like a `transform` built into the algorithm.

```cpp
struct Person { std::string name; int age; };
std::vector<Person> people{{"Alice", 30}, {"Bob", 25}, {"Charlie", 35}};

// Sort by age (projection = &Person::age)
std::ranges::sort(people, {}, &Person::age);
// people = [Bob(25), Alice(30), Charlie(35)]

// Find by name
auto it = std::ranges::find(people, "Bob", &Person::name);

// Max by age
auto oldest = std::ranges::max(people, {}, &Person::age);
// oldest = Charlie(35)
```

### Why not just use a custom comparator?

```cpp
// Custom comparator (verbose, error-prone):
std::ranges::sort(people, [](const Person& a, const Person& b) {
    return a.age < b.age;
});

// Projection (concise, reuses default comparator):
std::ranges::sort(people, {}, &Person::age);
// {} means std::ranges::less{} — the default comparator
```

Projections work with ALL range algorithms — find, count, min, max, etc.

---

## 7. Custom Views

### Creating your own view

Inherit from `std::ranges::view_interface` for free `empty()`, `size()`,
`operator bool`, `front()`, `back()`, `operator[]` when applicable:

```cpp
template<std::ranges::input_range R>
class stride_view : public std::ranges::view_interface<stride_view<R>> {
    R base_;
    std::size_t stride_;

    struct iterator {
        std::ranges::iterator_t<R> current_;
        std::ranges::sentinel_t<R> end_;
        std::size_t stride_;

        iterator& operator++() {
            for (std::size_t i = 0; i < stride_ && current_ != end_; ++i)
                ++current_;
            return *this;
        }
        auto& operator*() const { return *current_; }
        bool operator==(std::default_sentinel_t) const { return current_ == end_; }
    };

public:
    stride_view(R base, std::size_t stride)
        : base_(std::move(base)), stride_(stride) {}

    auto begin() { return iterator{std::ranges::begin(base_),
                                    std::ranges::end(base_), stride_}; }
    auto end() { return std::default_sentinel; }
};

// Usage:
auto every_third = vec | stride_view(3);  // {v[0], v[3], v[6], ...}
```

---

## 8. Common Pitfalls

### Dangling iterators

```cpp
// ❌ DANGLING — temporary vector destroyed!
auto it = std::ranges::find(get_vector(), 42);
// The vector returned by get_vector() is a temporary.
// The library returns std::ranges::dangling instead of an iterator!

// ✅ Store the range first:
auto v = get_vector();
auto it = std::ranges::find(v, 42);
```

### Views are lazy — side effects happen on iteration

```cpp
int count = 0;
auto counted = v | std::views::transform([&count](int x) {
    ++count;  // side effect!
    return x * 2;
});
// count is still 0 here!
for (auto x : counted) { /* now count increments */ }
```

### Views don't own data — lifetime matters

```cpp
// ❌ Dangling view:
auto bad_view() {
    std::vector<int> local{1, 2, 3};
    return local | std::views::take(2);  // local destroyed!
}

// ✅ Return the container, let caller create views
std::vector<int> good_data() { return {1, 2, 3}; }
```

---

## 9. Exercises

See `exercises.cpp`:

1. Rewrite loop-based code using views + pipes
2. Use `views::iota` + `filter` + `transform` to generate first N primes
3. Split a string into words using `views::split`
4. Use projections with `ranges::sort` and `ranges::find`
5. Compose multiple views to build a data processing pipeline
6. Implement a custom view (e.g., `stride_view` or `zip_view`)

---

**Next lecture:** C++20 Coroutines & Modules.
