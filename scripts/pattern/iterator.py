"""
Iterator Pattern
================
Intent: Provide a way to sequentially access elements of an aggregate object
without exposing its underlying representation.

Two versions:
  1. Custom iterator class (forward + reverse over a word list)
  2. Python generator-based iterator (Pythonic approach)
"""

from __future__ import annotations
from typing import Iterator as IterType, List


# ---------------------------------------------------------------------------
# 1. Custom iterator (manual)
# ---------------------------------------------------------------------------

class WordCollection:
    def __init__(self):
        self._words: List[str] = []

    def add(self, word: str) -> None:
        self._words.append(word)

    def __len__(self) -> int:
        return len(self._words)

    def forward_iterator(self) -> "ForwardIterator":
        return ForwardIterator(self._words)

    def reverse_iterator(self) -> "ReverseIterator":
        return ReverseIterator(self._words)

    # Python protocol: also make the collection itself iterable
    def __iter__(self):
        return iter(self._words)


class ForwardIterator:
    def __init__(self, words: List[str]):
        self._words = words
        self._index = 0

    def has_next(self) -> bool:
        return self._index < len(self._words)

    def next(self) -> str:
        if not self.has_next():
            raise StopIteration("Iterator exhausted")
        word = self._words[self._index]
        self._index += 1
        return word

    def __iter__(self):
        return self

    def __next__(self) -> str:
        return self.next()


class ReverseIterator:
    def __init__(self, words: List[str]):
        self._words = words
        self._index = len(words)

    def has_next(self) -> bool:
        return self._index > 0

    def next(self) -> str:
        if not self.has_next():
            raise StopIteration("Iterator exhausted")
        self._index -= 1
        return self._words[self._index]

    def __iter__(self):
        return self

    def __next__(self) -> str:
        return self.next()


# ---------------------------------------------------------------------------
# 2. Generator-based range iterator (Pythonic)
# ---------------------------------------------------------------------------

class NumberRange:
    def __init__(self, from_: int, to: int):
        self._from = from_
        self._to   = to

    def __iter__(self) -> IterType[int]:
        current = self._from
        while current <= self._to:
            yield current
            current += 1


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    col = WordCollection()
    for word in ("alpha", "beta", "gamma"):
        col.add(word)

    print("Forward:")
    for w in col.forward_iterator():
        print(" ", w)

    print("Reverse:")
    for w in col.reverse_iterator():
        print(" ", w)

    print("NumberRange(1,5):", list(NumberRange(1, 5)))
    print("Sum(1..10):", sum(NumberRange(1, 10)))
