# Lecture 11 — C++20 Coroutines & Modules

> **Goal:** Understand C++20 coroutines (co_await, co_yield, co_return) and
> modules (import, export) — the two most transformative C++20 features.
> Learn **what** coroutines are, **why** they're useful, **how** the compiler
> transforms them, and **when** to use modules over headers.

---

## Table of Contents

1. [Coroutines Overview](#1-coroutines-overview)
2. [Generator Pattern (co_yield)](#2-generator-pattern-co_yield)
3. [Async Tasks (co_await / co_return)](#3-async-tasks-co_await--co_return)
4. [Promise Type Mechanics](#4-promise-type-mechanics)
5. [Awaitable Protocol](#5-awaitable-protocol)
6. [Modules](#6-modules)
7. [Exercises](#7-exercises)

---

## 1. Coroutines Overview

### What is a coroutine?

A coroutine is a function that can **suspend** its execution (saving its state)
and **resume** later from where it left off. Unlike regular functions that run
start-to-finish, coroutines can pause mid-execution and continue later.

### Mental model

Think of reading a book:
- **Regular function**: You must read the entire book in one sitting
- **Coroutine**: You can use a bookmark — stop reading, do something else,
  and pick up exactly where you left off

### The three keywords

| Keyword | What it does | Example |
|---------|-------------|---------|
| `co_yield expr` | Suspend, produce a value to the caller | Lazy generators |
| `co_await expr` | Suspend until an async operation completes | Async I/O |
| `co_return expr` | Final return value, coroutine is done | Task results |

**Key rule:** Any function containing `co_yield`, `co_await`, or `co_return`
is automatically treated as a coroutine by the compiler. You don't declare it
differently — the keywords make it so.

### Why coroutines? What problems do they solve?

| Problem | Old solution | Coroutine solution |
|---------|-------------|-------------------|
| Lazy sequences | Complex iterator classes | `co_yield` in a simple loop |
| Async I/O | Callbacks or future chains | `co_await` looks synchronous |
| State machines | Manual switch + state enum | Suspend/resume IS the state |
| Generator patterns | Allocate all values upfront | Generate one at a time |

### How does suspension work?

When a coroutine suspends:
1. Its local variables are saved in a **coroutine frame** (heap-allocated)
2. Control returns to the **caller** (or resumer)
3. The coroutine handle stores a pointer to the suspended frame
4. When resumed, execution continues from the exact suspension point

```
Caller          Coroutine
  |                |
  | --- call ----> |  (starts executing)
  |                |  ... does work ...
  | <-- suspend -- |  (co_yield / co_await)
  |  (caller has   |
  |   control)     |  (state preserved on heap)
  |                |
  | --- resume --> |  (continues from suspension point)
  |                |  ... does more work ...
  | <-- suspend -- |  (another co_yield)
  ...              ...
```

---

## 2. Generator Pattern (co_yield)

### The use case

You want to produce a sequence of values **lazily** — compute each value
only when the caller asks for it.

### Without coroutines: complex iterator boilerplate

```cpp
// To iterate fibonacci numbers, you'd need:
class FibIterator {
    int a_ = 0, b_ = 1;
    int current_;
    bool done_ = false;
public:
    FibIterator() : current_(a_) {}
    int operator*() const { return current_; }
    FibIterator& operator++() {
        auto next = a_ + b_;
        a_ = b_; b_ = next; current_ = a_;
        return *this;
    }
    // ... equality, sentinel, etc.
};
// Plus a range class wrapping begin/end...
```

### With coroutines: just write the algorithm

```cpp
Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;            // suspend, produce 'a'
        auto next = a + b;
        a = b;
        b = next;
    }
}

auto gen = fibonacci();
while (gen.next()) {
    std::cout << gen.value() << " ";  // 0 1 1 2 3 5 8 13 ...
    if (gen.value() > 100) break;
}
```

### Complete Generator implementation

```cpp
#include <coroutine>

template<typename T>
struct Generator {
    // The promise_type is REQUIRED — the compiler looks for it
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() { return {}; }
        // ↑ Lazy start: don't run until first next() call

        std::suspend_always final_suspend() noexcept { return {}; }
        // ↑ Suspend at end so handle.done() returns true

        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};  // suspend after storing value
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    // --- The generator object the caller holds ---
    std::coroutine_handle<promise_type> handle_;

    explicit Generator(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~Generator() { if (handle_) handle_.destroy(); }

    // Move-only (coroutine frames are unique resources)
    Generator(Generator&& other) noexcept
        : handle_(std::exchange(other.handle_, {})) {}
    Generator(const Generator&) = delete;

    bool next() {
        handle_.resume();       // resume the coroutine
        return !handle_.done(); // did it finish?
    }

    T value() const { return handle_.promise().current_value; }
};
```

### More generator examples

```cpp
// Infinite counter
Generator<int> iota(int start = 0) {
    while (true) co_yield start++;
}

// Range with step
Generator<int> range(int start, int end, int step = 1) {
    for (int i = start; i < end; i += step)
        co_yield i;
}

// Filter: only yield values matching predicate
Generator<int> filter(Generator<int> source, std::function<bool(int)> pred) {
    while (source.next()) {
        if (pred(source.value()))
            co_yield source.value();
    }
}
```

---

## 3. Async Tasks (co_await / co_return)

### The use case

You have asynchronous operations (network I/O, file I/O, timers) and want
to write code that **reads like synchronous** code but doesn't block.

### Without coroutines: callback hell

```cpp
void fetch_and_process(std::string url) {
    http_get(url, [](Response resp) {           // callback 1
        parse_json(resp.body, [](Json data) {   // callback 2
            save_to_db(data, [](bool ok) {      // callback 3
                if (ok) notify_user();
            });
        });
    });
}
```

### With coroutines: sequential-looking async

```cpp
Task<void> fetch_and_process(std::string url) {
    auto resp = co_await http_get(url);          // looks synchronous!
    auto data = co_await parse_json(resp.body);
    bool ok = co_await save_to_db(data);
    if (ok) notify_user();
}
```

The code reads top-to-bottom. Each `co_await` suspends until the operation
completes, then resumes with the result. No nesting, no callbacks.

### How `co_await` works step by step

```cpp
auto result = co_await some_async_operation();
```

The compiler transforms this into approximately:

```cpp
auto&& awaitable = some_async_operation();
if (!awaitable.await_ready()) {          // already done?
    awaitable.await_suspend(my_handle);  // no → suspend me, start the work
    // ... suspended here ...
    // ... something resumes us later ...
}
auto result = awaitable.await_resume();  // get the result
```

---

## 4. Promise Type Mechanics

### What is the promise type?

The compiler transforms your coroutine into a state machine. It needs to know:
- What to return to the caller
- What to do at start and end
- How to handle `co_yield`, `co_return`, and exceptions

All of this is defined by the **promise type** — a struct nested inside your
coroutine return type.

### The promise type interface

| Method | When it's called | What it controls |
|--------|-----------------|-----------------|
| `get_return_object()` | Before coroutine body starts | What the caller receives |
| `initial_suspend()` | Right after construction | Lazy (suspend_always) or eager (suspend_never) start |
| `final_suspend()` | After coroutine body ends | Cleanup behavior |
| `yield_value(T)` | At each `co_yield` | Store the yielded value |
| `return_value(T)` | At `co_return expr` | Store the final result |
| `return_void()` | At `co_return;` or falling off end | For void coroutines |
| `unhandled_exception()` | If exception escapes body | Store or rethrow |
| `await_transform(expr)` | Before each `co_await` | Customize awaiting behavior |

### `initial_suspend` choices

```cpp
// Lazy: caller must resume manually (Generator pattern)
std::suspend_always initial_suspend() { return {}; }

// Eager: starts running immediately (Task pattern)
std::suspend_never initial_suspend() { return {}; }
```

### `final_suspend` choices

```cpp
// Always suspend: allows checking done(), safe destruction
std::suspend_always final_suspend() noexcept { return {}; }

// Never suspend: auto-destroys frame (careful with handle lifetime!)
std::suspend_never final_suspend() noexcept { return {}; }
```

---

## 5. Awaitable Protocol

### What makes something awaitable?

Any object with these three methods:

```cpp
struct MyAwaiter {
    bool await_ready() noexcept;
    // "Is the result already available? (skip suspend if true)"

    void await_suspend(std::coroutine_handle<> h) noexcept;
    // "The coroutine is suspending. Schedule work and resume h when done."

    T await_resume() noexcept;
    // "The coroutine is resuming. Return the result."
};
```

### Simple example: async sleep

```cpp
struct SleepAwaiter {
    std::chrono::milliseconds duration;

    bool await_ready() const noexcept {
        return duration.count() <= 0;  // if 0ms, don't bother suspending
    }

    void await_suspend(std::coroutine_handle<> h) const {
        // Launch a thread that resumes us after the delay
        std::thread([h, d = duration] {
            std::this_thread::sleep_for(d);
            h.resume();  // wake up the coroutine
        }).detach();
    }

    void await_resume() const noexcept {}  // nothing to return
};

// Usage in a coroutine:
Task<void> delayed_hello() {
    std::cout << "Starting...\n";
    co_await SleepAwaiter{std::chrono::seconds{2}};
    std::cout << "2 seconds later!\n";
}
```

### `await_suspend` return types

| Return type | Meaning |
|-------------|---------|
| `void` | Always suspend |
| `bool` | `true` = suspend, `false` = don't suspend |
| `coroutine_handle<>` | Suspend and immediately resume the returned handle (symmetric transfer) |

---

## 6. Modules

### What are modules?

A replacement for `#include` that provides:
- Proper encapsulation (only exported names are visible)
- Faster compilation (parsed once, not per-translation-unit)
- No macro leakage
- No include order problems

### The header model (old)

```cpp
// math.h — every #include copies ALL this text
#ifndef MATH_H
#define MATH_H
int add(int a, int b);        // visible
int helper_internal();         // ALSO visible (can't hide!)
#define MATH_VERSION 2         // macro leaks everywhere!
#endif
```

### The module model (new)

```cpp
// math.cppm — module interface unit
export module math;

export int add(int a, int b) { return a + b; }
export int multiply(int a, int b) { return a * b; }

// NOT exported — truly private!
int helper() { return 42; }
```

```cpp
// main.cpp — consumer
import math;

int main() {
    return add(1, 2);   // ✅ exported
    // helper();         // ❌ Error: not exported
    // MATH_VERSION      // ❌ No macros leak
}
```

### Modules vs Headers comparison

| Feature | Headers | Modules |
|---------|---------|---------|
| Compilation | Re-parsed every #include | Compiled once, binary interface |
| Encapsulation | Everything visible | Only exported symbols |
| Macros | Leak globally | No leakage |
| Include order | Can change meaning | Independent |
| Include guards | Manual | Unnecessary |
| Build speed | Slow (redundant parsing) | Fast |

### Module partitions

Split a module into logical parts:

```cpp
// math-impl.cppm — internal partition
module math:impl;
int helper() { return 42; }

// math.cppm — main module interface
export module math;
import :impl;  // import the partition
export int add(int a, int b) { return helper() + a + b; }
```

### Current state of module support (2024)

| Compiler | Support level |
|----------|--------------|
| MSVC | Best support, production-ready |
| GCC 14+ | Good support |
| Clang 17+ | Improving, some limitations |
| CMake 3.28+ | Module support with specific generators |

> **Practical note:** Module support in build systems is still maturing.
> Many projects still use headers. But the direction is clear — modules are
> the future of C++ code organization.

---

## 7. Exercises

See `exercises.cpp`:

1. Implement a `Generator<T>` from scratch
2. Write generators: fibonacci, primes, powers-of-two
3. Implement a simple `Task<T>` with co_return
4. Write a custom awaiter (e.g., async timer)
5. Create a coroutine-based state machine (traffic light)
6. Build a module with exported and private functions

---

**Next lecture:** C++20 Utilities — spaceship operator, std::format,
std::span, and std::jthread.
