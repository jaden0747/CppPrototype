"""
Strategy Pattern
================
Intent: Define a family of algorithms, encapsulate each one, and make them
interchangeable. Strategy lets the algorithm vary independently from the
clients that use it.

Real-world analogy: A navigation app that can switch between "fastest route",
"shortest route", and "avoid tolls" without changing the app itself.
"""

from __future__ import annotations
from abc import ABC, abstractmethod
from typing import List, Callable


# ---------------------------------------------------------------------------
# Strategy interface
# ---------------------------------------------------------------------------

class ISortStrategy(ABC):
    @abstractmethod
    def sort(self, data: List[int]) -> None: ...

    @property
    @abstractmethod
    def name(self) -> str: ...


# ---------------------------------------------------------------------------
# Concrete Strategies
# ---------------------------------------------------------------------------

class BubbleSort(ISortStrategy):
    def sort(self, data: List[int]) -> None:
        n = len(data)
        for i in range(n):
            for j in range(n - i - 1):
                if data[j] > data[j + 1]:
                    data[j], data[j + 1] = data[j + 1], data[j]

    @property
    def name(self) -> str:
        return "BubbleSort"


class QuickSort(ISortStrategy):
    def sort(self, data: List[int]) -> None:
        data.sort()

    @property
    def name(self) -> str:
        return "QuickSort"


class ReverseSort(ISortStrategy):
    def sort(self, data: List[int]) -> None:
        data.sort(reverse=True)

    @property
    def name(self) -> str:
        return "ReverseSort"


# ---------------------------------------------------------------------------
# Context
# ---------------------------------------------------------------------------

class Sorter:
    def __init__(self, strategy: ISortStrategy):
        self._strategy = strategy

    def set_strategy(self, strategy: ISortStrategy) -> None:
        self._strategy = strategy

    def sort(self, data: List[int]) -> None:
        self._strategy.sort(data)

    @property
    def strategy_name(self) -> str:
        return self._strategy.name


# ---------------------------------------------------------------------------
# Functional variant (callable / lambda as strategy)
# ---------------------------------------------------------------------------

class FunctionalSorter:
    def __init__(self, strategy: Callable[[List[int]], None]):
        self._strategy = strategy

    def set_strategy(self, strategy: Callable[[List[int]], None]) -> None:
        self._strategy = strategy

    def sort(self, data: List[int]) -> None:
        self._strategy(data)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    data = [5, 3, 8, 1, 9, 2]

    sorter = Sorter(BubbleSort())
    sorter.sort(data)
    print("BubbleSort:", data)

    data = [5, 3, 8, 1, 9, 2]
    sorter.set_strategy(ReverseSort())
    sorter.sort(data)
    print("ReverseSort:", data)

    # Functional variant
    data = [5, 3, 8, 1, 9, 2]
    fs = FunctionalSorter(lambda d: d.sort())
    fs.sort(data)
    print("Lambda sort:", data)
