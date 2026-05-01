"""
=============================================================================
CHAPTER 13: CONCURRENCY & PARALLELISM
=============================================================================
Understanding Python's concurrency model is essential for building
responsive and efficient applications.

KEY DISTINCTION:
- Concurrency: managing multiple tasks (may not run simultaneously)
- Parallelism: actually executing multiple tasks at the same time

PYTHON'S OPTIONS:
1. threading: concurrent I/O, limited by GIL for CPU work
2. multiprocessing: true parallelism (separate processes)
3. asyncio: single-threaded concurrency for I/O-bound code
4. concurrent.futures: high-level API for both threads and processes

DECISION GUIDE:
- I/O-bound (network, files, DB): asyncio (best) or threading
- CPU-bound (computation): multiprocessing
- Simple parallel tasks: concurrent.futures
- Need shared memory: threading (with locks)

=============================================================================
"""

# =============================================================================
# 13.1 THE GIL (Global Interpreter Lock)
# =============================================================================
"""
WHAT: The GIL is a mutex that allows only ONE thread to execute Python
      bytecode at a time. Only CPython has this (not Jython, PyPy).

WHY the GIL exists:
- Simplifies memory management (reference counting is thread-safe)
- Makes C extensions easier to write
- Historical design choice that's very hard to remove

IMPLICATIONS:
- Threading CANNOT parallelize CPU-bound Python code
- Threading DOES help with I/O-bound code (GIL is released during I/O)
- For true CPU parallelism: use multiprocessing or C extensions

FUTURE: Python 3.13+ introduces experimental free-threaded mode (no GIL)
"""


# =============================================================================
# 13.2 THREADING
# =============================================================================
"""
WHAT: Threads share memory space. Good for I/O-bound concurrency.

WHEN to use threading:
- Multiple network requests simultaneously
- Concurrent file I/O
- Background tasks in GUI applications
- Any I/O-bound work where you wait for external resources

WHEN NOT to use:
- CPU-bound computation (GIL prevents parallelism)
- When shared state becomes complex (use multiprocessing or asyncio)
"""

import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

# Basic thread creation
def download_page(url: str) -> str:
    """Simulate downloading a web page."""
    print(f"  Downloading {url}...")
    time.sleep(0.5)  # Simulate network I/O
    return f"Content of {url}"

# Method 1: Thread objects
threads = []
for url in ["page1", "page2", "page3"]:
    t = threading.Thread(target=download_page, args=(url,))
    threads.append(t)
    t.start()

for t in threads:
    t.join()  # Wait for all threads to complete

# Method 2: ThreadPoolExecutor (PREFERRED — cleaner, handles exceptions)
urls = ["page1", "page2", "page3", "page4", "page5"]

with ThreadPoolExecutor(max_workers=3) as executor:
    # Submit all tasks
    futures = {executor.submit(download_page, url): url for url in urls}

    # Process results as they complete
    for future in as_completed(futures):
        url = futures[future]
        try:
            result = future.result()
            print(f"  Got: {result}")
        except Exception as e:
            print(f"  Error downloading {url}: {e}")

# Thread synchronization — Lock
class ThreadSafeCounter:
    """Counter that's safe to use from multiple threads."""

    def __init__(self):
        self._count = 0
        self._lock = threading.Lock()

    def increment(self):
        with self._lock:  # Acquire lock, auto-release after block
            self._count += 1

    @property
    def value(self):
        return self._count

counter = ThreadSafeCounter()
threads = []
for _ in range(100):
    t = threading.Thread(target=counter.increment)
    threads.append(t)
    t.start()
for t in threads:
    t.join()
print(f"Counter: {counter.value}")  # Always 100 (thread-safe)


# =============================================================================
# 13.3 MULTIPROCESSING
# =============================================================================
"""
WHAT: Separate Python processes, each with its own GIL.
      TRUE parallelism for CPU-bound work.

WHEN to use multiprocessing:
- CPU-intensive computation (math, image processing, data crunching)
- Need to utilize multiple CPU cores
- Long-running computations that would block the main process

TRADE-OFFS:
- Higher memory usage (each process has its own memory space)
- Inter-process communication is slower (pickle serialization)
- Startup overhead (creating processes is slower than threads)
"""

