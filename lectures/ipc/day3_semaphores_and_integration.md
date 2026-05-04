# Day 3 — POSIX Semaphores + Putting IPC Together

> **Time budget:** 75 min concept · 150 min lab · 45 min drills · 15 min recap
> **Prerequisites:** Days 1 and 2 complete; ring buffer from Day 2 in hand
> **By the end you can:**
> - Implement a process-shared semaphore correctly (unnamed, in shared memory)
> - Build a bounded blocking queue with a process-shared mutex and two counting semaphores
> - Explain robust mutexes and recover from a process that dies holding a lock
> - Design and run the full sensor pipeline: shm + semaphores for data, FIFO for control
> - Articulate the mutex-vs-semaphore distinction without hesitation

---

## 1. Conceptual overview

A semaphore is an integer the kernel maintains with two atomic operations: `sem_wait` (decrement, block if zero) and `sem_post` (increment, wake a waiter). That's it. The integer value represents available resources or available slots.

**Named vs unnamed:**

| | Named (`sem_open`) | Unnamed (`sem_init`) |
|---|---|---|
| Namespace | Filesystem path (`/sem_name`) | None |
| Lifetime | Until `sem_unlink` | Until enclosing shared memory is destroyed |
| Cross-process | Yes (any process with path) | Yes, if placed in shared memory with `pshared=1` |
| Typical use | Unrelated processes needing discovery | Tightly coupled processes sharing a segment |

For a system where processes share a memory segment, unnamed semaphores *in that segment* are the canonical pattern. They live and die with the shared memory, require no separate namespace management, and are marginally faster.

**Semaphore vs mutex vs condition variable:**

A mutex is binary (locked / unlocked) and has ownership semantics — the thread that locks must unlock. A semaphore has no ownership; any process can `sem_post`. This makes semaphores natural for producer-consumer signaling (producer posts, consumer waits) and mutexes natural for mutual exclusion of a critical section.

A condition variable requires a mutex; it's a "wait until a predicate is true" mechanism. `pthread_cond_wait` atomically releases the mutex and sleeps; `pthread_cond_signal` wakes one waiter which reacquires the mutex. Cross-process condition variables require `PTHREAD_PROCESS_SHARED` on both the mutex attr and the condvar attr — possible but more complex than semaphores for the bounded-queue pattern.

**Robust mutexes — the embedded differentiator:**

If a process dies while holding a `pthread_mutex_t`, any other process blocked in `pthread_mutex_lock` on that mutex will wait forever. A robust mutex (`PTHREAD_MUTEX_ROBUST`) changes this: `pthread_mutex_lock` returns `EOWNERDEAD` when the previous owner died. The new owner can then call `pthread_mutex_consistent` to declare the protected data consistent (or abandon it), and unlock normally. This is essential in embedded multi-process systems where process death must not freeze the rest of the system.

---

## 2. Deep dive: unnamed semaphores in shared memory

```c
// sem_in_shm.c — process-shared semaphore setup
// Build: gcc -Wall -Wextra -o sem_in_shm sem_in_shm.c -lrt -lpthread
// Run:   ./sem_in_shm (forks a child to demonstrate cross-process semaphore)

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    sem_t sem;        // unnamed semaphore — pshared=1 required
    int   value;
} SharedData;

int main(void) {
    SharedData *sd = mmap(NULL, sizeof(SharedData),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sd == MAP_FAILED) { perror("mmap"); return 1; }

    // pshared=1: accessible from multiple processes
    // initial value=0: first wait will block
    if (sem_init(&sd->sem, /*pshared=*/1, /*value=*/0) < 0) {
        perror("sem_init"); return 1;
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        // Child: wait for parent to set a value, then read it
        printf("[child]  waiting on semaphore\n");
        sem_wait(&sd->sem);
        printf("[child]  got value: %d\n", sd->value);
        return 0;
    }

    // Parent: write a value, then signal the child
    sleep(1);
    sd->value = 42;
    printf("[parent] posted value %d\n", sd->value);
    sem_post(&sd->sem);

    waitpid(pid, NULL, 0);
    sem_destroy(&sd->sem);
    munmap(sd, sizeof(SharedData));
    return 0;
}
```

Note the `MAP_ANONYMOUS` flag — this creates an anonymous mapping (no file backing) shared between parent and child after `fork`. For *unrelated* processes, use `shm_open` + `mmap` as on Day 2.

