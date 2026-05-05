#include "pattern/observer.hpp"
#include <gtest/gtest.h>

using namespace pattern;

// ---------------------------------------------------------------------------
// StockMarket / IObserver
// ---------------------------------------------------------------------------
TEST(Observer, AttachIncreasesSubscriberCount)
{
    StockMarket market;
    Logger      logger;
    market.attach(&logger);
    EXPECT_EQ(1u, market.subscriberCount());
}

TEST(Observer, DetachDecreasesSubscriberCount)
{
    StockMarket market;
    Logger      logger;
    market.attach(&logger);
    market.detach(&logger);
    EXPECT_EQ(0u, market.subscriberCount());
}

TEST(Observer, LoggerReceivesEvent)
{
    StockMarket market;
    Logger      logger;
    market.attach(&logger);
    market.setPrice("AAPL", 150.0);
    ASSERT_EQ(1u, logger.log().size());
    EXPECT_NE(std::string::npos, logger.log()[0].find("AAPL"));
}

TEST(Observer, MultipleObserversAllNotified)
{
    StockMarket market;
    Logger      l1, l2;
    market.attach(&l1);
    market.attach(&l2);
    market.setPrice("GOOG", 2800.0);
    EXPECT_EQ(1u, l1.log().size());
    EXPECT_EQ(1u, l2.log().size());
}

TEST(Observer, DetachedObserverNotNotified)
{
    StockMarket market;
    Logger      l1, l2;
    market.attach(&l1);
    market.attach(&l2);
    market.detach(&l2);
    market.setPrice("MSFT", 300.0);
    EXPECT_EQ(1u, l1.log().size());
    EXPECT_EQ(0u, l2.log().size());
}

TEST(Observer, AlertMonitorFiresAboveThreshold)
{
    StockMarket  market;
    AlertMonitor monitor(200.0);
    market.attach(&monitor);
    market.setPrice("TSLA", 250.0);
    EXPECT_EQ(1u, monitor.alerts().size());
}

TEST(Observer, AlertMonitorSilentBelowThreshold)
{
    StockMarket  market;
    AlertMonitor monitor(200.0);
    market.attach(&monitor);
    market.setPrice("TSLA", 100.0);
    EXPECT_TRUE(monitor.alerts().empty());
}

// ---------------------------------------------------------------------------
// EventEmitter<T> (generic / callback-based)
// ---------------------------------------------------------------------------
TEST(Observer, EventEmitterSubscribeAndEmit)
{
    EventEmitter<int> emitter;
    int               received = -1;
    emitter.subscribe([&](const int& v) { received = v; });
    emitter.emit(42);
    EXPECT_EQ(42, received);
}

TEST(Observer, EventEmitterMultipleSubscribers)
{
    EventEmitter<std::string> emitter;
    std::vector<std::string>  log;
    emitter.subscribe([&](const std::string& s) { log.push_back("A:" + s); });
    emitter.subscribe([&](const std::string& s) { log.push_back("B:" + s); });
    emitter.emit("hello");
    EXPECT_EQ(2u, log.size());
}

TEST(Observer, EventEmitterUnsubscribe)
{
    EventEmitter<int> emitter;
    int               countA = 0, countB = 0;
    emitter.subscribe([&](const int&) { ++countA; });
    int idB = emitter.subscribe([&](const int&) { ++countB; });
    emitter.unsubscribe(idB);
    emitter.emit(1);
    EXPECT_EQ(1, countA);
    EXPECT_EQ(0, countB);
}

TEST(Observer, EventEmitterSubscriberCount)
{
    EventEmitter<int> emitter;
    auto              id = emitter.subscribe([](const int&) {});
    EXPECT_EQ(1u, emitter.subscriberCount());
    emitter.unsubscribe(id);
    EXPECT_EQ(0u, emitter.subscriberCount());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