from multiprocessing import Pool, cpu_count
from concurrent.futures import ProcessPoolExecutor

def cpu_intensive_task(n: int) -> int:
    """Simulate CPU-bound work."""
    total = 0
    for i in range(n):
        total += i * i
    return total

# Method 1: multiprocessing.Pool
# if __name__ == "__main__":  # Required on Windows!
#     with Pool(processes=cpu_count()) as pool:
#         results = pool.map(cpu_intensive_task, [10**6] * 8)
#         print(f"Results: {results[:3]}...")

# Method 2: ProcessPoolExecutor (same API as ThreadPoolExecutor)
# if __name__ == "__main__":
#     with ProcessPoolExecutor(max_workers=cpu_count()) as executor:
#         futures = [executor.submit(cpu_intensive_task, 10**6) for _ in range(8)]
#         for future in as_completed(futures):
#             print(f"  Result: {future.result()}")

print(f"CPU cores available: {cpu_count()}")


# =============================================================================
# 13.4 ASYNCIO — ASYNC/AWAIT
# =============================================================================
"""
WHAT: Single-threaded concurrency using cooperative scheduling.
      Functions yield control voluntarily at `await` points.

WHY asyncio:
- Handles thousands of concurrent I/O operations (network connections)
- No threading overhead (no context switching, no locks needed)
- More predictable than threads (no race conditions for shared state)
- Perfect for servers, web scrapers, API clients

HOW it works:
1. `async def` creates a coroutine function
2. `await` suspends execution until the awaited thing completes
3. Event loop manages all coroutines, running them cooperatively
4. Control returns to event loop at each `await`

WHEN to use asyncio:
- Many concurrent I/O operations (HTTP requests, DB queries)
- WebSocket servers
- Chat applications
- Event-driven architectures

WHEN NOT to use:
- CPU-bound work (use multiprocessing)
- Simple scripts with sequential I/O
- When libraries don't support async
"""

import asyncio

# Basic async function (coroutine)
async def fetch_data(name: str, delay: float) -> str:
    """Simulate async I/O operation."""
    print(f"  Starting {name}...")
    await asyncio.sleep(delay)  # Non-blocking sleep (yields to event loop)
    print(f"  Finished {name}")
    return f"Data from {name}"

# Running multiple coroutines concurrently
async def main():
    """Run multiple I/O operations concurrently."""
    # gather — run all concurrently, return all results
    results = await asyncio.gather(
        fetch_data("API-1", 1.0),
        fetch_data("API-2", 0.5),
        fetch_data("API-3", 0.8),
    )
    print(f"  All results: {results}")
    # Total time ≈ 1.0s (not 2.3s!) — concurrent!

# Run the event loop
# asyncio.run(main())  # Entry point for async code

# === TASKGROUP (Python 3.11+) — Structured Concurrency ===
"""
WHY TaskGroup over gather:
- Better error handling (cancels all tasks if one fails)
- Structured: all tasks complete before exiting the `async with` block
- Clearer ownership of task lifecycle
"""

async def main_taskgroup():
    """Structured concurrency with TaskGroup."""
    async with asyncio.TaskGroup() as tg:
        task1 = tg.create_task(fetch_data("fast", 0.5))
        task2 = tg.create_task(fetch_data("slow", 1.0))
        task3 = tg.create_task(fetch_data("medium", 0.7))

    # All tasks guaranteed complete here
    print(f"Results: {task1.result()}, {task2.result()}, {task3.result()}")

# === ASYNC PATTERNS ===

# Async generator
async def async_range(start, stop, delay=0.1):
    """Async generator — yields values with delays."""
    for i in range(start, stop):
        await asyncio.sleep(delay)
        yield i

async def consume_async_gen():
    async for value in async_range(0, 5):
        print(f"  Got: {value}")

