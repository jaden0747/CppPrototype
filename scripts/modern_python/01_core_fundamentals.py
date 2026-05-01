"""
=============================================================================
CHAPTER 1: CORE LANGUAGE FUNDAMENTALS
=============================================================================
Python's core fundamentals are the building blocks of everything you write.
Understanding them deeply lets you write cleaner, faster, more idiomatic code.

WHY learn fundamentals deeply?
- Python's dynamic typing has subtle behaviors that trip up beginners
- Understanding how Python handles objects, references, and memory prevents bugs
- Knowing the difference between mutable/immutable impacts design decisions

=============================================================================
"""

# =============================================================================
# 1.1 VARIABLES AND DYNAMIC TYPING
# =============================================================================
"""
WHAT: Variables in Python are names (labels) bound to objects. They are NOT boxes
      that hold values — they are references (pointers) to objects in memory.

WHY it matters:
- Assignment never copies data; it creates a new reference to the same object
- Understanding this prevents aliasing bugs with mutable objects
- This is fundamentally different from C/C++ where variables are memory locations

HOW Python handles variables:
1. Every value is an object (even integers, functions, modules)
2. Variables are entries in a namespace dictionary
3. `=` binds a name to an object, it does NOT copy

WHEN to care:
- When passing mutable objects to functions (lists, dicts get modified in-place)
- When creating copies vs references
- When reasoning about memory and garbage collection
"""

# Variables are references, not containers
a = [1, 2, 3]
b = a  # b points to the SAME list object as a
b.append(4)
print(f"a = {a}")  # a = [1, 2, 3, 4]  -- both see the change!
print(f"b = {b}")  # b = [1, 2, 3, 4]
print(f"a is b: {a is b}")  # True — same object in memory

# To make an independent copy:
c = a.copy()  # shallow copy
c.append(5)
print(f"a = {a}")  # a = [1, 2, 3, 4] — unchanged
print(f"c = {c}")  # c = [1, 2, 3, 4, 5]

# Python uses reference counting + garbage collection
import sys
x = "hello"
print(f"Reference count of x: {sys.getrefcount(x)}")  # Usually 2+ (arg to getrefcount is one ref)

# Dynamic typing: variables can be rebound to different types
value = 42       # int
value = "hello"  # now str — no error, but avoid this in practice
value = [1, 2]   # now list

# BEST PRACTICE: Don't reassign variables to different types
# It makes code confusing and breaks type checkers
# BAD:
result = get_user()  # might return User or None — use Optional[User] type hint

# PYTHONIC: Use type hints to document intent (even though they're not enforced at runtime)
name: str = "Alice"
age: int = 30


# =============================================================================
# 1.2 NUMERIC TYPES
# =============================================================================
"""
WHAT: Python has several numeric types:
- int: arbitrary precision integers (no overflow!)
- float: 64-bit IEEE 754 double precision
- complex: complex numbers (real + imaginary)
- decimal.Decimal: exact decimal arithmetic
- fractions.Fraction: exact rational numbers

WHY multiple numeric types?
- int: general purpose, unlimited size
- float: fast but imprecise (0.1 + 0.2 != 0.3)
- Decimal: financial calculations where precision matters
- Fraction: exact rational arithmetic

WHEN to use each:
- int: counting, indexing, IDs, bitwise operations
- float: scientific computation, graphics, performance-critical math
- Decimal: money, tax calculations, any exact decimal requirement
- Fraction: exact ratios, symbolic math
"""

# int — arbitrary precision (no overflow like in C/C++)
big_number = 10 ** 100  # googol — Python handles this natively
print(f"Googol: {big_number}")
print(f"Type: {type(big_number)}")  # <class 'int'>

# Underscore separators for readability (PEP 515)
population = 7_900_000_000
hex_color = 0xFF_AA_00
binary = 0b_1010_0011

