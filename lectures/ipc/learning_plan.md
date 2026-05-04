# 7-Day Crash Course: Embedded Systems Interview Prep

**Target role:** Systems / embedded engineer
**Starting point:** Comfortable with C++ and Python, weak on POSIX IPC
**Goal:** Walk into the interview confident on POSIX IPC, sharp on C++ concurrency, fluent enough on Python automation and JSON to handle adjacent questions.

---

## Strategy

Because IPC is the weakest area and the most likely to differentiate you in an embedded interview, **roughly 50% of total time goes to POSIX IPC** (Days 1–3 + revisits). The remaining time covers C++ concurrency (Day 4), Python (Day 5), JSON (Day 6), and a full mock-interview consolidation day (Day 7).

**Daily structure** (~4–6 hours/day):
1. **Concept block (60–90 min):** read the lecture, take notes
2. **Hands-on lab (90–120 min):** build something runnable
3. **Drill block (45–60 min):** interview-style Q&A and code review
4. **Recap (15 min):** write 5 bullet points you'd say out loud in an interview

Skip nothing. If a day runs long, cut the drill block — never cut the lab.

---

## Day 1 — POSIX IPC Foundations + Named Pipes (FIFOs)

**Why this first:** FIFOs are the gentlest entry point and reinforce the file-descriptor mental model that everything else (shm, semaphores, sockets) builds on.

### Concepts
- Process model recap: PID, fork/exec, file descriptors, the kernel as the boundary between processes
- The four POSIX IPC families: pipes/FIFOs, message queues, shared memory, semaphores — when each is the right tool
- Anonymous pipes (`pipe()`) vs named pipes (`mkfifo`): lifetime, namespace, blocking semantics
- Blocking behavior of FIFOs: open() blocks until both ends are present; read on empty FIFO blocks; write to FIFO with no readers raises SIGPIPE
- `O_NONBLOCK`, `select()` / `poll()` / `epoll` for multiplexing
- PIPE_BUF and atomic writes (≤ PIPE_BUF bytes are atomic)

### Lab
1. Write a producer process that creates a FIFO and writes timestamped log lines every 500 ms.
2. Write a consumer process that reads and prints them.
3. Kill the consumer mid-stream — observe SIGPIPE on the producer. Handle it.
4. Add a second consumer. Observe what happens (data is split, not duplicated).
5. Convert the consumer to non-blocking + `poll()`.

