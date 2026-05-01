"""
=============================================================================
CHAPTER 5: COMPREHENSIONS & GENERATORS
=============================================================================
Comprehensions and generators are Python's most distinctive features.
They enable concise, readable data transformations that are also performant.

WHY comprehensions exist:
- More readable than manual loops for simple transforms
- Faster than equivalent for-loops (optimized C implementation)
- Declarative style: describe WHAT you want, not HOW to build it

WHY generators exist:
- Process infinite/huge sequences without loading everything in memory
- Lazy evaluation — compute values on demand
- Pipeline processing — chain operations without intermediate lists

RULE OF THUMB:
- Simple transform/filter → comprehension
- Side effects or complex logic → regular for loop
- Large/infinite data → generator

=============================================================================
"""

# =============================================================================
# 5.1 LIST COMPREHENSIONS
# =============================================================================
"""
WHAT: [expression for item in iterable if condition]
      Creates a new list from an iterable with optional transformation and filtering.

SYNTAX BREAKDOWN:
    [output_expression for variable in iterable if condition]
    └── what to put ──┘ └── what to loop ──┘ └── filter ──┘

WHY list comprehensions are preferred:
- 2-3x faster than equivalent for-loop (no .append() overhead)
- More readable for simple transforms
- Clear intent: "I'm building a list"

WHEN to use:
- Transforming all elements: [f(x) for x in items]
- Filtering: [x for x in items if pred(x)]
- Flattening: [item for sublist in nested for item in sublist]

WHEN NOT to use:
- Side effects (printing, writing files) — use regular loop
- Complex logic (multiple if/else, state) — use regular loop
- More than 2 levels of nesting — use regular loop or helper function
"""

# Basic transformation
numbers = [1, 2, 3, 4, 5]
squares = [n ** 2 for n in numbers]
print(f"Squares: {squares}")  # [1, 4, 9, 16, 25]

# With filtering
evens = [n for n in numbers if n % 2 == 0]
print(f"Evens: {evens}")  # [2, 4]

# Transform + filter
even_squares = [n ** 2 for n in numbers if n % 2 == 0]
print(f"Even squares: {even_squares}")  # [4, 16]

# Conditional expression in output (NOT the same as filter)
labels = ["even" if n % 2 == 0 else "odd" for n in numbers]
print(f"Labels: {labels}")  # ['odd', 'even', 'odd', 'even', 'odd']

# String processing
words = ["  Hello  ", "  World  ", "  Python  "]
cleaned = [w.strip().lower() for w in words]
print(f"Cleaned: {cleaned}")  # ['hello', 'world', 'python']

# Nested comprehension (flattening)
matrix = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
flat = [num for row in matrix for num in row]
print(f"Flat: {flat}")  # [1, 2, 3, 4, 5, 6, 7, 8, 9]
# Read as: for row in matrix: for num in row: append num

# 2D list creation
grid = [[0] * 5 for _ in range(3)]  # 3 rows, 5 cols
# NOTE: Use _ for throwaway variable (convention for unused loop variable)

# WHEN comprehensions become too complex — STOP and use a loop
# BAD — too nested, hard to read:
# result = [transform(x) for group in data for x in group if valid(x) if special(x)]

# GOOD — break it up:
# result = []
# for group in data:
#     for x in group:
#         if valid(x) and special(x):
#             result.append(transform(x))

# PERFORMANCE COMPARISON
import timeit

# Comprehension is faster due to optimized bytecode
def with_loop():
    result = []
    for i in range(1000):
        result.append(i ** 2)
    return result

def with_comprehension():
    return [i ** 2 for i in range(1000)]

# loop_time = timeit.timeit(with_loop, number=10000)
# comp_time = timeit.timeit(with_comprehension, number=10000)
# Comprehension is typically 30-50% faster


# =============================================================================
# 5.2 DICT COMPREHENSIONS
# =============================================================================
"""
WHAT: {key_expr: value_expr for item in iterable if condition}
      Creates a new dictionary from an iterable.

WHEN to use:
- Building dicts from sequences
- Inverting dictionaries
- Filtering dictionaries
- Transforming dict values
"""

