# Day 7 — Mock Interview, Integration, and Final Prep

> **Time budget:** 180 min integration project · 120 min drill block · 45 min recap
> **Prerequisites:** Days 1–6 complete, or at minimum Days 1–4 (IPC + C++ concurrency).
> **By the end you can:**
> - Demo a complete multi-process telemetry pipeline from memory on a whiteboard
> - Answer 20 embedded systems interview questions without hesitation
> - Identify your remaining weak spots and address them in 45 minutes

---

## 1. Integration project

Build this once. Then be able to describe it in 90 seconds from memory — it's your walk-in story.

### Spec

A small embedded telemetry pipeline with three components:

```
┌─────────────┐  shm ring buffer   ┌─────────────┐  FIFO (JSON)   ┌─────────────────┐
│  sensor_proc │ ────────────────► │  aggregator  │ ─────────────► │ Python supervisor│
│   (C++)      │  mutex+semaphore  │    (C++)     │                │  validates JSON  │
└─────────────┘                    └─────────────┘                │  writes CSV      │
                                                                    │  runs pytest     │
                                                                    └─────────────────┘
```

**sensor_proc (C++):**
- Generates fake sensor readings (temperature, pressure) every 100 ms
- Serializes each as a JSON message using `nlohmann/json`
- Writes into a shared memory ring buffer (head/tail protected by process-shared mutex + two semaphores)
- Handles `SIGTERM`: releases the mutex if held, calls `shm_unlink`, exits cleanly

**aggregator (C++):**
- Reads from the shm ring buffer
- Computes rolling mean and max over the last 10 readings per sensor name
- Writes results as JSON lines to a FIFO
- Handles `SIGTERM` cleanly

**Python supervisor:**
- Launches both C++ processes as subprocesses
- Opens the FIFO and reads JSON lines with a timeout
- Validates each message against a JSON schema
- Writes results to a CSV report
- Has a `pytest` suite with at least one end-to-end test

### File layout

```
pipeline/
├── sensor_proc.cpp
├── aggregator.cpp
├── Makefile
├── telemetry.schema.json
├── supervisor.py
└── tests/
    └── test_pipeline.py
```

### sensor_proc.cpp

```cpp
// sensor_proc.cpp
// Build: see Makefile
#include <nlohmann/json.hpp>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

static constexpr int   RING_SIZE = 32;
static constexpr char  SHM_NAME[] = "/telemetry_shm";
static constexpr int   MSG_SIZE   = 512;

struct Slot {
    char data[MSG_SIZE];
    int  len;
};

struct RingBuffer {
    pthread_mutex_t mutex;
    sem_t           slots_free;   // counts free slots
    sem_t           items_ready;  // counts filled slots
    int             head;         // producer writes here
    int             tail;         // consumer reads here
    Slot            slots[RING_SIZE];
};

static RingBuffer* g_ring = nullptr;
static int         g_shm_fd = -1;
static volatile sig_atomic_t g_stop = 0;

static void on_sigterm(int) { g_stop = 1; }

static bool setup_shm() {
    g_shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (g_shm_fd < 0) { perror("shm_open"); return false; }

    if (ftruncate(g_shm_fd, sizeof(RingBuffer)) < 0) {
        perror("ftruncate"); return false;
    }

    g_ring = static_cast<RingBuffer*>(
        mmap(nullptr, sizeof(RingBuffer),
             PROT_READ | PROT_WRITE, MAP_SHARED, g_shm_fd, 0));
    if (g_ring == MAP_FAILED) { perror("mmap"); return false; }

    // Initialize synchronization primitives (only call once — producer owns this)
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&mattr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&g_ring->mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    sem_init(&g_ring->slots_free,  /*pshared=*/1, RING_SIZE);
    sem_init(&g_ring->items_ready, /*pshared=*/1, 0);
    g_ring->head = 0;
    g_ring->tail = 0;

    return true;
}

static void cleanup() {
    if (g_ring && g_ring != MAP_FAILED) {
        pthread_mutex_destroy(&g_ring->mutex);
        sem_destroy(&g_ring->slots_free);
        sem_destroy(&g_ring->items_ready);
        munmap(g_ring, sizeof(RingBuffer));
    }
    if (g_shm_fd >= 0) close(g_shm_fd);
    shm_unlink(SHM_NAME);
}

static bool write_slot(const std::string& json_str) {
    // Wait for a free slot
    while (sem_wait(&g_ring->slots_free) < 0) {
        if (errno == EINTR) { if (g_stop) return false; continue; }
        perror("sem_wait slots_free"); return false;
    }

    int rc = pthread_mutex_lock(&g_ring->mutex);
    if (rc == EOWNERDEAD) {
        pthread_mutex_consistent(&g_ring->mutex);
    } else if (rc != 0) {
        return false;
    }

    int idx = g_ring->head % RING_SIZE;
    int len = std::min((int)json_str.size(), MSG_SIZE - 1);
    memcpy(g_ring->slots[idx].data, json_str.c_str(), len);
    g_ring->slots[idx].data[len] = '\0';
    g_ring->slots[idx].len = len;
    g_ring->head++;

    pthread_mutex_unlock(&g_ring->mutex);
    sem_post(&g_ring->items_ready);
    return true;
}

int main() {
    signal(SIGTERM, on_sigterm);
    signal(SIGINT,  on_sigterm);

    if (!setup_shm()) { cleanup(); return 1; }

    using json = nlohmann::json;
    int seq = 0;

    while (!g_stop) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        int64_t ms = (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

        json msg = {
            {"device_id",  "a1b2c3d4-0000-0000-0000-000000000001"},
            {"timestamp",  ms},
            {"seq",        seq++},
            {"readings", {
                {{"name","temp"},     {"value", 20.0 + (seq % 10) * 0.5}, {"unit","C"}},
                {{"name","pressure"}, {"value", 101000.0 + seq % 100},    {"unit","Pa"}},
            }}
        };

        if (!write_slot(msg.dump())) break;
        usleep(100000);  // 100 ms
    }

    cleanup();
    return 0;
}
```

