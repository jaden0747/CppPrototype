# Lecture 04 — Compile-Time Power & Threads (C++11)

> **Goal:** Harness the compiler as a computation engine with `constexpr` and
> `static_assert`, master variadic templates for type-safe variadics, and
> write your first multi-threaded code with `<thread>` and `<mutex>`.
> Understand **why** compile-time computation matters, **how** parameter packs
> work, and **when** to use different synchronization primitives.

---

## Table of Contents

1. [`constexpr` — Compile-Time Evaluation](#1-constexpr--compile-time-evaluation)
2. [`static_assert` — Compile-Time Checks](#2-static_assert--compile-time-checks)
3. [Variadic Templates](#3-variadic-templates)
4. [`std::thread` & `std::mutex`](#4-stdthread--stdmutex)
5. [Condition Variables & `std::async`](#5-condition-variables--stdasync)
6. [Exercises](#6-exercises)

---

## 1. `constexpr` — Compile-Time Evaluation

### What is it?

`constexpr` means "this can be evaluated at compile time." The compiler
computes the result and embeds it as a constant in the binary — zero runtime cost.

### Why does this matter?

Without `constexpr`:
```cpp
const int TABLE_SIZE = 1024;        // might be compile-time... or might not
const int HASH_SIZE = computeSize(); // definitely runtime
```

With `constexpr`:
```cpp
constexpr int TABLE_SIZE = 1024;    // GUARANTEED compile-time
constexpr int HASH_SIZE = computeSize(); // compile-time IF computeSize is constexpr
```

Benefits:
- **Zero runtime cost** — value baked into the binary
- **Array sizes** — `int arr[constexpr_value]` works
- **Template arguments** — `std::array<int, constexpr_value>`
- **Optimization** — compiler can fold entire computations away

### C++11 constexpr rules (strict)

In C++11, `constexpr` functions are very limited:
- Must consist of a **single return statement**
- No local variables, no loops, no if/else (only ternary `?:`)
- Must return a literal type

```cpp
// ✅ Valid C++11 constexpr
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}
static_assert(factorial(5) == 120, "");  // computed at compile time!

// ❌ Invalid in C++11 (but OK in C++14):
constexpr int factorial_14(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}
```

### constexpr variables

```cpp
constexpr double PI = 3.14159265358979;
constexpr int TABLE_SIZE = 1024;
constexpr int FACT_10 = factorial(10);  // all evaluated at compile time
```

### `const` vs `constexpr` — what's the difference?

| Feature | `const` | `constexpr` |
|---------|---------|-------------|
| Promise | "I won't modify" | "Can evaluate at compile time" |
| Runtime init | ✅ Allowed | ❌ Must be compile-time |
| As template arg | Only if value is compile-time | ✅ Always |
| As array size | Only if value is compile-time | ✅ Always |

```cpp
const int x = computeAtRuntime();      // ✅ OK — runtime const
constexpr int y = computeAtRuntime();  // ❌ ERROR — not compile-time!
constexpr int z = 42;                  // ✅ OK — literal
const int w = 42;                      // ✅ OK — AND also usable as compile-time constant
```

### When is a constexpr function evaluated at compile time?

A `constexpr` function is evaluated at compile time **only when**:
1. All its arguments are compile-time constants, **AND**
2. The result is used in a context requiring a constant expression

```cpp
constexpr int square(int x) { return x * x; }

constexpr int a = square(5);  // ✅ Compile-time: result used in constexpr
int b = square(5);            // ⚠️ MAY be compile-time (optimizer decides)
int runtime_val = read_input();
int c = square(runtime_val);  // Runtime: argument not compile-time
```

---

## 2. `static_assert` — Compile-Time Checks

### What is it?

A compile-time `assert`. If the condition is `false`, compilation **fails**
with your custom error message.

```cpp
static_assert(sizeof(int) == 4, "This code requires 32-bit ints");
static_assert(sizeof(void*) == 8, "This code requires 64-bit platform");
```

### Why is this useful?

Runtime `assert` only catches bugs when the code actually runs — and only in
debug builds. `static_assert` catches impossible situations **before the code
ever runs**.

### Common use cases

```cpp
// 1. Platform/architecture assumptions
static_assert(sizeof(long) >= 8, "Need 64-bit long");
static_assert(alignof(double) == 8, "Unexpected alignment");

// 2. Template parameter validation (pre-C++20 concepts)
template<typename T>
class Buffer {
    static_assert(std::is_default_constructible<T>::value,
                  "Buffer<T> requires T to be default-constructible");
    static_assert(!std::is_reference<T>::value,
                  "Buffer<T> cannot store references");
};

// 3. Ensure struct layout for serialization
struct Packet {
    uint32_t header;
    uint64_t payload;
};
static_assert(sizeof(Packet) == 16, "Packet must be exactly 16 bytes (check packing)");

// 4. Constexpr computation verification
constexpr int result = complex_computation();
static_assert(result == expected, "Algorithm produced wrong result");
```

### C++11 vs C++17 syntax

```cpp
static_assert(condition, "message");   // C++11: message required
static_assert(condition);              // C++17: message optional
```

---

## 3. Variadic Templates

### The problem: variable number of arguments

Before C++11, handling variable arguments was either:
- **C-style `...`** — no type safety, no objects, printf-style UB
- **Write N overloads** — doesn't scale, can't handle arbitrary N

```cpp
// C-style: dangerous!
void log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // If types don't match format string → undefined behavior!
    va_end(args);
}
```

### The solution: parameter packs

A **parameter pack** is zero or more template arguments (types or values)
bundled together:

```cpp
template<typename... Args>  // Args is a "type parameter pack"
void print(const Args&... args);  // args is a "function parameter pack"
```

### How to unpack: recursive peeling

The fundamental pattern: peel off the first argument, process it, recurse:

```cpp
// Base case — empty pack
void print() {
    std::cout << "\n";
}

// Recursive case — peel first, recurse with rest
template<typename T, typename... Rest>
void print(const T& first, const Rest&... rest) {
    std::cout << first;
    if (sizeof...(rest) > 0) std::cout << ", ";
    print(rest...);  // expand remaining args
}

print(1, "hello", 3.14, 'x');
// → print(1, "hello", 3.14, 'x')
// → cout << 1; print("hello", 3.14, 'x')
// → cout << "hello"; print(3.14, 'x')
// → cout << 3.14; print('x')
// → cout << 'x'; print()  ← base case
```

### `sizeof...` — count elements in a pack

```cpp
template<typename... Args>
constexpr std::size_t count_args() {
    return sizeof...(Args);  // number of types
}

template<typename... Args>
void show_count(Args... args) {
    std::cout << "Got " << sizeof...(args) << " arguments\n";
}

static_assert(count_args<int, double, char>() == 3);
```

### Perfect forwarding with variadic templates

This is how `std::make_unique`, `std::make_shared`, `emplace_back`,
and `std::make_tuple` work:

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> my_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// std::forward preserves the value category of each argument:
// - lvalues stay lvalues
// - rvalues stay rvalues
// This means zero unnecessary copies!
```

### Pack expansion patterns

The `...` expands a pattern for each element:

```cpp
template<typename... Args>
auto make_tuple(Args&&... args) {
    return std::tuple<Args...>(std::forward<Args>(args)...);
    //     Pattern:  ^^^^^^^^                    ^^^^^^^^
    //     Expands to: tuple<int, double, string>(forward<int>(a), forward<double>(b), ...)
}

// Other expansion examples:
template<typename... Ts>
struct inherit_all : Ts... {};  // inherit from ALL types in the pack

template<typename... Ts>
void construct_all(Ts*... ptrs) {
    // Call constructor on each pointer:
    (new (ptrs) Ts(), ...);  // C++17 fold expression
}
```

---

## 4. `std::thread` & `std::mutex`

### Why multi-threading?

Modern CPUs have multiple cores. A single-threaded program uses only one core.
Multi-threading lets you:
- Speed up CPU-bound work (parallel computation)
- Keep UI responsive while doing background work
- Handle multiple I/O operations concurrently

### Creating and joining threads

```cpp
#include <thread>

void worker(int id) {
    std::cout << "Worker " << id << " running\n";
}

int main() {
    std::thread t1(worker, 1);  // start thread, pass argument
    std::thread t2(worker, 2);

    t1.join();  // block until t1 finishes
    t2.join();  // block until t2 finishes
}
// If you forget join() → std::terminate() is called!
```

### The data race problem

```cpp
int counter = 0;

void bad_increment(int times) {
    for (int i = 0; i < times; ++i)
        ++counter;  // ❌ DATA RACE! Multiple threads read/write simultaneously
}
// With 2 threads doing 1000 increments each:
// Expected: 2000. Actual: 1500? 1800? Undefined!
```

### Protecting shared data with `std::mutex`

```cpp
#include <mutex>

std::mutex mtx;
int counter = 0;

void safe_increment(int times) {
    for (int i = 0; i < times; ++i) {
        std::lock_guard<std::mutex> lock(mtx);  // lock on construction
        ++counter;                               // safe access
        // lock released on destruction (RAII!)
    }
}
```

### `lock_guard` vs `unique_lock` — when to use which

| Feature | `lock_guard` | `unique_lock` |
|---------|-------------|---------------|
| RAII lock/unlock | ✅ | ✅ |
| Manual lock/unlock | ❌ | ✅ |
| Use with condition_variable | ❌ | ✅ (required) |
| Deferred locking | ❌ | ✅ |
| Movable | ❌ | ✅ |
| Overhead | Minimal | Slightly more |

**Rule of thumb:** Use `lock_guard` by default. Use `unique_lock` only when
you need condition variables, manual unlock, or deferred locking.

### Avoiding deadlocks: `std::scoped_lock` (C++17)

```cpp
std::mutex m1, m2;

// ❌ Deadlock risk: Thread A locks m1 then m2, Thread B locks m2 then m1
void bad() {
    std::lock_guard<std::mutex> l1(m1);
    std::lock_guard<std::mutex> l2(m2);  // might deadlock!
}

// ✅ C++17: locks multiple mutexes atomically (deadlock-free)
void good() {
    std::scoped_lock lock(m1, m2);  // acquires both without deadlock
}
```

---

## 5. Condition Variables & `std::async`

### The problem: how to wait for a condition?

```cpp
// ❌ Busy-wait (wastes CPU):
while (!data_ready) {
    // spinning... burning CPU cycles
}
```

### Condition variables: efficient waiting

A condition variable lets a thread **sleep** until another thread signals it:

```cpp
#include <condition_variable>
#include <queue>

std::queue<int> work_queue;
std::mutex mtx;
std::condition_variable cv;
bool done = false;

// Producer — adds work items
void producer() {
    for (int i = 0; i < 10; ++i) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            work_queue.push(i);
        }
        cv.notify_one();  // wake up one waiting consumer
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
    }
    cv.notify_all();  // wake all consumers to check 'done'
}

// Consumer — waits for work
void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !work_queue.empty() || done; });
        // ↑ atomically: unlock mutex, sleep, re-lock when woken

        if (work_queue.empty() && done) break;

        int item = work_queue.front();
        work_queue.pop();
        lock.unlock();  // release lock before processing

        process(item);
    }
}
```

### Why the predicate in `cv.wait(lock, predicate)`?

**Spurious wakeups**: a thread can wake up from `wait()` even if nobody
called `notify`. The predicate guards against this:

```cpp
cv.wait(lock, []{ return !queue.empty(); });
// Equivalent to:
while (queue.empty()) {
    cv.wait(lock);  // might wake spuriously → check again
}
```

### `std::async` — high-level concurrency

For simple "fire and forget" or "compute in background" tasks:

```cpp
#include <future>

