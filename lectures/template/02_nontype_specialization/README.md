# Template 02 — Non-Type Parameters & Specialization

> **Goal:** Use compile-time values as template arguments. Customize behavior
> for specific types via full and partial specialization. Understand **when**
> and **why** to specialize, and the pitfalls to avoid.

---

## Table of Contents

1. [Non-Type Template Parameters](#1-non-type-template-parameters)
2. [Why Non-Type Parameters?](#2-why-non-type-parameters)
3. [`template<auto>` (C++17)](#3-templateauto-c17)
4. [Full Template Specialization](#4-full-template-specialization)
5. [Partial Template Specialization](#5-partial-template-specialization)
6. [Function Template Overloading vs Specialization](#6-function-template-overloading-vs-specialization)
7. [When To Specialize and When Not To](#7-when-to-specialize-and-when-not-to)
8. [Exercises](#8-exercises)

---

## 1. Non-Type Template Parameters

### What are they?

Template parameters can be **values**, not just types. This means you can
pass numbers, booleans, pointers, and (in C++20) even floating-point values
and class types as template arguments.

```cpp
template<typename T, int N>
struct FixedArray {
    T data[N];
    constexpr int size() const { return N; }
};

FixedArray<int, 5> a;      // 5-element int array
FixedArray<double, 10> b;  // 10-element double array
```

### Key insight: the value is known at compile time

`N` is a **compile-time constant**, not a runtime variable. This means:
- The compiler knows the array size at compile time
- `FixedArray<int, 5>` and `FixedArray<int, 10>` are **different types**
- The compiler can optimize based on the value (unroll loops, etc.)

### Allowed non-type parameter types

| Type | Available since | Example |
|------|----------------|---------|
| Integral (`int`, `size_t`, `char`, `bool`) | C++98 | `template<int N>` |
| Enum types | C++98 | `template<Color C>` |
| Pointers to objects/functions | C++98 | `template<int* P>` |
| `auto` (deduce the type) | C++17 | `template<auto V>` |
| Floating-point (`float`, `double`) | C++20 | `template<double D>` |
| Class types with `<=>` | C++20 | `template<MyStruct S>` |

### Real-world examples in the standard library

```cpp
std::array<int, 5>            // N = 5
std::bitset<64>               // N = 64
std::integer_sequence<int, 0, 1, 2>  // non-type pack
```

---

## 2. Why Non-Type Parameters?

### Compile-time guarantees

```cpp
template<int Rows, int Cols>
class Matrix {
    double data[Rows][Cols];
public:
    // Only allow multiplication when dimensions match
    template<int OtherCols>
    Matrix<Rows, OtherCols> operator*(const Matrix<Cols, OtherCols>& other);
    //                                        ^^^^ must match our Cols!
};

Matrix<2, 3> a;
Matrix<3, 4> b;
auto c = a * b;   // OK: 2x3 × 3x4 = 2x4

Matrix<3, 5> d;
// auto e = a * d; // ❌ Compile error! 2x3 × 3x5 ← our Cols=3 ≠ d's Rows=3... wait, it matches!
// auto f = b * a; // ❌ Compile error! 3x4 × 2x3 ← b's Cols=4 ≠ a's Rows=2
```

The dimensions are enforced **at compile time**. No runtime checks needed.

### Zero overhead

Because the value is baked into the type, the compiler generates specialized
code. `FixedArray<int, 4>` stores exactly 4 ints on the stack — no heap
allocation, no size member, no overhead.

---

## 3. `template<auto>` (C++17)

### What is it?

`auto` as a non-type parameter lets the compiler deduce the type of the value:

```cpp
template<auto Value>
struct Constant {
    static constexpr auto value = Value;
};

Constant<42>::value;     // int 42
Constant<'A'>::value;    // char 'A'
Constant<true>::value;   // bool true
```

### Why use it?

Before C++17, you had to specify the type:

```cpp
// Old way: must specify int
template<int N> struct OldConstant { static constexpr int value = N; };

// New way: auto deduces it
template<auto N> struct NewConstant { static constexpr auto value = N; };
```

### Non-type parameter packs with auto

```cpp
template<auto... Values>
struct ValueList {};

using ints = ValueList<1, 2, 3>;       // all int
using mixed = ValueList<1, 'a', true>; // int, char, bool
```

---

## 4. Full Template Specialization

### What is it?

Full specialization provides a **completely different implementation** for a
specific type. The compiler uses the specialization instead of the primary
template when the types match exactly.

### Why specialize?

- The generic implementation doesn't work for some type
- You can provide a much faster/better implementation for a specific type
- You need different behavior (e.g., `std::hash<T>` must be specialized for custom types)

### Syntax

```cpp
// Primary template — the general case
template<typename T>
struct TypeName {
    static const char* get() { return "unknown"; }
};

// Full specialization for int
template<>
struct TypeName<int> {
    static const char* get() { return "int"; }
};

// Full specialization for double
template<>
struct TypeName<double> {
    static const char* get() { return "double"; }
};

// Full specialization for std::string
template<>
struct TypeName<std::string> {
    static const char* get() { return "std::string"; }
};
```

### How it works

```cpp
TypeName<int>::get();         // "int"     — uses specialization
TypeName<double>::get();      // "double"  — uses specialization
TypeName<float>::get();       // "unknown" — uses primary template
TypeName<std::string>::get(); // "std::string" — uses specialization
```

### Key rules

1. The specialization must appear **after** the primary template declaration
2. `template<>` prefix — no template parameters left (all specified)
3. The specialization must match the primary template's structure

---

## 5. Partial Template Specialization

### What is it?

Partial specialization customizes a template for a **pattern** rather than a
specific type. For example, "all pointer types" or "all vectors of any type."

### Why use it?

- Handle categories of types (all pointers, all references, all containers)
- Recursive template metaprogramming (specialize for base case)

### Example: Serializer with partial specialization

```cpp
// Primary: generic
template<typename T>
struct Serializer {
    static std::string to_string(const T& val) {
        return std::to_string(val);
    }
};

// Partial: for all pointer types T*
template<typename T>
struct Serializer<T*> {
    static std::string to_string(T* ptr) {
        return ptr ? Serializer<T>::to_string(*ptr) : "null";
    }
};

// Partial: for all vectors
template<typename T>
struct Serializer<std::vector<T>> {
    static std::string to_string(const std::vector<T>& v) {
        std::string result = "[";
        for (size_t i = 0; i < v.size(); ++i) {
            if (i) result += ", ";
            result += Serializer<T>::to_string(v[i]);
        }
        return result + "]";
    }
};

// Partial: for all pairs
template<typename A, typename B>
struct Serializer<std::pair<A, B>> {
    static std::string to_string(const std::pair<A, B>& p) {
        return "(" + Serializer<A>::to_string(p.first) + ", "
                   + Serializer<B>::to_string(p.second) + ")";
    }
};
```

### Usage

```cpp
Serializer<int>::to_string(42);           // "42"
Serializer<int*>::to_string(&x);          // "42" (dereferences)
Serializer<std::vector<int>>::to_string({1,2,3}); // "[1, 2, 3]"
```

### Important: partial specialization is for **class templates only**

Function templates **cannot** be partially specialized. Use overloading instead.

---

## 6. Function Template Overloading vs Specialization

### The problem with function specialization

```cpp
// Primary template
template<typename T>
void process(T val) { std::cout << "generic\n"; }

// Specialization for int*
template<>
void process<int*>(int* val) { std::cout << "int pointer\n"; }

// Overload for any pointer
template<typename T>
void process(T* ptr) { std::cout << "pointer overload\n"; }

int x = 42;
process(&x);  // Which is called? The OVERLOAD! Not the specialization!
```

### Why?

The compiler resolves **overloads first**, then looks at specializations of
the chosen overload. Since the `T*` overload is a better match than the
primary `T` template, the specialization of the primary template is never
considered.

### Rule of thumb

| What you want | For functions | For classes |
|--------------|--------------|-------------|
| Different behavior for specific types | **Overloading** | Full specialization |
| Different behavior for patterns | **Overloading** | Partial specialization |
| Completely different implementation | **Overloading** | Full specialization |

**For functions:** Always prefer **overloading** over specialization.
**For classes:** Use specialization (it's the only option).

---

## 7. When To Specialize and When Not To

### Specialize when:

1. The generic implementation **doesn't compile** for a type
   (e.g., `std::hash<YourType>`)
2. You can provide a **significantly better** implementation
   (e.g., `std::vector<bool>` uses bit-packing)
3. You need **completely different behavior** for a type

### Don't specialize when:

1. A **simple `if constexpr`** would suffice (C++17)
2. **Concepts/constraints** can handle it (C++20)
3. The specialization is **trivially different** from the primary

### Modern alternatives to specialization

```cpp
// Instead of full specialization:
template<typename T>
std::string to_name() {
    if constexpr (std::is_same_v<T, int>) return "int";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else return "unknown";
}

// Or with concepts (C++20):
template<std::integral T>
void process(T val) { /* integer path */ }

template<std::floating_point T>
void process(T val) { /* float path */ }
```

---

## 8. Exercises

See `exercises.cpp`.

---

**Next lecture:** Variadic Templates.
