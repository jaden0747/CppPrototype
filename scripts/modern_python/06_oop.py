"""
=============================================================================
CHAPTER 6: OBJECT-ORIENTED PROGRAMMING
=============================================================================
Python's OOP is flexible and pragmatic — you're not forced to make everything
a class, but classes are powerful when used correctly.

WHY OOP in Python is different from Java/C++:
- Everything is already an object (even functions, modules, types)
- No access modifiers (no private/protected keywords — uses convention)
- Duck typing: "If it walks like a duck and quacks like a duck..."
- Multiple inheritance with MRO (Method Resolution Order)
- Protocols for structural typing (don't need to inherit!)

WHEN to use classes vs functions:
- Use classes when: you have state + operations on that state
- Use classes when: you need multiple instances with same interface
- Use functions when: stateless transformation
- Use closures when: lightweight state (counter, cache)
- DON'T use classes for: namespace-only (use module) or single method (use function)

=============================================================================
"""

# =============================================================================
# 6.1 CLASSES AND INSTANCES
# =============================================================================
"""
WHAT: Classes are blueprints for creating objects.
      `class` creates a new type. Instantiation calls __init__.

HOW Python classes work internally:
- Classes are objects too (instances of `type`)
- Instance attributes stored in __dict__ (or __slots__)
- Method lookup: instance → class → parent classes (MRO)
- `self` is explicit (unlike `this` in other languages)
"""

class Dog:
    # Class attribute — shared by all instances
    species = "Canis familiaris"

    def __init__(self, name: str, age: int):
        """Initialize instance attributes.

        self is the instance being created.
        __init__ is NOT a constructor — __new__ creates the object,
        __init__ initializes it.
        """
        # Instance attributes — unique to each object
        self.name = name
        self.age = age

    def bark(self) -> str:
        """Instance method — operates on self."""
        return f"{self.name} says Woof!"

    def __repr__(self) -> str:
        """Developer-friendly string representation.
        Should be unambiguous, ideally valid Python to recreate the object.
        """
        return f"Dog(name={self.name!r}, age={self.age})"

    def __str__(self) -> str:
        """User-friendly string representation.
        Used by print() and str().
        """
        return f"{self.name} ({self.age} years old)"

# Instantiation
rex = Dog("Rex", 5)
print(repr(rex))  # Dog(name='Rex', age=5)
print(str(rex))   # Rex (5 years old)
print(rex.bark()) # Rex says Woof!

# Class vs instance attributes
print(f"Species: {rex.species}")  # Accessed through instance (falls back to class)
print(f"Species: {Dog.species}")  # Accessed through class directly


# =============================================================================
# 6.2 INSTANCE, CLASS, AND STATIC METHODS
# =============================================================================
"""
WHAT:
- Instance methods: operate on instance (self), most common
- Class methods (@classmethod): operate on class (cls), for alternative constructors
- Static methods (@staticmethod): no access to instance or class, utility functions

WHEN to use each:
- Instance method: needs access to instance data (default choice)
- Class method: alternative constructors, factory patterns, class-level operations
- Static method: utility that logically belongs to class but needs no instance/class access
"""

import json
from datetime import datetime

class User:
    _count = 0  # Class-level counter

    def __init__(self, name: str, email: str):
        self.name = name
        self.email = email
        self.created_at = datetime.now()
        User._count += 1

    # Instance method — most common
    def greet(self) -> str:
        return f"Hi, I'm {self.name}"

    # Class method — alternative constructor
    @classmethod
    def from_json(cls, json_str: str) -> "User":
        """Create User from JSON string.

        WHY classmethod for constructors:
        - Works correctly with inheritance (cls is the actual subclass)
        - Clear intent: this is a factory method
        """
        data = json.loads(json_str)
        return cls(data["name"], data["email"])

    @classmethod
    def from_dict(cls, data: dict) -> "User":
        """Another alternative constructor."""
        return cls(data["name"], data["email"])

    @classmethod
    def get_count(cls) -> int:
        """Access class-level state."""
        return cls._count

    # Static method — utility function in class namespace
    @staticmethod
    def validate_email(email: str) -> bool:
        """Validate email format.

        WHY staticmethod:
        - Doesn't need self or cls
        - Logically belongs to User class
        - Could be a module-level function too (matter of organization)
        """
        return "@" in email and "." in email.split("@")[1]

# Using alternative constructors
user1 = User("Alice", "alice@example.com")
user2 = User.from_json('{"name": "Bob", "email": "bob@example.com"}')
user3 = User.from_dict({"name": "Charlie", "email": "charlie@example.com"})

