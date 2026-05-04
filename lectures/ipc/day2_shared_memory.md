# Day 2 — POSIX Shared Memory + Memory Mapping

> **Time budget:** 90 min concept · 120 min lab · 45 min drills · 15 min recap
> **Prerequisites:** Day 1 complete; comfortable with pointers and struct layout in C/C++
> **By the end you can:**
> - Recite the full `shm_open` → `mmap` → `munmap` → `shm_unlink` lifecycle
> - Explain why shared memory alone gives you no synchronization — and say it reflexively
> - Implement a ring buffer in shared memory using atomics
> - Inspect your segment in `/proc` and `/dev/shm`
> - Answer the shared-memory deep-dive questions that embedded interviewers favor

---

## 1. Conceptual overview

Shared memory is the fastest IPC primitive. When two processes map the same segment, they read and write the same physical RAM — no kernel copy on the data path. The kernel is involved in setup (`shm_open`, `mmap`) and teardown, but not in every read or write. This is the entire reason it exists.

**The cost: the kernel gives you nothing else.** No ordering, no atomicity beyond what the hardware and your memory model provide, no notification. You get a pointer and a promise that both processes see the same bytes. Everything else is your problem.

This is not optional nuance — it is the central fact of shared memory. Every shared-memory design question in an interview is really asking: *where is your synchronization?*

```
Process A (writer)          Physical RAM              Process B (reader)
   va: 0x7f3a0000  ───────▶  [shm segment]  ◀───────   va: 0x7f8b0000
                              head: uint64
                              tail: uint64
                              data[1024]
```

Both virtual addresses map to the same physical pages. The addresses differ; the storage does not.

**POSIX vs System V — one paragraph:**

System V (`shmget` / `shmat` / `shmdt` / `shmctl`) is the older API. It uses integer keys (from `ftok`), has arcane `ipcs`/`ipcrm` management, and does not compose with `mmap`. POSIX (`shm_open` + `mmap`) gives you a file descriptor, works with `fstat`, `ftruncate`, `fcntl`, and fits the fd mental model from Day 1. Prefer POSIX. Know System V exists because you'll see it in legacy codebases and interview questions.

**Persistence:**

A POSIX shared memory segment exists in the kernel until `shm_unlink` is called — not until processes exit. A process crash does not clean up the segment. You can `ls /dev/shm` to see live segments. This is both a feature (crash recovery can reattach) and a hazard (leaked segments accumulate). Always have a cleanup path.

---

## 2. Deep dive: the full lifecycle

```c
// lifecycle.c — annotated shm_open/mmap example
// Build: gcc -Wall -Wextra -o lifecycle lifecycle.c -lrt
// Run:   ./lifecycle (creates, writes, reads, then cleans up)

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/ipc_lab_shm"
#define SHM_SIZE 4096

int main(void) {
    // 1. Open (or create) the segment — returns an fd like any file
    //    O_CREAT | O_EXCL: fail if already exists (use O_CREAT alone to attach)
    int fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "segment exists — run: rm /dev/shm%s\n", SHM_NAME);
        } else {
            perror("shm_open");
        }
        return 1;
    }

    // 2. Set the size — MUST do this before mmap; a new segment has size 0
    if (ftruncate(fd, SHM_SIZE) < 0) { perror("ftruncate"); return 1; }

    // 3. Map into this process's address space
    void *addr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { perror("mmap"); return 1; }

    // fd is no longer needed after mmap — the mapping keeps the segment alive
    close(fd);

    // 4. Use the segment
    char *s = addr;
    strcpy(s, "hello from process A");
    printf("wrote: %s\n", s);

    // Inspect in another terminal: ls -la /dev/shm && cat /dev/shm/ipc_lab_shm

    // 5. Unmap this process's mapping (does NOT destroy the segment)
    if (munmap(addr, SHM_SIZE) < 0) { perror("munmap"); return 1; }

    // 6. Destroy the segment — this is the only call that frees the RAM
    if (shm_unlink(SHM_NAME) < 0) { perror("shm_unlink"); return 1; }

    puts("segment created, used, and cleaned up");
    return 0;
}
```

**Critical distinction:**
- `munmap` removes the mapping from this process. Other processes' mappings are unaffected.
- `shm_unlink` removes the name from the namespace. Existing mappings continue to work (the segment stays alive until all mappings are unmapped). This is the same unlink-vs-close semantics as regular files.

---

## 3. Deep dive: the ring buffer — broken then fixed

A ring buffer in shared memory is the canonical IPC data structure. Producer writes at `head`; consumer reads from `tail`. When `head == tail`, buffer is empty; when `(head + 1) % N == tail`, buffer is full.

**Version 1 — broken (no synchronization):**

