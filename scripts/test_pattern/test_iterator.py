"""Tests for the Iterator pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from iterator import WordCollection, ForwardIterator, ReverseIterator, NumberRange


class TestIterator(unittest.TestCase):

    # ------------------------------------------------------------------
    # ForwardIterator
    # ------------------------------------------------------------------
    def test_forward_iterates_all_words(self):
        col = WordCollection()
        for w in ("alpha", "beta", "gamma"):
            col.add(w)
        result = list(col.forward_iterator())
        self.assertEqual(["alpha", "beta", "gamma"], result)

    def test_forward_empty_collection(self):
        col = WordCollection()
        self.assertFalse(col.forward_iterator().has_next())

    def test_forward_exhausted_raises(self):
        col = WordCollection()
        col.add("only")
        it = col.forward_iterator()
        it.next()
        with self.assertRaises(StopIteration):
            it.next()

    # ------------------------------------------------------------------
    # ReverseIterator
    # ------------------------------------------------------------------
    def test_reverse_iterates_in_reverse(self):
        col = WordCollection()
        for w in ("a", "b", "c"):
            col.add(w)
        result = list(col.reverse_iterator())
        self.assertEqual(["c", "b", "a"], result)

    def test_reverse_empty_collection(self):
        col = WordCollection()
        self.assertFalse(col.reverse_iterator().has_next())

    def test_reverse_exhausted_raises(self):
        col = WordCollection()
        col.add("x")
        it = col.reverse_iterator()
        it.next()
        with self.assertRaises(StopIteration):
            it.next()

    # ------------------------------------------------------------------
    # NumberRange (generator)
    # ------------------------------------------------------------------
    def test_number_range_generates_correct_sequence(self):
        self.assertEqual([1, 2, 3, 4, 5], list(NumberRange(1, 5)))

    def test_number_range_sum(self):
        self.assertEqual(55, sum(NumberRange(1, 10)))

    def test_number_range_empty_when_from_greater_than_to(self):
        self.assertEqual([], list(NumberRange(5, 4)))

    def test_number_range_single_element(self):
        self.assertEqual([7], list(NumberRange(7, 7)))

    # ------------------------------------------------------------------
    # Collection is directly iterable
    # ------------------------------------------------------------------
    def test_collection_direct_iteration(self):
        col = WordCollection()
        col.add("x"); col.add("y")
        self.assertEqual(["x", "y"], list(col))


if __name__ == "__main__":
    unittest.main()
