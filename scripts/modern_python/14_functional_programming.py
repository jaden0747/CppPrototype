"""
=============================================================================
CHAPTER 14: FUNCTIONAL PROGRAMMING
=============================================================================
Python supports functional programming as one of its paradigms.
While not a pure FP language, Python's FP features enable concise,
composable, and testable code.

WHY functional programming in Python:
- Pure functions are easier to test and reason about
- Composition creates complex behavior from simple pieces
- Immutability prevents bugs from shared state
- Declarative style expresses WHAT, not HOW
- Works great alongside OOP (Python is multi-paradigm)

KEY FP CONCEPTS:
1. First-class functions (functions as values)
2. Pure functions (no side effects)
3. Immutability (data that doesn't change)
4. Higher-order functions (functions that take/return functions)
5. Function composition

=============================================================================
"""

# =============================================================================
# 14.1 PURE FUNCTIONS
# =============================================================================
"""
WHAT: A pure function:
- Always returns the same output for the same input (deterministic)
- Has no side effects (doesn't modify external state)

WHY pure functions matter:
- Easy to test (no setup needed, just input → output)
- Easy to reason about (no hidden dependencies)
- Safe to call in parallel (no shared mutable state)
- Cacheable/memoizable (same input = same output, always)

WHEN to write pure functions:
- Data transformations
- Business logic / calculations
- Validation
- Anything that doesn't NEED side effects
"""

# PURE — no side effects, deterministic
def calculate_tax(price: float, rate: float) -> float:
    """Pure: only depends on inputs, no external state."""
    return price * rate

def filter_adults(people: list[dict]) -> list[dict]:
    """Pure: returns new list, doesn't modify input."""
    return [p for p in people if p["age"] >= 18]

# IMPURE — has side effects or depends on external state
import random
import datetime

def impure_random():
    """Impure: different output each call."""
    return random.randint(1, 100)

def impure_time():
    """Impure: depends on external state (clock)."""
    return datetime.datetime.now()

def impure_modify(items: list):
    """Impure: modifies its input (side effect)."""
    items.append("modified!")

# PYTHONIC: Separate pure logic from impure I/O
# GOOD architecture:
#   1. Read data (impure)
#   2. Transform data (pure functions)
#   3. Write results (impure)


# =============================================================================
# 14.2 HIGHER-ORDER FUNCTIONS
# =============================================================================
"""
WHAT: Functions that take functions as arguments or return functions.

WHY: Enable code reuse by parameterizing behavior.
"""

from typing import Callable, TypeVar, Iterable
T = TypeVar("T")
R = TypeVar("R")

# Function that takes a function
def apply_to_all(func: Callable[[T], R], items: Iterable[T]) -> list[R]:
    """Apply func to each item (same as map, but returns list)."""
    return [func(item) for item in items]

numbers = [1, 2, 3, 4, 5]
print(apply_to_all(lambda x: x ** 2, numbers))   # [1, 4, 9, 16, 25]
print(apply_to_all(str, numbers))                  # ['1', '2', '3', '4', '5']

# Function that returns a function
def make_power(n: int) -> Callable[[float], float]:
    """Create a function that raises to the nth power."""
    def power(x: float) -> float:
        return x ** n
    return power

square = make_power(2)
cube = make_power(3)
print(f"square(5) = {square(5)}")  # 25
print(f"cube(3) = {cube(3)}")      # 27

# Function composition
def compose(*functions):
    """Compose functions right to left: compose(f, g, h)(x) = f(g(h(x)))."""
    def composed(x):
        result = x
        for func in reversed(functions):
            result = func(result)
        return result
    return composed

# Pipeline: strip → lowercase → split
process_text = compose(
    str.split,       # Last: split into words
    str.lower,       # Middle: lowercase
    str.strip,       # First: strip whitespace
)
print(process_text("  Hello World Python  "))  # ['hello', 'world', 'python']


# =============================================================================
# 14.3 FUNCTOOLS MODULE
# =============================================================================
"""
WHAT: functools provides tools for working with functions.
      Essential for functional programming in Python.
"""

import functools
from operator import add, mul, itemgetter, attrgetter

# === functools.reduce ===
"""
WHAT: Fold/accumulate a sequence into a single value.
      reduce(f, [a, b, c, d]) = f(f(f(a, b), c), d)
"""
numbers = [1, 2, 3, 4, 5]
total = functools.reduce(add, numbers)  # ((((1+2)+3)+4)+5) = 15
product = functools.reduce(mul, numbers)  # 120

# With initial value
total_with_init = functools.reduce(add, numbers, 100)  # 115

# PYTHONIC: Prefer sum(), math.prod(), or comprehensions over reduce
# sum(numbers) is clearer than reduce(add, numbers)
# Use reduce only for non-trivial accumulations

# === functools.partial ===
"""
WHAT: Create a new function with some arguments pre-filled.
      Like currying but more flexible.

WHY: Adapt functions to interfaces that expect fewer arguments.
"""
def power(base, exponent):
    return base ** exponent

square = functools.partial(power, exponent=2)
cube = functools.partial(power, exponent=3)
print(f"square(5) = {square(5)}")  # 25
print(f"cube(3) = {cube(3)}")      # 27

# Practical: configure a generic function
import json
pretty_json = functools.partial(json.dumps, indent=2, ensure_ascii=False)
print(pretty_json({"name": "Aloïs", "age": 30}))

# === operator module ===
"""
WHAT: Function equivalents of Python operators.
      Faster than lambda for common operations.
"""
from operator import add, mul, itemgetter, attrgetter, methodcaller

# itemgetter — get items by index/key (faster than lambda)
pairs = [(1, 'b'), (2, 'a'), (3, 'c')]
sorted_by_second = sorted(pairs, key=itemgetter(1))
print(f"Sorted by 2nd: {sorted_by_second}")