# int operations
print(f"Floor division: 7 // 3 = {7 // 3}")   # 2
print(f"Modulo: 7 % 3 = {7 % 3}")              # 1
print(f"divmod: divmod(7, 3) = {divmod(7, 3)}")  # (2, 1)
print(f"Power: 2 ** 10 = {2 ** 10}")            # 1024

# float — IEEE 754 double precision
print(f"\n0.1 + 0.2 = {0.1 + 0.2}")  # 0.30000000000000004 — NOT exact!
print(f"0.1 + 0.2 == 0.3: {0.1 + 0.2 == 0.3}")  # False!

# BEST PRACTICE: Never compare floats with ==
import math
print(f"math.isclose(0.1 + 0.2, 0.3): {math.isclose(0.1 + 0.2, 0.3)}")  # True

# Special float values
print(f"Infinity: {float('inf')}")
print(f"NaN: {float('nan')}")
print(f"NaN == NaN: {float('nan') == float('nan')}")  # False! NaN is never equal to anything

# complex numbers
z = 3 + 4j
print(f"\nComplex: {z}")
print(f"Real: {z.real}, Imaginary: {z.imag}")
print(f"Magnitude: {abs(z)}")  # 5.0

# Decimal — exact decimal arithmetic
from decimal import Decimal, getcontext
# IMPORTANT: Create from strings, not floats!
price = Decimal("19.99")
tax = Decimal("0.0825")
total = price * (1 + tax)
print(f"\nDecimal total: {total}")  # Exact!

# BAD: Decimal(0.1) — captures the float imprecision
# GOOD: Decimal("0.1") — exact decimal representation

# Set precision
getcontext().prec = 50
print(f"Pi to 50 digits: {Decimal(1) / Decimal(7)}")

# Fraction — exact rational arithmetic
from fractions import Fraction
f1 = Fraction(1, 3)
f2 = Fraction(1, 6)
print(f"\n1/3 + 1/6 = {f1 + f2}")  # 1/2 — exact!
print(f"As float: {float(f1 + f2)}")  # 0.5


# =============================================================================
# 1.3 STRINGS
# =============================================================================
"""
WHAT: Strings are immutable sequences of Unicode characters.
      Python 3 strings are Unicode by default (unlike Python 2).

WHY strings are immutable:
- Thread-safe without locks
- Can be dictionary keys and set members (hashable)
- Enables string interning for performance

HOW to work with strings (modern Python):
- f-strings for formatting (fastest and most readable)
- Raw strings for regex and file paths
- Byte strings for binary data and network protocols
- Multiline strings for documentation

WHEN to use each string type:
- f"..." : whenever you embed variables (preferred since Python 3.6)
- r"..." : regex patterns, Windows file paths
- b"..." : binary data, network I/O, file bytes
- '''...''' : multiline text, docstrings
"""

# f-strings (formatted string literals) — the PYTHONIC way
name = "Alice"
age = 30
# PYTHONIC — clear, readable, fast
greeting = f"Hello, {name}! You are {age} years old."

# f-strings can contain ANY expression
items = [1, 2, 3]
print(f"Sum: {sum(items)}")
print(f"Uppercase: {name.upper()}")
print(f"{'centered':^20}")  # '      centered      '

# f-string formatting specs
pi = 3.14159265
print(f"Pi: {pi:.2f}")          # Pi: 3.14
print(f"Big number: {1000000:,}")  # Big number: 1,000,000
print(f"Percentage: {0.856:.1%}")  # Percentage: 85.6%
print(f"Hex: {255:#x}")         # Hex: 0xff
print(f"Debug: {name=}")        # Debug: name='Alice' (3.8+ debugging trick!)

# AVOID these older approaches:
# "Hello, %s" % name          # C-style — error-prone
# "Hello, {}".format(name)    # Verbose — use f-strings instead

# Raw strings — no escape processing
path = r"C:\Users\name\new_folder"  # Backslashes are literal
import re
pattern = r"\d{3}-\d{4}"  # Regex pattern without escaping

