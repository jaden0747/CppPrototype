"""
=============================================================================
CHAPTER 4: FUNCTIONS
=============================================================================
Functions are the primary unit of code organization in Python.
Python treats functions as first-class objects — they can be passed around,
stored in variables, and composed together.

WHY functions matter so much in Python:
- First-class citizens (objects like any other)
- Closures enable powerful patterns (decorators, factories)
- Flexible argument handling (*args, **kwargs, /, *)
- Type hints make interfaces clear without runtime cost

=============================================================================
"""

# =============================================================================
# 4.1 DEFINING FUNCTIONS
# =============================================================================
"""
WHAT: `def` creates a function object and binds it to a name.
      Functions are objects — they have attributes and can be inspected.

PYTHONIC PRINCIPLES:
- Functions should do ONE thing (Single Responsibility)
- Functions should be short (if you need a scroll bar, it's too long)
- Name functions as verbs: get_user(), calculate_tax(), is_valid()
- Return early for error cases
"""

def greet(name):
    """Return a greeting string."""
    return f"Hello, {name}!"

# Functions are objects
print(type(greet))        # <class 'function'>
print(greet.__name__)     # 'greet'
print(greet.__doc__)      # 'Return a greeting string.'

# Assign to variable, pass around
say_hi = greet
print(say_hi("World"))    # "Hello, World!"

# Functions without explicit return → return None
def do_nothing():
    pass

result = do_nothing()
print(result)  # None

# Multiple return values (actually returns a tuple)
def min_max(numbers):
    return min(numbers), max(numbers)

lo, hi = min_max([3, 1, 4, 1, 5])
print(f"min={lo}, max={hi}")


# =============================================================================
# 4.2 ARGUMENTS
# =============================================================================
"""
WHAT: Python has the most flexible argument system of any mainstream language.

TYPES OF ARGUMENTS:
1. Positional arguments
2. Keyword arguments
3. Default values
4. *args (variable positional)
5. **kwargs (variable keyword)
6. Positional-only (before /)
7. Keyword-only (after *)

WHY this flexibility exists:
- Enables both simple calls and complex configurations
- Allows APIs to evolve without breaking callers
- Supports decorator patterns (forwarding all args)
"""

# === BASIC ARGUMENTS ===
def power(base, exponent=2):
    """
    `exponent` has a default value — it's optional.
    Default values are evaluated ONCE at function definition time!
    """
    return base ** exponent

print(power(3))       # 9 (exponent defaults to 2)
print(power(3, 3))    # 27 (positional)
print(power(3, exponent=3))  # 27 (keyword)

# === *args — VARIABLE POSITIONAL ===
"""
WHAT: *args collects extra positional arguments into a tuple.
WHEN: Function accepts any number of positional arguments.
"""
def sum_all(*args):
    """Sum any number of arguments."""
    print(f"args type: {type(args)}")  # <class 'tuple'>
    return sum(args)

print(sum_all(1, 2, 3, 4))  # 10

# === **kwargs — VARIABLE KEYWORD ===
"""
WHAT: **kwargs collects extra keyword arguments into a dict.
WHEN: Function accepts any named configuration options.
"""
def create_user(name, **kwargs):
    """Create user with optional extra fields."""
    user = {"name": name}
    user.update(kwargs)
    return user

user = create_user("Alice", age=30, city="NYC", role="admin")
print(user)  # {'name': 'Alice', 'age': 30, 'city': 'NYC', 'role': 'admin'}

# === COMBINING ALL ARGUMENT TYPES ===
def full_example(pos1, pos2, /, normal, *, kw_only, **kwargs):
    """
    pos1, pos2: positional-only (before /)
    normal: either positional or keyword
    kw_only: keyword-only (after *)
    **kwargs: catch-all for extra keywords
    """
    print(f"pos1={pos1}, pos2={pos2}")
    print(f"normal={normal}")
    print(f"kw_only={kw_only}")
    print(f"kwargs={kwargs}")

full_example(1, 2, 3, kw_only="hello", extra="world")
# full_example(pos1=1, ...)  # TypeError! pos1 is positional-only


# =============================================================================
# 4.3 POSITIONAL-ONLY AND KEYWORD-ONLY PARAMETERS
# =============================================================================
"""
WHAT:
- / separates positional-only params (before /) from regular params
- * separates regular params from keyword-only params (after *)

WHY positional-only (/):
- Allows renaming parameters without breaking callers
- Prevents callers from depending on parameter names
- Used in CPython builtins: len(obj) — you can't write len(obj=x)

WHY keyword-only (*):
- Forces clarity at call site for confusing parameters
- Prevents accidental positional usage of boolean flags
- Self-documenting API

WHEN to use:
- / for utility functions where param names are implementation details
- * for boolean flags, optional config that needs explicit naming
"""

# Positional-only — caller CANNOT use keyword syntax for these
def div(a, b, /):
    """a and b must be passed positionally."""
    return a / b

print(div(10, 3))   # OK
# div(a=10, b=3)    # TypeError!

