"""
Prototype Pattern
=================
Intent: Create new objects by copying a prototypical instance rather than
constructing from scratch.

Real-world analogy: A game spawner holds one "template" enemy and clones it
whenever a new enemy is needed — no expensive initialization required.
"""

import copy


# ---------------------------------------------------------------------------
# Shape hierarchy
# ---------------------------------------------------------------------------

class Shape:
    def __init__(self, color: str):
        self.color = color

    def clone(self) -> "Shape":
        return copy.deepcopy(self)

    def type(self) -> str:
        raise NotImplementedError


class Circle(Shape):
    def __init__(self, color: str, radius: float):
        super().__init__(color)
        self.radius = radius

    def type(self) -> str:
        return "Circle"

    def __repr__(self) -> str:
        return f"Circle(color={self.color!r}, radius={self.radius})"


class Rectangle(Shape):
    def __init__(self, color: str, width: float, height: float):
        super().__init__(color)
        self.width  = width
        self.height = height

    def type(self) -> str:
        return "Rectangle"

    def __repr__(self) -> str:
        return f"Rectangle(color={self.color!r}, {self.width}x{self.height})"


# ---------------------------------------------------------------------------
# Prototype Registry
# ---------------------------------------------------------------------------

class ShapeRegistry:
    def __init__(self):
        self._registry: dict = {}

    def add(self, key: str, prototype: Shape) -> None:
        self._registry[key] = prototype

    def get(self, key: str) -> Shape:
        if key not in self._registry:
            raise KeyError(f"Prototype not found: {key}")
        return self._registry[key].clone()

    def __contains__(self, key: str) -> bool:
        return key in self._registry


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    # Direct cloning
    original = Circle("red", 5.0)
    clone    = original.clone()
    clone.color = "blue"
    print(f"Original: {original}")
    print(f"Clone:    {clone}")

    # Registry
    registry = ShapeRegistry()
    registry.add("circle",    Circle("green", 3.0))
    registry.add("rectangle", Rectangle("yellow", 10.0, 5.0))

    for key in ("circle", "rectangle"):
        shape = registry.get(key)
        print(f"From registry [{key}]: {shape}")
