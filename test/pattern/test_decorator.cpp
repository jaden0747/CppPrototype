#include "pattern/decorator.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <memory>

using namespace pattern;

static const double EPS = 1e-9;

// ---------------------------------------------------------------------------
// Base component
// ---------------------------------------------------------------------------
TEST(Decorator, EspressoBaseCost)
{
    Espresso e;
    EXPECT_NEAR(1.00, e.cost(), EPS);
}

TEST(Decorator, EspressoDescription)
{
    Espresso e;
    EXPECT_EQ("Espresso", e.description());
}

// ---------------------------------------------------------------------------
// Single decorator
// ---------------------------------------------------------------------------
TEST(Decorator, MilkAddsCost)
{
    std::unique_ptr<Coffee> c(new MilkDecorator(std::unique_ptr<Coffee>(new Espresso())));
    EXPECT_NEAR(1.25, c->cost(), EPS);
}

TEST(Decorator, MilkAddsDescription)
{
    std::unique_ptr<Coffee> c(new MilkDecorator(std::unique_ptr<Coffee>(new Espresso())));
    EXPECT_NE(std::string::npos, c->description().find("Milk"));
    EXPECT_NE(std::string::npos, c->description().find("Espresso"));
}

TEST(Decorator, CaramelAddsCost)
{
    std::unique_ptr<Coffee> c(new CaramelDecorator(std::unique_ptr<Coffee>(new Espresso())));
    EXPECT_NEAR(1.50, c->cost(), EPS);
}

// ---------------------------------------------------------------------------
// Stacked decorators
// ---------------------------------------------------------------------------
TEST(Decorator, MilkAndCaramelCost)
{
    std::unique_ptr<Coffee> c = std::unique_ptr<Coffee>(
        new CaramelDecorator(std::unique_ptr<Coffee>(new MilkDecorator(std::unique_ptr<Coffee>(new Espresso())))));
    EXPECT_NEAR(1.75, c->cost(), EPS);
}

TEST(Decorator, TripleStackDescription)
{
    std::unique_ptr<Coffee> c    = std::unique_ptr<Coffee>(new WhipDecorator(
        std::unique_ptr<Coffee>(new CaramelDecorator(
            std::unique_ptr<Coffee>(new MilkDecorator(std::unique_ptr<Coffee>(new Espresso())))))));
    const std::string       desc = c->description();
    EXPECT_NE(std::string::npos, desc.find("Espresso"));
    EXPECT_NE(std::string::npos, desc.find("Milk"));
    EXPECT_NE(std::string::npos, desc.find("Caramel"));
    EXPECT_NE(std::string::npos, desc.find("Whip"));
    EXPECT_NEAR(2.05, c->cost(), EPS);
}

// ---------------------------------------------------------------------------
// Same decorator applied twice
// ---------------------------------------------------------------------------
TEST(Decorator, DoubleMilk)
{
    std::unique_ptr<Coffee> c = std::unique_ptr<Coffee>(
        new MilkDecorator(std::unique_ptr<Coffee>(new MilkDecorator(std::unique_ptr<Coffee>(new Espresso())))));
    EXPECT_NEAR(1.50, c->cost(), EPS);
}

// ---------------------------------------------------------------------------
// Polymorphic usage via base pointer
// ---------------------------------------------------------------------------
TEST(Decorator, PolymorphicDecoration)
{
    std::unique_ptr<Coffee> base(new SimpleCoffee());
    std::unique_ptr<Coffee> decorated(new MilkDecorator(std::move(base)));
    EXPECT_NEAR(0.75, decorated->cost(), EPS);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
