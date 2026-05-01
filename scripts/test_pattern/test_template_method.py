"""Tests for the Template Method pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from template_method import CsvMiner, JsonMiner


class TestCsvMiner(unittest.TestCase):

    def test_parses_rows(self):
        miner = CsvMiner()
        miner.mine("a;b;c")
        self.assertEqual(3, len(miner.parsed_lines))

    def test_parsed_lines_have_prefix(self):
        miner = CsvMiner()
        miner.mine("hello;world")
        self.assertEqual("csv:hello", miner.parsed_lines[0])
        self.assertEqual("csv:world", miner.parsed_lines[1])

    def test_report_contains_row_count(self):
        miner  = CsvMiner()
        report = miner.mine("a;b;c")
        self.assertIn("3", report)

    def test_report_contains_analysis_note(self):
        miner  = CsvMiner()
        report = miner.mine("x;y")
        self.assertIn("CSV analysis", report)

    def test_single_line(self):
        miner = CsvMiner()
        miner.mine("only")
        self.assertEqual(1, len(miner.parsed_lines))
        self.assertEqual("csv:only", miner.parsed_lines[0])


class TestJsonMiner(unittest.TestCase):

    def test_parses_one_record(self):
        miner = JsonMiner()
        miner.mine('{"key":"value"}')
        self.assertEqual(1, len(miner.parsed_lines))

    def test_parsed_line_has_prefix(self):
        miner = JsonMiner()
        miner.mine("doc")
        self.assertEqual("json:doc", miner.parsed_lines[0])

    def test_report_uses_default_analysis(self):
        miner  = JsonMiner()
        report = miner.mine("{}")
        self.assertIn("default analysis", report)


class TestTemplateMethod(unittest.TestCase):

    def test_mine_always_returns_nonempty_report(self):
        self.assertTrue(CsvMiner().mine("a"))
        self.assertTrue(JsonMiner().mine("b"))


if __name__ == "__main__":
    unittest.main()
