"""Tests for the Abstract Factory pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from abstract_factory import (
    ModernFurnitureFactory, VictorianFurnitureFactory,
    ModernChair, VictorianChair,
    make_furniture_factory
)


class TestAbstractFactory(unittest.TestCase):

    # ------------------------------------------------------------------
    # Modern factory
    # ------------------------------------------------------------------
    def test_modern_chair_style(self):
        f = ModernFurnitureFactory()
        self.assertEqual("Modern", f.create_chair().style())

    def test_modern_sofa_style(self):
        f = ModernFurnitureFactory()
        self.assertEqual("Modern", f.create_sofa().style())

    def test_modern_coffee_table_style(self):
        f = ModernFurnitureFactory()
        self.assertEqual("Modern", f.create_coffee_table().style())

    # ------------------------------------------------------------------
    # Victorian factory
    # ------------------------------------------------------------------
    def test_victorian_chair_style(self):
        f = VictorianFurnitureFactory()
        self.assertEqual("Victorian", f.create_chair().style())

    def test_victorian_sofa_style(self):
        f = VictorianFurnitureFactory()
        self.assertEqual("Victorian", f.create_sofa().style())

    def test_victorian_coffee_table_style(self):
        f = VictorianFurnitureFactory()
        self.assertEqual("Victorian", f.create_coffee_table().style())

    # ------------------------------------------------------------------
    # Family consistency
    # ------------------------------------------------------------------
    def test_modern_family_consistent(self):
        f = ModernFurnitureFactory()
        self.assertEqual(f.create_chair().style(), f.create_sofa().style())
        self.assertEqual(f.create_sofa().style(),  f.create_coffee_table().style())

    def test_victorian_family_consistent(self):
        f = VictorianFurnitureFactory()
        self.assertEqual(f.create_chair().style(), f.create_sofa().style())
        self.assertEqual(f.create_sofa().style(),  f.create_coffee_table().style())

    # ------------------------------------------------------------------
    # Helper
    # ------------------------------------------------------------------
    def test_make_factory_modern(self):
        f = make_furniture_factory("modern")
        self.assertIsInstance(f, ModernFurnitureFactory)

    def test_make_factory_victorian(self):
        f = make_furniture_factory("victorian")
        self.assertIsInstance(f, VictorianFurnitureFactory)

    def test_make_factory_unknown_raises(self):
        with self.assertRaises(ValueError):
            make_furniture_factory("art-deco")

    # ------------------------------------------------------------------
    # Each creation is a new object
    # ------------------------------------------------------------------
    def test_each_creation_is_unique(self):
        f = ModernFurnitureFactory()
        self.assertIsNot(f.create_chair(), f.create_chair())


if __name__ == "__main__":
    unittest.main()
