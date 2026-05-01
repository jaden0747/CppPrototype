"""
=============================================================================
CHAPTER 20: MODERN PYTHON FEATURES (3.8 — 3.13+)
=============================================================================
Python evolves rapidly. Each release adds powerful features.
This chapter covers the most important additions chronologically.

=============================================================================
"""

# =============================================================================
# 20.1 PYTHON 3.8 (Oct 2019)
# =============================================================================
"""
KEY FEATURES: Walrus operator, positional-only params, f-string =
"""

# --- Walrus Operator := (Assignment Expression) ---
"""
WHAT: Assign and use a value in the same expression.
WHY: Avoid redundant computations and repetitive code.
WHEN: Filtering with a computation, while loops with input.
"""

# Before 3.8 (compute twice or use temp variable):
data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
# results = []
# for x in data:
#     y = x ** 2 + x
#     if y > 20:
#         results.append(y)

# With walrus (compute once, inline):
results = [y for x in data if (y := x ** 2 + x) > 20]
print(f"3.8 walrus: {results}")

# While loop with walrus:
import io
buffer = io.StringIO("line1\nline2\nline3\n")
while (line := buffer.readline()):
    print(f"  Read: {line.strip()}")

# --- Positional-Only Parameters (/) ---
"""
WHAT: Parameters before / can ONLY be passed positionally.
WHY: Allow renaming params without breaking callers.
     Prevent confusion when param name conflicts with kwargs.
"""
def pow(base, exp, /, mod=None):
    """base and exp are positional-only."""
    result = base ** exp
    if mod is not None:
        result %= mod
    return result

# pow(2, 10)       ✓
# pow(2, 10, 3)    ✓
# pow(base=2, exp=10)  ✗ TypeError!

# Combined: positional-only / regular / keyword-only
def combined(pos_only, /, regular, *, kw_only):
    pass
# combined(1, 2, kw_only=3)      ✓
# combined(1, regular=2, kw_only=3)  ✓

# --- f-string = for debugging ---
x = 42
y = "hello"
print(f"{x=}")       # prints: x=42
print(f"{y=}")       # prints: y='hello'
print(f"{x + 1=}")   # prints: x + 1=43


# =============================================================================
# 20.2 PYTHON 3.9 (Oct 2020)
# =============================================================================
"""
KEY FEATURES: Dict merge operators, type hint generics, string methods
"""

# --- Dict Merge Operators (| and |=) ---
defaults = {"color": "blue", "size": "M", "weight": "normal"}
overrides = {"size": "L", "style": "bold"}

# Merge (new dict, overrides wins):
merged = defaults | overrides
print(f"3.9 dict merge: {merged}")

# In-place merge:
config = {"a": 1, "b": 2}
config |= {"b": 3, "c": 4}
print(f"3.9 dict |=: {config}")

# --- Built-in Generic Types (no more typing.List, typing.Dict) ---
# Before 3.9:
# from typing import List, Dict, Tuple, Set
# def process(items: List[int]) -> Dict[str, int]: ...

# 3.9+: use built-in types directly
def process(items: list[int]) -> dict[str, int]:
    return {str(x): x for x in items}

# --- String removeprefix/removesuffix ---
filename = "test_module.py"
print(f"removeprefix: {filename.removeprefix('test_')}")   # module.py
print(f"removesuffix: {filename.removesuffix('.py')}")     # test_module

# Replaces the error-prone pattern:
# name = filename[5:] if filename.startswith("test_") else filename


# =============================================================================
# 20.3 PYTHON 3.10 (Oct 2021)
# =============================================================================
"""
KEY FEATURES: Structural Pattern Matching, better error messages, union types
"""

# --- Structural Pattern Matching (match/case) ---
"""
WHAT: Like switch/case but with destructuring and guards.
WHY: Cleaner than if/elif chains for complex dispatch.
"""

def handle_command(command):
    match command.split():
        case ["quit"]:
            return "Quitting..."
        case ["hello", name]:
            return f"Hello, {name}!"
        case ["move", direction, *rest]:
            return f"Moving {direction}, extra: {rest}"
        case ["set", key, value] if key.isalpha():
            return f"Setting {key}={value}"
        case _:
            return f"Unknown command: {command}"

print(f"3.10 match: {handle_command('hello World')}")
print(f"3.10 match: {handle_command('move north fast')}")

# Pattern matching with classes:
from dataclasses import dataclass

@dataclass
class Point:
    x: float
    y: float

@dataclass
class Circle:
    center: Point
    radius: float

