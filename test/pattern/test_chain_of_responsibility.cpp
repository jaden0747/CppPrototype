#include "pattern/chain_of_responsibility.hpp"
#include <gtest/gtest.h>

using namespace pattern;

// ---------------------------------------------------------------------------
// Basic routing
// ---------------------------------------------------------------------------
TEST(ChainOfResponsibility, Level1HandledByTier1)
{
    auto chain  = buildDefaultChain();
    auto result = chain->handle({1, "password reset"});
    EXPECT_NE(std::string::npos, result.find("Tier1"));
}

TEST(ChainOfResponsibility, Level2HandledByTier2)
{
    auto chain  = buildDefaultChain();
    auto result = chain->handle({2, "network config"});
    EXPECT_NE(std::string::npos, result.find("Tier2"));
}

TEST(ChainOfResponsibility, Level3HandledByTier3)
{
    auto chain  = buildDefaultChain();
    auto result = chain->handle({3, "kernel panic"});
    EXPECT_NE(std::string::npos, result.find("Tier3"));
}

// ---------------------------------------------------------------------------
// Description is preserved in result
// ---------------------------------------------------------------------------
TEST(ChainOfResponsibility, ResultContainsDescription)
{
    auto chain  = buildDefaultChain();
    auto result = chain->handle({2, "unique_problem_42"});
    EXPECT_NE(std::string::npos, result.find("unique_problem_42"));
}

// ---------------------------------------------------------------------------
// Unhandled request (no handler covers level 4)
// ---------------------------------------------------------------------------
TEST(ChainOfResponsibility, UnhandledRequestReturnsUnhandled)
{
    auto chain  = buildDefaultChain();
    auto result = chain->handle({4, "alien invasion"});
    EXPECT_NE(std::string::npos, result.find("Unhandled"));
}

// ---------------------------------------------------------------------------
// Partial chain (only Tier2)
// ---------------------------------------------------------------------------
TEST(ChainOfResponsibility, PartialChainTier1Unhandled)
{
    auto t2     = std::unique_ptr<SupportHandler>(new Tier2Handler());
    auto result = t2->handle({1, "basic question"});
    EXPECT_NE(std::string::npos, result.find("Unhandled"));
}

TEST(ChainOfResponsibility, PartialChainTier2Handled)
{
    auto t2     = std::unique_ptr<SupportHandler>(new Tier2Handler());
    auto result = t2->handle({2, "medium question"});
    EXPECT_NE(std::string::npos, result.find("Tier2"));
}

// ---------------------------------------------------------------------------
// setNext returns the new tail for easy chaining
// ---------------------------------------------------------------------------
TEST(ChainOfResponsibility, SetNextReturnsNextPointer)
{
    auto  t1    = std::unique_ptr<SupportHandler>(new Tier1Handler());
    auto* t2raw = new Tier2Handler();
    auto* ret   = t1->setNext(std::unique_ptr<SupportHandler>(t2raw));
    EXPECT_EQ(t2raw, ret);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
