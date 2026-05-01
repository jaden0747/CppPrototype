"""
=============================================================================
CHAPTER 2: DATA STRUCTURES
=============================================================================
Python's built-in data structures are powerful, well-optimized, and cover most
use cases without needing external libraries.

WHY master data structures?
- Choosing the right structure makes code 10-100x faster
- Python's built-ins are implemented in C — hard to beat with custom code
- Understanding time complexity prevents hidden performance bugs

RULE OF THUMB for choosing:
- Need ordered, mutable sequence? → list
- Need immutable sequence (hashable)? → tuple
- Need fast lookup by key? → dict
- Need unique elements / set operations? → set
- Need FIFO/LIFO? → deque
- Need sorted access? → heapq or sortedcontainers

=============================================================================
"""

# =============================================================================
# 2.1 LISTS
# =============================================================================
"""
WHAT: Ordered, mutable, heterogeneous sequences (dynamic arrays internally).
      Lists are the workhorse collection in Python.

TIME COMPLEXITY:
- Index access: O(1)
- Append: O(1) amortized
- Insert at beginning: O(n) — avoid!
- Search (in): O(n)
- Sort: O(n log n)

WHY lists are so common:
- Flexible (any type of element)
- Great cache locality (contiguous memory for pointers)
- Rich built-in methods

WHEN to use list vs other structures:
- Use list when: you need ordered items, indexed access, or iteration
- DON'T use list when: you need fast membership testing (use set)
- DON'T use list when: you need FIFO queue (use deque)
"""

# Creation
numbers = [1, 2, 3, 4, 5]
mixed = [1, "hello", 3.14, None, [1, 2]]  # heterogeneous (but avoid in practice)
empty = []
from_range = list(range(10))  # [0, 1, 2, ..., 9]

# PYTHONIC: List comprehension (covered in chapter 5)
squares = [x ** 2 for x in range(10)]

# === SLICING ===
"""
Slicing is one of Python's most elegant features.
Syntax: list[start:stop:step]
- start: inclusive (default 0)
- stop: exclusive (default len)
- step: direction and skip (default 1)
"""
nums = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

print(nums[2:5])      # [2, 3, 4] — from index 2 up to (not including) 5
print(nums[:3])       # [0, 1, 2] — first 3
print(nums[-3:])      # [7, 8, 9] — last 3
print(nums[::2])      # [0, 2, 4, 6, 8] — every other element
print(nums[::-1])     # [9, 8, 7, ...] — reversed (creates new list)
print(nums[1::2])     # [1, 3, 5, 7, 9] — odd-indexed elements

# Slice assignment — modify portions in-place
nums[2:5] = [20, 30, 40]  # Replace elements
nums[2:5] = [20, 30]      # Can change length!

# PYTHONIC: Use slicing to copy
copy = nums[:]  # shallow copy (same as nums.copy() or list(nums))

# === SORTING ===
"""
Two approaches:
- sorted(iterable) → returns NEW sorted list (original unchanged)
- list.sort() → sorts IN-PLACE, returns None

BEST PRACTICE: Use sorted() when you need the original preserved.
               Use .sort() when you want to modify in-place (saves memory).
"""
words = ["banana", "apple", "cherry", "date"]

# sorted() — returns new list
alphabetical = sorted(words)
print(f"Original: {words}")        # unchanged
print(f"Sorted: {alphabetical}")

# .sort() — in-place
words.sort()
print(f"After sort(): {words}")  # modified!

# Custom sorting with key function
data = ["hello", "hi", "hey", "howdy"]
by_length = sorted(data, key=len)
print(f"By length: {by_length}")  # ['hi', 'hey', 'hello', 'howdy']

# Sort by multiple criteria using tuple key
students = [("Alice", 85), ("Bob", 92), ("Charlie", 85)]
# Sort by grade descending, then name ascending
sorted_students = sorted(students, key=lambda s: (-s[1], s[0]))
print(f"Sorted students: {sorted_students}")

# PYTHONIC: Use operator.itemgetter for performance
from operator import itemgetter
sorted_students = sorted(students, key=itemgetter(1), reverse=True)

# === COMMON PATTERNS ===