```c
// ring_broken.h — DO NOT use in production; illustrates the problem
#include <stdint.h>
#define RING_SIZE 64

typedef struct {
    uint64_t head;   // plain integer — no atomicity
    uint64_t tail;
    int64_t  data[RING_SIZE];
} RingBuffer;

// Producer writes head without any fence.
// Consumer reads tail and data without any fence.
// On a multi-core system these writes may be reordered by the CPU
// or compiler. The consumer can see a stale head while new data is
// already in data[], or see an updated head while data[] still has
// old values. The bug is non-deterministic and load-dependent.
```

Run two processes using this. Under light load it works. Under load (add a `sched_yield()` loop or a stress-ng job), the consumer sees corrupt or stale data. The corrupted output is the demonstration.

**Version 2 — fixed (acquire/release atomics):**

```cpp
// ring_fixed.h — correct SPSC ring buffer for shared memory
// Requires C++20 (std::atomic with lock_free guarantee for uint64_t)
// Build: g++ -std=c++20 -Wall -Wextra -O2 -o producer ring_producer.cpp -lrt
//        g++ -std=c++20 -Wall -Wextra -O2 -o consumer ring_consumer.cpp -lrt

#pragma once
#include <atomic>
#include <cstdint>
#include <optional>

inline constexpr std::size_t RING_SIZE = 64;

struct RingBuffer {
    // head and tail are the only shared control variables.
    // Align to cache lines to prevent false sharing.
    alignas(64) std::atomic<uint64_t> head{0};
    alignas(64) std::atomic<uint64_t> tail{0};
    int64_t data[RING_SIZE];

    // Called by producer
    bool try_push(int64_t value) {
        uint64_t h = head.load(std::memory_order_relaxed);
        uint64_t next = (h + 1) % RING_SIZE;
        // Check full: if next == tail, no space
        if (next == tail.load(std::memory_order_acquire))
            return false;
        data[h] = value;
        // release: ensures data[h] write is visible before head update
        head.store(next, std::memory_order_release);
        return true;
    }

    // Called by consumer
    std::optional<int64_t> try_pop() {
        uint64_t t = tail.load(std::memory_order_relaxed);
        // acquire: ensures we see data[] writes that happened before head store
        if (t == head.load(std::memory_order_acquire))
            return std::nullopt;  // empty
        int64_t value = data[t];
        tail.store((t + 1) % RING_SIZE, std::memory_order_release);
        return value;
    }
};

static_assert(std::atomic<uint64_t>::is_always_lock_free,
              "uint64_t atomics must be lock-free for shared memory use");
```

**Why these memory orders:**

- `head.load(relaxed)` in `try_push`: we only read our own variable. No ordering needed.
- `tail.load(acquire)` in `try_push`: we're checking if the consumer has advanced. The acquire pairs with the consumer's `release` store to `tail`, ensuring we see the consumer's completed reads before concluding there's space.
- `head.store(release)` in `try_push`: makes `data[h] = value` visible to the consumer before the head update. The consumer's `acquire` load of `head` pairs with this.

The invariant: the consumer never sees an updated `head` without also seeing the `data[]` write that preceded it.

**One warning about atomics in shared memory:**

`std::atomic<T>` is lock-free on most platforms for `uint64_t`, but confirm with `is_always_lock_free`. If the atomic uses an internal lock (mutex), that lock is local to the process — it will not work across processes. Always assert `is_always_lock_free` for any atomic you put in shared memory.

---

## 4. Lab

### Setup

```bash
# Remove any leftover segment from previous runs
rm -f /dev/shm/ipc_lab_shm

# Build
g++ -std=c++20 -Wall -Wextra -O2 -o ring_producer ring_producer.cpp -lrt
g++ -std=c++20 -Wall -Wextra -O2 -o ring_consumer ring_consumer.cpp -lrt
```

### Tasks

**Task 1 — Lifecycle walkthrough**

Build and run `lifecycle.c`. While it pauses (add a `sleep(10)` before `munmap`), in another terminal:
```bash
ls -la /dev/shm/
cat /proc/$(pgrep lifecycle)/maps | grep ipc_lab
```

**Done when:** You can see the segment in `/dev/shm` and its mapping in `/proc/<pid>/maps`.

**Task 2 — Broken ring buffer**

Write `ring_producer.cpp` and `ring_consumer.cpp` using the `RingBuffer` struct with plain `uint64_t` (no atomics). Run both. Then add a CPU spinner in a third terminal:
```bash
stress-ng --cpu 4 --timeout 10s   # or: while true; do :; done &
```

**Done when:** You observe out-of-order or corrupted sequence numbers in the consumer output.

**Task 3 — Fixed ring buffer**

