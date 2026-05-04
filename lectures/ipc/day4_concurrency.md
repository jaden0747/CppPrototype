# Day 4 — Modern C++ Concurrency

> **Time budget:** 60 min concept · 120 min lab · 60 min drills · 15 min recap
> **Prerequisites:** Basic threading familiarity; Days 1–3 for context on memory ordering
> **By the end you can:**
> - Explain `acquire`/`release` with a concrete producer-consumer example, without notes
> - Implement a lock-free SPSC queue using `std::atomic`
> - Choose the right memory order for a given pattern and defend the choice
> - Describe `std::jthread`, `std::stop_token`, and `std::expected` to an interviewer
> - Write a thread-safe bounded queue with `std::condition_variable`

---

## 1. Conceptual overview

You already know threads. The gap between "knows how to use `std::mutex`" and "embedded systems hire" is the **C++ memory model**. Interviewers ask about it directly. Most candidates fold here.

The memory model answers: *given that the compiler and CPU reorder instructions, what guarantees does C++ provide about the order in which one thread observes writes from another?*

The answer is: only what you explicitly request. Without synchronization, concurrent access to a shared variable is a data race, and data races are undefined behavior — not "might give the wrong answer," but genuinely undefined, including producing values that never existed.

**The happens-before relation:**

Operation A *happens-before* operation B if A is sequenced before B in the same thread, or if a synchronization edge (a release/acquire pair on the same atomic) connects them across threads.

```
Thread 1                        Thread 2
  data = 42;                     // ...
  flag.store(true,               if (flag.load(memory_order_acquire)) {
    memory_order_release);  ──▶      // happens-after the store
                                     // data == 42 is guaranteed here
                                 }
```

The release store on `flag` "publishes" everything that happened before it in Thread 1. The acquire load "subscribes" to everything that was published by the most recent release store it observed. This is the fundamental synchronization pattern.

**The five orders you need:**

| Order | Guarantee | Cost |
|---|---|---|
| `relaxed` | Atomic read/write, no ordering | Cheapest |
| `acquire` | No loads/stores after this can move before it | Load side of release/acquire pair |
| `release` | No loads/stores before this can move after it | Store side of release/acquire pair |
| `acq_rel` | Both acquire and release (for RMW operations) | For CAS, fetch_add on shared state |
| `seq_cst` | Total order across all `seq_cst` operations | Most expensive; default for `std::atomic` |

**Use `seq_cst` when:** multiple atomic variables must appear in the same order to all threads. This is rare.

**Use `acquire`/`release` when:** one thread publishes data and another consumes it (the canonical pattern). This is most of what you write.

**Use `relaxed` when:** you only need atomicity, not ordering — a counter, a flag you check with a separate synchronizing load, sequence numbers.

---

## 2. Deep dive: acquire/release in practice

```cpp
// publisher_subscriber.cpp
// Build: g++ -std=c++20 -Wall -Wextra -O2 -pthread -o pub_sub publisher_subscriber.cpp
// Run:   ./pub_sub

#include <atomic>
#include <thread>
#include <cassert>
#include <cstdio>

struct Payload {
    int  a;
    int  b;
    char msg[32];
};

static Payload         payload{};
static std::atomic<bool> ready{false};

void publisher() {
    // Write the payload (plain writes — not atomic)
    payload.a = 1;
    payload.b = 2;
    __builtin_memcpy(payload.msg, "hello", 6);

    // Release store: all writes above are visible to any thread
    // that observes this store with an acquire load.
    ready.store(true, std::memory_order_release);
}

void subscriber() {
    // Spin until published
    while (!ready.load(std::memory_order_acquire))
        ;
    // Acquire load: happens-after the release store.
    // payload.a, payload.b, payload.msg are safe to read.
    assert(payload.a == 1 && payload.b == 2);
    printf("subscriber got: a=%d b=%d msg=%s\n",
           payload.a, payload.b, payload.msg);
}

int main() {
    std::thread t1(publisher);
    std::thread t2(subscriber);
    t1.join();
    t2.join();
    return 0;
}
```

