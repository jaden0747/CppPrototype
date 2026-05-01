"""
Factory Method Pattern
======================
Intent: Define an interface for creating an object, but let subclasses decide
which class to instantiate.

Real-world analogy: Road Logistics creates Trucks; Sea Logistics creates Ships.
The delivery algorithm (planDelivery) stays the same — only the product differs.
"""

from abc import ABC, abstractmethod


# ---------------------------------------------------------------------------
# Product hierarchy
# ---------------------------------------------------------------------------

class Transport(ABC):
    @abstractmethod
    def deliver(self) -> str: ...

    @abstractmethod
    def type(self) -> str: ...


class Truck(Transport):
    def deliver(self) -> str:
        return "Delivering by land in a truck"

    def type(self) -> str:
        return "Truck"


class Ship(Transport):
    def deliver(self) -> str:
        return "Delivering by sea in a ship"

    def type(self) -> str:
        return "Ship"


class Plane(Transport):
    def deliver(self) -> str:
        return "Delivering by air in a plane"

    def type(self) -> str:
        return "Plane"


# ---------------------------------------------------------------------------
# Creator hierarchy
# ---------------------------------------------------------------------------

class Logistics(ABC):
    @abstractmethod
    def create_transport(self) -> Transport: ...

    def plan_delivery(self) -> str:
        transport = self.create_transport()
        return f"[{transport.type()}] {transport.deliver()}"


class RoadLogistics(Logistics):
    def create_transport(self) -> Transport:
        return Truck()


class SeaLogistics(Logistics):
    def create_transport(self) -> Transport:
        return Ship()


class AirLogistics(Logistics):
    def create_transport(self) -> Transport:
        return Plane()


# ---------------------------------------------------------------------------
# Helper factory function
# ---------------------------------------------------------------------------

def make_logistics(kind: str) -> Logistics:
    mapping = {
        "road": RoadLogistics,
        "sea":  SeaLogistics,
        "air":  AirLogistics,
    }
    if kind not in mapping:
        raise ValueError(f"Unknown logistics kind: {kind}")
    return mapping[kind]()


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    for kind in ("road", "sea", "air"):
        logistics = make_logistics(kind)
        print(logistics.plan_delivery())