# Async context manager
class AsyncDatabasePool:
    """Async context manager for connection pool."""

    async def __aenter__(self):
        print("  Opening connection pool")
        await asyncio.sleep(0.1)  # Simulate async connect
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        print("  Closing connection pool")
        await asyncio.sleep(0.1)  # Simulate async disconnect

    async def query(self, sql: str):
        await asyncio.sleep(0.1)
        return [{"id": 1, "name": "Alice"}]

# Usage:
# async with AsyncDatabasePool() as pool:
#     results = await pool.query("SELECT * FROM users")

# Semaphore — limit concurrent operations
async def limited_fetch(sem: asyncio.Semaphore, url: str):
    """Fetch with concurrency limit."""
    async with sem:  # Only N concurrent operations
        return await fetch_data(url, 0.5)

async def main_limited():
    sem = asyncio.Semaphore(5)  # Max 5 concurrent fetches
    urls = [f"page-{i}" for i in range(20)]
    tasks = [limited_fetch(sem, url) for url in urls]
    results = await asyncio.gather(*tasks)
    return results


# =============================================================================
# 13.5 SUBPROCESS
# =============================================================================
"""
WHAT: Run external commands/programs from Python.
WHEN: Calling system commands, running other programs, shell scripting.
"""

import subprocess

# Run command and capture output
result = subprocess.run(
    ["echo", "Hello from subprocess"],
    capture_output=True,
    text=True,  # Return str instead of bytes
    check=True,  # Raise CalledProcessError on non-zero exit
)
print(f"Output: {result.stdout.strip()}")
print(f"Return code: {result.returncode}")

# SECURITY: Never use shell=True with user input!
# BAD — shell injection vulnerability:
# subprocess.run(f"echo {user_input}", shell=True)  # DANGEROUS!

# GOOD — use list arguments (no shell interpretation):
# subprocess.run(["echo", user_input])

# Capture stderr separately
result = subprocess.run(
    ["python", "-c", "import sys; print('out'); print('err', file=sys.stderr)"],
    capture_output=True,
    text=True,
)
print(f"stdout: {result.stdout}")
print(f"stderr: {result.stderr}")


# =============================================================================
# 13.6 CHOOSING THE RIGHT CONCURRENCY MODEL
# =============================================================================
"""
DECISION MATRIX:

┌─────────────────────────┬────────────────────────┬────────────────────────┐
│ Situation               │ Best Choice            │ Why                    │
├─────────────────────────┼────────────────────────┼────────────────────────┤
│ Many HTTP requests      │ asyncio + httpx/aiohttp│ Thousands of conns     │
│ File I/O (multiple)     │ ThreadPoolExecutor     │ Simple, GIL released   │
│ CPU math/processing     │ ProcessPoolExecutor    │ True parallelism       │
│ Web server              │ asyncio (uvicorn)      │ High connection count  │
│ GUI + background work   │ threading              │ Keep UI responsive     │
│ Simple parallel tasks   │ concurrent.futures     │ Clean API, both modes  │
│ Data science pipeline   │ multiprocessing/dask   │ CPU parallel + memory  │
│ Websockets              │ asyncio                │ Long-lived connections │
└─────────────────────────┴────────────────────────┴────────────────────────┘

RULES OF THUMB:
1. Start with synchronous code. Add concurrency only when needed.
2. For I/O: asyncio > threading > multiprocessing
3. For CPU: multiprocessing > threading (threading won't help due to GIL)
4. Use concurrent.futures for the simplest API
5. Don't mix asyncio and threading unless you really need to
"""


# =============================================================================
# SUMMARY: CONCURRENCY BEST PRACTICES
# =============================================================================
"""
1. I/O-bound → asyncio or threading; CPU-bound → multiprocessing
2. Use concurrent.futures for simple parallel tasks
3. Use TaskGroup (3.11+) over gather for structured concurrency
4. Always use locks for shared mutable state in threading
5. Prefer asyncio.Semaphore to limit concurrent operations
6. Never use shell=True in subprocess with user input
7. Use `if __name__ == "__main__":` guard with multiprocessing
8. Async functions should never do CPU-heavy work (blocks the event loop)
9. Use asyncio.to_thread() to run blocking code in async context
10. Profile before parallelizing — overhead may exceed benefit for small tasks
"""