# Remove duplicates while preserving order
items = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3]
unique = list(dict.fromkeys(items))  # PYTHONIC — preserves order
print(f"Unique (ordered): {unique}")  # [3, 1, 4, 5, 9, 2, 6]

# Flatten a list of lists
nested = [[1, 2], [3, 4], [5, 6]]
flat = [item for sublist in nested for item in sublist]
print(f"Flattened: {flat}")  # [1, 2, 3, 4, 5, 6]

# IMPORTANT: list * creates shallow copies of references!
# BAD — creates 3 references to the SAME inner list:
grid_bad = [[0] * 3] * 3
grid_bad[0][0] = 1
print(f"BAD grid: {grid_bad}")  # [[1, 0, 0], [1, 0, 0], [1, 0, 0]]

# GOOD — creates 3 independent inner lists:
grid_good = [[0] * 3 for _ in range(3)]
grid_good[0][0] = 1
print(f"GOOD grid: {grid_good}")  # [[1, 0, 0], [0, 0, 0], [0, 0, 0]]


# =============================================================================
# 2.2 TUPLES AND NAMED TUPLES
# =============================================================================
"""
WHAT: Immutable ordered sequences. Used for fixed collections of items.

WHY tuples exist (not just "immutable lists"):
- Semantic: represent a RECORD (fixed structure) vs list (homogeneous collection)
- Hashable: can be dict keys and set members
- Performance: slightly faster than lists, less memory
- Safety: guarantees data won't be accidentally modified

WHEN to use tuples:
- Function return values (multiple returns)
- Dictionary keys (coordinates, compound keys)
- Unpacking assignments
- When you want to signal "this data is fixed"

PYTHONIC RULE:
- Lists: homogeneous, variable length (list of users)
- Tuples: heterogeneous, fixed length (a single record: name, age, email)
"""

# Basic tuples
point = (3, 4)
rgb = (255, 128, 0)
single = (42,)  # NOTE: comma required for single-element tuple!
not_a_tuple = (42)  # This is just int 42 with parentheses

# Tuple unpacking — extremely pythonic
x, y = point
print(f"x={x}, y={y}")

# Swap without temp variable — Python magic
a, b = 1, 2
a, b = b, a  # swap! This works because right side is evaluated first as tuple
print(f"a={a}, b={b}")  # a=2, b=1

# Named tuples — tuples with named fields
from collections import namedtuple

# WHY: Gives meaning to positional data, more readable than plain tuples
Point = namedtuple("Point", ["x", "y"])
Color = namedtuple("Color", "red green blue")  # string syntax also works

p = Point(3, 4)
print(f"Point: {p}")          # Point(x=3, y=4)
print(f"x={p.x}, y={p.y}")   # access by name
print(f"x={p[0]}, y={p[1]}") # still works as tuple

# Named tuple with defaults (3.6.1+)
Server = namedtuple("Server", ["host", "port", "protocol"], defaults=["https"])
s = Server("example.com", 8080)
print(f"Server: {s}")  # Server(host='example.com', port=8080, protocol='https')

# MODERN ALTERNATIVE: typing.NamedTuple (preferred — supports type hints)
from typing import NamedTuple

class Employee(NamedTuple):
    name: str
    department: str
    salary: float = 50000.0  # default value

emp = Employee("Alice", "Engineering", 95000.0)
print(f"Employee: {emp}")
print(f"Name: {emp.name}")

# NOTE: For mutable records, use @dataclass instead (see chapter 6)


# =============================================================================
# 2.3 DICTIONARIES
# =============================================================================
"""
WHAT: Hash tables mapping keys to values. The most important data structure
      in Python — used everywhere internally (namespaces, object attributes, etc.)

TIME COMPLEXITY:
- Get/Set/Delete: O(1) average
- Search: O(1) average (check `in`)
- Iteration: O(n)

KEY REQUIREMENTS: Keys must be hashable (immutable: str, int, tuple, frozenset)

WHY dicts are central to Python:
- Variable lookup is dict lookup (locals(), globals())
- Object attributes are stored in __dict__
- Module namespaces are dicts
- kwargs are dicts

SINCE PYTHON 3.7: Dicts maintain insertion order (guaranteed by spec)
"""

