#include <gtest/gtest.h>
#include "pattern/state.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// Initial state
// ---------------------------------------------------------------------------
TEST(State, InitialStateIsIdle)
{
    VendingMachine vm(3);
    EXPECT_EQ("Idle", vm.stateName());
}

TEST(State, ZeroStockStartsOutOfStock)
{
    VendingMachine vm(0);
    EXPECT_EQ("OutOfStock", vm.stateName());
}

// ---------------------------------------------------------------------------
// Happy path
// ---------------------------------------------------------------------------
TEST(State, InsertCoinTransitionsToHasCoin)
{
    VendingMachine vm(1);
    vm.insertCoin();
    EXPECT_EQ("HasCoin", vm.stateName());
}

TEST(State, SelectProductTransitionsToDispensing)
{
    VendingMachine vm(1);
    vm.insertCoin();
    vm.selectProduct();
    EXPECT_EQ("Dispensing", vm.stateName());
}

TEST(State, DispenseReducesStock)
{
    VendingMachine vm(2);
    vm.insertCoin();
    vm.selectProduct();
    vm.dispense();
    EXPECT_EQ(1, vm.stock());
}

TEST(State, DispenseReturnsToIdle)
{
    VendingMachine vm(2);
    vm.insertCoin();
    vm.selectProduct();
    vm.dispense();
    EXPECT_EQ("Idle", vm.stateName());
}

TEST(State, LastItemTransitionsToOutOfStock)
{
    VendingMachine vm(1);
    vm.insertCoin();
    vm.selectProduct();
    vm.dispense();
    EXPECT_EQ("OutOfStock", vm.stateName());
    EXPECT_EQ(0, vm.stock());
}

// ---------------------------------------------------------------------------
// Error paths
// ---------------------------------------------------------------------------
TEST(State, SelectBeforeCoinThrows)
{
    VendingMachine vm(1);
    EXPECT_THROW(vm.selectProduct(), std::logic_error);
}

TEST(State, DispenseBeforeCoinThrows)
{
    VendingMachine vm(1);
    EXPECT_THROW(vm.dispense(), std::logic_error);
}

TEST(State, InsertCoinTwiceThrows)
{
    VendingMachine vm(1);
    vm.insertCoin();
    EXPECT_THROW(vm.insertCoin(), std::logic_error);
}

TEST(State, OutOfStockInsertCoinThrows)
{
    VendingMachine vm(0);
    EXPECT_THROW(vm.insertCoin(), std::logic_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