### aggregator.cpp

```cpp
// aggregator.cpp
#include <nlohmann/json.hpp>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <map>
#include <deque>
#include <string>
#include <algorithm>

static constexpr int   RING_SIZE  = 32;
static constexpr char  SHM_NAME[] = "/telemetry_shm";
static constexpr char  FIFO_PATH[] = "/tmp/telemetry.fifo";
static constexpr int   MSG_SIZE   = 512;
static constexpr int   WINDOW     = 10;

struct Slot { char data[MSG_SIZE]; int len; };
struct RingBuffer {
    pthread_mutex_t mutex;
    sem_t           slots_free;
    sem_t           items_ready;
    int             head, tail;
    Slot            slots[RING_SIZE];
};

static RingBuffer* g_ring = nullptr;
static int         g_shm_fd = -1;
static int         g_fifo_fd = -1;
static volatile sig_atomic_t g_stop = 0;

static void on_sigterm(int) { g_stop = 1; }

static bool setup() {
    g_shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (g_shm_fd < 0) { perror("shm_open"); return false; }

    g_ring = static_cast<RingBuffer*>(
        mmap(nullptr, sizeof(RingBuffer),
             PROT_READ | PROT_WRITE, MAP_SHARED, g_shm_fd, 0));
    if (g_ring == MAP_FAILED) { perror("mmap"); return false; }

    mkfifo(FIFO_PATH, 0600);  // OK if already exists
    g_fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (g_fifo_fd < 0) { perror("open fifo"); return false; }

    return true;
}

static void cleanup() {
    if (g_fifo_fd >= 0) close(g_fifo_fd);
    if (g_ring && g_ring != MAP_FAILED) munmap(g_ring, sizeof(RingBuffer));
    if (g_shm_fd >= 0) close(g_shm_fd);
}

int main() {
    signal(SIGTERM, on_sigterm);
    signal(SIGINT,  on_sigterm);
    signal(SIGPIPE, SIG_IGN);

    if (!setup()) { cleanup(); return 1; }

    using json = nlohmann::json;
    std::map<std::string, std::deque<double>> windows;

    while (!g_stop) {
        // Timed wait to allow clean shutdown
        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += 1;

        if (sem_timedwait(&g_ring->items_ready, &deadline) < 0) {
            if (errno == ETIMEDOUT || errno == EINTR) continue;
            perror("sem_timedwait"); break;
        }

        int rc = pthread_mutex_lock(&g_ring->mutex);
        if (rc == EOWNERDEAD) pthread_mutex_consistent(&g_ring->mutex);
        else if (rc != 0) break;

        int idx = g_ring->tail % RING_SIZE;
        std::string raw(g_ring->slots[idx].data, g_ring->slots[idx].len);
        g_ring->tail++;

        pthread_mutex_unlock(&g_ring->mutex);
        sem_post(&g_ring->slots_free);

        // Parse and aggregate
        json msg;
        try { msg = json::parse(raw); } catch (...) { continue; }

        for (const auto& r : msg.at("readings")) {
            std::string name = r.at("name").get<std::string>();
            double val       = r.at("value").get<double>();
            auto& w = windows[name];
            w.push_back(val);
            if ((int)w.size() > WINDOW) w.pop_front();
        }

        // Emit stats
        json stats;
        stats["timestamp"] = msg.at("timestamp");
        stats["stats"]     = json::object();
        for (auto& [name, w] : windows) {
            double sum = 0; double mx = w.front();
            for (double v : w) { sum += v; mx = std::max(mx, v); }
            stats["stats"][name] = {{"mean", sum / w.size()}, {"max", mx}};
        }

        std::string out = stats.dump() + "\n";
        if (write(g_fifo_fd, out.c_str(), out.size()) < 0) {
            if (errno == EPIPE) break;  // reader gone
        }
    }

    cleanup();
    return 0;
}
```

