"""
State Pattern
=============
Intent: Allow an object to alter its behaviour when its internal state
changes. The object will appear to change its class.

Real-world analogy: A vending machine — each state (idle, has-coin,
dispensing, out-of-stock) controls which actions are legal.
"""

from __future__ import annotations
from abc import ABC, abstractmethod


# ---------------------------------------------------------------------------
# Forward declaration trick — use string annotation
# ---------------------------------------------------------------------------

class VendingState(ABC):
    @abstractmethod
    def insert_coin(self, vm: "VendingMachine") -> None: ...

    @abstractmethod
    def select_product(self, vm: "VendingMachine") -> None: ...

    @abstractmethod
    def dispense(self, vm: "VendingMachine") -> None: ...

    @property
    @abstractmethod
    def name(self) -> str: ...


# ---------------------------------------------------------------------------
# Concrete States
# ---------------------------------------------------------------------------

class IdleState(VendingState):
    def insert_coin(self, vm):
        vm.set_state(HasCoinState())

    def select_product(self, vm):
        raise RuntimeError("Insert coin first")

    def dispense(self, vm):
        raise RuntimeError("Insert coin first")

    @property
    def name(self):
        return "Idle"


class HasCoinState(VendingState):
    def insert_coin(self, vm):
        raise RuntimeError("Coin already inserted")

    def select_product(self, vm):
        vm.set_state(DispensingState())

    def dispense(self, vm):
        raise RuntimeError("Select a product first")

    @property
    def name(self):
        return "HasCoin"


class DispensingState(VendingState):
    def insert_coin(self, vm):
        raise RuntimeError("Please wait, dispensing")

    def select_product(self, vm):
        raise RuntimeError("Already dispensing")

    def dispense(self, vm):
        vm.decrease_stock()
        if vm.stock > 0:
            vm.set_state(IdleState())
        else:
            vm.set_state(OutOfStockState())

    @property
    def name(self):
        return "Dispensing"


class OutOfStockState(VendingState):
    def insert_coin(self, vm):
        raise RuntimeError("Out of stock")

    def select_product(self, vm):
        raise RuntimeError("Out of stock")

    def dispense(self, vm):
        raise RuntimeError("Out of stock")

    @property
    def name(self):
        return "OutOfStock"


# ---------------------------------------------------------------------------
# Context
# ---------------------------------------------------------------------------

class VendingMachine:
    def __init__(self, stock: int):
        self._stock = stock
        self._state: VendingState = IdleState() if stock > 0 else OutOfStockState()

    def set_state(self, state: VendingState) -> None:
        self._state = state

    def insert_coin(self) -> None:
        self._state.insert_coin(self)

    def select_product(self) -> None:
        self._state.select_product(self)

    def dispense(self) -> None:
        self._state.dispense(self)

    @property
    def state_name(self) -> str:
        return self._state.name

    @property
    def stock(self) -> int:
        return self._stock

    def decrease_stock(self) -> None:
        if self._stock > 0:
            self._stock -= 1


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    vm = VendingMachine(2)
    print("State:", vm.state_name)   # Idle

    vm.insert_coin()
    print("State:", vm.state_name)   # HasCoin

    vm.select_product()
    print("State:", vm.state_name)   # Dispensing

    vm.dispense()
    print("State:", vm.state_name, "| stock:", vm.stock)  # Idle, 1

    vm.insert_coin()
    vm.select_product()
    vm.dispense()
    print("State:", vm.state_name, "| stock:", vm.stock)  # OutOfStock, 0
