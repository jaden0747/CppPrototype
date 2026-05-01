# Template 07 — Standard Concepts & Constrained Templates

> **Goal:** Use `<concepts>` standard library concepts, understand concept
> subsumption, abbreviated function templates, constrain class templates,
> and migrate from SFINAE to concepts.

---

## Table of Contents

1. [Standard Library Concepts](#1-standard-library-concepts)
2. [Concept Subsumption](#2-concept-subsumption)
3. [Abbreviated Function Templates](#3-abbreviated-function-templates)
4. [Constraining Class Templates](#4-constraining-class-templates)
5. [SFINAE → Concepts Migration](#5-sfinae--concepts-migration)
6. [Real-World Patterns](#6-real-world-patterns)
7. [Exercises](#7-exercises)

---

## 1. Standard Library Concepts

### Why use standard concepts?

Instead of writing your own concepts for common requirements, the standard
library provides well-tested, well-documented concepts that everyone
understands.

### `<concepts>` — Core language concepts

| Category | Concept | What it checks |
|----------|---------|---------------|
| **Same/Derived** | `same_as<T, U>` | `T` and `U` are the exact same type |
| | `derived_from<D, B>` | `D` publicly derives from `B` |
| | `convertible_to<T, U>` | `T` is implicitly/explicitly convertible to `U` |
| **Arithmetic** | `integral<T>` | `T` is an integer type (int, long, char, bool, etc.) |
| | `signed_integral<T>` | `T` is a signed integer (int, long, etc.) |
| | `unsigned_integral<T>` | `T` is unsigned (unsigned int, size_t, etc.) |
| | `floating_point<T>` | `T` is float, double, or long double |
| **Comparison** | `equality_comparable<T>` | `T` supports `==` and `!=` |
| | `totally_ordered<T>` | `T` supports `<`, `>`, `<=`, `>=`, `==`, `!=` |
| **Object** | `movable<T>` | `T` is move-constructible and move-assignable |
| | `copyable<T>` | `T` is copyable + movable |
| | `semiregular<T>` | `T` is copyable + default-constructible |
| | `regular<T>` | `T` is semiregular + equality-comparable |
| **Callable** | `invocable<F, Args...>` | `F` can be called with `Args...` |
| | `predicate<F, Args...>` | `F` returns bool-like when called with `Args...` |
| | `relation<F, T, U>` | `F` is a binary relation on `T` and `U` |

### `<iterator>` concepts

| Concept | What it means |
|---------|--------------|
| `input_iterator` | Can read elements, single-pass (like reading a stream) |
| `forward_iterator` | Can read elements, multi-pass (like forward_list) |
| `bidirectional_iterator` | Can go forward AND backward (like list, set) |
| `random_access_iterator` | Can jump anywhere in O(1) (like vector, deque) |
| `contiguous_iterator` | Elements are in contiguous memory (like vector, array) |

### `<ranges>` concepts

| Concept | What it means |
|---------|--------------|
| `range` | Has `begin()` and `end()` |
| `sized_range` | Has `size()` in O(1) |
| `input_range` | Range with input iterators |
| `forward_range` | Range with forward iterators |
| `random_access_range` | Range with random access iterators |
| `contiguous_range` | Range with contiguous memory |

### Usage examples

```cpp
void f(std::integral auto x) { }               // any integer
void g(std::totally_ordered auto x) { }         // any ordered type
void h(std::invocable<int> auto func) { }       // callable with int
void i(std::ranges::input_range auto&& r) { }   // any input range
```

---

## 2. Concept Subsumption

### What is subsumption?

When multiple constrained overloads match a call, the compiler picks the
**most constrained** one. This works because some concepts **imply** others.

```cpp
template<typename T>
void process(T val) { std::cout << "unconstrained\n"; }

template<std::integral T>
void process(T val) { std::cout << "integral\n"; }

template<std::signed_integral T>
void process(T val) { std::cout << "signed integral\n"; }

process(42);     // "signed integral" — most constrained match
process(42u);    // "integral" — unsigned, so signed_integral doesn't match
process(3.14);   // "unconstrained" — not integral
```

### The concept hierarchy

```
                    movable
                       ↑
                    copyable
                       ↑
                   semiregular
                       ↑
                    regular

    integral ← signed_integral
             ← unsigned_integral
```

`signed_integral` subsumes `integral` because its definition includes
`integral`:

```cpp
template<typename T>
concept signed_integral = integral<T> && std::is_signed_v<T>;
// signed_integral IMPLIES integral, so it's "more constrained"
```

### Rules for subsumption

1. Subsumption only works with **named concepts** (not raw `requires`)
2. The compiler normalizes constraints into conjunctions/disjunctions
3. C1 subsumes C2 if C1's constraint **logically implies** C2

```cpp
// ✅ Subsumption works — uses named concepts
template<std::integral T>       void f(T) {}
template<std::signed_integral T> void f(T) {}
f(42); // OK: picks signed_integral

// ❌ Subsumption does NOT work — uses raw expressions
template<typename T> requires std::is_integral_v<T> void g(T) {}
template<typename T> requires (std::is_integral_v<T> && std::is_signed_v<T>) void g(T) {}
g(42); // AMBIGUOUS! Raw expressions don't participate in subsumption
```

---

## 3. Abbreviated Function Templates

### The terse syntax

```cpp
// auto → unconstrained template
void print(auto val) { std::cout << val; }
// Equivalent to: template<typename T> void print(T val);

// Concept auto → constrained template
void print_num(std::integral auto val) { std::cout << val; }
// Equivalent to: template<std::integral T> void print_num(T val);
```

### Each `auto` is an independent template parameter

```cpp
auto add(std::integral auto a, std::floating_point auto b) {
    return a + b;
}
// a and b are DIFFERENT types:
// Equivalent to: template<std::integral T, std::floating_point U> auto add(T a, U b);

add(3, 2.5);  // T=int, U=double
```

### When to use abbreviated syntax

| Syntax | Best for |
|--------|----------|
| `auto` / `Concept auto` | Simple functions, algorithm callbacks |
| `template<Concept T>` | When you need to refer to T multiple times |
| `template<typename T> requires ...` | Complex constraints |

```cpp
// ✅ Abbreviated: simple, no need to name the type
void print(std::integral auto x) { std::cout << x; }

// ✅ Full template: need to use T in the body
template<std::integral T>
std::vector<T> make_copies(T val, int n) {
    return std::vector<T>(n, val);
}
```

---

## 4. Constraining Class Templates

### Constrained class template

```cpp
template<std::regular T>
class Optional {
    // T must be regular (copyable, default-constructible, equality-comparable)
    bool has_value_ = false;
    T value_{};
public:
    Optional() = default;
    Optional(T val) : has_value_(true), value_(std::move(val)) {}
    // ...
};

Optional<int> ok;              // ✅ int is regular
// Optional<std::mutex> bad;   // ❌ mutex is not copyable
```

### Constrained member functions

You can constrain **individual member functions** — they only exist when
the constraint is satisfied:

```cpp
template<typename T>
class Container {
    std::vector<T> data_;
public:
    // Always available
    void push_back(const T& val) { data_.push_back(val); }
    size_t size() const { return data_.size(); }

    // Only available if T is totally_ordered
    void sort() requires std::totally_ordered<T> {
        std::ranges::sort(data_);
    }

    // Only available if T is printable
    void print() requires requires(T t) { std::cout << t; } {
        for (const auto& x : data_) std::cout << x << " ";
        std::cout << "\n";
    }
};

Container<int> c;
c.push_back(42);
c.sort();    // ✅ int is totally_ordered
c.print();   // ✅ int is printable

Container<std::mutex> m;
m.push_back(std::mutex{});
// m.sort();  // ❌ mutex is not totally_ordered — this member doesn't exist
```

### Partial specialization with concepts

```cpp
template<typename T>
class Formatter {
    // General case: convert to string
    std::string format(const T& val) { return std::to_string(val); }
};

template<std::integral T>
class Formatter<T> {
    // Specialized for integers: hex formatting
    std::string format(T val) {
        std::ostringstream oss;
        oss << "0x" << std::hex << val;
        return oss.str();
    }
};
```

---

## 5. SFINAE → Concepts Migration

### Side-by-side comparison

| SFINAE (C++11/14/17) | Concepts (C++20) |
|----------------------|------------------|
| `enable_if_t<is_integral_v<T>>` | `std::integral T` |
| `enable_if_t<is_convertible_v<T,U>>` | `std::convertible_to<T, U>` |
| `void_t<decltype(expr)>` | `requires { expr; }` |
| `void_t<decltype(a < b)>` | `{ a < b } -> std::convertible_to<bool>` |
| Tag dispatch with `iterator_category` | Constrained overloads with iterator concepts |
| `is_same_v<T, U>` | `std::same_as<T, U>` |
| `is_base_of_v<Base, T>` | `std::derived_from<T, Base>` |

### Migration example: constrained algorithm

```cpp
// Before (SFINAE)
template<typename Iter, typename = std::enable_if_t<
    std::is_same_v<typename std::iterator_traits<Iter>::iterator_category,
                   std::random_access_iterator_tag>>>
void fast_sort(Iter first, Iter last) {
    std::sort(first, last);
}

// After (Concepts)
void fast_sort(std::random_access_iterator auto first,
               std::random_access_iterator auto last) {
    std::sort(first, last);
}
```

### Migration example: conditional member

```cpp
// Before
template<typename T>
class Wrapper {
    T value_;
public:
    template<typename U = T, std::enable_if_t<std::is_integral_v<U>, int> = 0>
    U doubled() const { return value_ * 2; }
};

// After
template<typename T>
class Wrapper {
    T value_;
public:
    T doubled() const requires std::integral<T> { return value_ * 2; }
};
```

---

## 6. Real-World Patterns

### Factory with concept constraints

```cpp
template<typename T, typename... Args>
    requires std::constructible_from<T, Args...>
std::unique_ptr<T> make(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
```

### Range algorithm with concepts

```cpp
auto sum(std::ranges::input_range auto&& r) {
    using T = std::ranges::range_value_t<decltype(r)>;
    T total{};
    for (const auto& x : r) total += x;
    return total;
}
```

---

## 7. Exercises

See `exercises.cpp`.

---

**Next lecture:** CRTP & Static Polymorphism.
