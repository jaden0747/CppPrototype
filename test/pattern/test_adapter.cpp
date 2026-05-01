#include <gtest/gtest.h>
#include "pattern/adapter.hpp"
#include <cmath>

using namespace pattern;

static const double EPS = 1e-6;

// ---------------------------------------------------------------------------
// RoundHole / RoundPeg sanity
// ---------------------------------------------------------------------------
TEST(Adapter, RoundPegFitsInLargerHole)
{
    RoundHole hole(5.0);
    RoundPeg  peg(4.0);
    EXPECT_TRUE(hole.fits(peg.radius()));
}

TEST(Adapter, RoundPegDoesNotFitInSmallerHole)
{
    RoundHole hole(3.0);
    RoundPeg  peg(4.0);
    EXPECT_FALSE(hole.fits(peg.radius()));
}

TEST(Adapter, RoundPegFitsExactly)
{
    RoundHole hole(5.0);
    RoundPeg  peg(5.0);
    EXPECT_TRUE(hole.fits(peg.radius()));
}

// ---------------------------------------------------------------------------
// Object adapter: radius calculation
// ---------------------------------------------------------------------------
TEST(Adapter, AdapterRadiusIsHalfDiagonal)
{
    SquarePeg        peg(2.0);       // width = 2
    SquarePegAdapter adapter(peg);   // radius = 2 * sqrt(2) / 2 ≈ 1.414

    double expected = 2.0 * std::sqrt(2.0) / 2.0;
    EXPECT_NEAR(expected, adapter.radius(), EPS);
}

TEST(Adapter, SmallSquarePegFitsInBigHole)
{
    RoundHole        hole(5.0);
    SquarePeg        peg(4.0);       // enclosing circle radius ≈ 2.828
    SquarePegAdapter adapter(peg);
    EXPECT_TRUE(hole.fits(adapter.radius()));
}

TEST(Adapter, LargeSquarePegDoesNotFit)
{
    RoundHole        hole(5.0);
    SquarePeg        peg(8.0);       // enclosing circle radius ≈ 5.657
    SquarePegAdapter adapter(peg);
    EXPECT_FALSE(hole.fits(adapter.radius()));
}

// ---------------------------------------------------------------------------
// Class adapter
// ---------------------------------------------------------------------------
TEST(Adapter, ClassAdapterRadiusIsHalfDiagonal)
{
    SquarePegClassAdapter adapter(2.0);
    double expected = 2.0 * std::sqrt(2.0) / 2.0;
    EXPECT_NEAR(expected, adapter.radius(), EPS);
}

TEST(Adapter, ClassAdapterSmallPegFits)
{
    RoundHole             hole(10.0);
    SquarePegClassAdapter adapter(4.0);
    EXPECT_TRUE(hole.fits(adapter.radius()));
}

// ---------------------------------------------------------------------------
// SquarePeg describe
// ---------------------------------------------------------------------------
TEST(Adapter, SquarePegDescribeContainsWidth)
{
    SquarePeg peg(6.5);
    EXPECT_NE(std::string::npos, peg.describe().find("6.5"));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
