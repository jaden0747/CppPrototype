#include "pattern/flyweight.hpp"
#include <gtest/gtest.h>

using namespace pattern;

// ---------------------------------------------------------------------------
// TreeType (flyweight)
// ---------------------------------------------------------------------------
TEST(Flyweight, TreeTypeStoresName)
{
    TreeType t("Oak", "green", "rough");
    EXPECT_EQ("Oak", t.name());
}

TEST(Flyweight, TreeTypeRenderContainsCoords)
{
    TreeType    t("Oak", "green", "rough");
    std::string result = t.render(10, 20);
    EXPECT_NE(std::string::npos, result.find("10"));
    EXPECT_NE(std::string::npos, result.find("20"));
}

// ---------------------------------------------------------------------------
// Factory sharing
// ---------------------------------------------------------------------------
TEST(Flyweight, FactoryReturnsSameInstanceForSameKey)
{
    TreeTypeFactory factory;
    auto            a = factory.getTreeType("Oak", "green", "rough");
    auto            b = factory.getTreeType("Oak", "green", "rough");
    EXPECT_EQ(a.get(), b.get());
}

TEST(Flyweight, FactoryReturnsDifferentInstanceForDifferentKey)
{
    TreeTypeFactory factory;
    auto            a = factory.getTreeType("Oak", "green", "rough");
    auto            b = factory.getTreeType("Pine", "yellow", "smooth");
    EXPECT_NE(a.get(), b.get());
}

TEST(Flyweight, FactoryCacheSizeGrowsForNewTypes)
{
    TreeTypeFactory factory;
    factory.getTreeType("Oak", "green", "rough");
    EXPECT_EQ(1u, factory.cacheSize());
    factory.getTreeType("Pine", "yellow", "smooth");
    EXPECT_EQ(2u, factory.cacheSize());
    factory.getTreeType("Oak", "green", "rough"); // duplicate
    EXPECT_EQ(2u, factory.cacheSize());
}

// ---------------------------------------------------------------------------
// Forest
// ---------------------------------------------------------------------------
TEST(Flyweight, ForestTracksAllTrees)
{
    Forest f;
    f.plantTree(0, 0, "Oak", "green", "rough");
    f.plantTree(1, 2, "Oak", "green", "rough");
    f.plantTree(3, 4, "Pine", "yellow", "smooth");
    EXPECT_EQ(3u, f.treeCount());
}

TEST(Flyweight, ForestDeduplicatesTypes)
{
    Forest f;
    for (int i = 0; i < 100; ++i)
        f.plantTree(i, i, "Oak", "green", "rough");
    EXPECT_EQ(100u, f.treeCount());
    EXPECT_EQ(1u, f.uniqueTypeCount()); // only one TreeType created
}

TEST(Flyweight, ForestMultipleTypesDeduplication)
{
    Forest f;
    for (int i = 0; i < 50; ++i)
        f.plantTree(i, 0, "Oak", "green", "rough");
    for (int i = 0; i < 50; ++i)
        f.plantTree(i, 1, "Pine", "yellow", "smooth");
    EXPECT_EQ(100u, f.treeCount());
    EXPECT_EQ(2u, f.uniqueTypeCount());
}

TEST(Flyweight, TreeContextRenderContainsPosition)
{
    Forest f;
    f.plantTree(42, 7, "Oak", "green", "rough");
    std::string r = f.treeAt(0).render();
    EXPECT_NE(std::string::npos, r.find("42"));
    EXPECT_NE(std::string::npos, r.find("7"));
}

TEST(Flyweight, SharedTypePointerIdentity)
{
    Forest f;
    f.plantTree(0, 0, "Oak", "green", "rough");
    f.plantTree(1, 1, "Oak", "green", "rough");
    EXPECT_EQ(f.treeAt(0).type.get(), f.treeAt(1).type.get());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
