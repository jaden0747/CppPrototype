# Lecture 02 — Functions & Lambdas (C++11)

> **Goal:** Master lambdas, function objects, `= default` / `= delete`,
> delegating constructors, and type aliases. After this lecture you'll write
> expressive algorithms without ever needing hand-rolled functor classes.

---

## Table of Contents

1. [Lambda Expressions](#1-lambda-expressions)
2. [Captures In Depth](#2-captures-in-depth)
3. [`= default` and `= delete`](#3--default-and--delete)
4. [Delegating Constructors](#4-delegating-constructors)
5. [Type Aliases with `using`](#5-type-aliases-with-using)
6. [Exercises](#6-exercises)
7. [Capstone Mini-Project](#7-capstone-mini-project)
8. [Further Reading & Suggestions](#8-further-reading--suggestions)

---

## 1. Lambda Expressions

### What is a lambda?

A lambda is an **anonymous function object** you can define inline — right where
you need it. Under the hood the compiler generates a class with an `operator()`.

### Syntax

```
[captures](parameters) -> return_type { body }
```

- **captures** — which outside variables the lambda can see
- **parameters** — just like a normal function
- **return_type** — optional; deduced if omitted
- **body** — the code to execute

### Minimal examples

```cpp
// No captures, no params
auto hello = []{ std::cout << "hello\n"; };
hello();  // prints "hello"

// With parameters
auto add = [](int a, int b){ return a + b; };
std::cout << add(3, 4);  // 7

// Used inline with STL algorithms
std::vector<int> v{5, 2, 8, 1, 9};
std::sort(v.begin(), v.end(), [](int a, int b){ return a > b; });
// v is now {9, 8, 5, 2, 1}
```

### Why lambdas beat old-style functors

Before C++11 you wrote a **whole class** just to pass custom logic to an algorithm:

```cpp
// Old style — verbose, far from usage point
struct GreaterThan {
    int threshold;
    bool operator()(int x) const { return x > threshold; }
};
auto it = std::find_if(v.begin(), v.end(), GreaterThan{3});
```

Now:

```cpp
auto it = std::find_if(v.begin(), v.end(), [](int x){ return x > 3; });
```

One line, same performance, no boilerplate.

---

## 2. Captures In Depth

### Capture modes

| Syntax | Meaning |
|--------|---------|
| `[x]` | Capture `x` by value (copy) |
| `[&x]` | Capture `x` by reference |
| `[=]` | Capture everything used by value |
| `[&]` | Capture everything used by reference |
| `[=, &x]` | Everything by value, except `x` by reference |
| `[&, x]` | Everything by reference, except `x` by value |
| `[this]` | Capture the current object's `this` pointer |

### Lifetime warning

If a lambda captures by reference and **outlives** the referenced variable,
you get **dangling references** — undefined behaviour.

```cpp
std::function<int()> make_counter()
{
    int count = 0;
    return [&count]{ return ++count; };  // BUG! count dies when function returns
}
```

Fix: capture by value, and use `mutable` if you need to modify:

```cpp
std::function<int()> make_counter()
{
    int count = 0;
    return [count]() mutable { return ++count; };  // OK — own copy
}
```

### `mutable` lambdas

By default, the captured-by-value variables are `const` inside the lambda body.
Adding `mutable` removes that restriction:

```cpp
int x = 0;
auto inc = [x]() mutable { return ++x; };  // modifies local copy of x
inc();  // returns 1
inc();  // returns 2
// x is still 0 outside!
```

### Generic lambdas (C++14 preview)

In C++14 you can use `auto` in lambda parameters:

```cpp
auto print = [](const auto& val){ std::cout << val << "\n"; };
print(42);       // int
print("hello");  // const char*
```

---

## 3. `= default` and `= delete`

### The problem

The compiler automatically generates:
- Default constructor
- Copy constructor
- Copy assignment operator
- Destructor
- (C++11) Move constructor, Move assignment operator

Sometimes you want to **explicitly say** "yes generate it" or "no, forbid it".

### `= default`

```cpp
struct Widget {
    Widget() = default;                   // Compiler generates default ctor
    Widget(const Widget&) = default;      // Compiler generates copy ctor
    Widget& operator=(const Widget&) = default;
    ~Widget() = default;
};
```

Why bother? Clarity. And sometimes adding ANY user-declared constructor
suppresses the default one — `= default` brings it back.

### `= delete`

```cpp
struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;             // No copying
    NonCopyable& operator=(const NonCopyable&) = delete;  // No copy-assign
};
```

You can also delete specific overloads:

```cpp
void processInt(int x);
void processInt(double) = delete;  // Compile error if someone passes a double
```

### Rule of Five (briefly)

If you define ANY of: destructor, copy ctor, copy assign, move ctor, move
assign — you should define (or `= default` / `= delete`) ALL FIVE.
This is the **Rule of Five** and prevents subtle bugs.

---

## 4. Delegating Constructors

### The problem

Before C++11, if you had 4 constructors that all did common initialization,
you'd either duplicate code or call a private `init()` helper.

### The solution

One constructor can delegate to another:

```cpp
class Connection {
public:
    Connection(const std::string& host, int port, int timeout)
        : host_(host), port_(port), timeout_(timeout) { connect(); }

    // Delegate to the 3-arg ctor with a default timeout
    Connection(const std::string& host, int port)
        : Connection(host, port, 30) {}

    // Delegate with all defaults
    Connection()
        : Connection("localhost", 8080) {}

private:
    void connect() { /* ... */ }
    std::string host_;
    int port_;
    int timeout_;
};
```

### Rules

- The delegating ctor's member-initializer-list must contain ONLY the
  delegation — no other members can be initialized there.
- The target ctor runs completely before the delegating ctor's body.
- Circular delegation is undefined behaviour.

---

## 5. Type Aliases with `using`

### Old-style `typedef`

```cpp
typedef std::vector<std::pair<std::string, int>> Scoreboard;
```

### New-style `using` (C++11)

```cpp
using Scoreboard = std::vector<std::pair<std::string, int>>;
```

Same semantics, but **far more readable** — especially for templates:

```cpp
// Template alias — impossible with typedef!
template<typename T>
using Vec = std::vector<T>;

Vec<int> numbers;    // std::vector<int>
Vec<std::string> names;  // std::vector<std::string>
```

### Function pointer aliases

```cpp
// Old:
typedef void (*Callback)(int, const std::string&);

// New — MUCH clearer:
using Callback = void(*)(int, const std::string&);
```

### Best practices

- Prefer `using` over `typedef` in all new code.
- Template aliases are a superpower for reducing template soup.
- Don't alias too aggressively — `using Integer = int;` helps nobody.

---

## 6. Exercises

See `exercises.cpp` in this folder. Topics covered:

1. Write lambdas for STL algorithms (sort, find_if, count_if)
2. Capture semantics — predict output of capture scenarios
3. Design a class with `= delete` to prevent unwanted conversions
4. Use delegating constructors to DRY up a multi-constructor class
5. Create template aliases to simplify complex type declarations
6. Challenge: Build a mini event system using `std::function`

---

## 7. Capstone Mini-Project

**"Configurable Filter Pipeline"**

Build a pipeline that processes a vector of integers through a chain of
user-defined filters. Requirements:

- Each filter is a `std::function<bool(int)>` (lambda)
- A `Pipeline` class stores filters and has `.add_filter(...)` and `.run(data)`
- Use `= delete` to prevent copying the pipeline (it owns callbacks)
- Use delegating constructors for convenience overloads
- Use `using` to alias the filter and result types

Expected output:
```
Input:  1 2 3 4 5 6 7 8 9 10
After [even && > 4]: 6 8 10
```

---

## 8. Further Reading & Suggestions

| Topic | Resource |
|-------|----------|
| Lambda internals | *C++ Templates: The Complete Guide* ch. 11 |
| `std::function` overhead | Jason Turner's *C++ Weekly* ep. 144 |
| Rule of Five | cppreference.com/w/cpp/language/rule_of_three |
| `= delete` tricks | Abseil Tips #49 |

**Next lecture:** Move Semantics & Smart Pointers — where we learn why
`= delete` on copy operations is so common, and how to transfer ownership
without copying.
