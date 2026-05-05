#include "pattern/builder.hpp"
#include <gtest/gtest.h>

using namespace pattern;

// ---------------------------------------------------------------------------
// Default values
// ---------------------------------------------------------------------------
TEST(Builder, MeatBuilderDefaultBun)
{
    MeatBurgerBuilder b;
    EXPECT_EQ("sesame", b.getResult().bun);
}

TEST(Builder, VeggieBuilderDefaultPatty)
{
    VeggieBurgerBuilder b;
    EXPECT_EQ("black-bean", b.getResult().patty);
}

// ---------------------------------------------------------------------------
// Step-by-step customization
// ---------------------------------------------------------------------------
TEST(Builder, SetBunOverridesDefault)
{
    MeatBurgerBuilder b;
    b.setBun("brioche");
    EXPECT_EQ("brioche", b.getResult().bun);
}

TEST(Builder, AddMultipleToppings)
{
    MeatBurgerBuilder b;
    b.addTopping("lettuce");
    b.addTopping("tomato");
    Burger result = b.getResult();
    ASSERT_EQ(2u, result.toppings.size());
    EXPECT_EQ("lettuce", result.toppings[0]);
    EXPECT_EQ("tomato", result.toppings[1]);
}

TEST(Builder, ToastedFlag)
{
    MeatBurgerBuilder b;
    b.setToasted(true);
    EXPECT_TRUE(b.getResult().toasted);
}

// ---------------------------------------------------------------------------
// Director builds known recipes
// ---------------------------------------------------------------------------
TEST(Builder, DirectorClassicHasCheese)
{
    MeatBurgerBuilder builder;
    BurgerDirector    director(&builder);
    director.buildClassic();
    Burger result    = builder.getResult();
    bool   hasCheese = false;
    for (const auto& t : result.toppings)
        if (t == "cheese")
            hasCheese = true;
    EXPECT_TRUE(hasCheese);
}

TEST(Builder, DirectorDeluxeHasMoreToppings)
{
    MeatBurgerBuilder classic;
    BurgerDirector    d1(&classic);
    d1.buildClassic();
    Burger c = classic.getResult();

    MeatBurgerBuilder deluxe;
    BurgerDirector    d2(&deluxe);
    d2.buildDeluxe();
    Burger d = deluxe.getResult();

    EXPECT_GT(d.toppings.size(), c.toppings.size());
}

// ---------------------------------------------------------------------------
// Builder resets after getResult
// ---------------------------------------------------------------------------
TEST(Builder, BuilderResetsAfterGetResult)
{
    MeatBurgerBuilder b;
    b.addTopping("onion");
    b.getResult(); // consume + reset
    Burger second = b.getResult();
    EXPECT_TRUE(second.toppings.empty());
}

// ---------------------------------------------------------------------------
// Fluent builder
// ---------------------------------------------------------------------------
TEST(Builder, FluentBuilderSetsAllFields)
{
    Burger b = FluentBurger().withBun("brioche").withPatty("turkey").withTopping("avocado").toasted().build();

    EXPECT_EQ("brioche", b.bun);
    EXPECT_EQ("turkey", b.patty);
    EXPECT_EQ(1u, b.toppings.size());
    EXPECT_EQ("avocado", b.toppings[0]);
    EXPECT_TRUE(b.toasted);
}

TEST(Builder, FluentBuilderDescribeNotEmpty)
{
    Burger b = FluentBurger().withBun("bun").withPatty("patty").build();
    EXPECT_FALSE(b.describe().empty());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
