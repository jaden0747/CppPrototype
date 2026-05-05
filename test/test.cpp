#include <gtest/gtest.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "mylib/data_container.hpp"
#include "mylib/mylib.hpp"

// ---------------------------------------------------------------------------
// Sample custom type used across all data_container tests.
// ---------------------------------------------------------------------------
struct SensorReading
{
    uint32_t sensorId    = 0;
    float    temperature = 0.0f;
    float    humidity    = 0.0f;

    bool operator==(const SensorReading& o) const
    {
        return sensorId == o.sensorId && temperature == o.temperature && humidity == o.humidity;
    }
};

// ---------------------------------------------------------------------------
// Mempool tests
// ---------------------------------------------------------------------------
TEST(Mempool, AcquireReturnsFreeSlot)
{
    dc::Mempool<SensorReading> pool(4);
    auto*                      buf = pool.acquire();
    ASSERT_NE(buf, nullptr);
    EXPECT_EQ(buf->refs.load(), 1);
}

TEST(Mempool, PoolExhaustionReturnsNullptrOnlyWhenAllSlotsLive)
{
    dc::Mempool<SensorReading> pool(2);
    auto*                      b0 = pool.acquire();
    auto*                      b1 = pool.acquire();
    ASSERT_NE(b0, nullptr);
    ASSERT_NE(b1, nullptr);
    // Both slots are live (refs > 0) — ring cannot evict safely.
    EXPECT_EQ(pool.acquire(), nullptr);
    pool.release(*b0);
    pool.release(*b1);
}

TEST(Mempool, RingBufferEvictsOldestFreeSlot)
{
    dc::Mempool<SensorReading> pool(2);
    // Acquire both slots, then release the first.
    auto* b0 = pool.acquire(); // ring index 0
    auto* b1 = pool.acquire(); // ring index 1
    ASSERT_NE(b0, nullptr);
    ASSERT_NE(b1, nullptr);
    pool.release(*b0); // b0 is now free (refs == 0), b1 still live
    // acquire() should evict b0 even though b1 is still held.
    auto* b2 = pool.acquire();
    EXPECT_NE(b2, nullptr);
    EXPECT_EQ(b2->index, b0->index); // same physical slot recycled
    pool.release(*b1);
    pool.release(*b2);
}

TEST(Mempool, ReleaseReturnsSlotToPool)
{
    dc::Mempool<SensorReading> pool(1);
    auto*                      buf = pool.acquire();
    ASSERT_NE(buf, nullptr);
    EXPECT_EQ(pool.acquire(), nullptr); // only slot is live
    pool.release(*buf);
    EXPECT_NE(pool.acquire(), nullptr); // slot now evictable
}

TEST(Mempool, PoolCountMatchesConstruction)
{
    dc::Mempool<SensorReading> pool(8);
    EXPECT_EQ(pool.poolCount(), 8u);
}

// ---------------------------------------------------------------------------
// SenderPort tests
// ---------------------------------------------------------------------------
TEST(SenderPort, ReserveWithoutMempoolReturnsNullptr)
{
    dc::SenderPort<SensorReading> sender;
    EXPECT_EQ(sender.reserve(), nullptr);
}

TEST(SenderPort, ReserveWithMempoolReturnsPointer)
{
    dc::Mempool<SensorReading>    pool(4);
    dc::SenderPort<SensorReading> sender;
    sender.connectMempool(pool);
    EXPECT_NE(sender.reserve(), nullptr);
}

TEST(SenderPort, IsConnectedFalseWithNoReceivers)
{
    dc::SenderPort<SensorReading> sender;
    EXPECT_FALSE(sender.isConnected());
}

TEST(SenderPort, IsConnectedAfterReceiverConnects)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);
    EXPECT_TRUE(sender.isConnected());
}

