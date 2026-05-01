"""
=============================================================================
CHAPTER 17: PYTHONIC IDIOMS & BEST PRACTICES
=============================================================================
"Pythonic" code follows Python's design philosophy and conventions.
It's code that experienced Python developers recognize as natural, clear,
and taking advantage of the language's strengths.

THE ZEN OF PYTHON (import this):
- Beautiful is better than ugly
- Explicit is better than implicit
- Simple is better than complex
- Flat is better than nested
- Readability counts
- There should be one obvious way to do it
- If the implementation is hard to explain, it's a bad idea

=============================================================================
"""

# =============================================================================
# 17.1 EAFP vs LBYL
# =============================================================================
"""
WHAT:
- EAFP: Easier to Ask Forgiveness than Permission (try/except)
- LBYL: Look Before You Leap (check conditions first)

WHY EAFP is Pythonic:
- Faster when exception is rare (no precondition checks)
- Thread-safe (no TOCTOU race conditions)
- More readable for many scenarios
- Works with duck typing (don't check type, just try using it)

WHEN each approach is appropriate:
- EAFP: most situations in Python (default choice)
- LBYL: when the check is cheap AND failure is common AND no exception would be raised
"""

# LBYL style (non-Pythonic in most cases):
def get_value_lbyl(dictionary, key, default=None):
    if key in dictionary:  # Check first
        return dictionary[key]
    return default

# EAFP style (Pythonic):
def get_value_eafp(dictionary, key, default=None):
    try:
        return dictionary[key]  # Just try it
    except KeyError:
        return default

# MOST Pythonic: use built-in that does it for you
# dictionary.get(key, default)

# Real-world example: file operations
# LBYL (race condition! file could be deleted between check and open):
import os
# if os.path.exists("config.json"):
#     with open("config.json") as f:
#         config = json.load(f)

# EAFP (atomic, no race condition):
import json
try:
    with open("config.json") as f:
        config = json.load(f)
except (FileNotFoundError, json.JSONDecodeError):
    config = {}

# When LBYL is appropriate:
# - Checking permissions before attempting expensive operations
# - Validating user input before complex processing
# - When the "exception" case is common (50%+ of calls would raise)


# =============================================================================
# 17.2 UNPACKING AND STARRED ASSIGNMENTS
# =============================================================================
"""
WHAT: Destructure iterables into variables in a single statement.
      Python's unpacking is incredibly versatile and expressive.

WHY unpacking is essential:
- Eliminates index-based access (items[0], items[1])
- Self-documenting (variable names describe the data)
- Works in assignments, for loops, function arguments
"""

# Basic tuple unpacking
point = (3, 4)
x, y = point

# Swap values (no temp variable needed)
a, b = 1, 2
a, b = b, a

# Starred assignment — capture "the rest"
first, *rest = [1, 2, 3, 4, 5]
# first=1, rest=[2, 3, 4, 5]

*start, last = [1, 2, 3, 4, 5]
# start=[1, 2, 3, 4], last=5

first, *middle, last = [1, 2, 3, 4, 5]
# first=1, middle=[2, 3, 4], last=5

# Nested unpacking
(a, b), (c, d) = (1, 2), (3, 4)
# a=1, b=2, c=3, d=4

# Unpacking in for loops
pairs = [(1, "a"), (2, "b"), (3, "c")]
for number, letter in pairs:
    print(f"{number}: {letter}")

# Ignoring values with _
_, important, _ = (1, 42, 3)

# Unpack function returns
from os.path import splitext
name, ext = splitext("document.pdf")

# Unpack in function calls
def draw_point(x, y, z):
    print(f"Drawing at ({x}, {y}, {z})")

coords = [1, 2, 3]
draw_point(*coords)  # Unpack list as positional args

config = {"x": 10, "y": 20, "z": 30}
draw_point(**config)  # Unpack dict as keyword args


# =============================================================================
# 17.3 ENUMERATE OVER RANGE(LEN())
# =============================================================================
"""
WHAT: Use enumerate() when you need both index and value.
      NEVER use range(len(x)) in Python (C-style thinking).

WHY enumerate is superior:
- More readable (intent is clear)
- Less error-prone (no off-by-one errors)
- Works with any iterable (not just sequences)
- Can start from any index
"""

fruits = ["apple", "banana", "cherry"]

# BAD (C-style):
for i in range(len(fruits)):
    print(f"{i}: {fruits[i]}")

# GOOD (Pythonic):
for i, fruit in enumerate(fruits):
    print(f"{i}: {fruit}")

# Start from 1 (useful for display):
for i, fruit in enumerate(fruits, start=1):
    print(f"{i}. {fruit}")


# =============================================================================
# 17.4 ZIP FOR PARALLEL ITERATION
# =============================================================================
"""
WHAT: Iterate over multiple sequences in lockstep.
WHY: Cleaner than indexing, works with any iterables.
"""

names = ["Alice", "Bob", "Charlie"]
ages = [30, 25, 35]
cities = ["NYC", "LA", "Chicago"]

# BAD:
for i in range(len(names)):
    print(f"{names[i]} is {ages[i]} from {cities[i]}")

