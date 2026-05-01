"""Tests for the Observer pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from observer import StockMarket, Logger, AlertMonitor, EventEmitter, Event


class TestStockMarket(unittest.TestCase):

    def setUp(self):
        self.market = StockMarket()
        self.logger = Logger()

    def test_attach_increases_count(self):
        self.market.attach(self.logger)
        self.assertEqual(1, self.market.subscriber_count)

    def test_detach_decreases_count(self):
        self.market.attach(self.logger)
        self.market.detach(self.logger)
        self.assertEqual(0, self.market.subscriber_count)

    def test_logger_receives_event(self):
        self.market.attach(self.logger)
        self.market.set_price("AAPL", 150.0)
        self.assertEqual(1, len(self.logger.log))
        self.assertIn("AAPL", self.logger.log[0])

    def test_multiple_observers_all_notified(self):
        l1, l2 = Logger(), Logger()
        self.market.attach(l1)
        self.market.attach(l2)
        self.market.set_price("GOOG", 2800.0)
        self.assertEqual(1, len(l1.log))
        self.assertEqual(1, len(l2.log))

    def test_detached_not_notified(self):
        l1, l2 = Logger(), Logger()
        self.market.attach(l1)
        self.market.attach(l2)
        self.market.detach(l2)
        self.market.set_price("MSFT", 300.0)
        self.assertEqual(1, len(l1.log))
        self.assertEqual(0, len(l2.log))

    def test_alert_fires_above_threshold(self):
        monitor = AlertMonitor(200.0)
        self.market.attach(monitor)
        self.market.set_price("TSLA", 250.0)
        self.assertEqual(1, len(monitor.alerts))

    def test_alert_silent_below_threshold(self):
        monitor = AlertMonitor(200.0)
        self.market.attach(monitor)
        self.market.set_price("TSLA", 100.0)
        self.assertEqual(0, len(monitor.alerts))


class TestEventEmitter(unittest.TestCase):

    def test_subscribe_and_emit(self):
        emitter: EventEmitter[int] = EventEmitter()
        received = []
        emitter.subscribe(lambda v: received.append(v))
        emitter.emit(42)
        self.assertEqual([42], received)

    def test_multiple_subscribers(self):
        emitter: EventEmitter[str] = EventEmitter()
        log = []
        emitter.subscribe(lambda s: log.append("A:" + s))
        emitter.subscribe(lambda s: log.append("B:" + s))
        emitter.emit("hello")
        self.assertEqual(2, len(log))

    def test_unsubscribe(self):
        emitter: EventEmitter[int] = EventEmitter()
        count_a = [0]
        count_b = [0]
        emitter.subscribe(lambda v: count_a.__setitem__(0, count_a[0] + 1))
        id_b = emitter.subscribe(lambda v: count_b.__setitem__(0, count_b[0] + 1))
        emitter.unsubscribe(id_b)
        emitter.emit(1)
        self.assertEqual(1, count_a[0])
        self.assertEqual(0, count_b[0])

    def test_subscriber_count(self):
        emitter: EventEmitter[int] = EventEmitter()
        sid = emitter.subscribe(lambda v: None)
        self.assertEqual(1, emitter.subscriber_count)
        emitter.unsubscribe(sid)
        self.assertEqual(0, emitter.subscriber_count)


if __name__ == "__main__":
    unittest.main()
