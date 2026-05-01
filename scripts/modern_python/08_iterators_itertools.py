"""
=============================================================================
CHAPTER 8: ITERATORS & ITERTOOLS
=============================================================================
Iterators are the backbone of Python's data processing model.
The itertools module provides composable, memory-efficient building blocks
for working with sequences.

WHY iterators matter:
- Python's for loop, comprehensions, unpacking ALL use the iterator protocol
- Memory efficiency: process one item at a time (no need to load all in memory)
- Composability: chain operations into pipelines
- Infinite sequences: represent unbounded data streams

HOW the iterator protocol works:
1. __iter__() → returns the iterator object
2. __next__() → returns next value or raises StopIteration
3. `for x in obj:` calls iter(obj) then repeatedly calls next()

=============================================================================
"""

# =============================================================================
# 8.1 ITERATOR PROTOCOL
# =============================================================================
"""
WHAT: Any object that implements __iter__ and __next__ is an iterator.
      Any object that implements __iter__ (returning an iterator) is iterable.

KEY DISTINCTION:
- Iterable: has __iter__() → can be used in for loops (list, dict, str, file)
- Iterator: has __iter__() AND __next__() → produces values one at a time

IMPORTANT: Iterators are exhausted after one pass!
           Iterables can create fresh iterators each time.
"""

# Lists are ITERABLE (not iterators themselves)
numbers = [1, 2, 3]
it = iter(numbers)  # Get an iterator from the iterable
print(next(it))  # 1
print(next(it))  # 2
print(next(it))  # 3
# next(it)  # StopIteration!

# What `for` actually does internally:
# for item in iterable:
#     process(item)
#
# Is equivalent to:
# it = iter(iterable)
# while True:
#     try:
#         item = next(it)
#     except StopIteration:
#         break
#     process(item)

# Custom iterator class
class CountUp:
    """Iterator that counts from start to end."""

    def __init__(self, start: int, end: int):
        self.current = start
        self.end = end

    def __iter__(self):
        """Return self — iterators are their own iterables."""
        return self

    def __next__(self):
        """Return next value or raise StopIteration."""
        if self.current >= self.end:
            raise StopIteration
        value = self.current
        self.current += 1
        return value

for num in CountUp(1, 5):
    print(num)  # 1, 2, 3, 4

# Custom ITERABLE (creates fresh iterator each time)
class Sentence:
    """Iterable that can be iterated multiple times."""

    def __init__(self, text: str):
        self.words = text.split()

    def __iter__(self):
        """Return a fresh iterator each time."""
        return iter(self.words)  # Delegate to list's iterator

    def __len__(self):
        return len(self.words)

s = Sentence("Hello World Python")
# Can iterate multiple times (unlike an iterator):
print(list(s))  # ['Hello', 'World', 'Python']
print(list(s))  # ['Hello', 'World', 'Python'] — works again!


# =============================================================================
# 8.2 BUILT-IN ITERATION FUNCTIONS
# =============================================================================
"""
Python provides powerful built-in functions for working with iterables.
These are the workhorses of Pythonic code.
"""

# === enumerate() ===
"""
WHAT: Yields (index, value) pairs.
WHEN: Need both index and value during iteration.
WHY: Replaces manual counter variable.
"""
fruits = ["apple", "banana", "cherry"]
for i, fruit in enumerate(fruits, start=1):
    print(f"{i}. {fruit}")

# === zip() ===
"""
WHAT: Pairs up elements from multiple iterables.
WHEN: Parallel iteration over multiple sequences.
NOTE: Stops at shortest input. Use zip_longest for padding.
"""
names = ["Alice", "Bob", "Charlie"]
scores = [95, 87, 92]
grades = ["A", "B+", "A-"]

for name, score, grade in zip(names, scores, grades):
    print(f"{name}: {score} ({grade})")

