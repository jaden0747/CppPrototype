"""
=============================================================================
CHAPTER 21: DESIGN PATTERNS IN PYTHON
=============================================================================
Design patterns are reusable solutions to common problems.
In Python, many patterns are simpler than in other languages because
Python has first-class functions, duck typing, and dynamic dispatch.

KEY INSIGHT: Many Gang of Four patterns exist to work around
limitations of static languages (Java/C++). In Python:
- Strategy → just pass a function
- Singleton → just use a module
- Iterator → built into the language (__iter__)
- Observer → signals/events or simple callbacks

=============================================================================
"""

# =============================================================================
# 21.1 CREATIONAL PATTERNS
# =============================================================================

# --- SINGLETON ---
"""
WHAT: Ensure a class has only one instance.
WHEN: Global configuration, connection pools, caches.
PYTHONIC WAY: Use a module (modules are singletons in Python).
"""

# Method 1: Module-level (MOST PYTHONIC)
# config.py:
# settings = {"debug": False, "port": 8080}
# Usage: from config import settings

# Method 2: Class with __new__
class Singleton:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

s1 = Singleton()
s2 = Singleton()
assert s1 is s2

# Method 3: Module-level instance (recommended for complex singletons)
class _AppConfig:
    def __init__(self):
        self.debug = False
        self.port = 8080

app_config = _AppConfig()  # Single instance, exported from module


# --- FACTORY METHOD ---
"""
WHAT: Create objects without specifying exact class.
WHEN: The creation logic is complex or depends on runtime conditions.
PYTHONIC WAY: Simple function or classmethod.
"""

from dataclasses import dataclass
from abc import ABC, abstractmethod

@dataclass
class User:
    name: str
    role: str
    permissions: list

def create_user(role: str, name: str) -> User:
    """Factory function — simplest approach."""
    permissions = {
        "admin": ["read", "write", "delete", "manage"],
        "editor": ["read", "write"],
        "viewer": ["read"],
    }
    return User(name=name, role=role, permissions=permissions.get(role, []))

admin = create_user("admin", "Alice")
viewer = create_user("viewer", "Bob")
print(f"Factory: {admin}")

# Using classmethod (alternative factory):
class Document:
    def __init__(self, content: str, format: str):
        self.content = content
        self.format = format

    @classmethod
    def from_file(cls, path: str) -> "Document":
        """Factory classmethod."""
        ext = path.rsplit(".", 1)[-1]
        return cls(content=f"content of {path}", format=ext)

    @classmethod
    def from_template(cls, template_name: str) -> "Document":
        return cls(content=f"Template: {template_name}", format="html")


# --- BUILDER ---
"""
WHAT: Construct complex objects step by step.
WHEN: Object has many optional parameters or complex construction logic.
PYTHONIC WAY: Method chaining or dataclass with defaults.
"""

class QueryBuilder:
    """Fluent interface (method chaining)."""

    def __init__(self, table: str):
        self._table = table
        self._conditions: list[str] = []
        self._order: str | None = None
        self._limit: int | None = None

    def where(self, condition: str) -> "QueryBuilder":
        self._conditions.append(condition)
        return self  # Enable chaining

    def order_by(self, field: str) -> "QueryBuilder":
        self._order = field
        return self

    def limit(self, n: int) -> "QueryBuilder":
        self._limit = n
        return self

    def build(self) -> str:
        query = f"SELECT * FROM {self._table}"
        if self._conditions:
            query += " WHERE " + " AND ".join(self._conditions)
        if self._order:
            query += f" ORDER BY {self._order}"
        if self._limit:
            query += f" LIMIT {self._limit}"
        return query

# Usage:
query = (
    QueryBuilder("users")
    .where("age > 18")
    .where("active = true")
    .order_by("name")
    .limit(10)
    .build()
)
print(f"Builder: {query}")


# =============================================================================
# 21.2 STRUCTURAL PATTERNS
# =============================================================================

# --- ADAPTER ---
"""
WHAT: Make incompatible interfaces work together.
WHEN: Integrating third-party code or legacy systems.
"""