---

## 3. Deep dive: process-shared mutex + robust mutexes

```c
// robust_mutex.c — demonstrates PTHREAD_MUTEX_ROBUST across a fork
// Build: gcc -Wall -Wextra -o robust_mutex robust_mutex.c -lpthread
// Run:   ./robust_mutex

#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

typedef struct {
    pthread_mutex_t mtx;
    int             data;
} Shared;

static Shared *setup_shared(void) {
    Shared *s = mmap(NULL, sizeof(Shared),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED) { perror("mmap"); return NULL; }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    // Cross-process sharing
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    // Robust: lock returns EOWNERDEAD if previous owner died
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&s->mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return s;
}

int main(void) {
    Shared *s = setup_shared();
    if (!s) return 1;

    pid_t pid = fork();
    if (pid == 0) {
        // Child: acquire the mutex and then crash without releasing it
        pthread_mutex_lock(&s->mtx);
        printf("[child]  acquired mutex, crashing now\n");
        _exit(1);  // die holding the lock
    }

    waitpid(pid, NULL, 0);

    // Parent: attempt to acquire the mutex the child died holding
    int r = pthread_mutex_lock(&s->mtx);
    if (r == EOWNERDEAD) {
        printf("[parent] previous owner died — calling consistent\n");
        // Mark protected data as consistent (or repair it here)
        pthread_mutex_consistent(&s->mtx);
        s->data = 0;  // reset any potentially corrupt state
    } else if (r != 0) {
        fprintf(stderr, "lock failed: %s\n", strerror(r));
        return 1;
    }
    printf("[parent] holding mutex safely, data=%d\n", s->data);
    pthread_mutex_unlock(&s->mtx);

    pthread_mutex_destroy(&s->mtx);
    munmap(s, sizeof(Shared));
    return 0;
}
```

`pthread_mutex_consistent` does not repair your data — that's your job. It only tells the kernel that the mutex is in a consistent state and may be unlocked. Always inspect and repair the protected data structure before calling it.

---

## 4. Lab

### Setup

```bash
rm -f /dev/shm/sensor_pipeline /tmp/control_fifo

g++ -std=c++20 -Wall -Wextra -O2 -o sensor    sensor.cpp   -lrt -lpthread
g++ -std=c++20 -Wall -Wextra -O2 -o aggregator aggregator.cpp -lrt -lpthread
g++ -std=c++20 -Wall -Wextra -O2 -o logger    logger.cpp   -lrt -lpthread
```

### Tasks

**Task 1 — Bounded blocking queue with mutex + two semaphores**

Replace Day 2's lock-free ring buffer with a mutex-protected version. Two semaphores: `slots_free` (initialized to `N`, decremented by producer before writing, signals "there's space") and `items_avail` (initialized to 0, incremented by producer after writing, signals "there's data").

```cpp
// bounded_queue.h — process-shared bounded queue
// SPSC or MPMC (mutex handles multiple producers/consumers)
#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <cstdint>
#include <cstring>
#include <cerrno>

inline constexpr int QUEUE_CAP = 32;

struct BoundedQueue {
    pthread_mutex_t mtx;
    sem_t           slots_free;   // counts empty slots
    sem_t           items_avail;  // counts filled slots
    int64_t         data[QUEUE_CAP];
    int             head;
    int             tail;

    static void init(BoundedQueue *q) {
        pthread_mutexattr_t mattr;
        pthread_mutexattr_init(&mattr);
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST);
        pthread_mutex_init(&q->mtx, &mattr);
        pthread_mutexattr_destroy(&mattr);

        sem_init(&q->slots_free, /*pshared=*/1, QUEUE_CAP);
        sem_init(&q->items_avail, /*pshared=*/1, 0);
        q->head = q->tail = 0;
    }

    // Blocks when full
    void push(int64_t val) {
        sem_wait(&slots_free);
        {
            int r = pthread_mutex_lock(&mtx);
            if (r == EOWNERDEAD) pthread_mutex_consistent(&mtx);
            data[head] = val;
            head = (head + 1) % QUEUE_CAP;
            pthread_mutex_unlock(&mtx);
        }
        sem_post(&items_avail);
    }

    // Blocks when empty
    int64_t pop() {
        sem_wait(&items_avail);
        int r = pthread_mutex_lock(&mtx);
        if (r == EOWNERDEAD) pthread_mutex_consistent(&mtx);
        int64_t val = data[tail];
        tail = (tail + 1) % QUEUE_CAP;
        pthread_mutex_unlock(&mtx);
        sem_post(&slots_free);
        return val;
    }

    static void destroy(BoundedQueue *q) {
        pthread_mutex_destroy(&q->mtx);
        sem_destroy(&q->slots_free);
        sem_destroy(&q->items_avail);
    }
};
```

