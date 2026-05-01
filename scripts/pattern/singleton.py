"""
Singleton Pattern
=================
Intent: Ensure a class has only one instance and provide a global access
point to it.

Python approach:
  - Override __new__ to return the same instance every time.
  - Thread-safe version shown using threading.Lock.

Real-world analogy: A government has only one official president at a time.
"""

import threading


class Singleton:
    """Classic Singleton via __new__ override."""

    _instance = None
    _lock = threading.Lock()

    def __new__(cls):
        if cls._instance is None:
            with cls._lock:
                # Double-checked locking
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
                    cls._instance._counter = 0
        return cls._instance

    def increment(self):
        self._counter += 1

    def reset(self):
        self._counter = 0

    @property
    def value(self):
        return self._counter


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    a = Singleton()
    b = Singleton()

    assert a is b, "Both variables should point to the same instance"

    a.increment()
    a.increment()
    print(f"Counter via a: {a.value}")   # 2
    print(f"Counter via b: {b.value}")   # 2  (same object)

    b.reset()
    print(f"After reset via b: {a.value}")  # 0

    print("Singleton demo complete.")
