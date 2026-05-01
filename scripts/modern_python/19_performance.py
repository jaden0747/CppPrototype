"""
=============================================================================
CHAPTER 19: PERFORMANCE OPTIMIZATION
=============================================================================
Python is NOT the fastest language — but it's fast ENOUGH for most tasks.
When it isn't, Python has excellent tools to identify and fix bottlenecks.

GOLDEN RULE: "Premature optimization is the root of all evil." — Knuth
             First make it work, then make it right, then make it fast.

OPTIMIZATION WORKFLOW:
1. Write correct, readable code first
2. Measure (profile) to find the actual bottleneck
3. Optimize only the bottleneck
4. Measure again to verify improvement
5. Repeat if needed

=============================================================================
"""

# =============================================================================
# 19.1 PROFILING — FINDING BOTTLENECKS
# =============================================================================
"""
WHAT: Profiling measures WHERE your code spends time.
      NEVER optimize without profiling first — you'll guess wrong.

TOOLS:
- time.perf_counter(): simple timing
- timeit: micro-benchmarking
- cProfile: function-level profiling
- line_profiler: line-by-line profiling
- memory_profiler: memory usage profiling
- py-spy: sampling profiler (low overhead, production-safe)
"""

import time
import timeit

# === Simple timing ===
start = time.perf_counter()
result = sum(range(1_000_000))
elapsed = time.perf_counter() - start
print(f"Elapsed: {elapsed:.4f}s")

# === Context manager timer ===
from contextlib import contextmanager

@contextmanager
def timer(label=""):
    start = time.perf_counter()
    yield
    elapsed = time.perf_counter() - start
    print(f"{label}: {elapsed:.4f}s")

# Usage:
with timer("sum"):
    total = sum(range(10_000_000))

# === timeit — micro-benchmarking ===
# Use for comparing small code snippets
t1 = timeit.timeit("sum(range(1000))", number=10000)
t2 = timeit.timeit(
    "[x**2 for x in range(100)]",
    number=10000
)
print(f"sum: {t1:.4f}s, comprehension: {t2:.4f}s")

# === cProfile — function-level profiler ===
"""
$ python -m cProfile -s cumulative my_script.py

Output columns:
- ncalls: number of calls
- tottime: total time IN this function (excluding sub-calls)
- cumtime: cumulative time (including sub-calls)
- percall: per-call time

In code:
"""
import cProfile
import pstats

def slow_function():
    total = 0
    for i in range(1000000):
        total += i ** 2
    return total

# Profile and print stats:
# cProfile.run("slow_function()", sort="cumulative")

# More control:
# profiler = cProfile.Profile()
# profiler.enable()
# slow_function()
# profiler.disable()
# stats = pstats.Stats(profiler).sort_stats("cumulative")
# stats.print_stats(10)  # Top 10 functions

# === line_profiler (pip install line_profiler) ===
"""
Shows time spent on EACH LINE:

$ kernprof -l -v my_script.py

Decorate functions to profile:
    @profile
    def my_function():
        ...
"""

# === py-spy (pip install py-spy) — Sampling profiler ===
"""
WHY: Low overhead, can attach to running process, produces flame graphs.

$ py-spy top --pid 12345           # Live view
$ py-spy record -o profile.svg -- python my_script.py  # Flame graph
"""


# =============================================================================
# 19.2 ALGORITHM AND DATA STRUCTURE CHOICES
# =============================================================================
"""
THE BIGGEST PERFORMANCE WINS come from choosing the right algorithm
and data structure — NOT from micro-optimizations.

TIME COMPLEXITY MATTERS:
- O(1): dict/set lookup, list append
- O(log n): bisect, sorted container operations
- O(n): list search, iteration
- O(n log n): sorting
- O(n²): nested loops (avoid for large n!)
"""

import random

data = list(range(100_000))
random.shuffle(data)