# Creation
config = {"host": "localhost", "port": 8080, "debug": True}
from_pairs = dict([("a", 1), ("b", 2)])
from_kwargs = dict(host="localhost", port=8080)  # keys must be valid identifiers

# === ACCESSING VALUES ===
# Use [] when key MUST exist (raises KeyError if missing)
host = config["host"]

# Use .get() when key might not exist (returns None or default)
timeout = config.get("timeout")        # None (no KeyError)
timeout = config.get("timeout", 30)    # 30 (custom default)

# PYTHONIC: Choose based on intent
# "I expect this key" → use []
# "This key is optional" → use .get()

# === COMMON OPERATIONS ===

# Merging dictionaries (Python 3.9+)
defaults = {"color": "blue", "size": "medium", "weight": "normal"}
user_prefs = {"size": "large", "theme": "dark"}

# PYTHONIC (3.9+):
merged = defaults | user_prefs  # user_prefs overrides defaults
print(f"Merged: {merged}")

# In-place merge:
defaults |= user_prefs

# Before 3.9:
merged = {**defaults, **user_prefs}

# setdefault — set value only if key doesn't exist
word_counts = {}
words = ["hello", "world", "hello", "python", "world", "hello"]

# Pattern: group items
for word in words:
    word_counts.setdefault(word, 0)
    word_counts[word] += 1
# Better: use defaultdict (see section 2.5)

# Dictionary comprehension
squares = {n: n**2 for n in range(6)}
print(f"Squares dict: {squares}")  # {0: 0, 1: 1, 2: 4, 3: 9, 4: 16, 5: 25}

# Filter dict
config = {"host": "localhost", "port": 8080, "debug": True, "verbose": False}
truthy_config = {k: v for k, v in config.items() if v}
print(f"Truthy config: {truthy_config}")

# === ITERATION PATTERNS ===
# PYTHONIC ways to iterate dicts
menu = {"coffee": 3.50, "tea": 2.50, "cake": 4.00}

# Keys (default iteration)
for item in menu:
    print(f"Item: {item}")

# Values
for price in menu.values():
    print(f"Price: {price}")

# Key-value pairs — MOST COMMON
for item, price in menu.items():
    print(f"{item}: ${price:.2f}")

# === SAFE KEY DELETION ===
# pop with default (no KeyError)
removed = config.pop("debug", None)
print(f"Removed: {removed}")


# =============================================================================
# 2.4 SETS AND FROZENSETS
# =============================================================================
"""
WHAT: Unordered collections of unique hashable elements.
      Frozenset is the immutable version (can be dict key or set member).

TIME COMPLEXITY:
- Add/Remove/Contains: O(1) average
- Union/Intersection/Difference: O(min(len(s), len(t)))

WHY sets matter:
- O(1) membership testing (vs O(n) for lists)
- Mathematical set operations (union, intersection, difference)
- Automatic deduplication

WHEN to use sets:
- Removing duplicates from a sequence
- Fast membership testing (is element in collection?)
- Computing intersections, unions, differences
- Tracking "seen" items
"""

# Creation
fruits = {"apple", "banana", "cherry"}
empty_set = set()  # NOT {} — that's an empty dict!
from_list = set([1, 2, 2, 3, 3, 3])  # {1, 2, 3} — duplicates removed

# Set operations — clean mathematical syntax
a = {1, 2, 3, 4, 5}
b = {4, 5, 6, 7, 8}

print(f"Union: {a | b}")           # {1, 2, 3, 4, 5, 6, 7, 8}
print(f"Intersection: {a & b}")    # {4, 5}
print(f"Difference: {a - b}")      # {1, 2, 3}
print(f"Symmetric diff: {a ^ b}")  # {1, 2, 3, 6, 7, 8}
print(f"Subset: {a <= b}")         # False
print(f"Superset: {a >= {1, 2}}")  # True

# PYTHONIC: Use sets for membership testing
# BAD — O(n) lookup:
valid_extensions_list = [".py", ".js", ".ts", ".rs", ".go"]
if ext in valid_extensions_list:  # scans entire list
    pass