auto future = std::async(std::launch::async, [] {
    return expensive_computation();  // runs on a separate thread
});

// ... do other work while computation runs ...

int result = future.get();  // blocks until result is ready
```

### `std::async` launch policies

| Policy | Meaning |
|--------|---------|
| `std::launch::async` | Must run on a new thread |
| `std::launch::deferred` | Lazy: runs when `.get()` is called (same thread) |
| `std::launch::async \| std::launch::deferred` | Implementation decides (default) |

### `std::future` and `std::promise`

For more control over when/where the value is produced:

```cpp
std::promise<int> prom;
std::future<int> fut = prom.get_future();

std::thread t([&prom] {
    int result = heavy_work();
    prom.set_value(result);  // fulfills the promise
});

int val = fut.get();  // blocks until promise is fulfilled
t.join();
```

---

## 6. Exercises

See `exercises.cpp`:

1. Write constexpr Fibonacci and verify with static_assert
2. Use static_assert to validate template parameters
3. Implement a variadic `min()` function that takes any number of arguments
4. Implement a type-safe `printf` using variadic templates
5. Write a multi-threaded counter protected by a mutex
6. Implement a producer-consumer queue using condition variables

---

**Next lecture:** C++14 Refinements — generic lambdas, relaxed constexpr,
variable templates, and `make_unique`.
