"""Tests for the Adapter pattern."""

import sys
import os
import math
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from adapter import RoundHole, RoundPeg, SquarePeg, SquarePegAdapter


class TestAdapter(unittest.TestCase):

    EPS = 1e-6

    # ------------------------------------------------------------------
    # RoundHole / RoundPeg
    # ------------------------------------------------------------------
    def test_round_peg_fits_larger_hole(self):
        self.assertTrue(RoundHole(5.0).fits(RoundPeg(4.0).radius()))

    def test_round_peg_does_not_fit_smaller_hole(self):
        self.assertFalse(RoundHole(3.0).fits(RoundPeg(4.0).radius()))

    def test_round_peg_fits_exactly(self):
        self.assertTrue(RoundHole(5.0).fits(RoundPeg(5.0).radius()))

    # ------------------------------------------------------------------
    # Adapter radius calculation
    # ------------------------------------------------------------------
    def test_adapter_radius_is_half_diagonal(self):
        adapter = SquarePegAdapter(SquarePeg(2.0))
        expected = 2.0 * math.sqrt(2) / 2
        self.assertAlmostEqual(expected, adapter.radius(), places=6)

    def test_small_square_peg_fits_in_big_hole(self):
        hole    = RoundHole(5.0)
        adapter = SquarePegAdapter(SquarePeg(4.0))
        self.assertTrue(hole.fits(adapter.radius()))

    def test_large_square_peg_does_not_fit(self):
        hole    = RoundHole(5.0)
        adapter = SquarePegAdapter(SquarePeg(8.0))
        self.assertFalse(hole.fits(adapter.radius()))

    def test_adapter_exact_boundary(self):
        # Square peg whose enclosing circle exactly equals the hole
        r = 5.0
        # width = r * 2 / sqrt(2) = r * sqrt(2)
        width   = r * math.sqrt(2)
        adapter = SquarePegAdapter(SquarePeg(width))
        self.assertAlmostEqual(r, adapter.radius(), places=6)

    # ------------------------------------------------------------------
    # SquarePeg
    # ------------------------------------------------------------------
    def test_square_peg_describe_contains_width(self):
        self.assertIn("6.5", SquarePeg(6.5).describe())


if __name__ == "__main__":
    unittest.main()