### Makefile

```makefile
# Makefile
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LIBS     = -lpthread -lrt

all: sensor_proc aggregator

sensor_proc: sensor_proc.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

aggregator: aggregator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f sensor_proc aggregator
	rm -f /tmp/telemetry.fifo
	rm -f /dev/shm/telemetry_shm

.PHONY: all clean
```

### supervisor.py

```python
# supervisor.py
"""
Launches sensor_proc and aggregator, reads aggregated stats from the FIFO,
validates against the JSON schema, and writes a CSV report.
"""
import csv
import json
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import Iterator

import jsonschema

FIFO_PATH = "/tmp/telemetry.fifo"
BIN_DIR   = Path(".")

STATS_SCHEMA = {
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "type": "object",
    "required": ["timestamp", "stats"],
    "properties": {
        "timestamp": {"type": "integer", "minimum": 0},
        "stats": {
            "type": "object",
            "additionalProperties": {
                "type": "object",
                "required": ["mean", "max"],
                "properties": {
                    "mean": {"type": "number"},
                    "max":  {"type": "number"},
                },
            },
        },
    },
}

_validator = jsonschema.Draft202012Validator(STATS_SCHEMA)


def validate(msg: dict) -> list[str]:
    return [e.message for e in _validator.iter_errors(msg)]


def read_fifo(path: str, timeout: float) -> Iterator[dict]:
    """Yields parsed JSON dicts from the FIFO until timeout."""
    deadline = time.monotonic() + timeout
    try:
        fd = open(path, "r")  # blocks until writer is ready
    except OSError as e:
        print(f"Cannot open FIFO: {e}", file=sys.stderr)
        return

    with fd:
        while time.monotonic() < deadline:
            line = fd.readline()
            if not line:
                time.sleep(0.05)
                continue
            try:
                yield json.loads(line)
            except json.JSONDecodeError as e:
                print(f"WARN: bad JSON: {e}", file=sys.stderr)


def run(duration: float = 5.0, report_path: Path = Path("report.csv")) -> list[dict]:
    # Ensure clean state
    try:
        os.unlink(FIFO_PATH)
    except FileNotFoundError:
        pass
    os.mkfifo(FIFO_PATH)

    sensor = subprocess.Popen([str(BIN_DIR / "sensor_proc")])
    time.sleep(0.2)  # let sensor_proc initialize shm
    aggregator = subprocess.Popen([str(BIN_DIR / "aggregator")])

    records = []
    errors  = []

    try:
        for msg in read_fifo(FIFO_PATH, timeout=duration):
            errs = validate(msg)
            if errs:
                errors.append({"msg": msg, "errors": errs})
                print(f"VALIDATION ERROR: {errs[0]}", file=sys.stderr)
            else:
                records.append(msg)
    finally:
        sensor.terminate()
        aggregator.terminate()
        for proc in (sensor, aggregator):
            try:
                proc.wait(timeout=3.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
        try:
            os.unlink(FIFO_PATH)
        except FileNotFoundError:
            pass

    # Write CSV
    if records:
        sensor_names = list(records[0]["stats"].keys())
        with open(report_path, "w", newline="") as f:
            writer = csv.writer(f)
            header = ["timestamp"] + [f"{n}_mean" for n in sensor_names] + \
                     [f"{n}_max"  for n in sensor_names]
            writer.writerow(header)
            for r in records:
                row = [r["timestamp"]] + \
                      [r["stats"][n]["mean"] for n in sensor_names] + \
                      [r["stats"][n]["max"]  for n in sensor_names]
                writer.writerow(row)
        print(f"Wrote {len(records)} rows to {report_path}")

    if errors:
        print(f"WARNING: {len(errors)} messages failed validation", file=sys.stderr)

    return records


if __name__ == "__main__":
    msgs = run()
    assert msgs, "No valid messages received"
    print(f"Pipeline OK — {len(msgs)} valid messages")
```