Swap in `ring_fixed.h`. Rebuild. Repeat the stress test.

```cpp
// ring_producer.cpp
#include "ring_fixed.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#define SHM_NAME "/ipc_lab_shm"

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(RingBuffer)) < 0) { perror("ftruncate"); return 1; }

    auto *rb = static_cast<RingBuffer *>(
        mmap(nullptr, sizeof(RingBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (rb == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    // Initialize the ring buffer (producer owns setup)
    new (rb) RingBuffer{};

    for (int64_t i = 0; i < 1000000; ++i) {
        while (!rb->try_push(i))
            ;  // spin until space available
    }
    // Sentinel: signal consumer to stop
    while (!rb->try_push(-1))
        ;

    munmap(rb, sizeof(RingBuffer));
    shm_unlink(SHM_NAME);
    puts("[producer] done");
    return 0;
}
```

```cpp
// ring_consumer.cpp
#include "ring_fixed.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

#define SHM_NAME "/ipc_lab_shm"

int main() {
    // Wait for producer to create segment
    int fd = -1;
    while (fd < 0) {
        fd = shm_open(SHM_NAME, O_RDWR, 0600);
        if (fd < 0) usleep(1000);
    }
    if (ftruncate(fd, sizeof(RingBuffer)) < 0) { perror("ftruncate"); return 1; }

    auto *rb = static_cast<RingBuffer *>(
        mmap(nullptr, sizeof(RingBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (rb == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    int64_t expected = 0;
    int64_t errors = 0;
    for (;;) {
        auto val = rb->try_pop();
        if (!val) continue;
        if (*val == -1) break;  // sentinel
        if (*val != expected) {
            fprintf(stderr, "ERROR: expected %ld got %ld\n", expected, *val);
            ++errors;
        }
        expected = *val + 1;
    }
    printf("[consumer] done. errors: %ld / %ld\n", errors, expected);
    munmap(rb, sizeof(RingBuffer));
    return 0;
}
```

**Done when:** Consumer reports 0 errors across 1,000,000 values, even under `stress-ng` load.

**Task 4 — Inspect with pmap**

```bash
# While producer is running (add a sleep before shm_unlink):
pmap $(pgrep ring_producer)
cat /proc/$(pgrep ring_producer)/maps
```

Find the `ipc_lab_shm` entry. Note the virtual address, size, and permissions.

**Done when:** You can identify the shm segment in the process's memory map.

---

## 5. Common pitfalls

- **Forgetting `ftruncate`.** A new shared memory segment has size 0. `mmap` on a size-0 segment returns `MAP_FAILED` (or maps nothing useful). Always `ftruncate` immediately after `shm_open` with `O_CREAT`.

- **Putting pointers in shared memory.** Virtual addresses differ between processes. A pointer valid in Process A points to random memory in Process B. Use offsets from the base of the segment, not pointers.

- **Placing non-lock-free atomics in shared memory.** A mutex-backed `std::atomic` embeds a mutex that is local to one process's address space. Cross-process use is undefined behavior. Assert `is_always_lock_free`.

- **Not calling `shm_unlink`.** The segment persists after process exit. Leaked segments accumulate in `/dev/shm` and consume RAM. Use a signal handler or `atexit` to ensure cleanup. On restart, `O_CREAT | O_EXCL` will fail if a stale segment exists.

- **Assuming cache coherence means memory ordering.** x86 has strong memory ordering (TSO), but ARM does not. Code that works on x86 without fences may fail on ARM embedded targets. Write to the C++ memory model, not to x86 assumptions.

- **Racing on initialization.** Both processes call `shm_open` + `mmap` and then one initializes the buffer while the other is already reading it. Use a separate synchronization primitive (or an atomic flag in the segment itself) to signal "initialization complete."

---

## 6. Interview drills

**Q: Walk me through what happens between `shm_open` and `mmap`.**

`shm_open` creates or opens a POSIX shared memory object backed by a file descriptor pointing to an anonymous file in `tmpfs` (on Linux, under `/dev/shm`). `ftruncate` sets its size, allocating the backing pages. `mmap` with `MAP_SHARED` asks the kernel to map those pages into this process's virtual address space — the kernel updates the process's page table to point at the physical frames. The fd can then be closed; the mapping holds a reference to the underlying object. A second process calling `shm_open` with the same name gets an fd to the same object; its `mmap` maps the same physical frames at a (probably different) virtual address.

*Follow-up: "Why can you close the fd after mmap?"* — The mapping holds its own reference count on the object. Closing the fd decrements the fd reference but not the mmap reference. The pages stay mapped.

---

**Q: Two processes share memory. One writes a 64-bit value. Is the read on the other side atomic?**

