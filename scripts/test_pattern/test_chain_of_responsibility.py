"""Tests for the Chain of Responsibility pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from chain_of_responsibility import (
    SupportRequest, Tier1Handler, Tier2Handler, Tier3Handler,
    build_default_chain
)


class TestChainOfResponsibility(unittest.TestCase):

    # ------------------------------------------------------------------
    # Basic routing
    # ------------------------------------------------------------------
    def test_level1_handled_by_tier1(self):
        result = build_default_chain().handle(SupportRequest(1, "password reset"))
        self.assertIn("Tier1", result)

    def test_level2_handled_by_tier2(self):
        result = build_default_chain().handle(SupportRequest(2, "network config"))
        self.assertIn("Tier2", result)

    def test_level3_handled_by_tier3(self):
        result = build_default_chain().handle(SupportRequest(3, "kernel panic"))
        self.assertIn("Tier3", result)

    # ------------------------------------------------------------------
    # Description preserved
    # ------------------------------------------------------------------
    def test_result_contains_description(self):
        result = build_default_chain().handle(SupportRequest(2, "unique_42"))
        self.assertIn("unique_42", result)

    # ------------------------------------------------------------------
    # Unhandled
    # ------------------------------------------------------------------
    def test_unhandled_request(self):
        result = build_default_chain().handle(SupportRequest(4, "alien invasion"))
        self.assertIn("Unhandled", result)

    # ------------------------------------------------------------------
    # Partial chain
    # ------------------------------------------------------------------
    def test_partial_chain_tier1_unhandled(self):
        t2 = Tier2Handler()
        result = t2.handle(SupportRequest(1, "basic question"))
        self.assertIn("Unhandled", result)

    def test_partial_chain_tier2_handled(self):
        t2 = Tier2Handler()
        result = t2.handle(SupportRequest(2, "medium question"))
        self.assertIn("Tier2", result)

    # ------------------------------------------------------------------
    # set_next returns handler (fluent chaining)
    # ------------------------------------------------------------------
    def test_set_next_returns_next(self):
        t1 = Tier1Handler()
        t2 = Tier2Handler()
        ret = t1.set_next(t2)
        self.assertIs(t2, ret)


if __name__ == "__main__":
    unittest.main()