# Basic dict comprehension
squares_dict = {n: n**2 for n in range(6)}
print(f"Squares dict: {squares_dict}")  # {0: 0, 1: 1, 2: 4, ...}

# From two lists (zip pattern)
keys = ["name", "age", "city"]
values = ["Alice", 30, "NYC"]
person = {k: v for k, v in zip(keys, values)}
print(f"Person: {person}")

# Inverting a dictionary
original = {"a": 1, "b": 2, "c": 3}
inverted = {v: k for k, v in original.items()}
print(f"Inverted: {inverted}")  # {1: 'a', 2: 'b', 3: 'c'}

# Filtering a dictionary
config = {"debug": True, "verbose": False, "port": 8080, "host": ""}
# Keep only truthy values
active_config = {k: v for k, v in config.items() if v}
print(f"Active: {active_config}")  # {'debug': True, 'port': 8080}

# Transform values
prices = {"apple": 1.5, "banana": 0.75, "cherry": 3.0}
discounted = {item: price * 0.9 for item, price in prices.items()}
print(f"Discounted: {discounted}")

# PYTHONIC: dict.fromkeys() for uniform initialization
alphabet = dict.fromkeys("abcde", 0)
print(f"Alphabet: {alphabet}")  # {'a': 0, 'b': 0, ...}


# =============================================================================
# 5.3 SET COMPREHENSIONS
# =============================================================================
"""
WHAT: {expression for item in iterable if condition}
      Creates a new set (automatic deduplication).

WHEN to use:
- Collecting unique transformed values
- Building lookup sets from data
"""

# Unique word lengths
words = ["hello", "world", "python", "code", "fun"]
lengths = {len(w) for w in words}
print(f"Unique lengths: {lengths}")  # {3, 4, 5, 6}

# Unique first characters
first_chars = {w[0].upper() for w in words}
print(f"First chars: {first_chars}")


# =============================================================================
# 5.4 GENERATOR EXPRESSIONS
# =============================================================================
"""
WHAT: (expression for item in iterable if condition)
      Like list comprehension but produces values LAZILY (one at a time).

WHY generators are crucial:
- Memory efficient: never stores all values at once
- Can handle infinite sequences
- Enables pipeline processing
- Perfect for large files and streams

HOW they differ from list comprehensions:
- Use () instead of []
- Return a generator object (iterator), not a list
- Values computed on demand (lazy)
- Can only be iterated ONCE

WHEN to use:
- Processing large datasets (millions of rows)
- When you only need to iterate once
- As arguments to functions that consume iterables (sum, max, any, all)
- Pipeline transformations on streams
"""

# Generator expression — lazy evaluation
gen = (n ** 2 for n in range(1000000))
print(f"Type: {type(gen)}")  # <class 'generator'>
print(f"Size: {gen.__sizeof__()} bytes")  # ~100 bytes regardless of size!

# Compare memory usage:
import sys
list_comp = [n ** 2 for n in range(1000000)]
print(f"List size: {sys.getsizeof(list_comp)} bytes")  # ~8 MB!

# PYTHONIC: Use generator expression directly in function calls
# No need for extra parentheses when it's the only argument
total = sum(n ** 2 for n in range(1000000))  # memory efficient!
print(f"Sum of squares: {total}")

# Check if any/all elements satisfy condition
numbers = [2, 4, 6, 8, 10]
all_even = all(n % 2 == 0 for n in numbers)
any_big = any(n > 7 for n in numbers)
print(f"All even: {all_even}")  # True
print(f"Any > 7: {any_big}")    # True

# BEST PRACTICE: generator in function calls (no intermediate list)
# BAD — creates unnecessary list in memory:
# max([len(line) for line in open("big_file.txt")])
# GOOD — processes lazily:
# max(len(line) for line in open("big_file.txt"))


