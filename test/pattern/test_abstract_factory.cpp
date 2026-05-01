#include <gtest/gtest.h>
#include "pattern/abstract_factory.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// Modern factory tests
// ---------------------------------------------------------------------------
TEST(AbstractFactory, ModernChairHasCorrectStyle)
{
    ModernFurnitureFactory factory;
    auto chair = factory.createChair();
    ASSERT_NE(nullptr, chair.get());
    EXPECT_EQ("Modern", chair->style());
}

TEST(AbstractFactory, ModernSofaHasCorrectStyle)
{
    ModernFurnitureFactory factory;
    auto sofa = factory.createSofa();
    EXPECT_EQ("Modern", sofa->style());
}

TEST(AbstractFactory, ModernCoffeeTableHasCorrectStyle)
{
    ModernFurnitureFactory factory;
    auto table = factory.createCoffeeTable();
    EXPECT_EQ("Modern", table->style());
}

// ---------------------------------------------------------------------------
// Victorian factory tests
// ---------------------------------------------------------------------------
TEST(AbstractFactory, VictorianChairHasCorrectStyle)
{
    VictorianFurnitureFactory factory;
    auto chair = factory.createChair();
    EXPECT_EQ("Victorian", chair->style());
}

TEST(AbstractFactory, VictorianSofaHasCorrectStyle)
{
    VictorianFurnitureFactory factory;
    auto sofa = factory.createSofa();
    EXPECT_EQ("Victorian", sofa->style());
}

TEST(AbstractFactory, VictorianCoffeeTableHasCorrectStyle)
{
    VictorianFurnitureFactory factory;
    auto table = factory.createCoffeeTable();
    EXPECT_EQ("Victorian", table->style());
}

// ---------------------------------------------------------------------------
// Families are consistent (same factory produces matching styles)
// ---------------------------------------------------------------------------
TEST(AbstractFactory, ModernFamilyIsConsistent)
{
    ModernFurnitureFactory f;
    EXPECT_EQ(f.createChair()->style(),       f.createSofa()->style());
    EXPECT_EQ(f.createSofa()->style(),        f.createCoffeeTable()->style());
}

TEST(AbstractFactory, VictorianFamilyIsConsistent)
{
    VictorianFurnitureFactory f;
    EXPECT_EQ(f.createChair()->style(),       f.createSofa()->style());
    EXPECT_EQ(f.createSofa()->style(),        f.createCoffeeTable()->style());
}

// ---------------------------------------------------------------------------
// Polymorphic usage through abstract interface
// ---------------------------------------------------------------------------
TEST(AbstractFactory, PolymorphicFactory)
{
    std::unique_ptr<FurnitureFactory> factory =
        std::unique_ptr<FurnitureFactory>(new VictorianFurnitureFactory());
    auto chair = factory->createChair();
    EXPECT_NE(std::string::npos, chair->sitOn().find("Victorian"));
}

// ---------------------------------------------------------------------------
// Helper factory function
// ---------------------------------------------------------------------------
TEST(AbstractFactory, MakeFactoryModern)
{
    auto f = makeFurnitureFactory("modern");
    EXPECT_EQ("Modern", f->createChair()->style());
}

TEST(AbstractFactory, MakeFactoryVictorian)
{
    auto f = makeFurnitureFactory("victorian");
    EXPECT_EQ("Victorian", f->createChair()->style());
}

TEST(AbstractFactory, MakeFactoryUnknownThrows)
{
    EXPECT_THROW(makeFurnitureFactory("art-deco"), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Each product creation returns a distinct object
// ---------------------------------------------------------------------------
TEST(AbstractFactory, EachCreationIsUnique)
{
    ModernFurnitureFactory f;
    auto c1 = f.createChair();
    auto c2 = f.createChair();
    EXPECT_NE(c1.get(), c2.get());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