TEST(SenderPort, DeliverWithNoReceiversDoesNotCrash)
{
    dc::Mempool<SensorReading>    pool(4);
    dc::SenderPort<SensorReading> sender;
    sender.connectMempool(pool);
    SensorReading* slot = sender.reserve();
    ASSERT_NE(slot, nullptr);
    slot->sensorId = 7;
    sender.deliver(); // no receivers — should just release the slot
    // All 4 slots are now unreferenced and evictable.
    EXPECT_NE(pool.acquire(), nullptr);
}

TEST(SenderPort, DisconnectMempoolDropsReservation)
{
    dc::Mempool<SensorReading>    pool(1);
    dc::SenderPort<SensorReading> sender;
    sender.connectMempool(pool);
    ASSERT_NE(sender.reserve(), nullptr); // acquires the only slot (refs=1)
    EXPECT_EQ(pool.acquire(), nullptr);   // slot is live — cannot evict
    sender.disconnectMempool();           // releases reservation (refs→0)
    EXPECT_NE(pool.acquire(), nullptr);   // slot now evictable
}

// ---------------------------------------------------------------------------
// ReceiverPort tests
// ---------------------------------------------------------------------------
TEST(ReceiverPort, InitialStateIsEmpty)
{
    dc::ReceiverPort<SensorReading> recv;
    EXPECT_FALSE(recv.isConnected());
    EXPECT_FALSE(recv.hasData());
    EXPECT_FALSE(recv.hasNewData());
    EXPECT_EQ(recv.getData(), nullptr);
}

TEST(ReceiverPort, UpdateWithoutDeliverHasNoNewData)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    recv.update();
    EXPECT_FALSE(recv.hasNewData());
    EXPECT_FALSE(recv.hasData());
}

TEST(ReceiverPort, UpdateAfterDeliverProvidesData)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    SensorReading* slot = sender.reserve();
    ASSERT_NE(slot, nullptr);
    *slot = {42u, 23.5f, 60.0f};
    sender.deliver();

    recv.update();
    ASSERT_TRUE(recv.hasNewData());
    ASSERT_TRUE(recv.hasData());

    const SensorReading* data = recv.getData();
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->sensorId, 42u);
    EXPECT_FLOAT_EQ(data->temperature, 23.5f);
    EXPECT_FLOAT_EQ(data->humidity, 60.0f);
}

TEST(ReceiverPort, CleanupClearsNewDataFlag)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    *sender.reserve() = {1u, 0.0f, 0.0f};
    sender.deliver();

    recv.update();
    EXPECT_TRUE(recv.hasNewData());
    recv.cleanup();
    EXPECT_FALSE(recv.hasNewData());
    EXPECT_TRUE(recv.hasData()); // active buffer persists until replaced
}

TEST(ReceiverPort, HasDataPersistsAcrossFrames)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    *sender.reserve() = {5u, 10.0f, 20.0f};
    sender.deliver();

    recv.update();
    recv.cleanup();

    // Second frame: no new data delivered
    recv.update();
    EXPECT_FALSE(recv.hasNewData());
    EXPECT_TRUE(recv.hasData()); // stale data still accessible
    EXPECT_EQ(recv.getData()->sensorId, 5u);
}

// ---------------------------------------------------------------------------
// Multiclient fan-out
// ---------------------------------------------------------------------------
TEST(Multiclient, OneDeliverReachesAllReceivers)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> r0, r1, r2;
    sender.connectMempool(pool);
    r0.connect(sender);
    r1.connect(sender);
    r2.connect(sender);

    SensorReading* slot = sender.reserve();
    ASSERT_NE(slot, nullptr);
    *slot = {99u, 1.0f, 2.0f};
    sender.deliver();

    for (auto* r : {&r0, &r1, &r2})
    {
        r->update();
        ASSERT_TRUE(r->hasNewData());
        EXPECT_EQ(r->getData()->sensorId, 99u);
        r->cleanup();
    }
}

TEST(Multiclient, EachReceiverGetsIndependentUpdateCycle)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> r0, r1;
    sender.connectMempool(pool);
    r0.connect(sender);
    r1.connect(sender);

    *sender.reserve() = {10u, 0.0f, 0.0f};
    sender.deliver();

    r0.update();
    // r1 has not called update yet — r0's state should not affect r1
    EXPECT_TRUE(r0.hasNewData());
    EXPECT_FALSE(r1.hasNewData());

    r1.update();
    EXPECT_TRUE(r1.hasNewData());
}

