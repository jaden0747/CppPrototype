"""
=============================================================================
CHAPTER 7: DECORATORS & METAPROGRAMMING
=============================================================================
Decorators are one of Python's most powerful and distinctive features.
They modify or enhance functions/classes without changing their source code.

WHY decorators exist:
- Separate cross-cutting concerns (logging, auth, caching, timing)
- DRY principle: apply same behavior to many functions
- Clean, declarative syntax (@decorator before function)
- Foundation of many frameworks (Flask routes, pytest fixtures, FastAPI)

HOW decorators work:
- A decorator is a function that takes a function and returns a (modified) function
- @decorator syntax is sugar for: func = decorator(func)
- They execute at IMPORT TIME (when the module is loaded), not at call time

=============================================================================
"""

# =============================================================================
# 7.1 FUNCTION DECORATORS — BASICS
# =============================================================================
"""
WHAT: A decorator wraps a function, adding behavior before/after the call.

THE PATTERN:
    def decorator(func):
        @functools.wraps(func)  # preserve original function metadata
        def wrapper(*args, **kwargs):
            # before
            result = func(*args, **kwargs)
            # after
            return result
        return wrapper
"""

import functools
import time

# Basic decorator — timing
def timer(func):
    """Measure execution time of decorated function."""
    @functools.wraps(func)  # CRITICAL: preserves __name__, __doc__, __module__
    def wrapper(*args, **kwargs):
        start = time.perf_counter()
        result = func(*args, **kwargs)
        elapsed = time.perf_counter() - start
        print(f"{func.__name__}() took {elapsed:.4f}s")
        return result
    return wrapper

@timer
def slow_function():
    """Do something slow."""
    time.sleep(0.1)
    return "done"

# @timer is equivalent to: slow_function = timer(slow_function)
result = slow_function()
print(f"Result: {result}")
print(f"Name preserved: {slow_function.__name__}")  # 'slow_function' (thanks to @wraps)

# Logging decorator
def log_calls(func):
    """Log function calls with arguments and return value."""
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        args_repr = [repr(a) for a in args]
        kwargs_repr = [f"{k}={v!r}" for k, v in kwargs.items()]
        signature = ", ".join(args_repr + kwargs_repr)
        print(f"Calling {func.__name__}({signature})")
        result = func(*args, **kwargs)
        print(f"{func.__name__} returned {result!r}")
        return result
    return wrapper

@log_calls
def add(a, b):
    return a + b

add(3, 4)
# Output: Calling add(3, 4)
#         add returned 7


# =============================================================================
# 7.2 DECORATORS WITH ARGUMENTS
# =============================================================================
"""
WHAT: Decorators that accept configuration parameters.
      Requires an extra level of nesting (factory pattern).

WHY: Configurable decorators allow reusing the same pattern with different settings.

THE PATTERN (triple-nested):
    def decorator_factory(arg1, arg2):    # Takes decorator args
        def decorator(func):              # Takes the function
            @functools.wraps(func)
            def wrapper(*args, **kwargs):  # Takes function args
                # use arg1, arg2 here
                return func(*args, **kwargs)
            return wrapper
        return decorator
"""

# Retry decorator with configurable attempts
def retry(max_attempts: int = 3, exceptions: tuple = (Exception,)):
    """Retry a function on failure.

    Args:
        max_attempts: Maximum number of attempts.
        exceptions: Tuple of exception types to catch.
    """
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            last_exception = None
            for attempt in range(1, max_attempts + 1):
                try:
                    return func(*args, **kwargs)
                except exceptions as e:
                    last_exception = e
                    print(f"  Attempt {attempt}/{max_attempts} failed: {e}")
            raise last_exception
        return wrapper
    return decorator

@retry(max_attempts=3, exceptions=(ConnectionError, TimeoutError))
def fetch_data(url):
    """Simulate unreliable network call."""
    import random
    if random.random() < 0.7:
        raise ConnectionError("Network error")
    return f"Data from {url}"

# Rate limiter decorator
def rate_limit(calls_per_second: float):
    """Limit how frequently a function can be called."""
    min_interval = 1.0 / calls_per_second

    def decorator(func):
        last_called = [0.0]  # Mutable container for closure

        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            elapsed = time.time() - last_called[0]
            if elapsed < min_interval:
                time.sleep(min_interval - elapsed)
            last_called[0] = time.time()
            return func(*args, **kwargs)
        return wrapper
    return decorator

@rate_limit(calls_per_second=2)
def api_call(endpoint):
    return f"Response from {endpoint}"


# =============================================================================
# 7.3 CLASS DECORATORS
# =============================================================================
"""
WHAT: Decorators applied to classes. They receive the class object
      and return a (modified) class.

WHY class decorators:
- Add methods or attributes to classes
- Modify class behavior (register, validate, wrap methods)
- Alternative to metaclasses for simple cases
- Framework registration (e.g., pytest plugins, Django admin)

WHEN to use:
- Adding functionality to multiple classes uniformly
- Registration patterns (register class in a registry)
- Wrapping all methods of a class
"""