Replace `memory_order_release` with `memory_order_relaxed` in the store. Now the assertion can fire — the subscriber may see `ready == true` before seeing the updated `payload` values. Run with `-fsanitize=thread` to catch this reliably.

**Why not `seq_cst` here?**

`seq_cst` would also be correct. On x86, it costs an `MFENCE` instruction on the store (or an `XCHG`). On ARM, it costs a full `DMB ISH` barrier on both sides. For a hot path, the extra barrier matters. `acquire`/`release` on x86 compiles to the same instructions as plain loads/stores (TSO handles it in hardware); on ARM it compiles to `STLR`/`LDAR`, which are lighter than full barriers.

---

## 3. Deep dive: lock-free SPSC queue

The single-producer/single-consumer queue is the most common lock-free interview problem. It's also the correct tool for a great deal of real embedded work (sensor thread → processing thread).

```cpp
// spsc_queue.h
// Single-producer, single-consumer. Lock-free. Fixed capacity.
// NOT safe for multiple producers or multiple consumers.
#pragma once
#include <atomic>
#include <array>
#include <optional>
#include <cstddef>

template <typename T, std::size_t N>
class SPSCQueue {
    static_assert((N & (N - 1)) == 0, "N must be a power of 2");

    // Separate cache lines to prevent false sharing
    alignas(64) std::atomic<std::size_t> head_{0};  // written by producer
    alignas(64) std::atomic<std::size_t> tail_{0};  // written by consumer
    std::array<T, N> buf_{};

public:
    // Called by producer only
    bool try_push(T val) {
        std::size_t h = head_.load(std::memory_order_relaxed);
        std::size_t next = (h + 1) & (N - 1);
        // Check full: producer reads tail with acquire to sync with consumer's release
        if (next == tail_.load(std::memory_order_acquire))
            return false;
        buf_[h] = std::move(val);
        head_.store(next, std::memory_order_release);
        return true;
    }

    // Called by consumer only
    std::optional<T> try_pop() {
        std::size_t t = tail_.load(std::memory_order_relaxed);
        // Consumer reads head with acquire to sync with producer's release
        if (t == head_.load(std::memory_order_acquire))
            return std::nullopt;
        T val = std::move(buf_[t]);
        tail_.store((t + 1) & (N - 1), std::memory_order_release);
        return val;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }
};
```

**The ABA problem — does it apply here?**

Not in this design. ABA is an issue when a value is read, the memory is freed and reallocated with the same value, and a compare-exchange succeeds incorrectly. This queue uses array indices, not pointers, and indices wrap monotonically. No ABA.

**When does lock-free lose?**

When contention is high. A spinning producer in `try_push` burns a core. In those cases, a mutex + `condition_variable` (which yields the CPU) wins on throughput. Lock-free wins when: contention is low, latency matters more than throughput, or you cannot afford the scheduling jitter of a sleep/wake cycle (hard real-time).

---

## 4. Deep dive: `std::jthread`, `std::stop_token`, `std::expected`

These are the C++20/23 additions interviewers probe to gauge how current your knowledge is.

**`std::jthread` (C++20):**

`std::thread` does not join on destruction — a detached joinable thread in a destructor calls `std::terminate`. `std::jthread` joins automatically in its destructor. It also accepts a `std::stop_token` as its first argument.

```cpp
// Build: g++ -std=c++20 -Wall -Wextra -pthread -o jthread_demo jthread_demo.cpp
#include <thread>
#include <stop_token>
#include <chrono>
#include <cstdio>

void worker(std::stop_token st, int id) {
    while (!st.stop_requested()) {
        printf("[worker %d] running\n", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    printf("[worker %d] stop requested, exiting\n", id);
}

int main() {
    std::jthread t(worker, 42);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();   // sets the stop_token; worker sees it on next check
    // t.join() called automatically in destructor
    return 0;
}
```