// ---------------------------------------------------------------------------
// Latest-wins pending buffer policy
// ---------------------------------------------------------------------------
TEST(ReceiverPort, LatestWinsWhenTwoDeliversBeforeUpdate)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    *sender.reserve() = {1u, 1.0f, 1.0f};
    sender.deliver();

    *sender.reserve() = {2u, 2.0f, 2.0f}; // second delivery before update
    sender.deliver();

    recv.update();
    ASSERT_TRUE(recv.hasNewData());
    EXPECT_EQ(recv.getData()->sensorId, 2u); // latest wins
}

// ---------------------------------------------------------------------------
// Reverse
// ---------------------------------------------------------------------------
TEST(SenderPort, ReverseSwapsBytes)
{
    dc::Mempool<uint32_t>      pool(4);
    dc::SenderPort<uint32_t>   sender;
    dc::ReceiverPort<uint32_t> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    *sender.reserve() = 0x01020304u;
    sender.reverse();
    sender.deliver();

    recv.update();
    ASSERT_TRUE(recv.hasNewData());
    EXPECT_EQ(*recv.getData(), 0x04030201u);
}

// ---------------------------------------------------------------------------
// Disconnect / lifetime safety
// ---------------------------------------------------------------------------
TEST(ReceiverPort, DisconnectPreventsDataAfterwards)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);
    recv.disconnect();
    EXPECT_FALSE(recv.isConnected());

    *sender.reserve() = {3u, 0.0f, 0.0f};
    sender.deliver();

    recv.update();
    EXPECT_FALSE(recv.hasNewData());
    EXPECT_FALSE(recv.hasData());
}

TEST(SenderPort, DestroyedSenderNullifiesReceiverConnection)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::ReceiverPort<SensorReading> recv;
    {
        dc::SenderPort<SensorReading> sender;
        sender.connectMempool(pool);
        recv.connect(sender);
        EXPECT_TRUE(recv.isConnected());
    } // sender destroyed here
    EXPECT_FALSE(recv.isConnected());
}

TEST(ReceiverPort, ReconnectToNewSender)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   s0, s1;
    dc::ReceiverPort<SensorReading> recv;
    s0.connectMempool(pool);
    s1.connectMempool(pool);

    recv.connect(s0);
    EXPECT_TRUE(recv.isConnected());
    EXPECT_TRUE(s0.isConnected());

    recv.connect(s1); // implicitly disconnects from s0
    EXPECT_FALSE(s0.isConnected());
    EXPECT_TRUE(s1.isConnected());
    EXPECT_TRUE(recv.isConnected());

    *s1.reserve() = {7u, 5.0f, 80.0f};
    s1.deliver();
    recv.update();
    ASSERT_TRUE(recv.hasNewData());
    EXPECT_EQ(recv.getData()->sensorId, 7u);
}

// ---------------------------------------------------------------------------
// Minimal CommandRegistry — models the event-driven design from
// data_container.md:
//   "sender ports and mempool are public and static so that everyone can
//    access them. Commands registry will be able to use sender ports to send
//    data to receiver ports when there's new socket data coming."
//
// Any thread can call dispatch() to invoke a named handler that fires a send.
// ---------------------------------------------------------------------------
class CommandRegistry
{
public:
    using Handler = std::function<void()>;

    void registerCommand(const std::string& name, Handler handler)
    {
        std::lock_guard<std::mutex> lock(m_mu);
        m_commands[name] = std::move(handler);
    }

    // Returns false when the command is not registered.
    bool dispatch(const std::string& name)
    {
        Handler h;
        {
            std::lock_guard<std::mutex> lock(m_mu);
            auto                        it = m_commands.find(name);
            if (it == m_commands.end())
                return false;
            h = it->second; // copy out before releasing lock
        }
        h();
        return true;
    }

private:
    std::unordered_map<std::string, Handler> m_commands;
    std::mutex                               m_mu;
};

