"""Tests for the Interpreter pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from interpreter import Literal, Variable, NotExpr, AndExpr, OrExpr


class TestInterpreter(unittest.TestCase):

    # ------------------------------------------------------------------
    # Terminals
    # ------------------------------------------------------------------
    def test_literal_true(self):
        self.assertTrue(Literal(True).interpret({}))

    def test_literal_false(self):
        self.assertFalse(Literal(False).interpret({}))

    def test_variable_true(self):
        self.assertTrue(Variable("x").interpret({"x": True}))

    def test_variable_false(self):
        self.assertFalse(Variable("x").interpret({"x": False}))

    def test_undefined_variable_raises(self):
        with self.assertRaises(KeyError):
            Variable("missing").interpret({})

    # ------------------------------------------------------------------
    # NOT
    # ------------------------------------------------------------------
    def test_not_true(self):
        self.assertFalse(NotExpr(Literal(True)).interpret({}))

    def test_not_false(self):
        self.assertTrue(NotExpr(Literal(False)).interpret({}))

    # ------------------------------------------------------------------
    # AND
    # ------------------------------------------------------------------
    def test_and_true_true(self):
        self.assertTrue(AndExpr(Literal(True), Literal(True)).interpret({}))

    def test_and_true_false(self):
        self.assertFalse(AndExpr(Literal(True), Literal(False)).interpret({}))

    def test_and_false_false(self):
        self.assertFalse(AndExpr(Literal(False), Literal(False)).interpret({}))

    # ------------------------------------------------------------------
    # OR
    # ------------------------------------------------------------------
    def test_or_false_true(self):
        self.assertTrue(OrExpr(Literal(False), Literal(True)).interpret({}))

    def test_or_false_false(self):
        self.assertFalse(OrExpr(Literal(False), Literal(False)).interpret({}))

    # ------------------------------------------------------------------
    # Compound: (x AND y) OR (NOT z)
    # ------------------------------------------------------------------
    def test_compound_expression_true(self):
        expr = OrExpr(
            AndExpr(Variable("x"), Variable("y")),
            NotExpr(Variable("z"))
        )
        self.assertTrue(expr.interpret({"x": True, "y": False, "z": False}))

    def test_compound_expression_false(self):
        expr = OrExpr(
            AndExpr(Variable("x"), Variable("y")),
            NotExpr(Variable("z"))
        )
        self.assertFalse(expr.interpret({"x": False, "y": False, "z": True}))


if __name__ == "__main__":
    unittest.main()
