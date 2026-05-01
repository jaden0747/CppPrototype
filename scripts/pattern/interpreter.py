"""
Interpreter Pattern
===================
Intent: Define a grammar for a language and provide an interpreter to evaluate
sentences in that language.

Example domain: A simple Boolean expression evaluator.
  Grammar: var, true, false, NOT expr, (expr AND expr), (expr OR expr)
"""

from abc import ABC, abstractmethod
from typing import Dict


Context = Dict[str, bool]


# ---------------------------------------------------------------------------
# Abstract Expression
# ---------------------------------------------------------------------------

class BoolExpr(ABC):
    @abstractmethod
    def interpret(self, ctx: Context) -> bool: ...


# ---------------------------------------------------------------------------
# Terminal Expressions
# ---------------------------------------------------------------------------

class Literal(BoolExpr):
    def __init__(self, value: bool):
        self._value = value

    def interpret(self, ctx: Context) -> bool:
        return self._value


class Variable(BoolExpr):
    def __init__(self, name: str):
        self._name = name

    def interpret(self, ctx: Context) -> bool:
        if self._name not in ctx:
            raise KeyError(f"Undefined variable: {self._name}")
        return ctx[self._name]


# ---------------------------------------------------------------------------
# Non-Terminal Expressions
# ---------------------------------------------------------------------------

class NotExpr(BoolExpr):
    def __init__(self, operand: BoolExpr):
        self._operand = operand

    def interpret(self, ctx: Context) -> bool:
        return not self._operand.interpret(ctx)


class AndExpr(BoolExpr):
    def __init__(self, left: BoolExpr, right: BoolExpr):
        self._left  = left
        self._right = right

    def interpret(self, ctx: Context) -> bool:
        return self._left.interpret(ctx) and self._right.interpret(ctx)


class OrExpr(BoolExpr):
    def __init__(self, left: BoolExpr, right: BoolExpr):
        self._left  = left
        self._right = right

    def interpret(self, ctx: Context) -> bool:
        return self._left.interpret(ctx) or self._right.interpret(ctx)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    # (x AND y) OR (NOT z)
    expr = OrExpr(
        AndExpr(Variable("x"), Variable("y")),
        NotExpr(Variable("z"))
    )

    tests = [
        ({"x": True,  "y": True,  "z": True},  True),
        ({"x": True,  "y": False, "z": False}, True),
        ({"x": False, "y": False, "z": True},  False),
    ]

    for ctx, expected in tests:
        result = expr.interpret(ctx)
        status = "OK" if result == expected else "FAIL"
        print(f"{ctx} => {result} [{status}]")
