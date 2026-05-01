"""Tests for the Prototype pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from prototype import Circle, Rectangle, ShapeRegistry


class TestPrototype(unittest.TestCase):

    # ------------------------------------------------------------------
    # Basic clone
    # ------------------------------------------------------------------
    def test_clone_produces_new_object(self):
        c = Circle("red", 5.0)
        self.assertIsNot(c, c.clone())

    def test_clone_preserves_type(self):
        c = Circle("blue", 3.0)
        self.assertEqual("Circle", c.clone().type())

    def test_clone_preserves_color(self):
        c = Circle("green", 4.0)
        self.assertEqual("green", c.clone().color)

    def test_clone_circle_preserves_radius(self):
        c = Circle("red", 7.5)
        clone = c.clone()
        self.assertAlmostEqual(7.5, clone.radius)

    def test_clone_rectangle_preserves_dimensions(self):
        r = Rectangle("yellow", 10.0, 20.0)
        clone = r.clone()
        self.assertAlmostEqual(10.0, clone.width)
        self.assertAlmostEqual(20.0, clone.height)

    # ------------------------------------------------------------------
    # Independence
    # ------------------------------------------------------------------
    def test_mutating_clone_does_not_affect_original(self):
        c = Circle("red", 5.0)
        clone = c.clone()
        clone.color = "blue"
        self.assertEqual("red",  c.color)
        self.assertEqual("blue", clone.color)

    def test_mutating_radius_does_not_affect_original(self):
        c = Circle("red", 5.0)
        clone = c.clone()
        clone.radius = 99.0
        self.assertAlmostEqual(5.0, c.radius)
        self.assertAlmostEqual(99.0, clone.radius)

    # ------------------------------------------------------------------
    # Registry
    # ------------------------------------------------------------------
    def test_registry_returns_clones_not_originals(self):
        reg = ShapeRegistry()
        reg.add("circle", Circle("red", 1.0))
        a = reg.get("circle")
        b = reg.get("circle")
        self.assertIsNot(a, b)

    def test_registry_preserves_properties(self):
        reg = ShapeRegistry()
        reg.add("rect", Rectangle("blue", 4.0, 8.0))
        s = reg.get("rect")
        self.assertEqual("blue",      s.color)
        self.assertEqual("Rectangle", s.type())

    def test_registry_raises_on_missing_key(self):
        reg = ShapeRegistry()
        with self.assertRaises(KeyError):
            reg.get("nonexistent")

    def test_registry_contains(self):
        reg = ShapeRegistry()
        reg.add("circle", Circle("red", 1.0))
        self.assertIn("circle", reg)
        self.assertNotIn("triangle", reg)


if __name__ == "__main__":
    unittest.main()