# === USE SETS FOR MEMBERSHIP TESTING ===
# BAD O(n) — list search:
large_list = list(range(100_000))
# 99999 in large_list  # Scans entire list!

# GOOD O(1) — set lookup:
large_set = set(range(100_000))
# 99999 in large_set  # Hash table lookup!

# === USE DICTS FOR KEY-VALUE LOOKUPS ===
# BAD O(n) — linear search:
users_list = [{"id": i, "name": f"user_{i}"} for i in range(10000)]
# next(u for u in users_list if u["id"] == 9999)

# GOOD O(1) — dict access:
users_dict = {i: f"user_{i}" for i in range(10000)}
# users_dict[9999]

# === USE DEQUE FOR QUEUE OPERATIONS ===
from collections import deque

# BAD O(n) — list.pop(0) shifts all elements:
# queue = []
# queue.append(item)     # O(1)
# queue.pop(0)           # O(n)!

# GOOD O(1) — deque.popleft():
queue = deque()
queue.append("item")     # O(1)
queue.popleft()          # O(1)!

# === USE BISECT FOR SORTED OPERATIONS ===
import bisect

sorted_list = sorted(random.sample(range(1_000_000), 100_000))

# BAD O(n) — find insertion point manually
# GOOD O(log n):
pos = bisect.bisect_left(sorted_list, 500_000)


# =============================================================================
# 19.3 BUILT-IN OPTIMIZATIONS
# =============================================================================
"""
Python's built-ins are implemented in C — they're MUCH faster
than equivalent Python code. USE THEM.
"""

numbers = list(range(1_000_000))

# === USE BUILT-IN FUNCTIONS ===
# BAD (pure Python):
total = 0
for n in numbers:
    total += n

# GOOD (C implementation):
total = sum(numbers)  # 10-50x faster

# Same pattern:
maximum = max(numbers)           # Not manual loop
minimum = min(numbers)
exists = any(x > 999_000 for x in numbers)  # Short-circuits!
all_pos = all(x >= 0 for x in numbers)      # Short-circuits!

# === SORTING ===
# Python's sort (Timsort) is highly optimized
words = ["banana", "apple", "cherry"]
sorted_words = sorted(words)                # Returns new list
words.sort()                                 # In-place

# Use key= for custom sorting (not cmp):
words.sort(key=str.lower)
words.sort(key=len)

# operator.itemgetter is faster than lambda for sorting dicts:
from operator import itemgetter
users = [{"name": "Alice", "age": 30}, {"name": "Bob", "age": 25}]
users.sort(key=itemgetter("age"))  # Faster than key=lambda u: u["age"]

# === STRING OPERATIONS ===
# Use join (not +=)
parts = ["Hello"] * 10000
# BAD O(n²):
result = ""
for p in parts:
    result += p
# GOOD O(n):
result = "".join(parts)

# === DICT OPERATIONS ===
# dict.fromkeys() for initialization
keys = range(1000)
d = dict.fromkeys(keys, 0)  # Faster than {k: 0 for k in keys}


# =============================================================================
# 19.4 CACHING AND MEMOIZATION
# =============================================================================
"""
WHAT: Store results of expensive computations for reuse.
WHY: Trade memory for speed when same computation repeats.
"""

from functools import lru_cache, cache

# @cache (Python 3.9+) — unbounded cache
@cache
def fibonacci(n):
    """Without cache: O(2^n). With cache: O(n)."""
    if n < 2:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

print(fibonacci(100))  # Instant (without cache: heat death of universe)

# @lru_cache — bounded cache (evicts least recently used)
@lru_cache(maxsize=256)
def expensive_computation(x, y):
    """Cache up to 256 results."""
    import time
    time.sleep(0.01)  # Simulate expensive work
    return x ** y

# Cache info:
# expensive_computation.cache_info()
# CacheInfo(hits=10, misses=5, maxsize=256, currsize=5)

