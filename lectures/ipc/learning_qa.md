# Embedded Systems Interview Q&A — Complete Reference

**Topics:** POSIX IPC · C++ Modern/Concurrency · Python Scripting · JSON
**Target role:** Systems / Embedded Engineer
**Format:** Question → Model Answer → Code where applicable → Follow-up traps to expect

---

## Table of Contents

1. [POSIX IPC — Named Pipes (FIFOs)](#section-1-posix-ipc--named-pipes-fifos)
2. [POSIX IPC — Shared Memory](#section-2-posix-ipc--shared-memory)
3. [POSIX IPC — Semaphores & Mutexes](#section-3-posix-ipc--semaphores--mutexes)
4. [POSIX IPC — Design & Trade-offs](#section-4-posix-ipc--design--trade-offs)
5. [C++ — Memory Model & Atomics](#section-5-c--memory-model--atomics)
6. [C++ — Concurrency Primitives](#section-6-c--concurrency-primitives)
7. [C++ — Modern STL & Language Features](#section-7-c--modern-stl--language-features)
8. [C++ — Systems-Level Patterns](#section-8-c--systems-level-patterns)
9. [Python — Automation & Scripting](#section-9-python--automation--scripting)
10. [Python — Testing](#section-10-python--testing)
11. [Python — Concurrency & Performance](#section-11-python--concurrency--performance)
12. [JSON — Schema & Design](#section-12-json--schema--design)
13. [JSON — Parsing & C++/Python Integration](#section-13-json--parsing--cpython-integration)
14. [System Design — IPC Architecture](#section-14-system-design--ipc-architecture)
15. [Debugging & Observability](#section-15-debugging--observability)

---

## Section 1: POSIX IPC — Named Pipes (FIFOs)

---

### Q1. What is the difference between an anonymous pipe and a named pipe (FIFO)?

**Answer:**
An anonymous pipe (`pipe()`) is created with two file descriptors (read end, write end) that exist only in the kernel. It has no filesystem entry, so only related processes (parent/child sharing the fd after `fork`) can use it. It disappears when both ends are closed.

A named pipe (FIFO) is created with `mkfifo()` and appears as a special file in the filesystem. Any process that knows the path can open it — no shared ancestry required. The file persists on disk until explicitly unlinked, but the data buffer lives in kernel memory, not on disk.

```c
// Anonymous pipe — only usable across fork
int fds[2];
pipe(fds);           // fds[0] = read end, fds[1] = write end
pid_t pid = fork();
if (pid == 0) {
    close(fds[1]);   // child: close write end
    char buf[64];
    read(fds[0], buf, sizeof(buf));
} else {
    close(fds[0]);   // parent: close read end
    write(fds[1], "hello", 5);
}

// Named pipe — any process can open by path
// Terminal 1:
mkfifo("/tmp/myfifo", 0666);
int fd = open("/tmp/myfifo", O_WRONLY);   // blocks until reader appears
write(fd, "hello", 5);

// Terminal 2:
int fd = open("/tmp/myfifo", O_RDONLY);   // unblocks the writer's open()
char buf[64];
read(fd, buf, 5);
```

**Follow-up trap:** "What happens if you open a FIFO with `O_RDWR`?"
It unblocks immediately (a single fd satisfies both ends), but this is non-portable and generally a design smell. The correct answer is: use `O_NONBLOCK` on one end if you need to avoid the blocking open.

---

### Q2. Explain the blocking semantics of FIFO `open()`.

**Answer:**
Opening a FIFO blocks by default until the other end is also opened:
- `open(path, O_RDONLY)` blocks until another process opens it for writing.
- `open(path, O_WRONLY)` blocks until another process opens it for reading.

This symmetry ensures both ends are ready before data flows. It's often surprising because `open()` on regular files never blocks.

To avoid this: pass `O_NONBLOCK`. On the read end, `O_NONBLOCK` causes `open()` to succeed immediately even without a writer. On the write end, `O_NONBLOCK` causes `open()` to return `ENXIO` immediately if no reader is present.

```c
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

int fd = open("/tmp/myfifo", O_WRONLY | O_NONBLOCK);
if (fd == -1) {
    if (errno == ENXIO) {
        printf("No reader present yet\n");
    } else {
        perror("open");
    }
}
```

**Follow-up trap:** "In a producer/consumer system, which end should use `O_NONBLOCK`?"
Usually the producer, because a writer blocked forever with no reader is a hang, not a backpressure situation. Use `poll()`/`epoll` to wait for the reader to appear rather than blocking in `open()`.

---

### Q3. What is SIGPIPE and when does it occur with FIFOs?

**Answer:**
`SIGPIPE` is sent to a process when it writes to a pipe or FIFO that has no open readers. The default signal handler terminates the process. This is commonly surprising: if a consumer dies while the producer is still running, the producer gets killed by `SIGPIPE` rather than seeing an error from `write()`.

To handle this properly, either:
1. Ignore `SIGPIPE` (`signal(SIGPIPE, SIG_IGN)`) — `write()` then returns `-1` with `errno == EPIPE`.
2. Use `send()` with `MSG_NOSIGNAL` (for sockets; not available on FIFOs).
3. Use `sigaction` to install a custom handler.

```c
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int main(void) {
    // Option 1: ignore the signal, check errno on write()
    signal(SIGPIPE, SIG_IGN);

    int fd = open("/tmp/myfifo", O_WRONLY);
    while (1) {
        ssize_t n = write(fd, "data", 4);
        if (n == -1) {
            if (errno == EPIPE) {
                fprintf(stderr, "Reader gone — EPIPE\n");
                break;  // clean exit instead of crash
            }
            perror("write");
            break;
        }
    }
}
```

**Follow-up trap:** "Does `SIGPIPE` apply to sockets too?"
Yes — any write to a socket with no receiver. In server code, ignoring `SIGPIPE` globally is standard practice.

---

### Q4. What is `PIPE_BUF` and what does atomicity mean here?

**Answer:**
`PIPE_BUF` is the maximum number of bytes that the kernel guarantees will be written atomically to a pipe or FIFO in a single `write()` call. On Linux it is 4096 bytes (POSIX guarantees at least 512).

"Atomic" means: if multiple writers each write ≤ `PIPE_BUF` bytes in a single `write()`, their writes will not be interleaved. Readers will see complete messages. If any writer writes more than `PIPE_BUF` bytes, the kernel may split the write, and data from multiple writers can be interleaved.

This is critical for multi-writer single-reader patterns (e.g., logging from multiple processes):

```c
#include <limits.h>   // PIPE_BUF
#include <stdio.h>

// Safe: each write is atomic (assuming msg_len <= PIPE_BUF)
struct LogEntry {
    pid_t  pid;
    int    level;
    char   text[PIPE_BUF - sizeof(pid_t) - sizeof(int)];
};

// As long as sizeof(LogEntry) <= PIPE_BUF, writes are atomic
// Even with 10 concurrent writers, the reader sees complete entries
```

**Follow-up trap:** "How do you handle messages larger than `PIPE_BUF`?"
Use a length-prefix framing protocol or switch to a UNIX domain socket with `SOCK_SEQPACKET` for guaranteed message boundaries.

---

### Q5. How would you implement a non-blocking FIFO consumer using `poll()`?

**Answer:**
Open the FIFO with `O_NONBLOCK`, then use `poll()` to wait for data without burning CPU. When `poll()` returns `POLLIN`, call `read()`. If `read()` returns 0, the last writer closed — handle EOF.

```c
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {
    int fd = open("/tmp/myfifo", O_RDONLY | O_NONBLOCK);
    if (fd == -1) { perror("open"); return 1; }

    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    while (1) {
        int ret = poll(&pfd, 1, 5000);  // 5s timeout
        if (ret == 0) {
            printf("timeout — no data\n");
            continue;
        }
        if (ret < 0) { perror("poll"); break; }

        if (pfd.revents & POLLIN) {
            char buf[256];
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n == 0) {
                printf("EOF — all writers closed\n");
                break;
            }
            if (n > 0) {
                printf("Read %zd bytes\n", n);
            }
        }
        if (pfd.revents & POLLHUP) {
            // Writer closed — may still have data buffered; drain first
            printf("POLLHUP received\n");
        }
    }
    close(fd);
}
```

**Follow-up trap:** "What's the difference between `POLLHUP` and `EOF` (read returning 0)?"
`POLLHUP` indicates the write end closed, but there may still be unread data in the buffer. Always drain readable data after `POLLHUP` before treating it as done.

---

### Q6. Can you use a single FIFO for bidirectional communication?

**Answer:**
No, not safely. A FIFO is unidirectional. If a process writes to and reads from the same FIFO, it may read its own writes — the kernel doesn't distinguish by writer. For bidirectional communication, use two FIFOs (one per direction), a UNIX domain socket (`SOCK_STREAM` or `SOCK_SEQPACKET`), or a different IPC mechanism entirely.

```
Process A                           Process B
   │                                    │
   │── write ──► fifo_a_to_b ──► read──►│
   │                                    │
   │◄── read ◄── fifo_b_to_a ◄── write─│
```

```c
// Conventional naming
mkfifo("/tmp/a_to_b", 0600);
mkfifo("/tmp/b_to_a", 0600);

// Process A
int write_fd = open("/tmp/a_to_b", O_WRONLY);
int read_fd  = open("/tmp/b_to_a", O_RDONLY);

// Process B (opens in opposite order to avoid deadlock)
int read_fd  = open("/tmp/a_to_b", O_RDONLY);
int write_fd = open("/tmp/b_to_a", O_WRONLY);
```

**Follow-up trap:** "Why might both processes deadlock on `open()`?"
If both try to open their write-end first while waiting for the other's read-end, and both block — deadlock. Solution: open in opposite order (A: write then read; B: read then write), or use `O_NONBLOCK` for opens.

---

### Q7. How does data persist in a FIFO across process restarts?

**Answer:**
It doesn't. A FIFO's data buffer lives in kernel memory. When all file descriptors to the FIFO are closed, the buffer is discarded. The filesystem entry (the inode) persists until `unlink()` is called, but the data is gone.

This means if a consumer crashes mid-read, unread data in the pipe is lost. If durability is needed, write to a regular file with `fsync()` or use a message queue with persistence (like a database-backed queue, not POSIX mqueues which also lose data on reboot by default).

**Follow-up trap:** "How do POSIX message queues (`mq_open`) differ in this regard?"
Same — message queues live in kernel memory and data is lost on reboot unless the OS specifically persists them (Linux does not by default). They are mounted under `/dev/mqueue`.

---

## Section 2: POSIX IPC — Shared Memory

---

### Q8. Walk me through the complete lifecycle of a POSIX shared memory segment.

**Answer:**
Six steps: create, size, map, use, unmap, unlink.

```c
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 4096

// --- CREATOR PROCESS ---
int main_creator(void) {
    // 1. Create (or open) the shm object
    //    O_CREAT|O_EXCL fails if it already exists — prevents silent reuse
    int fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) { perror("shm_open"); return 1; }

    // 2. Set size (shm starts at size 0)
    if (ftruncate(fd, SHM_SIZE) == -1) { perror("ftruncate"); return 1; }

    // 3. Map into address space
    void *ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    close(fd);  // fd no longer needed after mmap

    // 4. Use
    strcpy((char *)ptr, "hello from creator");

    // 5. Unmap when done with this process's usage
    munmap(ptr, SHM_SIZE);

    // 6. Unlink the name (object still accessible to mapped processes)
    shm_unlink(SHM_NAME);  // typically done by the creator at teardown
    return 0;
}

// --- CONSUMER PROCESS ---
int main_consumer(void) {
    // Open existing segment (no O_CREAT)
    int fd = shm_open(SHM_NAME, O_RDONLY, 0);
    if (fd == -1) { perror("shm_open"); return 1; }

    void *ptr = mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    printf("Read: %s\n", (char *)ptr);
    munmap(ptr, SHM_SIZE);
    return 0;
}
```

**Follow-up trap:** "What happens if you call `shm_unlink` while another process still has it mapped?"
The name is removed from the filesystem (no new processes can open it by name), but existing mappings remain valid. The kernel keeps the underlying object alive until all mappings are released. This is the standard teardown pattern — unlink early, mappings persist.

---

### Q9. Why isn't shared memory sufficient for synchronization on its own?

**Answer:**
Shared memory gives you a shared address space — nothing more. There is no implicit ordering or exclusion. Two processes writing concurrently cause data races: torn writes, stale cache lines, and undefined behavior. Even writing a single `int` is not guaranteed to be atomic on all architectures (though it usually is for aligned 32-bit writes on x86).

The canonical demonstration:

```c
// Shared struct with NO synchronization — data race
struct Counter {
    int value;  // plain int, not atomic
};

// Process A:
shared->value++;   // read-modify-write, NOT atomic

// Process B (simultaneous):
shared->value++;   // same — both read 0, both write 1, net result is 1 not 2
```

Fix: use `_Atomic int` (C11) or `std::atomic<int>` with appropriate memory order, OR protect with a process-shared mutex.

```c
#include <stdatomic.h>

struct SafeCounter {
    _Atomic int value;  // C11 atomic
};

// Both processes:
atomic_fetch_add_explicit(&shared->value, 1, memory_order_relaxed);
// Relaxed is fine here — we just want atomicity, not ordering
```

**Follow-up trap:** "What's the difference between atomicity and memory ordering?"
Atomicity: the operation completes as one indivisible unit — no torn read/write.
Memory ordering: the visibility order of operations to other threads/processes. An atomic increment can be atomic but still not visible to another core in a deterministic order without the right memory fence.

---

### Q10. How do you place a mutex inside shared memory for cross-process locking?

**Answer:**
Use `pthread_mutexattr_setpshared(attr, PTHREAD_PROCESS_SHARED)`. By default, a pthread mutex can only be shared between threads of the same process. With `PROCESS_SHARED`, it can be placed in shared memory and used by multiple processes.

```c
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct {
    pthread_mutex_t mutex;
    int             data;
} SharedState;

SharedState *create_shared_state(const char *name) {
    int fd = shm_open(name, O_CREAT | O_RDWR, 0600);
    ftruncate(fd, sizeof(SharedState));
    SharedState *s = mmap(NULL, sizeof(SharedState),
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    // Configure mutex for cross-process use
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    // IMPORTANT: also make it robust (see Q11)
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

    pthread_mutex_init(&s->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    return s;
}

// Any process with the mapping can now lock:
void increment(SharedState *s) {
    pthread_mutex_lock(&s->mutex);
    s->data++;
    pthread_mutex_unlock(&s->mutex);
}
```

**Follow-up trap:** "What happens if the process holding the mutex crashes?"
Without `PTHREAD_MUTEX_ROBUST`: other processes block on `pthread_mutex_lock()` forever — deadlock. With `PTHREAD_MUTEX_ROBUST`: `pthread_mutex_lock()` returns `EOWNERDEAD`. The new owner must then call `pthread_mutex_consistent()` to mark the protected state as clean, then unlock. See Q11.

---

### Q11. What is a robust mutex and when do you need one?

**Answer:**
A robust mutex is one where the kernel detects that the owning process/thread died while holding the lock, and returns `EOWNERDEAD` to the next waiter instead of hanging forever. Critical for embedded/server systems where processes can crash independently.

```c
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

void safe_lock(pthread_mutex_t *m, int *data_is_consistent) {
    int ret = pthread_mutex_lock(m);

    if (ret == EOWNERDEAD) {
        // Previous owner died holding the lock.
        // The protected data may be in an inconsistent state.
        fprintf(stderr, "Previous owner died — recovering\n");

        // Inspect/repair the protected data here
        *data_is_consistent = 0;  // signal caller to validate/reset

        // Mark mutex as consistent so others can use it
        pthread_mutex_consistent(m);
        // We now hold the lock — proceed, then unlock normally
    } else if (ret != 0) {
        fprintf(stderr, "lock failed: %d\n", ret);
    }
}
```

**Follow-up trap:** "What if you don't call `pthread_mutex_consistent`?"
On the next `pthread_mutex_unlock`, the mutex enters the `ENOTRECOVERABLE` state permanently — all future lock attempts return `ENOTRECOVERABLE` and the mutex is unusable without re-initialization. This is intentional: forcing the developer to consciously repair state before proceeding.

---

### Q12. What is the difference between `shm_open` and `mmap` with `MAP_ANONYMOUS`?

**Answer:**
`shm_open` + `mmap(MAP_SHARED)`: creates a named, sharable memory object backed by a file descriptor. Multiple unrelated processes can map the same object by name. Data is visible across all mappers.

`mmap(MAP_ANONYMOUS | MAP_SHARED)`: creates an unnamed shared mapping. Only processes related by `fork()` can share it (the child inherits the mapping). No filesystem name, no `shm_open` needed, but not accessible to arbitrary processes.

```c
// MAP_ANONYMOUS | MAP_SHARED — only useful across fork
void *ptr = mmap(NULL, 4096,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS,
                 -1, 0);   // fd = -1 for anonymous
fork();
// Both parent and child now share this mapping

// vs shm_open — any process can connect
int fd = shm_open("/shared", O_CREAT | O_RDWR, 0600);
ftruncate(fd, 4096);
void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

**Follow-up trap:** "Does `MAP_PRIVATE` share data between processes?"
No. `MAP_PRIVATE` uses copy-on-write. Each process gets its own private copy the moment it writes. Use `MAP_SHARED` for actual sharing.

---

### Q13. How would you design a lock-free ring buffer in shared memory?

**Answer:**
Use two `_Atomic` (or `std::atomic`) indices — head (writer-owned) and tail (reader-owned). The writer advances head; the reader advances tail. Full when `head - tail == capacity`; empty when `head == tail`. Key: indices never wrap; use modulo for slot access. This avoids the classic wrap ambiguity.

```cpp
#include <atomic>
#include <cstdint>
#include <cstring>
#include <optional>

// Place this struct directly in shared memory
template <typename T, uint32_t N>
struct SPSCQueue {
    // N must be power of 2 for cheap modulo
    static_assert((N & (N - 1)) == 0, "N must be power of 2");

    alignas(64) std::atomic<uint64_t> head{0};  // writer increments
    alignas(64) std::atomic<uint64_t> tail{0};  // reader increments
    T slots[N];

    bool push(const T& item) {
        uint64_t h = head.load(std::memory_order_relaxed);
        uint64_t t = tail.load(std::memory_order_acquire);
        if (h - t == N) return false;  // full

        slots[h & (N - 1)] = item;
        head.store(h + 1, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        uint64_t t = tail.load(std::memory_order_relaxed);
        uint64_t h = head.load(std::memory_order_acquire);
        if (h == t) return std::nullopt;  // empty

        T item = slots[t & (N - 1)];
        tail.store(t + 1, std::memory_order_release);
        return item;
    }
};
```

**Why `alignas(64)` on head and tail?**
False sharing: if head and tail share a cache line, every write to either invalidates the other core's cache, creating contention. Aligning each to a cache line boundary prevents this.

**Follow-up trap:** "Is this safe for multiple producers?"
No. This is SPSC (single-producer, single-consumer). For MPSC, the head update needs a CAS loop (`compare_exchange_weak`). For MPMC, both indices do.

---

### Q14. How do you investigate what shared memory segments are active on a Linux system?

**Answer:**
POSIX shared memory segments live under `/dev/shm` — they're files on a tmpfs mount.

```bash
# List all active POSIX shm segments
ls -lh /dev/shm/

# See which processes have it mapped
lsof /dev/shm/my_shm

# See the segment in a process's memory map
cat /proc/<pid>/maps | grep my_shm

# Check size and details
stat /dev/shm/my_shm

# System V shared memory (older API)
ipcs -m                   # list all sysv shm segments
ipcrm -m <shmid>          # delete one
```

**Follow-up trap:** "What happens to `/dev/shm` entries if the process crashes without calling `shm_unlink`?"
They persist until the next reboot (or until manually removed). This is a common resource leak. Always unlink in a signal handler or use a wrapper process that cleans up.

---

## Section 3: POSIX IPC — Semaphores & Mutexes

---

### Q15. What is the difference between a named semaphore and an unnamed semaphore?

**Answer:**
A named semaphore (`sem_open`) has a filesystem name (under `/dev/shm` or `/sem` on Linux), can be opened by unrelated processes, and persists until `sem_unlink` is called. An unnamed semaphore (`sem_init`) lives in memory (stack, heap, or shared memory), has no name, and its lifetime is tied to the memory it's placed in.

For cross-process synchronization without a name, use `sem_init` with `pshared=1` and place the semaphore in shared memory.

```c
#include <semaphore.h>
#include <sys/mman.h>

// Named semaphore — any process can find it by name
sem_t *named = sem_open("/my_sem", O_CREAT, 0600, 1);
sem_wait(named);
// ... critical section ...
sem_post(named);
sem_close(named);
sem_unlink("/my_sem");  // remove name

// Unnamed semaphore in shared memory — two processes share
typedef struct { sem_t sem; int data; } Shared;
Shared *s = /* mmap shared memory */;
sem_init(&s->sem, 1, 1);  // pshared=1, initial value=1
sem_wait(&s->sem);
// ... critical section ...
sem_post(&s->sem);
sem_destroy(&s->sem);
```

**Follow-up trap:** "On macOS, does unnamed semaphore in shared memory work?"
No. macOS does not implement `sem_init` with `pshared=1` — it returns `ENOSYS`. Use named semaphores on macOS, or use a `pthread_mutex` with `PTHREAD_PROCESS_SHARED`.

---

### Q16. Explain the difference between a semaphore and a mutex. When do you use each?

**Answer:**

| Property | Mutex | Semaphore |
|---|---|---|
| Ownership | The locker must be the unlocker | Anyone can post |
| Count | Binary (locked/unlocked) | Counting (0 to N) |
| Purpose | Mutual exclusion of a resource | Signaling / resource counting |
| Priority inheritance | Supported (platform-specific) | Generally not |
| Typical use | Protecting a data structure | Producer-consumer signaling |

Use a **mutex** when: one thread accesses a shared data structure, and the same thread that locks must unlock. Use a **semaphore** when: you want to signal from one thread/process to another (e.g., "an item is ready"), or to count available resources (e.g., a pool of 4 connections).

```cpp
// Semaphore as signal (classic producer-consumer)
sem_t items_available;  // starts at 0
sem_t slots_free;       // starts at CAPACITY

// Producer:
sem_wait(&slots_free);       // block if no slots
enqueue(item);
sem_post(&items_available);  // signal consumer

// Consumer:
sem_wait(&items_available);  // block if no items
item = dequeue();
sem_post(&slots_free);       // signal producer
```

**Follow-up trap:** "Can you implement a mutex with a semaphore?"
Yes — initialize the semaphore to 1, and `sem_wait` / `sem_post` act as lock/unlock. But you lose ownership tracking, which means you lose deadlock detection and priority inheritance. It's a conceptual tool, not a practical replacement.

---

### Q17. What is priority inversion and how do you mitigate it?

**Answer:**
Priority inversion occurs when a high-priority task is blocked waiting for a resource held by a low-priority task, and a medium-priority task preempts the low-priority task — so the medium-priority task indirectly blocks the high-priority one.

Classic example: Mars Pathfinder (1997) suffered a reset storm caused by priority inversion on a VxWorks mutex without priority inheritance enabled.

**Mitigations:**

1. **Priority inheritance:** The low-priority task temporarily inherits the priority of the highest-priority waiter. `pthread_mutexattr_setprotocol(attr, PTHREAD_PRIO_INHERIT)`.
2. **Priority ceiling:** The mutex has a fixed ceiling priority; any locker runs at that ceiling. `PTHREAD_PRIO_PROTECT`. Faster than inheritance but requires knowing the max priority upfront.
3. **Lock-free algorithms:** Eliminate the mutex entirely.

```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
pthread_mutex_t m;
pthread_mutex_init(&m, &attr);
pthread_mutexattr_destroy(&attr);
```

**Follow-up trap:** "Does POSIX guarantee priority inheritance is implemented?"
No — it's optional. Check `_POSIX_THREAD_PRIO_INHERIT` at compile time. On Linux with NPTL it is supported; on some RTOSes it must be configured.

---

### Q18. What does `sem_timedwait` do and when do you need it?

**Answer:**
`sem_timedwait` blocks until the semaphore value becomes > 0 OR the absolute deadline is reached. On timeout, it returns `-1` with `errno == ETIMEDOUT`. Unlike `sem_wait`, it prevents indefinite hangs — essential in embedded systems where a dead sender must not freeze a receiver forever.

```c
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

int timed_receive(sem_t *sem, int timeout_ms) {
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);  // MUST be CLOCK_REALTIME for sem_timedwait
    deadline.tv_sec  += timeout_ms / 1000;
    deadline.tv_nsec += (timeout_ms % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L) {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }

    int ret = sem_timedwait(sem, &deadline);
    if (ret == -1 && errno == ETIMEDOUT) {
        fprintf(stderr, "Timeout — sender may be dead\n");
        return -1;
    }
    return 0;
}
```

**Follow-up trap:** "Why does `sem_timedwait` use `CLOCK_REALTIME` and not `CLOCK_MONOTONIC`?"
POSIX mandates `CLOCK_REALTIME` for `sem_timedwait`. This is a known wart — if the system clock is adjusted (NTP step), the timeout may fire early or late. For monotonic timeouts, use a condition variable with `pthread_condattr_setclock(attr, CLOCK_MONOTONIC)`.

---

### Q19. How would you implement a counting semaphore using only a mutex and condition variable?

**Answer:**
This is a classic interview question to test whether you understand what semaphores actually do internally.

```cpp
#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(int initial) : count_(initial) {}

    void wait() {
        std::unique_lock<std::mutex> lk(mu_);
        cv_.wait(lk, [this]{ return count_ > 0; });
        --count_;
    }

    bool try_wait_for(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lk(mu_);
        bool ok = cv_.wait_for(lk, timeout, [this]{ return count_ > 0; });
        if (ok) --count_;
        return ok;
    }

    void post() {
        {
            std::lock_guard<std::mutex> lk(mu_);
            ++count_;
        }
        cv_.notify_one();  // outside lock: avoid missed wakeup + lock contention
    }

private:
    std::mutex              mu_;
    std::condition_variable cv_;
    int                     count_;
};
```

**Why `notify_one` outside the lock?**
Holding the lock during `notify_one` is legal but can cause a "hurry up and wait" pattern: the notified thread wakes up, immediately tries to lock `mu_`, and blocks — wasted context switch. Notifying outside the lock allows the waiter to acquire `mu_` immediately.

**Follow-up trap:** "What's a spurious wakeup and how does the predicate lambda handle it?"
A spurious wakeup is when `wait()` returns even though `notify` was not called — allowed by POSIX for implementation freedom. The predicate lambda (`[this]{ return count_ > 0; }`) re-checks the condition after wakeup; if false, `wait()` re-blocks automatically.

---

## Section 4: POSIX IPC — Design & Trade-offs

---

### Q20. Compare FIFOs, UNIX domain sockets, POSIX message queues, and shared memory. When do you choose each?

**Answer:**

| Mechanism | Throughput | Latency | Persistence | Message boundaries | Multi-process | Best for |
|---|---|---|---|---|---|---|
| FIFO | Moderate | Moderate | None | No | Yes (read splits) | Simple 1:1 streaming |
| UNIX socket (STREAM) | High | Low | None | No | Yes | Local RPC, bidirectional |
| UNIX socket (SEQPACKET) | High | Low | None | Yes | Yes | Message-oriented IPC |
| POSIX mqueue | Moderate | Moderate | None | Yes | Yes | Priority-ordered messaging |
| Shared memory | Highest | Lowest | None | No (you design) | Yes | High-throughput, low-latency data |

**Decision tree:**
- Need priorities? → `mq_open` (POSIX message queues)
- Need message boundaries without framing code? → `SOCK_SEQPACKET` Unix socket
- Need bidirectional? → Unix socket pair
- Need maximum throughput / minimum latency? → shared memory + synchronization
- Simple streaming, one direction? → FIFO
- Need to pass file descriptors between processes? → Unix socket with `SCM_RIGHTS`

**Follow-up trap:** "Can you pass a file descriptor through a FIFO?"
No. File descriptor passing requires a UNIX domain socket with `sendmsg`/`recvmsg` and `SCM_RIGHTS` ancillary data. This is a common embedded/systems interview question.

---

### Q21. Design an IPC architecture for a system with 1 sensor process, 4 worker processes, and 1 logger.

**Answer:**
This is a design question — defend choices, not just name mechanisms.

```
Sensor ──shm ring buffer──► Workers (1..4) ──FIFO──► Logger
           │                     │
           ▼                     ▼
    semaphore (items)     mutex (protect per-worker output)
```

**Sensor → Workers:** Shared memory ring buffer. Sensor writes at high frequency; workers poll or use semaphore notification. Use a process-shared mutex + counting semaphore (items_available). Each worker takes one item per acquisition. This gives zero-copy data path.

**Workers → Logger:** One FIFO per worker (not a shared FIFO), because writes > `PIPE_BUF` from multiple writers to the same FIFO would interleave. Alternatively, one shared FIFO with messages ≤ `PIPE_BUF` and a length-prefix framing convention.

**Control plane (start/stop/config):** Named UNIX socket or POSIX mqueue from a controller to each process. Mqueue is good here because it supports priority (e.g., STOP > UPDATE > START) and each process has its own queue.

```
Controller ──mqueue(priority)──► Sensor
           ──mqueue(priority)──► Worker[0..3]
           ──mqueue(priority)──► Logger
```

**Follow-up trap:** "What changes if workers need to send results back to the sensor?"
Add a result shared memory region (worker writes, sensor reads) with per-worker slots and atomic flags. Or add per-worker result FIFOs. The key insight is keeping data path and control path separate.

---

### Q22. How do you handle cleanup of IPC resources when a process crashes?

**Answer:**
POSIX IPC objects (shm, named semaphores, FIFOs) are **not** automatically cleaned up on process crash — unlike file descriptors, which the kernel closes. You need explicit cleanup strategy:

1. **Signal handlers** for `SIGTERM`, `SIGINT`, `SIGHUP`, `SIGABRT`: call `shm_unlink`, `sem_unlink`, `unlink` (for FIFOs).

```c
#include <signal.h>
static const char *shm_name = "/my_shm";
static int shm_fd = -1;

void cleanup_handler(int sig) {
    if (shm_fd != -1) shm_unlink(shm_name);
    // re-raise to get default behavior (core dump for SIGABRT)
    signal(sig, SIG_DFL);
    raise(sig);
}

int main(void) {
    signal(SIGTERM, cleanup_handler);
    signal(SIGINT,  cleanup_handler);
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0600);
    // ...
}
```

2. **Watchdog / supervisor process** (common in embedded): a separate process monitors PIDs, detects crashes, and runs cleanup.

3. **Unlink immediately after creation** (for shm/semaphores): unlink the name right after creating and mapping. Existing mappings remain valid, new processes can't open by name. Clean death guaranteed when last user exits.

```c
// "Anonymous named" shm pattern:
int fd = shm_open("/tmp_shm", O_CREAT | O_RDWR, 0600);
ftruncate(fd, SIZE);
void *p = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
shm_unlink("/tmp_shm");  // unlink immediately — mapping persists, name gone
close(fd);
// No cleanup needed on crash — kernel cleans up on last munmap
```

4. **`atexit()` handlers** for normal exits (not crash), combined with signal handlers.

---

## Section 5: C++ — Memory Model & Atomics

---

### Q23. Explain `memory_order_acquire` and `memory_order_release` with a concrete example.

**Answer:**
`release` on a store: all prior writes in this thread are visible to any thread that subsequently does an `acquire` load of the same variable and sees the stored value.

`acquire` on a load: all subsequent reads in this thread will see the writes that happened-before the corresponding `release` store.

Together they form a synchronization point — a "handshake."

```cpp
#include <atomic>
#include <thread>
#include <cassert>

std::atomic<int>  data{0};
std::atomic<bool> ready{false};

void producer() {
    data.store(42, std::memory_order_relaxed);  // (1) write data
    ready.store(true, std::memory_order_release); // (2) RELEASE: (1) visible to acquirer
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {} // (3) ACQUIRE: sees (1)
    assert(data.load(std::memory_order_relaxed) == 42); // always passes
}

int main() {
    std::thread t1(producer), t2(consumer);
    t1.join(); t2.join();
}
```

The release on `ready` acts as a fence: everything written before it (including `data = 42`) is flushed. The acquire on `ready` acts as a fence: everything read after it (including `data`) is not speculated before the load.

**Without acquire/release (using relaxed everywhere):** the compiler or CPU may reorder (1) and (2), so the consumer might see `ready == true` but `data == 0`.

**Follow-up trap:** "When would you use `seq_cst` instead?"
`seq_cst` is `acquire+release` plus a total order guarantee across all `seq_cst` operations. Needed when you have multiple atomics and need a global ordering. It's slower on x86 (requires `mfence` or `xchg`) and much slower on ARM/POWER. Use `acquire/release` by default; reach for `seq_cst` only when you have multiple atomic flags and need global consistency.

---

### Q24. What is the ABA problem in lock-free programming?

**Answer:**
ABA: a thread reads a value A, another thread changes it to B then back to A, and the first thread's CAS succeeds as if nothing changed — even though the intermediate state may have invalidated assumptions.

Classic example: lock-free stack with pointer reuse.

```cpp
// Lock-free stack — simplified to show the ABA problem
struct Node { int val; Node* next; };
std::atomic<Node*> top{nullptr};

// Thread 1: reads top → A, then is preempted
// Thread 2: pops A, pops B, pushes A back (reusing the node)
// Thread 1: CAS(top, A, A->next) succeeds — but A->next is now stale/garbage
```

**Fix 1: Tagged pointer (double-wide CAS)**
Pack a version counter into the high bits of the pointer. ABA is detected because the version changes even when the address doesn't.

```cpp
struct TaggedPtr {
    Node*    ptr;
    uint64_t tag;  // incremented on every pop
};
std::atomic<TaggedPtr> top;

// CAS checks both ptr AND tag — ABA can't sneak through
```

**Fix 2: Hazard pointers** — threads publish which pointers they're currently reading, and memory is not reused until no thread is reading it.

**Fix 3: Epoch-based reclamation** — similar to RCU.

**Follow-up trap:** "On x86-64, does the hardware support 128-bit CAS?"
Yes — `CMPXCHG16B` (with `lock` prefix). GCC exposes it via `__int128` with `std::atomic`. You must align the struct to 16 bytes.

---

### Q25. What is `memory_order_relaxed` good for?

**Answer:**
`relaxed` provides atomicity (no torn read/write) but no ordering guarantees. No fences are emitted. Use it when you need an atomic counter but don't care what order it's observed relative to other operations.

```cpp
// Hit counter — we care about atomicity, not ordering
std::atomic<uint64_t> request_count{0};

void handle_request() {
    request_count.fetch_add(1, std::memory_order_relaxed);
    // No need for ordering — just count accurately
    do_actual_work();
}

uint64_t get_count() {
    return request_count.load(std::memory_order_relaxed);
}
```

Also appropriate for the "load" side of a published flag when you've already acquired:
```cpp
// After acquire-loading 'ready', subsequent relaxed loads are safe
// because the acquire already established the happens-before
data.load(std::memory_order_relaxed);  // safe — we already did acquire on 'ready'
```

**Follow-up trap:** "Is `relaxed` ever safe for a flag used to stop a loop?"
Usually yes — compilers are required to re-read `atomic` variables even with `relaxed`. But if you also need the loop to see stores made before the flag was set by another thread, you need `acquire` on the load.

---

### Q26. Implement a thread-safe singleton in C++11 without `std::call_once`.

**Answer:**
The "Meyers singleton" using local static initialization — guaranteed to be thread-safe since C++11 (the standard requires exactly-once initialization of function-local statics).

```cpp
class Config {
public:
    static Config& instance() {
        static Config inst;  // initialized exactly once, thread-safe since C++11
        return inst;
    }

    int value() const { return value_; }

    // Non-copyable
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

private:
    Config() : value_(42) {}  // expensive init here
    int value_;
};
```

**Why this works:** C++11 §6.7: "If control enters the declaration concurrently while the variable is being initialized, the concurrent execution shall wait for completion of the initialization." The compiler emits a guard variable and a `futex` (on Linux) internally.

**If `std::call_once` is allowed:**
```cpp
#include <mutex>
class Config {
    static std::once_flag flag_;
    static Config*        inst_;
public:
    static Config& instance() {
        std::call_once(flag_, []{ inst_ = new Config(); });
        return *inst_;
    }
};
```

**Follow-up trap:** "What's wrong with double-checked locking in C++03?"
Without the C++11 memory model, `instance != nullptr` check could be reordered past the object's construction — another thread could see a non-null pointer to an uninitialized object. In C++11 with `std::atomic` or properly ordered reads, DCLP is safe. Meyers singleton is simpler.

---

## Section 6: C++ — Concurrency Primitives

---

### Q27. Explain `std::condition_variable` and why the predicate lambda is mandatory.

**Answer:**
A condition variable allows a thread to atomically release a mutex and wait for a signal. Spurious wakeups (wakeups without a corresponding `notify`) are permitted by POSIX and the C++ standard. Without a predicate, a spurious wakeup causes the thread to proceed incorrectly.

```cpp
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex              mu;
std::condition_variable cv;
std::queue<int>         q;

// Consumer
void consumer() {
    std::unique_lock<std::mutex> lk(mu);

    // WRONG: no predicate — spurious wakeup proceeds on empty queue
    // cv.wait(lk);

    // CORRECT: predicate re-checked on every wakeup
    cv.wait(lk, []{ return !q.empty(); });
    // Equivalent to:
    // while (q.empty()) cv.wait(lk);

    int item = q.front(); q.pop();
}

// Producer
void producer(int item) {
    {
        std::lock_guard<std::mutex> lk(mu);
        q.push(item);
    }
    cv.notify_one();  // notify AFTER releasing lock
}
```

**Follow-up trap:** "Why do you release the lock before calling `notify_one`?"
If you notify while holding the lock, the waiting thread wakes up, immediately tries to reacquire the lock, and blocks again — a needless context switch. Notifying after releasing lets the waiting thread proceed immediately. (Both are correct; this is a performance point.)

---

### Q28. What is `std::jthread` (C++20) and how does it differ from `std::thread`?

**Answer:**
`std::jthread` is a joinable, stoppable thread. Differences:

1. **Auto-join on destruction:** `std::thread` terminates the program if destroyed while joinable (`std::terminate`). `std::jthread` automatically joins on destruction — no more forgetting `join()`.
2. **Cooperative cancellation:** `std::jthread` accepts a `std::stop_token` that the thread can poll to detect a stop request. The owning `jthread`'s destructor also calls `request_stop()` before joining.

```cpp
#include <thread>
#include <stop_token>
#include <chrono>
#include <iostream>

std::jthread worker([](std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        std::cout << "working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "stopped cleanly\n";
});

// When 'worker' goes out of scope:
// 1. request_stop() is called — stoken.stop_requested() becomes true
// 2. join() waits for the thread to exit
// Clean cooperative shutdown, no raw signal needed
```

**Follow-up trap:** "How do you wait on a condition variable with `stop_token`?"
Use `std::condition_variable_any::wait` with a `stop_token` overload:
```cpp
cv.wait(lk, stoken, [&]{ return !q.empty(); });
// Returns false if stop was requested rather than condition becoming true
```

---

### Q29. What is `std::scoped_lock` and when do you use it over `std::lock_guard`?

**Answer:**
`std::scoped_lock` (C++17) locks multiple mutexes simultaneously using a deadlock-avoidance algorithm. `std::lock_guard` locks exactly one.

Use `scoped_lock` when you need to lock two or more mutexes at once — doing it manually (lock A then lock B) risks deadlock if another thread locks B then A.

```cpp
std::mutex mu_a, mu_b;

// WRONG — potential deadlock
void transfer_wrong(Account& a, Account& b, int amount) {
    std::lock_guard lk_a(mu_a);  // Thread 1 locks mu_a
    std::lock_guard lk_b(mu_b);  // Thread 2 has mu_b — deadlock
    a.balance -= amount;
    b.balance += amount;
}

// CORRECT — deadlock-free
void transfer(Account& a, Account& b, int amount) {
    std::scoped_lock lk(mu_a, mu_b);  // acquires both atomically (or backs off)
    a.balance -= amount;
    b.balance += amount;
}
```

Internally, `scoped_lock` uses `std::lock()` which implements a backoff algorithm to avoid deadlock.

---

### Q30. What are the rules for `std::async` and when does the future block?

**Answer:**
`std::async` launches a callable, optionally on a new thread. The returned `std::future` blocks in its destructor — a subtle source of serial execution if you don't store the future.

```cpp
#include <future>
#include <iostream>

// GOTCHA: temporary future destructs immediately — blocks here, not later
std::async(std::launch::async, []{ expensive_work(); });  // blocks right here!

// CORRECT: store the future
auto f = std::async(std::launch::async, []{ return expensive_work(); });
// ... do other things ...
auto result = f.get();  // blocks here — intended

// Launch policy:
// std::launch::async  — always new thread
// std::launch::deferred — run in f.get()'s thread (lazy, same thread)
// std::launch::async | std::launch::deferred — implementation chooses (default)
```

**Follow-up trap:** "What's the difference between `std::async` and `std::thread` + `std::promise`?"
`async` automatically propagates exceptions through the future. `std::thread` terminates on an uncaught exception. With `promise`, you must explicitly `set_exception`. `async` is the high-level convenience; `promise/future` is the low-level primitive.

---

## Section 7: C++ — Modern STL & Language Features

---

### Q31. What is `std::span` and why was it added?

**Answer:**
`std::span<T>` (C++20) is a non-owning view over a contiguous sequence of `T`. It solves the "I want to accept any contiguous range without copying" problem — previously solved badly with raw pointer + size pairs or by overloading for `vector`, `array`, and raw arrays separately.

```cpp
#include <span>
#include <vector>
#include <array>

// One function accepts all of these:
void process(std::span<const uint8_t> data) {
    for (auto byte : data) { /* ... */ }
}

uint8_t raw[64];
std::vector<uint8_t> vec(64);
std::array<uint8_t, 64> arr;

process(raw);           // OK
process(vec);           // OK
process(arr);           // OK
process({raw, 32});     // OK — only first 32 bytes

// Zero-overhead: span is just (ptr, size)
static_assert(sizeof(std::span<int>) == 2 * sizeof(void*));
```

Particularly useful for embedded: receive a DMA buffer as `uint8_t*` + length, wrap it in `span`, pass to parsers without allocation.

---

### Q32. When do you use `std::optional` vs a pointer vs a sentinel value?

**Answer:**

| Mechanism | Allocation | Nullable | Intent clear | Preferred when |
|---|---|---|---|---|
| Raw pointer | No (if stack) | Yes | No | Referencing existing object |
| `std::optional<T>` | No | Yes | Yes | Representing "value or nothing" |
| Sentinel (-1, nullptr, "") | No | Implicit | No | Never (anti-pattern) |

```cpp
// Sentinel — bad: -1 is magic, easy to miss
int find_index(const std::vector<int>& v, int target) {
    for (int i = 0; i < v.size(); ++i)
        if (v[i] == target) return i;
    return -1;  // What does this mean? Easy to forget to check.
}

// optional — good: compiler forces you to handle the empty case
std::optional<int> find_index(const std::vector<int>& v, int target) {
    for (int i = 0; i < v.size(); ++i)
        if (v[i] == target) return i;
    return std::nullopt;  // explicit "not found"
}

auto idx = find_index(v, 42);
if (idx) process(*idx);          // forced to check
int i = idx.value_or(-1);       // or provide default inline
```

**Follow-up trap:** "Is `std::optional<T>` heap-allocated?"
No. The value is stored inline within the `optional` object. `sizeof(optional<T>)` is typically `sizeof(T) + alignment_padding + 1 bool`. No allocation.

---

### Q33. What is `std::string_view` and what are its lifetime pitfalls?

**Answer:**
`std::string_view` is a non-owning, read-only view over a character sequence. No allocation, no copy. Ideal for parsing and string processing APIs that shouldn't care about ownership.

**Pitfall: dangling view.**

```cpp
#include <string_view>

// DANGEROUS: view into a temporary string
std::string_view get_view() {
    std::string s = "hello";
    return s;   // s destroyed at end of function — view dangles
}

// SAFE: view into a string literal (static lifetime)
std::string_view sv = "hello";  // fine — string literal is static

// SAFE: view into a string that outlives the view
std::string s = "hello";
std::string_view sv = s;  // fine, as long as s lives longer than sv
s += " world";            // INVALIDATES sv if reallocation occurs!
```

**Key rules:**
- Never return a `string_view` of a local `std::string`.
- Any operation that may reallocate the underlying `std::string` invalidates all views.
- Prefer `string_view` for function parameters; avoid as return types unless the lifetime is clearly static.

---

### Q34. What is `std::variant` and how does it compare to `union`?

**Answer:**
`std::variant<A, B, C>` is a type-safe discriminated union. Unlike `union`, it tracks which type is active and throws `std::bad_variant_access` on incorrect access. It's the C++ replacement for C-style tagged unions.

```cpp
#include <variant>
#include <string>

// C union — no safety
union Payload { int i; float f; char str[16]; };
// Accessing wrong member: undefined behavior, no check

// std::variant — type-safe
using Result = std::variant<int, std::string, std::monostate>;
//                                              ^^^^^^^^^^^^ represents "no value"

Result parse(const std::string& input) {
    if (input == "fail") return std::monostate{};
    if (input[0] == '-') return std::stoi(input);
    return input;
}

Result r = parse("42");
std::visit([](auto&& val) {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, int>)
        printf("int: %d\n", val);
    else if constexpr (std::is_same_v<T, std::string>)
        printf("string: %s\n", val.c_str());
    else
        printf("no value\n");
}, r);
```

**Follow-up trap:** "Is `std::variant` zero-overhead compared to `union`?"
Almost. `variant` stores a discriminant (usually a `size_t` or `uint8_t`) and the union storage. No heap allocation. Size = `max(sizeof(Types)...) + discriminant + padding`. The `visit` dispatch is a jump table — O(1).

---

## Section 8: C++ — Systems-Level Patterns

---

### Q35. What is RAII and how do you apply it to POSIX IPC resources?

**Answer:**
RAII (Resource Acquisition Is Initialization): bind a resource's lifetime to an object's lifetime. The constructor acquires; the destructor releases. Guarantees cleanup even on exceptions or early returns.

```cpp
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <string>

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size, bool create)
        : name_(name), size_(size)
    {
        int flags = create ? (O_CREAT | O_EXCL | O_RDWR) : O_RDWR;
        fd_ = shm_open(name.c_str(), flags, 0600);
        if (fd_ == -1) throw std::system_error(errno, std::system_category());

        if (create) ftruncate(fd_, size);

        ptr_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (ptr_ == MAP_FAILED) {
            close(fd_);
            throw std::system_error(errno, std::system_category());
        }
        close(fd_);  // fd not needed after mmap
        fd_ = -1;
    }

    ~SharedMemory() {
        if (ptr_ != MAP_FAILED) munmap(ptr_, size_);
        if (owner_) shm_unlink(name_.c_str());
    }

    // Non-copyable, movable
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory& operator=(const SharedMemory&) = delete;

    void* data() const { return ptr_; }
    size_t size() const { return size_; }

private:
    std::string name_;
    size_t      size_;
    int         fd_   = -1;
    void*       ptr_  = MAP_FAILED;
    bool        owner_= true;  // creator is responsible for unlink
};
```

---

### Q36. What is the rule of five in C++11 and when does it apply?

**Answer:**
If a class defines any of: destructor, copy constructor, copy assignment, move constructor, move assignment — it should define all five (or explicitly delete/default them). This is because defining one implies non-trivial resource management, which likely means the compiler-generated versions of the others are wrong.

```cpp
class FileDescriptor {
public:
    explicit FileDescriptor(int fd) : fd_(fd) {}

    // 1. Destructor
    ~FileDescriptor() { if (fd_ != -1) close(fd_); }

    // 2. Copy constructor — DELETED (can't copy an fd)
    FileDescriptor(const FileDescriptor&) = delete;

    // 3. Copy assignment — DELETED
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    // 4. Move constructor — transfer ownership
    FileDescriptor(FileDescriptor&& other) noexcept : fd_(other.fd_) {
        other.fd_ = -1;  // neutralize the source
    }

    // 5. Move assignment
    FileDescriptor& operator=(FileDescriptor&& other) noexcept {
        if (this != &other) {
            if (fd_ != -1) close(fd_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    int get() const { return fd_; }

private:
    int fd_ = -1;
};
```

---

### Q37. How do you avoid undefined behavior when reading/writing shared memory structs across processes compiled separately?

**Answer:**
Multiple pitfalls when sharing structs across process boundaries (or across different compilers/versions):

1. **Padding:** Compilers may insert padding differently. Use `static_assert` on size and `__attribute__((packed))` or explicit padding.
2. **Endianness:** On heterogeneous systems, byte order matters. Use fixed-endian serialization.
3. **Pointer sizes:** Never store pointers in shared memory — use offsets from the base address.
4. **Version skew:** Processes compiled at different times may have different struct layouts.

```cpp
#pragma pack(push, 1)  // or use __attribute__((packed))
struct SensorReading {
    uint32_t device_id;     // 4 bytes, explicit size
    uint64_t timestamp_ms;  // 8 bytes
    float    value;         // 4 bytes — careful: float layout is IEEE 754 on all modern targets
    uint8_t  unit;          // 1 byte
    uint8_t  _pad[3];       // explicit padding to reach 8-byte alignment
    uint32_t version;       // schema version for forward compatibility
};
#pragma pack(pop)

static_assert(sizeof(SensorReading) == 24, "Layout mismatch!");
static_assert(offsetof(SensorReading, timestamp_ms) == 4, "Offset mismatch!");
```

**Follow-up trap:** "Why should you avoid `float` in cross-process/cross-machine shared memory?"
On modern architectures (x86-64, ARM), `float` is IEEE 754 single precision and is layout-compatible. The risk is more theoretical than practical for embedded Linux. But for safety-critical systems or heterogeneous platforms, use `int32_t` with a fixed scale factor instead.

---

## Section 9: Python — Automation & Scripting

---

### Q38. What's wrong with `subprocess.call(cmd, shell=True)` and what do you use instead?

**Answer:**
`shell=True` passes the command to the shell (`/bin/sh -c cmd`). If `cmd` contains any user-supplied data, this is a shell injection vulnerability. Also, `call` discards output.

```python
import subprocess

# BAD — shell injection risk, discards output
filename = user_input  # could be "file.log; rm -rf /"
subprocess.call(f"cat {filename}", shell=True)  # dangerous!

# GOOD — no shell, arguments are a list (never interpreted by shell)
result = subprocess.run(
    ["cat", filename],          # list: no shell expansion
    capture_output=True,        # capture stdout/stderr
    text=True,                  # decode to str (not bytes)
    timeout=30,                 # don't hang forever
    check=True,                 # raise CalledProcessError on non-zero exit
)
print(result.stdout)

# When you need streaming output (daemon monitor):
proc = subprocess.Popen(
    ["my_daemon", "--verbose"],
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    text=True,
)
for line in proc.stdout:
    print(line, end="")  # stream output in real time
proc.wait()
```

**Follow-up trap:** "What happens if the subprocess writes more to stdout than the pipe buffer can hold, and you're also reading stderr separately?"
Deadlock. Both pipes fill; the subprocess blocks on `write()`; your process blocks on `proc.stdout.read()`. Solution: use `communicate()` (reads both concurrently in threads) or use `stderr=subprocess.STDOUT` to merge.

---

### Q39. How do you robustly read a large log file in Python without loading it into memory?

**Answer:**
Use a generator with the file as a context manager. Python's `for line in file` is lazy — it reads one line at a time. For binary files, use `iter(partial(f.read, chunk_size), b"")`.

```python
from pathlib import Path
from typing import Iterator

def error_lines(path: Path) -> Iterator[str]:
    """Yield only ERROR lines from a log file, one at a time."""
    with path.open(encoding="utf-8", errors="replace") as f:
        for line in f:                   # lazy — one line per read
            if "ERROR" in line:
                yield line.rstrip()

# Works on a 10 GB log file with constant memory
for line in error_lines(Path("/var/log/app.log")):
    print(line)

# For binary chunked reading:
from functools import partial

def read_chunks(path: Path, chunk: int = 65536) -> Iterator[bytes]:
    with path.open("rb") as f:
        yield from iter(partial(f.read, chunk), b"")
```

---

### Q40. Explain the difference between `threading`, `multiprocessing`, and `asyncio`. When do you reach for each?

**Answer:**

| Module | Parallelism? | Overhead | Best for |
|---|---|---|---|
| `threading` | No (GIL) | Low | I/O-bound, blocking calls |
| `multiprocessing` | Yes | High (fork/spawn) | CPU-bound, parallel computation |
| `asyncio` | No | Very low | High-concurrency I/O, event loops |
| `concurrent.futures` | Yes (process) / No (thread) | Medium | Simple parallel map |

```python
# threading — good for: subprocess watching, network I/O, blocking calls
import threading
def watch_fifo(path):
    with open(path) as f:
        for line in f:
            process(line)
t = threading.Thread(target=watch_fifo, args=("/tmp/myfifo",), daemon=True)
t.start()

# multiprocessing — good for: CPU-intensive parsing, data crunching
from multiprocessing import Pool
with Pool(processes=4) as pool:
    results = pool.map(parse_log_file, log_files)  # true parallelism

# asyncio — good for: many concurrent connections, event-driven
import asyncio
async def read_sensor(reader: asyncio.StreamReader):
    while True:
        line = await reader.readline()
        await handle(line)

# concurrent.futures — cleanest high-level API
from concurrent.futures import ProcessPoolExecutor
with ProcessPoolExecutor() as ex:
    results = list(ex.map(parse_file, files))
```

---

### Q41. How do you use `pathlib` idiomatically instead of `os.path`?

**Answer:**
`pathlib.Path` is the modern, object-oriented, OS-agnostic path API. All string operations on paths become method calls; concatenation uses `/`.

```python
from pathlib import Path
import os

# OLD WAY (os.path) — verbose, procedural
log_dir = os.path.join("/var", "log", "myapp")
log_files = [f for f in os.listdir(log_dir) if f.endswith(".log")]
full_paths = [os.path.join(log_dir, f) for f in log_files]

# NEW WAY (pathlib)
log_dir = Path("/var/log/myapp")
log_files = list(log_dir.glob("*.log"))         # returns Path objects
latest = max(log_files, key=lambda p: p.stat().st_mtime)

# Common operations
p = Path("/var/log/myapp/server.log")
p.parent          # Path("/var/log/myapp")
p.name            # "server.log"
p.stem            # "server"
p.suffix          # ".log"
p.exists()        # bool
p.read_text()     # entire file as str (for small files)
p.write_text("content")
(p.parent / "backup" / p.name).mkdir(parents=True, exist_ok=True)

# Iterating
for child in log_dir.iterdir():
    if child.is_file():
        print(child)

# Recursive glob
for f in Path(".").rglob("*.cpp"):
    print(f)
```

---

## Section 10: Python — Testing

---

### Q42. Explain pytest fixtures — what is `yield` in a fixture and what is `tmp_path`?

**Answer:**
A pytest fixture is a function that provides setup (and optionally teardown) to tests. The value it `return`s (or `yield`s) is injected into any test that names it as a parameter.

Using `yield` splits the fixture into setup (before `yield`) and teardown (after `yield`). `tmp_path` is a built-in fixture that provides a fresh temporary directory per test.

```python
# conftest.py
import pytest
from pathlib import Path

@pytest.fixture
def log_directory(tmp_path: Path) -> Path:
    """Create a fake log directory with sample files."""
    log_dir = tmp_path / "logs"
    log_dir.mkdir()

    (log_dir / "app.log").write_text(
        "INFO start\nERROR disk full\nINFO stop\n"
    )
    (log_dir / "db.log").write_text(
        "ERROR connection refused\nINFO retry\n"
    )
    yield log_dir  # test runs here
    # teardown: tmp_path cleaned up automatically by pytest

# test_logscan.py
def test_finds_errors(log_directory: Path):
    errors = list(scan_errors(log_directory))
    assert len(errors) == 2
    assert any("disk full" in e for e in errors)

@pytest.mark.parametrize("log_level,expected_count", [
    ("ERROR", 2),
    ("INFO", 3),
    ("WARN", 0),
])
def test_level_filter(log_directory, log_level, expected_count):
    results = list(scan_by_level(log_directory, log_level))
    assert len(results) == expected_count
```

**Follow-up trap:** "What's the difference between `function`, `module`, and `session` scope for fixtures?"
Scope controls how often the fixture is created and torn down. `function` (default): new instance per test. `module`: shared across all tests in a file. `session`: shared across the entire test run. Use broader scope for expensive setup (DB connections, compiled binaries).

---

### Q43. How do you mock a `subprocess` call in pytest?

**Answer:**
Use `unittest.mock.patch` to replace `subprocess.run` with a `MagicMock` that returns a controlled `CompletedProcess`.

```python
from unittest.mock import patch, MagicMock
import subprocess
from my_module import run_sensor_binary  # calls subprocess.run internally

def test_sensor_binary_success():
    mock_result = MagicMock(spec=subprocess.CompletedProcess)
    mock_result.returncode = 0
    mock_result.stdout = '{"temp": 25.3, "unit": "C"}'
    mock_result.stderr = ""

    with patch("my_module.subprocess.run", return_value=mock_result) as mock_run:
        result = run_sensor_binary("/dev/ttyUSB0")

    mock_run.assert_called_once_with(
        ["sensor_reader", "--port", "/dev/ttyUSB0"],
        capture_output=True,
        text=True,
        timeout=5,
        check=True,
    )
    assert result["temp"] == 25.3

def test_sensor_binary_timeout():
    with patch("my_module.subprocess.run",
               side_effect=subprocess.TimeoutExpired("sensor_reader", 5)):
        with pytest.raises(RuntimeError, match="sensor timeout"):
            run_sensor_binary("/dev/ttyUSB0")
```

**Follow-up trap:** "Why do you patch `my_module.subprocess.run` instead of `subprocess.run`?"
Because you're replacing the name in the module that uses it, not in the `subprocess` module itself. Patching the wrong namespace has no effect on the code under test.

---

### Q44. How do you test code that uses `time.sleep` or real wall-clock timeouts?

**Answer:**
Mock `time.sleep` to make it a no-op (or to advance a virtual clock). For code that checks `time.time()`, use `freezegun` or `unittest.mock` to control the returned value.

```python
from unittest.mock import patch
import time

def test_retry_with_backoff():
    call_count = 0

    def failing_call():
        nonlocal call_count
        call_count += 1
        if call_count < 3:
            raise ConnectionError("not ready")

    # time.sleep is mocked — test runs instantly
    with patch("my_module.time.sleep") as mock_sleep:
        retry_with_backoff(failing_call, max_retries=3)

    assert call_count == 3
    # Verify backoff was called with increasing delays
    delays = [call.args[0] for call in mock_sleep.call_args_list]
    assert delays == [1.0, 2.0]  # exponential backoff

# Using freezegun for time-dependent logic:
from freezegun import freeze_time

@freeze_time("2024-01-01 12:00:00")
def test_timestamp_in_output():
    entry = create_log_entry("error occurred")
    assert entry["timestamp"] == "2024-01-01T12:00:00"
```

---

## Section 11: Python — Concurrency & Performance

---

### Q45. What is the GIL and what does it actually prevent?

**Answer:**
The Global Interpreter Lock is a mutex in CPython that ensures only one Python thread executes Python bytecode at a time. It prevents true parallel execution of Python code across cores.

**What the GIL prevents:** parallel CPU computation in Python threads.
**What the GIL does NOT prevent:**
- I/O operations (the GIL is released during blocking I/O — file reads, socket reads, `subprocess`)
- C extension code that explicitly releases the GIL (NumPy, most system calls)
- True parallelism via `multiprocessing` (separate processes have separate GILs)

```python
import threading
import time

# CPU-bound: GIL prevents parallelism — actually slower with threads
def count_up(n):
    x = 0
    for _ in range(n):
        x += 1

t1 = threading.Thread(target=count_up, args=(50_000_000,))
t2 = threading.Thread(target=count_up, args=(50_000_000,))
# These take ~same time as sequential — GIL serializes them

# I/O-bound: GIL released during read() — threads DO help
def read_file(path):
    with open(path) as f:
        return f.read()
# Two threads reading two files IS faster than sequential
```

**Python 3.13 note:** CPython 3.13 introduced an experimental "free-threaded" mode (`--disable-gil` build flag) that removes the GIL. Production use is not yet widespread.

---

### Q46. How do you use `ctypes` to call a C function from Python?

**Answer:**
`ctypes` loads shared libraries and calls C functions directly. Useful for calling your C++ sensor library from a Python test harness without writing Python bindings.

```python
import ctypes
import ctypes.util

# Load your shared library
lib = ctypes.CDLL("./libsensor.so")

# Define the function signature (critical — without this, ctypes assumes int args/return)
lib.sensor_read.restype  = ctypes.c_float
lib.sensor_read.argtypes = [ctypes.c_int, ctypes.c_char_p]

lib.sensor_init.restype  = ctypes.c_int
lib.sensor_init.argtypes = []

# Call it
ret = lib.sensor_init()
assert ret == 0, f"sensor_init failed: {ret}"

value = lib.sensor_read(0, b"/dev/ttyUSB0")  // note: bytes not str
print(f"Temperature: {value:.2f} °C")

# Structs
class SensorData(ctypes.Structure):
    _fields_ = [
        ("device_id", ctypes.c_uint32),
        ("timestamp",  ctypes.c_uint64),
        ("value",      ctypes.c_float),
    ]

lib.sensor_read_struct.restype  = ctypes.c_int
lib.sensor_read_struct.argtypes = [ctypes.POINTER(SensorData)]

data = SensorData()
lib.sensor_read_struct(ctypes.byref(data))
print(data.value)
```

---

## Section 12: JSON — Schema & Design

---

### Q47. What are the types in the JSON specification? What's notably absent?

**Answer:**
JSON (RFC 8259) defines six types: `object`, `array`, `string`, `number`, `boolean` (`true`/`false`), and `null`.

**Notably absent:**
- Comments (no `//` or `/* */`)
- Trailing commas (syntax error)
- `undefined` (JavaScript concept)
- `NaN` and `Infinity` (not valid JSON numbers)
- Integer vs float distinction (all are "number")
- Binary data (use Base64 in a string)
- Date/time types (use ISO 8601 string by convention)
- Integer size limits (spec doesn't define; parsers vary — 64-bit integers > 2^53 lose precision in JavaScript)

```json
// INVALID JSON — common mistakes:
{
  "name": "sensor",  // trailing comma on last item — INVALID
  "value": NaN,      // NaN not valid — INVALID
  "ts": undefined,   // undefined not a JSON type — INVALID
  // "comment": "this is a comment"  — comments not allowed — INVALID
}

// VALID:
{
  "name": "sensor",
  "value": null,
  "ts": "2024-01-01T12:00:00Z",
  "large_int_as_string": "9007199254740993"
}
```

---

### Q48. Design a JSON Schema for a versioned sensor telemetry message.

**Answer:**

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "$id": "https://example.com/schemas/telemetry/v1",
  "title": "SensorTelemetry",
  "description": "Versioned sensor reading with metadata",
  "type": "object",
  "required": ["schema_version", "device_id", "timestamp_ms", "readings"],
  "additionalProperties": false,
  "properties": {
    "schema_version": {
      "type": "integer",
      "const": 1,
      "description": "Schema version — increment on breaking changes"
    },
    "device_id": {
      "type": "string",
      "pattern": "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$",
      "description": "UUID v4"
    },
    "timestamp_ms": {
      "type": "integer",
      "minimum": 0,
      "description": "Unix epoch in milliseconds"
    },
    "readings": {
      "type": "array",
      "minItems": 1,
      "items": { "$ref": "#/$defs/Reading" }
    },
    "tags": {
      "type": "object",
      "additionalProperties": { "type": "string" },
      "description": "Optional key-value metadata"
    }
  },
  "$defs": {
    "Reading": {
      "type": "object",
      "required": ["name", "value", "unit"],
      "additionalProperties": false,
      "properties": {
        "name":  { "type": "string", "minLength": 1 },
        "value": { "type": "number" },
        "unit":  { "type": "string", "enum": ["°C", "°F", "Pa", "m/s", "g", "V", "A"] },
        "quality": {
          "type": "string",
          "enum": ["good", "uncertain", "bad"],
          "default": "good"
        }
      }
    }
  }
}
```

**Version evolution strategy:**
- `schema_version: 1` — use `const: 1` so parsers can reject unknown versions
- Adding optional fields: bump to `1.1` (backward compatible — old parsers ignore them if `additionalProperties` is not `false` on receiving end)
- Removing or renaming fields: bump to `schema_version: 2` — breaking change
- Consumers should check `schema_version` first and route to the right parser

---

### Q49. What is the difference between `oneOf`, `anyOf`, and `allOf` in JSON Schema?

**Answer:**

- `oneOf`: exactly one of the subschemas must validate. Mutually exclusive.
- `anyOf`: one or more of the subschemas must validate.
- `allOf`: all subschemas must validate simultaneously (used for combining/extending schemas).

```json
{
  "oneOf": [
    { "properties": { "type": { "const": "temperature" }, "celsius": { "type": "number" } },
      "required": ["type", "celsius"] },
    { "properties": { "type": { "const": "pressure" }, "pascals": { "type": "number" } },
      "required": ["type", "pascals"] }
  ]
}
```
A message with `"type": "temperature"` validates against the first subschema only. A message with both `celsius` and `pascals` would fail `oneOf` (both match — not exactly one).

```json
{
  "allOf": [
    { "$ref": "#/$defs/BaseMessage" },
    { "$ref": "#/$defs/TimestampedMessage" }
  ]
}
```
A message must satisfy both `BaseMessage` and `TimestampedMessage` — effectively an intersection / extension.

---

### Q50. When would you NOT use JSON for embedded IPC?

**Answer:**
This is a common interview question for embedded roles. Know the trade-offs:

**Avoid JSON when:**
- **High throughput / low latency:** JSON parsing is text-heavy. For 100k messages/sec, a binary format (MessagePack, FlatBuffers) is 5–20x faster to parse.
- **Strict size budgets:** JSON adds 2–4x overhead vs binary. A 20-byte sensor reading might expand to 80 bytes in JSON.
- **No dynamic allocation:** `nlohmann/json` and most parsers allocate. In no-heap embedded environments, use fixed-layout binary structs.
- **Schema evolution with backward compatibility:** Protocol Buffers handles this better — unknown fields are preserved.
- **Cross-language binary protocols:** CBOR, MessagePack, and Protobuf have libraries in C, C++, Python, Rust, Java.

**Alternatives:**

| Format | Strengths | Weaknesses |
|---|---|---|
| Protocol Buffers | Schema evolution, cross-language | Code generation step, no human-readable |
| FlatBuffers | Zero-copy parsing, no alloc | Complex schema, write-once |
| MessagePack | Binary JSON, small, schema-less | Still needs parsing |
| CBOR | IETF standard, self-describing binary | Less tooling than Protobuf |
| Raw C structs | Zero overhead, no parsing | No schema, no versioning, endianness |

---

## Section 13: JSON — Parsing & C++/Python Integration

---

### Q51. Compare `nlohmann/json`, `RapidJSON`, and `simdjson`. When do you choose each?

**Answer:**

| Library | Parse speed | Ergonomics | Heap use | SAX API | Best for |
|---|---|---|---|---|---|
| `nlohmann/json` | ~200 MB/s | Excellent | Yes | No | Developer productivity, config files |
| `RapidJSON` | ~500 MB/s | Verbose | Configurable | Yes | Balanced speed + control |
| `simdjson` | ~2.5 GB/s | Good | Yes | No | Maximum throughput |

```cpp
// nlohmann/json — ergonomic
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::string raw = R"({"device_id":"abc","value":25.3})";
auto j = json::parse(raw);           // throws on error
std::string id = j["device_id"];     // throws if missing
float val = j.value("value", 0.0f); // default if missing

// Error handling
try {
    auto j = json::parse(bad_input);
} catch (const json::parse_error& e) {
    fprintf(stderr, "Parse error at byte %zu: %s\n", e.byte, e.what());
}

// simdjson — fastest, more restrictive API
#include <simdjson.h>
simdjson::ondemand::parser parser;
auto doc = parser.iterate(raw);  // lazy — parses on access
std::string_view id = doc["device_id"];  // zero-copy string view
double val = doc["value"];
// Note: simdjson requires the input buffer to have 64 bytes of padding!
```

**Follow-up trap:** "What does simdjson's SIMD parsing actually do?"
It uses SIMD instructions (SSE4.2, AVX2, NEON) to classify 64 bytes of input at once — finding string boundaries, escape characters, and structural tokens in a single pass. It also uses a two-phase approach: first validate and index the structure, then extract values lazily.

---

### Q52. How do you validate JSON against a schema in Python?

**Answer:**
Use the `jsonschema` library. Prefer `jsonschema.validate` for simple cases; use `Draft202012Validator` with collected errors for user-facing error reporting.

```python
import jsonschema
import json

SCHEMA = {
    "type": "object",
    "required": ["device_id", "timestamp_ms", "readings"],
    "properties": {
        "device_id":    {"type": "string"},
        "timestamp_ms": {"type": "integer", "minimum": 0},
        "readings": {
            "type": "array",
            "minItems": 1,
            "items": {
                "type": "object",
                "required": ["name", "value"],
                "properties": {
                    "name":  {"type": "string"},
                    "value": {"type": "number"},
                }
            }
        }
    }
}

def validate_message(raw: str) -> dict:
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as e:
        raise ValueError(f"Invalid JSON: {e}")

    # Collect all errors (not just the first)
    validator = jsonschema.Draft202012Validator(SCHEMA)
    errors = list(validator.iter_errors(data))
    if errors:
        messages = [f"{e.json_path}: {e.message}" for e in errors]
        raise ValueError("Schema validation failed:\n" + "\n".join(messages))

    return data
```

**For production, prefer `pydantic`** — it validates and deserializes in one step with better performance:

```python
from pydantic import BaseModel, Field
from typing import List

class Reading(BaseModel):
    name: str
    value: float
    unit: str = "unknown"

class Telemetry(BaseModel):
    device_id: str
    timestamp_ms: int = Field(ge=0)
    readings: List[Reading] = Field(min_length=1)

msg = Telemetry.model_validate_json(raw_string)  # raises ValidationError on failure
print(msg.readings[0].value)
```

---

## Section 14: System Design — IPC Architecture

---

### Q53. Design an IPC protocol for a safety-critical embedded system where any process may crash at any time.

**Answer:**
Key principles: fail-safe defaults, detection of dead peers, atomic state transitions, clean crash recovery.

```
Architecture:
┌─────────────┐    heartbeat FIFO    ┌─────────────────┐
│  Watchdog   │◄────────────────────┤  Worker Process  │
│  Process    │────kill/restart────►│                  │
└─────────────┘                     └────────┬─────────┘
                                             │ data shm
                                    ┌────────▼─────────┐
                                    │  Shared Memory   │
                                    │  (robust mutex)  │
                                    └──────────────────┘
```

**Protocol design:**
1. **Heartbeat:** Each process writes a monotonically-increasing counter to its own shm slot every 100ms. Watchdog reads and detects stale values (no update in 500ms = dead).
2. **Robust mutex:** All cross-process mutexes use `PTHREAD_MUTEX_ROBUST`. On `EOWNERDEAD`, the new lock holder resets shared state to a safe default.
3. **Sequence numbers:** Each message has a monotonic sequence number. Consumers track the last seen sequence and detect gaps (missed messages due to crash).
4. **State machine with atomic transitions:** The shared state struct uses an atomic `state` field (IDLE, RUNNING, FAULT). CAS is used to transition — no transition happens if another process changed state first.
5. **Unlink-on-start cleanup:** On startup, each process checks for stale shm/semaphore names from a previous crash and cleans them up before creating new ones.

---

### Q54. How would you pass a file descriptor from one process to another?

**Answer:**
Use UNIX domain sockets with `sendmsg`/`recvmsg` and `SCM_RIGHTS` ancillary data. This is the only portable way to pass open file descriptors between processes.

```c
#include <sys/socket.h>
#include <sys/un.h>

// SENDER
void send_fd(int socket, int fd_to_send) {
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(int))];
    memset(buf, 0, sizeof(buf));

    // Dummy iovec (required by sendmsg)
    struct iovec iov;
    char dummy = 'F';
    iov.iov_base = &dummy;
    iov.iov_len = 1;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // Ancillary data carrying the fd
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type  = SCM_RIGHTS;
    cmsg->cmsg_len   = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));

    sendmsg(socket, &msg, 0);
}

// RECEIVER — gets a NEW fd in its own fd table
int recv_fd(int socket) {
    struct msghdr msg = {0};
    char buf[CMSG_SPACE(sizeof(int))];
    memset(buf, 0, sizeof(buf));

    struct iovec iov;
    char dummy;
    iov.iov_base = &dummy;
    iov.iov_len = 1;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    recvmsg(socket, &msg, 0);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    int received_fd;
    memcpy(&received_fd, CMSG_DATA(cmsg), sizeof(int));
    return received_fd;
}
```

---

## Section 15: Debugging & Observability

---

### Q55. How do you debug a process hanging on `sem_wait`?

**Answer:**
Methodical approach:

```bash
# 1. Find the PID and check its state
ps aux | grep myprocess
cat /proc/<pid>/status | grep State
# "S (sleeping)" means blocked in sem_wait

# 2. strace the hanging process — shows exactly which syscall it's in
strace -p <pid>
# Output: futex(0x7f..., FUTEX_WAIT_BITSET...) — confirms blocked in futex (sem_wait internals)

# 3. Get a stack trace without stopping the process
gdb -p <pid> -batch -ex "thread apply all bt" -ex detach
# Shows C call stack — confirms sem_wait and which semaphore

# 4. Check semaphore state (named semaphore)
ls -la /dev/shm/         # list named semaphores
cat /proc/sysvipc/sem    # System V semaphores

# 5. Check if the other process (the poster) is alive
pgrep -a sensor_process
# If dead → robust mutex would have returned EOWNERDEAD (if configured)
# If not using robust mutex → hang confirmed

# 6. Check for deadlock (two processes waiting for each other)
# Both show futex_wait in strace — classic deadlock

# 7. Valgrind Helgrind for threading/locking bugs (on smaller reproducer)
valgrind --tool=helgrind ./myprogram
```

---

### Q56. How do you use `strace` to understand what IPC calls a program makes?

**Answer:**

```bash
# Trace a running process
strace -p <pid> -e trace=ipc,file,desc

# Trace from start, show timestamps, filter to IPC-relevant syscalls
strace -tt -e trace=open,read,write,mmap,munmap,shm_open,shm_unlink,\
semaphore_open,semaphore_wait,semaphore_post,futex \
./my_program

# Follow child processes (after fork)
strace -f -e trace=all ./my_program 2>&1 | grep -E "(shm|mmap|futex|fifo)"

# Count syscalls (profiling)
strace -c ./my_program

# Typical output for FIFO:
# open("/tmp/myfifo", O_RDONLY)           = 3
# read(3, "hello\n", 256)                 = 6
# close(3)                                = 0
```

**Follow-up trap:** "Does `strace` affect timing?"
Yes — `strace` uses `ptrace` which stops the process on each syscall. It can slow down programs by 10–100x and perturb timing-sensitive behavior. Use with caution on production. For production tracing, use `perf` or eBPF (`bpftrace`).

---

### Q57. What tools do you use to detect memory errors in IPC code?

**Answer:**

```bash
# Valgrind memcheck — detects leaks, use-after-free, uninitialised reads
valgrind --leak-check=full --track-origins=yes ./myprogram

# AddressSanitizer (ASan) — much faster than Valgrind, catches more races
g++ -fsanitize=address,undefined -g -O1 -o myprogram myprogram.cpp
./myprogram

# ThreadSanitizer (TSan) — detects data races
g++ -fsanitize=thread -g -O1 -o myprogram myprogram.cpp
./myprogram

# For shared memory specifically:
# TSan does NOT automatically track cross-process races (it's in-process)
# For multi-process: use helgrind or manual inspection

# Check for leaked shm segments
ls /dev/shm/
ipcs -m  # System V

# Check for open file descriptors (FIFOs, shm fds)
lsof -p <pid> | grep -E "(FIFO|REG.*shm)"
```

---

### Q58. How do you measure the latency of your IPC mechanism?

**Answer:**
Use a ping-pong benchmark: process A writes a message, process B reads it and writes back, process A reads the response. Half the round-trip time is one-way latency.

```cpp
// Minimal latency benchmark using shared memory + sem
#include <semaphore.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>

#define ITERS 100000

struct Channel {
    sem_t a_to_b;
    sem_t b_to_a;
    volatile uint64_t payload;
};

// Process A (pinger)
void pinger(Channel *ch) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    for (int i = 0; i < ITERS; i++) {
        ch->payload = i;
        sem_post(&ch->a_to_b);
        sem_wait(&ch->b_to_a);
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    double elapsed_us = ((t1.tv_sec - t0.tv_sec) * 1e9 +
                         (t1.tv_nsec - t0.tv_nsec)) / 1e3;
    printf("Round-trip: %.2f µs avg (%.0f iterations)\n",
           elapsed_us / ITERS, (double)ITERS);
}

// Process B (ponger)
void ponger(Channel *ch) {
    for (int i = 0; i < ITERS; i++) {
        sem_wait(&ch->a_to_b);
        // echo back
        sem_post(&ch->b_to_a);
    }
}
```

Typical results on Linux x86-64:
- Shared memory + semaphore: ~2–5 µs round-trip
- FIFO: ~10–20 µs
- UNIX socket: ~8–15 µs
- TCP loopback: ~20–50 µs

---

*End of interview reference — 58 questions across POSIX IPC, C++ concurrency, Python, and JSON.*