# GOOD:
for name, age, city in zip(names, ages, cities):
    print(f"{name} is {age} from {city}")

# Create dict from two lists
name_age = dict(zip(names, ages))
print(f"Dict: {name_age}")

# Unzip (transpose)
pairs = [(1, "a"), (2, "b"), (3, "c")]
numbers, letters = zip(*pairs)


# =============================================================================
# 17.5 DICTIONARY IDIOMS
# =============================================================================
"""
Best practices for working with dictionaries.
"""

# Use .get() for optional keys (not if/else)
config = {"host": "localhost", "port": 8080}

# BAD:
# if "timeout" in config:
#     timeout = config["timeout"]
# else:
#     timeout = 30

# GOOD:
timeout = config.get("timeout", 30)

# setdefault — get or set (atomic)
cache = {}
# BAD:
# if key not in cache:
#     cache[key] = compute_value(key)
# return cache[key]

# GOOD:
# return cache.setdefault(key, compute_value(key))
# NOTE: compute_value always runs! For lazy, use defaultdict

# Merge dicts (3.9+)
defaults = {"color": "blue", "size": "M"}
overrides = {"size": "L", "weight": "bold"}
final = defaults | overrides  # overrides wins

# In-place merge
defaults |= overrides

# Dict comprehension for transformation
prices = {"apple": 1.5, "banana": 0.75, "cherry": 3.0}
expensive = {k: v for k, v in prices.items() if v > 1.0}


# =============================================================================
# 17.6 TRUTHINESS AND FALSY VALUES
# =============================================================================
"""
WHAT: Use Python's truthiness rules to write concise conditions.
      Don't compare to True, False, None, [], "", 0 explicitly.
"""

items = [1, 2, 3]
name = "Alice"
count = 0
data = None

# GOOD — use truthiness directly:
if items:          # True if non-empty
    pass
if not name:       # True if empty string
    pass
if data is None:   # Use `is` for None specifically
    pass

# BAD — unnecessary comparisons:
# if items != []:
# if len(items) > 0:
# if name != "":
# if data == None:   # WRONG — use `is`
# if count == 0:     # Only bad if checking truthiness, fine for numeric comparison


# =============================================================================
# 17.7 AVOIDING MUTABLE DEFAULT ARGUMENTS
# =============================================================================
"""
WHY: Mutable defaults are shared across ALL calls (created once at def time).
     This is Python's most common gotcha.
"""

# BUG:
def append_to_buggy(item, target=[]):
    target.append(item)
    return target

print(append_to_buggy(1))  # [1]
print(append_to_buggy(2))  # [1, 2] — BUG! Same list reused!

# FIX:
def append_to_fixed(item, target=None):
    if target is None:
        target = []
    target.append(item)
    return target

print(append_to_fixed(1))  # [1]
print(append_to_fixed(2))  # [2] — correct, independent list each time


# =============================================================================
# 17.8 CONTEXT MANAGERS FOR RESOURCE MANAGEMENT
# =============================================================================
"""
WHY: Guarantees cleanup even on exception. Eliminates resource leaks.
RULE: If something needs cleanup, use a context manager.
"""

# Files
with open("data.txt", "w") as f:
    f.write("hello")

# Locks
import threading
lock = threading.Lock()
with lock:
    pass  # Critical section

# Database connections
# with get_connection() as conn:
#     conn.execute("...")

# Temporary directory
import tempfile
with tempfile.TemporaryDirectory() as tmpdir:
    pass  # Auto-deleted after


# =============================================================================
# 17.9 COMPREHENSIONS OVER MAP/FILTER
# =============================================================================
"""
PYTHONIC RULE: Prefer comprehensions over map()/filter() with lambda.
              Use map()/filter() only when you have a named function already.
"""

numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

# GOOD — comprehension (clear, readable):
squares = [x ** 2 for x in numbers]
evens = [x for x in numbers if x % 2 == 0]

# LESS PYTHONIC — map/filter with lambda:
squares = list(map(lambda x: x ** 2, numbers))
evens = list(filter(lambda x: x % 2 == 0, numbers))

# ACCEPTABLE — map with named function:
strings = ["1", "2", "3"]
ints = list(map(int, strings))  # This is actually cleaner than [int(s) for s in strings]


# =============================================================================
# 17.10 STRING OPERATIONS
# =============================================================================
"""
Key string idioms every Python developer should know.
"""

# Use join() for concatenation (not + in loops)
words = ["Hello", "World", "Python"]
# BAD — O(n²):
result = ""
for w in words:
    result += w + " "
# GOOD — O(n):
result = " ".join(words)

# f-strings over .format() and %
name, age = "Alice", 30
# BEST (3.6+):
msg = f"{name} is {age}"
# OK (older code):
msg = "{} is {}".format(name, age)
# AVOID:
msg = "%s is %d" % (name, age)

# Use pathlib over os.path string manipulation
from pathlib import Path
path = Path("data") / "output" / "results.csv"
# NOT: os.path.join("data", "output", "results.csv")

