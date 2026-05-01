# Template 06 — Concepts: Defining & Using (C++20)

> **Goal:** Replace SFINAE with readable, composable constraints using
> C++20 concepts — `concept`, `requires` clause, and `requires` expression.
> Understand **why** concepts were introduced, **how** they work, and
> **when** to use each syntax form.

---

## Table of Contents

1. [Why Concepts?](#1-why-concepts)
2. [The `concept` Keyword](#2-the-concept-keyword)
3. [Three Ways to Apply Concepts](#3-three-ways-to-apply-concepts)
4. [`requires` Clause](#4-requires-clause)
5. [`requires` Expression](#5-requires-expression)
6. [Compound & Nested Requirements](#6-compound--nested-requirements)
7. [Combining Concepts](#7-combining-concepts)
8. [Concept Design Guidelines](#8-concept-design-guidelines)
9. [Exercises](#9-exercises)

---

## 1. Why Concepts?

### The problem before concepts

SFINAE gives you compile-time constraints, but at a cost:

```cpp
// SFINAE: What does this even mean?
template<typename T,
         std::enable_if_t<std::is_integral_v<T> &&
                          !std::is_same_v<T, bool>, int> = 0>
T add(T a, T b) { return a + b; }

// Error message if you call add("hello", "world"):
// "no matching function for call to 'add'"
// "note: candidate template ignored: substitution failure [with T = const char*]"
// ... 50 more lines of template noise
```

### Concepts fix both problems

```cpp
// Concept: clear intent
template<std::integral T>
T add(T a, T b) { return a + b; }

// Error message:
// "constraints not satisfied for 'add'"
// "note: 'const char*' does not satisfy 'integral'"
```

### What concepts give you

| Benefit | Details |
|---------|---------|
| **Readability** | `template<Sortable T>` vs `enable_if_t<is_sortable_v<T>>` |
| **Better errors** | Compiler tells you *which constraint* failed and *why* |
| **Composability** | Combine with `&&` and `||` like boolean logic |
| **Subsumption** | Compiler picks the "most constrained" overload automatically |
| **Documentation** | Concepts serve as self-documenting requirements |

---

## 2. The `concept` Keyword

### What is a concept?

A concept is a **named compile-time boolean predicate** on types:

```cpp
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;
// Numeric<int> == true
// Numeric<std::string> == false
```

### Concepts can use `requires` expressions

```cpp
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};
// Addable checks: "Can I add two T's and get something convertible to T?"
```

### Concepts can combine type traits

```cpp
template<typename T>
concept SignedNumeric = std::is_arithmetic_v<T> && std::is_signed_v<T>;
```

### Key rules

- Concepts are always `bool` (true or false)
- Concepts cannot be specialized
- Concepts are evaluated at compile time — zero runtime cost
- Concepts should describe **semantic requirements**, not just syntax

---

## 3. Three Ways to Apply Concepts

Given a concept `Numeric`, you can constrain a template in three ways:

```cpp
// 1. Constrained template parameter (most concise)
template<Numeric T>
T square(T x) { return x * x; }

// 2. requires clause (most flexible)
template<typename T> requires Numeric<T>
T cube(T x) { return x * x * x; }

// 3. Trailing requires clause (useful for member functions)
template<typename T>
T negate(T x) requires Numeric<T> { return -x; }
```

### When to use which?

| Syntax | Best for |
|--------|----------|
| `template<Concept T>` | Simple, single constraint — default choice |
| `requires Concept<T>` | Complex expressions, multiple conditions |
| `T f(T x) requires ...` | Class member functions, when template head is far away |

### Abbreviated function templates (terse syntax)

```cpp
// Even shorter — auto with concept
void print(std::integral auto val) {
    std::cout << val << "\n";
}

// Equivalent to:
template<std::integral T>
void print(T val) {
    std::cout << val << "\n";
}
```

---

## 4. `requires` Clause

### What is it?

A `requires` clause attaches a boolean constraint to a template:

```cpp
template<typename T>
    requires std::is_default_constructible_v<T>
T make_default() { return T{}; }

template<typename T>
    requires (sizeof(T) <= 8)        // parentheses for non-concept expressions
void process_small(T val) { /* ... */ }
```

### Combining constraints in requires clause

```cpp
template<typename T>
    requires std::integral<T> && (sizeof(T) >= 4)
void process(T val) { /* only 32-bit+ integers */ }

template<typename T>
    requires (std::integral<T> || std::floating_point<T>)
void math_func(T val) { /* integers or floats */ }
```

### Constraining non-type parameters

```cpp
template<typename T, int N>
    requires (N > 0 && N <= 1024)
class FixedBuffer {
    T data_[N];
};
```

---

## 5. `requires` Expression

### What is it?

A `requires` expression tests if a set of expressions are **valid** (compile)
for a given type. It's used **inside** concept definitions.

### Simple requirements

```cpp
template<typename T>
concept Printable = requires(T a) {
    std::cout << a;  // This expression must compile
};

template<typename T>
concept Container = requires(T c) {
    c.begin();                   // must have begin()
    c.end();                     // must have end()
    c.size();                    // must have size()
    typename T::value_type;      // must have a value_type member type
    typename T::iterator;        // must have an iterator type
};
```

### How to read it

```
requires(T c) {        ← "given a variable c of type T..."
    c.begin();         ← "...the expression c.begin() must be valid"
    c.size();          ← "...and c.size() must be valid"
    typename T::value_type;  ← "...and T::value_type must name a type"
}
```

### Type requirements with `typename`

```cpp
template<typename T>
concept HasValueType = requires {
    typename T::value_type;  // T must have a nested ::value_type type
};
```

---

## 6. Compound & Nested Requirements

### Compound requirements: constrain the return type

```cpp
template<typename T>
concept Sortable = requires(T a, T b) {
    // Simple: expression must compile
    a < b;

    // Compound: expression must compile AND return type must satisfy constraint
    { a < b } -> std::convertible_to<bool>;
    { a == b } -> std::same_as<bool>;
};
```

### How compound requirements work

```
{ expression } -> concept<Args...>;
```
means:
1. `expression` must be a valid expression
2. `decltype((expression))` must satisfy `concept<decltype((expression)), Args...>`

### Nested requirements: check compile-time conditions

```cpp
template<typename T>
concept SmallSortable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;

    // Nested requirement: a compile-time boolean check
    requires std::is_copy_constructible_v<T>;
    requires sizeof(T) <= 64;
};
```

### Combining all three

```cpp
template<typename T>
concept FullyFeatured = requires(T a, T b) {
    // Simple requirement
    a + b;

    // Compound requirement
    { a - b } -> std::same_as<T>;

    // Type requirement
    typename T::value_type;

    // Nested requirement
    requires std::is_move_constructible_v<T>;
};
```

---

## 7. Combining Concepts

### Conjunction (AND)

```cpp
template<typename T>
concept OrderedContainer = Container<T> && requires(T c) {
    requires std::totally_ordered<typename T::value_type>;
};
```

### Disjunction (OR)

```cpp
template<typename T>
concept StringLike = std::same_as<T, std::string>
                  || std::same_as<T, std::string_view>
                  || std::same_as<T, const char*>;
```

### Concept refinement (building hierarchies)

```cpp
template<typename T>
concept Movable = std::is_move_constructible_v<T>
               && std::is_move_assignable_v<T>;

template<typename T>
concept Copyable = Movable<T>
                && std::is_copy_constructible_v<T>
                && std::is_copy_assignable_v<T>;

template<typename T>
concept Regular = Copyable<T>
               && std::is_default_constructible_v<T>
               && std::equality_comparable<T>;
```

This is exactly how the standard library concepts are organized.

---

## 8. Concept Design Guidelines

### Good concepts

```cpp
// ✅ Describes semantic requirement
template<typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

// ✅ Named after what the type CAN DO
template<typename T>
concept Drawable = requires(T obj, Canvas& c) {
    obj.draw(c);
    { obj.bounding_box() } -> std::same_as<Rect>;
};
```

### Avoid

```cpp
// ❌ Too specific — just use the concrete type
template<typename T>
concept IsExactlyMyClass = std::same_as<T, MyClass>;

// ❌ Too broad — every type satisfies this
template<typename T>
concept Anything = true;

// ❌ Checks syntax without semantic meaning
template<typename T>
concept HasFoo = requires(T t) { t.foo(); };
// What does foo() mean? A good concept name should tell you.
```

---

## 9. Exercises

See `exercises.cpp`.

---

**Next lecture:** Standard Concepts & Constrained Templates.