### Interview drills
- "Why would you choose a FIFO over a socket?"
- "What happens if a writer writes 5KB and PIPE_BUF is 4096?"
- "How do you build a pub/sub system with FIFOs?" (trick — you don't, you use sockets or shm; explain why)

---

## Day 2 — POSIX Shared Memory + Memory Mapping

**Why now:** Shared memory is the highest-performance IPC and the most likely deep-dive question for embedded/low-latency roles.

### Concepts
- POSIX `shm_open` + `mmap` vs System V `shmget` (know both names; POSIX is preferred)
- Lifecycle: `shm_open` → `ftruncate` → `mmap` → use → `munmap` → `shm_unlink`
- Persistence: shared memory survives process death until `shm_unlink` is called — and the implications
- `/dev/shm` on Linux: it's a tmpfs mount; you can `ls` your shared memory segments
- Memory ordering and the **need for synchronization**: shared memory alone gives you no atomicity
- Cache coherence vs memory ordering — what hardware guarantees vs what the C++/C memory model guarantees
- Layout for IPC structs: alignment, padding, no pointers (offsets only), versioning fields

### Lab
1. Create a shared memory segment containing a fixed-size ring buffer struct (head, tail, slots).
2. Producer writes integers; consumer reads them.
3. First version: no synchronization — observe corruption under load.
4. Second version: add a `std::atomic<uint64_t>` head/tail with `memory_order_acquire/release`. Verify correctness.
5. Use `pmap` and `cat /proc/<pid>/maps` to see your segment mapped.

### Interview drills
- "Walk me through what happens between `shm_open` and `mmap`."
- "Two processes share memory. One writes a 64-bit value. Is the read on the other side atomic?"
- "How do you cleanly tear down shared memory if a process crashes?"
- "Why is shared memory faster than pipes?" (no kernel copy on data path)

---

## Day 3 — POSIX Semaphores + Putting IPC Together

### Concepts
- Named semaphores (`sem_open`) vs unnamed semaphores (`sem_init`) — when to use each
- Unnamed semaphores in shared memory: this is the standard pattern for cross-process synchronization
- Counting vs binary semaphores
- `sem_wait` / `sem_post` / `sem_trywait` / `sem_timedwait`
- Semaphores vs mutexes vs condition variables — and what `pthread_mutexattr_setpshared(PTHREAD_PROCESS_SHARED)` gives you
- Deadlock, priority inversion, the producer-consumer pattern
- Robust mutexes (`PTHREAD_MUTEX_ROBUST`) — critical for embedded systems where a process may die holding a lock

### Lab
1. Take Day 2's ring buffer. Replace the lock-free atomics with a process-shared mutex + two semaphores ("slots empty" and "items available").
2. Implement a bounded blocking queue. Producer blocks when full; consumer blocks when empty.
3. Kill the producer while it holds the mutex. Observe the consumer hang. Switch to a robust mutex; recover.
4. Build a small "sensor pipeline": process A reads (simulated) sensor data, writes via shm; process B aggregates; process C logs. Use a FIFO for control commands (start/stop/reset) and shm + semaphores for data.

### Interview drills
- "Difference between a mutex and a binary semaphore?"
- "What's priority inversion? How do you prevent it?"
- "A process holding a process-shared mutex crashes. What happens to the others?"
- "Design IPC for a system with 1 sensor process, 3 worker processes, 1 logger." (defend your choices)

---

## Day 4 — Modern C++ Concurrency

**Why now:** You already know C++; this is sharpening, not learning. Focus on what intersects with IPC.

### Concepts
- `std::thread`, `std::jthread` (C++20), `std::async`, `std::future`, `std::promise`
- `std::mutex`, `std::lock_guard`, `std::unique_lock`, `std::scoped_lock`
- `std::condition_variable` — the wait/notify pattern, why you need a predicate, spurious wakeups
- `std::atomic<T>`: load/store, compare_exchange, fetch_add
- The C++ memory model: `memory_order_relaxed`, `acquire`, `release`, `acq_rel`, `seq_cst` — when each is appropriate
- Lock-free vs wait-free, ABA problem
- Modern STL relevant to systems: `std::span`, `std::string_view`, `std::optional`, `std::variant`, `std::expected` (C++23)
- Move semantics refresher: rvalue references, perfect forwarding, when copies actually happen
- RAII for resource handles (file descriptors, shm handles) — write a small RAII wrapper

### Lab
1. Implement a thread-safe bounded queue using `std::mutex` + `std::condition_variable`.
2. Reimplement it lock-free using `std::atomic` (single-producer/single-consumer is enough — say so in the interview).
3. Benchmark both with `std::chrono::steady_clock`. Note where the lock-free version wins and where it loses.
4. Wrap a POSIX shm segment in an RAII class. Verify with valgrind that nothing leaks.

### Interview drills
- "When would you use `acquire`/`release` instead of `seq_cst`?"
- "Show me how a `condition_variable` wait works under the hood."
- "Why is `std::shared_ptr` thread-safe for the control block but not the pointee?"
- "Write a thread-safe singleton. Now do it without `std::call_once`."

---

## Day 5 — Python for Automation, Testing, and Data

**Why now:** Embedded teams use Python heavily for test harnesses, log processing, and tooling. You already write Python; this day is about idioms interviewers expect.

### Concepts
- `subprocess` (the right and wrong way: `run`, `Popen`, capturing output, timeouts, never `shell=True` with user input)
- `argparse` for CLI tools; `pathlib` instead of `os.path`
- File I/O patterns: context managers, generators for large files, `mmap` module
- `logging` module: levels, handlers, formatters — never use `print` in real tooling
- `pytest`: fixtures, parametrize, monkeypatch, `tmp_path`, marking slow tests
- `unittest.mock` for patching subprocess, file I/O, network calls
- Concurrency in Python: `threading`, `multiprocessing`, `concurrent.futures`, `asyncio` — and the GIL
- Data processing: `csv`, `json`, basic `pandas` (read_csv, filter, groupby, to_csv)
- Talking to C/C++ from Python: `ctypes` and `cffi` basics (briefly — just enough to demo)

### Lab
1. Write a CLI tool `logscan` that takes a directory, finds all `*.log` files, extracts ERROR lines, and emits a JSON report. Use `argparse`, `pathlib`, `logging`.
2. Write `pytest` tests for it, including a fixture that builds a fake log directory in `tmp_path`.
3. Write a script that launches your Day 3 IPC pipeline as subprocesses, sends test commands via the FIFO, reads the shm output, and asserts results. **This is the kind of script embedded teams actually use.**
4. Add a `--parallel` flag to `logscan` using `concurrent.futures.ProcessPoolExecutor`.

### Interview drills
- "When would you choose `multiprocessing` over `threading` in Python?"
- "How do you test code that calls `subprocess`?"
- "Walk me through a pytest fixture with `yield`."
- "How would you automate testing of a C++ daemon?"

---

## Day 6 — JSON: Schema, Parsing, Serialization

**Why now:** JSON is everywhere — config files, IPC payloads, test artifacts, telemetry. The interview bar is "do you understand the trade-offs," not "can you parse it."

### Concepts
- JSON spec (RFC 8259): types, what's NOT in JSON (no comments, no trailing commas, no `NaN`/`Infinity`)
- Encoding pitfalls: integers > 2^53 (JS precision), Unicode, escaping
- JSON Schema (draft 2020-12): `type`, `properties`, `required`, `additionalProperties`, `oneOf`/`anyOf`/`allOf`, `$ref`, `pattern`, `enum`
- Validation: when to validate (boundaries, never internally), how to report errors usefully
- Parsing in C++: `nlohmann/json` (ergonomic), `RapidJSON` / `simdjson` (fast). Know the trade-offs.
- Parsing in Python: `json` stdlib, `pydantic` for schema-validated models, `jsonschema` for explicit schema validation
- Streaming/incremental parsing for large files
- Alternatives and when to reach for them: Protocol Buffers, FlatBuffers, MessagePack, CBOR — embedded interviewers love this question

### Lab
1. Design a JSON schema for a sensor telemetry message: `device_id` (string, UUID), `timestamp` (int64, ms epoch), `readings` (array of `{name, value, unit}`), optional `tags` (object of string→string).
2. Write a `jsonschema` validator in Python; test with valid and invalid samples.
3. Write a C++ program using `nlohmann/json` that produces and consumes these messages. Send them over the FIFO from Day 1.
4. Benchmark `nlohmann/json` vs `simdjson` parsing 10 MB of telemetry. Be ready to discuss the result.

### Interview drills
- "When would you NOT use JSON?" (binary embedded protocols, high-throughput, schema evolution requirements)
- "How do you evolve a JSON schema without breaking old clients?"
- "What's the difference between `oneOf` and `anyOf`?"
- "Walk me through error handling when parsing untrusted JSON in C++."

---

## Day 7 — Mock Interview, Integration, Recap

### Morning: full integration project (3 hrs)
Build one program that ties everything together. This is your **walk-in story** for the interview.

**Spec:** A small embedded telemetry pipeline.
- **Sensor process (C++):** generates fake readings, serializes to JSON via `nlohmann/json`, writes into a shared memory ring buffer.
- **Aggregator process (C++):** reads from shm, computes rolling stats (mean, max), writes results to a FIFO.
- **Python supervisor:** launches both processes, reads the FIFO, validates each message against your JSON schema, writes a CSV report. Uses `pytest` for end-to-end tests.
- Synchronization: process-shared mutex + counting semaphore in shm.
- Clean shutdown: signal handler in C++ that releases shm and unlinks it.

You should be able to demo this on a whiteboard from memory. Practice describing it in 90 seconds.

### Afternoon: drill block (2 hrs)
Run through this list out loud, as if to an interviewer:
1. Compare FIFOs, sockets, message queues, and shared memory. When do you pick which?
2. Walk through the lifecycle of POSIX shared memory.
3. Explain `memory_order_acquire` vs `memory_order_release` with a concrete producer-consumer example.
4. What's the difference between a process-shared mutex and a regular mutex? When does each fail?
5. How would you write a Python test harness for a multi-process C++ daemon?
6. Design a JSON schema for a configuration file with optional fields and version evolution.
7. How do you debug a process that's hanging on `sem_wait`? (`strace`, `gdb`, `lsof`, `/proc/<pid>/status`)
8. Describe RAII and why it matters for IPC resources.

### Evening: recap (45 min)
Write a one-page cheatsheet (by hand). Topics: IPC syscalls, C++ memory orders, pytest patterns, JSON Schema keywords. **Re-read the morning before the interview.**

---

## Reference Materials (use selectively — don't try to read all of these)

### POSIX IPC
- *The Linux Programming Interface* by Michael Kerrisk — chapters 43–57. The bible. Use as reference.
- `man 7 sem_overview`, `man 7 shm_overview`, `man 7 fifo`
- Beej's Guide to Unix IPC (free online — short and friendly)

### C++
- *C++ Concurrency in Action* (2nd ed.) by Anthony Williams — skim chapters 1–5, focus on chapter 5 (memory model)
- cppreference.com — primary reference for STL and memory orders

### Python
- *Effective Python* by Brett Slatkin — the items on subprocess, generators, and concurrency
- pytest official docs — fixtures and parametrize sections

### JSON
- json-schema.org — the "Getting Started" guide
- nlohmann/json README — comprehensive examples
- simdjson docs — for the performance discussion

---

## Daily Time Budget (suggested)

| Day | Topic | Concept | Lab | Drills | Recap |
|-----|-------|---------|-----|--------|-------|
| 1 | FIFOs | 75 min | 120 min | 45 min | 15 min |
| 2 | Shared memory | 90 min | 120 min | 45 min | 15 min |
| 3 | Semaphores + integration | 75 min | 150 min | 45 min | 15 min |
| 4 | C++ concurrency | 60 min | 120 min | 60 min | 15 min |
| 5 | Python | 60 min | 120 min | 45 min | 15 min |
| 6 | JSON | 60 min | 120 min | 45 min | 15 min |
| 7 | Integration + mock | — | 180 min | 120 min | 45 min |

---

## Red Flags to Avoid in the Interview

- Saying "shared memory is fastest" without mentioning that you still need synchronization.
- Using `volatile` for thread synchronization (it's not — explain `std::atomic` instead).
- Confusing System V (`shmget`) and POSIX (`shm_open`) APIs. Know which is which.
- Claiming Python is "slow" without nuance — discuss the GIL, multiprocessing, and C extensions.
- Designing a JSON schema with no versioning strategy.
- Forgetting to `shm_unlink` or unlink FIFOs in your cleanup story.