### tests/test_pipeline.py

```python
# tests/test_pipeline.py
import json
import subprocess
import sys
import time
import os
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from supervisor import validate, run


def test_validate_accepts_good_message() -> None:
    msg = {
        "timestamp": 1700000000000,
        "stats": {
            "temp":     {"mean": 22.5, "max": 25.0},
            "pressure": {"mean": 101200.0, "max": 101300.0},
        },
    }
    assert validate(msg) == []


def test_validate_rejects_missing_stats() -> None:
    errors = validate({"timestamp": 1700000000000})
    assert errors  # non-empty


def test_validate_rejects_negative_timestamp() -> None:
    msg = {"timestamp": -1, "stats": {}}
    errors = validate(msg)
    assert errors


@pytest.mark.slow
def test_pipeline_end_to_end(tmp_path: Path) -> None:
    """Starts the real pipeline; expects at least 3 valid messages in 6 seconds."""
    report = tmp_path / "report.csv"
    msgs = run(duration=6.0, report_path=report)
    assert len(msgs) >= 3, f"Expected >=3 messages, got {len(msgs)}"
    assert report.exists()
    lines = report.read_text().splitlines()
    assert len(lines) >= 4  # header + at least 3 data rows
```

```bash
# Build and run
cd pipeline
make
python3 supervisor.py

# Run tests (unit tests only)
pytest tests/ -v -m "not slow"

# Run including the end-to-end test
pytest tests/ -v
```

---

## 2. 20 interview questions with model answers

Work through these out loud. Set a timer — aim for 2 minutes per question.

---

**1. Compare FIFOs, sockets, message queues, and shared memory. When do you pick which?**

FIFOs: byte-stream, unidirectional, kernel-buffered, simple. Use for logging pipelines and one-way control channels where simplicity matters. Sockets (Unix domain): bidirectional, can do datagram or stream, work across hosts if you switch to TCP. Use when you need full-duplex or pub/sub to multiple clients. POSIX message queues: kernel-buffered, typed, prioritized. Use when you want natural message framing and priority without shared memory complexity. Shared memory: fastest (no kernel copy on the data path), but requires explicit synchronization. Use for high-throughput data pipelines between local processes. In practice: shm for data, FIFO or socket for control.

---

**2. Walk through the lifecycle of POSIX shared memory.**

`shm_open(name, O_CREAT|O_RDWR, 0600)` → returns a file descriptor. `ftruncate(fd, size)` → sets the size (new segment is zero-initialized). `mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)` → maps it into the process's address space. `close(fd)` → the mapping persists; the fd is no longer needed. Use it. `munmap(ptr, size)` → removes the mapping. `shm_unlink(name)` → removes the name from the filesystem; the segment persists until all mappings are released (like `unlink` on a file). Segment appears in `/dev/shm` on Linux.