# Keyword-only — caller MUST use keyword syntax for these
def connect(host, port, *, timeout=30, use_ssl=True):
    """timeout and use_ssl must be passed as keywords."""
    print(f"Connecting to {host}:{port} (timeout={timeout}, ssl={use_ssl})")

connect("localhost", 8080, timeout=60, use_ssl=False)  # OK
# connect("localhost", 8080, 60, False)  # TypeError!

# BEST PRACTICE: Force keyword for boolean/unclear params
# BAD — what does True mean here?
# process_data(data, True, False, True)

# GOOD — crystal clear:
# process_data(data, normalize=True, verbose=False, cache=True)


# =============================================================================
# 4.4 LAMBDA FUNCTIONS
# =============================================================================
"""
WHAT: Anonymous, single-expression functions.
      Syntax: lambda args: expression

WHY lambdas exist:
- Short callbacks where a full `def` is overkill
- Especially with sorted(), map(), filter(), key functions

WHEN to use:
- Simple one-line expressions as arguments to higher-order functions
- When the function is trivial and naming it adds no clarity

WHEN NOT to use:
- Complex logic (use def instead — it's more readable)
- When you'd assign lambda to a variable (just use def!)
"""

# GOOD use: sorting key
pairs = [(1, "one"), (3, "three"), (2, "two")]
pairs_sorted = sorted(pairs, key=lambda p: p[1])  # sort by second element
print(pairs_sorted)

# GOOD use: simple callback
from functools import reduce
product = reduce(lambda a, b: a * b, [1, 2, 3, 4, 5])
print(f"Product: {product}")  # 120

# BAD — don't assign lambda to a variable:
# BAD:
# double = lambda x: x * 2
# GOOD:
def double(x):
    return x * 2

# WHY? Because:
# 1. `def` gives the function a proper __name__ for debugging
# 2. `def` supports docstrings
# 3. lambda assigned to variable provides no advantage over def


# =============================================================================
# 4.5 CLOSURES AND NONLOCAL
# =============================================================================
"""
WHAT: A closure is a function that remembers values from its enclosing scope,
      even after that scope has finished executing.

WHY closures matter:
- Enable factory functions (functions that create functions)
- Foundation of decorators
- State without classes (lightweight alternative)
- Data hiding (encapsulation without class)

HOW closures work:
- Inner function references a variable from outer function
- Python "captures" that variable (stores it in __closure__)
- The variable lives as long as the inner function exists
"""

# Basic closure — function factory
def make_multiplier(factor):
    """Create a function that multiplies by a fixed factor."""
    def multiply(x):
        return x * factor  # `factor` is captured from enclosing scope
    return multiply

double = make_multiplier(2)
triple = make_multiplier(3)
print(f"double(5) = {double(5)}")  # 10
print(f"triple(5) = {triple(5)}")  # 15

# Inspecting the closure
print(f"Closure vars: {double.__closure__[0].cell_contents}")  # 2

# === NONLOCAL ===
"""
WHAT: `nonlocal` lets inner functions MODIFY variables in enclosing scope.
      Without `nonlocal`, assignment creates a new local variable.

WHEN to use:
- Counters in closures
- State that needs to be modified by the inner function
"""

def make_counter(start=0):
    """Create a counter function with internal state."""
    count = start

    def increment():
        nonlocal count  # Without this, `count = count + 1` would be UnboundLocalError
        count += 1
        return count

    def get():
        return count

    def reset():
        nonlocal count
        count = start

    # Return multiple functions sharing the same state
    return increment, get, reset

inc, get, reset = make_counter(0)
print(inc())   # 1
print(inc())   # 2
print(inc())   # 3
print(get())   # 3
reset()
print(get())   # 0

# Common pitfall: closure variable binding in loops
# BAD — all functions capture the SAME variable `i`:
functions = []
for i in range(5):
    functions.append(lambda: i)
print([f() for f in functions])  # [4, 4, 4, 4, 4] — all see final i=4!

# FIX — use default argument to capture current value:
functions = []
for i in range(5):
    functions.append(lambda i=i: i)  # default arg captures current i
print([f() for f in functions])  # [0, 1, 2, 3, 4] — correct!


# =============================================================================
# 4.6 RECURSION
# =============================================================================
"""
WHAT: Functions calling themselves to solve problems that have
      recursive structure (trees, fractals, divide-and-conquer).

PYTHON LIMITATIONS:
- Default recursion limit: 1000 (sys.setrecursionlimit to change)
- No tail-call optimization (Python doesn't support it)
- Deep recursion → use iteration or explicit stack instead

WHEN to use recursion:
- Tree traversal
- Problems with natural recursive structure (factorial, fibonacci, etc.)
- When clarity matters more than performance

WHEN NOT to use:
- Linear problems with deep recursion (use iteration)
- Performance-critical code (function call overhead)
"""

# Classic recursion
def factorial(n):
    if n <= 1:
        return 1
    return n * factorial(n - 1)

# PYTHONIC: Use itertools or math for built-in operations
import math
print(f"factorial(10) = {math.factorial(10)}")  # Use stdlib!