**Done when:** Producer blocks when queue is full; consumer blocks when empty; no busy-wait loops needed.

**Task 2 — Kill the producer mid-run, observe hang, fix with robust mutex**

Run producer and consumer. While producer holds the mutex mid-push, kill it with `kill -9`. Consumer hangs in `pthread_mutex_lock`. Rebuild with `PTHREAD_MUTEX_ROBUST` and verify consumer recovers with `EOWNERDEAD`.

**Done when:** Consumer prints a recovery message and continues processing.

**Task 3 — Sensor pipeline (integration)**

Build a three-process pipeline that you will reuse on Day 7.

```
sensor (C++)  ──shm+semaphore──▶  aggregator (C++)  ──FIFO──▶  logger (C++)
                                                         ▲
                                              FIFO ──────┘  (control: start/stop/reset)
```

**`sensor.cpp`** — generates fake readings (sin wave + noise), packs into a struct, pushes to `BoundedQueue` in shared memory:

```cpp
// sensor.cpp
#include "bounded_queue.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define SHM_NAME "/sensor_pipeline"

static volatile sig_atomic_t running = 1;
static void on_signal(int) { running = 0; }

int main() {
    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(BoundedQueue)) < 0) { perror("ftruncate"); return 1; }

    auto *q = static_cast<BoundedQueue *>(
        mmap(nullptr, sizeof(BoundedQueue), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if (q == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    BoundedQueue::init(q);
    printf("[sensor] started, writing to %s\n", SHM_NAME);

    long t = 0;
    while (running) {
        // Simulate sensor reading: scaled integer (avoid floats in shm)
        int64_t reading = (int64_t)(1000.0 * sin(t * 0.1) + rand() % 100);
        q->push(reading);
        ++t;
        usleep(10000);  // 100 Hz
    }

    // Poison pill to stop aggregator
    q->push(INT64_MIN);

    BoundedQueue::destroy(q);
    munmap(q, sizeof(BoundedQueue));
    shm_unlink(SHM_NAME);
    puts("[sensor] shutdown complete");
    return 0;
}
```

**`aggregator.cpp`** — reads from shm, computes rolling mean and max over a window of 10, writes JSON-ish lines to a FIFO:

```cpp
// aggregator.cpp
#include "bounded_queue.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <cstdio>
#include <cstring>
#include <numeric>

#define SHM_NAME   "/sensor_pipeline"
#define FIFO_PATH  "/tmp/control_fifo"

int main() {
    // Open shm (producer creates it first)
    int fd = -1;
    while (fd < 0) { fd = shm_open(SHM_NAME, O_RDWR, 0600); usleep(1000); }
    if (ftruncate(fd, sizeof(BoundedQueue)) < 0) { perror("ftruncate"); return 1; }
    auto *q = static_cast<BoundedQueue *>(
        mmap(nullptr, sizeof(BoundedQueue), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if (q == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    // Open output FIFO
    if (mkfifo(FIFO_PATH, 0600) < 0 && errno != EEXIST) { perror("mkfifo"); return 1; }
    printf("[aggregator] waiting for logger to open FIFO\n");
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd < 0) { perror("open fifo"); return 1; }
    printf("[aggregator] connected\n");

    int64_t window[10] = {};
    int     w = 0;
    long    count = 0;
    char    buf[128];

    for (;;) {
        int64_t val = q->pop();
        if (val == INT64_MIN) break;  // poison pill

        window[w % 10] = val;
        ++w; ++count;

        if (count % 10 == 0) {
            int64_t sum = 0, mx = INT64_MIN;
            for (int i = 0; i < 10; ++i) {
                sum += window[i];
                if (window[i] > mx) mx = window[i];
            }
            int n = snprintf(buf, sizeof(buf),
                             "{\"count\":%ld,\"mean\":%ld,\"max\":%ld}\n",
                             count, sum / 10, mx);
            if (write(fifo_fd, buf, (size_t)n) < 0) { perror("write fifo"); break; }
        }
    }

    close(fifo_fd);
    munmap(q, sizeof(BoundedQueue));
    puts("[aggregator] shutdown");
    return 0;
}
```