---

**3. Explain `memory_order_acquire` vs `memory_order_release` with a concrete example.**

Release on the producer side: `flag.store(1, memory_order_release)` — all prior writes are visible to any thread that later acquires this flag. Acquire on the consumer side: `while (!flag.load(memory_order_acquire));` — all writes that happened before the release store are now visible. The pair creates a happens-before edge. Without it, the compiler and CPU can reorder the data write after the flag store, or the consumer can read the data before the flag is visible. `seq_cst` is stronger — it provides a total order across all atomic operations — but costs a full memory fence on x86 for stores (which are already acquire/release). Use acquire/release when you only need to synchronize one producer with one consumer.

---

**4. What's the difference between a process-shared mutex and a regular `std::mutex`?**

`std::mutex` lives in process memory and uses futex internally. It cannot be used across processes. A process-shared mutex uses `pthread_mutex_t` with `pthread_mutexattr_setpshared(attr, PTHREAD_PROCESS_SHARED)` and must live in shared memory. It's visible to all processes that map that shared memory segment. Regular mutex failure: if a thread dies holding it, no automatic recovery — the mutex is permanently locked. Robust mutex (`PTHREAD_MUTEX_ROBUST`): if the owning thread/process dies, the next `pthread_mutex_lock` returns `EOWNERDEAD`, allowing the caller to call `pthread_mutex_consistent` and recover.

---

**5. A process holding a process-shared mutex crashes. What happens?**