# =============================================================================
# 5.5 GENERATOR FUNCTIONS (yield)
# =============================================================================
"""
WHAT: Functions containing `yield` become generator functions.
      When called, they return a generator object (not the result).
      Each `yield` produces a value and suspends execution until next() is called.

WHY generator functions exist:
- Handle complex iteration logic that comprehensions can't express
- Maintain state between yields (local variables preserved)
- Can be infinite (generate forever)
- Memory constant regardless of how many values produced

HOW generators work:
1. Calling a generator function returns a generator object
2. Calling next(gen) executes until the next yield
3. The yielded value is returned by next()
4. Function state is frozen until next next() call
5. When function returns (or reaches end), StopIteration is raised

WHEN to use generator functions:
- Complex iteration logic (state machines, tree traversal)
- Infinite sequences
- Lazy file/stream processing
- When you need to yield from multiple places (vs single expression)
"""

# Basic generator function
def countdown(n):
    """Yield numbers from n down to 1."""
    print(f"Starting countdown from {n}")
    while n > 0:
        yield n  # Suspend here, resume on next()
        n -= 1
    print("Liftoff!")

# Usage
for num in countdown(5):
    print(num)  # 5, 4, 3, 2, 1

# Manual iteration to see the mechanics
gen = countdown(3)
print(next(gen))  # "Starting countdown from 3" then 3
print(next(gen))  # 2
print(next(gen))  # 1
# next(gen) would raise StopIteration and print "Liftoff!"

# Infinite generator
def fibonacci():
    """Generate infinite Fibonacci sequence."""
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b

# Take first 10 Fibonacci numbers
from itertools import islice
fib_10 = list(islice(fibonacci(), 10))
print(f"First 10 Fibonacci: {fib_10}")  # [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]

# Generator for file processing (memory efficient)
def read_large_file(filepath, chunk_size=1024):
    """Read a large file in chunks without loading entirely into memory."""
    with open(filepath, 'r') as f:
        while True:
            chunk = f.read(chunk_size)
            if not chunk:
                break
            yield chunk

# Pipeline pattern — chain generators
def read_lines(filepath):
    """Yield lines from a file."""
    with open(filepath) as f:
        for line in f:
            yield line.strip()

def filter_comments(lines):
    """Skip lines starting with #."""
    for line in lines:
        if not line.startswith("#"):
            yield line

def parse_csv_line(lines):
    """Split CSV lines into fields."""
    for line in lines:
        yield line.split(",")

# Usage (each step is lazy — processes one line at a time):
# pipeline = parse_csv_line(filter_comments(read_lines("data.csv")))
# for row in pipeline:
#     process(row)


# =============================================================================
# 5.6 YIELD FROM
# =============================================================================
"""
WHAT: `yield from` delegates to a sub-generator or iterable.
      It yields each item from the sub-iterator.

WHY yield from exists:
- Simplifies recursive generators (tree traversal)
- Flattens nested iteration
- Enables generator delegation patterns (coroutines)

WHEN to use:
- Recursively traversing nested structures
- Combining multiple generators
- Delegating to sub-generators
"""

# Without yield from — verbose
def chain_manual(*iterables):
    for iterable in iterables:
        for item in iterable:
            yield item

# With yield from — elegant
def chain_elegant(*iterables):
    for iterable in iterables:
        yield from iterable  # delegates to each iterable

result = list(chain_elegant([1, 2], [3, 4], [5, 6]))
print(f"Chained: {result}")  # [1, 2, 3, 4, 5, 6]

# Recursive generator — flatten nested lists
def flatten(nested):
    """Recursively flatten a nested list structure."""
    for item in nested:
        if isinstance(item, (list, tuple)):
            yield from flatten(item)  # recursive delegation
        else:
            yield item

deeply_nested = [1, [2, [3, [4, 5]], 6], [7, 8]]
print(list(flatten(deeply_nested)))  # [1, 2, 3, 4, 5, 6, 7, 8]

# Tree traversal with yield from
def inorder(tree):
    """In-order traversal of a binary tree using yield from."""
    if tree is not None:
        yield from inorder(tree.get("left"))
        yield tree["val"]
        yield from inorder(tree.get("right"))

