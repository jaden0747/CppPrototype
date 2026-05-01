"""Tests for the Composite pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from composite import File, Directory


class TestComposite(unittest.TestCase):

    # ------------------------------------------------------------------
    # Leaf (File)
    # ------------------------------------------------------------------
    def test_file_returns_size(self):
        self.assertEqual(1024, File("doc.txt", 1024).size())

    def test_file_is_not_directory(self):
        self.assertFalse(File("img.png", 512).is_directory())

    def test_file_name(self):
        self.assertEqual("readme.md", File("readme.md", 256).name)

    # ------------------------------------------------------------------
    # Directory
    # ------------------------------------------------------------------
    def test_empty_directory_size_is_zero(self):
        self.assertEqual(0, Directory("empty").size())

    def test_directory_is_directory(self):
        self.assertTrue(Directory("home").is_directory())

    def test_directory_size_is_child_sum(self):
        d = Directory("root")
        d.add(File("a.txt", 100))
        d.add(File("b.txt", 200))
        self.assertEqual(300, d.size())

    def test_nested_directory_sizes(self):
        inner = Directory("inner")
        inner.add(File("x.cpp", 500))
        inner.add(File("y.cpp", 300))

        outer = Directory("outer")
        outer.add(File("main.cpp", 200))
        outer.add(inner)

        self.assertEqual(1000, outer.size())

    def test_child_count(self):
        d = Directory("src")
        d.add(File("a.cpp", 10))
        d.add(File("b.cpp", 20))
        self.assertEqual(2, len(d.children))

    # ------------------------------------------------------------------
    # Deep nesting
    # ------------------------------------------------------------------
    def test_deep_nested_sizes(self):
        c = Directory("c")
        c.add(File("file.txt", 999))
        b = Directory("b")
        b.add(c)
        a = Directory("a")
        a.add(b)
        root = Directory("root")
        root.add(a)
        self.assertEqual(999, root.size())


if __name__ == "__main__":
    unittest.main()
