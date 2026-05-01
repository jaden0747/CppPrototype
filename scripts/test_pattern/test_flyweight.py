"""Tests for the Flyweight pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from flyweight import TreeType, TreeTypeFactory, Tree, Forest


class TestFlyweight(unittest.TestCase):

    # ------------------------------------------------------------------
    # TreeType
    # ------------------------------------------------------------------
    def test_tree_type_stores_name(self):
        self.assertEqual("Oak", TreeType("Oak", "green", "rough").name)

    def test_tree_type_render_contains_coords(self):
        t = TreeType("Oak", "green", "rough")
        result = t.render(10, 20)
        self.assertIn("10", result)
        self.assertIn("20", result)

    # ------------------------------------------------------------------
    # Factory sharing
    # ------------------------------------------------------------------
    def test_factory_returns_same_instance_for_same_key(self):
        f = TreeTypeFactory()
        a = f.get_tree_type("Oak", "green", "rough")
        b = f.get_tree_type("Oak", "green", "rough")
        self.assertIs(a, b)

    def test_factory_returns_different_instance_for_different_key(self):
        f = TreeTypeFactory()
        a = f.get_tree_type("Oak",  "green",  "rough")
        b = f.get_tree_type("Pine", "yellow", "smooth")
        self.assertIsNot(a, b)

    def test_factory_cache_size_grows_for_new_types(self):
        f = TreeTypeFactory()
        f.get_tree_type("Oak",  "green",  "rough")
        self.assertEqual(1, f.cache_size)
        f.get_tree_type("Pine", "yellow", "smooth")
        self.assertEqual(2, f.cache_size)
        f.get_tree_type("Oak",  "green",  "rough")  # duplicate
        self.assertEqual(2, f.cache_size)

    # ------------------------------------------------------------------
    # Forest
    # ------------------------------------------------------------------
    def test_forest_tracks_all_trees(self):
        f = Forest()
        f.plant_tree(0, 0, "Oak",  "green",  "rough")
        f.plant_tree(1, 2, "Oak",  "green",  "rough")
        f.plant_tree(3, 4, "Pine", "yellow", "smooth")
        self.assertEqual(3, f.tree_count)

    def test_forest_deduplicates_types(self):
        f = Forest()
        for i in range(100):
            f.plant_tree(i, i, "Oak", "green", "rough")
        self.assertEqual(100, f.tree_count)
        self.assertEqual(1,   f.unique_type_count)

    def test_forest_multiple_types_deduplication(self):
        f = Forest()
        for i in range(50): f.plant_tree(i, 0, "Oak",  "green",  "rough")
        for i in range(50): f.plant_tree(i, 1, "Pine", "yellow", "smooth")
        self.assertEqual(100, f.tree_count)
        self.assertEqual(2,   f.unique_type_count)

    def test_shared_type_identity(self):
        f = Forest()
        f.plant_tree(0, 0, "Oak", "green", "rough")
        f.plant_tree(1, 1, "Oak", "green", "rough")
        self.assertIs(f.tree_at(0).type, f.tree_at(1).type)

    def test_context_render_contains_position(self):
        f = Forest()
        f.plant_tree(42, 7, "Oak", "green", "rough")
        r = f.tree_at(0).render()
        self.assertIn("42", r)
        self.assertIn("7",  r)


if __name__ == "__main__":
    unittest.main()
