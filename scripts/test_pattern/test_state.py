"""Tests for the State pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from state import VendingMachine


class TestState(unittest.TestCase):

    # ------------------------------------------------------------------
    # Initial state
    # ------------------------------------------------------------------
    def test_initial_state_is_idle(self):
        vm = VendingMachine(3)
        self.assertEqual("Idle", vm.state_name)

    def test_zero_stock_is_out_of_stock(self):
        vm = VendingMachine(0)
        self.assertEqual("OutOfStock", vm.state_name)

    # ------------------------------------------------------------------
    # Happy path
    # ------------------------------------------------------------------
    def test_insert_coin_transitions_to_has_coin(self):
        vm = VendingMachine(1)
        vm.insert_coin()
        self.assertEqual("HasCoin", vm.state_name)

    def test_select_product_transitions_to_dispensing(self):
        vm = VendingMachine(1)
        vm.insert_coin()
        vm.select_product()
        self.assertEqual("Dispensing", vm.state_name)

    def test_dispense_reduces_stock(self):
        vm = VendingMachine(2)
        vm.insert_coin()
        vm.select_product()
        vm.dispense()
        self.assertEqual(1, vm.stock)

    def test_dispense_returns_to_idle(self):
        vm = VendingMachine(2)
        vm.insert_coin()
        vm.select_product()
        vm.dispense()
        self.assertEqual("Idle", vm.state_name)

    def test_last_item_goes_to_out_of_stock(self):
        vm = VendingMachine(1)
        vm.insert_coin()
        vm.select_product()
        vm.dispense()
        self.assertEqual("OutOfStock", vm.state_name)
        self.assertEqual(0, vm.stock)

    # ------------------------------------------------------------------
    # Error paths
    # ------------------------------------------------------------------
    def test_select_before_coin_raises(self):
        vm = VendingMachine(1)
        with self.assertRaises(RuntimeError):
            vm.select_product()

    def test_dispense_before_coin_raises(self):
        vm = VendingMachine(1)
        with self.assertRaises(RuntimeError):
            vm.dispense()

    def test_insert_coin_twice_raises(self):
        vm = VendingMachine(1)
        vm.insert_coin()
        with self.assertRaises(RuntimeError):
            vm.insert_coin()

    def test_out_of_stock_insert_raises(self):
        vm = VendingMachine(0)
        with self.assertRaises(RuntimeError):
            vm.insert_coin()


if __name__ == "__main__":
    unittest.main()
