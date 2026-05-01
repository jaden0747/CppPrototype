# Stdlib 13 — Concurrency

> **Goal:** Master threads, synchronization primitives, atomics, futures,
> and C++20 synchronization tools. Understand when and why to use each
> concurrency mechanism, and how to avoid common pitfalls like data races
> and deadlocks.

---

## Table of Contents

1. [Why Concurrency?](#1-why-concurrency)
2. [`std::thread` and `std::jthread`](#2-stdthread-and-stdjthread)
3. [`std::mutex` — Mutual Exclusion](#3-stdmutex--mutual-exclusion)
4. [Lock Guards: `lock_guard`, `unique_lock`, `scoped_lock`](#4-lock-guards)
5. [`std::condition_variable` — Waiting for Events](#5-stdcondition_variable)
6. [Atomics](#6-atomics)
7. [Async and Futures](#7-async-and-futures)
8. [C++20 Synchronization: Latch, Barrier, Semaphore](#8-c20-synchronization)
9. [Common Pitfalls](#9-common-pitfalls)
10. [Exercises](#10-exercises)

---

## 1. Why Concurrency?

### The problem: single-threaded bottleneck

Modern CPUs have 4-16+ cores, but a single-threaded program uses only ONE.
Concurrency lets you:
- **Speed up CPU-bound work** (parallel computation)
- **Keep UI responsive** (background I/O while UI thread runs)
- **Handle multiple connections** (servers)
- **Overlap I/O and computation** (read file while processing)

### The danger: shared mutable state

When multiple threads access the same data, and at least one thread writes,
you have a **data race** — undefined behavior!

```cpp
int counter = 0;  // shared between threads

// Thread 1: counter++;  // READ → INCREMENT → WRITE
// Thread 2: counter++;  // READ → INCREMENT → WRITE (at the same time!)
// Result: might be 1 instead of 2! (lost update)
```

---

## 2. `std::thread` and `std::jthread`

### `std::thread` — basic thread

```cpp
#include <thread>

void worker(int id) {
    std::cout << "Thread " << id << " running\n";
}

std::thread t1(worker, 1);  // start thread, pass argument
std::thread t2(worker, 2);

t1.join();  // wait for t1 to finish
t2.join();  // wait for t2 to finish
// MUST join or detach before thread object is destroyed!
```

### The join/detach rule

```cpp
std::thread t(func);
// If t goes out of scope without join() or detach():
// → std::terminate() is called! Your program CRASHES.

t.join();    // block until thread finishes
// OR
t.detach();  // let thread run independently (careful: lifetime issues!)
```

### `std::jthread` (C++20) — the better thread

```cpp
std::jthread t(worker, 1);
// Automatically joins in destructor — no crash if you forget!
// Also supports cooperative cancellation via stop_token.
```

### Cooperative cancellation with `stop_token`

```cpp
std::jthread t([](std::stop_token token) {
    while (!token.stop_requested()) {
        do_work();
    }
    std::cout << "Thread stopping gracefully\n";
});

// Later:
t.request_stop();  // asks the thread to stop
// t.~jthread() joins automatically
```

### Passing arguments to threads

```cpp
// By value (copied):
std::thread t(func, value);

// By reference: MUST use std::ref!
int x = 42;
std::thread t(func, std::ref(x));

// Lambda (most flexible):
std::thread t([&x]() { x = 100; });
```

---

## 3. `std::mutex` — Mutual Exclusion

### What is it?

A mutex ("mutual exclusion") ensures only **one thread** accesses a
shared resource at a time. Think of it as a **lock on a bathroom door**.

```cpp
#include <mutex>

std::mutex mtx;
int counter = 0;

void increment() {
    mtx.lock();      // acquire lock (blocks if another thread holds it)
    counter++;       // safe: only one thread in here
    mtx.unlock();    // release lock
}
```

### Never use `lock()/unlock()` directly!

```cpp
void dangerous() {
    mtx.lock();
    do_work();     // what if this throws an exception?
    mtx.unlock();  // NEVER REACHED → deadlock! Other threads wait forever.
}
```

Use **lock guards** instead (next section).

### `std::recursive_mutex`

Allows the **same thread** to lock multiple times (must unlock same number):

```cpp
std::recursive_mutex rmtx;
void a() { std::lock_guard lock(rmtx); b(); }
void b() { std::lock_guard lock(rmtx); /* OK: same thread */ }
```

---

## 4. Lock Guards

### `std::lock_guard` — simplest RAII lock

```cpp
void safe_increment() {
    std::lock_guard<std::mutex> lock(mtx);  // locks mtx
    counter++;
}  // lock destroyed → mtx automatically unlocked (even on exception!)
```

With C++17 CTAD (class template argument deduction):
```cpp
std::lock_guard lock(mtx);  // no need to write <std::mutex>
```

### `std::unique_lock` — flexible lock

Like `lock_guard` but supports:
- Deferred locking
- Timed locking
- Manual lock/unlock
- Required by `condition_variable`

```cpp
std::unique_lock lock(mtx);           // locks immediately
std::unique_lock lock(mtx, std::defer_lock); // don't lock yet
lock.lock();                          // lock manually later
lock.unlock();                        // unlock manually
```

### `std::scoped_lock` (C++17) — lock multiple mutexes

```cpp
std::mutex mtx1, mtx2;

// Locks BOTH mutexes atomically — no deadlock!
std::scoped_lock lock(mtx1, mtx2);
// If you locked them separately, Thread A could lock mtx1 while
// Thread B locks mtx2 → both waiting for the other → DEADLOCK!
```

### When to use which

| Lock type | Use when |
|-----------|----------|
| `lock_guard` | Simple lock/unlock scope |
| `unique_lock` | Need condition_variable, timed lock, or manual control |
| `scoped_lock` | Locking multiple mutexes at once |

---

## 5. `std::condition_variable` — Waiting for Events

### What is it?

A way for one thread to **wait** until another thread signals that some
condition is true. Without it, you'd have to busy-wait (spin):

```cpp
// BAD: busy waiting (wastes CPU!)
while (!ready) { /* spin... */ }

// GOOD: condition_variable (sleeps until signaled)
std::unique_lock lock(mtx);
cv.wait(lock, []{ return ready; });
```

### Producer-consumer example

```cpp
std::mutex mtx;
std::condition_variable cv;
std::queue<int> q;

// Producer thread:
void produce() {
    {
        std::lock_guard lock(mtx);
        q.push(42);
    }
    cv.notify_one();  // wake up one waiting consumer
}

// Consumer thread:
void consume() {
    std::unique_lock lock(mtx);
    cv.wait(lock, [&]{ return !q.empty(); }); // sleep until queue non-empty
    int val = q.front();
    q.pop();
}
```

### Why the predicate in `wait()`?

**Spurious wakeups!** The OS may wake up a waiting thread for no reason.
The predicate re-checks the condition:

```cpp
cv.wait(lock, []{ return ready; });
// Equivalent to:
// while (!ready) cv.wait(lock);
```

---

## 6. Atomics

### What is it?

`std::atomic<T>` provides **lock-free** thread-safe operations on a single
variable. No mutex needed — the CPU handles it with special instructions.

```cpp
#include <atomic>

std::atomic<int> counter{0};

// Thread 1:
counter++;  // atomic increment — thread-safe!

// Thread 2:
counter++;  // also safe, no mutex needed!

counter.load();              // read
counter.store(42);           // write
counter.fetch_add(5);        // add and return old value
counter.compare_exchange_strong(expected, desired); // CAS
```

### When to use atomics vs mutex

| Scenario | Use |
|----------|-----|
| Single variable (counter, flag) | `atomic` — faster, no lock overhead |
| Multiple related variables | `mutex` — can't atomically update 2 vars |
| Complex operations (if/then/update) | `mutex` — atomics are for single ops |

### `std::atomic_flag` — the simplest atomic

```cpp
std::atomic_flag flag = ATOMIC_FLAG_INIT;
bool was_set = flag.test_and_set();  // set flag, return previous value
flag.clear();                        // reset flag
```

Used to build **spinlocks** (busy-wait locks):

```cpp
class Spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock()   { while (flag.test_and_set(std::memory_order_acquire)) { } }
    void unlock() { flag.clear(std::memory_order_release); }
};
```

### Memory orders (advanced)

| Order | Guarantee |
|-------|----------|
| `memory_order_relaxed` | No ordering guarantee (fastest) |
| `memory_order_acquire` | All reads after this see writes before a release |
| `memory_order_release` | All writes before this are visible to an acquire |
| `memory_order_seq_cst` | Full sequential consistency (default, safest) |

**Rule:** Use the default (`seq_cst`) unless profiling shows it's a bottleneck
and you deeply understand the memory model.

---

## 7. Async and Futures

### `std::async` — fire and forget (almost)

```cpp
#include <future>

auto future = std::async(std::launch::async, []() {
    return expensive_computation();
});

// Do other work while computation runs...

int result = future.get();  // blocks until result is ready
```

### Launch policies

```cpp
// Guaranteed new thread:
auto f1 = std::async(std::launch::async, func);

// May run lazily (on get()):
auto f2 = std::async(std::launch::deferred, func);

// Implementation chooses:
auto f3 = std::async(func);  // async or deferred
```

### `std::promise` and `std::future` — manual channel

```cpp
std::promise<int> p;
std::future<int> f = p.get_future();

// Thread A:
std::thread t([&p]() {
    int result = compute();
    p.set_value(result);  // send result
});

// Thread B (main):
int val = f.get();  // blocks until value is set
t.join();
```

### `std::packaged_task` — wraps a callable with a future

```cpp
std::packaged_task<int()> task([]{ return 42; });
auto future = task.get_future();

std::thread t(std::move(task));
int result = future.get();  // 42
t.join();
```

---

## 8. C++20 Synchronization: Latch, Barrier, Semaphore

### `std::latch` — one-time countdown

```cpp
#include <latch>

std::latch done(3);  // countdown from 3

// Three worker threads:
for (int i = 0; i < 3; ++i) {
    threads.emplace_back([&done]() {
        do_work();
        done.count_down();  // decrement
    });
}

done.wait();  // blocks until count reaches 0
// All 3 workers are done!
```

**Use case:** Wait for N initialization tasks to complete.

### `std::barrier` — reusable synchronization point

```cpp
#include <barrier>

std::barrier sync_point(3, []() noexcept {
    std::cout << "All threads reached barrier\n";
});

// Each thread:
void worker() {
    for (int phase = 0; phase < 5; ++phase) {
        do_phase_work(phase);
        sync_point.arrive_and_wait();  // wait for all threads
        // All threads proceed to next phase together
    }
}
```

**Use case:** Iterative algorithms where threads must synchronize between phases.

### `std::counting_semaphore` — limit concurrent access

```cpp
#include <semaphore>

std::counting_semaphore<3> sem(3);  // max 3 concurrent accesses

void worker() {
    sem.acquire();       // decrement (blocks if count is 0)
    access_resource();   // at most 3 threads here
    sem.release();       // increment (allow another thread in)
}

// Binary semaphore (same as semaphore with max 1):
std::binary_semaphore gate(0);
gate.release();   // signal
gate.acquire();   // wait
```

**Use case:** Connection pools, rate limiting, producer-consumer with bounded buffer.

---

## 9. Common Pitfalls

### 1. Data race

```cpp
int x = 0;
std::thread t1([&]{ x++; });  // ❌ data race!
std::thread t2([&]{ x++; });
// Fix: use atomic<int> or protect with mutex
```

### 2. Deadlock

```cpp
// Thread 1: lock(A) → lock(B)
// Thread 2: lock(B) → lock(A)
// → Both stuck forever!
// Fix: always lock in same order, or use scoped_lock(A, B)
```

### 3. Forgetting to join/detach

```cpp
{
    std::thread t(func);
}  // ❌ t destroyed without join/detach → std::terminate()!
// Fix: use std::jthread, or always join before scope exit
```

### 4. Dangling references with detached threads

```cpp
void bad() {
    int local = 42;
    std::thread t([&local] { use(local); });
    t.detach();
}  // local destroyed, thread still running → undefined behavior!
// Fix: pass by value, or use join
```

---

## 10. Exercises

See `exercises.cpp`.

---

**This is the final lecture in the stdlib series.**
