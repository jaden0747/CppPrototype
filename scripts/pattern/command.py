"""
Command Pattern
===============
Intent: Encapsulate a request as an object, allowing undo/redo, queueing,
and logging of operations.

Real-world analogy: A text editor — each user action is a Command object
stored in a history stack that supports undo and redo.
"""

from abc import ABC, abstractmethod
from collections import deque
from typing import Deque


# ---------------------------------------------------------------------------
# Receiver
# ---------------------------------------------------------------------------

class TextEditor:
    def __init__(self):
        self._content = ""

    def append(self, text: str) -> None:
        self._content += text

    def delete_last(self, n: int) -> None:
        if n >= len(self._content):
            self._content = ""
        else:
            self._content = self._content[:-n]

    @property
    def content(self) -> str:
        return self._content


# ---------------------------------------------------------------------------
# Command interface
# ---------------------------------------------------------------------------

class Command(ABC):
    @abstractmethod
    def execute(self) -> None: ...
    @abstractmethod
    def undo(self) -> None: ...
    @abstractmethod
    def name(self) -> str: ...


# ---------------------------------------------------------------------------
# Concrete Commands
# ---------------------------------------------------------------------------

class AppendCommand(Command):
    def __init__(self, editor: TextEditor, text: str):
        self._editor = editor
        self._text   = text

    def execute(self) -> None: self._editor.append(self._text)
    def undo(self)    -> None: self._editor.delete_last(len(self._text))
    def name(self)    -> str:  return f"Append({self._text!r})"


class DeleteLastCommand(Command):
    def __init__(self, editor: TextEditor, n: int):
        self._editor  = editor
        self._n       = n
        self._deleted = ""

    def execute(self) -> None:
        c = self._editor.content
        actual = min(self._n, len(c))
        self._deleted = c[-actual:] if actual > 0 else ""
        self._editor.delete_last(self._n)

    def undo(self) -> None:
        self._editor.append(self._deleted)

    def name(self) -> str:
        return f"DeleteLast({self._n})"


# ---------------------------------------------------------------------------
# Invoker (history + undo/redo)
# ---------------------------------------------------------------------------

class CommandHistory:
    def __init__(self):
        self._history: Deque[Command] = deque()
        self._redo:    Deque[Command] = deque()

    def execute(self, cmd: Command) -> None:
        cmd.execute()
        self._history.append(cmd)
        self._redo.clear()

    def undo(self) -> bool:
        if not self._history:
            return False
        cmd = self._history.pop()
        cmd.undo()
        self._redo.append(cmd)
        return True

    def redo(self) -> bool:
        if not self._redo:
            return False
        cmd = self._redo.pop()
        cmd.execute()
        self._history.append(cmd)
        return True

    @property
    def can_undo(self) -> bool: return bool(self._history)
    @property
    def can_redo(self) -> bool: return bool(self._redo)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    editor  = TextEditor()
    history = CommandHistory()

    history.execute(AppendCommand(editor, "Hello"))
    history.execute(AppendCommand(editor, " World"))
    print(f"After appends: '{editor.content}'")

    history.undo()
    print(f"After undo:    '{editor.content}'")

    history.redo()
    print(f"After redo:    '{editor.content}'")

    history.execute(DeleteLastCommand(editor, 5))
    print(f"After delete:  '{editor.content}'")

    history.undo()
    print(f"After undo:    '{editor.content}'")
