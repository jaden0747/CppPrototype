"""
Adapter Pattern
===============
Intent: Convert the interface of a class into another interface that clients
expect, allowing incompatible classes to work together.

Real-world analogy: A travel adapter lets a US flat-pin plug work in a
European round-pin socket.

Two forms:
  - Object Adapter (composition) — wraps an instance.
  - (Class Adapter via multiple inheritance is omitted in Python;
    duck-typing usually makes it unnecessary.)
"""

import math


# ---------------------------------------------------------------------------
# Target interface (what the client uses)
# ---------------------------------------------------------------------------

class RoundHole:
    def __init__(self, radius: float):
        self.radius = radius

    def fits(self, peg_radius: float) -> bool:
        return peg_radius <= self.radius


class RoundPeg:
    def __init__(self, radius: float):
        self._radius = radius

    def radius(self) -> float:
        return self._radius


# ---------------------------------------------------------------------------
# Incompatible legacy class (Adaptee)
# ---------------------------------------------------------------------------

class SquarePeg:
    def __init__(self, width: float):
        self.width = width

    def describe(self) -> str:
        return f"SquarePeg(width={self.width})"


# ---------------------------------------------------------------------------
# Object Adapter
# ---------------------------------------------------------------------------

class SquarePegAdapter(RoundPeg):
    """Wraps a SquarePeg and exposes the RoundPeg interface."""

    def __init__(self, peg: SquarePeg):
        super().__init__(0.0)  # placeholder
        self._peg = peg

    def radius(self) -> float:
        # Smallest enclosing circle of a square of side `w`: r = w*sqrt(2)/2
        return self._peg.width * math.sqrt(2) / 2


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    hole = RoundHole(5.0)

    round_peg = RoundPeg(4.0)
    print(f"Round peg (r=4) fits in hole (r=5): {hole.fits(round_peg.radius())}")

    sq_small = SquarePeg(4.0)
    sq_large = SquarePeg(8.0)

    adapter_small = SquarePegAdapter(sq_small)
    adapter_large = SquarePegAdapter(sq_large)

    print(f"Square peg (w=4, r≈{adapter_small.radius():.3f}) fits: {hole.fits(adapter_small.radius())}")
    print(f"Square peg (w=8, r≈{adapter_large.radius():.3f}) fits: {hole.fits(adapter_large.radius())}")