# Clear cache:
# expensive_computation.cache_clear()

# === MANUAL CACHING ===
# For more control, use a dict:
_cache = {}
def compute_with_cache(key):
    if key not in _cache:
        _cache[key] = expensive_operation(key)
    return _cache[key]

def expensive_operation(key):
    return key ** 2


# =============================================================================
# 19.5 MEMORY OPTIMIZATION
# =============================================================================
"""
WHAT: Reduce memory usage for data-intensive applications.

TOOLS:
- sys.getsizeof(): size of single object
- tracemalloc: track memory allocations
- memory_profiler: line-by-line memory usage
- objgraph: visualize object references
"""

import sys

# Object sizes
print(f"int: {sys.getsizeof(0)} bytes")
print(f"float: {sys.getsizeof(0.0)} bytes")
print(f"str '': {sys.getsizeof('')} bytes")
print(f"list []: {sys.getsizeof([])} bytes")
print(f"dict {{}}: {sys.getsizeof({})} bytes")

# === __slots__ — reduce memory for many instances ===
class PointRegular:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class PointSlots:
    __slots__ = ('x', 'y')
    def __init__(self, x, y):
        self.x = x
        self.y = y

# PointSlots uses ~40% less memory per instance
# (no __dict__ per instance)

# === GENERATORS OVER LISTS ===
# BAD — stores all in memory:
# big_list = [x ** 2 for x in range(10_000_000)]  # ~80MB

# GOOD — generates on demand:
# big_gen = (x ** 2 for x in range(10_000_000))   # ~0MB

# === TRACEMALLOC — TRACK MEMORY ===
import tracemalloc

tracemalloc.start()
# ... code to measure ...
data = [i ** 2 for i in range(100_000)]
current, peak = tracemalloc.get_traced_memory()
tracemalloc.stop()
print(f"Current: {current / 1024:.1f}KB, Peak: {peak / 1024:.1f}KB")

# === ARRAY FOR NUMERIC DATA ===
from array import array

# list of ints: each int is ~28 bytes (Python object)
# array of ints: each int is 4-8 bytes (raw C data)
int_list = list(range(1000))                # ~8KB
int_array = array('i', range(1000))         # ~4KB (half!)


# =============================================================================
# 19.6 NUMPY FOR NUMERICAL PERFORMANCE
# =============================================================================
"""
WHAT: NumPy provides C-speed array operations in Python.
      100-1000x faster than pure Python for numerical work.

WHY: NumPy operations are:
- Vectorized (C loops, not Python loops)
- Contiguous memory (cache-friendly)
- BLAS/LAPACK for linear algebra
"""

# PURE PYTHON (slow):
def dot_product_python(a, b):
    return sum(x * y for x, y in zip(a, b))

# NUMPY (fast):
# import numpy as np
# a = np.array([1.0, 2.0, 3.0])
# b = np.array([4.0, 5.0, 6.0])
# result = np.dot(a, b)  # 100x+ faster for large arrays

# Key principles:
# 1. Avoid Python loops over numpy arrays
# 2. Use vectorized operations (a + b, a * b, np.sin(a))
# 3. Use broadcasting for dimension mismatches
# 4. Use views (slices) to avoid copies


# =============================================================================
# 19.7 CONCURRENCY FOR I/O-BOUND TASKS
# =============================================================================
"""
WHAT: Use asyncio/threading for I/O-bound tasks (network, disk).
      Use multiprocessing for CPU-bound tasks.

See Chapter 13 for details. Key performance insight:
- I/O-bound: asyncio or threading (10-100x speedup for many requests)
- CPU-bound: multiprocessing (Nx speedup for N cores)
"""

import asyncio

async def fetch_urls_fast(urls):
    """Concurrent I/O is the #1 performance win for web apps."""
    async with asyncio.TaskGroup() as tg:
        tasks = [tg.create_task(fetch_one(url)) for url in urls]
    return [t.result() for t in tasks]