print(f"Count: {User.get_count()}")  # 3
print(f"Valid: {User.validate_email('test@example.com')}")  # True


# =============================================================================
# 6.3 PROPERTIES
# =============================================================================
"""
WHAT: @property turns method access into attribute-like access.
      Implements getters/setters/deleters without changing the API.

WHY properties are pythonic:
- Start with simple attributes, add logic later WITHOUT changing callers
- No need for get_X() / set_X() methods (unlike Java)
- Uniform access principle: callers don't know if it's computed or stored
- Can add validation, caching, logging transparently

WHEN to use:
- Adding validation to attribute assignment
- Computing derived values (area from width/height)
- Lazy evaluation (compute on first access, cache)
- Deprecating direct attribute access
"""

class Temperature:
    """Demonstrate properties with validation and computed values."""

    def __init__(self, celsius: float = 0):
        # Use the setter (which validates)
        self.celsius = celsius  # calls the setter!

    @property
    def celsius(self) -> float:
        """Get temperature in Celsius."""
        return self._celsius

    @celsius.setter
    def celsius(self, value: float):
        """Set temperature with validation."""
        if value < -273.15:
            raise ValueError(f"Temperature below absolute zero: {value}")
        self._celsius = value

    @property
    def fahrenheit(self) -> float:
        """Computed property — derived from celsius."""
        return self._celsius * 9 / 5 + 32

    @fahrenheit.setter
    def fahrenheit(self, value: float):
        """Set via fahrenheit — converts to celsius internally."""
        self.celsius = (value - 32) * 5 / 9

# Usage — looks like simple attribute access!
temp = Temperature(100)
print(f"{temp.celsius}°C = {temp.fahrenheit}°F")  # 100°C = 212°F

temp.fahrenheit = 72
print(f"{temp.celsius:.1f}°C = {temp.fahrenheit}°F")  # 22.2°C = 72°F

# This would raise ValueError:
# temp.celsius = -300  # ValueError: Temperature below absolute zero

# BEST PRACTICE: Start with simple attributes. Add @property only when needed.
# DON'T preemptively create getters/setters (that's Java thinking).


# =============================================================================
# 6.4 INHERITANCE AND MRO
# =============================================================================
"""
WHAT: Classes can inherit from other classes to reuse and extend behavior.
      Python supports multiple inheritance with C3 linearization (MRO).

WHY Python uses MRO:
- Resolves the "diamond problem" deterministically
- Ensures each class in hierarchy is called exactly once
- Makes super() work correctly with multiple inheritance

BEST PRACTICES:
- Prefer composition over inheritance for most cases
- Use inheritance for "is-a" relationships
- Keep hierarchies shallow (2-3 levels max)
- Use ABCs to define interfaces
"""

class Animal:
    def __init__(self, name: str):
        self.name = name

    def speak(self) -> str:
        raise NotImplementedError("Subclasses must implement speak()")

    def __repr__(self) -> str:
        return f"{type(self).__name__}({self.name!r})"

class Dog(Animal):
    def speak(self) -> str:
        return f"{self.name} says Woof!"

class Cat(Animal):
    def speak(self) -> str:
        return f"{self.name} says Meow!"

# Polymorphism — same interface, different behavior
animals = [Dog("Rex"), Cat("Whiskers"), Dog("Buddy")]
for animal in animals:
    print(animal.speak())

# super() — call parent class methods
class GuideDog(Dog):
    def __init__(self, name: str, handler: str):
        super().__init__(name)  # Call parent __init__
        self.handler = handler

    def speak(self) -> str:
        base = super().speak()  # Call parent speak()
        return f"{base} (trained guide dog for {self.handler})"

# MRO — Method Resolution Order
class A:
    def method(self):
        return "A"

class B(A):
    def method(self):
        return "B"

class C(A):
    def method(self):
        return "C"

class D(B, C):
    pass

# D → B → C → A → object
print(f"MRO: {[cls.__name__ for cls in D.__mro__]}")
d = D()
print(f"D.method(): {d.method()}")  # "B" (first in MRO after D)


# =============================================================================
# 6.5 ABSTRACT BASE CLASSES (ABC)
# =============================================================================
"""
WHAT: ABCs define interfaces — classes that CANNOT be instantiated,
      only subclassed. They enforce that subclasses implement required methods.

WHY use ABCs:
- Document the expected interface clearly
- Fail fast: error at instantiation, not at method call
- Enable isinstance() checks against the interface
- IDE support: know what methods to implement

WHEN to use:
- Defining plugin/extension interfaces
- Framework APIs that users must implement
- When you want compile-time-like checks for missing methods
"""