**`logger.cpp`** — reads lines from the FIFO and prints them with a timestamp:

```cpp
// logger.cpp
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <ctime>

#define FIFO_PATH "/tmp/control_fifo"

int main() {
    printf("[logger] opening FIFO %s\n", FIFO_PATH);
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }
    printf("[logger] connected\n");

    char buf[256];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        printf("[%ld.%03ld] %s", (long)ts.tv_sec, ts.tv_nsec / 1000000L, buf);
        fflush(stdout);
    }
    close(fd);
    puts("[logger] done");
    return 0;
}
```

**Run the pipeline:**
```bash
# Terminal 1
./sensor

# Terminal 2
./aggregator

# Terminal 3
./logger

# Stop: Ctrl-C on sensor; aggregator and logger exit via poison pill / SIGPIPE
```

**Done when:** All three processes run, logger prints rolling stats, and sending SIGINT to sensor causes clean shutdown of all three.

---

## 5. Common pitfalls

- **`sem_init` with `pshared=0` for cross-process use.** `pshared=0` means thread-local. The semaphore is placed in process memory; another process's `sem_wait` on it is undefined behavior. Always use `pshared=1` for semaphores in shared memory.

- **Using named semaphores when you already have shared memory.** Named semaphores live in a separate namespace, require separate cleanup, and add complexity. If you already have a shared memory segment, put unnamed semaphores in it.

- **Forgetting that semaphore values can overflow.** `sem_post` on a semaphore already at its maximum (usually `SEM_VALUE_MAX`) returns `EOVERFLOW`. In a well-designed system this shouldn't happen, but don't silently ignore `sem_post` return values.

- **Destroying a semaphore with waiters.** `sem_destroy` on a semaphore that other threads/processes are waiting on is undefined behavior. Signal all waiters (post enough times) before destroying.

- **Not calling `pthread_mutex_consistent` after `EOWNERDEAD`.** If you get `EOWNERDEAD` and proceed to use the mutex without calling `pthread_mutex_consistent`, the mutex remains in an inconsistent state and the next `pthread_mutex_lock` also returns `EOWNERDEAD`.

- **Priority inversion with FIFO scheduling.** A low-priority process holds a mutex; a high-priority process blocks waiting for it; a medium-priority process preempts the low-priority one. The high-priority process is blocked behind medium forever. Prevention: `PTHREAD_PRIO_INHERIT` mutex attribute, which temporarily raises the lock holder to the waiting thread's priority.

---

## 6. Interview drills

**Q: What's the difference between a mutex and a binary semaphore?**

Ownership. A mutex has an owner — the thread that locked it must unlock it. The kernel can use this for priority inheritance. A binary semaphore has no owner; any thread or process can post it. This makes semaphores suitable for signaling between producer and consumer (producer posts, consumer waits), while mutexes are right for protecting a critical section. Using a semaphore as a mutex loses priority inheritance and makes debugging harder.

*Follow-up: "Can you implement a mutex with semaphores?"* — Yes, initialized to 1, wait to lock, post to unlock. But you lose ownership tracking, recursive locking, and priority inheritance. Don't do it unless on a platform without mutexes.

---

**Q: What's priority inversion? How do you prevent it?**

A high-priority task blocks on a resource held by a low-priority task. A medium-priority task preempts the low-priority task. The high-priority task is now effectively blocked behind the medium-priority task indefinitely. Prevention: priority inheritance (`PTHREAD_PRIO_INHERIT`) temporarily elevates the lock holder to the highest priority of any waiter. Priority ceiling (`PTHREAD_PRIO_PROTECT`) sets a static ceiling the holder runs at while holding the lock. Embedded RTOS schedulers (FreeRTOS, VxWorks) implement these natively; Linux `pthread` supports both attributes.

---

**Q: A process holding a process-shared mutex crashes. What happens to the others?**

