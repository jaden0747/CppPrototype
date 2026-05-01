"""
Visitor Pattern
===============
Intent: Represent an operation to be performed on elements of an object
structure. Visitor lets you define a new operation without changing the
classes of the elements on which it operates.

Real-world analogy: An accountant who visits every department. Each
department type is processed differently, but departments don't need to know
about all possible accountant operations.
"""

from __future__ import annotations
import math
from abc import ABC, abstractmethod
from typing import List


# ---------------------------------------------------------------------------
# Visitor interface
# ---------------------------------------------------------------------------

class IShapeVisitor(ABC):
    @abstractmethod
    def visit_circle(self, circle: "Circle") -> None: ...

    @abstractmethod
    def visit_rectangle(self, rect: "Rectangle") -> None: ...

    @abstractmethod
    def visit_triangle(self, tri: "Triangle") -> None: ...


# ---------------------------------------------------------------------------
# Element interface
# ---------------------------------------------------------------------------

class Shape(ABC):
    @abstractmethod
    def accept(self, visitor: IShapeVisitor) -> None: ...

    @property
    @abstractmethod
    def name(self) -> str: ...


# ---------------------------------------------------------------------------
# Concrete elements
# ---------------------------------------------------------------------------

class Circle(Shape):
    def __init__(self, radius: float):
        self.radius = radius

    def accept(self, visitor: IShapeVisitor) -> None:
        visitor.visit_circle(self)

    @property
    def name(self) -> str:
        return "Circle"


class Rectangle(Shape):
    def __init__(self, width: float, height: float):
        self.width  = width
        self.height = height

    def accept(self, visitor: IShapeVisitor) -> None:
        visitor.visit_rectangle(self)

    @property
    def name(self) -> str:
        return "Rectangle"


class Triangle(Shape):
    def __init__(self, a: float, b: float, c: float):
        self.a = a
        self.b = b
        self.c = c

    def accept(self, visitor: IShapeVisitor) -> None:
        visitor.visit_triangle(self)

    @property
    def name(self) -> str:
        return "Triangle"


# ---------------------------------------------------------------------------
# Concrete Visitors
# ---------------------------------------------------------------------------

class AreaVisitor(IShapeVisitor):
    def __init__(self):
        self.total = 0.0

    def reset(self) -> None:
        self.total = 0.0

    def visit_circle(self, c: Circle) -> None:
        self.total += math.pi * c.radius ** 2

    def visit_rectangle(self, r: Rectangle) -> None:
        self.total += r.width * r.height

    def visit_triangle(self, t: Triangle) -> None:
        s = (t.a + t.b + t.c) / 2.0
        self.total += math.sqrt(s * (s - t.a) * (s - t.b) * (s - t.c))


class PerimeterVisitor(IShapeVisitor):
    def __init__(self):
        self.total = 0.0

    def reset(self) -> None:
        self.total = 0.0

    def visit_circle(self, c: Circle) -> None:
        self.total += 2 * math.pi * c.radius

    def visit_rectangle(self, r: Rectangle) -> None:
        self.total += 2 * (r.width + r.height)

    def visit_triangle(self, t: Triangle) -> None:
        self.total += t.a + t.b + t.c


class NameCollectorVisitor(IShapeVisitor):
    def __init__(self):
        self.names: List[str] = []

    def visit_circle(self,    c: Circle)    -> None: self.names.append(c.name)
    def visit_rectangle(self, r: Rectangle) -> None: self.names.append(r.name)
    def visit_triangle(self,  t: Triangle)  -> None: self.names.append(t.name)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    shapes: List[Shape] = [
        Circle(5.0),
        Rectangle(4.0, 3.0),
        Triangle(3.0, 4.0, 5.0),
    ]

    av = AreaVisitor()
    pv = PerimeterVisitor()
    nc = NameCollectorVisitor()

    for s in shapes:
        s.accept(av)
        s.accept(pv)
        s.accept(nc)

    print(f"Total area:      {av.total:.4f}")
    print(f"Total perimeter: {pv.total:.4f}")
    print(f"Shape names:     {nc.names}")
