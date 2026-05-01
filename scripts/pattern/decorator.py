"""
Decorator Pattern
=================
Intent: Attach additional responsibilities to an object dynamically without
modifying the class. Provides a flexible alternative to subclassing.

Real-world analogy: Building a coffee order — start with Espresso, wrap with
Milk, Caramel, Whip. Each wrapper adds its own cost and description.
"""

from abc import ABC, abstractmethod


# ---------------------------------------------------------------------------
# Component interface
# ---------------------------------------------------------------------------

class Coffee(ABC):
    @abstractmethod
    def cost(self) -> float: ...

    @abstractmethod
    def description(self) -> str: ...


# ---------------------------------------------------------------------------
# Concrete components
# ---------------------------------------------------------------------------

class Espresso(Coffee):
    def cost(self) -> float:        return 1.00
    def description(self) -> str:   return "Espresso"


class SimpleCoffee(Coffee):
    def cost(self) -> float:        return 0.50
    def description(self) -> str:   return "Simple coffee"


# ---------------------------------------------------------------------------
# Base Decorator
# ---------------------------------------------------------------------------

class CoffeeDecorator(Coffee):
    def __init__(self, wrappee: Coffee):
        self._wrappee = wrappee

    def cost(self) -> float:        return self._wrappee.cost()
    def description(self) -> str:   return self._wrappee.description()


# ---------------------------------------------------------------------------
# Concrete Decorators
# ---------------------------------------------------------------------------

class MilkDecorator(CoffeeDecorator):
    def cost(self) -> float:        return self._wrappee.cost() + 0.25
    def description(self) -> str:   return self._wrappee.description() + ", Milk"


class CaramelDecorator(CoffeeDecorator):
    def cost(self) -> float:        return self._wrappee.cost() + 0.50
    def description(self) -> str:   return self._wrappee.description() + ", Caramel"


class WhipDecorator(CoffeeDecorator):
    def cost(self) -> float:        return self._wrappee.cost() + 0.30
    def description(self) -> str:   return self._wrappee.description() + ", Whip"


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    order = WhipDecorator(CaramelDecorator(MilkDecorator(Espresso())))
    print(f"{order.description()} = ${order.cost():.2f}")

    simple = MilkDecorator(SimpleCoffee())
    print(f"{simple.description()} = ${simple.cost():.2f}")