Without robustness: they block forever in `pthread_mutex_lock`. With `PTHREAD_MUTEX_ROBUST`: `pthread_mutex_lock` returns `EOWNERDEAD`. The recovering process must call `pthread_mutex_consistent` (after repairing the protected data) and then `pthread_mutex_unlock`. If it does not call `pthread_mutex_consistent`, subsequent lock attempts also return `EOWNERDEAD` and eventually `ENOTRECOVERABLE`, at which point the mutex is permanently unusable.

---

**Q: Design IPC for a system with 1 sensor, 3 workers, 1 logger. Defend your choices.**

Sensor → workers: shared memory ring buffer with a counting semaphore (`items_avail`) for notification and a process-shared mutex for head/tail access. Workers are consumers that pop from a single queue (task-distribution semantics, not pub/sub). If each worker needs its own copy of every reading, use three separate queues or a broadcast mechanism. Workers → logger: a FIFO per worker, or workers write to a single mutex-protected FIFO (writes `<= PIPE_BUF` are atomic). Logger: reads from FIFO(s), timestamps and writes to disk. Crash safety: robust mutex on the shm queue; each worker handles `EOWNERDEAD`. FIFO for logger because log writes are sequential and low-frequency — no need for shm overhead there.

---

**Q: `sem_wait` vs `sem_timedwait` — when do you use which?**

`sem_wait` blocks indefinitely — acceptable when the system is well-behaved. In production embedded code, always use `sem_timedwait` with a watchdog timeout. If the semaphore isn't posted within the deadline, the process can log the anomaly, attempt recovery, and alert a supervisor. An indefinitely blocked `sem_wait` is indistinguishable from a deadlock to any monitoring system.

---

## 7. Cheatsheet

### Unnamed semaphore lifecycle (in shared memory)

```c
sem_init(&sem, /*pshared=*/1, initial_value)   // in the shared segment
sem_wait(&sem)      // decrement; block if 0
sem_trywait(&sem)   // decrement or EAGAIN (non-blocking)
sem_timedwait(&sem, &abs_timespec)              // decrement or ETIMEDOUT
sem_post(&sem)      // increment; wake one waiter
sem_getvalue(&sem, &val)                        // read current value
sem_destroy(&sem)   // clean up (no waiters allowed)
```

### Process-shared mutex setup

```c
pthread_mutexattr_t a;
pthread_mutexattr_init(&a);
pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
pthread_mutexattr_setrobust(&a, PTHREAD_MUTEX_ROBUST);   // optional but recommended
pthread_mutexattr_setprotocol(&a, PTHREAD_PRIO_INHERIT); // prevents priority inversion
pthread_mutex_init(&mtx, &a);
pthread_mutexattr_destroy(&a);
```

### Robust mutex recovery pattern

```c
int r = pthread_mutex_lock(&mtx);
if (r == EOWNERDEAD) {
    // repair protected data here
    pthread_mutex_consistent(&mtx);
} else if (r != 0) {
    // unrecoverable error
}
// critical section
pthread_mutex_unlock(&mtx);
```

### Bounded queue — two-semaphore pattern

```
Producer:               Consumer:
sem_wait(slots_free)    sem_wait(items_avail)
mutex_lock              mutex_lock
  write data              read data
mutex_unlock            mutex_unlock
sem_post(items_avail)   sem_post(slots_free)
```

### Named semaphore (for reference)

```c
sem_t *s = sem_open("/name", O_CREAT, 0600, initial_value);
sem_wait(s);
sem_post(s);
sem_close(s);      // close this process's handle
sem_unlink("/name"); // destroy
```

### Key flags / attributes

| Attribute | Value | Effect |
|---|---|---|
| `pshared` | `PTHREAD_PROCESS_SHARED` | Cross-process mutex/condvar |
| Robustness | `PTHREAD_MUTEX_ROBUST` | `EOWNERDEAD` on owner crash |
| Protocol | `PTHREAD_PRIO_INHERIT` | Priority inheritance |

---

## 8. Further reading

- Kerrisk, *The Linux Programming Interface*, Chapter 53 (POSIX semaphores) — complete reference
- `man pthread_mutexattr_setrobust` — short; covers the `EOWNERDEAD` / `ENOTRECOVERABLE` states precisely
- `man sem_overview` — good summary of named vs unnamed semantics