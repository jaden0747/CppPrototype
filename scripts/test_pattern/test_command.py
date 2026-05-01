"""Tests for the Command pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from command import TextEditor, AppendCommand, DeleteLastCommand, CommandHistory


class TestCommand(unittest.TestCase):

    # ------------------------------------------------------------------
    # TextEditor receiver
    # ------------------------------------------------------------------
    def test_append_text(self):
        e = TextEditor()
        e.append("Hello")
        self.assertEqual("Hello", e.content)

    def test_delete_last(self):
        e = TextEditor()
        e.append("Hello")
        e.delete_last(2)
        self.assertEqual("Hel", e.content)

    def test_delete_more_than_content_clears_all(self):
        e = TextEditor()
        e.append("Hi")
        e.delete_last(100)
        self.assertEqual("", e.content)

    # ------------------------------------------------------------------
    # AppendCommand
    # ------------------------------------------------------------------
    def test_append_command_execute(self):
        e = TextEditor(); e.append("Hello")
        AppendCommand(e, " World").execute()
        self.assertEqual("Hello World", e.content)

    def test_append_command_undo(self):
        e = TextEditor(); e.append("Hello")
        cmd = AppendCommand(e, " World")
        cmd.execute(); cmd.undo()
        self.assertEqual("Hello", e.content)

    # ------------------------------------------------------------------
    # DeleteLastCommand
    # ------------------------------------------------------------------
    def test_delete_last_command_execute(self):
        e = TextEditor(); e.append("Hello")
        DeleteLastCommand(e, 3).execute()
        self.assertEqual("He", e.content)

    def test_delete_last_command_undo(self):
        e = TextEditor(); e.append("Hello")
        cmd = DeleteLastCommand(e, 3)
        cmd.execute(); cmd.undo()
        self.assertEqual("Hello", e.content)

    # ------------------------------------------------------------------
    # CommandHistory
    # ------------------------------------------------------------------
    def test_history_execute_updates_content(self):
        e = TextEditor()
        CommandHistory().execute(AppendCommand(e, "Hi"))
        self.assertEqual("Hi", e.content)

    def test_history_undo_reverts(self):
        e = TextEditor()
        h = CommandHistory()
        h.execute(AppendCommand(e, "Hi"))
        h.undo()
        self.assertEqual("", e.content)

    def test_history_redo_reapplies(self):
        e = TextEditor()
        h = CommandHistory()
        h.execute(AppendCommand(e, "Hi"))
        h.undo(); h.redo()
        self.assertEqual("Hi", e.content)

    def test_history_undo_empty_returns_false(self):
        self.assertFalse(CommandHistory().undo())

    def test_history_new_action_clears_redo(self):
        e = TextEditor()
        h = CommandHistory()
        h.execute(AppendCommand(e, "A"))
        h.undo()
        self.assertTrue(h.can_redo)
        h.execute(AppendCommand(e, "B"))
        self.assertFalse(h.can_redo)

    def test_multiple_undo_redo(self):
        e = TextEditor()
        h = CommandHistory()
        h.execute(AppendCommand(e, "Hello"))
        h.execute(AppendCommand(e, " World"))
        h.undo()
        self.assertEqual("Hello", e.content)
        h.redo()
        self.assertEqual("Hello World", e.content)


if __name__ == "__main__":
    unittest.main()