# Unzip pattern (transpose):
pairs = [(1, 'a'), (2, 'b'), (3, 'c')]
numbers, letters = zip(*pairs)
print(f"Numbers: {numbers}")  # (1, 2, 3)
print(f"Letters: {letters}")  # ('a', 'b', 'c')

# zip strict mode (Python 3.10+) — error if lengths differ
# for a, b in zip(short_list, long_list, strict=True):
#     pass  # ValueError if lengths differ

# === map() and filter() ===
"""
WHAT: Apply function to each element / filter elements by predicate.
WHEN: Simple transformations (but prefer comprehensions in most cases).
WHY still useful: when you have a pre-existing function to apply.
"""
# map — apply function to each element (lazy)
numbers = [1, 2, 3, 4, 5]
squared = list(map(lambda x: x**2, numbers))

# PYTHONIC PREFERENCE: comprehension is usually clearer
squared = [x**2 for x in numbers]

# map IS better when you have a named function already:
strings = ["1", "2", "3", "4"]
ints = list(map(int, strings))  # Cleaner than [int(s) for s in strings]

# filter — keep elements where predicate is True (lazy)
evens = list(filter(lambda x: x % 2 == 0, numbers))
# Prefer: evens = [x for x in numbers if x % 2 == 0]

# === sorted() with key ===
"""
WHAT: Sort any iterable using a key function.
WHEN: Need sorted output from any iterable (not just lists).
"""
words = ["banana", "Apple", "cherry", "Date"]

# Case-insensitive sort
print(sorted(words, key=str.lower))

# Sort by multiple criteria (tuple key)
students = [("Alice", 90), ("Bob", 85), ("Charlie", 90), ("Diana", 85)]
# Sort by grade descending, then name ascending
result = sorted(students, key=lambda s: (-s[1], s[0]))
print(result)

# === any() and all() ===
"""
WHAT: Test if any/all elements satisfy a condition.
WHEN: Validation, checking properties across a collection.
WHY: Short-circuits (stops early on first True/False).
"""
numbers = [2, 4, 6, 8, 10]
print(f"All even: {all(n % 2 == 0 for n in numbers)}")    # True
print(f"Any > 5: {any(n > 5 for n in numbers)}")          # True
print(f"Any negative: {any(n < 0 for n in numbers)}")     # False

# Practical: validate all required fields present
required = {"name", "email", "age"}
data = {"name": "Alice", "email": "a@b.com", "age": 30}
print(f"All required: {all(field in data for field in required)}")  # True


# =============================================================================
# 8.3 ITERTOOLS — THE SWISS ARMY KNIFE
# =============================================================================
"""
WHAT: itertools provides fast, memory-efficient tools for iterator operations.
      All functions return LAZY iterators (compute on demand).

CATEGORIES:
1. Infinite iterators: count, cycle, repeat
2. Terminating iterators: chain, compress, dropwhile, takewhile, groupby, etc.
3. Combinatoric: product, permutations, combinations
"""

import itertools

# === INFINITE ITERATORS ===

# count(start, step) — infinite counter
counter = itertools.count(10, 2)  # 10, 12, 14, 16, ...
first_5 = [next(counter) for _ in range(5)]
print(f"Count: {first_5}")  # [10, 12, 14, 16, 18]

# cycle(iterable) — repeat infinitely
colors = itertools.cycle(["red", "green", "blue"])
first_7 = [next(colors) for _ in range(7)]
print(f"Cycle: {first_7}")  # ['red', 'green', 'blue', 'red', 'green', 'blue', 'red']

# repeat(value, n) — repeat value n times
fives = list(itertools.repeat(5, 3))
print(f"Repeat: {fives}")  # [5, 5, 5]

# === TERMINATING ITERATORS ===

# chain(*iterables) — concatenate iterables
combined = list(itertools.chain([1, 2], [3, 4], [5, 6]))
print(f"Chain: {combined}")  # [1, 2, 3, 4, 5, 6]