# GOOD — O(1) lookup:
VALID_EXTENSIONS = {".py", ".js", ".ts", ".rs", ".go"}
if ext in VALID_EXTENSIONS:  # hash lookup, instant
    pass

# PYTHONIC: Remove duplicates
items = [1, 5, 2, 1, 9, 1, 5, 10]
unique = list(set(items))  # NOTE: order not preserved!
# To preserve order:
unique_ordered = list(dict.fromkeys(items))

# Frozenset — immutable set (can be used as dict key)
permissions = frozenset(["read", "write"])
role_permissions = {
    frozenset(["read"]): "viewer",
    frozenset(["read", "write"]): "editor",
    frozenset(["read", "write", "admin"]): "admin",
}


# =============================================================================
# 2.5 DEQUE, COUNTER, CHAINMAP
# =============================================================================
"""
WHAT: Specialized containers from the `collections` module that solve
      specific problems more efficiently than basic types.

WHY use collections module:
- deque: O(1) operations on both ends (list is O(n) for left operations)
- Counter: counting/tallying without manual loops
- ChainMap: layered dict lookup without merging
"""

from collections import deque, Counter, ChainMap, defaultdict

# === DEQUE (double-ended queue) ===
"""
WHEN to use deque:
- Need fast append/pop on BOTH ends → deque (list is O(n) on left)
- Implementing queues (FIFO) or sliding windows
- Bounded buffers (maxlen parameter)
"""
# FIFO queue
queue = deque()
queue.append("first")    # add to right
queue.append("second")
queue.append("third")
print(queue.popleft())   # "first" — O(1)! (list.pop(0) is O(n))

# Bounded buffer — automatically discards oldest
history = deque(maxlen=5)
for i in range(10):
    history.append(i)
print(f"Last 5: {list(history)}")  # [5, 6, 7, 8, 9]

# Sliding window pattern
def sliding_window(iterable, n):
    """Yield sliding windows of size n."""
    it = iter(iterable)
    window = deque(maxlen=n)
    for _ in range(n):
        window.append(next(it))
    yield tuple(window)
    for item in it:
        window.append(item)
        yield tuple(window)

print(list(sliding_window([1, 2, 3, 4, 5], 3)))
# [(1, 2, 3), (2, 3, 4), (3, 4, 5)]

# === COUNTER ===
"""
WHEN to use Counter:
- Counting occurrences of elements
- Finding most common elements
- Mathematical multiset operations
"""
text = "hello world hello python hello world"
word_counts = Counter(text.split())
print(f"Counts: {word_counts}")  # Counter({'hello': 3, 'world': 2, 'python': 1})
print(f"Most common 2: {word_counts.most_common(2)}")  # [('hello', 3), ('world', 2)]

# Counter arithmetic
inventory = Counter(apples=5, oranges=3)
sold = Counter(apples=2, oranges=1)
remaining = inventory - sold  # Counter({'apples': 3, 'oranges': 2})

# === DEFAULTDICT ===
"""
WHEN to use defaultdict:
- Grouping items (avoid checking if key exists)
- Building indexes/inverted indexes
- Any pattern where you'd use .setdefault() or check `if key in dict`
"""
# Grouping pattern
from collections import defaultdict

# WITHOUT defaultdict — verbose
groups = {}
words = ["apple", "bat", "bar", "atom", "book"]
for word in words:
    key = word[0]  # first letter
    if key not in groups:
        groups[key] = []
    groups[key].append(word)

# WITH defaultdict — PYTHONIC
groups = defaultdict(list)
for word in words:
    groups[word[0]].append(word)
print(f"Groups: {dict(groups)}")  # {'a': ['apple', 'atom'], 'b': ['bat', 'bar', 'book']}

# === CHAINMAP ===
"""
WHEN to use ChainMap:
- Layered configuration (defaults → user → command-line)
- Scoped variable lookup (like Python's own name resolution)
- When you want to search multiple dicts without merging them
"""
defaults = {"color": "blue", "size": "medium"}
env_config = {"color": "green"}
cli_args = {"size": "large"}

# ChainMap searches in order — first match wins
config = ChainMap(cli_args, env_config, defaults)
print(f"color: {config['color']}")  # "green" (from env_config)
print(f"size: {config['size']}")    # "large" (from cli_args)


