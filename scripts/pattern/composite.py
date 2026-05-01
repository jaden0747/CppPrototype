"""
Composite Pattern
=================
Intent: Compose objects into tree structures to represent part-whole
hierarchies. Clients treat individual objects (Leaf) and compositions
(Composite) uniformly.

Real-world analogy: A file system where Files and Directories both support
a `size()` operation. Calling `size()` on a directory sums all descendants.
"""

from abc import ABC, abstractmethod
from typing import List


# ---------------------------------------------------------------------------
# Component
# ---------------------------------------------------------------------------

class FileSystemNode(ABC):
    def __init__(self, name: str):
        self.name = name

    @abstractmethod
    def size(self) -> int: ...

    @abstractmethod
    def is_directory(self) -> bool: ...

    def print_tree(self, indent: str = "") -> None:
        print(f"{indent}{self}")


# ---------------------------------------------------------------------------
# Leaf
# ---------------------------------------------------------------------------

class File(FileSystemNode):
    def __init__(self, name: str, bytes_: int):
        super().__init__(name)
        self._bytes = bytes_

    def size(self) -> int:        return self._bytes
    def is_directory(self) -> bool: return False

    def __repr__(self) -> str:
        return f"File({self.name!r}, {self._bytes} bytes)"


# ---------------------------------------------------------------------------
# Composite
# ---------------------------------------------------------------------------

class Directory(FileSystemNode):
    def __init__(self, name: str):
        super().__init__(name)
        self._children: List[FileSystemNode] = []

    def add(self, node: FileSystemNode) -> None:
        self._children.append(node)

    def size(self) -> int:
        return sum(child.size() for child in self._children)

    def is_directory(self) -> bool:
        return True

    @property
    def children(self) -> List[FileSystemNode]:
        return self._children

    def print_tree(self, indent: str = "") -> None:
        print(f"{indent}{self.name}/")
        for child in self._children:
            child.print_tree(indent + "  ")

    def __repr__(self) -> str:
        return f"Directory({self.name!r}, {len(self._children)} children)"


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    root = Directory("root")

    src = Directory("src")
    src.add(File("main.cpp", 200))
    src.add(File("utils.cpp", 150))

    docs = Directory("docs")
    docs.add(File("readme.md", 50))
    docs.add(File("api.md", 80))

    root.add(src)
    root.add(docs)
    root.add(File("CMakeLists.txt", 30))

    root.print_tree()
    print(f"Total size: {root.size()} bytes")
