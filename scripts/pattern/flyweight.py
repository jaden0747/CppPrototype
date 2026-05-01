"""
Flyweight Pattern
=================
Intent: Use sharing to efficiently support a large number of fine-grained
objects. Separate intrinsic state (shared) from extrinsic state (per-object).

Real-world analogy: A forest of a million trees. Each tree shares one of ~10
TreeType objects (texture, mesh) while having unique (x, y) coordinates.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Dict, List, Tuple


# ---------------------------------------------------------------------------
# Flyweight (intrinsic state)
# ---------------------------------------------------------------------------

class TreeType:
    def __init__(self, name: str, colour: str, texture: str):
        self.name    = name
        self.colour  = colour
        self.texture = texture

    def render(self, x: int, y: int) -> str:
        return (f"Draw {self.name} [{self.colour}/{self.texture}]"
                f" at ({x},{y})")


# ---------------------------------------------------------------------------
# Flyweight Factory
# ---------------------------------------------------------------------------

class TreeTypeFactory:
    def __init__(self):
        self._cache: Dict[Tuple[str, str, str], TreeType] = {}

    def get_tree_type(self, name: str, colour: str, texture: str) -> TreeType:
        key = (name, colour, texture)
        if key not in self._cache:
            self._cache[key] = TreeType(name, colour, texture)
        return self._cache[key]

    @property
    def cache_size(self) -> int:
        return len(self._cache)


# ---------------------------------------------------------------------------
# Context (extrinsic state)
# ---------------------------------------------------------------------------

@dataclass
class Tree:
    x: int
    y: int
    type: TreeType

    def render(self) -> str:
        return self.type.render(self.x, self.y)


# ---------------------------------------------------------------------------
# Forest
# ---------------------------------------------------------------------------

class Forest:
    def __init__(self):
        self._factory = TreeTypeFactory()
        self._trees: List[Tree] = []

    def plant_tree(self, x: int, y: int,
                   name: str, colour: str, texture: str) -> None:
        tree_type = self._factory.get_tree_type(name, colour, texture)
        self._trees.append(Tree(x=x, y=y, type=tree_type))

    @property
    def tree_count(self) -> int:
        return len(self._trees)

    @property
    def unique_type_count(self) -> int:
        return self._factory.cache_size

    def tree_at(self, idx: int) -> Tree:
        return self._trees[idx]


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    forest = Forest()
    for i in range(5):
        forest.plant_tree(i, 0, "Oak",  "green",  "rough")
    for i in range(5):
        forest.plant_tree(i, 1, "Pine", "yellow", "smooth")

    print(f"Trees planted: {forest.tree_count}")
    print(f"Unique types:  {forest.unique_type_count}")
    for i in range(forest.tree_count):
        print(forest.tree_at(i).render())
