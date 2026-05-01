"""
=============================================================================
CHAPTER 3: CONTROL FLOW
=============================================================================
Python's control flow is designed to be readable and expressive.
Key philosophy: "Flat is better than nested."

WHY Python's control flow is different:
- No switch/case (until 3.10's match/case) — designed for clarity
- for/else and while/else — unique to Python
- Exception handling is a control flow mechanism (EAFP)
- Context managers replace try/finally boilerplate

=============================================================================
"""

# =============================================================================
# 3.1 IF / ELIF / ELSE
# =============================================================================
"""
WHAT: Conditional branching. Python uses indentation (not braces) for blocks.

PYTHONIC PRINCIPLES:
- Use truthiness directly (no `== True`, `!= []`, `> 0` for lengths)
- Prefer early returns over deep nesting
- Keep conditions simple — extract complex logic into well-named functions
"""

# PYTHONIC: Use truthiness
items = [1, 2, 3]

# GOOD:
if items:
    print("Has items")

# BAD (unnecessarily verbose):
# if len(items) > 0:
# if items != []:
# if bool(items) == True:

# PYTHONIC: Early return (guard clauses) — flat is better than nested
def process_payment(user, amount):
    """Demonstrate early returns vs nested ifs."""
    # GOOD — guard clauses
    if user is None:
        return "No user"
    if not user.is_active:
        return "User inactive"
    if amount <= 0:
        return "Invalid amount"
    if amount > user.balance:
        return "Insufficient funds"

    # Happy path — at lowest indentation level
    user.balance -= amount
    return "Payment successful"

    # BAD — deep nesting:
    # if user is not None:
    #     if user.is_active:
    #         if amount > 0:
    #             if amount <= user.balance:
    #                 user.balance -= amount
    #                 return "Payment successful"

# PYTHONIC: Conditional expression (ternary)
age = 20
status = "adult" if age >= 18 else "minor"
# Use for simple cases only. Don't nest ternaries!

# BAD — hard to read:
# result = "a" if x > 0 else "b" if x == 0 else "c"
# GOOD — use regular if/elif for complex cases


# =============================================================================
# 3.2 FOR LOOPS
# =============================================================================
"""
WHAT: Python's `for` iterates over ANY iterable (not just indices).
      This is fundamentally different from C-style for loops.

WHY Python for loops are different:
- Iterate over items directly (not indices)
- Works with any iterable (lists, dicts, files, generators, custom objects)
- Combined with enumerate() for indices when needed

BEST PRACTICES:
- Never use range(len(x)) when you can iterate directly
- Use enumerate() when you need index + value
- Use zip() for parallel iteration
- Prefer comprehensions for simple transformations
"""

# PYTHONIC: Iterate directly
fruits = ["apple", "banana", "cherry"]

# GOOD:
for fruit in fruits:
    print(fruit)

# BAD (C-style thinking):
# for i in range(len(fruits)):
#     print(fruits[i])

# PYTHONIC: enumerate() for index + value
for i, fruit in enumerate(fruits):
    print(f"{i}: {fruit}")

# Start from different index
for i, fruit in enumerate(fruits, start=1):
    print(f"{i}. {fruit}")

# PYTHONIC: zip() for parallel iteration
names = ["Alice", "Bob", "Charlie"]
ages = [30, 25, 35]
cities = ["NYC", "LA", "Chicago"]

for name, age, city in zip(names, ages, cities):
    print(f"{name}, {age}, {city}")

# zip stops at shortest — use zip_longest for padding
from itertools import zip_longest
short = [1, 2]
long = [10, 20, 30]
for a, b in zip_longest(short, long, fillvalue=0):
    print(f"{a}, {b}")  # (1,10), (2,20), (0,30)

# === FOR/ELSE ===
"""
WHAT: The `else` clause on a for loop runs ONLY if the loop completed
      without hitting `break`. Think of it as "no break".

WHY this exists:
- Clean pattern for "search and found/not found"
- Eliminates the need for flag variables

WHEN to use:
- Searching for an item (break when found, else = not found)
- Validation (break on first failure, else = all passed)
"""

