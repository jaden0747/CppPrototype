"""Tests for the Builder pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from builder import (
    Burger, MeatBurgerBuilder, VeggieBurgerBuilder,
    BurgerDirector, FluentBurger
)


class TestBuilder(unittest.TestCase):

    # ------------------------------------------------------------------
    # Default values
    # ------------------------------------------------------------------
    def test_meat_builder_default_bun(self):
        self.assertEqual("sesame", MeatBurgerBuilder().get_result().bun)

    def test_veggie_builder_default_patty(self):
        self.assertEqual("black-bean", VeggieBurgerBuilder().get_result().patty)

    # ------------------------------------------------------------------
    # Step-by-step customization
    # ------------------------------------------------------------------
    def test_set_bun_overrides_default(self):
        b = MeatBurgerBuilder()
        b.set_bun("brioche")
        self.assertEqual("brioche", b.get_result().bun)

    def test_add_multiple_toppings(self):
        b = MeatBurgerBuilder()
        b.add_topping("lettuce")
        b.add_topping("tomato")
        result = b.get_result()
        self.assertEqual(["lettuce", "tomato"], result.toppings)

    def test_toasted_flag(self):
        b = MeatBurgerBuilder()
        b.set_toasted(True)
        self.assertTrue(b.get_result().toasted)

    # ------------------------------------------------------------------
    # Director recipes
    # ------------------------------------------------------------------
    def test_director_classic_has_cheese(self):
        builder = MeatBurgerBuilder()
        BurgerDirector(builder).build_classic()
        self.assertIn("cheese", builder.get_result().toppings)

    def test_director_deluxe_has_more_toppings_than_classic(self):
        b1 = MeatBurgerBuilder()
        BurgerDirector(b1).build_classic()
        c = b1.get_result()

        b2 = MeatBurgerBuilder()
        BurgerDirector(b2).build_deluxe()
        d = b2.get_result()

        self.assertGreater(len(d.toppings), len(c.toppings))

    # ------------------------------------------------------------------
    # Reset after get_result
    # ------------------------------------------------------------------
    def test_builder_resets_after_get_result(self):
        b = MeatBurgerBuilder()
        b.add_topping("onion")
        b.get_result()  # consume + reset
        second = b.get_result()
        self.assertEqual([], second.toppings)

    # ------------------------------------------------------------------
    # Fluent builder
    # ------------------------------------------------------------------
    def test_fluent_sets_all_fields(self):
        b = (FluentBurger()
             .with_bun("brioche")
             .with_patty("turkey")
             .with_topping("avocado")
             .toasted()
             .build())
        self.assertEqual("brioche",   b.bun)
        self.assertEqual("turkey",    b.patty)
        self.assertEqual(["avocado"], b.toppings)
        self.assertTrue(b.toasted)

    def test_fluent_describe_not_empty(self):
        b = FluentBurger().with_bun("bun").with_patty("patty").build()
        self.assertTrue(len(b.describe()) > 0)


if __name__ == "__main__":
    unittest.main()
