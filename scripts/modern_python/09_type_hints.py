"""
=============================================================================
CHAPTER 9: TYPE HINTS & STATIC TYPING
=============================================================================
Type hints make Python code self-documenting and enable static analysis tools
(mypy, pyright) to catch bugs before runtime.

WHY type hints:
- Documentation that doesn't go stale (checked by tools)
- IDE autocomplete and refactoring support
- Catch bugs at "compile time" (before running)
- Makes code easier to understand for new developers
- Enables better tooling (auto-generated docs, serialization)

KEY PRINCIPLE: Type hints are OPTIONAL and have NO runtime effect.
They're checked by external tools (mypy, pyright), not by Python itself.

WHEN to add type hints:
- Public API functions (always)
- Complex return types
- Function signatures that aren't obvious
- Library code (enables users to get IDE support)

WHEN to skip:
- Simple local variables where type is obvious
- Tests (often more verbose than helpful)
- Prototype/throwaway code

=============================================================================
"""

# =============================================================================
# 9.1 BASIC TYPE ANNOTATIONS
# =============================================================================
"""
WHAT: Annotate variable types, function parameters, and return types.
"""

# Variable annotations
name: str = "Alice"
age: int = 30
is_active: bool = True
scores: list[int] = [95, 87, 92]        # Python 3.9+ built-in generics
metadata: dict[str, str] = {"role": "admin"}

# Function annotations
def greet(name: str, excited: bool = False) -> str:
    """Parameters and return type annotated."""
    greeting = f"Hello, {name}!"
    if excited:
        greeting = greeting.upper()
    return greeting

# Multiple return types (returning tuple)
def min_max(numbers: list[int]) -> tuple[int, int]:
    return min(numbers), max(numbers)


# =============================================================================
# 9.2 OPTIONAL, UNION, AND | SYNTAX
# =============================================================================
"""
WHAT:
- Optional[X] = X | None (value that might be None)
- Union[X, Y] = X | Y (value that can be either type)
- The | syntax (Python 3.10+) replaces both

WHEN to use Optional:
- Function might return None (not found, no result)
- Parameter that can be omitted (defaults to None)
"""

from typing import Optional, Union

# Before Python 3.10:
def find_user(user_id: int) -> Optional[dict]:
    """Return user or None."""
    users = {1: {"name": "Alice"}}
    return users.get(user_id)

def process(value: Union[str, int]) -> str:
    """Accept string or int."""
    return str(value)

# Python 3.10+ — cleaner | syntax:
def find_user_modern(user_id: int) -> dict | None:
    """Same as Optional[dict]."""
    users = {1: {"name": "Alice"}}
    return users.get(user_id)

def process_modern(value: str | int | float) -> str:
    """Multiple types with | syntax."""
    return str(value)


# =============================================================================
# 9.3 TYPE ALIASES
# =============================================================================
"""
WHAT: Give complex types a readable name.

HOW:
- Python 3.9 and earlier: TypeAlias or simple assignment
- Python 3.12+: `type` statement (preferred)
"""

from typing import TypeAlias

# Python 3.9+
UserId = int  # Simple alias
Coordinates = tuple[float, float]
Matrix = list[list[float]]

# Explicit TypeAlias (clearer intent)
UserDict: TypeAlias = dict[str, str | int | bool]
Callback: TypeAlias = "Callable[[int, int], int]"

# Python 3.12+ — the `type` statement
# type Vector = list[float]
# type UserID = int
# type JSON = dict[str, "JSON"] | list["JSON"] | str | int | float | bool | None

# Usage
def create_user(data: UserDict) -> UserId:
    return 1