# chain.from_iterable — flatten one level of nesting
nested = [[1, 2], [3, 4], [5, 6]]
flat = list(itertools.chain.from_iterable(nested))
print(f"Flatten: {flat}")  # [1, 2, 3, 4, 5, 6]

# islice(iterable, stop) or islice(iterable, start, stop, step) — lazy slicing
# CRITICAL: works on ANY iterable (generators, files, etc.)
from itertools import islice

def infinite_squares():
    n = 0
    while True:
        yield n * n
        n += 1

first_10_squares = list(islice(infinite_squares(), 10))
print(f"First 10 squares: {first_10_squares}")

# takewhile / dropwhile — take/skip while condition is true
numbers = [1, 3, 5, 7, 2, 4, 6, 8]
taken = list(itertools.takewhile(lambda x: x < 6, numbers))
print(f"Takewhile < 6: {taken}")  # [1, 3, 5]

dropped = list(itertools.dropwhile(lambda x: x < 6, numbers))
print(f"Dropwhile < 6: {dropped}")  # [7, 2, 4, 6, 8]

# groupby(iterable, key) — group consecutive elements
"""
IMPORTANT: groupby only groups CONSECUTIVE elements with the same key!
           Sort first if you want all same-key elements grouped.
"""
data = [("fruit", "apple"), ("fruit", "banana"), ("veggie", "carrot"),
        ("fruit", "date"), ("veggie", "eggplant")]

# Sort first to group properly
data.sort(key=lambda x: x[0])
for key, group in itertools.groupby(data, key=lambda x: x[0]):
    items = [item[1] for item in group]
    print(f"{key}: {items}")
# fruit: ['apple', 'banana', 'date']
# veggie: ['carrot', 'eggplant']

# accumulate — running total (or running application of any function)
numbers = [1, 2, 3, 4, 5]
running_sum = list(itertools.accumulate(numbers))
print(f"Running sum: {running_sum}")  # [1, 3, 6, 10, 15]

import operator
running_product = list(itertools.accumulate(numbers, operator.mul))
print(f"Running product: {running_product}")  # [1, 2, 6, 24, 120]

# Running maximum
running_max = list(itertools.accumulate([3, 1, 4, 1, 5, 9], max))
print(f"Running max: {running_max}")  # [3, 3, 4, 4, 5, 9]

# starmap — like map but unpacks argument tuples
pairs = [(2, 5), (3, 2), (10, 3)]
powers = list(itertools.starmap(pow, pairs))
print(f"Starmap pow: {powers}")  # [32, 9, 1000]

# compress — filter using selector
data = ['a', 'b', 'c', 'd', 'e']
selectors = [True, False, True, False, True]
result = list(itertools.compress(data, selectors))
print(f"Compress: {result}")  # ['a', 'c', 'e']

# === COMBINATORIC ITERATORS ===

# product — Cartesian product (nested loops equivalent)
colors = ["red", "blue"]
sizes = ["S", "M", "L"]
for combo in itertools.product(colors, sizes):
    print(f"  {combo}")
# ('red', 'S'), ('red', 'M'), ('red', 'L'), ('blue', 'S'), ...

# permutations — all orderings
perms = list(itertools.permutations([1, 2, 3], 2))
print(f"Permutations(3,2): {perms}")
# [(1,2), (1,3), (2,1), (2,3), (3,1), (3,2)]

# combinations — all subsets of size r (order doesn't matter)
combos = list(itertools.combinations([1, 2, 3, 4], 2))
print(f"Combinations(4,2): {combos}")
# [(1,2), (1,3), (1,4), (2,3), (2,4), (3,4)]

# combinations_with_replacement — allows repeated elements
combos_r = list(itertools.combinations_with_replacement([1, 2, 3], 2))
print(f"Combinations w/ replacement: {combos_r}")
# [(1,1), (1,2), (1,3), (2,2), (2,3), (3,3)]


# =============================================================================
# 8.4 PRACTICAL PATTERNS WITH ITERTOOLS
# =============================================================================
"""
Real-world recipes combining itertools functions.
"""

