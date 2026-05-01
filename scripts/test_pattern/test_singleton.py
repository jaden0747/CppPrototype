"""Tests for the Singleton pattern."""

import sys
import os
import threading
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from singleton import Singleton


class TestSingleton(unittest.TestCase):

    def setUp(self):
        # Reset counter before each test for isolation
        Singleton().reset()

    # ------------------------------------------------------------------
    # Identity
    # ------------------------------------------------------------------
    def test_same_instance_returned(self):
        a = Singleton()
        b = Singleton()
        self.assertIs(a, b)

    def test_same_identity_across_multiple_calls(self):
        instances = [Singleton() for _ in range(10)]
        self.assertTrue(all(inst is instances[0] for inst in instances))

    # ------------------------------------------------------------------
    # State
    # ------------------------------------------------------------------
    def test_value_persists_between_calls(self):
        Singleton().increment()
        Singleton().increment()
        self.assertEqual(2, Singleton().value)

    def test_reset_works(self):
        Singleton().increment()
        Singleton().reset()
        self.assertEqual(0, Singleton().value)

    def test_increment_accumulates(self):
        for _ in range(5):
            Singleton().increment()
        self.assertEqual(5, Singleton().value)

    # ------------------------------------------------------------------
    # Thread-safety
    # ------------------------------------------------------------------
    def test_thread_safe_initialization(self):
        instances = [None] * 32
        threads = []

        def capture(idx):
            instances[idx] = Singleton()

        for i in range(32):
            t = threading.Thread(target=capture, args=(i,))
            threads.append(t)

        for t in threads:
            t.start()
        for t in threads:
            t.join()

        first = instances[0]
        self.assertTrue(all(inst is first for inst in instances))

    def test_concurrent_increments(self):
        NUM_THREADS = 20
        threads = []

        def do_increments():
            for _ in range(10):
                Singleton().increment()

        for _ in range(NUM_THREADS):
            t = threading.Thread(target=do_increments)
            threads.append(t)

        for t in threads:
            t.start()
        for t in threads:
            t.join()

        # Each thread did 10 increments: total should be 200
        self.assertEqual(NUM_THREADS * 10, Singleton().value)


if __name__ == "__main__":
    unittest.main()