# Byte strings — for binary data
data = b"Hello"
print(f"Type: {type(data)}")  # <class 'bytes'>
print(f"First byte: {data[0]}")  # 72 (ASCII for 'H')

# Converting between str and bytes
text = "café"
encoded = text.encode("utf-8")  # str -> bytes
decoded = encoded.decode("utf-8")  # bytes -> str
print(f"Encoded: {encoded}")  # b'caf\xc3\xa9'

# Multiline strings
query = """
SELECT name, age
FROM users
WHERE age > 18
ORDER BY name
"""

# PYTHONIC string methods (prefer over manual logic)
text = "  Hello, World!  "
print(text.strip())        # Remove whitespace: "Hello, World!"
print(text.lower())        # "  hello, world!  "
print("hello".startswith("he"))  # True
print("hello".removeprefix("he"))  # "llo" (3.9+)
print("hello".removesuffix("lo"))  # "hel" (3.9+)

# PYTHONIC: Use join() for concatenation (NOT + in loops)
words = ["Hello", "World", "Python"]
# BAD — O(n²) because strings are immutable, creates new string each time
result = ""
for w in words:
    result += w + " "
# GOOD — O(n), joins all at once
result = " ".join(words)
print(result)  # "Hello World Python"

# String testing
print("123".isdigit())    # True
print("abc".isalpha())    # True
print("abc123".isalnum()) # True


# =============================================================================
# 1.4 BOOLEAN AND NONE
# =============================================================================
"""
WHAT:
- bool: True or False (subclass of int: True == 1, False == 0)
- None: singleton representing "no value" or "nothing"

WHY None exists:
- Signals absence of a value (like null in other languages)
- Default return value for functions that don't explicitly return
- Used as sentinel value for optional parameters

BEST PRACTICES:
- Always use `is None` / `is not None` (NEVER `== None`)
- Use `if x:` for truthiness checks (not `if x == True:`)
- Don't use mutable default arguments (use None instead)
"""

# bool is a subclass of int
print(f"True + True = {True + True}")   # 2
print(f"True * 10 = {True * 10}")       # 10
print(f"isinstance(True, int) = {isinstance(True, int)}")  # True

# None — always check with `is`
value = None
# PYTHONIC:
if value is None:
    print("No value")
# BAD — can be overridden by __eq__:
# if value == None:

# Truthiness — values that evaluate to False in boolean context
"""
Falsy values:
- None
- False
- 0, 0.0, 0j, Decimal(0), Fraction(0, 1)
- "" (empty string)
- [] (empty list)
- {} (empty dict)
- set() (empty set)
- () (empty tuple)
- range(0)
- Objects with __bool__() returning False or __len__() returning 0
"""

# PYTHONIC: Use truthiness directly
items = [1, 2, 3]
# GOOD:
if items:
    print("List has items")
# BAD (unnecessary):
# if len(items) > 0:
# if items != []:

# PYTHONIC default argument pattern
def append_to(element, target=None):
    """
    WHY: Mutable default arguments are shared across ALL calls!
    Using None and creating inside the function is the safe pattern.
    """
    if target is None:
        target = []
    target.append(element)
    return target

# Without this pattern, you get the infamous mutable default bug:
def buggy_append(element, target=[]):  # DON'T DO THIS!
    target.append(element)
    return target

print(buggy_append(1))  # [1]
print(buggy_append(2))  # [1, 2] — BUG! Same list object reused!


# =============================================================================
# 1.5 TYPE COERCION AND TRUTHINESS
# =============================================================================
"""
WHAT: Python performs implicit type conversions in certain contexts (bool context,
      numeric operations) but is generally strict about type mixing.

WHY Python is "strongly typed" but "dynamically typed":
- Strongly: "1" + 1 raises TypeError (no silent coercion like JavaScript)
- Dynamically: types are checked at runtime, not compile time

WHEN implicit conversion happens:
- In boolean contexts (if, while, and, or)
- In numeric operations (int + float -> float)
- NEVER with strings (must explicitly convert)
"""

