"""
Memento Pattern
===============
Intent: Without violating encapsulation, capture and externalise an object's
internal state so the object can be restored to that state later.

Real-world analogy: A text editor's "undo" history. Each save() call creates
a Memento snapshot; restore() reverts to a previous one.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import List


# ---------------------------------------------------------------------------
# Memento (value-object snapshot)
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class Memento:
    """Opaque snapshot — holds state but exposes no behaviour."""
    _content: str
    label: str = ""

    # The leading underscore discourages direct access (Python convention).
    @property
    def content(self) -> str:          # read-only access for caretaker
        return self._content


# ---------------------------------------------------------------------------
# Originator
# ---------------------------------------------------------------------------

class Editor:
    def __init__(self):
        self._content: str = ""

    def type(self, text: str) -> None:
        self._content += text

    def delete_last(self, n: int) -> None:
        self._content = self._content[:-n] if n < len(self._content) else ""

    @property
    def content(self) -> str:
        return self._content

    def save(self, label: str = "") -> Memento:
        return Memento(_content=self._content, label=label)

    def restore(self, memento: Memento) -> None:
        self._content = memento.content


# ---------------------------------------------------------------------------
# Caretaker
# ---------------------------------------------------------------------------

class History:
    def __init__(self):
        self._snapshots: List[Memento] = []

    def push(self, memento: Memento) -> None:
        self._snapshots.append(memento)

    def pop(self) -> Memento:
        if not self._snapshots:
            raise IndexError("History is empty")
        return self._snapshots.pop()

    def top(self) -> Memento:
        if not self._snapshots:
            raise IndexError("History is empty")
        return self._snapshots[-1]

    def is_empty(self) -> bool:
        return len(self._snapshots) == 0

    def __len__(self) -> int:
        return len(self._snapshots)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    editor  = Editor()
    history = History()

    history.push(editor.save("empty"))
    editor.type("Hello")
    history.push(editor.save("Hello"))
    editor.type(" World")
    history.push(editor.save("Hello World"))

    print("Current:", editor.content)   # Hello World

    # Undo once
    history.pop()
    editor.restore(history.top())
    print("After undo:", editor.content)  # Hello

    # Undo again
    history.pop()
    editor.restore(history.top())
    print("After 2nd undo:", editor.content)  # (empty)
