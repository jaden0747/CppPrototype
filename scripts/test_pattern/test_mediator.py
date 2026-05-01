"""Tests for the Mediator pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from mediator import ChatUser, ChatRoom


def make_room(*names):
    room = ChatRoom()
    users = [ChatUser(n) for n in names]
    for u in users:
        room.add_user(u)
    return room, users


class TestMediator(unittest.TestCase):

    # ------------------------------------------------------------------
    # Basic delivery
    # ------------------------------------------------------------------
    def test_sender_does_not_receive_own_message(self):
        room, (alice, bob, carol) = make_room("Alice", "Bob", "Carol")
        alice.send("Hello")
        self.assertEqual([], alice.inbox)

    def test_other_users_receive_message(self):
        room, (alice, bob, carol) = make_room("Alice", "Bob", "Carol")
        alice.send("Hello")
        self.assertEqual(1, len(bob.inbox))
        self.assertEqual(1, len(carol.inbox))

    def test_inbox_contains_sender_name(self):
        room, (alice, bob) = make_room("Alice", "Bob")
        alice.send("Hi")
        self.assertIn("Alice", bob.inbox[0])

    def test_inbox_contains_message_text(self):
        room, (alice, bob) = make_room("Alice", "Bob")
        alice.send("secret_msg")
        self.assertIn("secret_msg", bob.inbox[0])

    # ------------------------------------------------------------------
    # Multiple senders
    # ------------------------------------------------------------------
    def test_bob_sends_to_others(self):
        room, (alice, bob, carol) = make_room("Alice", "Bob", "Carol")
        bob.send("Hey")
        self.assertEqual(1, len(alice.inbox))
        self.assertEqual([], bob.inbox)
        self.assertEqual(1, len(carol.inbox))

    def test_multiple_messages(self):
        room, (alice, bob) = make_room("Alice", "Bob")
        alice.send("m1")
        alice.send("m2")
        self.assertEqual(2, len(bob.inbox))

    # ------------------------------------------------------------------
    # Room user count
    # ------------------------------------------------------------------
    def test_chat_room_user_count(self):
        room = ChatRoom()
        room.add_user(ChatUser("U1"))
        room.add_user(ChatUser("U2"))
        self.assertEqual(2, room.user_count)

    # ------------------------------------------------------------------
    # No mediator: no crash
    # ------------------------------------------------------------------
    def test_user_without_mediator_does_not_crash(self):
        loner = ChatUser("Loner")
        loner.send("hello")  # should not raise
        self.assertEqual([], loner.inbox)


if __name__ == "__main__":
    unittest.main()
