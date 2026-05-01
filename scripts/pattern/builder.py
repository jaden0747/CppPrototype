"""
Builder Pattern
===============
Intent: Separate the construction of a complex object from its representation
so that the same construction process can create different representations.

Real-world analogy: Building a custom burger step-by-step. The Director knows
the recipe; the Builder knows how to assemble its specific variant.
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from typing import List


# ---------------------------------------------------------------------------
# Product
# ---------------------------------------------------------------------------

@dataclass
class Burger:
    bun: str = ""
    patty: str = ""
    toppings: List[str] = field(default_factory=list)
    toasted: bool = False

    def describe(self) -> str:
        desc = f"[{self.bun}] with {self.patty} patty"
        if self.toasted:
            desc += " (toasted)"
        if self.toppings:
            desc += ", " + ", ".join(self.toppings)
        return desc


# ---------------------------------------------------------------------------
# Abstract Builder
# ---------------------------------------------------------------------------

class BurgerBuilder(ABC):
    @abstractmethod
    def set_bun(self, bun: str) -> None: ...
    @abstractmethod
    def set_patty(self, patty: str) -> None: ...
    @abstractmethod
    def add_topping(self, topping: str) -> None: ...
    @abstractmethod
    def set_toasted(self, toasted: bool) -> None: ...
    @abstractmethod
    def get_result(self) -> Burger: ...


# ---------------------------------------------------------------------------
# Concrete Builders
# ---------------------------------------------------------------------------

class MeatBurgerBuilder(BurgerBuilder):
    def __init__(self):
        self._reset()

    def _reset(self):
        self._burger = Burger(bun="sesame", patty="beef")

    def set_bun(self, bun: str)          -> None: self._burger.bun = bun
    def set_patty(self, patty: str)      -> None: self._burger.patty = patty
    def add_topping(self, topping: str)  -> None: self._burger.toppings.append(topping)
    def set_toasted(self, toasted: bool) -> None: self._burger.toasted = toasted

    def get_result(self) -> Burger:
        result = self._burger
        self._reset()
        return result


class VeggieBurgerBuilder(BurgerBuilder):
    def __init__(self):
        self._reset()

    def _reset(self):
        self._burger = Burger(bun="whole-wheat", patty="black-bean")

    def set_bun(self, bun: str)          -> None: self._burger.bun = bun
    def set_patty(self, patty: str)      -> None: self._burger.patty = patty
    def add_topping(self, topping: str)  -> None: self._burger.toppings.append(topping)
    def set_toasted(self, toasted: bool) -> None: self._burger.toasted = toasted

    def get_result(self) -> Burger:
        result = self._burger
        self._reset()
        return result


# ---------------------------------------------------------------------------
# Director
# ---------------------------------------------------------------------------

class BurgerDirector:
    def __init__(self, builder: BurgerBuilder):
        self._builder = builder

    def build_classic(self):
        self._builder.set_toasted(True)
        self._builder.add_topping("lettuce")
        self._builder.add_topping("tomato")
        self._builder.add_topping("cheese")

    def build_deluxe(self):
        self._builder.set_toasted(True)
        for t in ("lettuce", "tomato", "cheese", "pickles", "onion", "special sauce"):
            self._builder.add_topping(t)


# ---------------------------------------------------------------------------
# Fluent Builder
# ---------------------------------------------------------------------------

class FluentBurger:
    def __init__(self):
        self._burger = Burger()

    def with_bun(self, bun: str)         -> "FluentBurger": self._burger.bun = bun;          return self
    def with_patty(self, patty: str)     -> "FluentBurger": self._burger.patty = patty;      return self
    def with_topping(self, t: str)       -> "FluentBurger": self._burger.toppings.append(t); return self
    def toasted(self, v: bool = True)    -> "FluentBurger": self._burger.toasted = v;        return self
    def build(self)                      -> Burger:         return self._burger


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    # Director approach
    builder  = MeatBurgerBuilder()
    director = BurgerDirector(builder)
    director.build_classic()
    print("Classic meat:", builder.get_result().describe())

    builder2  = VeggieBurgerBuilder()
    director2 = BurgerDirector(builder2)
    director2.build_deluxe()
    print("Deluxe veggie:", builder2.get_result().describe())

    # Fluent approach
    b = FluentBurger().with_bun("brioche").with_patty("turkey").with_topping("avocado").toasted().build()
    print("Fluent:", b.describe())
