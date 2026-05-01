# Template 04 — SFINAE & Type Traits

> **Goal:** Control overload resolution at compile time using SFINAE and
> leverage `<type_traits>` for type inspection and transformation. Understand
> why SFINAE exists, how it works, and when to use it vs modern alternatives.

---

## Table of Contents

1. [What Is SFINAE?](#1-what-is-sfinae)
2. [How SFINAE Works](#2-how-sfinae-works)
3. [`std::enable_if`](#3-stdenable_if)
4. [Type Traits Overview](#4-type-traits-overview)
5. [Type Transformations](#5-type-transformations)
6. [`decltype` & `declval`](#6-decltype--declval)
7. [Detection Idiom (Pre-Concepts)](#7-detection-idiom)
8. [SFINAE vs Concepts — When to Use Which](#8-sfinae-vs-concepts)
9. [Exercises](#9-exercises)

---

## 1. What Is SFINAE?

### The name

**S**ubstitution **F**ailure **I**s **N**ot **A**n **E**rror.

### The problem it solves

When you have multiple template overloads, what happens when one of them
doesn't work for a given type?

```cpp
template<typename T>
typename T::value_type get_first(const T& container) {
    return *container.begin();
}

template<typename T>
T identity(T value) {
    return value;
}

get_first(std::vector<int>{1, 2, 3}); // OK: vector has ::value_type
identity(42);                           // OK: int has no ::value_type, but that's fine!
```

When the compiler tries `get_first(42)`, it substitutes `T = int` and gets
`int::value_type` — which doesn't exist. Without SFINAE, this would be a
**compilation error**. With SFINAE, the compiler simply **removes this
overload from consideration** and tries the next one.

### The key insight

SFINAE turns "impossible overloads" into "ignored overloads." This lets you
write templates that **conditionally exist** based on type properties.

---

## 2. How SFINAE Works

### Step by step

1. Compiler sees a function call like `foo(42)`
2. Finds all candidate overloads/templates named `foo`
3. Tries to **substitute** template arguments for each template
4. If substitution fails in the **immediate context** → silently remove
   (SFINAE)
5. If substitution succeeds → add to viable overloads
6. Pick the best match from remaining viable overloads

### "Immediate context" — the critical detail

SFINAE only applies to failures in:
- Return type
- Template parameter list
- Function parameter types

Failures **inside the function body** are **hard errors** (not SFINAE):

```cpp
template<typename T>
auto foo(T x) -> decltype(x.bar()) {  // SFINAE-friendly (return type)
    return x.bar();
}

template<typename T>
void foo(T x) {
    x.bar();  // ❌ NOT SFINAE! If T has no .bar(), this is a hard error
}
```

---

## 3. `std::enable_if`

### What is it?

`enable_if` is the classic tool for SFINAE. It either provides a type member
(`type`) or doesn't, depending on a boolean condition:

```cpp
// If B is true:  enable_if<true, T>  has ::type = T
// If B is false: enable_if<false, T> has NO ::type → substitution failure!
```

### Usage pattern 1: in return type

```cpp
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
safe_divide(T a, T b) {
    return b != 0 ? a / b : 0;
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T>
safe_divide(T a, T b) {
    return b != T{} ? a / b : T{};
}

safe_divide(10, 3);    // calls integer version → 3
safe_divide(10.0, 3.0); // calls floating version → 3.333...
safe_divide("hi", "lo"); // ❌ no matching overload!
```

### Usage pattern 2: in template parameter (cleaner)

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void process(T val) { /* integer path */ }

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
void process(T val) { /* floating path */ }
```

The `= 0` default means you don't need to pass the extra parameter.
The `int` is just a dummy type — any type works.

### Usage pattern 3: in function parameter

```cpp
template<typename T>
void process(T val, std::enable_if_t<std::is_integral_v<T>>* = nullptr) {
    /* integer path */
}
```

### Which pattern to prefer?

1. **Template parameter** (pattern 2) — cleanest, most common
2. **Return type** (pattern 1) — visible in function signature
3. **Function parameter** (pattern 3) — works but ugly

---

## 4. Type Traits Overview

### What are type traits?

Type traits are **compile-time queries** about types. They answer questions
like "Is T an integer?", "Is T const?", "Is T a pointer?".

They live in `<type_traits>` and provide `::value` (a `bool`) plus the
`_v` shorthand.

### Categories of type traits

| Category | Examples | What they ask |
|----------|---------|--------------|
| **Primary** | `is_integral`, `is_floating_point`, `is_pointer`, `is_class`, `is_enum` | "What kind of type is T?" |
| **Composite** | `is_arithmetic`, `is_fundamental`, `is_object`, `is_scalar` | "Is T in this broad category?" |
| **Properties** | `is_const`, `is_volatile`, `is_signed`, `is_unsigned`, `is_empty` | "Does T have this property?" |
| **Relationships** | `is_same`, `is_base_of`, `is_convertible` | "How do T and U relate?" |
| **Operations** | `is_constructible`, `is_assignable`, `is_destructible`, `is_invocable` | "Can I do X with T?" |

### Examples

```cpp
static_assert(std::is_integral_v<int>);          // true
static_assert(std::is_floating_point_v<double>);  // true
static_assert(std::is_pointer_v<int*>);           // true
static_assert(!std::is_pointer_v<int>);           // true (int is NOT a pointer)
static_assert(std::is_same_v<int, int>);          // true
static_assert(!std::is_same_v<int, long>);        // true
static_assert(std::is_base_of_v<Base, Derived>);  // true
static_assert(std::is_constructible_v<std::string, const char*>); // true
```

### Real-world use case: compile-time validation

```cpp
template<typename T>
class Buffer {
    static_assert(std::is_trivially_copyable_v<T>,
                  "Buffer requires trivially copyable types for memcpy safety");
    T data_[1024];
};
```

---

## 5. Type Transformations

Type traits can also **transform** types:

```cpp
// Remove qualifiers
std::remove_const_t<const int>          // int
std::remove_volatile_t<volatile int>    // int
std::remove_cv_t<const volatile int>    // int
std::remove_reference_t<int&>           // int
std::remove_reference_t<int&&>          // int
std::remove_pointer_t<int*>             // int

// Add qualifiers
std::add_const_t<int>                   // const int
std::add_pointer_t<int>                 // int*
std::add_lvalue_reference_t<int>        // int&
std::add_rvalue_reference_t<int>        // int&&

// Compound transformations
std::decay_t<const int&>               // int (strips ref, const, array→ptr)
std::decay_t<int[5]>                   // int*
std::decay_t<int(double)>             // int(*)(double) (function→function ptr)

// Conditional
std::conditional_t<true, int, double>   // int
std::conditional_t<false, int, double>  // double

// Common type
std::common_type_t<int, double>         // double
std::common_type_t<int, long, float>    // float
```

### `std::decay_t` explained

`decay_t` mimics what happens when you pass an argument **by value**:
- Removes references
- Removes `const`/`volatile`
- Converts arrays to pointers
- Converts functions to function pointers

---

## 6. `decltype` & `declval`

### `decltype` — deduce the type of an expression

```cpp
int x = 42;
decltype(x) y = 10;          // y is int
decltype(x + 1.0) z = 3.14;  // z is double

auto add(int a, double b) -> decltype(a + b) {
    return a + b;  // return type is double
}
```

### `declval` — get a "fake" reference to T

`declval<T>()` gives you a reference to T **without constructing it**.
This is essential for SFINAE checks on types that may not be
default-constructible:

```cpp
template<typename T, typename U>
using add_result_t = decltype(std::declval<T>() + std::declval<U>());

// Works even if T and U have no default constructor!
// We're just asking: "what type would T + U produce?"
add_result_t<int, double>;  // double
```

### `declval` can only be used in unevaluated contexts

```cpp
// OK: decltype is unevaluated
using type = decltype(std::declval<Widget>());

// ❌ ERROR: can't actually call declval at runtime
auto w = std::declval<Widget>();
```

---

## 7. Detection Idiom (Pre-Concepts)

### The "void_t" trick

`std::void_t<...>` maps any set of valid types to `void`. If any type is
invalid, SFINAE kicks in:

```cpp
// Primary: T does NOT have .size()
template<typename T, typename = void>
struct has_size : std::false_type {};

// Specialization: T DOES have .size()
template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

has_size<std::vector<int>>::value;  // true
has_size<int>::value;               // false
```

### How it works

1. Compiler tries the partial specialization first
2. It evaluates `std::void_t<decltype(declval<T>().size())>`
3. If `T::size()` exists → evaluates to `void` → specialization matches → `true_type`
4. If `T::size()` doesn't exist → substitution failure → falls back to primary → `false_type`

---

## 8. SFINAE vs Concepts — When to Use Which

### C++20 concepts are the modern replacement

| SFINAE approach | Concepts equivalent |
|----------------|-------------------|
| `enable_if_t<is_integral_v<T>>` | `std::integral T` |
| `void_t<decltype(expr)>` | `requires { expr; }` |
| Tag dispatch with traits | Constrained overloads |
| `is_same_v<T, U>` | `std::same_as<T, U>` |

### When to use SFINAE (still)

- Pre-C++20 codebases
- Working with code that must compile with C++14/17
- Some edge cases where concepts can't express the constraint

### When to use concepts (preferred)

- Any C++20 or later code
- When readability matters (concepts are much clearer)
- When you want better error messages

```cpp
// SFINAE — hard to read, cryptic errors
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T add(T a, T b) { return a + b; }

// Concepts — clear intent, readable errors
template<std::integral T>
T add(T a, T b) { return a + b; }
```

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** Template Aliases, Variable Templates & Lambdas.