# Pattern: Search with for/else
def find_prime_factor(n):
    """Find smallest prime factor."""
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            print(f"Found factor: {i}")
            break
    else:
        # Only runs if NO break occurred (loop completed normally)
        print(f"{n} is prime!")

find_prime_factor(17)  # "17 is prime!"
find_prime_factor(15)  # "Found factor: 3"

# Equivalent without for/else (requires flag variable):
def find_prime_factor_verbose(n):
    found = False
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            print(f"Found factor: {i}")
            found = True
            break
    if not found:
        print(f"{n} is prime!")


# =============================================================================
# 3.3 WHILE LOOPS
# =============================================================================
"""
WHAT: Loop while condition is true. Less common than `for` in Python.

WHEN to use while (instead of for):
- Unknown number of iterations (user input, convergence)
- Infinite loops (servers, event loops) with explicit break
- When you need the walrus operator pattern
"""

# Walrus operator + while — read-process pattern
import io

# Simulate reading from a file/socket
source = io.StringIO("line1\nline2\nline3\n")
while (line := source.readline()):
    print(f"Processing: {line.strip()}")

# Convergence loop
def newton_sqrt(n, tolerance=1e-10):
    """Newton's method for square root."""
    guess = n / 2.0
    while True:
        new_guess = (guess + n / guess) / 2
        if abs(new_guess - guess) < tolerance:
            return new_guess
        guess = new_guess

print(f"sqrt(2) ≈ {newton_sqrt(2)}")  # 1.4142135623...

# while/else — same semantics as for/else (else runs if no break)


# =============================================================================
# 3.4 MATCH / CASE (Structural Pattern Matching, 3.10+)
# =============================================================================
"""
WHAT: Python's answer to switch/case, but MUCH more powerful.
      It's not just value matching — it's structural decomposition.

WHY match/case is a game-changer:
- Replaces complex if/elif chains
- Destructures data (extract values while matching structure)
- Guards (if conditions within patterns)
- Type matching (isinstance replacement)

WHEN to use match/case:
- Processing commands or messages with different structures
- Parsing ASTs or nested data
- State machines
- Any code with lots of isinstance checks

HOW it works:
- Matches the STRUCTURE and VALUES of data
- Binds variables during matching (capture patterns)
- Falls through to `case _:` as wildcard/default
"""

# Basic value matching (like switch/case in other languages)
def http_status(status):
    match status:
        case 200:
            return "OK"
        case 301:
            return "Moved Permanently"
        case 404:
            return "Not Found"
        case 500:
            return "Internal Server Error"
        case _:
            return f"Unknown status: {status}"

# OR patterns — match multiple values
def classify_char(char):
    match char:
        case 'a' | 'e' | 'i' | 'o' | 'u':
            return "vowel"
        case ' ' | '\t' | '\n':
            return "whitespace"
        case _:
            return "other"

# Sequence patterns — destructure lists/tuples
def handle_point(point):
    match point:
        case (0, 0):
            return "Origin"
        case (x, 0):
            return f"On x-axis at x={x}"
        case (0, y):
            return f"On y-axis at y={y}"
        case (x, y):
            return f"Point at ({x}, {y})"

print(handle_point((0, 0)))   # Origin
print(handle_point((5, 0)))   # On x-axis at x=5
print(handle_point((3, 4)))   # Point at (3, 4)

# Mapping patterns — match dict structures
def process_api_response(response):
    match response:
        case {"status": "success", "data": data}:
            return f"Got data: {data}"
        case {"status": "error", "message": msg}:
            return f"Error: {msg}"
        case {"status": "redirect", "url": url}:
            return f"Redirect to: {url}"
        case _:
            return "Unknown response format"

# Class patterns — match object attributes (replaces isinstance chains)
from dataclasses import dataclass

