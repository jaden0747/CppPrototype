"""Tests for the Memento pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from memento import Editor, History, Memento


class TestEditor(unittest.TestCase):

    def test_type_appends(self):
        e = Editor()
        e.type("Hello")
        self.assertEqual("Hello", e.content)

    def test_save_captures_content(self):
        e = Editor()
        e.type("Hello")
        m = e.save()
        self.assertEqual("Hello", m.content)

    def test_restore_reverts(self):
        e = Editor()
        e.type("Hello")
        m = e.save()
        e.type(" World")
        e.restore(m)
        self.assertEqual("Hello", e.content)

    def test_memento_label(self):
        e = Editor()
        m = e.save("initial")
        self.assertEqual("initial", m.label)

    def test_delete_last(self):
        e = Editor()
        e.type("HelloWorld")
        e.delete_last(5)
        self.assertEqual("Hello", e.content)

    def test_delete_last_more_than_length_clears(self):
        e = Editor()
        e.type("Hi")
        e.delete_last(100)
        self.assertEqual("", e.content)


class TestHistory(unittest.TestCase):

    def test_empty_initially(self):
        h = History()
        self.assertTrue(h.is_empty())

    def test_push_increases_len(self):
        e = Editor()
        h = History()
        h.push(e.save())
        self.assertEqual(1, len(h))

    def test_pop_returns_last(self):
        e = Editor()
        h = History()
        e.type("A")
        h.push(e.save())
        e.type("B")
        h.push(e.save())
        m = h.pop()
        self.assertEqual("AB", m.content)

    def test_pop_empty_raises(self):
        h = History()
        with self.assertRaises(IndexError):
            h.pop()

    def test_top_empty_raises(self):
        h = History()
        with self.assertRaises(IndexError):
            h.top()


class TestUndoWorkflow(unittest.TestCase):

    def test_single_undo(self):
        editor  = Editor()
        history = History()

        editor.type("Hello")
        history.push(editor.save())
        editor.type(" World")
        history.push(editor.save())

        history.pop()
        editor.restore(history.top())
        self.assertEqual("Hello", editor.content)

    def test_multiple_undos(self):
        editor  = Editor()
        history = History()

        history.push(editor.save("empty"))
        editor.type("A")
        history.push(editor.save("A"))
        editor.type("B")
        history.push(editor.save("AB"))

        history.pop()
        editor.restore(history.top())
        self.assertEqual("A", editor.content)

        history.pop()
        editor.restore(history.top())
        self.assertEqual("", editor.content)


if __name__ == "__main__":
    unittest.main()
