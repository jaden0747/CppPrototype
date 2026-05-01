"""Tests for the Visitor pattern."""

import sys
import os
import math
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from visitor import Circle, Rectangle, Triangle, AreaVisitor, PerimeterVisitor, NameCollectorVisitor

EPS = 1e-9


class TestAreaVisitor(unittest.TestCase):

    def test_circle_area(self):
        v = AreaVisitor()
        Circle(5.0).accept(v)
        self.assertAlmostEqual(math.pi * 25.0, v.total, places=9)

    def test_rectangle_area(self):
        v = AreaVisitor()
        Rectangle(4.0, 3.0).accept(v)
        self.assertAlmostEqual(12.0, v.total, places=9)

    def test_triangle_area_3_4_5(self):
        v = AreaVisitor()
        Triangle(3.0, 4.0, 5.0).accept(v)
        self.assertAlmostEqual(6.0, v.total, places=9)

    def test_accumulates(self):
        v = AreaVisitor()
        Rectangle(2.0, 3.0).accept(v)
        Circle(1.0).accept(v)
        self.assertAlmostEqual(6.0 + math.pi, v.total, places=9)

    def test_reset(self):
        v = AreaVisitor()
        Rectangle(2.0, 3.0).accept(v)
        v.reset()
        self.assertEqual(0.0, v.total)


class TestPerimeterVisitor(unittest.TestCase):

    def test_circle_perimeter(self):
        v = PerimeterVisitor()
        Circle(5.0).accept(v)
        self.assertAlmostEqual(2 * math.pi * 5.0, v.total, places=9)

    def test_rectangle_perimeter(self):
        v = PerimeterVisitor()
        Rectangle(4.0, 3.0).accept(v)
        self.assertAlmostEqual(14.0, v.total, places=9)

    def test_triangle_perimeter(self):
        v = PerimeterVisitor()
        Triangle(3.0, 4.0, 5.0).accept(v)
        self.assertAlmostEqual(12.0, v.total, places=9)


class TestNameCollectorVisitor(unittest.TestCase):

    def test_names_in_order(self):
        shapes = [Circle(1.0), Rectangle(2.0, 3.0), Triangle(3.0, 4.0, 5.0)]
        v = NameCollectorVisitor()
        for s in shapes:
            s.accept(v)
        self.assertEqual(["Circle", "Rectangle", "Triangle"], v.names)


if __name__ == "__main__":
    unittest.main()
