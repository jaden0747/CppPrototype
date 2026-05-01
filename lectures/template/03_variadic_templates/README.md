# Template 03 — Variadic Templates

> **Goal:** Work with templates that accept **any number of arguments**.
> Master parameter packs, pack expansion, fold expressions, and recursive
> template patterns. Understand when and why to use each technique.

---

## Table of Contents

1. [What Are Variadic Templates?](#1-what-are-variadic-templates)
2. [Parameter Packs](#2-parameter-packs)
3. [Pack Expansion](#3-pack-expansion)
4. [Recursive Unpacking (Pre-C++17)](#4-recursive-unpacking)
5. [Fold Expressions (C++17)](#5-fold-expressions-c17)
6. [`sizeof...` Operator](#6-sizeof-operator)
7. [Real-World Use Cases](#7-real-world-use-cases)
8. [Common Patterns](#8-common-patterns)
9. [Exercises](#9-exercises)

---

## 1. What Are Variadic Templates?

### The problem

How do you write a function that accepts **any number of arguments of any type**?

```cpp
// Works for 2 args, but what about 3? 5? 10?
template<typename T1, typename T2>
void print(T1 a, T2 b);
```

### The solution

Variadic templates use `...` to accept zero or more template parameters:

```cpp
template<typename... Ts>      // Ts is a "parameter pack" of types
void print(Ts... args) {      // args is a "parameter pack" of values
    ((std::cout << args << " "), ...);  // expand and print each
}

print(1, 2.0, "hello", 'x');  // works with any number/types!
```

### Where are variadic templates used?

They power some of the most important C++ features:
- `std::tuple<int, double, string>` — holds any number of types
- `std::make_unique<T>(args...)` — forwards any constructor arguments
- `std::variant<int, double, string>` — type-safe union
- `std::format("{} {} {}", a, b, c)` — format strings
- `std::function<void(int, double)>` — callable with any signature

---

## 2. Parameter Packs

### Type parameter pack

```cpp
template<typename... Ts>  // Ts is the TYPE parameter pack
struct Tuple {};

Tuple<int, double, std::string> t;  // Ts = {int, double, std::string}
Tuple<> empty;                       // Ts = {} (empty pack)
```

### Function parameter pack

```cpp
template<typename... Ts>
void f(Ts... args) {}  // args is the FUNCTION parameter pack

f(1, 2.0, "hi");  // args = {1, 2.0, "hi"}
f();               // args = {} (no arguments)
```

### Non-type parameter pack

```cpp
template<int... Ns>
struct IntSequence {};

IntSequence<1, 2, 3, 4, 5> seq;  // Ns = {1, 2, 3, 4, 5}
```

### Critical rule: packs must always be expanded

You **cannot** use a pack directly — you must expand it with `...`:

```cpp
template<typename... Ts>
void f(Ts... args) {
    // args;      // ❌ Error: can't use pack without expansion
    // args...;   // ✅ Expands to: arg1, arg2, arg3, ...
}
```

---

## 3. Pack Expansion

### What is pack expansion?

A **pattern** followed by `...` expands the pack by applying the pattern
to each element:

```cpp
template<typename... Ts>
void f(Ts... args) {
    g(args...);           // expands to: g(arg1, arg2, arg3)
    g(process(args)...);  // expands to: g(process(arg1), process(arg2), ...)
}
```

### Expansion contexts

Packs can be expanded in many places:

```cpp
// 1. Function call arguments
f(args...);                  // f(a1, a2, a3)

// 2. Template arguments
std::tuple<Ts...>            // tuple<T1, T2, T3>

// 3. Initializer lists
int dummy[] = { (f(args), 0)... };  // calls f for each arg

// 4. Base classes
template<typename... Bases>
struct Derived : Bases... {          // inherits from all
    using Bases::operator()...;       // using declarations (C++17)
};

// 5. Lambda captures
auto lambda = [args...] { };         // capture each arg by copy
auto lambda2 = [&args...] { };      // capture each arg by reference
```

### Pattern expansion — the key insight

The `...` applies to the **entire pattern before it**:

```cpp
template<typename... Ts>
auto make_pointers(Ts... args) {
    return std::make_tuple(&args...);
    // Pattern: &args
    // Expansion: &arg1, &arg2, &arg3
}

template<typename... Ts>
auto double_all(Ts... args) {
    return std::make_tuple((args * 2)...);
    // Pattern: (args * 2)
    // Expansion: (arg1*2), (arg2*2), (arg3*2)
}
```

---

## 4. Recursive Unpacking (Pre-C++17)

Before fold expressions, the standard technique was **recursive peeling**:
process the first element, then recurse on the rest.

### The pattern

```cpp
// Base case: no more arguments
void print() {
    std::cout << "\n";
}

// Recursive case: peel off first, recurse on rest
template<typename T, typename... Rest>
void print(T first, Rest... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0)
        std::cout << ", ";
    print(rest...);  // recurse with remaining args
}

print(1, 2.0, "three");
// Call 1: first=1,     rest={2.0, "three"} → print "1, "
// Call 2: first=2.0,   rest={"three"}      → print "2.0, "
// Call 3: first="three", rest={}           → print "three"
// Call 4: base case                         → print "\n"
```

### How it works

```
print(1, 2.0, "three")
  → T=int, first=1,     Rest={double, const char*}, rest={2.0, "three"}
  → print(2.0, "three")
    → T=double, first=2.0, Rest={const char*}, rest={"three"}
    → print("three")
      → T=const char*, first="three", Rest={}, rest={}
      → print()    ← base case
```

### When to use recursion vs fold expressions

| Technique | Use when |
|-----------|----------|
| Fold expressions | Simple operations: sum, print, logical AND/OR |
| Recursive | Complex logic per element, need index tracking, C++14 |

---

## 5. Fold Expressions (C++17)

### What are they?

Fold expressions let you apply a binary operator across all elements of a
parameter pack **without recursion**. They're the modern, clean way to
process packs.

### The four forms

| Form | Name | Expansion (for a, b, c) |
|------|------|------------------------|
| `(args op ...)` | Right fold | `a op (b op c)` |
| `(... op args)` | Left fold | `(a op b) op c` |
| `(args op ... op init)` | Right fold + init | `a op (b op (c op init))` |
| `(init op ... op args)` | Left fold + init | `((init op a) op b) op c` |

### Examples

```cpp
// Sum all arguments
template<typename... Ts>
auto sum(Ts... args) {
    return (args + ...);  // right fold: a + (b + (c + d))
}
sum(1, 2, 3, 4);  // 10

// Logical AND: are all true?
template<typename... Ts>
bool all(Ts... args) {
    return (args && ...);
}
all(true, true, false);  // false

// Print all with spaces
template<typename... Ts>
void print_all(Ts... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}
print_all(1, 2.0, "hello");  // "1 2 hello\n"

// Sum with initial value
template<typename... Ts>
auto sum_from(int init, Ts... args) {
    return (init + ... + args);  // left fold with init
}
sum_from(100, 1, 2, 3);  // 106
```

### Empty pack behavior

| Operator | Empty pack result |
|----------|------------------|
| `&&` | `true` |
| `\|\|` | `false` |
| `,` | `void()` |
| Others | Compile error! Use init form. |

```cpp
sum();              // ❌ Error! empty + fold
sum_from(0);        // ✅ OK: init value handles empty pack
(args && ...);      // ✅ OK: empty && fold returns true
```

---

## 6. `sizeof...` Operator

Returns the number of elements in a parameter pack at compile time:

```cpp
template<typename... Ts>
constexpr size_t count() {
    return sizeof...(Ts);
}

count<int, double, char>();  // 3

template<typename... Ts>
void f(Ts... args) {
    constexpr auto n = sizeof...(args);  // same thing, for function pack
    static_assert(sizeof...(args) > 0, "Need at least one argument");
}
```

### Use case: different behavior based on pack size

```cpp
template<typename... Ts>
void process(Ts... args) {
    if constexpr (sizeof...(args) == 0)
        std::cout << "no args\n";
    else if constexpr (sizeof...(args) == 1)
        std::cout << "one arg\n";
    else
        std::cout << sizeof...(args) << " args\n";
}
```

---

## 7. Real-World Use Cases

### 1. Perfect forwarding factory function

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
// Forwards any number of constructor arguments with perfect forwarding
```

### 2. Tuple implementation (simplified)

```cpp
template<typename... Ts>
struct Tuple;

template<>
struct Tuple<> {};  // base case: empty tuple

template<typename Head, typename... Tail>
struct Tuple<Head, Tail...> : Tuple<Tail...> {
    Head value;
    Tuple(Head h, Tail... t) : Tuple<Tail...>(t...), value(h) {}
};
```

### 3. Type-safe printf

```cpp
template<typename... Args>
void my_printf(const char* fmt, Args... args) {
    // Use fold expression to process format string and args together
    // (simplified — real implementation uses recursive unpacking)
}
```

---

## 8. Common Patterns

### The "overloaded" pattern for variant visiting

```cpp
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;  // pack expansion in using declaration
};

// C++17 deduction guide
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// Usage:
std::visit(overloaded{
    [](int i)    { std::cout << "int: " << i; },
    [](double d) { std::cout << "double: " << d; },
    [](auto x)   { std::cout << "other"; },
}, my_variant);
```

### Index sequence trick

```cpp
template<typename Tuple, size_t... Is>
void print_tuple_impl(const Tuple& t, std::index_sequence<Is...>) {
    ((std::cout << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
}

template<typename... Ts>
void print_tuple(const std::tuple<Ts...>& t) {
    print_tuple_impl(t, std::index_sequence_for<Ts...>{});
}
```

---

## 9. Exercises

See `exercises.cpp`.
```cpp
template<typename... Ts>
auto make_pointers(Ts... args) {
    return std::make_tuple(&args...);
    // Pattern: &args
    // Expansion: &arg1, &arg2, &arg3
}

template<typename... Ts>
auto double_all(Ts... args) {
    return std::make_tuple((args * 2)...);
    // Pattern: (args * 2)
    // Expansion: (arg1*2), (arg2*2), (arg3*2)
}
```

---

## 4. Recursive Unpacking (Pre-C++17)

Before fold expressions, the standard technique was **recursive peeling**:
process the first element, then recurse on the rest.

### The pattern

```cpp
// Base case: no more arguments
void print() {
    std::cout << "\n";
}

// Recursive case: peel off first, recurse on rest
template<typename T, typename... Rest>
void print(T first, Rest... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0)
        std::cout << ", ";
    print(rest...);  // recurse with remaining args
}

print(1, 2.0, "three");
// Call 1: first=1,     rest={2.0, "three"} → print "1, "
// Call 2: first=2.0,   rest={"three"}      → print "2.0, "
// Call 3: first="three", rest={}           → print "three"
// Call 4: base case                         → print "\n"
```

### How it works

```
print(1, 2.0, "three")
  → T=int, first=1,     Rest={double, const char*}, rest={2.0, "three"}
  → print(2.0, "three")
    → T=double, first=2.0, Rest={const char*}, rest={"three"}
    → print("three")
      → T=const char*, first="three", Rest={}, rest={}
      → print()    ← base case
```

### When to use recursion vs fold expressions

| Technique | Use when |
|-----------|----------|
| Fold expressions | Simple operations: sum, print, logical AND/OR |
| Recursive | Complex logic per element, need index tracking, C++14 |

---

## 5. Fold Expressions (C++17)

### What are they?

Fold expressions let you apply a binary operator across all elements of a
parameter pack **without recursion**. They're the modern, clean way to
process packs.

### The four forms

| Form | Name | Expansion (for a, b, c) |
|------|------|------------------------|
| `(args op ...)` | Right fold | `a op (b op c)` |
| `(... op args)` | Left fold | `(a op b) op c` |
| `(args op ... op init)` | Right fold + init | `a op (b op (c op init))` |
| `(init op ... op args)` | Left fold + init | `((init op a) op b) op c` |

### Examples

```cpp
// Sum all arguments
template<typename... Ts>
auto sum(Ts... args) {
    return (args + ...);  // right fold: a + (b + (c + d))
}
sum(1, 2, 3, 4);  // 10

// Logical AND: are all true?
template<typename... Ts>
bool all(Ts... args) {
    return (args && ...);
}
all(true, true, false);  // false

// Print all with spaces
template<typename... Ts>
void print_all(Ts... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}
print_all(1, 2.0, "hello");  // "1 2 hello\n"

// Sum with initial value
template<typename... Ts>
auto sum_from(int init, Ts... args) {
    return (init + ... + args);  // left fold with init
}
sum_from(100, 1, 2, 3);  // 106
```

### Empty pack behavior

| Operator | Empty pack result |
|----------|------------------|
| `&&` | `true` |
| `\|\|` | `false` |
| `,` | `void()` |
| Others | Compile error! Use init form. |

```cpp
sum();              // ❌ Error! empty + fold
sum_from(0);        // ✅ OK: init value handles empty pack
(args && ...);      // ✅ OK: empty && fold returns true
```

---

## 6. `sizeof...` Operator

Returns the number of elements in a parameter pack at compile time:

```cpp
template<typename... Ts>
constexpr size_t count() {
    return sizeof...(Ts);
}

count<int, double, char>();  // 3

template<typename... Ts>
void f(Ts... args) {
    constexpr auto n = sizeof...(args);  // same thing, for function pack
    static_assert(sizeof...(args) > 0, "Need at least one argument");
}
```

### Use case: different behavior based on pack size

```cpp
template<typename... Ts>
void process(Ts... args) {
    if constexpr (sizeof...(args) == 0)
        std::cout << "no args\n";
    else if constexpr (sizeof...(args) == 1)
        std::cout << "one arg\n";
    else
        std::cout << sizeof...(args) << " args\n";
}
```

---

## 7. Real-World Use Cases

### 1. Perfect forwarding factory function

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
// Forwards any number of constructor arguments with perfect forwarding
```

### 2. Tuple implementation (simplified)

```cpp
template<typename... Ts>
struct Tuple;

template<>
struct Tuple<> {};  // base case: empty tuple

template<typename Head, typename... Tail>
struct Tuple<Head, Tail...> : Tuple<Tail...> {
    Head value;
    Tuple(Head h, Tail... t) : Tuple<Tail...>(t...), value(h) {}
};
```

### 3. Type-safe printf

```cpp
template<typename... Args>
void my_printf(const char* fmt, Args... args) {
    // Use fold expression to process format string and args together
    // (simplified — real implementation uses recursive unpacking)
}
```

---

## 8. Common Patterns

### The "overloaded" pattern for variant visiting

```cpp
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;  // pack expansion in using declaration
};

// C++17 deduction guide
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// Usage:
std::visit(overloaded{
    [](int i)    { std::cout << "int: " << i; },
    [](double d) { std::cout << "double: " << d; },
    [](auto x)   { std::cout << "other"; },
}, my_variant);
```

### Index sequence trick

```cpp
template<typename Tuple, size_t... Is>
void print_tuple_impl(const Tuple& t, std::index_sequence<Is...>) {
    ((std::cout << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
}

template<typename... Ts>
void print_tuple(const std::tuple<Ts...>& t) {
    print_tuple_impl(t, std::index_sequence_for<Ts...>{});
}
```

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** SFINAE & Type Traits.
