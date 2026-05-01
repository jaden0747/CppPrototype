"""
Mediator Pattern
================
Intent: Define an object (mediator) that encapsulates how a set of objects
interact, reducing direct coupling between them.

Real-world analogy: A chat-room server. Users don't message each other
directly — they all send to the room, which distributes messages.
"""

from abc import ABC, abstractmethod
from typing import List, Optional


# ---------------------------------------------------------------------------
# Abstract Mediator
# ---------------------------------------------------------------------------

class Mediator(ABC):
    @abstractmethod
    def notify(self, sender: "Component", event: str) -> None: ...


# ---------------------------------------------------------------------------
# Component base
# ---------------------------------------------------------------------------

class Component:
    def __init__(self, name: str):
        self.name = name
        self._mediator: Optional[Mediator] = None

    def set_mediator(self, mediator: Mediator) -> None:
        self._mediator = mediator

    def _trigger(self, event: str) -> None:
        if self._mediator:
            self._mediator.notify(self, event)


# ---------------------------------------------------------------------------
# Concrete Component (chat user)
# ---------------------------------------------------------------------------

class ChatUser(Component):
    def __init__(self, name: str):
        super().__init__(name)
        self._last_sent: str = ""
        self.inbox: List[str] = []

    def send(self, message: str) -> None:
        self._last_sent = message
        self._trigger(f"message:{message}")

    def receive(self, from_name: str, message: str) -> None:
        self.inbox.append(f"[{from_name}]: {message}")

    @property
    def last_sent(self) -> str:
        return self._last_sent


# ---------------------------------------------------------------------------
# Concrete Mediator (chat room)
# ---------------------------------------------------------------------------

class ChatRoom(Mediator):
    def __init__(self):
        self._users: List[ChatUser] = []

    def add_user(self, user: ChatUser) -> None:
        self._users.append(user)
        user.set_mediator(self)

    def notify(self, sender: Component, event: str) -> None:
        prefix = "message:"
        if not event.startswith(prefix):
            return
        text = event[len(prefix):]
        for user in self._users:
            if user is not sender:
                user.receive(sender.name, text)

    @property
    def user_count(self) -> int:
        return len(self._users)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    room  = ChatRoom()
    alice = ChatUser("Alice")
    bob   = ChatUser("Bob")
    carol = ChatUser("Carol")

    for user in (alice, bob, carol):
        room.add_user(user)

    alice.send("Hello everyone!")
    bob.send("Hi Alice!")

    print("Bob's inbox:",   bob.inbox)
    print("Carol's inbox:", carol.inbox)
    print("Alice's inbox:", alice.inbox)