`stop_token` / `stop_source` / `stop_callback` form a cooperative cancellation system. No `volatile bool` globals; no `atomic<bool>` you manage yourself. Prefer this for new code.

**`std::expected<T, E>` (C++23):**

A type that holds either a value `T` or an error `E`, without exceptions. Critical for embedded systems where exceptions are often disabled.

```cpp
#include <expected>
#include <cerrno>
#include <cstring>
#include <cstdio>

std::expected<int, std::string> open_device(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return std::unexpected(std::string(strerror(errno)));
    return fd;
}

// Caller:
auto result = open_device("/dev/sensor0");
if (!result)
    fprintf(stderr, "open failed: %s\n", result.error().c_str());
else
    use_fd(result.value());
```

---

## 5. Lab

### Setup

```bash
g++ -std=c++20 -Wall -Wextra -O2 -pthread -o mutex_queue   mutex_queue.cpp
g++ -std=c++20 -Wall -Wextra -O2 -pthread -o lockfree_queue lockfree_queue.cpp
g++ -std=c++20 -Wall -Wextra -O2 -pthread -fsanitize=thread -o tsan_demo tsan_demo.cpp
```

### Tasks

**Task 1 — Mutex + condition_variable bounded queue**

```cpp
// mutex_queue.cpp
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <chrono>
#include <cstdio>

template <typename T>
class BoundedQueue {
    std::mutex              mtx_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    std::queue<T>           q_;
    std::size_t             cap_;

public:
    explicit BoundedQueue(std::size_t cap) : cap_(cap) {}

    void push(T val) {
        std::unique_lock lock(mtx_);
        // Predicate prevents spurious wakeup bugs
        not_full_.wait(lock, [&]{ return q_.size() < cap_; });
        q_.push(std::move(val));
        not_empty_.notify_one();
    }

    T pop() {
        std::unique_lock lock(mtx_);
        not_empty_.wait(lock, [&]{ return !q_.empty(); });
        T val = std::move(q_.front());
        q_.pop();
        not_full_.notify_one();
        return val;
    }
};

int main() {
    BoundedQueue<int> q(8);

    std::jthread producer([&](std::stop_token st) {
        for (int i = 0; !st.stop_requested(); ++i) {
            q.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::jthread consumer([&](std::stop_token st) {
        while (!st.stop_requested()) {
            int v = q.pop();
            printf("consumed: %d\n", v);
        }
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    producer.request_stop();
    consumer.request_stop();
    return 0;
}
```

**Done when:** Consumer prints values in order; no crash under valgrind.

**Task 2 — Lock-free SPSC queue benchmark**

Instantiate `SPSCQueue<int, 1024>`. Producer pushes 10 million integers; consumer pops them. Time with `std::chrono::steady_clock`. Compare against `BoundedQueue` at the same throughput. Note the crossover point — at what message rate does the lock-free version stop winning?

**Done when:** You have two timing numbers and can explain the difference.

**Task 3 — ThreadSanitizer demo**

Write a program with a deliberate data race (shared `int` incremented from two threads with no synchronization). Build with `-fsanitize=thread`. Observe the report. Then fix it with `std::atomic<int>`. Confirm TSan is clean.

**Done when:** You can read a TSan report and identify the racing accesses.

**Task 4 — RAII shm wrapper**