def describe_shape(shape):
    match shape:
        case Point(x=0, y=0):
            return "Origin"
        case Point(x, y) if x == y:
            return f"On diagonal at {x}"
        case Point(x, y):
            return f"Point({x}, {y})"
        case Circle(center=Point(0, 0), radius=r):
            return f"Circle at origin, radius={r}"
        case _:
            return "Unknown shape"

print(f"3.10 pattern: {describe_shape(Point(3, 3))}")

# --- Union Types with | ---
# Before 3.10:
# from typing import Union, Optional
# def square(x: Union[int, float]) -> Union[int, float]: ...

# 3.10+:
def square(x: int | float) -> int | float:
    return x ** 2

# Optional equivalent:
def find(name: str) -> dict | None:
    return None

# --- Better Error Messages ---
"""
3.10+ gives much better error messages:
  - Points to exact column of error
  - Suggests fixes for NameError
  - Better messages for unclosed brackets
"""


# =============================================================================
# 20.4 PYTHON 3.11 (Oct 2022)
# =============================================================================
"""
KEY FEATURES: Exception groups, TaskGroup, 10-60% faster, tomllib
"""

# --- Exception Groups (ExceptionGroup) ---
"""
WHAT: Raise and handle MULTIPLE exceptions simultaneously.
WHY: Essential for concurrent code where multiple tasks can fail.
"""

def process_batch():
    errors = []
    for i in [1, 0, "x"]:
        try:
            result = 10 / i
        except (ZeroDivisionError, TypeError) as e:
            errors.append(e)
    if errors:
        raise ExceptionGroup("batch errors", errors)

# Handle with except*:
try:
    process_batch()
except* ZeroDivisionError as eg:
    print(f"3.11 Zero errors: {eg.exceptions}")
except* TypeError as eg:
    print(f"3.11 Type errors: {eg.exceptions}")

# --- TaskGroup (structured concurrency) ---
import asyncio

async def fetch(url):
    await asyncio.sleep(0.01)
    return f"data from {url}"

async def main_311():
    async with asyncio.TaskGroup() as tg:
        task1 = tg.create_task(fetch("url1"))
        task2 = tg.create_task(fetch("url2"))
    # All tasks guaranteed complete here
    print(f"3.11 TaskGroup: {task1.result()}, {task2.result()}")

# asyncio.run(main_311())

# --- tomllib (TOML parser, built-in) ---
import tomllib

toml_data = """
[project]
name = "myapp"
version = "1.0.0"

[project.dependencies]
requests = ">=2.28"
"""

config = tomllib.loads(toml_data)
print(f"3.11 tomllib: {config['project']['name']}")

# --- Performance: 10-60% faster than 3.10 ---
"""
CPython 3.11 introduced:
- Specializing Adaptive Interpreter
- Faster startup
- Zero-cost exceptions (try/except is free if no exception raised)
"""


# =============================================================================
# 20.5 PYTHON 3.12 (Oct 2023)
# =============================================================================
"""
KEY FEATURES: Type parameter syntax, f-string improvements, per-interpreter GIL
"""

# --- Type Parameter Syntax (PEP 695) ---
"""
New syntax for generic classes and functions.
"""

# Before 3.12:
# from typing import TypeVar, Generic
# T = TypeVar("T")
# class Stack(Generic[T]):
#     def push(self, item: T) -> None: ...
#     def pop(self) -> T: ...

# 3.12+ (much cleaner):
# class Stack[T]:
#     def push(self, item: T) -> None: ...
#     def pop(self) -> T: ...

# Generic functions:
# def first[T](items: list[T]) -> T:
#     return items[0]

# Type aliases:
# type Vector = list[float]
# type Matrix[T] = list[list[T]]

# --- F-string improvements ---
# Can now use quotes, backslashes, comments inside f-strings:
items = ["a", "b", "c"]
print(f"3.12 f-string: {", ".join(items)}")
# Before 3.12, this would be a syntax error (quotes inside f-string)

# Multiline expressions in f-strings:
value = f"{
    "hello"
    " world"
}"
print(f"3.12 multiline f-string: {value}")

# --- Per-Interpreter GIL (PEP 684) ---
"""
Each sub-interpreter can have its own GIL.
Enables true parallelism within one process.
Foundation for future free-threading Python.
(C API only in 3.12, Python API coming later)
"""

# --- Improved error messages ---
"""
Even better suggestions:
  NameError: name 'sys' is not defined. Did you forget to import 'sys'?
  Did you mean: 'self.name' (for missing attribute on class)?
"""


