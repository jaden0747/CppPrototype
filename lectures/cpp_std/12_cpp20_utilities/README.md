# Lecture 12 — C++20 Utilities

> **Goal:** Master the remaining C++20 powerhouses: the spaceship operator
> (`<=>`), `std::format`, `std::span`, `std::jthread`, and other quality-of-life
> improvements. Understand **what** each solves, **why** it's better than
> the old way, **how** to use it correctly, and **when** to prefer alternatives.

---

## Table of Contents

1. [Three-Way Comparison (`<=>`)](#1-three-way-comparison-)
2. [`std::format`](#2-stdformat)
3. [`std::span`](#3-stdspan)
4. [`std::jthread` & Stop Tokens](#4-stdjthread--stop-tokens)
5. [`std::source_location`](#5-stdsource_location)
6. [Calendar & Time Zones (`<chrono>`)](#6-calendar--time-zones-chrono)
7. [Exercises](#7-exercises)

---

## 1. Three-Way Comparison (`<=>`)

### The problem: writing 6 operators is tedious and error-prone

Before C++20, to make a type fully comparable you needed:

```cpp
struct Point {
    int x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Point& o) const { return !(*this == o); }
    bool operator< (const Point& o) const { return std::tie(x,y) < std::tie(o.x,o.y); }
    bool operator<=(const Point& o) const { return !(o < *this); }
    bool operator> (const Point& o) const { return o < *this; }
    bool operator>=(const Point& o) const { return !(*this < o); }
};
// 6 operators, all boilerplate!
```

### The solution: one `= default` generates all six

```cpp
#include <compare>

struct Point {
    int x, y;
    auto operator<=>(const Point&) const = default;
};

Point a{1, 2}, b{1, 3};
a < b;   // true (lexicographic: first by x, then by y)
a == b;  // false
a != b;  // true
a >= b;  // false
// All six operators work — generated from <=>
```

### How does `<=>` work?

The spaceship operator returns one of three **comparison categories**:

| Category | Meaning | When to use |
|----------|---------|-------------|
| `std::strong_ordering` | Equal values are substitutable | `int`, `string` — a == b means a and b are identical |
| `std::weak_ordering` | Equal values might differ | Case-insensitive string — "ABC" == "abc" but they're not identical |
| `std::partial_ordering` | Some values can't be compared | `double` — NaN is not comparable to anything |

Each category has these possible values:

```cpp
// strong_ordering:
std::strong_ordering::less
std::strong_ordering::equal
std::strong_ordering::greater

// weak_ordering:
std::weak_ordering::less
std::weak_ordering::equivalent  // not "equal"!
std::weak_ordering::greater

// partial_ordering:
std::partial_ordering::less
std::partial_ordering::equivalent
std::partial_ordering::greater
std::partial_ordering::unordered  // NaN comparisons
```

### Writing a custom spaceship operator

```cpp
struct CaseInsensitiveString {
    std::string data;

    std::weak_ordering operator<=>(const CaseInsensitiveString& other) const {
        // weak_ordering because "ABC" ≡ "abc" but they're different strings
        auto to_lower = [](unsigned char c) { return std::tolower(c); };

        auto lhs = data;
        auto rhs = other.data;
        std::ranges::transform(lhs, lhs.begin(), to_lower);
        std::ranges::transform(rhs, rhs.begin(), to_lower);

        return lhs <=> rhs;  // string has strong_ordering, implicitly converts to weak
    }

    // Note: when you define <=>, you often need == separately for efficiency
    bool operator==(const CaseInsensitiveString& other) const {
        if (data.size() != other.data.size()) return false;
        return (*this <=> other) == 0;
    }
};
```

### Key insight: `==` and `<=>` are separate

The compiler generates:
- `==` and `!=` from `operator==()`
- `<`, `<=`, `>`, `>=` from `operator<=>()`

When you write `auto operator<=>(const T&) const = default;`, it generates
BOTH `==` and `<=>`. But if you write a custom `<=>`, you should also provide
`==` for efficiency (equality can often short-circuit on size).

### When to use which category

```cpp
// Strong ordering: most types
struct Point { int x, y; auto operator<=>(const Point&) const = default; };

// Weak ordering: case-insensitive, locale-aware, semantic equivalence
struct Grade { /* A == a, but they're different chars */ };

// Partial ordering: mathematical types with undefined comparisons
struct FloatWrapper {
    double val;
    std::partial_ordering operator<=>(const FloatWrapper& o) const {
        return val <=> o.val;  // double already gives partial_ordering (NaN)
    }
};
```

---

## 2. `std::format`

### What is it?

Type-safe, Python-like string formatting that replaces both `printf` (unsafe)
and `iostream` (verbose):

```cpp
#include <format>

std::string s = std::format("Hello, {}!", "world");  // "Hello, world!"
std::string n = std::format("{:>10}", 42);           // "        42"
std::string f = std::format("{:.2f}", 3.14159);      // "3.14"
```

### Why not printf? Why not iostream?

| Feature | `printf` | `iostream` | `std::format` |
|---------|----------|-----------|---------------|
| Type safety | ❌ UB on wrong format | ✅ | ✅ |
| Readability | Medium | Poor | ✅ Best |
| Extensible | ❌ | ✅ `operator<<` | ✅ Formatter specialization |
| Performance | Fast | Slow | Fast |
| Localization | Partial | Partial | ✅ Full |
| Positional args | Non-standard | ❌ | ✅ |

```cpp
// printf: type-unsafe, no custom types
printf("x=%d y=%f\n", x, y);  // wrong specifier = UB!

// iostream: verbose, can't reorder
std::cout << "x=" << x << " y=" << y << "\n";

// format: clear, safe, positional
std::format("x={0} y={1}", x, y);  // or just {}, {}
```

### Format specifiers — the mini-language

```
{[arg_id]:[fill][align][sign][#][0][width][.precision][type]}
```

| Component | Options | Example |
|-----------|---------|---------|
| fill | Any character | `{:*>10}` → `"********42"` |
| align | `<` left, `>` right, `^` center | `{:^10}` → `"    42    "` |
| sign | `+` always, `-` negative only, ` ` space | `{:+d}` → `"+42"` |
| `#` | Alternative form | `{:#x}` → `"0x2a"` |
| `0` | Zero-pad | `{:05d}` → `"00042"` |
| width | Minimum width | `{:10}` |
| .precision | Decimal places (float) or max chars (string) | `{:.3f}` → `"3.142"` |
| type | `d` dec, `x` hex, `b` bin, `e` sci, `f` fixed | `{:b}` → `"101010"` |

### Practical examples

```cpp
// Table formatting
std::format("{:<15} {:>8} {:>8}", "Name", "Score", "Grade");
// "Name                Score    Grade"

// Numbers
std::format("{:+.2f}", -3.14);      // "-3.14"
std::format("{:#010x}", 255);       // "0x000000ff"
std::format("{:b}", 42);            // "101010"

// Positional arguments (reuse)
std::format("{0} + {0} = {1}", 3, 6);  // "3 + 3 = 6"
```

### Custom formatters for your types

```cpp
struct Point { int x, y; };

template<>
struct std::formatter<Point> : std::formatter<std::string> {
    auto format(const Point& p, auto& ctx) const {
        return std::formatter<std::string>::format(
            std::format("({}, {})", p.x, p.y), ctx);
    }
};

std::format("Position: {}", Point{3, 4});  // "Position: (3, 4)"
```

---

## 3. `std::span`

### What is it?

A **non-owning view** over a contiguous sequence of elements. Think of it
as a "fat pointer": a pointer + a length.

### The problem it solves

```cpp
// Without span: which overload do you write?
void process(std::vector<int>& v);      // only vectors
void process(int* data, size_t len);    // C-style, error-prone
void process(std::array<int, 5>& arr);  // only arrays of size 5

// With span: ONE function handles ALL contiguous data
void process(std::span<const int> data) {
    for (int x : data) std::cout << x << " ";
}

std::vector<int> v{1, 2, 3};
int arr[] = {4, 5, 6};
std::array<int, 3> a{7, 8, 9};

process(v);    // ✅
process(arr);  // ✅
process(a);    // ✅
process({v.data(), 2});  // first 2 elements only
```

### Static vs dynamic extent

```cpp
// Dynamic extent: size known at runtime (most common)
std::span<int> dynamic_span = vec;  // span<int, std::dynamic_extent>

// Static extent: size known at compile time (enables more optimizations)
std::span<int, 5> fixed_span = arr5;  // exactly 5 elements
// fixed_span = vec;  // ❌ Error: vector doesn't have static size 5
```

### Key operations

```cpp
std::span<int> s = vec;
s.size();            // number of elements
s.size_bytes();      // size in bytes
s.data();            // underlying pointer
s.front();           // first element
s.back();            // last element
s[i];                // indexed access
s.subspan(2, 3);     // elements [2, 3, 4]
s.first(3);          // first 3 elements
s.last(2);           // last 2 elements
s.empty();           // is it empty?
```

### `span` vs `string_view`

| | `span<T>` | `string_view` |
|-|-----------|---------------|
| For | Any contiguous data | Characters only |
| Operations | Subspan, indexing | Find, substr, starts_with |
| Mutability | `span<T>` is mutable, `span<const T>` is not | Always const |

### Lifetime rules (same as string_view)

```cpp
// ❌ Dangling span:
std::span<int> bad() {
    std::vector<int> local{1, 2, 3};
    return local;  // local destroyed! span dangles!
}

// ✅ Safe: span refers to something that outlives it
void good(std::span<const int> data) { /* data valid for duration of call */ }
```

---

## 4. `std::jthread` & Stop Tokens

### What is `jthread`?

`jthread` = `std::thread` + two improvements:
1. **Auto-join**: destructor calls `join()` automatically (no more forgotten joins)
2. **Stop tokens**: built-in cooperative cancellation

### The problem with `std::thread`

```cpp
void bad_example() {
    std::thread t([] { /* work */ });
    if (error_condition) return;  // ❌ CRASH! t not joined, std::terminate()
}
// Must remember to join() or detach() in ALL code paths
```

### `jthread` auto-join

```cpp
void good_example() {
    std::jthread t([] { /* work */ });
    if (error_condition) return;  // ✅ ~jthread() auto-joins, waits for thread
}
```

### Cooperative cancellation with stop tokens

```cpp
#include <thread>
#include <stop_token>

std::jthread worker([](std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        // do some work...
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Worker stopped cleanly\n";
});

// Later (from any thread):
worker.request_stop();  // sets the stop flag
// ~jthread() will auto-join, waiting for the loop to see the flag and exit
```

### How is this better than `atomic<bool>`?

```cpp
// Old way: manual boolean flag
std::atomic<bool> should_stop{false};
std::thread t([&] {
    while (!should_stop) { /* work */ }
});
should_stop = true;
t.join();  // must remember!

// New way: built-in, no manual flag, auto-join
std::jthread t([](std::stop_token st) {
    while (!st.stop_requested()) { /* work */ }
});
t.request_stop();
// auto-joins in destructor
```

### `stop_callback` — react to cancellation

```cpp
std::jthread t([](std::stop_token st) {
    // Register a callback that fires when stop is requested:
    std::stop_callback cb(st, [] {
        std::cout << "Cancellation requested! Cleaning up...\n";
    });

    while (!st.stop_requested()) {
        // do work...
    }
});
```

---

## 5. `std::source_location`

### What is it?

Compile-time access to the caller's file, line, column, and function name.
Replaces `__FILE__` and `__LINE__` macros with a type-safe mechanism.

### The problem with macros

```cpp
// Macros capture the WRONG location when wrapped:
#define LOG(msg) log_impl(__FILE__, __LINE__, msg)

void log_impl(const char* file, int line, std::string_view msg) {
    std::cout << file << ":" << line << " " << msg << "\n";
}
// LOG("hello") → prints the location of the LOG macro call ✅
// But if you wrap it in another function, it's the wrong location
```

### The solution

```cpp
#include <source_location>

void log(std::string_view msg,
         std::source_location loc = std::source_location::current())
{
    std::cout << loc.file_name() << ":" << loc.line()
              << " [" << loc.function_name() << "] " << msg << "\n";
}

// In main.cpp, line 42:
log("something happened");
// Output: "main.cpp:42 [main] something happened"
```

### Why does this work?

The default argument `std::source_location::current()` is evaluated at the
**call site**, not inside the function. So it captures the caller's location.

### Available information

| Member | Returns |
|--------|---------|
| `loc.file_name()` | Source file path |
| `loc.line()` | Line number (1-based) |
| `loc.column()` | Column number |
| `loc.function_name()` | Enclosing function name |

---

## 6. Calendar & Time Zones (`<chrono>`)

### What's new in C++20 chrono?

C++11 gave us durations and clocks. C++20 adds:
- **Calendar types**: year, month, day, weekday
- **Time zones**: convert between zones
- **Formatting**: `std::format` integration

### Calendar dates

```cpp
#include <chrono>
using namespace std::chrono;

// Construct dates
auto christmas = 2024y/December/25;          // year_month_day
auto leap_day = February/29/2024y;           // same type, different syntax
auto today = year_month_day{floor<days>(system_clock::now())};

// Arithmetic
auto next_week = sys_days{today} + days{7};
auto next_month = today.year()/today.month() + months{1}/today.day();

// Queries
christmas.ok();     // true (valid date)
(2024y/February/30).ok();  // false (invalid!)
```

### Time zones

```cpp
auto now = system_clock::now();
auto ny_time = zoned_time{"America/New_York", now};
auto tokyo_time = zoned_time{"Asia/Tokyo", now};

std::cout << std::format("NY:    {}\n", ny_time);
std::cout << std::format("Tokyo: {}\n", tokyo_time);
```

---

## 7. Exercises

See `exercises.cpp`:

1. Add `<=>` to a custom class and verify all 6 operators work
2. Format a table of data using `std::format` with alignment
3. Write functions accepting `std::span` instead of vector references
4. Use `jthread` + stop tokens for cooperative cancellation
5. Build a logging system using `source_location`
6. Implement a custom formatter for a complex type

---

**Next lecture:** C++23 Additions — deducing this, std::expected,
std::print, multidimensional subscript.
