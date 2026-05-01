# Stdlib 08 — Numeric, Set & Heap Algorithms

> **Goal:** Master numeric algorithms (`accumulate`, `reduce`, `transform_reduce`),
> set operations on sorted ranges, heap operations, binary search, and
> utilities from `<bit>`, `<random>`, and `<cmath>`.

---

## Table of Contents

1. [Numeric Algorithms Overview](#1-numeric-algorithms-overview)
2. [`accumulate` and `reduce`](#2-accumulate-and-reduce)
3. [`transform_reduce` — Map + Reduce in One Step](#3-transform_reduce)
4. [`partial_sum`, `inclusive_scan`, `exclusive_scan`](#4-partial_sum-and-scans)
5. [`adjacent_difference`](#5-adjacent_difference)
6. [`gcd`, `lcm`, `midpoint`, `lerp`](#6-math-utilities)
7. [Set Operations on Sorted Ranges](#7-set-operations)
8. [Heap Operations](#8-heap-operations)
9. [Binary Search](#9-binary-search)
10. [`<bit>` Utilities (C++20)](#10-bit-utilities)
11. [`<random>` — Engines and Distributions](#11-random)
12. [Exercises](#12-exercises)

---

## 1. Numeric Algorithms Overview

### Where are they?

Numeric algorithms live in `<numeric>` (not `<algorithm>`):

```cpp
#include <numeric>   // accumulate, reduce, iota, partial_sum, etc.
#include <cmath>     // sqrt, sin, cos, pow, etc.
#include <bit>       // C++20 bit operations
#include <random>    // random number generation
```

### Why separate?

Historical reasons: the original STL separated "algorithms on sequences"
(`<algorithm>`) from "numeric computations" (`<numeric>`).

---

## 2. `accumulate` and `reduce`

### `std::accumulate` — sequential fold

"Add up all elements" (or any binary operation):

```cpp
std::vector<int> v{1, 2, 3, 4, 5};

int sum = std::accumulate(v.begin(), v.end(), 0);
// sum = 15 (0 + 1 + 2 + 3 + 4 + 5)

int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<>{});
// product = 120 (1 * 1 * 2 * 3 * 4 * 5)
```

### The initial value matters!

```cpp
// Wrong: initial value 0 with multiplication gives 0!
std::accumulate(v.begin(), v.end(), 0, std::multiplies<>{});  // 0!

// Wrong type: int initial value truncates doubles!
std::vector<double> d{1.5, 2.5, 3.5};
auto bad = std::accumulate(d.begin(), d.end(), 0);   // 6 (int!)
auto good = std::accumulate(d.begin(), d.end(), 0.0); // 7.5 (double)
```

### `std::reduce` (C++17) — parallelizable version

```cpp
int sum = std::reduce(v.begin(), v.end());  // same as accumulate for +
// But: can be executed in parallel! (unspecified order)
```

### `accumulate` vs `reduce` — when to use which

| Feature | `accumulate` | `reduce` |
|---------|-------------|---------|
| Order guaranteed? | ✅ Left-to-right | ❌ Unspecified |
| Parallel execution? | ❌ No | ✅ With execution policy |
| Operation must be? | Any | Associative + commutative |
| Header | `<numeric>` | `<numeric>` |

**Rule:** Use `reduce` when the operation is associative and commutative
(addition, multiplication, min, max). Use `accumulate` when order matters
(string concatenation, building a result from left to right).

### Parallel reduce

```cpp
#include <execution>
int sum = std::reduce(std::execution::par, v.begin(), v.end());
// Uses multiple threads!
```

---

## 3. `transform_reduce` — Map + Reduce in One Step

### What is it?

Combine `transform` and `reduce` — apply a function to each element, then
accumulate the results. This is the classic **map-reduce** pattern.

### Unary form: transform each, then sum

```cpp
std::vector<int> v{1, 2, 3, 4, 5};

// Sum of squares: transform(x → x²) then reduce(+)
int sum_sq = std::transform_reduce(
    v.begin(), v.end(),
    0,                    // initial value
    std::plus<>{},        // reduce operation
    [](int x) { return x * x; }  // transform
);
// sum_sq = 1 + 4 + 9 + 16 + 25 = 55
```

### Binary form: dot product

```cpp
std::vector<int> a{1, 2, 3};
std::vector<int> b{4, 5, 6};

int dot = std::transform_reduce(
    a.begin(), a.end(),
    b.begin(),
    0  // initial value
);
// dot = 1*4 + 2*5 + 3*6 = 32
```

### Why not just transform then accumulate?

`transform_reduce` is:
1. **One pass** instead of two
2. **Parallelizable** (with execution policy)
3. **No intermediate container** needed

---

## 4. `partial_sum` and Scans

### `std::partial_sum` — running total

```cpp
std::vector<int> v{1, 2, 3, 4, 5};
std::vector<int> result(v.size());

std::partial_sum(v.begin(), v.end(), result.begin());
// result = {1, 3, 6, 10, 15}
// i.e., {1, 1+2, 1+2+3, 1+2+3+4, 1+2+3+4+5}
```

### `std::inclusive_scan` (C++17) — parallel-friendly version

```cpp
std::inclusive_scan(v.begin(), v.end(), result.begin());
// Same result as partial_sum, but can be parallelized
```

### `std::exclusive_scan` (C++17) — excludes current element

```cpp
std::exclusive_scan(v.begin(), v.end(), result.begin(), 0);
// result = {0, 1, 3, 6, 10}
// Each position is the sum of all PREVIOUS elements
```

### When to use which

| Algorithm | result[i] = | Use case |
|-----------|------------|----------|
| `partial_sum` | sum of v[0..i] | Running total, prefix sums |
| `inclusive_scan` | sum of v[0..i] (parallel-safe) | Same, but parallel |
| `exclusive_scan` | sum of v[0..i-1] | Offsets, "how much before me" |

---

## 5. `adjacent_difference`

### What is it?

Computes the difference between consecutive elements:

```cpp
std::vector<int> v{1, 3, 6, 10, 15};
std::vector<int> diff(v.size());

std::adjacent_difference(v.begin(), v.end(), diff.begin());
// diff = {1, 2, 3, 4, 5}
// i.e., {v[0], v[1]-v[0], v[2]-v[1], v[3]-v[2], v[4]-v[3]}
```

**It's the inverse of `partial_sum`!**

### Use case: velocity from positions

```cpp
std::vector<double> positions{0, 1, 4, 9, 16, 25}; // position over time
std::vector<double> velocity(positions.size());
std::adjacent_difference(positions.begin(), positions.end(), velocity.begin());
// velocity = {0, 1, 3, 5, 7, 9} — changes between time steps
```

---

## 6. Math Utilities

### `std::gcd` and `std::lcm` (C++17)

```cpp
#include <numeric>

std::gcd(12, 8);   // 4 — greatest common divisor
std::lcm(4, 6);    // 12 — least common multiple
```

### `std::midpoint` (C++20) — safe midpoint calculation

```cpp
#include <numeric>

std::midpoint(0, 10);        // 5
std::midpoint(0.0, 1.0);     // 0.5

// Why not just (a + b) / 2?
// Because a + b can OVERFLOW for large integers!
// midpoint handles this correctly.
```

### `std::lerp` (C++20) — linear interpolation

```cpp
#include <cmath>

std::lerp(0.0, 10.0, 0.5);  // 5.0  — halfway between 0 and 10
std::lerp(0.0, 10.0, 0.25); // 2.5  — quarter of the way
// lerp(a, b, t) = a + t * (b - a)
```

---

## 7. Set Operations on Sorted Ranges

### Prerequisites: inputs must be sorted!

Set operations treat sorted ranges as mathematical sets.

### `std::set_union` — combine two sets (A ∪ B)

```cpp
std::vector<int> a{1, 2, 3, 5, 7};
std::vector<int> b{2, 4, 5, 6, 8};
std::vector<int> result;

std::ranges::set_union(a, b, std::back_inserter(result));
// result = {1, 2, 3, 4, 5, 6, 7, 8}
```

### `std::set_intersection` — common elements (A ∩ B)

```cpp
std::ranges::set_intersection(a, b, std::back_inserter(result));
// result = {2, 5}
```

### `std::set_difference` — in A but not B (A \ B)

```cpp
std::ranges::set_difference(a, b, std::back_inserter(result));
// result = {1, 3, 7}
```

### `std::set_symmetric_difference` — in A or B but not both (A △ B)

```cpp
std::ranges::set_symmetric_difference(a, b, std::back_inserter(result));
// result = {1, 3, 4, 6, 7, 8}
```

### `std::includes` — is A a superset of B?

```cpp
std::ranges::includes(a, std::vector{2, 5}); // true — a contains 2 and 5
```

---

## 8. Heap Operations

### What is a heap?

A **binary heap** is a tree stored in an array where the largest element is
always at the front (max-heap). It's the data structure behind `priority_queue`.

### Why use heap operations directly?

`priority_queue` doesn't expose iterators. If you need to iterate the
underlying data, or need more control, use heap operations on a `vector`.

### The four operations

```cpp
std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};

// 1. make_heap: turn array into a heap
std::make_heap(v.begin(), v.end());
// v[0] is now the max (9)

// 2. push_heap: after pushing to back, restore heap property
v.push_back(7);
std::push_heap(v.begin(), v.end());

// 3. pop_heap: move max to back, shrink heap
std::pop_heap(v.begin(), v.end()); // moves max to v.back()
int max_val = v.back();            // 9
v.pop_back();

// 4. sort_heap: sort a heap in ascending order (destroys heap property)
std::sort_heap(v.begin(), v.end());
```

---

## 9. Binary Search

### Prerequisites: the range must be sorted!

### `std::lower_bound` — first element >= value

```cpp
std::vector<int> v{1, 2, 4, 4, 4, 6, 8};

auto it = std::lower_bound(v.begin(), v.end(), 4);
// points to first 4 (index 2)
```

### `std::upper_bound` — first element > value

```cpp
auto it = std::upper_bound(v.begin(), v.end(), 4);
// points to 6 (index 5) — first element AFTER all 4's
```

### `std::equal_range` — range of equal elements

```cpp
auto [lo, hi] = std::equal_range(v.begin(), v.end(), 4);
// lo points to first 4, hi points to 6
// distance(lo, hi) = 3 — there are three 4's
```

### `std::binary_search` — does the value exist?

```cpp
bool found = std::binary_search(v.begin(), v.end(), 4); // true
```

### When to use which

| Function | Returns | Use when |
|----------|---------|----------|
| `lower_bound` | Iterator to first >= val | Insertion point for maintaining sorted order |
| `upper_bound` | Iterator to first > val | End of equal range |
| `equal_range` | Pair of iterators | Need both bounds |
| `binary_search` | `bool` | Just need to know if present |

---

## 10. `<bit>` Utilities (C++20)

```cpp
#include <bit>

std::popcount(0b1011u);         // 3 — number of 1 bits
std::countl_zero(uint8_t(1));   // 7 — leading zeros
std::countr_zero(uint8_t(4));   // 2 — trailing zeros
std::has_single_bit(8u);        // true — is power of 2
std::bit_ceil(5u);              // 8 — next power of 2
std::bit_floor(5u);             // 4 — previous power of 2
std::bit_cast<float>(0x3F800000u); // 1.0f — reinterpret bits
```

---

## 11. `<random>` — Engines and Distributions

### Why not `rand()`?

`rand()` is terrible: low-quality randomness, global state, not thread-safe,
modulo bias with `rand() % n`.

### Modern random number generation

```cpp
#include <random>

// Step 1: Create an engine (generates raw random bits)
std::mt19937 gen(std::random_device{}()); // Mersenne Twister, seeded randomly

// Step 2: Create a distribution (shapes the output)
std::uniform_int_distribution<int> dice(1, 6);
std::normal_distribution<double> gaussian(0.0, 1.0);

// Step 3: Generate numbers
int roll = dice(gen);         // random integer in [1, 6]
double sample = gaussian(gen); // random from N(0, 1)
```

### Common distributions

| Distribution | What it generates |
|-------------|------------------|
| `uniform_int_distribution<int>(a, b)` | Integer in [a, b] |
| `uniform_real_distribution<double>(a, b)` | Double in [a, b) |
| `normal_distribution<double>(mean, stddev)` | Gaussian bell curve |
| `bernoulli_distribution(p)` | `true` with probability p |
| `discrete_distribution<int>({w1, w2, ...})` | Weighted integer |

---

## 12. Exercises

See `exercises.cpp`.

---

**Next lecture:** Strings & Text.
