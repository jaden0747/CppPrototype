#include "pattern/iterator.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <numeric>
#include <vector>

using namespace pattern;

// ---------------------------------------------------------------------------
// ForwardIterator
// ---------------------------------------------------------------------------
TEST(Iterator, ForwardIteratesAllWords)
{
    WordCollection col;
    col.add("alpha");
    col.add("beta");
    col.add("gamma");

    std::vector<std::string> result;
    auto                     it = col.forwardIterator();
    while (it.hasNext())
        result.push_back(it.next());

    ASSERT_EQ(3u, result.size());
    EXPECT_EQ("alpha", result[0]);
    EXPECT_EQ("beta", result[1]);
    EXPECT_EQ("gamma", result[2]);
}

TEST(Iterator, ForwardIteratorEmptyCollection)
{
    WordCollection col;
    auto           it = col.forwardIterator();
    EXPECT_FALSE(it.hasNext());
}

TEST(Iterator, ForwardIteratorExhaustedThrows)
{
    WordCollection col;
    col.add("only");
    auto it = col.forwardIterator();
    it.next();
    EXPECT_THROW(it.next(), std::out_of_range);
}

// ---------------------------------------------------------------------------
// ReverseIterator
// ---------------------------------------------------------------------------
TEST(Iterator, ReverseIteratesInReverse)
{
    WordCollection col;
    col.add("a");
    col.add("b");
    col.add("c");

    std::vector<std::string> result;
    auto                     it = col.reverseIterator();
    while (it.hasNext())
        result.push_back(it.next());

    ASSERT_EQ(3u, result.size());
    EXPECT_EQ("c", result[0]);
    EXPECT_EQ("b", result[1]);
    EXPECT_EQ("a", result[2]);
}

TEST(Iterator, ReverseIteratorEmptyCollection)
{
    WordCollection col;
    auto           it = col.reverseIterator();
    EXPECT_FALSE(it.hasNext());
}

// ---------------------------------------------------------------------------
// STL-compatible NumberRange
// ---------------------------------------------------------------------------
TEST(Iterator, NumberRangeRangeFor)
{
    NumberRange      range(1, 5);
    std::vector<int> result;
    for (int n : range)
        result.push_back(n);

    EXPECT_EQ(std::vector<int>({1, 2, 3, 4, 5}), result);
}

TEST(Iterator, NumberRangeStdAccumulate)
{
    NumberRange range(1, 10);
    int         sum = std::accumulate(range.begin(), range.end(), 0);
    EXPECT_EQ(55, sum);
}

TEST(Iterator, NumberRangeEmptyWhenFromGreaterThanTo)
{
    NumberRange range(5, 4);
    int         count = 0;
    for (int n : range)
    {
        (void)n;
        ++count;
    }
    EXPECT_EQ(0, count);
}

TEST(Iterator, NumberRangeSingleElement)
{
    NumberRange      range(7, 7);
    std::vector<int> result(range.begin(), range.end());
    EXPECT_EQ(std::vector<int>({7}), result);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
