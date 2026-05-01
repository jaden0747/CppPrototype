"""Tests for the Decorator pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from decorator import (
    Espresso, SimpleCoffee,
    MilkDecorator, CaramelDecorator, WhipDecorator
)


class TestDecorator(unittest.TestCase):

    EPS = 1e-9

    # ------------------------------------------------------------------
    # Base component
    # ------------------------------------------------------------------
    def test_espresso_cost(self):
        self.assertAlmostEqual(1.00, Espresso().cost())

    def test_espresso_description(self):
        self.assertEqual("Espresso", Espresso().description())

    # ------------------------------------------------------------------
    # Single decorator
    # ------------------------------------------------------------------
    def test_milk_adds_cost(self):
        self.assertAlmostEqual(1.25, MilkDecorator(Espresso()).cost())

    def test_milk_adds_description(self):
        desc = MilkDecorator(Espresso()).description()
        self.assertIn("Milk",     desc)
        self.assertIn("Espresso", desc)

    def test_caramel_adds_cost(self):
        self.assertAlmostEqual(1.50, CaramelDecorator(Espresso()).cost())

    # ------------------------------------------------------------------
    # Stacked decorators
    # ------------------------------------------------------------------
    def test_milk_and_caramel_cost(self):
        c = CaramelDecorator(MilkDecorator(Espresso()))
        self.assertAlmostEqual(1.75, c.cost())

    def test_triple_stack_cost(self):
        c = WhipDecorator(CaramelDecorator(MilkDecorator(Espresso())))
        self.assertAlmostEqual(2.05, c.cost())

    def test_triple_stack_description(self):
        c = WhipDecorator(CaramelDecorator(MilkDecorator(Espresso())))
        desc = c.description()
        for word in ("Espresso", "Milk", "Caramel", "Whip"):
            self.assertIn(word, desc)

    # ------------------------------------------------------------------
    # Same decorator twice
    # ------------------------------------------------------------------
    def test_double_milk(self):
        c = MilkDecorator(MilkDecorator(Espresso()))
        self.assertAlmostEqual(1.50, c.cost())

    # ------------------------------------------------------------------
    # Polymorphic use
    # ------------------------------------------------------------------
    def test_simple_coffee_with_milk(self):
        self.assertAlmostEqual(0.75, MilkDecorator(SimpleCoffee()).cost())


if __name__ == "__main__":
    unittest.main()