# Singleton pattern via class decorator
def singleton(cls):
    """Make a class a singleton (only one instance ever created)."""
    instances = {}

    @functools.wraps(cls, updated=[])  # Preserve class metadata
    def get_instance(*args, **kwargs):
        if cls not in instances:
            instances[cls] = cls(*args, **kwargs)
        return instances[cls]

    return get_instance

@singleton
class DatabaseConnection:
    def __init__(self, url="localhost"):
        self.url = url
        print(f"Creating connection to {url}")

# Only creates one instance
db1 = DatabaseConnection("production-db")
db2 = DatabaseConnection("other-db")  # Returns same instance!
print(f"Same object: {db1 is db2}")  # True

# Registry pattern via class decorator
class PluginRegistry:
    """Registry for auto-discovering plugins."""
    _plugins = {}

    @classmethod
    def register(cls, name: str):
        """Decorator factory that registers a class."""
        def decorator(plugin_cls):
            cls._plugins[name] = plugin_cls
            return plugin_cls
        return decorator

    @classmethod
    def get(cls, name: str):
        return cls._plugins.get(name)

    @classmethod
    def list_all(cls):
        return list(cls._plugins.keys())

@PluginRegistry.register("csv")
class CSVExporter:
    def export(self, data):
        return "CSV export"

@PluginRegistry.register("json")
class JSONExporter:
    def export(self, data):
        return "JSON export"

print(f"Plugins: {PluginRegistry.list_all()}")  # ['csv', 'json']
exporter = PluginRegistry.get("json")()
print(exporter.export([]))  # "JSON export"


# =============================================================================
# 7.4 BUILT-IN AND STANDARD LIBRARY DECORATORS
# =============================================================================
"""
WHAT: Python provides many useful decorators out of the box.

KEY BUILT-IN DECORATORS:
- @staticmethod, @classmethod, @property (covered in OOP chapter)
- @functools.wraps — preserve decorated function metadata
- @functools.lru_cache / @functools.cache — memoization
- @functools.singledispatch — function overloading by type
- @functools.total_ordering — generate comparison methods
- @contextlib.contextmanager — create context managers from generators
- @abc.abstractmethod — mark methods as abstract
- @dataclasses.dataclass — auto-generate class boilerplate
"""

# === @functools.cache / @functools.lru_cache ===
"""
WHY: Automatically cache function results. Eliminates redundant computation.
WHEN: Pure functions (same input → same output) that are called repeatedly.
"""

@functools.cache  # Python 3.9+ (unlimited cache)
def fibonacci(n: int) -> int:
    """Compute nth Fibonacci number with automatic memoization."""
    if n < 2:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

# Without cache: O(2^n), with cache: O(n)
print(f"fib(100) = {fibonacci(100)}")  # Instant!
print(f"Cache info: {fibonacci.cache_info()}")

# lru_cache — bounded cache (oldest entries evicted)
@functools.lru_cache(maxsize=128)
def expensive_computation(x, y):
    """Cache last 128 unique inputs."""
    time.sleep(0.01)  # simulate expensive work
    return x ** y

# Clear cache when needed
expensive_computation.cache_clear()

# === @functools.singledispatch ===
"""
WHY: Function overloading based on the type of the first argument.
     Python's answer to method overloading (sort of).
"""

@functools.singledispatch
def format_value(value) -> str:
    """Default implementation for unknown types."""
    return str(value)

@format_value.register(int)
def _(value: int) -> str:
    return f"{value:,}"  # Format with thousands separator

@format_value.register(float)
def _(value: float) -> str:
    return f"{value:.2f}"

@format_value.register(list)
def _(value: list) -> str:
    return f"[{', '.join(str(x) for x in value)}]"

print(format_value(1000000))    # "1,000,000"
print(format_value(3.14159))    # "3.14"
print(format_value([1, 2, 3]))  # "[1, 2, 3]"
print(format_value("hello"))    # "hello" (default)

# === @functools.total_ordering ===
"""
WHY: Define only __eq__ and ONE comparison method (__lt__),
     and get all six comparison methods auto-generated.
"""

@functools.total_ordering
class Student:
    def __init__(self, name: str, grade: float):
        self.name = name
        self.grade = grade

    def __eq__(self, other):
        return self.grade == other.grade

    def __lt__(self, other):
        return self.grade < other.grade

# Now <=, >, >= all work too!
s1 = Student("Alice", 90)
s2 = Student("Bob", 85)
print(f"Alice > Bob: {s1 > s2}")   # True
print(f"Alice >= Bob: {s1 >= s2}") # True


