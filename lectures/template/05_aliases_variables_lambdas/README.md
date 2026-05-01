# Template 05 — Aliases, Variable Templates & Lambdas

> **Goal:** Master template aliases, variable templates, generic lambdas,
> `if constexpr`, and C++20 template lambdas. Understand how these features
> simplify template code and reduce boilerplate.

---

## Table of Contents

1. [Template Aliases](#1-template-aliases)
2. [Variable Templates](#2-variable-templates)
3. [Generic Lambdas (C++14)](#3-generic-lambdas-c14)
4. [`if constexpr` (C++17)](#4-if-constexpr-c17)
5. [Template Lambdas (C++20)](#5-template-lambdas-c20)
6. [Putting It All Together](#6-putting-it-all-together)
7. [Exercises](#7-exercises)

---

## 1. Template Aliases

### The problem: verbose template types

```cpp
std::map<std::string, std::vector<std::pair<int, double>>> data;
// This is painful to type and read
```

### The solution: `using` alias templates

```cpp
template<typename T>
using Vec = std::vector<T>;

template<typename K, typename V>
using Map = std::map<K, V>;

template<typename T>
using StringMap = std::map<std::string, T>;

Vec<int> numbers;              // std::vector<int>
Map<std::string, int> ages;    // std::map<std::string, int>
StringMap<double> scores;      // std::map<std::string, double>
```

### Why `using` instead of `typedef`?

`typedef` **cannot** create alias templates:

```cpp
// ❌ typedef can't do this:
// typedef std::vector<T> Vec<T>;  // syntax error!

// ✅ using can:
template<typename T>
using Vec = std::vector<T>;
```

### When to use alias templates

| Use case | Example |
|----------|---------|
| Simplify long types | `using Vec = std::vector<T>` |
| Partial specialization shortcuts | `using StringMap = std::map<std::string, T>` |
| Type trait shortcuts | `using remove_ref_t = typename remove_reference<T>::type` |
| Platform abstraction | `using Handle = platform_specific_type<T>` |

### Standard library examples

The `_t` suffix convention in `<type_traits>` is exactly this pattern:

```cpp
// These are just alias templates!
template<bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template<typename T>
using remove_const_t = typename remove_const<T>::type;

template<typename T>
using decay_t = typename decay<T>::type;
```

### Important rule: alias templates cannot be specialized

```cpp
template<typename T>
using Vec = std::vector<T>;

// ❌ Can't do this:
// template<>
// using Vec<bool> = std::vector<char>;  // not allowed!

// ✅ But you CAN alias a specializable template:
template<typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
```

---

## 2. Variable Templates

### What are they? (C++14)

A variable whose type or value depends on a template parameter:

```cpp
template<typename T>
constexpr T pi = T(3.14159265358979323846);

double d = pi<double>;   // 3.14159265358979323846
float f = pi<float>;     // 3.14159274f (float precision)
```

### Why do variable templates exist?

Before C++14, you needed workarounds:

```cpp
// Old way 1: constexpr function
template<typename T>
constexpr T pi() { return T(3.14159265358979323846); }
double d = pi<double>();  // extra parentheses needed

// Old way 2: static member of a class
template<typename T>
struct constants { static constexpr T pi = T(3.14159265358979323846); };
double d = constants<double>::pi;  // verbose

// New way: variable template (clean!)
template<typename T>
constexpr T pi = T(3.14159265358979323846);
double d = pi<double>;  // simple!
```

### Standard library `_v` suffix

The `_v` suffix convention is variable templates:

```cpp
// Old: access ::value
std::is_integral<int>::value         // true

// New: variable template (C++17)
std::is_integral_v<int>              // true

// Implementation:
template<typename T>
inline constexpr bool is_integral_v = is_integral<T>::value;
```

### Non-type variable templates

```cpp
template<int N>
constexpr int factorial = N * factorial<N - 1>;

template<>
constexpr int factorial<0> = 1;

static_assert(factorial<5> == 120);
static_assert(factorial<0> == 1);
```

---

## 3. Generic Lambdas (C++14)

### What are they?

A lambda with `auto` parameters is a **template in disguise**:

```cpp
auto add = [](auto a, auto b) { return a + b; };
// Equivalent to:
// struct __lambda {
//     template<typename T, typename U>
//     auto operator()(T a, U b) const { return a + b; }
// };

add(1, 2);       // int + int = int
add(1.5, 2.5);   // double + double = double
add(1, 2.5);     // int + double = double
```

### Why use generic lambdas?

- **Less boilerplate**: No need to write full template functions for simple operations
- **Perfect for algorithms**: STL algorithms accept callables

```cpp
std::vector<int> v = {3, 1, 4, 1, 5};

// Verbose function template
// template<typename T> bool greater(T a, T b) { return a > b; }

// Clean generic lambda
std::sort(v.begin(), v.end(), [](auto a, auto b) { return a > b; });
```

### Each `auto` is an independent template parameter

```cpp
auto f = [](auto a, auto b) { return a + b; };
// a and b can be DIFFERENT types:
f(1, 2.0);        // a is int, b is double
f("hi"s, "!"s);   // a and b are std::string
```

### Generic lambdas with perfect forwarding

```cpp
auto wrapper = [](auto&& func, auto&&... args) {
    return std::forward<decltype(func)>(func)(
        std::forward<decltype(args)>(args)...
    );
};
```

---

## 4. `if constexpr` (C++17)

### The problem with regular `if`

```cpp
template<typename T>
void process(T val) {
    if (std::is_integral_v<T>)
        std::cout << val % 2;    // ❌ Error if T is double! % not defined
    else
        std::cout << std::fixed << val;
}
```

Even though the `if` branch won't execute for `double`, the compiler still
**type-checks both branches** for every T. This causes a compilation error.

### `if constexpr` discards the false branch

```cpp
template<typename T>
void process(T val) {
    if constexpr (std::is_integral_v<T>)
        std::cout << val % 2;    // ✅ Not compiled when T is double
    else
        std::cout << std::fixed << val;
}
```

`if constexpr` evaluates the condition at **compile time**. The false branch
is completely **discarded** — it doesn't even need to be valid C++ for that T.

### When to use `if constexpr`

| Scenario | Before (SFINAE) | After (`if constexpr`) |
|----------|-----------------|----------------------|
| Type dispatch | Multiple overloads + `enable_if` | Single function + `if constexpr` |
| Pack processing | `sizeof...(args) > 0` check | `if constexpr (sizeof...(args) > 0)` |
| Platform code | Preprocessor `#ifdef` | `if constexpr (is_windows)` |

### `if constexpr` with type traits

```cpp
template<typename T>
std::string to_string_helper(const T& val) {
    if constexpr (std::is_arithmetic_v<T>)
        return std::to_string(val);
    else if constexpr (std::is_same_v<T, std::string>)
        return val;
    else if constexpr (std::is_same_v<T, const char*>)
        return std::string(val);
    else
        return "unknown";
}
```

### `if constexpr` replaces SFINAE in many cases

```cpp
// SFINAE version — complex
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void foo(T x) { /* integer path */ }

template<typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
void foo(T x) { /* other path */ }

// if constexpr version — simple!
template<typename T>
void foo(T x) {
    if constexpr (std::is_integral_v<T>)
        { /* integer path */ }
    else
        { /* other path */ }
}
```

---

## 5. Template Lambdas (C++20)

### What's new?

C++20 lets you write explicit template parameter lists in lambdas:

```cpp
// C++14: auto — each param is independent
auto f = [](auto a, auto b) { return a + b; };
// a and b can be different types

// C++20: explicit template — constrain to same type
auto g = []<typename T>(T a, T b) { return a + b; };
// a and b MUST be the same type
g(1, 2);     // ✅ OK: both int
// g(1, 2.0); // ❌ Error: int vs double
```

### Why template lambdas?

1. **Force same-type parameters** (can't do with `auto`)
2. **Access the type inside the lambda** (no need for `decltype`)
3. **Use concepts directly**

```cpp
// Access type T directly inside lambda body
auto print_vec = []<typename T>(const std::vector<T>& v) {
    std::cout << "Vector of " << typeid(T).name() << ": ";
    for (const auto& x : v) std::cout << x << " ";
    std::cout << "\n";
};

// With concepts
auto add = []<std::integral T>(T a, T b) { return a + b; };
```

### Template lambdas with non-type parameters

```cpp
auto repeat = []<int N>(auto value) {
    for (int i = 0; i < N; ++i)
        std::cout << value << " ";
    std::cout << "\n";
};

repeat.operator()<3>("hello");  // "hello hello hello\n"
```

---

## 6. Putting It All Together

### Combining features

```cpp
// Alias template for readability
template<typename T>
using NumericVector = std::vector<T>;

// Variable template for constants
template<typename T>
constexpr T epsilon = T(1e-10);

// Generic lambda with if constexpr
auto compare = [](auto a, auto b) {
    using T = std::common_type_t<decltype(a), decltype(b)>;
    if constexpr (std::is_floating_point_v<T>)
        return std::abs(T(a) - T(b)) < epsilon<T>;
    else
        return a == b;
};

compare(1, 1);           // true (integer comparison)
compare(0.1 + 0.2, 0.3); // true (epsilon comparison)
```

---

## 7. Exercises

See `exercises.cpp`.

---

**Next lecture:** Concepts — Defining & Using.