# =============================================================================
# 20.6 PYTHON 3.13 (Oct 2024)
# =============================================================================
"""
KEY FEATURES: Free-threading (no GIL!), JIT compiler, improved REPL
"""

# --- Free-Threading (PEP 703) — EXPERIMENTAL ---
"""
WHAT: Python WITHOUT the GIL!
      True multi-threaded parallelism for CPU-bound code.
WHY: The GIL has been Python's biggest limitation for CPU parallelism.

STATUS: Experimental in 3.13, must opt-in:
    $ python3.13t     # Free-threaded build (note the 't')

IMPLICATIONS:
- threading can now achieve true parallelism
- C extensions need updating for thread safety
- Some single-threaded code may be slightly slower
- Full stabilization expected around 3.15-3.16
"""

# --- JIT Compiler (PEP 744) — EXPERIMENTAL ---
"""
WHAT: Just-In-Time compiler for CPython.
      Compiles hot code paths to machine code at runtime.

STATUS: Experimental "copy-and-patch" JIT in 3.13.
        Not yet producing significant speedups (foundation for future).
        Expected to mature over 3.14-3.15.
"""

# --- Improved Interactive Interpreter ---
"""
WHAT: New REPL with:
  - Multi-line editing with history
  - Syntax highlighting
  - Paste mode (auto-detects pasted blocks)
  - Better tracebacks with color
"""

# --- typing.ReadOnly (PEP 705) ---
"""
from typing import ReadOnly, TypedDict

class Config(TypedDict):
    name: ReadOnly[str]     # Cannot be modified
    port: int               # Can be modified
"""

# --- Deprecation of old-style typing ---
"""
typing.Dict, typing.List, etc. emit deprecation warnings.
Use dict, list, tuple, set directly (available since 3.9).
"""


# =============================================================================
# 20.7 UPCOMING: PYTHON 3.14+ (2025+)
# =============================================================================
"""
EXPECTED FEATURES:

Python 3.14 (Oct 2025, "Pi-thon"):
- Template strings (PEP 750) — t"Hello {name}" with lazy evaluation
- Deferred evaluation of annotations (PEP 649)
- More JIT improvements
- Free-threading stabilization progress

Python 3.15+:
- Stable free-threading
- Mature JIT compiler
- Potential removal of GIL as default
"""

# --- Template Strings (PEP 750, expected 3.14) ---
"""
WHAT: New string prefix 't' for template strings.
      Unlike f-strings, templates are NOT immediately evaluated.

WHY:
- Security: prevent injection (SQL, HTML, etc.)
- Lazy evaluation (don't format until needed)
- Custom processing of template parts

# Future syntax (3.14):
name = "Alice"
greeting = t"Hello, {name}!"
# greeting is a Template object, not a string
# Can be processed, sanitized, then rendered
"""


# =============================================================================
# 20.8 VERSION MIGRATION GUIDE
# =============================================================================
"""
MINIMUM VERSION RECOMMENDATIONS (2024):
- New projects: Python 3.12+
- Existing projects: Python 3.10+ minimum
- Libraries with wide support: Python 3.9+
- End of life: 3.8 (Oct 2024)

KEY FEATURES BY VERSION:
  3.8:  walrus :=, positional-only /, f"{x=}"
  3.9:  dict |, built-in generics (list[int]), removeprefix
  3.10: match/case, int | str union, better errors
  3.11: ExceptionGroup, TaskGroup, tomllib, 10-60% faster
  3.12: type X syntax, f-string improvements, per-interpreter GIL
  3.13: free-threading (exp), JIT (exp), new REPL

UPGRADING TIPS:
1. Use pyupgrade or ruff's UP rules to modernize syntax
2. Run tests with new version before upgrading
3. Check dependency compatibility (especially C extensions)
4. Use `python -W default` to see deprecation warnings
"""


# =============================================================================
# SUMMARY: FEATURES WORTH ADOPTING IMMEDIATELY
# =============================================================================
"""
IF YOU'RE ON 3.8+:
  - Walrus operator for while loops and comprehension filtering
  - f"{x=}" for quick debugging

IF YOU'RE ON 3.9+:
  - dict | merge syntax
  - list[int] instead of typing.List[int]
  - str.removeprefix() / str.removesuffix()

IF YOU'RE ON 3.10+:
  - match/case for complex dispatch
  - int | str instead of Union[int, str]

IF YOU'RE ON 3.11+:
  - asyncio.TaskGroup for structured concurrency
  - except* for exception groups
  - tomllib for config files

IF YOU'RE ON 3.12+:
  - class Stack[T] for generics
  - type aliases: type Vector = list[float]
  - Complex f-string expressions
"""
