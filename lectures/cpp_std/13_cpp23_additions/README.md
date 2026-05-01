# Lecture 13 — C++23 Additions

> **Goal:** Explore the latest C++23 features: deducing `this`, `std::expected`,
> `std::print`, multidimensional subscript operator, and other refinements.
> Understand **what** each feature enables, **why** it was needed, **how** it
> works under the hood, and **when** to use it in practice.

---

## Table of Contents

1. [Deducing `this`](#1-deducing-this)
2. [`std::expected<T, E>`](#2-stdexpectedt-e)
3. [`std::print` / `std::println`](#3-stdprint--stdprintln)
4. [Multidimensional Subscript `operator[]`](#4-multidimensional-subscript-operator)
5. [`std::ranges::to`](#5-stdrangesto)
6. [Other C++23 Gems](#6-other-c23-gems)
7. [Exercises](#7-exercises)

---

## 1. Deducing `this`

### What is it?

In C++23, you can write `this` as an **explicit parameter** and deduce its type:

```cpp
struct Widget {
    template<typename Self>
    auto&& get_name(this Self&& self) {
        return std::forward<Self>(self).name_;
    }
private:
    std::string name_;
};
```

### Why was this needed? The 4-overload problem

Before C++23, to support all combinations of const/non-const × lvalue/rvalue,
you had to write **four** overloads:

```cpp
struct Widget {
    std::string& get_name() & { return name_; }
    const std::string& get_name() const& { return name_; }
    std::string&& get_name() && { return std::move(name_); }
    const std::string&& get_name() const&& { return std::move(name_); }
    // FOUR versions of the same function!
};
```

With deducing `this`: **one template** handles all four cases.

### How does the deduction work?

The compiler deduces `Self` based on how the object is used:

```cpp
Widget w;
const Widget cw;

w.get_name();              // Self = Widget&       → returns string&
cw.get_name();             // Self = const Widget& → returns const string&
std::move(w).get_name();   // Self = Widget        → returns string&&
Widget{}.get_name();       // Self = Widget        → returns string&&
```

### Use case 1: CRTP without inheriting from a template

```cpp
// Old CRTP (complex, exposes implementation):
template<typename Derived>
struct Base {
    void interface() { static_cast<Derived*>(this)->impl(); }
};
struct Derived : Base<Derived> {
    void impl() { std::cout << "Derived!\n"; }
};

// C++23 (no template base class needed!):
struct Base {
    template<typename Self>
    void interface(this Self& self) {
        self.impl();  // calls the most-derived version
    }
};
struct Derived : Base {
    void impl() { std::cout << "Derived!\n"; }
};
```

### Use case 2: Recursive lambdas

Before C++23, a lambda couldn't call itself without a `std::function` wrapper:

```cpp
// Before: need std::function (heap allocation, type erasure)
std::function<int(int)> fib = [&fib](int n) -> int {
    return n <= 1 ? n : fib(n-1) + fib(n-2);
};

// C++23: lambda can name itself via deducing this
auto fib = [](this auto self, int n) -> int {
    if (n <= 1) return n;
    return self(n-1) + self(n-2);
};
fib(10);  // 55 — no std::function, no overhead!
```

### Use case 3: Deduplicate const/non-const overloads

```cpp
struct Container {
    template<typename Self>
    auto&& operator[](this Self&& self, size_t i) {
        return std::forward<Self>(self).data_[i];
    }
private:
    std::vector<int> data_;
};
// One function handles: container[i], const_container[i], std::move(container)[i]
```

---

## 2. `std::expected<T, E>`

### What is it?

A type that holds either a **success value** of type `T` or an **error** of
type `E`. Like `optional`, but the "empty" state carries error information.

### The problem: how to report errors?

| Mechanism | Pros | Cons |
|-----------|------|------|
| Exceptions | Separates error/happy path | Performance hit, hidden control flow |
| Error codes | Fast, explicit | Easy to ignore, no context |
| `optional` | Clean API | Can't say WHY it failed |
| `expected` | Clean API + error info | Requires C++23 |

### Basic usage

```cpp
#include <expected>

std::expected<int, std::string> parse_int(std::string_view sv) {
    try {
        return std::stoi(std::string(sv));
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

auto result = parse_int("42");
if (result) {
    std::cout << "Value: " << *result << "\n";   // 42
} else {
    std::cout << "Error: " << result.error() << "\n";
}
```

### Key operations

| Operation | Description |
|-----------|-------------|
| `result.has_value()` | Has a success value? (same as `bool(result)`) |
| `*result` | Access value (UB if error!) |
| `result.value()` | Access value (throws `bad_expected_access` if error) |
| `result.error()` | Access the error |
| `result.value_or(def)` | Value or fallback |

### Monadic operations — functional chaining

This is where `expected` really shines:

```cpp
auto final_value = parse_int("10")
    .transform([](int x) { return x * 2; })            // map: int→int
    .transform([](int x) { return std::to_string(x); }) // map: int→string
    .value_or("error");                                  // unwrap

// .and_then() for operations that can also fail:
std::expected<double, std::string> safe_divide(int a, int b) {
    if (b == 0) return std::unexpected("division by zero");
    return static_cast<double>(a) / b;
}

auto result = parse_int("10")
    .and_then([](int x) { return safe_divide(x, 3); })  // flatmap
    .transform([](double d) { return d * 100; });

// .or_else() for error recovery:
auto recovered = parse_int("bad")
    .or_else([](const std::string& err) -> std::expected<int, std::string> {
        std::cerr << "Recovering from: " << err << "\n";
        return 0;  // default value
    });
```

### `expected` vs `optional` vs exceptions

| Feature | `optional<T>` | `expected<T,E>` | Exceptions |
|---------|---------------|-----------------|------------|
| Success info | ✅ value | ✅ value | ✅ return value |
| Error info | ❌ just "empty" | ✅ error of type E | ✅ exception object |
| Performance | Fast | Fast | Slow on error path |
| Composable | C++23 monadic | C++23 monadic | try/catch nesting |
| Visible in API | ✅ in return type | ✅ in return type | ❌ hidden |

---

## 3. `std::print` / `std::println`

### What is it?

Direct output using `std::format` syntax. Combines formatting and output
in one call:

```cpp
#include <print>

std::println("Hello, {}!", "world");           // Hello, world!\n
std::print("x = {}, y = {}\n", 3, 4);         // x = 3, y = 4\n
std::println("{:>10.2f}", 3.14159);            // "      3.14\n"
std::println("{:#x}", 255);                    // 0xff\n
```

### Why not just `std::cout`?

```cpp
// std::cout: verbose, stateful formatting, slow
std::cout << std::setw(10) << std::setfill('*') << std::hex << 255 << std::endl;
// The fill/width/hex state persists! Easy to forget to reset.

// std::print: concise, no state, fast
std::print("{:*>10x}\n", 255);
// No persistent state — each call is independent.
```

### `println` vs `print`

- `std::print(fmt, args...)` — output without trailing newline
- `std::println(fmt, args...)` — output with trailing newline
- `std::println()` — just outputs `\n`

### Output to file/stream

```cpp
std::ofstream file("output.txt");
std::println(file, "Written to file: {}", 42);
std::println(stderr, "Error: {}", message);
```

---

## 4. Multidimensional Subscript `operator[]`

### The problem

Before C++23, `operator[]` could only take ONE argument:

```cpp
// Had to use operator() or chained [] or proxy objects:
matrix(row, col);          // not subscript syntax
matrix[row][col];          // requires proxy object, confusing
matrix[{row, col}];        // awkward aggregate
```

### C++23 solution

```cpp
struct Matrix {
    double data[3][3];

    double& operator[](size_t row, size_t col) {
        return data[row][col];
    }

    const double& operator[](size_t row, size_t col) const {
        return data[row][col];
    }
};

Matrix m{};
m[1, 2] = 3.14;              // natural mathematical syntax!
double val = m[0, 0];
```

### Works with any number of dimensions

```cpp
template<typename T, size_t... Dims>
struct Tensor {
    T& operator[](auto... indices) requires (sizeof...(indices) == sizeof...(Dims)) {
        // compute flat index from multi-dimensional indices
        return data_[flat_index(indices...)];
    }
};

Tensor<float, 2, 3, 4> t;
t[1, 2, 3] = 1.0f;  // 3D indexing!
```

---

## 5. `std::ranges::to`

### The problem

In C++20, materializing a view into a container was awkward:

```cpp
auto view = numbers | std::views::filter(is_even) | std::views::transform(square);

// C++20: manual, verbose
std::vector<int> result(view.begin(), view.end());
// Or:
std::vector<int> result;
std::ranges::copy(view, std::back_inserter(result));
```

### C++23: `ranges::to`

```cpp
auto vec = std::views::iota(1, 11)
    | std::views::filter([](int x) { return x % 2 == 0; })
    | std::ranges::to<std::vector>();
// vec = {2, 4, 6, 8, 10}

// Works with any container:
auto set = data | std::views::transform(f) | std::ranges::to<std::set>();
auto str = chars | std::ranges::to<std::string>();
auto list = items | std::ranges::to<std::list>();

// Can also specify allocators and other template args:
auto vec = data | std::ranges::to<std::vector<double>>();
```

### Nested containers

```cpp
// Materialize nested ranges:
auto vec_of_vecs = outer_range
    | std::views::transform([](auto inner) {
        return inner | std::ranges::to<std::vector>();
    })
    | std::ranges::to<std::vector>();
```

---

## 6. Other C++23 Gems

### `std::stacktrace`

```cpp
#include <stacktrace>

void crash_handler() {
    auto trace = std::stacktrace::current();
    std::cerr << "Stack trace:\n" << trace << "\n";
}
// Prints a full readable stack trace — no more addr2line!
```

### `if consteval`

Detect whether code is running at compile time:

```cpp
constexpr double compute(double x) {
    if consteval {
        // Compile-time path: must be fully constexpr-safe
        return x * x;
    } else {
        // Runtime path: can use runtime-only features
        return std::sqrt(x);  // not constexpr
    }
}
```

### `std::flat_map` / `std::flat_set`

Cache-friendly sorted containers backed by contiguous memory:

```cpp
#include <flat_map>

std::flat_map<std::string, int> fm{{"apple", 1}, {"banana", 2}};
// Internally: sorted vector of keys + vector of values
// Better cache performance than std::map (which is a tree)
```

**When to use `flat_map` over `map`:**
- Read-heavy workloads (cache-friendly iteration)
- Small to medium sizes
- When you don't need iterator stability

### `std::generator` — standard library generator coroutine

No more writing your own Generator template:

```cpp
#include <generator>

std::generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

for (int x : fibonacci() | std::views::take(10)) {
    std::cout << x << " ";  // 0 1 1 2 3 5 8 13 21 34
}
```

### Zip views

Iterate multiple ranges in parallel:

```cpp
std::vector nums{1, 2, 3};
std::vector names{"one", "two", "three"};

for (auto [num, name] : std::views::zip(nums, names)) {
    std::println("{}: {}", num, name);
}
// 1: one
// 2: two
// 3: three

// Also: zip_transform, adjacent, adjacent_transform
auto sums = std::views::zip_transform(std::plus{}, vec1, vec2);
```

### `std::mdspan` — multidimensional view

Non-owning multidimensional view over contiguous data:

```cpp
#include <mdspan>

std::vector<double> data(12);  // flat storage
std::mdspan matrix(data.data(), 3, 4);  // view as 3×4 matrix

matrix[1, 2] = 3.14;  // uses multidimensional subscript!
// No copy — just a view over the flat vector
```

---

## 7. Exercises

See `exercises.cpp`:

1. Use deducing `this` to write a CRTP-free mixin
2. Implement error handling with `std::expected` (replace exceptions)
3. Use `std::print`/`println` for formatted output
4. Implement a Matrix class with multidimensional `operator[]`
5. Use `ranges::to` to materialize pipelines
6. Build a recursive lambda using deducing `this` for tree traversal

---

## Congratulations!

You've completed all 13 lectures covering C++11 through C++23. You now have
a comprehensive understanding of modern C++. The exercises throughout this
series will solidify your mastery. Keep building, keep reading proposals,
and contribute to the C++ community!