// ---------------------------------------------------------------------------
// Multithread tests
// ---------------------------------------------------------------------------

// Sender runs on a worker thread; receiver polls on the calling thread.
// Verifies that deliver() and update() running concurrently never corrupt data.
TEST(Multithread, SenderOnWorkerReceiverOnMain)
{
    dc::Mempool<SensorReading>      pool(8);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    constexpr int     kSends = 200;
    std::atomic<int>  sentCount{0};
    std::atomic<bool> workerDone{false};

    std::thread worker(
        [&]()
        {
            for (int i = 0; i < kSends; ++i)
            {
                SensorReading* slot = sender.reserve();
                if (slot)
                {
                    slot->sensorId    = static_cast<uint32_t>(i);
                    slot->temperature = static_cast<float>(i);
                    sender.deliver();
                    sentCount.fetch_add(1, std::memory_order_relaxed);
                }
            }
            workerDone.store(true, std::memory_order_release);
        });

    int received = 0;
    while (!workerDone.load(std::memory_order_acquire))
    {
        recv.update();
        if (recv.hasNewData())
            ++received;
        recv.cleanup();
    }
    recv.update(); // final drain
    if (recv.hasNewData())
        ++received;

    worker.join();

    EXPECT_GE(sentCount.load(), 1);
    EXPECT_GE(received, 1);
    ASSERT_TRUE(recv.hasData());
    EXPECT_LT(recv.getData()->sensorId, static_cast<uint32_t>(kSends));
}

// One sender delivers a single message; four receiver threads call update()
// concurrently.  All should observe the data without any data race.
TEST(Multithread, MultipleReceiversUpdatingConcurrently)
{
    constexpr int kReceivers = 4;

    dc::Mempool<SensorReading>    pool(16);
    dc::SenderPort<SensorReading> sender;
    sender.connectMempool(pool);

    std::vector<std::unique_ptr<dc::ReceiverPort<SensorReading>>> receivers;
    for (int i = 0; i < kReceivers; ++i)
    {
        auto r = std::make_unique<dc::ReceiverPort<SensorReading>>();
        r->connect(sender);
        receivers.push_back(std::move(r));
    }

    *sender.reserve() = {77u, 3.14f, 99.9f};
    sender.deliver();

    std::atomic<int>         sawNewData{0};
    std::vector<std::thread> threads;
    for (auto& r : receivers)
    {
        threads.emplace_back(
            [&r, &sawNewData]()
            {
                r->update();
                if (r->hasNewData())
                    sawNewData.fetch_add(1, std::memory_order_relaxed);
                r->cleanup();
            });
    }
    for (auto& t : threads)
        t.join();

    EXPECT_EQ(sawNewData.load(), kReceivers);
    for (auto& r : receivers)
    {
        ASSERT_TRUE(r->hasData());
        EXPECT_EQ(r->getData()->sensorId, 77u);
    }
}

