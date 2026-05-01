"""
Chain of Responsibility Pattern
================================
Intent: Pass a request along a chain of handlers. Each handler decides to
process the request or pass it to the next handler.

Real-world analogy: Technical support escalation:
Tier-1 handles basic issues; escalates to Tier-2; then Tier-3 engineering.
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Optional


# ---------------------------------------------------------------------------
# Request
# ---------------------------------------------------------------------------

@dataclass
class SupportRequest:
    level: int         # 1 = basic, 2 = intermediate, 3 = advanced
    description: str


# ---------------------------------------------------------------------------
# Abstract Handler
# ---------------------------------------------------------------------------

class SupportHandler(ABC):
    def __init__(self):
        self._next: Optional["SupportHandler"] = None

    def set_next(self, handler: "SupportHandler") -> "SupportHandler":
        self._next = handler
        return handler  # allows fluent chaining

    @abstractmethod
    def handle(self, request: SupportRequest) -> str: ...

    def _pass_to_next(self, request: SupportRequest) -> str:
        if self._next:
            return self._next.handle(request)
        return f"Unhandled: {request.description}"


# ---------------------------------------------------------------------------
# Concrete Handlers
# ---------------------------------------------------------------------------

class Tier1Handler(SupportHandler):
    def handle(self, request: SupportRequest) -> str:
        if request.level == 1:
            return f"Tier1 handled: {request.description}"
        return self._pass_to_next(request)


class Tier2Handler(SupportHandler):
    def handle(self, request: SupportRequest) -> str:
        if request.level == 2:
            return f"Tier2 handled: {request.description}"
        return self._pass_to_next(request)


class Tier3Handler(SupportHandler):
    def handle(self, request: SupportRequest) -> str:
        if request.level == 3:
            return f"Tier3 handled: {request.description}"
        return self._pass_to_next(request)


# ---------------------------------------------------------------------------
# Helper: build default chain (Tier1 → Tier2 → Tier3)
# ---------------------------------------------------------------------------

def build_default_chain() -> SupportHandler:
    t1, t2, t3 = Tier1Handler(), Tier2Handler(), Tier3Handler()
    t1.set_next(t2).set_next(t3)
    return t1


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    chain = build_default_chain()
    for req in (
        SupportRequest(1, "password reset"),
        SupportRequest(2, "network config"),
        SupportRequest(3, "kernel panic"),
        SupportRequest(4, "unknown level"),
    ):
        print(chain.handle(req))