@dataclass
class Circle:
    radius: float

@dataclass
class Rectangle:
    width: float
    height: float

@dataclass
class Triangle:
    base: float
    height: float

def area(shape):
    match shape:
        case Circle(radius=r):
            return 3.14159 * r ** 2
        case Rectangle(width=w, height=h):
            return w * h
        case Triangle(base=b, height=h):
            return 0.5 * b * h
        case _:
            raise ValueError(f"Unknown shape: {shape}")

print(f"Circle area: {area(Circle(5)):.2f}")
print(f"Rectangle area: {area(Rectangle(3, 4)):.2f}")

# Guards — add conditions to patterns
def classify_number(n):
    match n:
        case x if x < 0:
            return "negative"
        case 0:
            return "zero"
        case x if x % 2 == 0:
            return "positive even"
        case _:
            return "positive odd"

# Star patterns — capture remaining elements
def first_and_rest(items):
    match items:
        case []:
            return "Empty"
        case [single]:
            return f"Just one: {single}"
        case [first, *rest]:
            return f"First: {first}, Rest: {rest}"


# =============================================================================
# 3.5 EXCEPTION HANDLING
# =============================================================================
"""
WHAT: try/except/else/finally for handling errors gracefully.

WHY Python uses exceptions extensively:
- EAFP principle: "Easier to Ask Forgiveness than Permission"
- Exceptions are cheap in Python (unlike C++)
- Cleaner than checking for errors at every step

EAFP vs LBYL:
- LBYL (Look Before You Leap): Check if operation is possible, then do it
- EAFP (Easier to Ask Forgiveness): Just do it, handle exception if it fails
- Python prefers EAFP — it's faster when exceptions are rare, and thread-safe

THE FULL try STATEMENT:
    try:
        # Code that might raise
    except SomeError as e:
        # Handle specific error
    except (TypeError, ValueError):
        # Handle multiple error types
    else:
        # Runs ONLY if no exception was raised (success path)
    finally:
        # ALWAYS runs (cleanup)
"""

# PYTHONIC: EAFP approach
# BAD (LBYL — race condition possible, verbose):
import os
# if os.path.exists("config.json"):
#     with open("config.json") as f:
#         data = f.read()

# GOOD (EAFP — atomic, clean):
try:
    with open("config.json") as f:
        data = f.read()
except FileNotFoundError:
    data = "{}"  # default

# PYTHONIC: Use specific exceptions (NEVER bare except)
# BAD — catches EVERYTHING including KeyboardInterrupt, SystemExit:
# try:
#     do_something()
# except:  # NEVER DO THIS
#     pass

# BAD — too broad:
# except Exception:
#     pass

# GOOD — specific:
try:
    value = int("not a number")
except ValueError as e:
    print(f"Invalid input: {e}")

# The `else` clause — runs only on success
def safe_divide(a, b):
    try:
        result = a / b
    except ZeroDivisionError:
        print("Cannot divide by zero")
        return None
    else:
        # Only runs if no exception — good place for success logic
        print(f"Division successful: {result}")
        return result
    finally:
        # Always runs — good for cleanup
        print("Division attempted")

# Multiple except with different handling
def parse_config(text):
    try:
        import json
        config = json.loads(text)
        port = config["port"]
        return int(port)
    except json.JSONDecodeError as e:
        print(f"Invalid JSON: {e}")
    except KeyError:
        print("Missing 'port' in config")
    except (TypeError, ValueError) as e:
        print(f"Invalid port value: {e}")
    return None


# =============================================================================
# 3.6 EXCEPTION GROUPS (Python 3.11+)
# =============================================================================
"""
WHAT: ExceptionGroup wraps multiple exceptions that occurred concurrently.
      `except*` handles specific exception types from the group.

WHY: Async code (TaskGroup) can have multiple simultaneous failures.
     Exception groups let you handle each type separately.
"""