# Legacy system with different interface:
class OldPaymentSystem:
    def make_payment(self, amount_cents: int, currency_code: str):
        return f"Paid {amount_cents} cents in {currency_code}"

# Our expected interface:
class PaymentProcessor(ABC):
    @abstractmethod
    def pay(self, amount: float, currency: str = "USD") -> str:
        pass

# Adapter:
class PaymentAdapter(PaymentProcessor):
    def __init__(self, old_system: OldPaymentSystem):
        self._old = old_system

    def pay(self, amount: float, currency: str = "USD") -> str:
        cents = int(amount * 100)
        return self._old.make_payment(cents, currency)

adapter = PaymentAdapter(OldPaymentSystem())
print(f"Adapter: {adapter.pay(19.99)}")


# --- DECORATOR PATTERN (not Python decorators!) ---
"""
WHAT: Add behavior to objects dynamically without subclassing.
WHEN: You need to add features to objects at runtime.
NOTE: Different from Python's @decorator syntax (though related in spirit).
"""

class DataSource(ABC):
    @abstractmethod
    def read(self) -> str:
        pass

    @abstractmethod
    def write(self, data: str) -> None:
        pass

class FileDataSource(DataSource):
    def __init__(self, filename: str):
        self._filename = filename
        self._data = ""

    def read(self) -> str:
        return self._data

    def write(self, data: str) -> None:
        self._data = data

class EncryptionDecorator(DataSource):
    """Adds encryption to any DataSource."""

    def __init__(self, source: DataSource):
        self._source = source

    def read(self) -> str:
        data = self._source.read()
        return self._decrypt(data)

    def write(self, data: str) -> None:
        self._source.write(self._encrypt(data))

    def _encrypt(self, data: str) -> str:
        return data[::-1]  # Simple reversal as "encryption"

    def _decrypt(self, data: str) -> str:
        return data[::-1]

# Compose behaviors:
source = FileDataSource("data.txt")
encrypted_source = EncryptionDecorator(source)
encrypted_source.write("secret data")
print(f"Decorator pattern: {encrypted_source.read()}")


# --- FACADE ---
"""
WHAT: Provide a simple interface to a complex subsystem.
WHEN: Hiding complexity of multiple components behind a clean API.
"""

class VideoConverter:
    """Facade over complex video processing subsystem."""

    def convert(self, filename: str, format: str) -> str:
        # Internally coordinates multiple complex components:
        # codec = self._get_codec(format)
        # source = self._read_file(filename)
        # audio = self._extract_audio(source)
        # video = self._process_video(source, codec)
        # result = self._mux(audio, video)
        return f"Converted {filename} to {format}"

# Simple interface:
converter = VideoConverter()
print(f"Facade: {converter.convert('movie.avi', 'mp4')}")


# =============================================================================
# 21.3 BEHAVIORAL PATTERNS
# =============================================================================

# --- STRATEGY ---
"""
WHAT: Define a family of algorithms, make them interchangeable.
WHEN: You need different algorithms for the same task.
PYTHONIC WAY: Just pass a function! (First-class functions replace classes)
"""

from typing import Callable

# Pythonic strategy — functions as strategies:
def price_regular(price: float) -> float:
    return price

def price_premium(price: float) -> float:
    return price * 0.9  # 10% discount

def price_vip(price: float) -> float:
    return price * 0.8  # 20% discount

def calculate_total(
    prices: list[float],
    strategy: Callable[[float], float] = price_regular
) -> float:
    """Apply pricing strategy to all items."""
    return sum(strategy(p) for p in prices)

items = [100.0, 50.0, 75.0]
print(f"Strategy regular: {calculate_total(items)}")
print(f"Strategy VIP: {calculate_total(items, price_vip)}")

# Class-based strategy (when strategy has state):
class DiscountStrategy(ABC):
    @abstractmethod
    def calculate(self, price: float) -> float:
        pass

class PercentageDiscount(DiscountStrategy):
    def __init__(self, percent: float):
        self.percent = percent

    def calculate(self, price: float) -> float:
        return price * (1 - self.percent / 100)


# --- OBSERVER ---
"""
WHAT: Objects subscribe to events and get notified when they occur.
WHEN: Event-driven systems, UI updates, pub/sub messaging.
"""

