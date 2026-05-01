# Template 11 — Perfect Forwarding & Reference Collapsing

> **Goal:** Understand forwarding references, reference collapsing rules,
> `std::forward`, and how to build factory functions and wrappers that
> perfectly preserve value categories. Know **why** this matters, **how**
> the compiler makes it work, and **when** to use each tool.

---

## Table of Contents

1. [The Problem: Losing Value Category](#1-the-problem)
2. [Forwarding References](#2-forwarding-references)
3. [Reference Collapsing Rules](#3-reference-collapsing-rules)
4. [How Forwarding References Work](#4-how-forwarding-references-work)
5. [`std::forward`](#5-stdforward)
6. [`std::move` vs `std::forward`](#6-stdmove-vs-stdforward)
7. [Factory Functions](#7-factory-functions)
8. [Common Pitfalls](#8-common-pitfalls)
9. [Exercises](#9-exercises)

---

## 1. The Problem: Losing Value Category

### What is value category?

Every expression in C++ is either:
- An **lvalue** — has identity, can be addressed (`x`, `arr[0]`, `*ptr`)
- An **rvalue** — temporary, about to be destroyed (`42`, `std::move(x)`, `f()`)

### Why does it matter?

```cpp
void process(const std::string& s) { /* copies */ }
void process(std::string&& s)      { /* moves — much faster! */ }

std::string name = "hello";
process(name);             // lvalue → copies (safe)
process(std::move(name));  // rvalue → moves (fast)
process("temporary");      // rvalue → moves (fast)
```

### The forwarding problem

When you write a wrapper function, the value category gets lost:

```cpp
template<typename T>
void wrapper(T arg) {    // arg is ALWAYS an lvalue inside wrapper!
    process(arg);         // always calls the copy overload 😞
}

wrapper(std::move(name));  // rvalue passed in, but arg is lvalue → copies!
```

Even if the caller passed an rvalue, `arg` is a named variable, so it's an
lvalue. We need a way to **forward** the original value category.

---

## 2. Forwarding References

### What is a forwarding reference?

A **forwarding reference** (also called "universal reference") is `T&&` where
`T` is a **deduced** template parameter:

```cpp
template<typename T>
void f(T&& arg);   // ← forwarding reference

auto&& x = expr;   // ← also a forwarding reference
```

### What makes it special?

A forwarding reference can bind to **both lvalues AND rvalues**:

```cpp
template<typename T>
void f(T&& arg);

int x = 42;
f(x);            // T deduced as int&,  arg is int&   (lvalue)
f(42);           // T deduced as int,   arg is int&&  (rvalue)
f(std::move(x)); // T deduced as int,   arg is int&&  (rvalue)
```

### NOT forwarding references

These look similar but are **not** forwarding references:

```cpp
void f(int&& x);              // ❌ concrete type, not deduced → rvalue reference
void f(const int&& x);        // ❌ const-qualified → rvalue reference

template<typename T>
void f(std::vector<T>&& v);   // ❌ T is deduced, but the form isn't T&& → rvalue reference

template<typename T>
class Widget {
    void f(T&& x);            // ❌ T is a class template parameter, not deduced here
};
```

### The rule

A forwarding reference requires **both**:
1. The form `T&&` (exactly, no const, no container wrapping)
2. `T` is deduced in **that** function call

---

## 3. Reference Collapsing Rules

### What is reference collapsing?

C++ doesn't allow "reference to reference" (`int& &`). When references
compose (through templates, typedefs, or decltype), they **collapse**:

| Input | Result | Rule |
|-------|--------|------|
| `T& &` | `T&` | lvalue wins |
| `T& &&` | `T&` | lvalue wins |
| `T&& &` | `T&` | lvalue wins |
| `T&& &&` | `T&&` | both rvalue → rvalue |

### The simple rule

If **either** reference is an lvalue reference (`&`), the result is `&`.
Only `&& &&` produces `&&`.

### Why does this matter?

Reference collapsing is what makes forwarding references work:

```cpp
template<typename T>
void f(T&& arg);

int x = 42;
f(x);   // T = int&   → T&& = int& && → collapses to int&  → lvalue!
f(42);  // T = int     → T&& = int&&                        → rvalue!
```

---

## 4. How Forwarding References Work

### Complete deduction walkthrough

```cpp
template<typename T>
void f(T&& arg);

int x = 42;
const int cx = 42;
```

| Call | T deduced as | T&& becomes | arg type |
|------|-------------|-------------|----------|
| `f(x)` | `int&` | `int& &&` → `int&` | lvalue ref |
| `f(cx)` | `const int&` | `const int& &&` → `const int&` | const lvalue ref |
| `f(42)` | `int` | `int&&` | rvalue ref |
| `f(std::move(x))` | `int` | `int&&` | rvalue ref |

### The magic

- When called with an **lvalue**, T deduces as `T&` (reference!)
- When called with an **rvalue**, T deduces as `T` (no reference)
- Reference collapsing then produces the correct type

---

## 5. `std::forward`

### What does it do?

`std::forward<T>(arg)` **conditionally casts** arg to an rvalue:
- If T is a non-reference type → casts to rvalue (was called with rvalue)
- If T is an lvalue reference → does nothing (was called with lvalue)

```cpp
template<typename T>
void wrapper(T&& arg) {
    process(std::forward<T>(arg));
    // If caller passed lvalue → forwards as lvalue
    // If caller passed rvalue → forwards as rvalue
}
```

### How forward works internally

```cpp
// Simplified implementation:
template<typename T>
T&& forward(std::remove_reference_t<T>& arg) {
    return static_cast<T&&>(arg);
}
```

For `T = int&`:
- `static_cast<int& &&>(arg)` → collapses to `int&` → lvalue!

For `T = int`:
- `static_cast<int&&>(arg)` → rvalue!

### Always specify the template argument

```cpp
std::forward<T>(arg);    // ✅ correct — T comes from the function template
std::forward(arg);       // ❌ won't compile — T can't be deduced
```

---

## 6. `std::move` vs `std::forward`

| | `std::move` | `std::forward<T>` |
|--|------------|-------------------|
| **Does** | Unconditionally casts to rvalue | Conditionally casts based on T |
| **Use when** | You KNOW you want to move | You want to preserve original category |
| **Typical location** | Last use of a local variable | Inside a forwarding wrapper |

```cpp
// std::move — "I'm done with this, take it"
std::string name = "hello";
std::string other = std::move(name);  // name is now moved-from

// std::forward — "pass this along as the caller intended"
template<typename T>
void wrapper(T&& arg) {
    inner(std::forward<T>(arg));  // preserves lvalue/rvalue-ness
}
```

---

## 7. Factory Functions

### The classic use case

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

### Why perfect forwarding matters here

```cpp
struct Widget {
    Widget(const std::string& name);  // copies
    Widget(std::string&& name);       // moves — much faster
};

std::string n = "test";
make_unique<Widget>(n);              // forwards as lvalue → copies
make_unique<Widget>(std::move(n));   // forwards as rvalue → moves
make_unique<Widget>("temporary");    // forwards as rvalue → moves
```

Without `std::forward`, the rvalue cases would degrade to copies.

### Emplace functions

All `emplace` methods use perfect forwarding:

```cpp
std::vector<std::string> v;
v.emplace_back("hello");          // constructs string in-place from const char*
v.emplace_back(5, 'x');           // constructs string in-place from (count, char)
```

---

## 8. Common Pitfalls

### 1. Forwarding more than once

After forwarding (or moving), the value may be **moved from**:

```cpp
template<typename T>
void bad(T&& arg) {
    f(std::forward<T>(arg));  // might move arg
    g(std::forward<T>(arg));  // ❌ UB if arg was moved! Use-after-move
}

template<typename T>
void good(T&& arg) {
    f(arg);                       // pass as lvalue (safe copy)
    g(std::forward<T>(arg));      // last use: OK to forward
}
```

### 2. `const T&&` is NOT a forwarding reference

```cpp
template<typename T>
void f(const T&& arg);  // This is an rvalue reference, NOT forwarding!
// Only accepts rvalues. The const prevents moving anyway.
```

### 3. Named variables are always lvalues

```cpp
void f(std::string&& s) {
    // s is an RVALUE REFERENCE, but as a named variable, s is an LVALUE!
    g(s);              // passes as lvalue
    g(std::move(s));   // must move explicitly to pass as rvalue
}
```

### 4. Don't return `std::forward` from a function that returns by value

```cpp
template<typename T>
auto bad_wrapper(T&& arg) {
    return std::forward<T>(arg);  // ⚠️ May return dangling reference!
}

template<typename T>
auto good_wrapper(T&& arg) {
    return std::decay_t<T>(std::forward<T>(arg));  // ✅ Return by value
}
```

### 5. Member variables are always lvalues

```cpp
struct Widget {
    std::string name_;

    void use() {
        // name_ is ALWAYS an lvalue, even in an rvalue Widget
        process(name_);             // lvalue
        process(std::move(name_));  // explicitly move
    }
};
```

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** Library Design Patterns.
