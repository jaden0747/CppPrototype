"""Tests for the Proxy pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from proxy import (
    LazyImageProxy,
    Role, RealService, ProtectionProxy,
    SlowDataSource, CachingProxy
)


class TestProxy(unittest.TestCase):

    # ------------------------------------------------------------------
    # 1. Virtual (Lazy) Proxy
    # ------------------------------------------------------------------
    def test_lazy_proxy_not_loaded_before_display(self):
        self.assertFalse(LazyImageProxy("photo.png").is_loaded())

    def test_lazy_proxy_loaded_after_display(self):
        p = LazyImageProxy("photo.png")
        p.display()
        self.assertTrue(p.is_loaded())

    def test_lazy_proxy_name(self):
        self.assertEqual("vacation.jpg", LazyImageProxy("vacation.jpg").name())

    def test_lazy_proxy_display_twice_stays_loaded(self):
        p = LazyImageProxy("img.bmp")
        p.display()
        p.display()
        self.assertTrue(p.is_loaded())

    # ------------------------------------------------------------------
    # 2. Protection Proxy
    # ------------------------------------------------------------------
    def test_user_can_get_data(self):
        p = ProtectionProxy(RealService(), Role.User)
        self.assertEqual("Sensitive data", p.get_data())

    def test_guest_cannot_get_data(self):
        p = ProtectionProxy(RealService(), Role.Guest)
        with self.assertRaises(PermissionError):
            p.get_data()

    def test_admin_can_delete_data(self):
        p = ProtectionProxy(RealService(), Role.Admin)
        self.assertTrue(p.delete_data())

    def test_user_cannot_delete_data(self):
        p = ProtectionProxy(RealService(), Role.User)
        with self.assertRaises(PermissionError):
            p.delete_data()

    def test_guest_cannot_delete_data(self):
        p = ProtectionProxy(RealService(), Role.Guest)
        with self.assertRaises(PermissionError):
            p.delete_data()

    # ------------------------------------------------------------------
    # 3. Caching Proxy
    # ------------------------------------------------------------------
    def test_caching_proxy_returns_correct_value(self):
        proxy = CachingProxy(SlowDataSource())
        self.assertEqual("value_of_a", proxy.fetch("a"))

    def test_caching_proxy_calls_real_only_once(self):
        slow  = SlowDataSource()
        proxy = CachingProxy(slow)
        proxy.fetch("key1")
        proxy.fetch("key1")
        proxy.fetch("key1")
        self.assertEqual(1, slow.fetch_count)

    def test_caching_proxy_cache_size_grows_for_new_keys(self):
        proxy = CachingProxy(SlowDataSource())
        proxy.fetch("k1")
        proxy.fetch("k2")
        proxy.fetch("k1")
        self.assertEqual(2, proxy.cache_size)

    def test_caching_proxy_different_keys_fetched_once_each(self):
        slow  = SlowDataSource()
        proxy = CachingProxy(slow)
        proxy.fetch("a"); proxy.fetch("b"); proxy.fetch("c")
        self.assertEqual(3, slow.fetch_count)


if __name__ == "__main__":
    unittest.main()