class EventEmitter:
    """Simple observer/event system."""

    def __init__(self):
        self._listeners: dict[str, list[Callable]] = {}

    def on(self, event: str, callback: Callable) -> None:
        """Subscribe to an event."""
        self._listeners.setdefault(event, []).append(callback)

    def emit(self, event: str, *args, **kwargs) -> None:
        """Notify all listeners of an event."""
        for callback in self._listeners.get(event, []):
            callback(*args, **kwargs)

    def off(self, event: str, callback: Callable) -> None:
        """Unsubscribe from an event."""
        if event in self._listeners:
            self._listeners[event].remove(callback)

# Usage:
store = EventEmitter()

def on_price_change(item, new_price):
    print(f"  Observer: {item} price changed to ${new_price}")

def on_price_log(item, new_price):
    print(f"  Logger: Recording price change for {item}")

store.on("price_change", on_price_change)
store.on("price_change", on_price_log)

print("Observer pattern:")
store.emit("price_change", "Widget", 9.99)


# --- COMMAND ---
"""
WHAT: Encapsulate a request as an object.
WHEN: Undo/redo, queuing operations, macro recording.
"""

class Command(ABC):
    @abstractmethod
    def execute(self) -> None:
        pass

    @abstractmethod
    def undo(self) -> None:
        pass

class TextEditor:
    def __init__(self):
        self.content = ""
        self._history: list[Command] = []

    def execute(self, command: "TextCommand") -> None:
        command.execute()
        self._history.append(command)

    def undo(self) -> None:
        if self._history:
            command = self._history.pop()
            command.undo()

class InsertText(Command):
    def __init__(self, editor: TextEditor, text: str):
        self._editor = editor
        self._text = text

    def execute(self) -> None:
        self._editor.content += self._text

    def undo(self) -> None:
        self._editor.content = self._editor.content[:-len(self._text)]

# TextCommand alias for type hints:
TextCommand = Command

editor = TextEditor()
editor.execute(InsertText(editor, "Hello"))
editor.execute(InsertText(editor, " World"))
print(f"Command: '{editor.content}'")
editor.undo()
print(f"After undo: '{editor.content}'")


# --- CHAIN OF RESPONSIBILITY ---
"""
WHAT: Pass request along a chain of handlers until one handles it.
WHEN: Request processing pipelines, middleware, event handling.
"""

class Handler(ABC):
    def __init__(self):
        self._next: Handler | None = None

    def set_next(self, handler: "Handler") -> "Handler":
        self._next = handler
        return handler

    def handle(self, request: dict) -> str | None:
        if self._next:
            return self._next.handle(request)
        return None

class AuthHandler(Handler):
    def handle(self, request: dict) -> str | None:
        if not request.get("authenticated"):
            return "Error: Not authenticated"
        return super().handle(request)

class RateLimitHandler(Handler):
    def handle(self, request: dict) -> str | None:
        if request.get("requests_per_minute", 0) > 100:
            return "Error: Rate limit exceeded"
        return super().handle(request)

class BusinessHandler(Handler):
    def handle(self, request: dict) -> str | None:
        return f"Success: Processed request for {request.get('user')}"

# Build chain:
auth = AuthHandler()
rate_limit = RateLimitHandler()
business = BusinessHandler()
auth.set_next(rate_limit).set_next(business)

# Process requests:
print(f"Chain: {auth.handle({'authenticated': True, 'user': 'Alice', 'requests_per_minute': 50})}")
print(f"Chain: {auth.handle({'authenticated': False, 'user': 'Eve'})}")


# --- STATE ---
"""
WHAT: Object behavior changes based on internal state.
WHEN: State machines, workflow engines, game logic.
"""

class OrderState(ABC):
    @abstractmethod
    def next(self, order: "Order") -> None:
        pass

    @abstractmethod
    def status(self) -> str:
        pass

class PendingState(OrderState):
    def next(self, order: "Order") -> None:
        order._state = ProcessingState()
    def status(self) -> str:
        return "Pending"

class ProcessingState(OrderState):
    def next(self, order: "Order") -> None:
        order._state = ShippedState()
    def status(self) -> str:
        return "Processing"