# =============================================================================
# 2.6 HEAPS AND PRIORITY QUEUES
# =============================================================================
"""
WHAT: heapq provides a min-heap implemented as a list.
      Efficiently get smallest element.

TIME COMPLEXITY:
- Push: O(log n)
- Pop min: O(log n)
- Peek min: O(1)

WHEN to use:
- Need "get minimum/maximum efficiently"
- Priority queues
- Top-K problems
- Merge sorted sequences
"""
import heapq

# Basic heap operations
heap = []
heapq.heappush(heap, 5)
heapq.heappush(heap, 1)
heapq.heappush(heap, 3)
print(f"Min: {heap[0]}")           # 1 — peek without removing
print(f"Pop min: {heapq.heappop(heap)}")  # 1 — remove and return smallest

# Get N largest/smallest — more efficient than sorting
data = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5]
print(f"3 largest: {heapq.nlargest(3, data)}")   # [9, 6, 5]
print(f"3 smallest: {heapq.nsmallest(3, data)}") # [1, 1, 2]

# Priority queue with tuples (priority, item)
# Lower number = higher priority
tasks = []
heapq.heappush(tasks, (2, "code review"))
heapq.heappush(tasks, (1, "fix critical bug"))
heapq.heappush(tasks, (3, "write docs"))

while tasks:
    priority, task = heapq.heappop(tasks)
    print(f"  [{priority}] {task}")
# Output: fix critical bug, code review, write docs


# =============================================================================
# 2.7 STRUCTURAL PATTERN MATCHING WITH DATA STRUCTURES
# =============================================================================
"""
WHAT: Python 3.10+ match/case can destructure and match data structures.
      This is incredibly powerful for processing complex nested data.

WHY: Replaces complex if/elif chains with clear, declarative patterns.
"""

# Matching sequences
def process_command(command):
    match command.split():
        case ["quit"]:
            return "Exiting..."
        case ["hello", name]:
            return f"Hello, {name}!"
        case ["move", direction, distance]:
            return f"Moving {direction} by {distance}"
        case ["add", *items]:
            return f"Adding items: {items}"
        case _:
            return "Unknown command"

print(process_command("hello World"))      # Hello, World!
print(process_command("add a b c"))        # Adding items: ['a', 'b', 'c']

# Matching dicts
def handle_event(event):
    match event:
        case {"type": "click", "x": x, "y": y}:
            return f"Click at ({x}, {y})"
        case {"type": "keypress", "key": key}:
            return f"Key pressed: {key}"
        case {"type": "scroll", "direction": "up" | "down" as d}:
            return f"Scroll {d}"
        case _:
            return "Unknown event"

print(handle_event({"type": "click", "x": 100, "y": 200}))
print(handle_event({"type": "keypress", "key": "Enter"}))


# =============================================================================
# SUMMARY: DATA STRUCTURES DECISION GUIDE
# =============================================================================
"""
┌─────────────────────────────┬──────────────────────────────────────────────┐
│ Need                        │ Use                                          │
├─────────────────────────────┼──────────────────────────────────────────────┤
│ Ordered, mutable sequence   │ list                                         │
│ Immutable sequence/record   │ tuple / NamedTuple                           │
│ Key-value mapping           │ dict                                         │
│ Unique elements             │ set                                          │
│ Fast both-ends queue        │ collections.deque                            │
│ Counting elements           │ collections.Counter                          │
│ Grouping by key             │ collections.defaultdict(list)                │
│ Layered config              │ collections.ChainMap                         │
│ Priority queue              │ heapq                                        │
│ Immutable dict key (set)    │ frozenset                                    │
│ Typed record (mutable)      │ @dataclass                                   │
│ Typed record (immutable)    │ NamedTuple or @dataclass(frozen=True)        │
└─────────────────────────────┴──────────────────────────────────────────────┘

PERFORMANCE RULES:
- `in` operator: set/dict O(1), list/tuple O(n)
- Append to end: list O(1), deque O(1)
- Append to front: list O(n), deque O(1)
- Sort: list O(n log n)
- Min/Max: heapq O(log n) for repeated ops, min()/max() O(n) for one-shot
"""
