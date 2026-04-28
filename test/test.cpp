#include <gtest/gtest.h>

#include "mylib/mylib.hpp"

TEST(TEST_SUITE, test_1)
{
    int result = Main::add(2, 3);
    EXPECT_EQ(result, 5);
}