# String methods over regex for simple cases
text = "Hello, World!"
# GOOD:
text.startswith("Hello")
text.endswith("!")
text.replace(",", ";")
"World" in text

# Use removeprefix/removesuffix (3.9+)
filename = "test_module.py"
module_name = filename.removeprefix("test_").removesuffix(".py")
print(f"Module: {module_name}")  # "module"


# =============================================================================
# 17.11 GENERATOR EXPRESSIONS FOR MEMORY EFFICIENCY
# =============================================================================
"""
RULE: If you only iterate once, use a generator expression (not list).
      Especially in function arguments.
"""

numbers = range(1_000_000)

# BAD — creates million-element list just to sum it:
total = sum([x ** 2 for x in numbers])

# GOOD — generates values one at a time, O(1) memory:
total = sum(x ** 2 for x in numbers)

# Same pattern:
any_big = any(x > 999_999 for x in numbers)  # Short-circuits!
max_val = max(len(line) for line in open("file.txt"))


# =============================================================================
# 17.12 FLAT IS BETTER THAN NESTED
# =============================================================================
"""
PRINCIPLE: Reduce nesting by using early returns, comprehensions,
           helper functions, and built-in operations.
"""

# BAD — deeply nested:
def process_orders_bad(orders):
    result = []
    for order in orders:
        if order is not None:
            if order.get("status") == "active":
                items = order.get("items", [])
                for item in items:
                    if item.get("quantity", 0) > 0:
                        result.append(item)
    return result

# GOOD — flat with early continues:
def process_orders_good(orders):
    result = []
    for order in orders:
        if order is None:
            continue
        if order.get("status") != "active":
            continue
        for item in order.get("items", []):
            if item.get("quantity", 0) > 0:
                result.append(item)
    return result

# EVEN BETTER — extract helper:
def is_valid_item(item):
    return item.get("quantity", 0) > 0

def get_active_items(orders):
    return [
        item
        for order in orders
        if order and order.get("status") == "active"
        for item in order.get("items", [])
        if is_valid_item(item)
    ]


# =============================================================================
# 17.13 DUCK TYPING AND PROTOCOLS
# =============================================================================
"""
PRINCIPLE: "If it walks like a duck and quacks like a duck, it's a duck."
           Check behavior, not type.
"""

# BAD — checking type:
def get_length_bad(obj):
    if isinstance(obj, (list, tuple, str)):
        return len(obj)
    raise TypeError("Unsupported type")

# GOOD — duck typing (EAFP):
def get_length_good(obj):
    return len(obj)  # Works with anything that has __len__

# With type hints + Protocol for documentation:
from typing import Protocol, Sized

def get_length_typed(obj: Sized) -> int:
    return len(obj)


# =============================================================================
# 17.14 SINGLE RESPONSIBILITY AND SMALL FUNCTIONS
# =============================================================================
"""
PRINCIPLE: Each function should do ONE thing well.
           If you can't name it clearly, it does too much.
"""

# BAD — does too many things:
def process_user_data_bad(filepath):
    with open(filepath) as f:
        data = json.load(f)

    valid_users = []
    for user in data:
        if user.get("age", 0) >= 18 and "@" in user.get("email", ""):
            user["name"] = user["name"].strip().title()
            valid_users.append(user)

    with open("output.json", "w") as f:
        json.dump(valid_users, f)

    return len(valid_users)

# GOOD — each function does one thing:
def load_users(filepath):
    """Read users from file."""
    with open(filepath) as f:
        return json.load(f)

def is_valid_user(user):
    """Check if user meets criteria."""
    return user.get("age", 0) >= 18 and "@" in user.get("email", "")

def normalize_name(user):
    """Normalize user's name."""
    return {**user, "name": user["name"].strip().title()}

def save_users(users, filepath):
    """Write users to file."""
    with open(filepath, "w") as f:
        json.dump(users, f)

def process_user_data_good(filepath):
    """Orchestrate the pipeline."""
    users = load_users(filepath)
    valid = [normalize_name(u) for u in users if is_valid_user(u)]
    save_users(valid, "output.json")
    return len(valid)


# =============================================================================
# SUMMARY: PYTHONIC CHECKLIST
# =============================================================================
"""
✓ Use EAFP (try/except) over LBYL (if/else checks)
✓ Use unpacking: a, b = func_that_returns_tuple()
✓ Use enumerate() instead of range(len())
✓ Use zip() for parallel iteration
✓ Use .get() for optional dict keys
✓ Use truthiness: `if items:` not `if len(items) > 0:`
✓ Use `is None` not `== None`
✓ Use None for mutable default arguments
✓ Use context managers (with) for resources
✓ Use comprehensions over map/filter with lambda
✓ Use f-strings for formatting
✓ Use join() for string concatenation
✓ Use pathlib for file paths
✓ Use generators for large data (sum(x for x in ...))
✓ Keep code flat (guard clauses, early returns)
✓ Use duck typing (don't check types, check behavior)
✓ One function = one responsibility
✓ Use _ for throwaway variables
✓ Use proper naming: functions=verbs, classes=nouns, bool=is/has/can
"""