# Pattern 1: Pairwise iteration (adjacent pairs)
def pairwise(iterable):
    """Yield consecutive pairs: (s0,s1), (s1,s2), (s2,s3), ..."""
    a, b = itertools.tee(iterable)
    next(b, None)
    return zip(a, b)
# NOTE: Python 3.10+ has itertools.pairwise()

data = [1, 2, 3, 4, 5]
print(f"Pairwise: {list(pairwise(data))}")
# [(1, 2), (2, 3), (3, 4), (4, 5)]

# Pattern 2: Flatten nested iterables
def flatten(iterable):
    """Flatten one level of nesting."""
    return itertools.chain.from_iterable(iterable)

nested = [[1, 2, 3], [4, 5], [6, 7, 8, 9]]
print(f"Flattened: {list(flatten(nested))}")

# Pattern 3: Chunking / Batching
def chunked(iterable, n):
    """Yield chunks of n elements."""
    it = iter(iterable)
    while chunk := list(islice(it, n)):
        yield chunk

data = range(10)
print(f"Chunks of 3: {list(chunked(data, 3))}")
# [[0, 1, 2], [3, 4, 5], [6, 7, 8], [9]]

# Pattern 4: Round-robin from multiple iterables
def roundrobin(*iterables):
    """Visit each iterable in turn, cycling."""
    iterators = [iter(it) for it in iterables]
    while iterators:
        for it in iterators[:]:
            try:
                yield next(it)
            except StopIteration:
                iterators.remove(it)

print(list(roundrobin("ABC", "D", "EF")))
# ['A', 'D', 'E', 'B', 'F', 'C']

# Pattern 5: Unique elements preserving order
def unique_everseen(iterable, key=None):
    """Yield unique elements preserving order."""
    seen = set()
    for element in iterable:
        k = key(element) if key else element
        if k not in seen:
            seen.add(k)
            yield element

data = [1, 5, 2, 1, 9, 1, 5, 10]
print(f"Unique: {list(unique_everseen(data))}")  # [1, 5, 2, 9, 10]

# Case-insensitive unique
words = ["Hello", "hello", "HELLO", "World", "world"]
print(list(unique_everseen(words, key=str.lower)))  # ['Hello', 'World']


# =============================================================================
# SUMMARY: ITERATORS & ITERTOOLS
# =============================================================================
"""
KEY PRINCIPLES:
1. Prefer lazy iteration (generators, itertools) over building lists in memory
2. Use enumerate() instead of range(len(x))
3. Use zip() for parallel iteration
4. Use itertools for complex iteration patterns
5. Remember: iterators are exhausted after one pass (iterables are reusable)

ITERTOOLS CHEAT SHEET:
┌──────────────────────┬─────────────────────────────────────────────────────┐
│ Function             │ Purpose                                             │
├──────────────────────┼─────────────────────────────────────────────────────┤
│ chain()              │ Concatenate iterables                               │
│ islice()             │ Slice any iterable (lazy)                           │
│ groupby()            │ Group consecutive elements by key                   │
│ accumulate()         │ Running reduction (sum, max, etc.)                  │
│ takewhile()          │ Take while condition true                           │
│ dropwhile()          │ Skip while condition true                           │
│ product()            │ Cartesian product (nested loops)                    │
│ permutations()       │ All orderings                                       │
│ combinations()       │ All subsets of size r                               │
│ starmap()            │ map() but unpacks argument tuples                   │
│ compress()           │ Filter with boolean selector                        │
│ count()              │ Infinite counter                                    │
│ cycle()              │ Infinite repetition of iterable                     │
│ tee()                │ Clone an iterator into n independent copies         │
│ pairwise() (3.10+)  │ Adjacent pairs                                      │
│ batched() (3.12+)   │ Group into fixed-size chunks                        │
└──────────────────────┴─────────────────────────────────────────────────────┘
"""