Not guaranteed. On x86-64, naturally aligned 64-bit stores are atomic at the hardware level, but the C++ memory model does not guarantee this without `std::atomic`. The compiler is free to split the store into two 32-bit operations, or to reorder it. Use `std::atomic<uint64_t>` with the appropriate memory order. Anything less is undefined behavior in C++ and a data race in practice on non-x86 architectures.

*Follow-up: "What memory order for a simple flag?"* — `memory_order_release` on the writer, `memory_order_acquire` on the reader. `seq_cst` is correct but adds unnecessary cost on ARM (a full barrier).

---

**Q: How do you cleanly tear down shared memory if a process crashes?**

Three strategies, often combined: (1) A supervisor process monitors the workers and calls `shm_unlink` on detected death. (2) Install a signal handler (`SIGTERM`, `SIGINT`, `SIGSEGV`) that calls `shm_unlink` before exiting — `SIGSEGV` handlers are unreliable but better than nothing. (3) At startup, check for a stale segment: attempt `shm_open(O_CREAT|O_EXCL)`; if it fails with `EEXIST`, decide whether to reuse or unlink-and-recreate based on a validity marker in the segment. Robust mutexes (Day 3) handle the lock-holding-at-crash case.

---

**Q: Why is shared memory faster than pipes for high-throughput IPC?**

Pipes and FIFOs copy data twice: once from user space into the kernel buffer (on write), and once from the kernel buffer back to user space (on read). Shared memory has zero copies on the data path after setup — the producer writes directly to physical RAM that the consumer reads directly. The kernel is not involved per message. At high message rates or large message sizes, this difference dominates.

*Follow-up: "Is shared memory always faster?"* — No. For small infrequent messages, the synchronization overhead (atomic ops, cache line bouncing) can exceed the copy cost. Pipes also benefit from the kernel's copy-on-write and prefetch optimizations. Shared memory wins at high throughput and large payloads.

---

**Q: What are the struct layout rules for shared memory?**

No pointers (use offsets). Explicit padding to avoid struct holes that may differ between compilers or platforms. Alignment: place each field at its natural alignment. Include a version field so you can detect layout mismatches between processes compiled at different times. Use `static_assert(sizeof(MyStruct) == EXPECTED_SIZE)` and `static_assert(offsetof(MyStruct, field) == EXPECTED_OFFSET)` to catch surprises.

---

## 7. Cheatsheet

### Lifecycle

```
shm_open(name, O_CREAT|O_RDWR, 0600) → fd
ftruncate(fd, size)                   ← MUST before mmap on new segment
mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0) → ptr
close(fd)                             ← safe after mmap
--- use ptr ---
munmap(ptr, size)                     ← removes this process's mapping
shm_unlink(name)                      ← destroys the segment (do once)
```

### Key syscalls

| Syscall | Notes |
|---|---|
| `shm_open(name, flags, mode)` | Name must start with `/`; link with `-lrt` |
| `ftruncate(fd, size)` | Required on new segments; sets size |
| `mmap(addr, len, prot, MAP_SHARED, fd, offset)` | `MAP_SHARED` for IPC; `MAP_PRIVATE` for COW copy |
| `munmap(ptr, len)` | Unmap this process's view |
| `shm_unlink(name)` | Delete from namespace; last munmap frees RAM |

### Inspection

```bash
ls -la /dev/shm/                         # list all segments
cat /proc/<pid>/maps | grep shm          # see mappings
pmap <pid>                               # summary view
ipcs -m                                  # System V segments (different API)
```

### Atomic ring buffer memory orders (SPSC)

| Operation | Memory order | Why |
|---|---|---|
| Producer: load own head | `relaxed` | No ordering needed |
| Producer: load tail (check full) | `acquire` | Sync with consumer's tail release |
| Producer: store data | plain | Ordered by following release |
| Producer: store head | `release` | Makes data visible to consumer |
| Consumer: load own tail | `relaxed` | No ordering needed |
| Consumer: load head (check empty) | `acquire` | Sync with producer's head release |
| Consumer: store tail | `release` | Signals space to producer |

### Struct rules for shared memory
- No raw pointers — use `ptrdiff_t` offsets from segment base
- Natural alignment; add explicit padding
- `static_assert(sizeof(S) == N)` and `static_assert(offsetof(S, f) == M)`
- Version/magic field for compatibility checks
- `std::atomic<T>::is_always_lock_free` must be true

---

## 8. Further reading

- Kerrisk, *The Linux Programming Interface*, Chapters 48–54 — POSIX and System V shared memory, thorough
- `man 7 shm_overview` — short; read it
- cppreference: `std::memory_order` — the authoritative C++ memory model reference