# =============================================================================
# 7.5 STACKING DECORATORS
# =============================================================================
"""
WHAT: Multiple decorators applied to the same function.
      They execute bottom-up (closest to function first).

HOW execution order works:
    @decorator_a
    @decorator_b
    def func():
        pass

    # Equivalent to: func = decorator_a(decorator_b(func))
    # Execution: decorator_b wraps first, decorator_a wraps second
    # Call order: decorator_a's wrapper → decorator_b's wrapper → func
"""

def bold(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        return f"<b>{func(*args, **kwargs)}</b>"
    return wrapper

def italic(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        return f"<i>{func(*args, **kwargs)}</i>"
    return wrapper

@bold
@italic
def greet(name):
    return f"Hello, {name}"

# italic wraps greet first, then bold wraps the result
print(greet("World"))  # <b><i>Hello, World</i></b>


# =============================================================================
# 7.6 METACLASSES
# =============================================================================
"""
WHAT: Metaclasses are "classes of classes." They control class creation.
      A class is an instance of its metaclass (default: `type`).

WHY metaclasses exist:
- Customize class creation (validate, modify, register)
- Framework magic (Django models, SQLAlchemy, pytest)
- Enforce invariants across all classes

WHEN to use metaclasses:
- Almost NEVER in application code
- Framework/library internals
- When class decorators and __init_subclass__ aren't sufficient

SIMPLER ALTERNATIVES (prefer these):
1. Class decorators — for modifying a class after creation
2. __init_subclass__ — for hook when a class is subclassed
3. __set_name__ — for descriptors that need to know their attribute name
"""

# __init_subclass__ — simpler alternative to metaclass (Python 3.6+)
class Validator:
    """Base class that validates subclasses have required attributes."""

    def __init_subclass__(cls, required_fields=(), **kwargs):
        """Called when a class inherits from Validator."""
        super().__init_subclass__(**kwargs)
        for field in required_fields:
            if not hasattr(cls, field):
                raise TypeError(f"{cls.__name__} must define '{field}'")

class UserForm(Validator, required_fields=("fields", "validate")):
    fields = ["name", "email"]

    def validate(self):
        return True

# This would raise TypeError at class definition time:
# class BadForm(Validator, required_fields=("fields", "validate")):
#     pass  # Missing 'fields' and 'validate'!

# Metaclass example (for understanding, rarely needed in practice)
class SingletonMeta(type):
    """Metaclass that makes classes singletons."""
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super().__call__(*args, **kwargs)
        return cls._instances[cls]

class AppConfig(metaclass=SingletonMeta):
    def __init__(self):
        self.settings = {}

# Only one instance ever
c1 = AppConfig()
c2 = AppConfig()
print(f"Same instance: {c1 is c2}")  # True


# =============================================================================
# 7.7 DESCRIPTORS
# =============================================================================
"""
WHAT: Objects that customize attribute access on OTHER objects.
      They implement __get__, __set__, and/or __delete__.

WHY descriptors matter:
- Foundation of @property, @classmethod, @staticmethod
- Reusable attribute logic (validation, type checking, lazy loading)
- More powerful than @property when you need the same logic on multiple attributes

WHEN to use:
- Same validation/logic needed on multiple attributes
- Implementing ORMs (column types)
- Lazy computation with caching
"""

class Validated:
    """Descriptor that validates values on assignment."""

    def __init__(self, validator, error_msg="Invalid value"):
        self.validator = validator
        self.error_msg = error_msg

    def __set_name__(self, owner, name):
        """Called when descriptor is assigned to class attribute."""
        self.public_name = name
        self.private_name = f"_{name}"

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        return getattr(obj, self.private_name, None)

    def __set__(self, obj, value):
        if not self.validator(value):
            raise ValueError(f"{self.public_name}: {self.error_msg} (got {value!r})")
        setattr(obj, self.private_name, value)

# Usage
class Person:
    name = Validated(
        lambda x: isinstance(x, str) and len(x) > 0,
        "must be a non-empty string"
    )
    age = Validated(
        lambda x: isinstance(x, int) and 0 <= x <= 150,
        "must be an integer between 0 and 150"
    )

    def __init__(self, name: str, age: int):
        self.name = name  # Goes through descriptor __set__
        self.age = age

p = Person("Alice", 30)
print(f"{p.name}, {p.age}")
# p.age = -1  # ValueError: age: must be an integer between 0 and 150


# =============================================================================
# SUMMARY: DECORATORS & METAPROGRAMMING BEST PRACTICES
# =============================================================================
"""
1. ALWAYS use @functools.wraps in decorator wrappers (preserves metadata)
2. Use @functools.cache for memoizing pure functions
3. Use @functools.singledispatch for type-based dispatch
4. Prefer class decorators over metaclasses
5. Prefer __init_subclass__ over metaclasses for subclass hooks
6. Use descriptors for reusable attribute validation patterns
7. Keep decorators focused — one decorator, one concern
8. Document that a function is decorated (side effects may surprise users)
9. For configurable decorators, use the factory pattern (triple nesting)
10. Test decorators separately from the functions they wrap
"""
