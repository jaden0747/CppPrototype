"""Tests for the Bridge pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from bridge import TV, Radio, RemoteControl, AdvancedRemote


class TestBridge(unittest.TestCase):

    # ------------------------------------------------------------------
    # Device (TV)
    # ------------------------------------------------------------------
    def test_tv_starts_disabled(self):
        self.assertFalse(TV().is_enabled())

    def test_tv_enable_disable(self):
        tv = TV()
        tv.enable()
        self.assertTrue(tv.is_enabled())
        tv.disable()
        self.assertFalse(tv.is_enabled())

    def test_tv_volume_clamped_high(self):
        tv = TV()
        tv.set_volume(150)
        self.assertEqual(100, tv.volume())

    def test_tv_volume_clamped_low(self):
        tv = TV()
        tv.set_volume(-10)
        self.assertEqual(0, tv.volume())

    # ------------------------------------------------------------------
    # RemoteControl + TV
    # ------------------------------------------------------------------
    def test_toggle_power_enables_tv(self):
        remote = RemoteControl(TV())
        remote.toggle_power()
        self.assertTrue(remote.device.is_enabled())

    def test_toggle_power_twice_restores_state(self):
        remote = RemoteControl(TV())
        remote.toggle_power()
        remote.toggle_power()
        self.assertFalse(remote.device.is_enabled())

    def test_volume_up_increases_by_10(self):
        tv = TV()
        initial = tv.volume()
        RemoteControl(tv).volume_up()
        self.assertEqual(initial + 10, tv.volume())

    def test_volume_down_decreases_by_10(self):
        tv = TV()
        initial = tv.volume()
        RemoteControl(tv).volume_down()
        self.assertEqual(initial - 10, tv.volume())

    def test_channel_up_increases_by_1(self):
        tv = TV()
        initial = tv.channel()
        RemoteControl(tv).channel_up()
        self.assertEqual(initial + 1, tv.channel())

    # ------------------------------------------------------------------
    # RemoteControl + Radio
    # ------------------------------------------------------------------
    def test_remote_works_with_radio(self):
        remote = RemoteControl(Radio())
        remote.toggle_power()
        self.assertTrue(remote.device.is_enabled())
        self.assertEqual("Radio", remote.device.name())

    # ------------------------------------------------------------------
    # AdvancedRemote
    # ------------------------------------------------------------------
    def test_advanced_remote_mute(self):
        tv = TV()
        AdvancedRemote(tv).mute()
        self.assertEqual(0, tv.volume())

    def test_advanced_remote_set_channel(self):
        tv = TV()
        AdvancedRemote(tv).set_channel(42)
        self.assertEqual(42, tv.channel())


if __name__ == "__main__":
    unittest.main()