# Numeric promotion: int -> float -> complex
result = 1 + 2.0    # int + float -> float (3.0)
result2 = 1 + 2j    # int + complex -> complex (1+2j)

# NO implicit string conversion
# "Count: " + 5  # TypeError!
print("Count: " + str(5))   # Explicit conversion
print(f"Count: {5}")         # f-string does it for you (preferred)

# Boolean context — Python calls __bool__() or __len__()
class MyContainer:
    def __init__(self, items):
        self.items = items

    def __len__(self):
        return len(self.items)

    # If __bool__ is not defined, Python falls back to __len__
    # If neither is defined, object is always truthy

container = MyContainer([])
print(f"Empty container is falsy: {not container}")  # True

# Short-circuit evaluation with `and` / `or`
# `or` returns the first truthy value (or the last value)
name = "" or "Anonymous"  # "Anonymous"
# `and` returns the first falsy value (or the last value)
result = "hello" and "world"  # "world"

# PYTHONIC: Use `or` for defaults (but be careful with falsy valid values!)
# GOOD for strings:
username = input_name or "Guest"
# DANGEROUS for numbers (0 is falsy!):
# count = user_count or 10  # BUG if user_count is legitimately 0


# =============================================================================
# 1.6 OPERATORS
# =============================================================================
"""
WHAT: Python operators are syntactic sugar for dunder methods.
      Understanding this lets you make custom classes work with operators.

KEY OPERATORS:
- Arithmetic: +, -, *, /, //, %, **, @
- Comparison: ==, !=, <, >, <=, >=
- Logical: and, or, not
- Bitwise: &, |, ^, ~, <<, >>
- Identity: is, is not
- Membership: in, not in
- Assignment: =, :=, +=, |=, etc.
- Unpacking: *, **

MODERN ADDITIONS:
- := (walrus operator, 3.8+) — assign in expressions
- | for dict merge (3.9+)
- @ for matrix multiplication (3.5+)
"""

# Walrus operator := (assignment expression) — Python 3.8+
"""
WHY: Eliminates repeated computation and nested patterns.
WHEN: Use when you need to both compute and test a value.
"""
import re

# WITHOUT walrus — compute twice or use temp variable
text = "Hello, World 123"
match = re.search(r"\d+", text)
if match:
    print(f"Found: {match.group()}")

# WITH walrus — compute once, test, and use in one expression
if (match := re.search(r"\d+", text)):
    print(f"Found: {match.group()}")

# Walrus in while loops — very common pattern
# Read lines until empty
import io
data = io.StringIO("line1\nline2\nline3\n")
while (line := data.readline()):
    print(f"Read: {line.strip()}")

# Walrus in list comprehensions — filter and transform
numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
# Get squares > 50 (compute square only once)
results = [square for n in numbers if (square := n ** 2) > 50]
print(f"Squares > 50: {results}")  # [64, 81, 100]

# BEST PRACTICE: Don't overuse walrus. Use when it clearly reduces duplication.
# BAD — walrus makes simple code confusing:
# y := f(x)  # just use y = f(x)

# Unpacking operators * and **
"""
WHY: Unpacking is one of Python's most powerful features for clean code.
WHEN: Collecting remaining items, merging collections, forwarding arguments.
"""

# * for iterable unpacking
first, *rest = [1, 2, 3, 4, 5]
print(f"first={first}, rest={rest}")  # first=1, rest=[2, 3, 4, 5]

*start, last = [1, 2, 3, 4, 5]
print(f"start={start}, last={last}")  # start=[1, 2, 3, 4], last=5

first, *middle, last = [1, 2, 3, 4, 5]
print(f"first={first}, middle={middle}, last={last}")

# * in function calls — unpack iterables as positional args
def add(a, b, c):
    return a + b + c

numbers = [1, 2, 3]
print(f"Unpacked call: {add(*numbers)}")  # 6