def distance(p1: Coordinates, p2: Coordinates) -> float:
    return ((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2) ** 0.5


# =============================================================================
# 9.4 GENERICS (TypeVar, Generic, ParamSpec)
# =============================================================================
"""
WHAT: Generics let you write functions/classes that work with ANY type
      while preserving type information.

WHY: Without generics, you'd have to use `Any` and lose type safety.

HOW: TypeVar creates a "type variable" that gets resolved at call site.
"""

from typing import TypeVar, Generic, Sequence

# TypeVar — constrained type variable
T = TypeVar("T")  # Can be any type

def first(items: Sequence[T]) -> T:
    """Return first element, preserving its type."""
    return items[0]

# Type checker knows the return type:
result_int = first([1, 2, 3])      # Type: int
result_str = first(["a", "b"])     # Type: str

# Bounded TypeVar — restrict to specific types
Number = TypeVar("Number", int, float)

def double(x: Number) -> Number:
    return x * 2

# Generic classes
class Stack(Generic[T]):
    """A typed stack implementation."""

    def __init__(self) -> None:
        self._items: list[T] = []

    def push(self, item: T) -> None:
        self._items.append(item)

    def pop(self) -> T:
        return self._items.pop()

    def peek(self) -> T:
        return self._items[-1]

    def is_empty(self) -> bool:
        return len(self._items) == 0

# Usage with specific type
int_stack: Stack[int] = Stack()
int_stack.push(42)
# int_stack.push("hello")  # Type error!

str_stack: Stack[str] = Stack()
str_stack.push("hello")

# Python 3.12+ syntax for generics (much cleaner):
# class Stack[T]:
#     def __init__(self) -> None:
#         self._items: list[T] = []
#     def push(self, item: T) -> None:
#         self._items.append(item)
#     def pop(self) -> T:
#         return self._items.pop()


# =============================================================================
# 9.5 LITERAL, FINAL, CLASSVAR
# =============================================================================
"""
WHAT: Special typing constructs for precise type constraints.

- Literal[values]: restricts to specific values
- Final: marks a variable/method as non-reassignable/non-overridable
- ClassVar: marks a class-level variable (not instance)
"""

from typing import Literal, Final, ClassVar

# Literal — restrict to specific values
def set_direction(direction: Literal["north", "south", "east", "west"]) -> None:
    """Only accepts these exact string values."""
    print(f"Moving {direction}")

set_direction("north")  # OK
# set_direction("up")  # Type error!

# Useful for mode parameters
def open_file(path: str, mode: Literal["r", "w", "a", "rb", "wb"]) -> None:
    pass

# Final — constant declaration
MAX_RETRIES: Final = 3
API_URL: Final[str] = "https://api.example.com"
# MAX_RETRIES = 5  # Type error: cannot reassign Final

# ClassVar — class-level (not instance) attributes
from dataclasses import dataclass

@dataclass
class Config:
    instances: ClassVar[list["Config"]] = []  # Shared across all instances
    name: str  # Instance attribute

    def __post_init__(self):
        Config.instances.append(self)


# =============================================================================
# 9.6 TYPEDDICT
# =============================================================================
"""
WHAT: Type hint for dictionaries with specific string keys and typed values.
      Unlike a regular dict type, each key can have a DIFFERENT value type.

WHY: JSON/API responses often have fixed structure with heterogeneous values.
WHEN: Typing JSON-like dicts, config dicts, or any fixed-key mapping.
"""

from typing import TypedDict, Required, NotRequired

class UserProfile(TypedDict):
    """Dictionary with specific key-value types."""
    name: str
    age: int
    email: str
    is_admin: bool

# Usage
user: UserProfile = {
    "name": "Alice",
    "age": 30,
    "email": "alice@example.com",
    "is_admin": False,
}

# With optional keys (Python 3.11+):
class APIResponse(TypedDict, total=False):
    """All keys are optional (total=False)."""
    data: list[dict]
    error: str
    page: int

# Mix required and optional:
class Config(TypedDict):
    host: str                    # Required
    port: int                    # Required
    debug: NotRequired[bool]     # Optional (3.11+)
    timeout: NotRequired[float]  # Optional


# =============================================================================
# 9.7 PROTOCOL (Structural Subtyping)
# =============================================================================
"""
WHAT: Define interfaces based on structure, not inheritance.
      "If it has read() and write(), it's a stream" — no inheritance needed.

WHY Protocol is revolutionary for Python:
- Formalizes duck typing for static analysis
- Works with existing code (no modification needed)
- Replaces many uses of ABCs without coupling
"""

from typing import Protocol, runtime_checkable

@runtime_checkable
class Closeable(Protocol):
    """Anything with a close() method."""
    def close(self) -> None: ...

class DatabaseConn:
    """Not inheriting from Closeable, but satisfies the protocol."""
    def close(self) -> None:
        print("DB closed")

class FileHandle:
    """Also satisfies Closeable."""
    def close(self) -> None:
        print("File closed")

def cleanup(resource: Closeable) -> None:
    """Accepts anything with close()."""
    resource.close()

cleanup(DatabaseConn())  # OK!
cleanup(FileHandle())    # OK!

# More complex protocol
class Comparable(Protocol):
    """Objects that support < comparison."""
    def __lt__(self, other: "Comparable") -> bool: ...

def find_min(items: list[Comparable]) -> Comparable:
    """Works with any type that supports <."""
    return min(items)


# =============================================================================
# 9.8 CALLABLE TYPES
# =============================================================================
"""
WHAT: Type hints for function objects (callbacks, handlers, decorators).

Syntax: Callable[[arg_types], return_type]
"""

from typing import Callable, ParamSpec, Concatenate
from collections.abc import Callable as ABCCallable

# Basic Callable
def apply(func: Callable[[int, int], int], a: int, b: int) -> int:
    return func(a, b)

result = apply(lambda x, y: x + y, 3, 4)

# Callable with no arguments
NoArgCallback = Callable[[], None]

def register_callback(cb: NoArgCallback) -> None:
    cb()

# ParamSpec — preserve function signatures in decorators (Python 3.10+)
P = ParamSpec("P")
R = TypeVar("R")

def logged(func: Callable[P, R]) -> Callable[P, R]:
    """Decorator that preserves exact function signature for type checkers."""
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> R:
        print(f"Calling {func.__name__}")
        return func(*args, **kwargs)
    return wrapper

@logged
def add(a: int, b: int) -> int:
    return a + b

# Type checker knows add still takes (int, int) -> int


# =============================================================================
# 9.9 TYPEGUARD AND TYPE NARROWING
# =============================================================================
"""
WHAT: TypeGuard tells the type checker that a function narrows a type.
      After calling a TypeGuard function, the type checker knows the specific type.

WHY: isinstance() narrows types automatically, but custom checks don't.
     TypeGuard bridges this gap.
"""

from typing import TypeGuard, Any

def is_list_of_strings(val: list[Any]) -> TypeGuard[list[str]]:
    """If this returns True, type checker knows val is list[str]."""
    return all(isinstance(x, str) for x in val)

def process_data(data: list[Any]) -> None:
    if is_list_of_strings(data):
        # Type checker now knows data is list[str] in this branch
        for s in data:
            print(s.upper())  # No type error — s is known to be str


# =============================================================================
# 9.10 SELF TYPE (Python 3.11+)
# =============================================================================
"""
WHAT: `Self` refers to the current class type. Essential for methods that
      return the same type (fluent interfaces, builders, classmethods).

WHY: Before Self, you had to use TypeVar bound to the class, which was verbose.
"""

from typing import Self

class Builder:
    """Fluent builder pattern with proper typing."""

    def __init__(self) -> None:
        self.name: str = ""
        self.age: int = 0

    def set_name(self, name: str) -> Self:
        """Returns Self — works correctly with subclasses."""
        self.name = name
        return self

    def set_age(self, age: int) -> Self:
        self.age = age
        return self

# Chaining works with proper types
person = Builder().set_name("Alice").set_age(30)


# =============================================================================
# 9.11 ANNOTATED TYPE
# =============================================================================
"""
WHAT: Attach metadata to types. Used by frameworks (Pydantic, FastAPI, etc.)
      for validation, documentation, and runtime behavior.

WHY: Type hints carry information beyond just the type — constraints,
     descriptions, format specifications.
"""

from typing import Annotated

# Basic annotations (metadata ignored by type checker, used by frameworks)
Username = Annotated[str, "Must be 3-20 characters"]
Port = Annotated[int, "Must be between 1 and 65535"]
Email = Annotated[str, "Must be valid email format"]

def create_account(username: Username, email: Email, port: Port = 8080) -> None:
    pass

# In practice, used with validators like Pydantic:
# from pydantic import Field
# class User(BaseModel):
#     name: Annotated[str, Field(min_length=1, max_length=50)]
#     age: Annotated[int, Field(ge=0, le=150)]


# =============================================================================
# 9.12 USING MYPY / PYRIGHT
# =============================================================================
"""
WHAT: Static type checkers that analyze your code without running it.

mypy: The original, most widely used. pip install mypy
pyright: Microsoft's type checker, faster, stricter. Used by Pylance in VS Code.

HOW TO RUN:
    $ mypy my_module.py
    $ mypy --strict my_module.py     # Maximum strictness
    $ pyright my_module.py

CONFIGURATION (pyproject.toml):
    [tool.mypy]
    python_version = "3.12"
    strict = true
    warn_return_any = true
    warn_unused_configs = true

    [tool.pyright]
    pythonVersion = "3.12"
    typeCheckingMode = "strict"

GRADUAL TYPING STRATEGY:
1. Start with no type hints (dynamic Python)
2. Add hints to new code and public APIs
3. Use `# type: ignore` for legacy code that's hard to type
4. Increase strictness over time
5. Goal: fully typed public API, mostly typed internals

COMMON PATTERNS:
"""

# Ignoring specific lines
# result = untyped_library.do_thing()  # type: ignore[no-untyped-call]

# Overload — different return types based on input
from typing import overload

@overload
def parse(data: str) -> dict: ...
@overload
def parse(data: bytes) -> list: ...

def parse(data: str | bytes) -> dict | list:
    """Implementation handles both cases."""
    if isinstance(data, str):
        import json
        return json.loads(data)
    else:
        return list(data)

# cast() — tell type checker "trust me, it's this type"
from typing import cast

def get_config() -> dict[str, Any]:
    return {"port": 8080}

# Type checker doesn't know the value type
config = get_config()
port = cast(int, config["port"])  # Tell mypy: this is int, trust me

# reveal_type() — debugging tool (shows what type checker infers)
# reveal_type(port)  # mypy will print: Revealed type is "int"


# =============================================================================
# SUMMARY: TYPE HINTS BEST PRACTICES
# =============================================================================
"""
1. Add type hints to public APIs (functions, classes, module variables)
2. Use modern syntax: list[int] not List[int], X | Y not Union[X, Y] (3.10+)
3. Use `None` return type explicitly: def delete(id: int) -> None:
4. Use TypeAlias for complex types that are used multiple times
5. Use Protocol for duck typing (not ABCs unless you share implementation)
6. Use @overload when return type depends on input type
7. Use TypeGuard for custom type-narrowing functions
8. Use Self for fluent interfaces and classmethods
9. Configure mypy/pyright in pyproject.toml
10. Use gradual typing: start loose, tighten over time
11. Never use `Any` as a lazy escape — it disables ALL type checking for that value
12. Use Final for constants, ClassVar for class-level attributes
"""