```cpp
// shm_handle.h — RAII wrapper for a POSIX shm segment
#pragma once
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <string>

class ShmHandle {
    void  *ptr_  = nullptr;
    size_t size_ = 0;
    std::string name_;

public:
    ShmHandle(const std::string &name, size_t size, bool create)
        : size_(size), name_(name)
    {
        int flags = O_RDWR | (create ? O_CREAT : 0);
        int fd = shm_open(name.c_str(), flags, 0600);
        if (fd < 0) throw std::system_error(errno, std::generic_category(), "shm_open");
        if (create && ftruncate(fd, (off_t)size) < 0) {
            close(fd);
            throw std::system_error(errno, std::generic_category(), "ftruncate");
        }
        ptr_ = mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        if (ptr_ == MAP_FAILED)
            throw std::system_error(errno, std::generic_category(), "mmap");
    }

    ~ShmHandle() {
        if (ptr_ && ptr_ != MAP_FAILED) munmap(ptr_, size_);
        if (!name_.empty()) shm_unlink(name_.c_str());
    }

    // Non-copyable, movable
    ShmHandle(const ShmHandle &) = delete;
    ShmHandle &operator=(const ShmHandle &) = delete;
    ShmHandle(ShmHandle &&o) noexcept
        : ptr_(o.ptr_), size_(o.size_), name_(std::move(o.name_)) {
        o.ptr_ = nullptr; o.size_ = 0;
    }

    template <typename T> T *as() { return static_cast<T *>(ptr_); }
};
```

Verify with valgrind that no mapping leaks when an exception is thrown in the constructor.

**Done when:** `valgrind --leak-check=full` shows no leaks.

---

## 6. Common pitfalls

- **Using `volatile` for thread synchronization.** `volatile` prevents the compiler from caching a variable in a register; it does not create memory ordering constraints or prevent CPU reordering. `std::atomic<T>` with the appropriate order is correct. This is one of the most common interview mistakes.

- **Passing a predicate-less wait to `condition_variable`.** `cv.wait(lock)` without a predicate is vulnerable to spurious wakeups — the thread wakes, the condition is still false, it re-waits, but it may miss a notification. Always use `wait(lock, predicate)`.

- **Forgetting `alignas(64)` on independent atomics.** If `head_` and `tail_` share a cache line, a write to `head_` invalidates the cache line holding `tail_` in the consumer's cache, causing unnecessary cache coherence traffic. Align each to 64 bytes (one cache line).

- **`shared_ptr` thread safety misconception.** The control block (reference count) is thread-safe. The managed object is not. Two threads calling non-const methods on the same `shared_ptr<T>` concurrently (not the same `*T`) is also a data race on the pointer itself.

- **`compare_exchange_weak` in a non-loop.** `compare_exchange_weak` can fail spuriously (on LL/SC architectures). It must be in a loop. `compare_exchange_strong` does not fail spuriously but is slower. Use `weak` in a loop, `strong` when you can't loop.

- **`std::thread` destructor with a joinable thread.** Calls `std::terminate`. Always join or detach before the destructor. Use `std::jthread` for new code to avoid this entirely.

---

## 7. Interview drills

**Q: When would you use `acquire`/`release` instead of `seq_cst`?**

Whenever the pattern is a single publish/subscribe pair — one thread writes data and stores a flag (release), another loads the flag (acquire) and reads the data. `acquire`/`release` provides exactly the required guarantee at lower cost: on ARM it compiles to `STLR`/`LDAR` instead of `DMB ISH` barriers. Use `seq_cst` only when you need a total order across multiple atomic variables observed from multiple threads — a rare case in practice.

*Follow-up: "Give me a case where acquire/release is insufficient."* — A Dekker-style mutual exclusion algorithm that relies on the order of two independent stores being seen in that order by another thread. `seq_cst` provides a single total order; `acquire`/`release` only provides per-variable ordering.

---

**Q: Show me how `condition_variable::wait` works under the hood.**

`wait(lock, pred)` is equivalent to `while (!pred()) { cv.wait(lock); }`. The single-argument `wait(lock)` atomically releases the mutex and puts the thread to sleep in the kernel. When `notify_one` or `notify_all` is called, one or all waiters are woken, each reacquires the mutex, and returns. Spurious wakeups (the thread wakes without a notification) are permitted by the standard — this is why the predicate form exists. The predicate is rechecked on every wakeup.