tree = {
    "val": 4,
    "left": {"val": 2, "left": {"val": 1, "left": None, "right": None},
             "right": {"val": 3, "left": None, "right": None}},
    "right": {"val": 6, "left": {"val": 5, "left": None, "right": None},
              "right": {"val": 7, "left": None, "right": None}}
}
print(f"Inorder: {list(inorder(tree))}")  # [1, 2, 3, 4, 5, 6, 7]


# =============================================================================
# 5.7 GENERATOR PATTERNS AND IDIOMS
# =============================================================================
"""
Advanced generator patterns used in real-world Python code.
"""

# Pattern 1: Sliding window
def sliding_window(iterable, n):
    """Generate overlapping windows of size n."""
    from collections import deque
    it = iter(iterable)
    window = deque(maxlen=n)
    for _ in range(n):
        window.append(next(it))
    yield tuple(window)
    for item in it:
        window.append(item)
        yield tuple(window)

print(list(sliding_window(range(7), 3)))
# [(0, 1, 2), (1, 2, 3), (2, 3, 4), (3, 4, 5), (4, 5, 6)]

# Pattern 2: Batching
def batched(iterable, n):
    """Yield successive n-sized chunks from iterable."""
    from itertools import islice
    it = iter(iterable)
    while True:
        batch = list(islice(it, n))
        if not batch:
            break
        yield batch

print(list(batched(range(10), 3)))
# [[0, 1, 2], [3, 4, 5], [6, 7, 8], [9]]
# NOTE: Python 3.12+ has itertools.batched()

# Pattern 3: Generator as coroutine (send values in)
def running_average():
    """Compute running average, receiving values via send()."""
    total = 0
    count = 0
    average = None
    while True:
        value = yield average  # Receive value, yield current average
        total += value
        count += 1
        average = total / count

avg = running_average()
next(avg)  # Prime the generator (advance to first yield)
print(avg.send(10))  # 10.0
print(avg.send(20))  # 15.0
print(avg.send(30))  # 20.0

# Pattern 4: Generator for state machine
def tokenizer(text):
    """Simple tokenizer as a generator state machine."""
    token = ""
    for char in text:
        if char.isspace():
            if token:
                yield token
                token = ""
        elif char in ".,;:!?":
            if token:
                yield token
                token = ""
            yield char
        else:
            token += char
    if token:
        yield token

tokens = list(tokenizer("Hello, World! How are you?"))
print(f"Tokens: {tokens}")


# =============================================================================
# SUMMARY: COMPREHENSIONS & GENERATORS
# =============================================================================
"""
DECISION GUIDE:
┌─────────────────────────────────┬─────────────────────────────────────────┐
│ Situation                       │ Use                                     │
├─────────────────────────────────┼─────────────────────────────────────────┤
│ Transform list → new list       │ List comprehension                      │
│ Build dict from iterable        │ Dict comprehension                      │
│ Collect unique values           │ Set comprehension                       │
│ Large data / iterate once       │ Generator expression                    │
│ Complex iteration logic         │ Generator function (yield)              │
│ Recursive iteration             │ yield from                              │
│ Side effects needed             │ Regular for loop                        │
│ Multiple operations in sequence │ Chain generators (pipeline)             │
└─────────────────────────────────┴─────────────────────────────────────────┘

PERFORMANCE RULES:
- Comprehensions are ~30-50% faster than equivalent for+append loops
- Generator expressions use O(1) memory regardless of input size
- Use `sum(gen)`, `max(gen)`, `any(gen)` instead of building intermediate lists
- For "first match" use next(gen, default) instead of building full list

KEY PRINCIPLES:
1. If it fits in one line of comprehension → use comprehension
2. If you only iterate once → use generator
3. If you need all values in memory → use list comprehension
4. If logic is complex (>2 conditions, state) → use regular loop or generator function
5. Never use comprehension for side effects (printing, writing, appending to external list)
"""