# ** for dict unpacking / merging
defaults = {"color": "blue", "size": 10}
overrides = {"size": 20, "weight": "bold"}

# Dict merge with ** (works in all Python 3)
merged = {**defaults, **overrides}
print(f"Merged: {merged}")  # {'color': 'blue', 'size': 20, 'weight': 'bold'}

# Dict merge with | operator (Python 3.9+) — PYTHONIC
merged = defaults | overrides  # same result, cleaner syntax
# In-place merge:
defaults |= overrides

# Identity vs Equality
"""
CRITICAL DISTINCTION:
- == calls __eq__() — tests if values are equal
- is checks if two names point to the SAME object in memory

WHEN to use `is`:
- Comparing to None: `if x is None:`
- Comparing to sentinel objects
- Checking type identity (rarely needed)

NEVER use `is` for value comparison (except None/True/False)
"""
a = [1, 2, 3]
b = [1, 2, 3]
print(f"a == b: {a == b}")  # True — same value
print(f"a is b: {a is b}")  # False — different objects!

# Small integer caching (implementation detail — don't rely on this!)
x = 256
y = 256
print(f"256 is 256: {x is y}")  # True (CPython caches -5 to 256)
x = 257
y = 257
# print(f"257 is 257: {x is y}")  # May be False! Don't rely on this.

# Membership operator `in`
# PYTHONIC: Use `in` for containment checks
if "hello" in ["hello", "world"]:
    print("Found!")

# `in` with dicts checks KEYS (not values)
config = {"debug": True, "port": 8080}
if "debug" in config:  # checks keys
    print(f"Debug mode: {config['debug']}")


# =============================================================================
# 1.7 COMMENTS AND DOCSTRINGS
# =============================================================================
"""
WHAT:
- # comments: explain WHY, not WHAT
- Docstrings: document the interface (what, parameters, return value)

BEST PRACTICES:
- Comments should explain intent and reasoning, not restate code
- Docstrings go on modules, classes, and functions
- Use Google, NumPy, or Sphinx style consistently

WHEN to comment:
- Complex algorithms or non-obvious logic
- Business rules and constraints
- Workarounds and their reasons
- TODO/FIXME with ticket numbers

WHEN NOT to comment:
- Self-explanatory code (good naming eliminates most comments)
- Obvious operations
"""

# BAD comments — stating the obvious:
# x = x + 1  # increment x

# GOOD comments — explaining WHY:
# Retry up to 3 times because the external API has transient failures
for attempt in range(3):
    pass

# Docstring styles:

# Google style (recommended for most projects)
def calculate_discount(price: float, percentage: float) -> float:
    """Calculate the discounted price.

    Applies a percentage discount to the given price. The discount
    is capped at 100% to prevent negative prices.

    Args:
        price: Original price in dollars. Must be non-negative.
        percentage: Discount percentage (0-100).

    Returns:
        The price after discount is applied.

    Raises:
        ValueError: If price is negative or percentage is out of range.

    Examples:
        >>> calculate_discount(100.0, 20.0)
        80.0
        >>> calculate_discount(50.0, 100.0)
        0.0
    """
    if price < 0:
        raise ValueError(f"Price must be non-negative, got {price}")
    if not 0 <= percentage <= 100:
        raise ValueError(f"Percentage must be 0-100, got {percentage}")
    return price * (1 - percentage / 100)


# =============================================================================
# SUMMARY: CORE FUNDAMENTALS BEST PRACTICES
# =============================================================================
"""
1. Variables are references — understand aliasing with mutable objects
2. Use f-strings for all string formatting
3. Use Decimal for money, float for science, int for counting
4. Check None with `is`, not `==`
5. Use truthiness directly: `if items:` not `if len(items) > 0:`
6. Use None as default for mutable arguments
7. Master unpacking (*, **) — it's everywhere in Python
8. Walrus := reduces duplication but don't overuse
9. Comments explain WHY, docstrings explain WHAT/HOW
10. Use type hints to document intent
"""
