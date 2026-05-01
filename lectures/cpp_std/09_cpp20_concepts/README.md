# Lecture 09 — C++20 Concepts

> **Goal:** Master concepts — the C++20 mechanism for constraining templates
> with clear, readable requirements. Eliminate cryptic SFINAE errors forever.
> Understand **what** concepts are, **why** they're revolutionary for C++,
> **when** to use each syntax, and **how** the compiler resolves overloads.

---

## Table of Contents

1. [The Problem Concepts Solve](#1-the-problem-concepts-solve)
2. [Defining Concepts](#2-defining-concepts)
3. [Using Concepts (4 syntaxes)](#3-using-concepts-4-syntaxes)
4. [`requires` Expressions](#4-requires-expressions)
5. [Standard Library Concepts](#5-standard-library-concepts)
6. [Concept Subsumption & Overloading](#6-concept-subsumption--overloading)
7. [Practical Design Guidelines](#7-practical-design-guidelines)
8. [Exercises](#8-exercises)

---

## 1. The Problem Concepts Solve

### Before concepts: SFINAE horror

```cpp
// SFINAE mess — hard to read, hard to debug:
template<typename T,
         typename = std::enable_if_t<
             std::is_integral_v<T> && !std::is_same_v<T, bool>>>
T add(T a, T b) { return a + b; }
```

When you call `add(3.14, 2.71)`, the error message is something like:

```
error: no matching function for template 'add'
note: candidate template ignored: requirement
'std::is_integral_v<double>' was not satisfied [with T = double]
note: in instantiation of default argument for 'add<double>'
note: ... 50 more lines of template noise ...
```

### After concepts: readable constraints and clear errors

```cpp
template<std::integral T>
T add(T a, T b) { return a + b; }
```

Error for `add(3.14, 2.71)`:
```
error: constraints not satisfied: 'std::integral<double>' is false
```

One line. Clear. Actionable.

### What IS a concept?

A concept is a **named compile-time predicate on types** (or values). It answers:
"Does type T have the properties I need?"

Think of it as a **contract** between the template author and the template user:
- Author says: "Give me any type that satisfies `Sortable`"
- User sees: clear documentation of what's required
- Compiler checks: at the call site, not deep inside template internals

---

## 2. Defining Concepts

### Syntax

```cpp
template<typename T>
concept ConceptName = constraint_expression;
```

The constraint expression must evaluate to `bool` at compile time.

### Simple concepts using type traits

```cpp
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept SignedNumber = Numeric<T> && std::is_signed_v<T>;
```

### Concepts using `requires` expressions

```cpp
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;  // a+b must be valid AND return T
};

template<typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Printable = requires(std::ostream& os, T val) {
    { os << val } -> std::same_as<std::ostream&>;
};
```

### Composing concepts with `&&` and `||`

```cpp
template<typename T>
concept PrintableNumeric = Numeric<T> && Printable<T>;

template<typename T>
concept NumberOrString = Numeric<T> || std::convertible_to<T, std::string>;

template<typename T>
concept Sortable = std::random_access_iterator<T>
                && std::totally_ordered<std::iter_value_t<T>>;
```

### Multi-parameter concepts

```cpp
template<typename T, typename U>
concept Addable2 = requires(T a, U b) {
    { a + b };  // T + U must be valid
};

template<typename F, typename... Args>
concept CallableWith = std::invocable<F, Args...>;
```

---

## 3. Using Concepts (4 syntaxes)

All four are equivalent — use whichever reads best for the situation:

```cpp
// 1. Constrained template parameter — cleanest for simple cases
template<Numeric T>
T square(T x) { return x * x; }

// 2. Requires clause — for complex compound constraints
template<typename T> requires Numeric<T> && Printable<T>
T square(T x) { return x * x; }

// 3. Trailing requires clause — when constraint uses return type
template<typename T>
T square(T x) requires Numeric<T> { return x * x; }

// 4. Abbreviated function template (terse) — shortest
auto square(Numeric auto x) { return x * x; }
```

### When to use which syntax?

| Syntax | Best for |
|--------|----------|
| `template<Concept T>` | Simple, single constraint on one parameter |
| `requires` clause | Multiple constraints, compound expressions |
| Trailing requires | When constraint involves return type or other params |
| Terse (`Concept auto`) | Short utility functions, lambdas |

### Terse syntax with multiple parameters

```cpp
// Each parameter can have its own concept:
void process(std::integral auto x, std::floating_point auto y) {
    // x and y can be DIFFERENT types
    // Unlike template<Numeric T> where both would be T
}

// Constraining a lambda:
auto print_num = [](Numeric auto val) { std::cout << val; };
```

---

## 4. `requires` Expressions

A `requires` expression is a compile-time test: "Can I write this code with type T?"

### Simple requirements — "is this expression valid?"

```cpp
requires(T a) {
    a + a;          // expression must compile
    a.size();       // must have .size() method
    T{};            // must be default-constructible
    *a;             // must be dereferenceable
}
```

### Type requirements — "does this nested type exist?"

```cpp
requires {
    typename T::value_type;      // must have nested type
    typename T::iterator;        // must have nested iterator type
    typename std::hash<T>;       // must be hashable
}
```

### Compound requirements — "expression valid AND returns right type"

```cpp
requires(T a, T b) {
    // Expression must be valid AND its return type must satisfy a concept:
    { a + b } -> std::same_as<T>;
    { a.size() } -> std::convertible_to<std::size_t>;
    { a < b } -> std::same_as<bool>;  // NOT convertible_to — must BE bool

    // Can also require noexcept:
    { a.swap(b) } noexcept;
    { a < b } noexcept -> std::same_as<bool>;
}
```

### Nested requirements — "compile-time predicate must be true"

```cpp
requires(T a) {
    requires sizeof(T) <= 16;                         // size constraint
    requires std::is_trivially_copyable_v<T>;         // trait must be true
    requires std::derived_from<T, SomeBase>;          // concept within requires
}
```

### Complete example: a Container concept

```cpp
template<typename C>
concept Container = requires(C c) {
    // Type requirements
    typename C::value_type;
    typename C::iterator;
    typename C::size_type;

    // Simple requirements
    { c.begin() } -> std::same_as<typename C::iterator>;
    { c.end() } -> std::same_as<typename C::iterator>;
    { c.size() } -> std::convertible_to<std::size_t>;
    { c.empty() } -> std::same_as<bool>;

    // Nested requirement
    requires std::input_iterator<typename C::iterator>;
};
```

---

## 5. Standard Library Concepts

### From `<concepts>` — core language concepts

| Concept | Meaning | Example types |
|---------|---------|---------------|
| `std::same_as<T, U>` | Types are identical | — |
| `std::derived_from<D, B>` | D derives from B publicly | — |
| `std::convertible_to<From, To>` | Implicit + explicit conversion | int→double |
| `std::integral<T>` | Integer types | int, long, char |
| `std::floating_point<T>` | Float types | float, double |
| `std::signed_integral<T>` | Signed integers | int, long |
| `std::unsigned_integral<T>` | Unsigned integers | unsigned, size_t |
| `std::regular<T>` | Copyable + == + default init | most value types |
| `std::movable<T>` | Move construct + assign + swap | unique_ptr |
| `std::copyable<T>` | Copy + move | shared_ptr, string |
| `std::semiregular<T>` | Copyable + default init | most containers |
| `std::equality_comparable<T>` | Has `==` and `!=` | most types |
| `std::totally_ordered<T>` | Has all 6 comparison ops | int, string |
| `std::invocable<F, Args...>` | F callable with Args | lambdas, functors |

### From `<iterator>` — iterator concepts

- `std::input_iterator`, `std::output_iterator`
- `std::forward_iterator`, `std::bidirectional_iterator`
- `std::random_access_iterator`, `std::contiguous_iterator`
- `std::sentinel_for<S, I>`

### From `<ranges>` — range concepts

- `std::ranges::range`, `std::ranges::sized_range`
- `std::ranges::input_range`, `std::ranges::forward_range`
- `std::ranges::view`

---

## 6. Concept Subsumption & Overloading

### What is subsumption?

When multiple constrained overloads match, the compiler picks the **most
constrained** one. A concept C1 **subsumes** C2 if C1's constraints include
all of C2's constraints (and more).

```cpp
template<typename T>
void f(T) { std::cout << "unconstrained\n"; }

template<std::integral T>
void f(T) { std::cout << "integral\n"; }

template<std::signed_integral T>
void f(T) { std::cout << "signed_integral\n"; }

f(42);    // "signed_integral" — most constrained match
f(42u);   // "integral" — unsigned, so signed doesn't match
f(3.14);  // "unconstrained" — not integral at all
```

### Why does this work?

`signed_integral` is defined as:
```cpp
template<typename T>
concept signed_integral = std::integral<T> && std::is_signed_v<T>;
```

So `signed_integral` includes all requirements of `integral` plus more.
The compiler sees that `signed_integral` ⊃ `integral` ⊃ unconstrained.

### Subsumption only works with concepts!

```cpp
// ❌ Ambiguous — no subsumption for raw type traits:
template<typename T> requires std::is_integral_v<T>
void g(T);
template<typename T> requires std::is_integral_v<T> && std::is_signed_v<T>
void g(T);
// g(42) → AMBIGUOUS! Compiler can't compare raw requires clauses.

// ✅ Works — use named concepts:
template<std::integral T> void g(T);
template<std::signed_integral T> void g(T);
// g(42) → calls signed_integral version
```

---

## 7. Practical Design Guidelines

### When to create a concept

- **DO** create concepts for interfaces you'll use in multiple places
- **DO** create concepts that correspond to documented requirements
- **DON'T** create one-off concepts for a single function
- **DON'T** over-constrain: only require what you actually use

### Naming conventions

```cpp
// Good: adjective-like or noun-like
concept Sortable = ...;
concept Container = ...;
concept Printable = ...;
concept RandomAccessRange = ...;

// Avoid: verb forms or too-specific names
concept CanBeSorted = ...;     // prefer Sortable
concept HasSizeAndBegin = ...; // too implementation-specific
```

### Prefer concepts over SFINAE for new code

```cpp
// Old SFINAE code:
template<typename T, std::enable_if_t<has_to_string_v<T>, int> = 0>
std::string convert(T val);

// New concepts code:
template<typename T> requires requires(T t) { { t.to_string() } -> std::same_as<std::string>; }
std::string convert(T val);

// Even better: name the concept
template<typename T>
concept Stringifiable = requires(T t) {
    { t.to_string() } -> std::same_as<std::string>;
};
template<Stringifiable T>
std::string convert(T val);
```

---

## 8. Exercises

See `exercises.cpp`:

1. Define a `Container` concept (has `begin()`, `end()`, `size()`)
2. Write a `Serializable` concept and constrained `serialize()` function
3. Use all four syntaxes to write the same constrained function
4. Implement concept-based overloading (dispatch by constraint)
5. Create a concept hierarchy (Number → Integer → SignedInteger)
6. Use subsumption to resolve overloads correctly

---

**Next lecture:** C++20 Ranges — lazy, composable algorithms.