class ShippedState(OrderState):
    def next(self, order: "Order") -> None:
        order._state = DeliveredState()
    def status(self) -> str:
        return "Shipped"

class DeliveredState(OrderState):
    def next(self, order: "Order") -> None:
        pass  # Final state
    def status(self) -> str:
        return "Delivered"

class Order:
    def __init__(self):
        self._state: OrderState = PendingState()

    def next_step(self) -> None:
        self._state.next(self)

    @property
    def status(self) -> str:
        return self._state.status()

order = Order()
print(f"State: {order.status}")
order.next_step()
print(f"State: {order.status}")
order.next_step()
print(f"State: {order.status}")


# =============================================================================
# 21.4 PYTHON-SPECIFIC PATTERNS
# =============================================================================

# --- MIXIN ---
"""
WHAT: Add functionality through multiple inheritance.
WHEN: Share behavior across unrelated classes.
PYTHONIC: Very common in Python (Django, Flask, etc.)
"""

class SerializableMixin:
    """Add JSON serialization to any class."""
    def to_dict(self) -> dict:
        return {k: v for k, v in self.__dict__.items() if not k.startswith('_')}

    def to_json(self) -> str:
        import json
        return json.dumps(self.to_dict())

class ValidatableMixin:
    """Add validation to any class."""
    def validate(self) -> bool:
        for field, value in self.__dict__.items():
            if value is None:
                raise ValueError(f"{field} cannot be None")
        return True

@dataclass
class Product(SerializableMixin, ValidatableMixin):
    name: str
    price: float
    category: str

product = Product("Widget", 9.99, "Tools")
print(f"Mixin: {product.to_json()}")


# --- REGISTRY ---
"""
WHAT: Auto-register classes/functions by name for later lookup.
WHEN: Plugin systems, command dispatch, handler registration.
"""

# Function registry:
_handlers: dict[str, Callable] = {}

def register(name: str):
    """Decorator to register a handler."""
    def decorator(func):
        _handlers[name] = func
        return func
    return decorator

@register("greet")
def handle_greet(data):
    return f"Hello, {data['name']}!"

@register("farewell")
def handle_farewell(data):
    return f"Goodbye, {data['name']}!"

def dispatch(command: str, data: dict) -> str:
    handler = _handlers.get(command)
    if handler is None:
        return f"Unknown command: {command}"
    return handler(data)

print(f"Registry: {dispatch('greet', {'name': 'Alice'})}")


# --- CONTEXT MANAGER AS A PATTERN ---
"""
Resource management pattern built into Python's syntax.
"""

from contextlib import contextmanager

@contextmanager
def transaction(connection):
    """Database transaction pattern."""
    try:
        yield connection
        connection.get("commit", lambda: None)()
    except Exception:
        connection.get("rollback", lambda: None)()
        raise

# Usage:
# with transaction(db_conn) as conn:
#     conn.execute("INSERT ...")
#     conn.execute("UPDATE ...")
# Auto-committed or auto-rolled-back


# =============================================================================
# SUMMARY: WHEN TO USE EACH PATTERN
# =============================================================================
"""
CREATIONAL:
- Singleton → Module-level instance (or just use module)
- Factory → Function that returns different types based on input
- Builder → Method chaining for complex object construction

STRUCTURAL:
- Adapter → Wrap incompatible interface to match expected one
- Decorator → Add behavior without modifying original (wrapping)
- Facade → Simplify complex subsystem behind clean API

BEHAVIORAL:
- Strategy → Pass function/callable for algorithm selection
- Observer → Event emitter with subscribe/emit
- Command → Encapsulate actions for undo/redo/queue
- Chain of Responsibility → Pipeline of handlers
- State → Object behavior changes with internal state

PYTHON-SPECIFIC:
- Mixin → Share behavior via multiple inheritance
- Registry → Auto-register handlers/plugins with decorators
- Context Manager → Resource management (with statement)

REMEMBER:
- Don't force patterns — use them when they solve a real problem
- In Python, many patterns reduce to "just pass a function"
- Duck typing eliminates need for many interface patterns
- The simplest solution that works IS the best pattern
"""