from abc import ABC, abstractmethod

class Shape(ABC):
    """Abstract base class for shapes."""

    @abstractmethod
    def area(self) -> float:
        """Compute the area. Must be implemented by subclasses."""
        ...

    @abstractmethod
    def perimeter(self) -> float:
        """Compute the perimeter. Must be implemented by subclasses."""
        ...

    # Concrete method — shared by all subclasses
    def describe(self) -> str:
        return f"{type(self).__name__}: area={self.area():.2f}, perimeter={self.perimeter():.2f}"

# shape = Shape()  # TypeError: Can't instantiate abstract class!

class Circle(Shape):
    def __init__(self, radius: float):
        self.radius = radius

    def area(self) -> float:
        import math
        return math.pi * self.radius ** 2

    def perimeter(self) -> float:
        import math
        return 2 * math.pi * self.radius

class Rectangle(Shape):
    def __init__(self, width: float, height: float):
        self.width = width
        self.height = height

    def area(self) -> float:
        return self.width * self.height

    def perimeter(self) -> float:
        return 2 * (self.width + self.height)

shapes = [Circle(5), Rectangle(3, 4)]
for shape in shapes:
    print(shape.describe())


# =============================================================================
# 6.6 DUNDER (MAGIC) METHODS
# =============================================================================
"""
WHAT: Special methods with double underscores (__method__) that Python calls
      implicitly. They customize how objects behave with operators, built-in
      functions, and language constructs.

WHY they matter:
- Make your objects work with Python's syntax (+ - * == < for in len str)
- Integration with standard library (sorted, print, format, etc.)
- Implement protocols (iterator, context manager, descriptor, etc.)

COMMON DUNDER METHODS:
- __repr__, __str__: string representation
- __eq__, __hash__: equality and hashing (for sets/dict keys)
- __lt__, __le__, __gt__, __ge__: comparison
- __add__, __sub__, __mul__: arithmetic
- __len__, __getitem__, __setitem__: container behavior
- __iter__, __next__: iterator protocol
- __enter__, __exit__: context manager
- __call__: make instance callable
"""