# Tree recursal — where recursion shines
def tree_depth(node):
    """Find depth of a tree."""
    if node is None:
        return 0
    return 1 + max(tree_depth(node.get("left")), tree_depth(node.get("right")))

tree = {
    "val": 1,
    "left": {"val": 2, "left": None, "right": None},
    "right": {"val": 3, "left": {"val": 4, "left": None, "right": None}, "right": None}
}
print(f"Tree depth: {tree_depth(tree)}")  # 3

# Converting recursion to iteration (for deep problems)
def factorial_iterative(n):
    """Iterative version — no stack overflow risk."""
    result = 1
    for i in range(2, n + 1):
        result *= i
    return result


# =============================================================================
# 4.7 FUNCTION ANNOTATIONS AND TYPE HINTS
# =============================================================================
"""
WHAT: Type hints document expected types. NOT enforced at runtime.
      They're for tools (mypy, IDEs) and documentation.

WHY use type hints:
- Self-documenting code
- IDE autocomplete and error detection
- Static analysis catches bugs before runtime
- Makes refactoring safer

WHEN to add type hints:
- Public API functions (always)
- Complex internal functions
- When types aren't obvious from context
- Skip for trivial local helper functions

HOW: Annotations are stored in __annotations__ but have NO runtime effect.
"""

def greeting(name: str) -> str:
    """Type-annotated function."""
    return f"Hello, {name}"

# Complex types
from typing import Optional, Union

def find_user(user_id: int) -> Optional[dict]:
    """Return user dict or None if not found."""
    users = {1: {"name": "Alice"}, 2: {"name": "Bob"}}
    return users.get(user_id)

# Modern syntax (3.10+): use | instead of Union/Optional
def find_user_modern(user_id: int) -> dict | None:
    """Same as above, modern syntax."""
    pass

# Callable type hints
from typing import Callable

def apply_operation(
    x: float,
    y: float,
    operation: Callable[[float, float], float]
) -> float:
    """Apply a binary operation to two numbers."""
    return operation(x, y)

result = apply_operation(3, 4, lambda a, b: a + b)


# =============================================================================
# 4.8 FIRST-CLASS FUNCTIONS AND HIGHER-ORDER FUNCTIONS
# =============================================================================
"""
WHAT:
- First-class: functions are objects, can be stored/passed/returned
- Higher-order: functions that take or return other functions

WHY this matters:
- Enables functional programming patterns
- Foundation for decorators
- Allows strategy pattern without classes
- Clean callback APIs

COMMON HIGHER-ORDER FUNCTIONS:
- map(func, iterable): apply func to each element
- filter(func, iterable): keep elements where func returns True
- sorted(iterable, key=func): sort using func for comparison key
- functools.reduce(func, iterable): fold/accumulate
"""

# Functions as arguments (strategy pattern)
def process_data(data, transformer):
    """Apply any transformation function to data."""
    return [transformer(item) for item in data]

numbers = [1, 2, 3, 4, 5]
print(process_data(numbers, lambda x: x ** 2))    # squares
print(process_data(numbers, lambda x: x * 10))    # multiply by 10
print(process_data(numbers, str))                  # convert to strings

# Functions as return values (factory pattern)
def make_validator(min_val, max_val):
    """Create a validation function with specific bounds."""
    def validate(value):
        return min_val <= value <= max_val
    return validate

is_valid_age = make_validator(0, 150)
is_valid_score = make_validator(0, 100)
print(f"Age 25 valid: {is_valid_age(25)}")      # True
print(f"Score 150 valid: {is_valid_score(150)}")  # False

# Storing functions in data structures
operations = {
    "+": lambda a, b: a + b,
    "-": lambda a, b: a - b,
    "*": lambda a, b: a * b,
    "/": lambda a, b: a / b,
}

def calculate(expression):
    """Simple calculator using function dispatch."""
    parts = expression.split()
    a, op, b = float(parts[0]), parts[1], float(parts[2])
    return operations[op](a, b)

print(f"3 + 4 = {calculate('3 + 4')}")    # 7.0
print(f"10 / 3 = {calculate('10 / 3')}")  # 3.333...

# PYTHONIC: Use functions in data structures instead of long if/elif chains
# BAD:
# if op == "+": return a + b
# elif op == "-": return a - b
# elif op == "*": return a * b

# GOOD: dispatch dictionary (shown above)


# =============================================================================
# SUMMARY: FUNCTION BEST PRACTICES
# =============================================================================
"""
1. Functions should do ONE thing — if name has "and", split it
2. Use type hints for public APIs
3. Prefer keyword-only (*) for boolean/config parameters
4. Never use mutable default arguments (use None instead)
5. Use closures for lightweight state (instead of single-method classes)
6. Prefer named functions over lambda for anything non-trivial
7. Return early for error cases (guard clauses)
8. Use *args/**kwargs for forwarding (decorators, wrappers)
9. Document with docstrings (Google style recommended)
10. Use dispatch dicts instead of long if/elif chains for function selection
"""