With a standard process-shared mutex: the mutex remains locked. Any process waiting on it blocks indefinitely. With `PTHREAD_MUTEX_ROBUST`: the next `pthread_mutex_lock` call returns `EOWNERDEAD`. The caller must decide whether the protected data is in a consistent state. If it is (or it's been repaired), call `pthread_mutex_consistent` to mark it so; then unlock normally. If not, unlock and propagate an error — the data is gone. This is the correct pattern for embedded systems where process death is a real failure mode.

---

**6. Walk me through what happens between `shm_open` and `mmap`.**

`shm_open` creates or opens a POSIX shared memory object, returning a regular file descriptor pointing to an anonymous inode in `tmpfs` (on Linux, backed by `/dev/shm`). The object has size zero at creation. `ftruncate` sets the size — the kernel allocates pages but doesn't fault them in yet. `mmap` with `MAP_SHARED` maps the object into the process's virtual address space; the mapping is shared with any other process that maps the same name. Pages are demand-faulted on first access. Two processes mapping the same object see the same physical pages.

---

**7. Two processes share memory. One writes a 64-bit value. Is the read on the other side atomic?**

Not guaranteed without explicit atomics. On x86-64, naturally aligned 64-bit loads/stores are single-bus-cycle and observably atomic at the hardware level — but the C++ memory model does not guarantee this for non-atomic types. The compiler can split the write into two 32-bit stores. Use `std::atomic<uint64_t>` in shared memory, or `_Atomic uint64_t` in C. Then the store is guaranteed atomic (no tearing) and you control the memory order. Without this, it's undefined behavior regardless of what the hardware does.

---

**8. How do you cleanly tear down shared memory if a process crashes?**

Register a `SIGTERM` / `SIGINT` handler that calls `shm_unlink` before exit. For `SIGKILL` (which can't be caught): use a watchdog process that owns the `shm_unlink` responsibility. Alternatively, the last process to exit can call `shm_unlink`. In practice: designate one process as the shm owner (usually the first to create it). That process calls `shm_unlink` on exit. Other processes only `munmap` — they don't unlink. If the owner crashes, the segment persists in `/dev/shm` until manually cleaned or the system reboots. A startup-time cleanup (check `/dev/shm` for stale segments by name) handles crash recovery.

---

**9. Difference between a mutex and a binary semaphore?**

Both provide mutual exclusion, but they differ in ownership semantics. A mutex has an owner: only the thread that locked it can unlock it. Violating this is undefined behavior. A binary semaphore has no owner: any thread can post it, even one that didn't wait it. Semaphores are signaling primitives; mutexes are ownership primitives. Use a mutex for protecting a critical section. Use a semaphore to signal availability (e.g., "item ready to consume"). Consequence: robust mutexes (`EOWNERDEAD`) work because the kernel tracks the owner; robust semaphores don't exist in POSIX.

---

**10. What's priority inversion? How do you prevent it?**

Priority inversion: a high-priority thread is blocked waiting for a lock held by a low-priority thread, which is itself preempted by a medium-priority thread. The high-priority thread effectively runs at the priority of the low-priority one. Prevention: priority inheritance (the kernel temporarily raises the lock-holder's priority to match the highest waiter — available with `PTHREAD_PRIO_INHERIT`). Priority ceiling (`PTHREAD_PRIO_PROTECT`): the lock-holder is always raised to a predetermined ceiling. The Mars Pathfinder reboot bug in 1997 was caused by priority inversion; it was fixed by enabling priority inheritance on the relevant mutex.

---

**11. Design IPC for 1 sensor process, 3 worker processes, 1 logger.**

Sensor → workers: shared memory ring buffer with 3 reader slots, or a work queue protected by a mutex + semaphores. Sensor pushes data; each worker reads independently (if each needs all data) or a round-robin model (if work is partitioned). Worker → logger: each worker writes to a shared memory region or a named FIFO. For a logger aggregating from 3 sources, Unix domain socket with each worker as a client and logger as server scales better (multiplexing with `epoll`). Control channel (start/stop/reset): a FIFO per worker process or Unix signals (`SIGUSR1`/`SIGUSR2`). The key trade-off: FIFOs are simple but require one reader; sockets handle fan-in naturally.

---

**12. When would you use `acquire`/`release` instead of `seq_cst`?**

When you only need to synchronize a specific producer-consumer pair and don't need a total global order. `seq_cst` provides a single total order across all atomic operations in all threads — necessary when multiple threads write to different atomics and other threads read both. The cost: on x86 stores are already release, but `seq_cst` stores require an `mfence` or `lock xchg`. On ARM, both loads and stores need barriers. If you have a single flag that one thread sets and another polls, acquire/release is sufficient and cheaper. In the SPSC lock-free queue in Day 4, acquire/release on head/tail is the right choice — no other atomics interact with them globally.

---

**13. `std::shared_ptr` is thread-safe for the control block but not the pointee — explain.**

The reference count in the control block is modified atomically — `shared_ptr` copy/destroy is thread-safe. Two threads can copy and destroy `shared_ptr` instances pointing to the same object simultaneously without data races on the count. However, the pointed-to object itself has no internal synchronization. Two threads calling non-const methods on the same object through different `shared_ptr` copies is a data race unless the object has its own locking. Additionally, reading and writing the `shared_ptr` variable itself (the pointer + count pair) from two threads is a data race — you must protect the `shared_ptr` variable with a mutex or use `std::atomic<std::shared_ptr<T>>` (C++20).

---

**14. How would you write a Python test harness for a multi-process C++ daemon?**

Launch the daemon as a subprocess in a `pytest` fixture with `yield`-based teardown. Communicate through its actual IPC interface (FIFO, Unix socket, shm) — don't mock it at the C++ level. Use `tmp_path` for any files or FIFOs it needs. Read output with a timeout to prevent hanging CI. For shm-based output, read from `/dev/shm` directly or have a reader process. Assert on observable output rather than internal state. Run under `valgrind` in a separate `@pytest.mark.slow` test to catch leaks. Use `monkeypatch.setenv` to inject configuration without modifying files.

---

**15. When would you NOT use JSON in an embedded protocol?**

Serial protocols (UART, CAN, SPI) with tight bandwidth budgets — binary packing is 3–10x smaller. High-frequency telemetry (>10k msg/s) — JSON parsing costs CPU and allocates memory. Safety-critical or schema-enforced protocols — the format must enforce schema, not just convention (use Protobuf or ASN.1). Real-time control loops — variable message size and parse latency are unacceptable. Memory-constrained MCUs — a JSON parser adds 10–50 KB of code. For IoT, CBOR (RFC 7049) is the IETF-approved binary alternative that mirrors JSON's data model.

---

**16. How do you evolve a JSON schema without breaking old clients?**

Add fields as optional only. Never remove `required` fields. Never change a field's type. Keep `additionalProperties` absent in schemas exposed to external clients. Add a `schema_version` integer field from day one. Support at least two versions simultaneously during transitions. For hard breaks: run parallel endpoints or topics, migrate clients, then deprecate the old version. Document field deprecation in the schema via `deprecated: true` (JSON Schema 2019-09+) before removal.

---

**17. How do you debug a process hanging on `sem_wait`?**

`strace -p <pid>` — shows the blocked syscall and its arguments. `cat /proc/<pid>/status` — check `State: S (sleeping)` and the semaphore address. `lsof -p <pid>` — if using named semaphores, shows the file descriptor. `ipcs -s` — lists System V semaphores (less relevant for POSIX). For POSIX named semaphores, check `/dev/shm/sem.*`. For unnamed semaphores in shm, attach with `gdb -p <pid>` and inspect the `sem_t` value directly. Also check: did the producer crash? Is `slots_free` or `items_ready` stuck at 0? A robust mutex that was never marked consistent will also cause a permanent hang.

---

**18. Describe RAII and why it matters for IPC resources.**

RAII (Resource Acquisition Is Initialization): bind resource lifetime to object lifetime. Constructor acquires, destructor releases. Guarantees release on all exit paths — normal return, exception, early return. For IPC: wrap a `shm_fd` in a class whose destructor calls `close`; wrap a mapped region so the destructor calls `munmap` and optionally `shm_unlink`; wrap a FIFO fd so the destructor closes it. Without RAII, any early return or exception leaks the resource. In C++20, `std::unique_ptr` with a custom deleter, or a small wrapper class, is the idiomatic approach. Never manually manage IPC resource cleanup in function bodies — a signal arriving between the creation and the cleanup will leak it.

---

**19. `volatile` for thread synchronization — why doesn't it work?**

`volatile` tells the compiler "don't cache this in a register; read/write memory every time." It does NOT prevent CPU reordering, does NOT emit memory barriers, and is NOT part of the C++ memory model for concurrency. Two CPUs can each have a stale cache line for a volatile variable. `std::atomic<T>` is the correct tool: it guarantees atomicity (no tearing), controls memory ordering via the memory model, and emits the appropriate hardware barriers. `volatile` is for memory-mapped I/O registers, not inter-thread communication. Using `volatile` for synchronization is undefined behavior under the C++ standard.

---

**20. Walk me through your IPC pipeline — the one you built today.**

*Practice this in 90 seconds. Suggested structure:*

"sensor_proc generates fake readings at 100 ms intervals, serializes them as JSON using nlohmann, and pushes them into a POSIX shared memory ring buffer. The ring buffer is protected by a process-shared robust mutex and two counting semaphores — one for free slots, one for filled slots — following the classic bounded buffer pattern. aggregator maps the same shm segment, reads entries with a timed semaphore wait for clean shutdown, computes rolling mean and max per sensor name, and writes JSON lines to a FIFO. The Python supervisor launches both binaries, reads the FIFO with a timeout, validates each message against a JSON schema, and writes a CSV report. Each C++ process handles SIGTERM, releases the mutex if held, and calls shm_unlink on the owner side. The pytest suite has unit tests for the validator and an end-to-end test that runs the full pipeline for 6 seconds."

---

## 3. Things candidates often forget

Run through this checklist before the interview:

- [ ] `shm_unlink` must be called; `munmap` alone doesn't remove the segment from `/dev/shm`
- [ ] FIFO `open()` blocks until both ends are present — this is often the source of "my program hangs at startup"
- [ ] `SIGPIPE` is raised, not `EPIPE`, when writing to a FIFO with no reader; handle or ignore it
- [ ] `std::atomic` in shared memory works if you use `std::atomic_ref` (C++20) or ensure the type is lock-free; complex `std::atomic` types may use an internal mutex that is NOT process-shared
- [ ] Shared memory persists after process death — always design a cleanup owner
- [ ] Robust mutexes require `pthread_mutex_consistent` after `EOWNERDEAD` or the mutex stays broken
- [ ] `memory_order_relaxed` is not "no ordering" — it's "no synchronization with other atomics"
- [ ] Python's `subprocess.run` with `capture_output=True` can deadlock if the subprocess fills both stdout and stderr buffers; use `communicate()` or drain threads
- [ ] `json::operator[]` on a missing key inserts null; use `.at()` for validation
- [ ] JSON integers > 2^53 lose precision in JavaScript parsers; use strings for 64-bit IDs

---

## 4. Pre-interview routine (60 minutes the morning of)

Start this 60 minutes before you need to leave.

**Minutes 0–15: Re-read your cheatsheets**
Flip through the cheatsheet sections from Days 1–6. Don't re-read the full lectures. You're priming recall, not learning.

**Minutes 15–30: Whiteboard the pipeline**
On paper (not a screen), draw the Day 7 pipeline architecture. Label: shm segment name, FIFO path, semaphore semantics, which process calls `shm_unlink`. Be able to draw this in under 3 minutes.

**Minutes 30–45: Say 5 things out loud**
Pick 5 questions from Section 2 that felt shaky yesterday. Say the answer aloud — not in your head. Speaking activates retrieval differently than reading. If you blank, look it up, then say it again.

**Minutes 45–55: Scan the red flags list from the learning plan**
Read: volatile, System V vs POSIX confusion, "shared memory is fastest" without mentioning sync, Python GIL claims without nuance. Make sure none of these would come out of your mouth unguarded.

**Minutes 55–60: Stop reviewing**
Close everything. Drink water. The interview tests thinking under pressure, not memorization.

---

## 5. Cheatsheet

### IPC selection at a glance

| Mechanism | Direction | Kernel copy | Persistence | Best for |
|---|---|---|---|---|
| FIFO | Unidirectional | Yes | Until unlinked | Logging, control channels |
| Unix socket | Bidirectional | Yes | Process lifetime | Full-duplex, fan-in |
| POSIX mq | Both (discrete msgs) | Yes | Until unlinked | Typed, prioritized messages |
| Shared memory | N-to-N | No (mmap) | Until `shm_unlink` | High-throughput data |

### POSIX IPC syscalls

```c
/* Shared memory */
shm_open(name, flags, mode)   ftruncate(fd, size)
mmap(NULL, size, prot, MAP_SHARED, fd, 0)
munmap(ptr, size)             shm_unlink(name)

/* Semaphores */
sem_open(name, flags, mode, value)   sem_wait/post/trywait/timedwait
sem_init(sem, pshared=1, value)      sem_destroy
sem_close  sem_unlink

/* Mutex (process-shared) */
pthread_mutexattr_setpshared(attr, PTHREAD_PROCESS_SHARED)
pthread_mutexattr_setrobust(attr, PTHREAD_MUTEX_ROBUST)
/* On EOWNERDEAD: */ pthread_mutex_consistent(mutex)
```

### C++ memory orders

| Order | Use when |
|---|---|
| `relaxed` | Counter increments; no sync needed |
| `acquire` | Load that starts a happens-before (consumer flag read) |
| `release` | Store that ends a happens-before (producer flag set) |
| `acq_rel` | Read-modify-write that does both (e.g., `fetch_add` in a lock) |
| `seq_cst` | Total order needed across multiple atomic variables |

### Pytest patterns

```python
@pytest.fixture
def resource(tmp_path):
    r = setup(tmp_path)
    yield r
    teardown(r)   # runs even on test failure

@pytest.mark.parametrize("a,b", [(1,2),(3,4)])
def test_f(a, b): ...

monkeypatch.setenv / setattr / delattr
with patch("mod.subprocess.run") as m: m.return_value = ...
```

### JSON Schema essentials

```
required  additionalProperties  $ref  oneOf/anyOf/allOf
type  enum  minimum/maximum  minLength/maxLength  pattern
```

### nlohmann safety

```cpp
j = json::parse(raw);            // catch json::parse_error
j.at("key").get<int64_t>();      // catch json::out_of_range, json::type_error
j.contains("key")                // safe check without insertion
j.value("key", default_val)      // returns default if missing
```