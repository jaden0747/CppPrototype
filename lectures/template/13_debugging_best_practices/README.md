# Template 13 — Debugging & Best Practices

> **Goal:** Practical skills for working with templates in real projects —
> reading error messages, reducing compile times, testing templates, and
> knowing when NOT to use them. This lecture is about **surviving templates
> in production code**.

---

## Table of Contents

1. [Reading Template Error Messages](#1-reading-template-error-messages)
2. [Reducing Compile Times](#2-reducing-compile-times)
3. [Testing Templates](#3-testing-templates)
4. [When NOT to Use Templates](#4-when-not-to-use-templates)
5. [Common Template Bugs](#5-common-template-bugs)
6. [Best Practices Summary](#6-best-practices-summary)
7. [Exercises](#7-exercises)

---

## 1. Reading Template Error Messages

### Why are template errors so bad?

When template instantiation fails, the compiler shows the **entire chain**
from the call site down to the actual failure. A simple mistake can produce
hundreds of lines of errors.

### Strategy: Read from the bottom up

```
/usr/include/c++/13/bits/stl_algo.h:4860:14: error:
    no match for 'operator<' (operand types are 'MyClass' and 'MyClass')
        ...
In file included from /usr/include/c++/13/algorithm:62:
note: required from 'void std::sort(_RAIter, _RAIter) [with _RAIter = ...]'
note: required from here
        std::sort(v.begin(), v.end());
```

1. **Bottom**: Your code — `std::sort(v.begin(), v.end())`
2. **Middle**: The template instantiation chain (skip this)
3. **Top**: The actual error — `no match for 'operator<'`
4. **Fix**: Add `operator<` to `MyClass`

### Before concepts (C++17 and earlier)

```
error: no matching function for call to 'sort'
note: candidate template ignored: substitution failure
    [with T = MyClass]: no member named 'operator<' in 'MyClass'
```

This tells you WHAT failed but not WHY the constraint exists.

### After concepts (C++20)

```
error: 'MyClass' does not satisfy 'totally_ordered'
note: because 'a < b' would be invalid: no match for 'operator<'
```

Much better! The concept name tells you the **intent**.

### Debugging techniques

| Technique | How |
|-----------|-----|
| `static_assert` with message | `static_assert(std::is_integral_v<T>, "T must be integral")` |
| Instantiate explicitly | `template class MyTemplate<int>;` to force errors early |
| Simplify the type | Replace `std::map<std::string, std::vector<int>>` with `int` to isolate |
| Use concepts (C++20) | Constrain templates for clearer errors |
| Print types at compile time | `template<typename T> struct TD; TD<decltype(x)> td;` (intentional error shows type) |

### The "type displayer" trick

```cpp
// Intentionally undefined — causes error that SHOWS the type
template<typename T> struct TypeDisplayer;

// Usage:
auto x = some_complex_expression();
TypeDisplayer<decltype(x)> td;
// Error: implicit instantiation of undefined template
//        'TypeDisplayer<std::vector<std::pair<int, double>>>'
// Now you know the exact type!
```

### Compiler flags for better template diagnostics

| Flag | Compiler | What it does |
|------|----------|-------------|
| `-ftemplate-backtrace-limit=0` | GCC/Clang | Show full instantiation chain |
| `-ftemplate-backtrace-limit=5` | GCC/Clang | Limit chain to 5 levels |
| `-fconcepts-diagnostics-depth=2` | GCC | Show deeper concept failure info |
| `-fdiagnostics-show-template-tree` | Clang | Tree view of template diffs |
| `-Wno-error` | All | Don't turn warnings into errors during debugging |

---

## 2. Reducing Compile Times

### Why do templates slow down compilation?

- **Header-only**: Templates must be in headers → every translation unit
  re-parses and re-instantiates them
- **Deep instantiation**: `vector<map<string, vector<int>>>` causes
  cascading instantiations
- **Combinatorial explosion**: N types × M functions = N×M instantiations

### Technique 1: Extern templates

Prevent redundant instantiations across translation units:

```cpp
// widget.h
template<typename T>
class Widget { /* ... */ };

extern template class Widget<int>;     // declare: "don't instantiate here"
extern template class Widget<double>;

// widget.cpp
template class Widget<int>;            // define: instantiate ONCE here
template class Widget<double>;
```

### Technique 2: Forward declarations

```cpp
// ❌ Slow: includes entire header
#include <vector>
#include <string>
#include <map>

// ✅ Fast: forward declare when you only need a pointer/reference
class Widget;  // forward declare
void process(Widget& w);  // only needs declaration, not definition
```

### Technique 3: Precompiled headers (PCH)

```cmake
# CMake
target_precompile_headers(my_target PRIVATE
    <vector>
    <string>
    <map>
    <algorithm>
    <memory>
)
```

### Technique 4: Reduce template depth

```cpp
// ❌ Deep recursion: N instantiations for Fibonacci<N>
template<int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N-1>::value + Fibonacci<N-2>::value;
};

// ✅ Flat: just one constexpr function
constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) { auto t = a + b; a = b; b = t; }
    return b;
}
```

### Technique 5: PIMPL for template-heavy classes

```cpp
// widget.h — minimal header, no templates exposed
class Widget {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    Widget();
    ~Widget();
    void do_thing();
};

// widget.cpp — all template-heavy code isolated here
#include <vector>
#include <map>
#include <algorithm>

struct Widget::Impl {
    std::map<std::string, std::vector<int>> data_;
    // ... template-heavy implementation
};
```

### Measuring compile times

| Tool | Command | What it shows |
|------|---------|-------------|
| GCC time report | `g++ -ftime-report` | Time per compilation phase |
| Clang time trace | `clang++ -ftime-trace` | JSON flame chart |
| Templight | `templight++` | Template instantiation profiler |
| Ninja | `ninja -j1` | Per-file compile times |

---

## 3. Testing Templates

### Strategy: test with multiple types

A template that works with `int` might break with `std::string`:

```cpp
// Test with at least:
// - int (trivial, fast)
// - std::string (non-trivial, has allocations)
// - double (floating-point edge cases)
// - A custom type (to test with minimal interface)

template<typename T>
void test_stack() {
    Stack<T> s;
    s.push(T{});
    ASSERT(!s.empty());
    s.pop();
    ASSERT(s.empty());
}

test_stack<int>();
test_stack<std::string>();
test_stack<double>();
test_stack<NonCopyable>();  // test move-only types too
```

### Compile-time tests with `static_assert`

```cpp
// Test type traits
static_assert(std::is_same_v<decltype(add(1, 2)), int>);
static_assert(std::is_same_v<decltype(add(1.0, 2.0)), double>);

// Test concepts
static_assert(Numeric<int>);
static_assert(Numeric<double>);
static_assert(!Numeric<std::string>);
static_assert(!Numeric<void*>);

// Test type transformations
static_assert(std::is_same_v<RemoveAllPointers<int***>::type, int>);
static_assert(std::is_same_v<Head<TypeList<int, double>>::type, int>);
```

### Negative tests: verify that invalid code DOESN'T compile

With concepts:

```cpp
// Test that non-numeric types are rejected
template<typename T>
concept CanCallAdd = requires(T a, T b) { add(a, b); };

static_assert(!CanCallAdd<std::string>);  // should NOT be callable with string
```

### Edge-case types to test with

| Type | What it tests |
|------|-------------|
| `int` | Basic arithmetic type |
| `std::string` | Non-trivial type with allocations |
| `std::unique_ptr<int>` | Move-only type |
| `const int` | Const-correctness |
| `int&` | Reference handling |
| Empty struct | Zero-size type |
| Large struct (>1KB) | Copy cost, alignment |
| Non-default-constructible | Constructor requirements |

---

## 4. When NOT to Use Templates

### Use templates when...

| Scenario | Why templates help |
|----------|-------------------|
| Algorithm works for many types | Write once, works for all |
| Zero-overhead abstraction | No runtime cost |
| Type safety at compile time | Catch errors before running |
| Performance-critical code | Inlining, no virtual dispatch |
| Library with many users | Flexibility for users |

### Avoid templates when...

| Scenario | Why templates hurt |
|----------|-------------------|
| Only 1-2 concrete types needed | Just write the concrete function |
| Runtime polymorphism is fine | Virtual is simpler and sufficient |
| Compile times matter more than runtime | Templates bloat compilation |
| Code simplicity is priority | Templates add complexity |
| Building a quick prototype | Move fast, refactor later |

### Signs of over-templating

1. **Every function is a template** — even when only used with one type
2. **Error messages are incomprehensible** — for you AND your teammates
3. **Compile times are unreasonable** — minutes for small changes
4. **Code is harder to read than the problem** — template syntax noise
5. **Only you can maintain it** — teammates avoid your code

### The rule of thumb

> If you wouldn't write a virtual base class for it, you probably don't
> need a template either. Start concrete, generalize when you have
> evidence of reuse.

---

## 5. Common Template Bugs

### 1. Missing `typename` / `template` keywords

```cpp
template<typename T>
void f() {
    typename T::value_type x;           // ✅ need typename for dependent types
    T::template nested_template<int>(); // ✅ need template for dependent templates
}
```

### 2. Two-phase lookup surprises

```cpp
template<typename T>
struct Base { void foo() {} };

template<typename T>
struct Derived : Base<T> {
    void bar() {
        foo();            // ❌ Error! foo() is not found in phase 1
        this->foo();      // ✅ OK: this-> defers to phase 2
        Base<T>::foo();   // ✅ OK: explicit qualification
    }
};
```

### 3. ODR violations with templates in headers

```cpp
// In header (OK — templates are implicitly inline)
template<typename T>
void f(T x) { }  // ✅ OK in header

// In header (BAD — non-template function)
void g(int x) { }  // ❌ ODR violation if header included in multiple TUs

// Fix: mark it inline
inline void g(int x) { }  // ✅ OK
```

### 4. Forgetting to instantiate

```cpp
// header.h
template<typename T>
class Widget {
    void foo();  // declared
};

// widget.cpp
template<typename T>
void Widget<T>::foo() { /* ... */ }  // defined

// main.cpp
Widget<int> w;
w.foo();  // ❌ Linker error! Widget<int>::foo() not instantiated in widget.cpp

// Fix: either put implementation in header, or explicitly instantiate:
// widget.cpp:
template class Widget<int>;
```

---

## 6. Best Practices Summary

### The top 10

1. **Use concepts** over SFINAE where possible (C++20)
2. **Use `if constexpr`** over tag dispatch / enable_if (C++17)
3. **Use `auto` return types** to reduce verbosity (C++14)
4. **Prefer function overloading** over function template specialization
5. **Use `static_assert`** for early, clear error messages
6. **Limit template depth** — prefer flat over recursive
7. **Document constraints** — state what types are expected
8. **Test with multiple types** — don't assume one test covers all
9. **Measure compile times** — templates can silently bloat builds
10. **Keep it simple** — the best template code is the simplest

### Quick reference: which technique for which C++ version

| Task | C++14 | C++17 | C++20 |
|------|-------|-------|-------|
| Constrain templates | `enable_if` | `enable_if` / `if constexpr` | Concepts |
| Type dispatch | Tag dispatch / SFINAE | `if constexpr` | Concepts |
| Generic callables | Generic lambdas | Generic lambdas | Template lambdas |
| Value computation | TMP / constexpr | `constexpr` | `consteval` / `constexpr` |
| Type computation | TMP | TMP | TMP (still!) |

---

## 7. Exercises

See `exercises.cpp`.

---

**Congratulations!** You've completed the C++ Templates lecture series.
