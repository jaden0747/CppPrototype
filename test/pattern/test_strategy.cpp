#include <gtest/gtest.h>
#include "pattern/strategy.hpp"

using namespace pattern;

namespace {
std::vector<int> unsorted() { return {5, 3, 8, 1, 9, 2}; }
std::vector<int> expected_asc() { return {1, 2, 3, 5, 8, 9}; }
std::vector<int> expected_desc() { return {9, 8, 5, 3, 2, 1}; }
} // namespace

// ---------------------------------------------------------------------------
// BubbleSort
// ---------------------------------------------------------------------------
TEST(Strategy, BubbleSortSortsAscending)
{
    Sorter sorter(makeBubbleSort());
    auto data = unsorted();
    sorter.sort(data);
    EXPECT_EQ(expected_asc(), data);
}

TEST(Strategy, BubbleSortName)
{
    Sorter sorter(makeBubbleSort());
    EXPECT_EQ("BubbleSort", sorter.strategyName());
}

// ---------------------------------------------------------------------------
// QuickSort
// ---------------------------------------------------------------------------
TEST(Strategy, QuickSortSortsAscending)
{
    Sorter sorter(makeQuickSort());
    auto data = unsorted();
    sorter.sort(data);
    EXPECT_EQ(expected_asc(), data);
}

// ---------------------------------------------------------------------------
// ReverseSort
// ---------------------------------------------------------------------------
TEST(Strategy, ReverseSortSortsDescending)
{
    Sorter sorter(makeReverseSort());
    auto data = unsorted();
    sorter.sort(data);
    EXPECT_EQ(expected_desc(), data);
}

// ---------------------------------------------------------------------------
// Strategy swap at runtime
// ---------------------------------------------------------------------------
TEST(Strategy, SwapStrategyAtRuntime)
{
    Sorter sorter(makeBubbleSort());
    auto data = unsorted();
    sorter.sort(data);
    EXPECT_EQ(expected_asc(), data);

    auto data2 = unsorted();
    sorter.setStrategy(makeReverseSort());
    sorter.sort(data2);
    EXPECT_EQ(expected_desc(), data2);
    EXPECT_EQ("ReverseSort", sorter.strategyName());
}

// ---------------------------------------------------------------------------
// Empty / single-element edge cases
// ---------------------------------------------------------------------------
TEST(Strategy, SortEmptyVector)
{
    Sorter sorter(makeQuickSort());
    std::vector<int> empty;
    EXPECT_NO_THROW(sorter.sort(empty));
    EXPECT_TRUE(empty.empty());
}

TEST(Strategy, SortSingleElement)
{
    Sorter sorter(makeBubbleSort());
    std::vector<int> single{42};
    sorter.sort(single);
    EXPECT_EQ(std::vector<int>{42}, single);
}

// ---------------------------------------------------------------------------
// FunctionalSorter (lambda strategy)
// ---------------------------------------------------------------------------
TEST(Strategy, FunctionalSorterWithLambda)
{
    FunctionalSorter sorter([](std::vector<int>& v){
        std::sort(v.begin(), v.end());
    });
    auto data = unsorted();
    sorter.sort(data);
    EXPECT_EQ(expected_asc(), data);
}

TEST(Strategy, FunctionalSorterSwap)
{
    FunctionalSorter sorter([](std::vector<int>& v){
        std::sort(v.begin(), v.end());
    });
    sorter.setStrategy([](std::vector<int>& v){
        std::sort(v.rbegin(), v.rend());
    });
    auto data = unsorted();
    sorter.sort(data);
    EXPECT_EQ(expected_desc(), data);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