async def fetch_one(url):
    await asyncio.sleep(0.01)  # Simulate network I/O
    return f"Response from {url}"


# =============================================================================
# 19.8 C EXTENSIONS AND ALTERNATIVES
# =============================================================================
"""
WHEN Python isn't fast enough (rare), you have options:

1. Cython — Write Python-like code, compile to C
   - 10-100x speedup for numerical code
   - Easy to integrate with existing Python

2. ctypes/cffi — Call C libraries from Python
   - Use existing C code
   - No compilation step for ctypes

3. PyO3/maturin — Write Python extensions in Rust
   - Memory-safe, fast
   - Growing ecosystem (pydantic, ruff, uv are Rust)

4. Numba — JIT compiler for NumPy code
   - Add @jit decorator → C-speed
   - Great for loops over arrays
   - No code changes needed

5. PyPy — Alternative Python interpreter with JIT
   - Drop-in replacement for CPython
   - 5-10x faster for many workloads
   - Limited C extension support
"""

# Numba example:
# from numba import jit
#
# @jit(nopython=True)
# def sum_squares(n):
#     total = 0
#     for i in range(n):
#         total += i ** 2
#     return total
#
# sum_squares(10_000_000)  # First call compiles, subsequent calls are fast


# =============================================================================
# 19.9 COMMON PERFORMANCE ANTI-PATTERNS
# =============================================================================
"""
AVOID THESE:
"""

# 1. String concatenation in loops (O(n²))
# BAD:
result = ""
for i in range(10000):
    result += str(i)  # Creates new string each iteration!
# GOOD:
result = "".join(str(i) for i in range(10000))

# 2. Calling len() in loop condition
# BAD (recalculates each iteration in some languages, but Python caches):
# for i in range(len(items)):  # Use enumerate instead
# GOOD:
# for i, item in enumerate(items):

# 3. Creating objects in tight loops
# BAD:
# for line in file:
#     pattern = re.compile(r'\d+')  # Recompiled every iteration!
#     match = pattern.search(line)
# GOOD:
import re
PATTERN = re.compile(r'\d+')  # Compile once
# for line in file:
#     match = PATTERN.search(line)

# 4. Global variable access in hot loops
# BAD (global lookup each iteration):
# for x in data:
#     result = math.sqrt(x)
# GOOD (local reference):
sqrt = __import__('math').sqrt
# for x in data:
#     result = sqrt(x)

# 5. Not using proper data structures
# BAD: checking membership in list O(n)
# GOOD: checking membership in set O(1)

# 6. Unnecessary copying
# BAD:
items = list(range(1000))
# copy = items[:]  # Full copy just to iterate
# GOOD:
# Iterate the original if you don't modify it


# =============================================================================
# 19.10 PERFORMANCE CHECKLIST
# =============================================================================
"""
OPTIMIZATION PRIORITY (most impact first):

1. ALGORITHM: Choose O(n log n) over O(n²)
2. DATA STRUCTURES: dict/set for lookups, deque for queues
3. BUILT-INS: Use sum(), min(), max(), sorted(), any(), all()
4. VECTORIZE: NumPy for numerical work (avoid Python loops)
5. CACHE: @lru_cache for repeated computations
6. CONCURRENCY: asyncio for I/O, multiprocessing for CPU
7. GENERATORS: For large data that's only iterated once
8. C EXTENSIONS: Cython/Rust/Numba for critical hot paths

MEASUREMENT TOOLS:
- timeit: micro-benchmarks
- cProfile: function-level bottlenecks
- line_profiler: line-level within a function
- tracemalloc: memory tracking
- py-spy: production profiling

REMEMBER:
- Profile FIRST, optimize SECOND
- 80% of time is in 20% of code (find that 20%)
- Readability > speed (unless profile says otherwise)
- Python's strength is development speed, not runtime speed
"""