class Vector:
    """2D vector demonstrating dunder methods."""

    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

    # === REPRESENTATION ===
    def __repr__(self) -> str:
        """For developers. Should be unambiguous."""
        return f"Vector({self.x}, {self.y})"

    def __str__(self) -> str:
        """For users. Can be informal."""
        return f"({self.x}, {self.y})"

    # === ARITHMETIC ===
    def __add__(self, other: "Vector") -> "Vector":
        """v1 + v2"""
        return Vector(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector") -> "Vector":
        """v1 - v2"""
        return Vector(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector":
        """v * scalar"""
        return Vector(self.x * scalar, self.y * scalar)

    def __rmul__(self, scalar: float) -> "Vector":
        """scalar * v (when left operand doesn't support the operation)"""
        return self.__mul__(scalar)

    # === COMPARISON ===
    def __eq__(self, other: object) -> bool:
        """v1 == v2"""
        if not isinstance(other, Vector):
            return NotImplemented
        return self.x == other.x and self.y == other.y

    def __hash__(self) -> int:
        """Required when __eq__ is defined. Makes Vector usable as dict key."""
        return hash((self.x, self.y))

    # === CONTAINER-LIKE ===
    def __len__(self) -> int:
        """len(v) — number of dimensions"""
        return 2

    def __getitem__(self, index: int) -> float:
        """v[0], v[1] — index access"""
        if index == 0:
            return self.x
        elif index == 1:
            return self.y
        raise IndexError(f"Vector index {index} out of range")

    def __iter__(self):
        """for component in v: — makes Vector iterable"""
        yield self.x
        yield self.y

    # === CALLABLE ===
    def __call__(self, scale: float = 1.0) -> "Vector":
        """v(2.0) — make instance callable"""
        return Vector(self.x * scale, self.y * scale)

    # === MAGNITUDE ===
    def __abs__(self) -> float:
        """abs(v) — magnitude of vector"""
        return (self.x ** 2 + self.y ** 2) ** 0.5

    def __bool__(self) -> bool:
        """bool(v) — True if non-zero vector"""
        return self.x != 0 or self.y != 0

# Demonstration
v1 = Vector(3, 4)
v2 = Vector(1, 2)

print(f"v1 + v2 = {v1 + v2}")      # (4, 6)
print(f"v1 * 3 = {v1 * 3}")        # (9, 12)
print(f"3 * v1 = {3 * v1}")        # (9, 12) — __rmul__
print(f"|v1| = {abs(v1)}")          # 5.0
print(f"v1[0] = {v1[0]}")          # 3
print(f"len(v1) = {len(v1)}")      # 2
print(f"list(v1) = {list(v1)}")    # [3, 4] — iterable
print(f"v1 == Vector(3, 4): {v1 == Vector(3, 4)}")  # True

# Can be dict key (because __hash__ is defined)
vector_names = {Vector(0, 0): "origin", Vector(1, 0): "unit_x"}


# =============================================================================
# 6.7 __SLOTS__
# =============================================================================
"""
WHAT: __slots__ restricts instances to declared attributes only.
      Replaces __dict__ with a more efficient fixed-size structure.

WHY use __slots__:
- 40-50% less memory per instance (no __dict__)
- Slightly faster attribute access
- Prevents accidental attribute creation (catches typos)

WHEN to use:
- Many instances (thousands+) of the same class
- Performance-critical code
- When you want to prevent dynamic attribute addition

WHEN NOT to use:
- Prototyping or small number of instances
- When you need __dict__ (dynamic attributes, weakrefs)
- Classes using multiple inheritance (slots don't compose well)
"""

class Point2D:
    __slots__ = ("x", "y")

    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

# Efficient — no __dict__
p = Point2D(3, 4)
print(f"Point: ({p.x}, {p.y})")
# p.z = 5  # AttributeError! Can't add attributes not in __slots__

# Memory comparison
import sys

class PointWithDict:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class PointWithSlots:
    __slots__ = ("x", "y")
    def __init__(self, x, y):
        self.x = x
        self.y = y

# PointWithSlots uses ~40-50% less memory per instance


# =============================================================================
# 6.8 DATACLASSES
# =============================================================================
"""
WHAT: @dataclass auto-generates __init__, __repr__, __eq__, and optionally
      __hash__, __lt__, etc. based on class attributes.

WHY dataclasses are a game-changer:
- Eliminate boilerplate for data-holding classes
- Self-documenting (fields are declared at class level with types)
- Customizable (frozen, ordered, slots)
- Replace namedtuples for mutable data with defaults

WHEN to use dataclasses:
- Any class that's primarily a data container
- Configuration objects
- DTOs (Data Transfer Objects)
- Replacing dicts for structured data

WHEN NOT to use:
- Very complex initialization logic
- When you need immutability AND methods (consider frozen=True or NamedTuple)
- Compatibility with Python < 3.7
"""

from dataclasses import dataclass, field, asdict, astuple

@dataclass
class Product:
    """Auto-generates __init__, __repr__, __eq__."""
    name: str
    price: float
    quantity: int = 0
    tags: list = field(default_factory=list)  # Mutable default — use field()!

    # You can still add methods
    @property
    def total_value(self) -> float:
        return self.price * self.quantity

    def apply_discount(self, percent: float) -> None:
        self.price *= (1 - percent / 100)

# Auto-generated __init__
p = Product("Widget", 9.99, quantity=100, tags=["sale"])
print(repr(p))  # Product(name='Widget', price=9.99, quantity=100, tags=['sale'])

# Auto-generated __eq__ (compares all fields)
p2 = Product("Widget", 9.99, quantity=100, tags=["sale"])
print(f"p == p2: {p == p2}")  # True

# Conversion utilities
print(f"As dict: {asdict(p)}")
print(f"As tuple: {astuple(p)}")

# === FROZEN DATACLASS (immutable) ===
@dataclass(frozen=True)
class Coordinate:
    """Immutable dataclass — can be used as dict key."""
    lat: float
    lon: float

coord = Coordinate(40.7128, -74.0060)
# coord.lat = 0  # FrozenInstanceError!
# Can be dict key:
locations = {coord: "New York City"}

# === DATACLASS WITH SLOTS (Python 3.10+) ===
@dataclass(slots=True)
class Pixel:
    """Memory-efficient dataclass with __slots__."""
    x: int
    y: int
    color: str = "black"

# === POST-INIT PROCESSING ===
@dataclass
class Employee:
    first_name: str
    last_name: str
    salary: float
    full_name: str = field(init=False)  # Not in __init__

    def __post_init__(self):
        """Called after __init__ — for derived fields."""
        self.full_name = f"{self.first_name} {self.last_name}"

emp = Employee("Alice", "Smith", 75000)
print(f"Full name: {emp.full_name}")  # "Alice Smith"

# === ORDERING ===
@dataclass(order=True)
class Priority:
    """Generates __lt__, __le__, __gt__, __ge__ based on fields."""
    level: int
    name: str = field(compare=False)  # Excluded from comparison

tasks = [Priority(3, "low"), Priority(1, "critical"), Priority(2, "medium")]
print(f"Sorted: {sorted(tasks)}")


# =============================================================================
# 6.9 ENUMS
# =============================================================================
"""
WHAT: Enums define a fixed set of named constants.
      They're self-documenting, type-safe, and iterable.

WHY use enums:
- Replace magic strings/numbers with meaningful names
- Type-safe — IDE catches invalid values
- Iterable — can list all valid values
- Comparable and hashable

WHEN to use:
- Status codes, states, modes
- Configuration options with fixed choices
- Anything that's "one of these specific values"
"""

from enum import Enum, auto, IntEnum, StrEnum

class Color(Enum):
    RED = 1
    GREEN = 2
    BLUE = 3

# Access
print(Color.RED)        # Color.RED
print(Color.RED.name)   # "RED"
print(Color.RED.value)  # 1
print(Color["RED"])     # Color.RED (by name)
print(Color(1))         # Color.RED (by value)

# Iteration
for color in Color:
    print(f"{color.name}: {color.value}")

# Comparison
print(Color.RED == Color.RED)   # True
print(Color.RED == Color.BLUE)  # False
print(Color.RED is Color.RED)   # True (singletons)

# auto() — auto-assign values
class Direction(Enum):
    NORTH = auto()
    SOUTH = auto()
    EAST = auto()
    WEST = auto()

# StrEnum (Python 3.11+) — values are strings
class Status(StrEnum):
    PENDING = auto()    # "pending"
    ACTIVE = auto()     # "active"
    CLOSED = auto()     # "closed"

# Can use directly as string:
print(f"Status: {Status.ACTIVE}")  # "active"

# Enum with methods
class Planet(Enum):
    MERCURY = (3.303e+23, 2.4397e6)
    VENUS = (4.869e+24, 6.0518e6)
    EARTH = (5.976e+24, 6.37814e6)

    def __init__(self, mass, radius):
        self.mass = mass
        self.radius = radius

    @property
    def surface_gravity(self):
        G = 6.67300E-11
        return G * self.mass / (self.radius ** 2)


# =============================================================================
# 6.10 PROTOCOLS (Structural Subtyping)
# =============================================================================
"""
WHAT: Protocols define interfaces through structure, not inheritance.
      If an object has the right methods/attributes, it satisfies the protocol.

WHY Protocols matter (Python 3.8+):
- Duck typing with type checker support
- No need to inherit from ABC just to satisfy an interface
- Works with existing code that wasn't designed for your interface
- "If it has .read() and .write(), it's a file-like object"

WHEN to use Protocols vs ABCs:
- Protocol: when you want structural typing (duck typing + type safety)
- ABC: when you want to enforce inheritance and share implementation
"""

from typing import Protocol, runtime_checkable

@runtime_checkable
class Readable(Protocol):
    """Any object with a read() method satisfies this protocol."""
    def read(self) -> str:
        ...

class FileReader:
    """Not inheriting from Readable, but satisfies the protocol!"""
    def read(self) -> str:
        return "file content"

class StringReader:
    """Also satisfies Readable without inheriting."""
    def read(self) -> str:
        return "string content"

def process(source: Readable) -> str:
    """Accepts anything with a read() method."""
    return source.read().upper()

# Both work — no inheritance needed!
print(process(FileReader()))
print(process(StringReader()))

# runtime_checkable allows isinstance checks
print(isinstance(FileReader(), Readable))  # True


# =============================================================================
# SUMMARY: OOP BEST PRACTICES
# =============================================================================
"""
1. Start with functions. Only use classes when you have state + behavior.
2. Use @dataclass for data containers (avoid manual __init__/__repr__/__eq__)
3. Prefer composition over inheritance ("has-a" over "is-a")
4. Keep inheritance hierarchies shallow (max 2-3 levels)
5. Use Protocols for duck typing + type safety (not ABCs unless you need shared code)
6. Use @property to add logic to attribute access (start with plain attributes)
7. Use __slots__ for memory-intensive classes (many instances)
8. Implement __repr__ for ALL classes (debugging must-have)
9. Use Enum for fixed sets of constants (not magic strings)
10. Don't create classes for things that could be:
    - A module (namespace only)
    - A function (no state)
    - A closure (simple state)
    - A dict or namedtuple (data only)
"""
