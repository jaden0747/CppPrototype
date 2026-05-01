# Lecture 06 — C++17 Ergonomics

> **Goal:** Master the daily-use C++17 features that make code cleaner and
> eliminate boilerplate: structured bindings, if/switch with initializer,
> class template argument deduction (CTAD), fold expressions, and more.
> Understand **why** each feature exists and **when** to use it.

---

## Table of Contents

1. [Structured Bindings](#1-structured-bindings)
2. [If/Switch with Initializer](#2-ifswitch-with-initializer)
3. [Class Template Argument Deduction (CTAD)](#3-class-template-argument-deduction-ctad)
4. [Fold Expressions](#4-fold-expressions)
5. [`inline` Variables](#5-inline-variables)
6. [`constexpr if`](#6-constexpr-if)
7. [Nested Namespaces](#7-nested-namespaces)
8. [`std::byte`](#8-stdbyte)
9. [Exercises](#9-exercises)

---

## 1. Structured Bindings

### What is it?

Decompose an aggregate, tuple, or pair into named variables in one line:

```cpp
std::map<std::string, int> scores{{"Alice", 95}, {"Bob", 87}};
for (const auto& [name, score] : scores) {
    std::cout << name << ": " << score << "\n";
}
```

### Why does this matter?

Before C++17, working with pairs and tuples was verbose:

```cpp
// C++11: verbose and unreadable
for (const auto& entry : scores) {
    std::cout << entry.first << ": " << entry.second << "\n";
}

// Even worse with insert results:
auto result = map.insert({"key", 42});
if (result.second) {
    use(result.first->second);  // what is first? what is second?
}

// C++17: clear intent
auto [iter, inserted] = map.insert({"key", 42});
if (inserted) {
    use(iter->second);
}
```

### What can be decomposed?

| Source type | Example |
|-------------|---------|
| `std::pair` | `auto [key, val] = std::make_pair(1, "hello")` |
| `std::tuple` | `auto [x, y, z] = std::make_tuple(1, 2.0, "hi")` |
| Arrays | `int arr[3] = {1,2,3}; auto [a,b,c] = arr;` |
| Structs (all public) | `struct P {int x,y;}; auto [px,py] = P{3,4};` |

### Binding modes

```cpp
auto [a, b] = expr;        // copies — a and b are independent copies
auto& [a, b] = expr;       // references — a and b refer to the original
const auto& [a, b] = expr; // const references
auto&& [a, b] = expr;      // forwarding references
```

### Rules and limitations

- You must bind **all** members (can't skip one)
- Works with public non-static data members only
- Custom types need either public members or `std::tuple_size`/`get`
- Cannot use `[[maybe_unused]]` on individual bindings (C++26 may fix this)

### Structured bindings with `if` init (combining C++17 features)

```cpp
if (auto [iter, success] = map.try_emplace(key, value); success) {
    std::cout << "Inserted: " << iter->second << "\n";
}
```

---

## 2. If/Switch with Initializer

### What is it?

Declare a variable scoped to the `if` or `switch` statement:

```cpp
if (init-statement; condition) { ... }
switch (init-statement; value) { ... }
```

### The problem: leaking scope

```cpp
// Before: 'it' leaks into outer scope
auto it = map.find(key);
if (it != map.end()) {
    use(it->second);
}
// 'it' still visible here — potential misuse!
```

### The solution

```cpp
// After: 'it' scoped to the if
if (auto it = map.find(key); it != map.end()) {
    use(it->second);
}
// 'it' no longer exists here
```

### Common use cases

```cpp
// 1. Map lookups
if (auto it = cache.find(key); it != cache.end())
    return it->second;

// 2. Lock acquisition
if (std::lock_guard lk(mtx); !queue.empty()) {
    auto item = queue.front();
    queue.pop();
}

// 3. Dynamic cast
if (auto* derived = dynamic_cast<Derived*>(base); derived) {
    derived->special_method();
}

// 4. Error codes
if (auto ec = connect(host, port); ec) {
    log_error(ec);
}

// 5. Switch with init
switch (auto ch = read_char(); ch) {
    case 'q': quit(); break;
    case 's': save(); break;
    default: process(ch);
}
```

---

## 3. Class Template Argument Deduction (CTAD)

### What is it?

The compiler deduces template parameters from constructor arguments:

```cpp
// Before C++17: must write full type
std::pair<int, const char*> p{42, "hello"};
std::vector<int> v{1, 2, 3};
std::lock_guard<std::mutex> lock{mtx};

// C++17: compiler deduces the types
std::pair p{42, "hello"};           // pair<int, const char*>
std::vector v{1, 2, 3, 4, 5};      // vector<int>
std::lock_guard lock{mtx};          // lock_guard<mutex>
std::tuple t{1, 2.0, "hello"s};    // tuple<int, double, string>
```

### Why does CTAD exist?

Redundancy. When you write `std::pair<int, double>(42, 3.14)`, the compiler
already knows the types from the constructor arguments. Why repeat yourself?

### Where CTAD works automatically

Any class template where the constructor parameters reveal the template types:

```cpp
std::optional opt{42};           // optional<int>
std::array arr{1, 2, 3, 4, 5};  // array<int, 5>
std::unique_lock lock{mtx};     // unique_lock<mutex>
```

### Deduction guides for your own classes

When the constructor doesn't directly reveal the template type, write a
**deduction guide**:

```cpp
template<typename T>
struct Wrapper {
    T value;
    Wrapper(T v) : value(v) {}
};

// Without deduction guide: Wrapper w{42} won't compile
// With deduction guide:
template<typename T>
Wrapper(T) -> Wrapper<T>;

Wrapper w{42};  // Wrapper<int>
```

### CTAD pitfalls

```cpp
std::vector v1{1, 2, 3};    // ✅ vector<int>
std::vector v2{1.0, 2, 3};  // ❌ Error: mixed types

// Be careful with strings:
std::vector v3{"hello", "world"};  // vector<const char*>, NOT vector<string>!
std::vector v4{"hello"s, "world"s}; // vector<string> — use string literals
```

---

## 4. Fold Expressions

### What is it?

Reduce a parameter pack with a single operator — no recursion needed:

```cpp
template<typename... Args>
auto sum(Args... args) { return (args + ...); }

sum(1, 2, 3, 4);  // 10
```

### The four forms

| Form | Name | Expansion (for a, b, c) |
|------|------|-------------------------|
| `(pack op ...)` | Right fold | `a op (b op c)` |
| `(... op pack)` | Left fold | `(a op b) op c` |
| `(pack op ... op init)` | Right fold + init | `a op (b op (c op init))` |
| `(init op ... op pack)` | Left fold + init | `((init op a) op b) op c` |

### Practical examples

```cpp
// Print all with spaces
template<typename... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}
print(1, "hello", 3.14);  // "1 hello 3.14\n"

// All true?
template<typename... Args>
bool all_true(Args... args) { return (args && ...); }

// Push back multiple items
template<typename Container, typename... Args>
void push_many(Container& c, Args&&... args) {
    (c.push_back(std::forward<Args>(args)), ...);
}
std::vector<int> v;
push_many(v, 1, 2, 3, 4, 5);  // v = {1, 2, 3, 4, 5}
```

### Empty pack behavior

| Operator | Empty pack value |
|----------|-----------------|
| `&&` | `true` |
| `\|\|` | `false` |
| `,` | `void()` |
| Others | Compile error! Use fold-with-init. |

---

## 5. `inline` Variables

### The problem: ODR violations with header-defined globals

Before C++17, defining a variable in a header included by multiple translation
units violated the One Definition Rule (ODR):

```cpp
// header.h — included in file1.cpp AND file2.cpp
int counter = 0;  // ❌ ODR violation: multiple definitions!
```

### The solution

```cpp
// header.h
inline int counter = 0;          // ✅ OK even if included in multiple TUs
inline const std::string VERSION = "1.0";
```

### Why `inline` for variables?

Same meaning as `inline` for functions: "this may appear in multiple
translation units, and they're all the same definition." The linker picks one.

### Use cases

```cpp
// 1. Header-only constants
inline constexpr int MAX_SIZE = 1024;

// 2. Static class members (no separate .cpp definition needed)
struct Config {
    static inline int max_threads = 8;    // ✅ C++17: defined right here
    // Before C++17: declared here, defined in Config.cpp
};
```

---

## 6. `constexpr if`

### What is it?

Compile-time conditional — the false branch is **completely discarded**:

```cpp
template<typename T>
auto stringify(T value) {
    if constexpr (std::is_arithmetic_v<T>)
        return std::to_string(value);
    else
        return std::string(value);
}

stringify(42);       // calls to_string(42)
stringify("hello");  // calls string("hello")
// The other branch doesn't even need to compile!
```

### Why not regular `if`?

```cpp
template<typename T>
auto bad_stringify(T value) {
    if (std::is_arithmetic_v<T>)
        return std::to_string(value);  // ❌ Error when T=const char*
    else
        return std::string(value);     // ❌ Error when T=int
}
// Regular if: BOTH branches are compiled for every T
// constexpr if: only the matching branch is compiled
```

### Eliminates SFINAE for many cases

```cpp
// Before: SFINAE (complex)
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void process(T val) { std::cout << "integer: " << val; }

template<typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
void process(T val) { std::cout << "other: " << val; }

// After: constexpr if (simple!)
template<typename T>
void process(T val) {
    if constexpr (std::is_integral_v<T>)
        std::cout << "integer: " << val;
    else
        std::cout << "other: " << val;
}
```

---

## 7. Nested Namespaces

```cpp
// Before C++17:
namespace my { namespace lib { namespace detail { /* ... */ } } }

// C++17:
namespace my::lib::detail { /* ... */ }
```

Small quality-of-life improvement that reduces indentation.

---

## 8. `std::byte`

A type for raw byte data that is distinct from `char` and `unsigned char`:

```cpp
#include <cstddef>

std::byte b{0xFF};
std::byte c = b & std::byte{0x0F};  // bitwise AND
// std::byte is NOT an integer — can't do arithmetic
// b + c;  // ❌ Error!
auto val = std::to_integer<int>(b);  // convert when needed
```

This prevents accidentally using byte buffers as characters or numbers.

---

## 9. Exercises

See `exercises.cpp`:

1. Use structured bindings with maps, tuples, and custom structs
2. Refactor code to use if-with-initializer
3. Write code using CTAD (no explicit template args)
4. Implement variadic functions using fold expressions
5. Use `constexpr if` to write a single function handling multiple types
6. Create a header-only library using `inline` variables

---
```cpp
std::vector v1{1, 2, 3};    // ✅ vector<int>
std::vector v2{1.0, 2, 3};  // ❌ Error: mixed types

// Be careful with strings:
std::vector v3{"hello", "world"};  // vector<const char*>, NOT vector<string>!
std::vector v4{"hello"s, "world"s}; // vector<string> — use string literals
```

---

## 4. Fold Expressions

### What is it?

Reduce a parameter pack with a single operator — no recursion needed:

```cpp
template<typename... Args>
auto sum(Args... args) { return (args + ...); }

sum(1, 2, 3, 4);  // 10
```

### The four forms

| Form | Name | Expansion (for a, b, c) |
|------|------|-------------------------|
| `(pack op ...)` | Right fold | `a op (b op c)` |
| `(... op pack)` | Left fold | `(a op b) op c` |
| `(pack op ... op init)` | Right fold + init | `a op (b op (c op init))` |
| `(init op ... op pack)` | Left fold + init | `((init op a) op b) op c` |

### Practical examples

```cpp
// Print all with spaces
template<typename... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}
print(1, "hello", 3.14);  // "1 hello 3.14\n"

// All true?
template<typename... Args>
bool all_true(Args... args) { return (args && ...); }

// Push back multiple items
template<typename Container, typename... Args>
void push_many(Container& c, Args&&... args) {
    (c.push_back(std::forward<Args>(args)), ...);
}
std::vector<int> v;
push_many(v, 1, 2, 3, 4, 5);  // v = {1, 2, 3, 4, 5}
```

### Empty pack behavior

| Operator | Empty pack value |
|----------|-----------------|
| `&&` | `true` |
| `\|\|` | `false` |
| `,` | `void()` |
| Others | Compile error! Use fold-with-init. |

---

## 5. `inline` Variables

### The problem: ODR violations with header-defined globals

Before C++17, defining a variable in a header included by multiple translation
units violated the One Definition Rule (ODR):

```cpp
// header.h — included in file1.cpp AND file2.cpp
int counter = 0;  // ❌ ODR violation: multiple definitions!
```

### The solution

```cpp
// header.h
inline int counter = 0;          // ✅ OK even if included in multiple TUs
inline const std::string VERSION = "1.0";
```

### Why `inline` for variables?

Same meaning as `inline` for functions: "this may appear in multiple
translation units, and they're all the same definition." The linker picks one.

### Use cases

```cpp
// 1. Header-only constants
inline constexpr int MAX_SIZE = 1024;

// 2. Static class members (no separate .cpp definition needed)
struct Config {
    static inline int max_threads = 8;    // ✅ C++17: defined right here
    // Before C++17: declared here, defined in Config.cpp
};
```

---

## 6. `constexpr if`

### What is it?

Compile-time conditional — the false branch is **completely discarded**:

```cpp
template<typename T>
auto stringify(T value) {
    if constexpr (std::is_arithmetic_v<T>)
        return std::to_string(value);
    else
        return std::string(value);
}

stringify(42);       // calls to_string(42)
stringify("hello");  // calls string("hello")
// The other branch doesn't even need to compile!
```

### Why not regular `if`?

```cpp
template<typename T>
auto bad_stringify(T value) {
    if (std::is_arithmetic_v<T>)
        return std::to_string(value);  // ❌ Error when T=const char*
    else
        return std::string(value);     // ❌ Error when T=int
}
// Regular if: BOTH branches are compiled for every T
// constexpr if: only the matching branch is compiled
```

### Eliminates SFINAE for many cases

```cpp
// Before: SFINAE (complex)
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void process(T val) { std::cout << "integer: " << val; }

template<typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
void process(T val) { std::cout << "other: " << val; }

// After: constexpr if (simple!)
template<typename T>
void process(T val) {
    if constexpr (std::is_integral_v<T>)
        std::cout << "integer: " << val;
    else
        std::cout << "other: " << val;
}
```

---

## 7. Nested Namespaces

```cpp
// Before C++17:
namespace my { namespace lib { namespace detail { /* ... */ } } }

// C++17:
namespace my::lib::detail { /* ... */ }
```

Small quality-of-life improvement that reduces indentation.

---

## 8. `std::byte`

A type for raw byte data that is distinct from `char` and `unsigned char`:

```cpp
#include <cstddef>

std::byte b{0xFF};
std::byte c = b & std::byte{0x0F};  // bitwise AND
// std::byte is NOT an integer — can't do arithmetic
// b + c;  // ❌ Error!
auto val = std::to_integer<int>(b);  // convert when needed
```

This prevents accidentally using byte buffers as characters or numbers.

---

## 9. Exercises

See `exercises.cpp`:

1. Use structured bindings with maps, tuples, and custom structs
2. Refactor code to use if-with-initializer
3. Write code using CTAD (no explicit template args)
4. Implement variadic functions using fold expressions
5. Use `constexpr if` to write a single function handling multiple types
6. Create a header-only library using `inline` variables

---

**Next lecture:** C++17 Vocabulary Types — `optional`, `variant`, `any`, and
`string_view`.
