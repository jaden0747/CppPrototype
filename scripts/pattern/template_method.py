"""
Template Method Pattern
=======================
Intent: Define the skeleton of an algorithm in a base class, deferring some
steps to subclasses. Template Method lets subclasses redefine certain steps
without changing the algorithm's overall structure.

Real-world analogy: A data-mining pipeline. Steps are always:
open → extract → parse → analyse → build report → close.
Concrete miners (CSV, JSON) override only the format-specific steps.
"""

from __future__ import annotations
from abc import ABC, abstractmethod
from typing import List


# ---------------------------------------------------------------------------
# Abstract class with template method
# ---------------------------------------------------------------------------

class DataMiner(ABC):

    def __init__(self):
        self._raw_lines: List[str]    = []
        self._parsed_lines: List[str] = []
        self._analysis_note: str      = ""

    # Template method — not overridable by convention (leading underscore
    # is used in Python to signal intent; truly preventing override in
    # Python requires a metaclass, which is beyond this scope).
    def mine(self, path: str) -> str:
        self._open_file(path)
        self._extract_data()
        self._parse_data()
        self._analyse_data()
        report = self._build_report()
        self._close_file()
        return report

    # Primitive operations — must be overridden
    @abstractmethod
    def _open_file(self, path: str) -> None: ...

    @abstractmethod
    def _extract_data(self) -> None: ...

    @abstractmethod
    def _parse_data(self) -> None: ...

    # Hooks — have default implementations
    def _analyse_data(self) -> None:
        self._analysis_note = "(default analysis)"

    def _close_file(self) -> None:
        pass  # optional hook

    def _build_report(self) -> str:
        return (
            f"Report[{self._analysis_note}]: "
            f"{len(self._parsed_lines)} records"
        )

    # Accessors for testing
    @property
    def raw_lines(self) -> List[str]:
        return self._raw_lines

    @property
    def parsed_lines(self) -> List[str]:
        return self._parsed_lines


# ---------------------------------------------------------------------------
# Concrete: CSV miner
# ---------------------------------------------------------------------------

class CsvMiner(DataMiner):

    def _open_file(self, path: str) -> None:
        # Simulate: split "path" by semicolons (for easy testing)
        self._raw_lines = path.split(";")

    def _extract_data(self) -> None:
        pass  # data already in raw_lines

    def _parse_data(self) -> None:
        self._parsed_lines = [f"csv:{line}" for line in self._raw_lines]

    def _analyse_data(self) -> None:
        self._analysis_note = f"CSV analysis ({len(self._parsed_lines)} rows)"


# ---------------------------------------------------------------------------
# Concrete: JSON miner
# ---------------------------------------------------------------------------

class JsonMiner(DataMiner):

    def _open_file(self, path: str) -> None:
        self._raw_lines = [path]

    def _extract_data(self) -> None:
        pass

    def _parse_data(self) -> None:
        self._parsed_lines = [f"json:{self._raw_lines[0]}"]

    # Inherits default _analyse_data and _close_file


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    csv_miner  = CsvMiner()
    json_miner = JsonMiner()

    print(csv_miner.mine("Alice,30;Bob,25;Carol,35"))
    print(json_miner.mine('{"name":"Alice"}'))