# Creating and handling exception groups
def validate_data(data):
    errors = []
    if not data.get("name"):
        errors.append(ValueError("name is required"))
    if not data.get("email"):
        errors.append(ValueError("email is required"))
    if data.get("age", 0) < 0:
        errors.append(TypeError("age must be non-negative"))

    if errors:
        raise ExceptionGroup("Validation failed", errors)

# Handling with except*
try:
    validate_data({"age": -1})
except* ValueError as eg:
    print(f"Value errors: {eg.exceptions}")
except* TypeError as eg:
    print(f"Type errors: {eg.exceptions}")


# =============================================================================
# 3.7 CONTEXT MANAGERS (with statement)
# =============================================================================
"""
WHAT: Context managers guarantee cleanup code runs, even if exceptions occur.
      They implement __enter__ and __exit__ (or use @contextmanager).

WHY context managers are essential:
- Guarantee resource cleanup (files, locks, connections)
- Replace try/finally boilerplate
- Make resource management impossible to forget
- Composable (multiple `with` in one statement)

WHEN to use:
- File operations (always!)
- Database connections/transactions
- Locks and synchronization
- Temporary state changes (monkeypatch, directory change)
- Anything with setup/teardown

HOW to create custom context managers:
1. Class with __enter__/__exit__
2. @contextmanager decorator (simpler)
"""

# Basic usage — file handling
# PYTHONIC — file is guaranteed to close:
with open("example.txt", "w") as f:
    f.write("hello")
# f is closed here, even if exception occurred

# Multiple context managers (Python 3.1+, parenthesized 3.10+)
# Old style:
# with open("in.txt") as fin:
#     with open("out.txt", "w") as fout:
#         fout.write(fin.read())

# Modern style (3.10+ parenthesized):
# with (
#     open("in.txt") as fin,
#     open("out.txt", "w") as fout,
# ):
#     fout.write(fin.read())

# Custom context manager using @contextmanager
from contextlib import contextmanager
import time

@contextmanager
def timer(label):
    """Measure execution time of a block."""
    start = time.perf_counter()
    try:
        yield  # Control passes to the `with` block here
    finally:
        elapsed = time.perf_counter() - start
        print(f"{label}: {elapsed:.4f} seconds")

# Usage:
with timer("Processing"):
    total = sum(range(1_000_000))

# Custom context manager using class
class DatabaseConnection:
    """Context manager for database connections."""

    def __init__(self, connection_string):
        self.connection_string = connection_string
        self.connection = None

    def __enter__(self):
        """Called when entering `with` block. Returns the resource."""
        print(f"Connecting to {self.connection_string}")
        self.connection = {"connected": True}  # simulate
        return self.connection

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Called when leaving `with` block (even on exception).

        Args:
            exc_type: Exception type (or None)
            exc_val: Exception value (or None)
            exc_tb: Traceback (or None)

        Returns:
            True to suppress the exception, False/None to propagate it.
        """
        print("Closing connection")
        self.connection = None
        return False  # Don't suppress exceptions

# contextlib utilities
from contextlib import suppress, redirect_stdout

# suppress — ignore specific exceptions
# Instead of:
# try:
#     os.remove("file.txt")
# except FileNotFoundError:
#     pass

# PYTHONIC:
with suppress(FileNotFoundError):
    os.remove("file.txt")


# =============================================================================
# SUMMARY: CONTROL FLOW BEST PRACTICES
# =============================================================================
"""
1. Use truthiness: `if items:` not `if len(items) > 0:`
2. Prefer early returns (guard clauses) over deep nesting
3. Iterate directly: `for item in items:` not `for i in range(len(items)):`
4. Use enumerate() for index+value, zip() for parallel iteration
5. Use for/else for "search and found/not found" patterns
6. Use match/case (3.10+) for structural decomposition
7. Follow EAFP: try/except instead of checking preconditions
8. Catch specific exceptions, never bare `except:`
9. Use context managers (with) for ALL resource management
10. Keep conditional logic flat — extract complex conditions into functions
"""
