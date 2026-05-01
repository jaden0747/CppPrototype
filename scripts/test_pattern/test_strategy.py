"""Tests for the Strategy pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from strategy import Sorter, BubbleSort, QuickSort, ReverseSort, FunctionalSorter


def unsorted():
    return [5, 3, 8, 1, 9, 2]

EXPECTED_ASC  = [1, 2, 3, 5, 8, 9]
EXPECTED_DESC = [9, 8, 5, 3, 2, 1]


class TestStrategy(unittest.TestCase):

    def test_bubble_sort_ascending(self):
        sorter = Sorter(BubbleSort())
        data   = unsorted()
        sorter.sort(data)
        self.assertEqual(EXPECTED_ASC, data)

    def test_bubble_sort_name(self):
        self.assertEqual("BubbleSort", Sorter(BubbleSort()).strategy_name)

    def test_quick_sort_ascending(self):
        sorter = Sorter(QuickSort())
        data   = unsorted()
        sorter.sort(data)
        self.assertEqual(EXPECTED_ASC, data)

    def test_reverse_sort_descending(self):
        sorter = Sorter(ReverseSort())
        data   = unsorted()
        sorter.sort(data)
        self.assertEqual(EXPECTED_DESC, data)

    def test_swap_strategy_at_runtime(self):
        sorter = Sorter(BubbleSort())
        data1  = unsorted()
        sorter.sort(data1)
        self.assertEqual(EXPECTED_ASC, data1)

        sorter.set_strategy(ReverseSort())
        data2 = unsorted()
        sorter.sort(data2)
        self.assertEqual(EXPECTED_DESC, data2)
        self.assertEqual("ReverseSort", sorter.strategy_name)

    def test_sort_empty(self):
        sorter = Sorter(QuickSort())
        data   = []
        sorter.sort(data)
        self.assertEqual([], data)

    def test_sort_single_element(self):
        sorter = Sorter(BubbleSort())
        data   = [42]
        sorter.sort(data)
        self.assertEqual([42], data)


class TestFunctionalSorter(unittest.TestCase):

    def test_lambda_ascending(self):
        fs   = FunctionalSorter(lambda d: d.sort())
        data = unsorted()
        fs.sort(data)
        self.assertEqual(EXPECTED_ASC, data)

    def test_swap_lambda(self):
        fs   = FunctionalSorter(lambda d: d.sort())
        fs.set_strategy(lambda d: d.sort(reverse=True))
        data = unsorted()
        fs.sort(data)
        self.assertEqual(EXPECTED_DESC, data)


if __name__ == "__main__":
    unittest.main()
