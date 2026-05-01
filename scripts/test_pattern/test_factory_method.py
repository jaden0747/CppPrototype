"""Tests for the Factory Method pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from factory_method import (
    RoadLogistics, SeaLogistics, AirLogistics,
    Truck, Ship, Plane, make_logistics
)


class TestFactoryMethod(unittest.TestCase):

    # ------------------------------------------------------------------
    # Product creation
    # ------------------------------------------------------------------
    def test_road_logistics_creates_truck(self):
        t = RoadLogistics().create_transport()
        self.assertIsInstance(t, Truck)
        self.assertEqual("Truck", t.type())

    def test_sea_logistics_creates_ship(self):
        t = SeaLogistics().create_transport()
        self.assertIsInstance(t, Ship)
        self.assertEqual("Ship", t.type())

    def test_air_logistics_creates_plane(self):
        t = AirLogistics().create_transport()
        self.assertIsInstance(t, Plane)
        self.assertEqual("Plane", t.type())

    # ------------------------------------------------------------------
    # Deliver messages
    # ------------------------------------------------------------------
    def test_truck_deliver_message(self):
        self.assertIn("land", Truck().deliver())

    def test_ship_deliver_message(self):
        self.assertIn("sea", Ship().deliver())

    def test_plane_deliver_message(self):
        self.assertIn("air", Plane().deliver())

    # ------------------------------------------------------------------
    # plan_delivery (template method using factory)
    # ------------------------------------------------------------------
    def test_plan_delivery_road(self):
        self.assertIn("Truck", RoadLogistics().plan_delivery())

    def test_plan_delivery_sea(self):
        self.assertIn("Ship", SeaLogistics().plan_delivery())

    def test_plan_delivery_air(self):
        self.assertIn("Plane", AirLogistics().plan_delivery())

    # ------------------------------------------------------------------
    # Helper factory function
    # ------------------------------------------------------------------
    def test_make_logistics_road(self):
        l = make_logistics("road")
        self.assertIsInstance(l, RoadLogistics)

    def test_make_logistics_sea(self):
        l = make_logistics("sea")
        self.assertIsInstance(l, SeaLogistics)

    def test_make_logistics_air(self):
        l = make_logistics("air")
        self.assertIsInstance(l, AirLogistics)

    def test_make_logistics_unknown_raises(self):
        with self.assertRaises(ValueError):
            make_logistics("teleport")

    # ------------------------------------------------------------------
    # Each creation returns a new instance
    # ------------------------------------------------------------------
    def test_each_creation_is_unique(self):
        logistics = RoadLogistics()
        t1 = logistics.create_transport()
        t2 = logistics.create_transport()
        self.assertIsNot(t1, t2)


if __name__ == "__main__":
    unittest.main()
