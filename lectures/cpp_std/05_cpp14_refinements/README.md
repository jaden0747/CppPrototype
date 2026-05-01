# Lecture 05 — C++14 Refinements

> **Goal:** Master the quality-of-life improvements in C++14 — generic lambdas,
> relaxed constexpr, `make_unique`, variable templates, and return type deduction.
> Understand **why** each feature was added and **how** it improves on C++11.

---

## Table of Contents

1. [Generic Lambdas (`auto` parameters)](#1-generic-lambdas)
2. [Relaxed `constexpr`](#2-relaxed-constexpr)
3. [`std::make_unique`](#3-stdmake_unique)
4. [Variable Templates](#4-variable-templates)
5. [Return Type Deduction](#5-return-type-deduction)
6. [Binary Literals & Digit Separators](#6-binary-literals--digit-separators)
7. [`[[deprecated]]` Attribute](#7-deprecated-attribute)
8. [Lambda Capture Enhancements](#8-lambda-capture-enhancements)
9. [Exercises](#9-exercises)

---

## 1. Generic Lambdas

### What changed?

In C++11, every lambda parameter needed an explicit type. C++14 allows `auto`:

```cpp
// C++11 — must specify exact types
auto add = [](int a, int b) { return a + b; };

// C++14 — auto makes it generic!
auto add = [](auto a, auto b) { return a + b; };
add(1, 2);       // int + int
add(1.5, 2.5);   // double + double
add(std::string("hello"), std::string(" world"));  // string concat
```

### Why does this matter?

Generic lambdas are **template functions in disguise**. Under the hood,
the compiler generates:

```cpp
struct __lambda {
    template<typename A, typename B>
    auto operator()(A a, B b) const { return a + b; }
};
```

This means you get **template power** without the verbose `template<typename...>`
syntax. This is huge for STL algorithms:

```cpp
std::vector<int> v = {3, 1, 4, 1, 5};

// C++11: must write the full type or a named function
std::sort(v.begin(), v.end(), [](int a, int b) { return a > b; });

// C++14: auto makes it reusable with ANY type
auto descending = [](auto a, auto b) { return a > b; };
std::sort(v.begin(), v.end(), descending);  // works with int, double, string...
```

### Each `auto` is independent

```cpp
auto f = [](auto a, auto b) { return a + b; };
// a and b can be DIFFERENT types:
f(1, 2.5);      // int + double → double
f("hi"s, "!"s); // string + string → string
```

### Perfect-forwarding lambda

```cpp
auto wrapper = [](auto&&... args) {
    return realFunction(std::forward<decltype(args)>(args)...);
};
// This single lambda can forward ANY number of arguments of ANY type
// with perfect value-category preservation
```

### When to use generic lambdas

| Use case | Example |
|----------|---------|
| STL algorithms | `std::sort(v.begin(), v.end(), [](auto a, auto b) {...})` |
| Callbacks | `button.on_click([](auto event) {...})` |
| Wrappers | `auto log = [](auto&& fn) { /*...*/ fn(); /*...*/ }` |
| Higher-order functions | `auto compose = [](auto f, auto g) { return [=](auto x) { return f(g(x)); }; }` |

---

## 2. Relaxed `constexpr`

### The problem in C++11

C++11 `constexpr` functions were extremely limited:
- Only a single `return` statement
- No local variables
- No loops (`for`, `while`)
- No `if`/`else` (only ternary `?:`)

```cpp
// C++11: awkward recursive one-liner
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);  // single return + ternary
}
```

### C++14 removes most restrictions

Now you can write **normal-looking code** that runs at compile time:

```cpp
// C++14: clean iterative implementation
constexpr int factorial(int n) {
    int result = 1;              // ✅ local variables
    for (int i = 2; i <= n; ++i) // ✅ loops
        result *= i;
    return result;               // ✅ multiple statements
}
static_assert(factorial(10) == 3628800, "");
```

### What's now allowed in C++14 constexpr?

| Feature | C++11 | C++14 |
|---------|-------|-------|
| Local variables | ❌ | ✅ |
| `if`/`else` | ❌ | ✅ |
| `for`/`while` loops | ❌ | ✅ |
| Multiple `return` statements | ❌ | ✅ |
| Modify local variables | ❌ | ✅ |
| Call non-constexpr functions | ❌ | ❌ (still forbidden) |
| Dynamic allocation | ❌ | ❌ (added in C++20) |

### Why does this matter?

Relaxed constexpr makes compile-time computation **practical**. You can now
write real algorithms that execute at compile time:

```cpp
constexpr bool is_prime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; ++i)
        if (n % i == 0) return false;
    return true;
}

static_assert(is_prime(97));
static_assert(!is_prime(100));

// Compile-time lookup table generation!
constexpr auto build_prime_table() {
    std::array<int, 25> primes{};
    int count = 0;
    for (int n = 2; count < 25; ++n)
        if (is_prime(n)) primes[count++] = n;
    return primes;
}

constexpr auto primes = build_prime_table();
// primes = {2, 3, 5, 7, 11, 13, ...} — computed at compile time!
```

---

## 3. `std::make_unique`

### The gap in C++11

C++11 had `std::make_shared` but no `std::make_unique`. This meant you
still had to write `new`:

```cpp
// C++11: raw new required
auto p = std::unique_ptr<Widget>(new Widget(42, "hello"));
```

### C++14 adds the missing function

```cpp
auto p = std::make_unique<Widget>(42, "hello");
```

### Why `make_unique` matters (exception safety)

```cpp
// DANGEROUS (C++11):
foo(std::unique_ptr<A>(new A), std::unique_ptr<B>(new B));
// The compiler can:
//   1. new A
//   2. new B  ← if this throws, A leaks! unique_ptr not yet constructed
//   3. construct unique_ptr<A>
//   4. construct unique_ptr<B>

// SAFE (C++14):
foo(std::make_unique<A>(), std::make_unique<B>());
// Each make_unique is a single expression — no interleaving possible
```

### Array version

```cpp
auto arr = std::make_unique<int[]>(100);  // array of 100 value-initialized ints
arr[0] = 42;
```

### The no-raw-new rule

With `make_unique` and `make_shared`, you should **never** write `new` in
application code:

```cpp
// ❌ Old style
Widget* w = new Widget(42);
delete w;

// ❌ Better but still has new
auto w = std::unique_ptr<Widget>(new Widget(42));

// ✅ Best (C++14)
auto w = std::make_unique<Widget>(42);
```

---

## 4. Variable Templates

### What are they?

A variable whose type or value depends on a template parameter:

```cpp
template<typename T>
constexpr T pi = T(3.14159265358979323846L);

double area = pi<double> * r * r;   // 3.14159265358979...
float areaf = pi<float> * rf * rf;  // 3.14159274f (less precision)
```

### Before C++14 (workarounds)

```cpp
// Workaround 1: constexpr function (needs parentheses)
template<typename T>
constexpr T pi() { return T(3.14159265358979323846L); }
double area = pi<double>() * r * r;  // extra ()

// Workaround 2: struct static member (verbose)
template<typename T>
struct constants { static constexpr T pi = T(3.14159265358979323846L); };
double area = constants<double>::pi * r * r;  // verbose!

// C++14 variable template (clean!)
template<typename T>
constexpr T pi = T(3.14159265358979323846L);
double area = pi<double> * r * r;  // simple
```

### Creating `_v` trait shortcuts

The standard library added `_v` suffixes in C++17, but you can write your
own in C++14:

```cpp
template<typename T>
constexpr bool is_integral_v = std::is_integral<T>::value;

template<typename T, typename U>
constexpr bool is_same_v = std::is_same<T, U>::value;

// Usage:
if (is_integral_v<T>) { /* ... */ }
```

---

## 5. Return Type Deduction

### `auto` return type

Functions can deduce their return type from the `return` statement:

```cpp
auto multiply(int a, int b) {
    return a * b;  // deduced as int
}

auto make_pair_like(int a, double b) {
    return std::make_pair(a, b);  // deduced as std::pair<int, double>
}
```

### Rules for auto return

- All `return` statements must return the same type
- For recursive functions, at least one non-recursive return must come first

```cpp
auto fib(int n) -> int {  // trailing return helps with recursion
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}
```

### `decltype(auto)` — preserving references

`auto` strips references. `decltype(auto)` preserves them:

```cpp
// auto strips references:
auto get_value() {
    static int x = 42;
    return x;  // returns int (copy)
}

// decltype(auto) preserves:
decltype(auto) get_ref() {
    static int x = 42;
    return (x);  // returns int& (reference!) because (x) is an lvalue
}

int& r = get_ref();
r = 100;  // modifies the static x!
```

### When to use which

| Return type | Behavior | Use when |
|-------------|----------|----------|
| `auto` | Strips refs and const | Returning by value (most common) |
| `decltype(auto)` | Preserves exact type | Forwarding return values in wrappers |
| Explicit type | No deduction | When you want to document the API |

---

## 6. Binary Literals & Digit Separators

### Binary literals

```cpp
int mask = 0b1111'0000;       // 240
int flags = 0b0000'0001;      // 1
int pattern = 0b1010'1010;    // 170
```

### Digit separators

The `'` character can appear anywhere in a number for readability:

```cpp
long population = 7'900'000'000;     // 7.9 billion
double avogadro = 6.022'140'76e23;   // Avogadro's number
int hex_color = 0xFF'00'FF;          // magenta
int binary = 0b0001'0010'0011'0100;  // bit pattern
```

---

## 7. `[[deprecated]]` Attribute

Mark functions, types, or variables as deprecated with a message:

```cpp
[[deprecated("Use newFunction() instead")]]
void oldFunction() { /* ... */ }

[[deprecated("Use std::string_view instead")]]
using CString = const char*;

struct [[deprecated("Use NewWidget")]] OldWidget { };

oldFunction();   // Compiler WARNING: 'oldFunction' is deprecated
```

This is a standard replacement for compiler-specific `__attribute__((deprecated))`.

---

## 8. Lambda Capture Enhancements

### Init captures (generalized captures)

C++14 lets you create new variables in the capture list:

```cpp
// Move a unique_ptr into a lambda
auto ptr = std::make_unique<Widget>(42);
auto lambda = [p = std::move(ptr)] {
    p->do_something();  // ptr is now owned by the lambda
};

// Create a new variable
auto counter = [count = 0]() mutable {
    return ++count;
};
counter();  // 1
counter();  // 2
counter();  // 3
```

### Why init captures matter

Before C++14, you couldn't move-capture. This was a serious limitation for
move-only types like `unique_ptr`:

```cpp
// C++11: IMPOSSIBLE to capture unique_ptr by move
auto ptr = std::make_unique<int>(42);
// auto f = [ptr] { ... };  // ❌ unique_ptr is not copyable!

// C++14: init capture enables move
auto f = [p = std::move(ptr)] { return *p; };  // ✅
```

---

## 9. Exercises

See `exercises.cpp`:

1. Write generic lambdas that work with multiple types
2. Implement constexpr sorting (bubble sort) using relaxed constexpr
3. Refactor raw `new` calls to use `make_unique`
4. Create variable templates for mathematical constants
5. Use return type deduction with `decltype(auto)`
6. Use init captures to move a `unique_ptr` into a lambda

---

**Next lecture:** C++17 Ergonomics — structured bindings, if-init, class
template argument deduction, and fold expressions.
