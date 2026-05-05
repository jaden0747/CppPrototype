#include "pattern/singleton.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using pattern::Singleton;

// ---------------------------------------------------------------------------
// Basic identity tests
// ---------------------------------------------------------------------------
TEST(Singleton, SameInstanceReturned)
{
    Singleton& a = Singleton::instance();
    Singleton& b = Singleton::instance();
    EXPECT_EQ(&a, &b);
}

TEST(Singleton, ValuePersistsBetweenCalls)
{
    Singleton::instance().reset();
    Singleton::instance().increment();
    Singleton::instance().increment();
    EXPECT_EQ(2, Singleton::instance().value());
}

TEST(Singleton, ResetWorks)
{
    Singleton::instance().increment();
    Singleton::instance().reset();
    EXPECT_EQ(0, Singleton::instance().value());
}

// ---------------------------------------------------------------------------
// Copy / move prevention (compile-time: verified by deleted special members)
// We can document it with a static_assert.
// ---------------------------------------------------------------------------
TEST(Singleton, NotCopyConstructible)
{
    EXPECT_FALSE(std::is_copy_constructible<Singleton>::value);
}

TEST(Singleton, NotCopyAssignable)
{
    EXPECT_FALSE(std::is_copy_assignable<Singleton>::value);
}

TEST(Singleton, NotMoveConstructible)
{
    EXPECT_FALSE(std::is_move_constructible<Singleton>::value);
}

// ---------------------------------------------------------------------------
// Thread-safety: many threads must all get the same address
// ---------------------------------------------------------------------------
TEST(Singleton, ThreadSafeInitialization)
{
    const int                NUM_THREADS = 32;
    std::vector<Singleton*>  ptrs(NUM_THREADS, nullptr);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&ptrs, i]() { ptrs[i] = &Singleton::instance(); });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 1; i < NUM_THREADS; ++i)
        EXPECT_EQ(ptrs[0], ptrs[i]);
}

// ---------------------------------------------------------------------------
// State isolation: reset before each logical section
// ---------------------------------------------------------------------------
TEST(Singleton, IncrementAccumulates)
{
    Singleton::instance().reset();
    for (int i = 0; i < 5; ++i)
        Singleton::instance().increment();
    EXPECT_EQ(5, Singleton::instance().value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
