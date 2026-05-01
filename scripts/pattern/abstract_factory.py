"""
Abstract Factory Pattern
========================
Intent: Provide an interface for creating families of related objects without
specifying their concrete classes.

Real-world analogy: A furniture shop has Modern and Victorian product lines.
Pick a factory (style) and get a consistent family of products.
"""

from abc import ABC, abstractmethod


# ---------------------------------------------------------------------------
# Product interfaces
# ---------------------------------------------------------------------------

class Chair(ABC):
    @abstractmethod
    def sit_on(self) -> str: ...
    @abstractmethod
    def style(self) -> str: ...

class Sofa(ABC):
    @abstractmethod
    def lie_on(self) -> str: ...
    @abstractmethod
    def style(self) -> str: ...

class CoffeeTable(ABC):
    @abstractmethod
    def put_on(self) -> str: ...
    @abstractmethod
    def style(self) -> str: ...


# ---------------------------------------------------------------------------
# Modern family
# ---------------------------------------------------------------------------

class ModernChair(Chair):
    def sit_on(self) -> str: return "Sitting on a sleek modern chair"
    def style(self)  -> str: return "Modern"

class ModernSofa(Sofa):
    def lie_on(self) -> str: return "Lying on a minimalist modern sofa"
    def style(self)  -> str: return "Modern"

class ModernCoffeeTable(CoffeeTable):
    def put_on(self) -> str: return "Placing items on a glass modern table"
    def style(self)  -> str: return "Modern"


# ---------------------------------------------------------------------------
# Victorian family
# ---------------------------------------------------------------------------

class VictorianChair(Chair):
    def sit_on(self) -> str: return "Sitting on an ornate Victorian chair"
    def style(self)  -> str: return "Victorian"

class VictorianSofa(Sofa):
    def lie_on(self) -> str: return "Lying on a velvet Victorian sofa"
    def style(self)  -> str: return "Victorian"

class VictorianCoffeeTable(CoffeeTable):
    def put_on(self) -> str: return "Placing items on a carved Victorian table"
    def style(self)  -> str: return "Victorian"


# ---------------------------------------------------------------------------
# Abstract factory
# ---------------------------------------------------------------------------

class FurnitureFactory(ABC):
    @abstractmethod
    def create_chair(self) -> Chair: ...
    @abstractmethod
    def create_sofa(self) -> Sofa: ...
    @abstractmethod
    def create_coffee_table(self) -> CoffeeTable: ...


class ModernFurnitureFactory(FurnitureFactory):
    def create_chair(self)        -> Chair:       return ModernChair()
    def create_sofa(self)         -> Sofa:        return ModernSofa()
    def create_coffee_table(self) -> CoffeeTable: return ModernCoffeeTable()


class VictorianFurnitureFactory(FurnitureFactory):
    def create_chair(self)        -> Chair:       return VictorianChair()
    def create_sofa(self)         -> Sofa:        return VictorianSofa()
    def create_coffee_table(self) -> CoffeeTable: return VictorianCoffeeTable()


# ---------------------------------------------------------------------------
# Helper
# ---------------------------------------------------------------------------

def make_furniture_factory(style: str) -> FurnitureFactory:
    mapping = {
        "modern":    ModernFurnitureFactory,
        "victorian": VictorianFurnitureFactory,
    }
    if style not in mapping:
        raise ValueError(f"Unknown furniture style: {style}")
    return mapping[style]()


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    for style in ("modern", "victorian"):
        factory = make_furniture_factory(style)
        chair = factory.create_chair()
        sofa  = factory.create_sofa()
        table = factory.create_coffee_table()
        print(f"--- {style.capitalize()} ---")
        print(chair.sit_on())
        print(sofa.lie_on())
        print(table.put_on())
