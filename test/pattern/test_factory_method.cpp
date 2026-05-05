#include "pattern/factory_method.hpp"
#include <gtest/gtest.h>

using namespace pattern;

// ---------------------------------------------------------------------------
// Product creation tests
// ---------------------------------------------------------------------------
TEST(FactoryMethod, RoadLogisticsCreatesTruck)
{
    RoadLogistics logistics;
    auto          transport = logistics.createTransport();
    ASSERT_NE(nullptr, transport.get());
    EXPECT_EQ("Truck", transport->type());
}

TEST(FactoryMethod, SeaLogisticsCreatesShip)
{
    SeaLogistics logistics;
    auto         transport = logistics.createTransport();
    ASSERT_NE(nullptr, transport.get());
    EXPECT_EQ("Ship", transport->type());
}

TEST(FactoryMethod, AirLogisticsCreatesPlane)
{
    AirLogistics logistics;
    auto         transport = logistics.createTransport();
    ASSERT_NE(nullptr, transport.get());
    EXPECT_EQ("Plane", transport->type());
}

// ---------------------------------------------------------------------------
// Deliver message tests
// ---------------------------------------------------------------------------
TEST(FactoryMethod, TruckDeliverMessage)
{
    Truck t;
    EXPECT_NE(std::string::npos, t.deliver().find("land"));
}

TEST(FactoryMethod, ShipDeliverMessage)
{
    Ship s;
    EXPECT_NE(std::string::npos, s.deliver().find("sea"));
}

TEST(FactoryMethod, PlaneDeliverMessage)
{
    Plane p;
    EXPECT_NE(std::string::npos, p.deliver().find("air"));
}

// ---------------------------------------------------------------------------
// planDelivery (template method using factory method)
// ---------------------------------------------------------------------------
TEST(FactoryMethod, PlanDeliveryUsesCorrectTransport)
{
    RoadLogistics road;
    EXPECT_NE(std::string::npos, road.planDelivery().find("Truck"));

    SeaLogistics sea;
    EXPECT_NE(std::string::npos, sea.planDelivery().find("Ship"));

    AirLogistics air;
    EXPECT_NE(std::string::npos, air.planDelivery().find("Plane"));
}

// ---------------------------------------------------------------------------
// Polymorphism through base pointer
// ---------------------------------------------------------------------------
TEST(FactoryMethod, PolymorphicUsage)
{
    std::unique_ptr<Logistics> logistics = std::unique_ptr<Logistics>(new SeaLogistics());
    auto                       transport = logistics->createTransport();
    EXPECT_EQ("Ship", transport->type());
}

// ---------------------------------------------------------------------------
// Helper factory function
// ---------------------------------------------------------------------------
TEST(FactoryMethod, MakeLogisticsRoad)
{
    auto l = makeLogistics("road");
    ASSERT_NE(nullptr, l.get());
    EXPECT_NE(std::string::npos, l->planDelivery().find("Truck"));
}

TEST(FactoryMethod, MakeLogisticsUnknownThrows)
{
    EXPECT_THROW(makeLogistics("teleport"), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Each call to createTransport returns a distinct object (not shared)
// ---------------------------------------------------------------------------
TEST(FactoryMethod, EachCreationIsUnique)
{
    RoadLogistics logistics;
    auto          t1 = logistics.createTransport();
    auto          t2 = logistics.createTransport();
    EXPECT_NE(t1.get(), t2.get());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