---

**Q: Why is `std::shared_ptr` thread-safe for the control block but not the pointee?**

The control block's reference count is an atomic integer; increment and decrement are thread-safe. However, copying or assigning a `shared_ptr` instance itself — the pointer and the control block pointer — is not atomic as a pair. Two threads writing to the same `shared_ptr` variable concurrently is a data race on the instance. And of course, the managed object `*T` has no inherent thread safety.

---

**Q: Write a thread-safe singleton. Now do it without `std::call_once`.**

```cpp
// With call_once:
Foo &get_foo() {
    static std::once_flag flag;
    static Foo *instance = nullptr;
    std::call_once(flag, []{ instance = new Foo; });
    return *instance;
}

// Without call_once — using function-local static (C++11 and later):
Foo &get_foo() {
    static Foo instance;  // initialization is guaranteed thread-safe by the standard
    return instance;
}
```

The function-local static approach is the simplest correct singleton in C++11+. The standard (§6.7) guarantees that concurrent initialization of a function-local static waits for the first thread to complete initialization. No explicit synchronization needed.

---

**Q: What's the difference between `std::thread` and `std::jthread`?**

`std::jthread` joins automatically in its destructor (preventing the `std::terminate` trap of a joinable `std::thread` going out of scope). It also carries a `std::stop_source` and accepts a `std::stop_token` as the first argument to the thread function, enabling cooperative cancellation via `request_stop()` without external `atomic<bool>` flags. For new code, prefer `std::jthread`.

---

## 8. Cheatsheet

### Memory orders — quick reference

| Pattern | Store | Load |
|---|---|---|
| Publish data, consumer reads | `release` | `acquire` |
| Shared counter, no ordering needed | `relaxed` | `relaxed` |
| RMW (CAS, fetch_add) on shared state | `acq_rel` | — |
| Multiple atomics, total order required | `seq_cst` | `seq_cst` |

### Synchronization primitives — when to use

| Primitive | Use when |
|---|---|
| `std::mutex` + `unique_lock` | General mutual exclusion |
| `std::scoped_lock` | Locking multiple mutexes (deadlock-safe) |
| `std::condition_variable` | Wait until a condition is true |
| `std::atomic<T>` | Lock-free flag or counter |
| SPSC queue (lock-free) | Single producer/consumer, latency-sensitive |
| `std::jthread` | Prefer over `std::thread` for new code |

### SPSC queue invariants
- `head_` written only by producer; `tail_` written only by consumer
- Producer: `acquire` load of `tail_`; `release` store of `head_`
- Consumer: `acquire` load of `head_`; `release` store of `tail_`
- Align head and tail to separate cache lines (64 bytes)

### Build flags

```bash
g++ -std=c++20 -Wall -Wextra -O2 -pthread          # standard
g++ -std=c++20 -fsanitize=thread -pthread           # TSan
g++ -std=c++20 -fsanitize=address,undefined         # ASan + UBSan
valgrind --tool=helgrind ./binary                    # data race detection
```

### Key types (C++20/23)

| Type | Header | Purpose |
|---|---|---|
| `std::jthread` | `<thread>` | Auto-joining thread with stop support |
| `std::stop_token` | `<stop_token>` | Cooperative cancellation check |
| `std::stop_source` | `<stop_token>` | Requests stop on associated tokens |
| `std::expected<T,E>` | `<expected>` | Value-or-error without exceptions (C++23) |
| `std::latch` | `<latch>` | One-time count-down synchronization |
| `std::barrier` | `<barrier>` | Reusable phase synchronization |

---

## 9. Further reading

- Williams, *C++ Concurrency in Action* (2nd ed.), Chapter 5 — the memory model; required reading
- cppreference.com: `std::memory_order` — the normative reference
- Herb Sutter, "atomic Weapons" (CppCon 2012) — still the clearest explanation of acquire/release; available on YouTube