// N threads call reserve→deliver on the SAME SenderPort concurrently.
// Individual operations are mutex-protected, so no crash or torn writes may
// occur.  Some reservations will be dropped (latest-caller-wins), which is
// the expected behaviour for a single-slot port.
TEST(Multithread, ConcurrentReserveDeliverOnSharedSenderIsSafe)
{
    constexpr int kThreads        = 8;
    constexpr int kSendsPerThread = 50;

    dc::Mempool<SensorReading>      pool(kThreads * 4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    std::atomic<int>         deliveries{0};
    std::vector<std::thread> workers;
    for (int t = 0; t < kThreads; ++t)
    {
        workers.emplace_back(
            [&, t]()
            {
                for (int i = 0; i < kSendsPerThread; ++i)
                {
                    SensorReading* slot = sender.reserve();
                    if (slot)
                    {
                        slot->sensorId = static_cast<uint32_t>(t * 1000 + i);
                        sender.deliver();
                        deliveries.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
    }
    for (auto& w : workers)
        w.join();

    recv.update();

    // Integrity check: if data arrived, the sensorId must encode a valid
    // (thread, index) pair — no torn writes.
    if (recv.hasData())
    {
        uint32_t sid = recv.getData()->sensorId;
        EXPECT_LT(static_cast<int>(sid / 1000), kThreads);
        EXPECT_LT(static_cast<int>(sid % 1000), kSendsPerThread);
    }
    EXPECT_GE(deliveries.load(), 1);
}

// Two threads call update()+cleanup() on the same ReceiverPort concurrently.
// The port’s internal mutex must serialise these without deadlock or crash.
TEST(Multithread, ConcurrentUpdateCleanupOnSameReceiverIsSafe)
{
    dc::Mempool<SensorReading>      pool(4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    *sender.reserve() = {1u, 0.f, 0.f};
    sender.deliver();

    std::thread t1(
        [&]()
        {
            for (int i = 0; i < 200; ++i)
            {
                recv.update();
                recv.cleanup();
            }
        });
    std::thread t2(
        [&]()
        {
            for (int i = 0; i < 200; ++i)
            {
                recv.update();
                recv.cleanup();
            }
        });
    t1.join();
    t2.join();

    SUCCEED(); // reaching here without crash/deadlock is the pass criterion
}

// ---------------------------------------------------------------------------
// Event-driven tests (command registry pattern)
// ---------------------------------------------------------------------------

// A command registered in the registry triggers a send from a worker thread;
// the main loop picks it up via update() on the next iteration.
TEST(EventDriven, CommandFromWorkerThreadDeliveredToMainLoop)
{
    dc::Mempool<SensorReading>      pool(8);
    dc::SenderPort<SensorReading>   sender; // would be static/public in production
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    CommandRegistry registry;
    registry.registerCommand(
        "sensor/update",
        [&]()
        {
            SensorReading* slot = sender.reserve();
            if (!slot)
                return;
            *slot = {55u, 21.0f, 45.0f};
            sender.deliver();
        });

    std::atomic<bool> eventFired{false};
    std::thread       workerThread(
        [&]()
        {
            registry.dispatch("sensor/update"); // simulates socket data arriving
            eventFired.store(true, std::memory_order_release);
        });

    // Main loop: poll until event has been fired, then do one final update.
    while (!eventFired.load(std::memory_order_acquire))
    {
        recv.update();
        recv.cleanup();
    }
    recv.update(); // final frame after worker finished
    workerThread.join();

    ASSERT_TRUE(recv.hasNewData() || recv.hasData());
    const SensorReading* d = recv.getData();
    ASSERT_NE(d, nullptr);
    EXPECT_EQ(d->sensorId, 55u);
    EXPECT_FLOAT_EQ(d->temperature, 21.0f);
    EXPECT_FLOAT_EQ(d->humidity, 45.0f);
}

// Two command types route independently to separate SenderPort+ReceiverPort
// channels.  Commands fired from different worker threads must not cross-
// contaminate each other.
TEST(EventDriven, MultipleCommandTypesRoutedToIndependentChannels)
{
    dc::Mempool<SensorReading>      pool(16);
    dc::SenderPort<SensorReading>   tempSender, humSender;
    dc::ReceiverPort<SensorReading> tempRecv, humRecv;
    tempSender.connectMempool(pool);
    humSender.connectMempool(pool);
    tempRecv.connect(tempSender);
    humRecv.connect(humSender);

    CommandRegistry registry;
    registry.registerCommand(
        "sensor/temperature",
        [&]()
        {
            SensorReading* slot = tempSender.reserve();
            if (!slot)
                return;
            *slot = {1u, 36.6f, 0.0f};
            tempSender.deliver();
        });
    registry.registerCommand(
        "sensor/humidity",
        [&]()
        {
            SensorReading* slot = humSender.reserve();
            if (!slot)
                return;
            *slot = {2u, 0.0f, 78.5f};
            humSender.deliver();
        });

    std::thread w0([&]() { registry.dispatch("sensor/temperature"); });
    std::thread w1([&]() { registry.dispatch("sensor/humidity"); });
    w0.join();
    w1.join();

    tempRecv.update();
    humRecv.update();

    ASSERT_TRUE(tempRecv.hasNewData());
    ASSERT_TRUE(humRecv.hasNewData());
    EXPECT_EQ(tempRecv.getData()->sensorId, 1u);
    EXPECT_EQ(humRecv.getData()->sensorId, 2u);
    EXPECT_FLOAT_EQ(tempRecv.getData()->temperature, 36.6f);
    EXPECT_FLOAT_EQ(humRecv.getData()->humidity, 78.5f);
}

// Stress: a thread pool hammers the registry with dispatches while the main
// loop continuously drains the receiver.  At least one message must arrive;
// any data that does arrive must be non-corrupt.
TEST(EventDriven, HighThroughputCommandDispatch)
{
    constexpr int kWorkers    = 4;
    constexpr int kDispatches = 100;

    dc::Mempool<SensorReading>      pool(kWorkers * 4);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    std::atomic<uint32_t> seq{0};
    CommandRegistry       registry;
    registry.registerCommand(
        "ping",
        [&]()
        {
            SensorReading* slot = sender.reserve();
            if (!slot)
                return;
            slot->sensorId = seq.fetch_add(1, std::memory_order_relaxed);
            sender.deliver();
        });

    std::atomic<bool>        allDone{false};
    std::vector<std::thread> workers;
    for (int t = 0; t < kWorkers; ++t)
        workers.emplace_back(
            [&]()
            {
                for (int i = 0; i < kDispatches; ++i)
                    registry.dispatch("ping");
            });

    // Main-loop pattern: update top, work, cleanup bottom
    int         received = 0;
    std::thread joiner(
        [&]()
        {
            for (auto& w : workers)
                w.join();
            allDone.store(true, std::memory_order_release);
        });
    while (!allDone.load(std::memory_order_acquire))
    {
        recv.update();
        if (recv.hasNewData())
            ++received;
        recv.cleanup();
    }
    recv.update(); // final drain
    if (recv.hasNewData())
        ++received;
    joiner.join();

    EXPECT_GE(seq.load(), 1u);
    EXPECT_GE(received, 1);
    if (recv.hasData())
        EXPECT_LT(recv.getData()->sensorId, static_cast<uint32_t>(kWorkers * kDispatches));
}

// Verifies the prescribed main-loop pattern from data_container.md:
//   1. receivers.update() at top of loop
//   2. act on hasNewData() in the middle
//   3. receivers.cleanup() at bottom of loop
// while commands arrive asynchronously from background threads.
TEST(EventDriven, MainLoopUpdateCleanupPatternWithAsyncCommand)
{
    dc::Mempool<SensorReading>      pool(8);
    dc::SenderPort<SensorReading>   sender;
    dc::ReceiverPort<SensorReading> recv;
    sender.connectMempool(pool);
    recv.connect(sender);

    CommandRegistry registry;
    registry.registerCommand(
        "event",
        [&]()
        {
            SensorReading* slot = sender.reserve();
            if (!slot)
                return;
            *slot = {42u, 1.5f, 2.5f};
            sender.deliver();
        });

    std::atomic<bool> eventSent{false};
    std::thread       eventThread(
        [&]()
        {
            std::this_thread::yield(); // let main loop spin at least once first
            registry.dispatch("event");
            eventSent.store(true, std::memory_order_release);
        });

    bool seen = false;
    for (int frame = 0; frame < 100'000 && !seen; ++frame)
    {
        recv.update(); // --- top of loop ---
        if (recv.hasNewData())
        {
            const SensorReading* d = recv.getData();
            ASSERT_NE(d, nullptr);
            EXPECT_EQ(d->sensorId, 42u);
            EXPECT_FLOAT_EQ(d->temperature, 1.5f);
            seen = true;
        }
        recv.cleanup(); // --- bottom of loop ---
    }
    eventThread.join();
    EXPECT_TRUE(seen);
}
