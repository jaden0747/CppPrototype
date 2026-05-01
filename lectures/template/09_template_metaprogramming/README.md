# Template 09 — Template Metaprogramming

> **Goal:** Compute values and manipulate types entirely at compile time
> using recursive templates, type lists, `integral_constant`, and the
> relationship between TMP and `constexpr`. Understand **why** TMP exists,
> **when** to use it, and **how** modern C++ has replaced many of its patterns.

---

## Table of Contents

1. [What Is Template Metaprogramming?](#1-what-is-template-metaprogramming)
2. [Compile-Time Value Computation](#2-compile-time-value-computation)
3. [Recursive Type Manipulation](#3-recursive-type-manipulation)
4. [Type Lists](#4-type-lists)
5. [`integral_constant` & Friends](#5-integral_constant--friends)
6. [`constexpr` vs TMP](#6-constexpr-vs-tmp)
7. [Practical TMP Patterns](#7-practical-tmp-patterns)
8. [Exercises](#8-exercises)

---

## 1. What Is Template Metaprogramming?

### The idea

Template metaprogramming (TMP) is **programming that runs at compile time**.
The "program" is written using templates, and the compiler "executes" it
during compilation. The output is types, values, or code that gets compiled
into the final binary.

### Why does TMP exist?

1. **Zero-runtime cost** — all work happens during compilation
2. **Type manipulation** — `constexpr` can compute values, but only TMP can
   transform types (e.g., remove const, extract element types, build type lists)
3. **Code generation** — generate different code paths based on types
4. **Historical** — before `constexpr` (C++11), TMP was the ONLY way to
   compute values at compile time

### The key insight

Templates are a **functional programming language** that runs at compile time:
- **Variables** are types and compile-time constants
- **Functions** are template specializations
- **Conditionals** are partial specialization or `if constexpr`
- **Loops** are recursive template instantiation
- **Data structures** are type lists (variadic templates)

---

## 2. Compile-Time Value Computation

### Factorial (the "hello world" of TMP)

```cpp
// TMP version: computation via recursive template instantiation
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

static_assert(Factorial<5>::value == 120);
static_assert(Factorial<0>::value == 1);
```

### How the compiler "executes" this

```
Factorial<5>::value
  = 5 * Factorial<4>::value
  = 5 * 4 * Factorial<3>::value
  = 5 * 4 * 3 * Factorial<2>::value
  = 5 * 4 * 3 * 2 * Factorial<1>::value
  = 5 * 4 * 3 * 2 * 1 * Factorial<0>::value   ← base case
  = 5 * 4 * 3 * 2 * 1 * 1
  = 120
```

Each `Factorial<N>` is a **separate class** that the compiler generates.

### Fibonacci

```cpp
template<int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N-1>::value + Fibonacci<N-2>::value;
};

template<> struct Fibonacci<0> { static constexpr int value = 0; };
template<> struct Fibonacci<1> { static constexpr int value = 1; };

static_assert(Fibonacci<10>::value == 55);
```

### GCD (greatest common divisor)

```cpp
template<int A, int B>
struct GCD {
    static constexpr int value = GCD<B, A % B>::value;
};

template<int A>
struct GCD<A, 0> {
    static constexpr int value = A;
};

static_assert(GCD<12, 8>::value == 4);
```

---

## 3. Recursive Type Manipulation

### The power unique to TMP

`constexpr` can compute **values**, but only TMP can manipulate **types**:

```cpp
// Count pointer nesting depth
template<typename T>
struct PointerDepth {
    static constexpr int value = 0;
};

template<typename T>
struct PointerDepth<T*> {           // partial specialization for T*
    static constexpr int value = 1 + PointerDepth<T>::value;
};

static_assert(PointerDepth<int>::value == 0);
static_assert(PointerDepth<int*>::value == 1);
static_assert(PointerDepth<int***>::value == 3);
```

### Remove all pointers

```cpp
template<typename T>
struct RemoveAllPointers {
    using type = T;
};

template<typename T>
struct RemoveAllPointers<T*> {
    using type = typename RemoveAllPointers<T>::type;
};

// RemoveAllPointers<int***>::type == int
using result = RemoveAllPointers<int***>::type;
static_assert(std::is_same_v<result, int>);
```

### How it works

```
RemoveAllPointers<int***>
  → T* matches, T = int**
  → RemoveAllPointers<int**>::type
    → T* matches, T = int*
    → RemoveAllPointers<int*>::type
      → T* matches, T = int
      → RemoveAllPointers<int>::type
        → base case: type = int
```

---

## 4. Type Lists

### What is a type list?

A type list is a compile-time "container" of types, implemented as a
variadic template:

```cpp
template<typename... Ts>
struct TypeList {};

using MyTypes = TypeList<int, double, std::string>;
```

### Size

```cpp
template<typename TL> struct Size;

template<typename... Ts>
struct Size<TypeList<Ts...>> {
    static constexpr size_t value = sizeof...(Ts);
};

static_assert(Size<TypeList<int, double, char>>::value == 3);
```

### Head (first element)

```cpp
template<typename TL> struct Head;

template<typename T, typename... Rest>
struct Head<TypeList<T, Rest...>> {
    using type = T;
};

static_assert(std::is_same_v<Head<TypeList<int, double>>::type, int>);
```

### Tail (everything except first)

```cpp
template<typename TL> struct Tail;

template<typename T, typename... Rest>
struct Tail<TypeList<T, Rest...>> {
    using type = TypeList<Rest...>;
};

// Tail<TypeList<int, double, char>>::type == TypeList<double, char>
```

### Append

```cpp
template<typename TL, typename T> struct Append;

template<typename... Ts, typename T>
struct Append<TypeList<Ts...>, T> {
    using type = TypeList<Ts..., T>;
};

// Append<TypeList<int, double>, char>::type == TypeList<int, double, char>
```

### Contains

```cpp
template<typename TL, typename T> struct Contains;

template<typename T>
struct Contains<TypeList<>, T> : std::false_type {};

template<typename T, typename... Rest>
struct Contains<TypeList<T, Rest...>, T> : std::true_type {};

template<typename Head, typename... Rest, typename T>
struct Contains<TypeList<Head, Rest...>, T> : Contains<TypeList<Rest...>, T> {};

static_assert(Contains<TypeList<int, double, char>, double>::value);
static_assert(!Contains<TypeList<int, double, char>, float>::value);
```

---

## 5. `integral_constant` & Friends

### What is `integral_constant`?

It wraps a **compile-time value** as a **type**, bridging the value/type
worlds:

```cpp
using two = std::integral_constant<int, 2>;
// two::value == 2
// two::type == std::integral_constant<int, 2>
// two{} is implicitly convertible to int (returns 2)
```

### `bool_constant`, `true_type`, `false_type`

```cpp
using true_type = std::integral_constant<bool, true>;
using false_type = std::integral_constant<bool, false>;

// Shorter alias:
using yes = std::bool_constant<true>;
using no = std::bool_constant<false>;
```

### Why wrap values as types?

Because TMP operates on **types**, not values. By wrapping a value in a type,
you can pass it through template machinery:

```cpp
// Type traits inherit from true_type or false_type
template<typename T>
struct is_pointer : std::false_type {};

template<typename T>
struct is_pointer<T*> : std::true_type {};

// is_pointer<int*>::value == true
// is_pointer<int>::value == false
```

---

## 6. `constexpr` vs TMP

### Comparison

| Feature | TMP | `constexpr` |
|---------|-----|-------------|
| Compute values | ✓ (via recursive specialization) | ✓ (via normal functions) |
| Manipulate types | ✓ (the only way) | ✗ |
| Readability | Low (template syntax) | High (normal C++) |
| Error messages | Poor (deep instantiation chains) | Better (function-like) |
| Debugging | Very hard | Easier (constexpr debugger support) |
| Available since | C++98 | C++11/14/17/20 |

### Example: TMP vs constexpr for value computation

```cpp
// TMP — hard to read
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};
template<> struct Factorial<0> { static constexpr int value = 1; };

// constexpr — just a normal function!
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

static_assert(Factorial<5>::value == factorial(5));  // both produce 120
```

### Modern advice

| Task | Approach |
|------|----------|
| Compute values at compile time | Use `constexpr` functions |
| Transform types | Use TMP (type traits, type lists) |
| Conditional compilation | Use `if constexpr` |
| Type-based dispatch | Use concepts (C++20) or TMP |

---

## 7. Practical TMP Patterns

### Index sequence (used everywhere in the standard library)

```cpp
// std::index_sequence<0, 1, 2, 3, 4>
// Generated by: std::make_index_sequence<5>

template<typename Tuple, size_t... Is>
void print_tuple_impl(const Tuple& t, std::index_sequence<Is...>) {
    ((std::cout << std::get<Is>(t) << " "), ...);
}

template<typename... Ts>
void print_tuple(const std::tuple<Ts...>& t) {
    print_tuple_impl(t, std::index_sequence_for<Ts...>{});
}
```

### Conditional type selection

```cpp
template<bool Condition, typename TrueType, typename FalseType>
struct conditional {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct conditional<false, TrueType, FalseType> {
    using type = FalseType;
};

// This is exactly how std::conditional works!
```

---

## 8. Exercises

See `exercises.cpp`.

---

**Next lecture:** Expression Templates & Tag Dispatch.
