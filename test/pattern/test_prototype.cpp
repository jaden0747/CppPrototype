#include <gtest/gtest.h>
#include "pattern/prototype.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// Basic clone tests
// ---------------------------------------------------------------------------
TEST(Prototype, CloneProducesNewObject)
{
    Circle original("red", 5.0);
    auto cloned = original.clone();
    EXPECT_NE(&original, cloned.get());
}

TEST(Prototype, ClonePreservesType)
{
    Circle original("blue", 3.0);
    auto cloned = original.clone();
    EXPECT_EQ("Circle", cloned->type());
}

TEST(Prototype, ClonePreservesColor)
{
    Circle original("green", 4.0);
    auto cloned = original.clone();
    EXPECT_EQ("green", cloned->color());
}

TEST(Prototype, CloneCirclePreservesRadius)
{
    Circle original("red", 7.5);
    auto cloned = original.clone();
    Circle* c = dynamic_cast<Circle*>(cloned.get());
    ASSERT_NE(nullptr, c);
    EXPECT_DOUBLE_EQ(7.5, c->radius());
}

TEST(Prototype, CloneRectanglePreservesDimensions)
{
    Rectangle original("yellow", 10.0, 20.0);
    auto cloned = original.clone();
    Rectangle* r = dynamic_cast<Rectangle*>(cloned.get());
    ASSERT_NE(nullptr, r);
    EXPECT_DOUBLE_EQ(10.0, r->width());
    EXPECT_DOUBLE_EQ(20.0, r->height());
}

// ---------------------------------------------------------------------------
// Independence: mutating clone does not affect original
// ---------------------------------------------------------------------------
TEST(Prototype, MutatingCloneDoesNotAffectOriginal)
{
    Circle original("red", 5.0);
    auto cloned = original.clone();
    cloned->setColor("blue");
    EXPECT_EQ("red",  original.color());
    EXPECT_EQ("blue", cloned->color());
}

TEST(Prototype, MutatingCircleRadiusDoesNotAffectOriginal)
{
    Circle original("red", 5.0);
    auto cloned = original.clone();
    Circle* c = dynamic_cast<Circle*>(cloned.get());
    c->setRadius(99.0);
    EXPECT_DOUBLE_EQ(5.0,  original.radius());
    EXPECT_DOUBLE_EQ(99.0, c->radius());
}

// ---------------------------------------------------------------------------
// Registry tests
// ---------------------------------------------------------------------------
TEST(Prototype, RegistryReturnsClonesNotOriginals)
{
    ShapeRegistry reg;
    reg.add("circle", std::unique_ptr<Shape>(new Circle("red", 1.0)));

    auto a = reg.get("circle");
    auto b = reg.get("circle");
    EXPECT_NE(a.get(), b.get());
}

TEST(Prototype, RegistryPreservesProperties)
{
    ShapeRegistry reg;
    reg.add("rect", std::unique_ptr<Shape>(new Rectangle("blue", 4.0, 8.0)));

    auto s = reg.get("rect");
    EXPECT_EQ("blue",      s->color());
    EXPECT_EQ("Rectangle", s->type());
}

TEST(Prototype, RegistryThrowsOnMissingKey)
{
    ShapeRegistry reg;
    EXPECT_THROW(reg.get("nonexistent"), std::out_of_range);
}

TEST(Prototype, RegistryHas)
{
    ShapeRegistry reg;
    reg.add("circle", std::unique_ptr<Shape>(new Circle("red", 1.0)));
    EXPECT_TRUE(reg.has("circle"));
    EXPECT_FALSE(reg.has("triangle"));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