# Multiple keys
data = [{"name": "Alice", "age": 30}, {"name": "Bob", "age": 25}]
get_name_age = itemgetter("name", "age")
print(get_name_age(data[0]))  # ('Alice', 30)

# attrgetter — get attributes from objects
from collections import namedtuple
Point = namedtuple("Point", ["x", "y"])
points = [Point(3, 4), Point(1, 2), Point(5, 0)]
sorted_by_x = sorted(points, key=attrgetter("x"))
print(f"By x: {sorted_by_x}")

# methodcaller — call a method
words = ["hello", "WORLD", "Python"]
upper_words = list(map(methodcaller("upper"), words))
print(f"Upper: {upper_words}")

# === functools.singledispatch ===
"""
Function overloading based on first argument type.
(Covered in chapter 7 Decorators)
"""


# =============================================================================
# 14.4 IMMUTABILITY PATTERNS
# =============================================================================
"""
WHAT: Working with data that doesn't change after creation.

WHY immutability:
- No aliasing bugs (safe to share references)
- Thread-safe without locks
- Easier to reason about (value never changes under you)
- Enables caching (hashable)

HOW to achieve immutability in Python:
- Use tuples instead of lists (where possible)
- Use frozenset instead of set
- Use @dataclass(frozen=True)
- Return new objects instead of modifying in place
"""

from dataclasses import dataclass

# Frozen dataclass — immutable record
@dataclass(frozen=True)
class Point:
    x: float
    y: float

    def moved(self, dx: float, dy: float) -> "Point":
        """Return NEW point (don't modify self)."""
        return Point(self.x + dx, self.y + dy)

p1 = Point(1, 2)
p2 = p1.moved(3, 4)  # New object
print(f"p1={p1}, p2={p2}")  # p1 unchanged!
# p1.x = 10  # FrozenInstanceError!

# Immutable transformations on collections
def remove_item(items: tuple, item) -> tuple:
    """Return new tuple without item (original unchanged)."""
    return tuple(x for x in items if x != item)

original = (1, 2, 3, 4, 5)
without_3 = remove_item(original, 3)
print(f"Original: {original}")   # (1, 2, 3, 4, 5) — unchanged
print(f"Without 3: {without_3}")  # (1, 2, 4, 5)

# Dict immutable update pattern
config = {"host": "localhost", "port": 8080}
# Create new dict with update (don't modify original)
new_config = {**config, "port": 9090, "debug": True}
print(f"Original: {config}")     # unchanged
print(f"New: {new_config}")


# =============================================================================
# 14.5 FUNCTION PIPELINES
# =============================================================================
"""
WHAT: Chain transformations together into a readable pipeline.

WHY pipelines:
- Each step is simple and testable
- Easy to add/remove/reorder steps
- Self-documenting (reads top to bottom)
"""

# Pipeline pattern using function composition
def pipeline(*functions):
    """Create a pipeline: apply functions left to right."""
    def pipe(data):
        result = data
        for func in functions:
            result = func(result)
        return result
    return pipe

# Data processing pipeline
def normalize(text: str) -> str:
    return text.strip().lower()

def remove_punctuation(text: str) -> str:
    import string
    return text.translate(str.maketrans("", "", string.punctuation))

def tokenize(text: str) -> list[str]:
    return text.split()

def remove_stopwords(words: list[str]) -> list[str]:
    stopwords = {"the", "a", "an", "is", "are", "was", "were", "in", "on", "at"}
    return [w for w in words if w not in stopwords]

# Build pipeline
text_processor = pipeline(
    normalize,
    remove_punctuation,
    tokenize,
    remove_stopwords,
)

text = "  The Quick Brown Fox, IS jumping! On the lazy dog.  "
result = text_processor(text)
print(f"Processed: {result}")
# ['quick', 'brown', 'fox', 'jumping', 'lazy', 'dog']

# Alternative: Method chaining (common in pandas, Django QuerySets)
# queryset.filter(active=True).exclude(role="admin").order_by("name")[:10]


# =============================================================================
# 14.6 PATTERN MATCHING AS FUNCTIONAL DECOMPOSITION
# =============================================================================
"""
WHAT: Use match/case for type-based dispatch (functional style).
      Replaces visitor pattern and isinstance chains.
"""

@dataclass(frozen=True)
class Add:
    left: "Expr"
    right: "Expr"

@dataclass(frozen=True)
class Mul:
    left: "Expr"
    right: "Expr"

@dataclass(frozen=True)
class Num:
    value: float

Expr = Add | Mul | Num

def evaluate(expr: Expr) -> float:
    """Evaluate expression tree (functional decomposition)."""
    match expr:
        case Num(value=v):
            return v
        case Add(left=l, right=r):
            return evaluate(l) + evaluate(r)
        case Mul(left=l, right=r):
            return evaluate(l) * evaluate(r)

# Build expression: (2 + 3) * 4
expr = Mul(Add(Num(2), Num(3)), Num(4))
print(f"(2 + 3) * 4 = {evaluate(expr)}")  # 20.0


# =============================================================================
# SUMMARY: FUNCTIONAL PROGRAMMING BEST PRACTICES
# =============================================================================
"""
1. Write pure functions for core logic (no side effects)
2. Separate I/O from computation (read → transform → write)
3. Return new data instead of modifying arguments
4. Use functools.partial for pre-configuring functions
5. Use operator module instead of trivial lambdas
6. Use frozen dataclasses for immutable data structures
7. Compose small functions into pipelines
8. Use comprehensions and generators for data transformations
9. Use reduce only when sum/prod/max aren't applicable
10. Mix FP with OOP — Python is multi-paradigm, use the best tool for each job
"""
