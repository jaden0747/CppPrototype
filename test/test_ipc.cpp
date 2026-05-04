// test_ipc.cpp — functional and non-functional tests for the IPC abstractions
//
// Layout:
//   MockChannel   in-process stub for Consumer unit tests (no OS resources)
//   Consumer      line-splitting and partial-line buffering (MockChannel)
//   FifoChannel   FIFO lifecycle, data integrity, EPIPE handling
//   Producer      thread lifecycle, stop latency, EPIPE exit
//   Integration   Producer + Consumer end-to-end over a real FIFO
//
// Functional requirements verified:
//   FR-1  Consumer splits byte stream on '\n' into discrete messages
//   FR-2  Partial lines (no trailing '\n') are buffered across drain() calls
//   FR-3  FifoChannel::create() produces an OS FIFO at the given path
//   FR-4  openReader() is non-blocking; openWriter() unblocks once reader is open
//   FR-5  tryRead() returns -1/EAGAIN when the pipe has no data
//   FR-6  FIFO preserves write order across concurrent drain() calls
//   FR-7  Producer thread writes at the configured interval
//   FR-8  Closing the read end delivers EPIPE; producer exits without crashing
//   FR-9  stop() is idempotent (safe to call multiple times)
//   FR-10 Custom MessageFn is respected; sensorSource() is the default
//
// Non-functional requirements verified:
//   NFR-1  stop() completes within 200 ms (cv.notify_one() interrupts sleep)
//   NFR-2  Shutdown order (consumer first, then producer) avoids deadlock
//   NFR-3  destroy() is idempotent (no crash on double-call)
//   NFR-4  No crash when drain() is called before open()

#include <gtest/gtest.h>

#include "mylib/ipc/channel.h"
#include "mylib/ipc/consumer.h"
#include "mylib/ipc/fifo_channel.h"
#include "mylib/ipc/producer.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <queue>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

// ---------------------------------------------------------------------------
// Unique FIFO paths per process so parallel test runs don't collide.
// Each test fixture appends a suffix to isolate its own OS resource.
// ---------------------------------------------------------------------------
static const std::string kBase =
    "/tmp/gtest_ipc_" + std::to_string(static_cast<long>(::getpid()));

// ---------------------------------------------------------------------------
// MockChannel — fulfils IChannel without touching the filesystem.
// Each enqueued string represents exactly one tryRead() return value.
// Used to drive Consumer in isolation (no FIFO, no threads).
// ---------------------------------------------------------------------------
class MockChannel final : public IChannel
{
public:
    void enqueue(std::string chunk) { m_chunks.push(std::move(chunk)); }

    void    create()  override {}
    void    destroy() override {}
    bool    openWriter()                    override { return true; }
    ssize_t write(const void*, size_t n)    override { return static_cast<ssize_t>(n); }
    void    closeWriter()                   override {}
    bool    openReader()                    override { return true; }

    ssize_t tryRead(void* buf, size_t n) override
    {
        if (m_chunks.empty()) { errno = EAGAIN; return -1; }
        const auto& chunk = m_chunks.front();
        size_t      count = std::min(n, chunk.size());
        std::memcpy(buf, chunk.data(), count);
        m_chunks.pop();
        return static_cast<ssize_t>(count);
    }

    void closeReader() override {}

private:
    std::queue<std::string> m_chunks;
};

// ===========================================================================
// Consumer unit tests  (FR-1, FR-2, NFR-4)
// ===========================================================================

// FR-1: empty channel produces no lines
TEST(Consumer, DrainOnEmptyChannelReturnsNoLines)
{
    MockChannel mc;
    Consumer    c{mc};
    c.open();
    EXPECT_TRUE(c.drain().empty());
}

// FR-1: a single chunk ending with '\n' produces exactly one line (without the newline)
TEST(Consumer, DrainSingleCompleteLine)
{
    MockChannel mc;
    mc.enqueue("hello\n");
    Consumer c{mc};
    c.open();

    auto lines = c.drain();
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "hello");
}

// FR-1: multiple newline-terminated messages in one chunk are all returned
TEST(Consumer, DrainMultipleCompleteLinesSingleChunk)
{
    MockChannel mc;
    mc.enqueue("alpha\nbeta\ngamma\n");
    Consumer c{mc};
    c.open();

    auto lines = c.drain();
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "alpha");
    EXPECT_EQ(lines[1], "beta");
    EXPECT_EQ(lines[2], "gamma");
}

// FR-2: a chunk without '\n' is buffered; the line appears only after the
// delimiter arrives in a subsequent drain() call
TEST(Consumer, PartialLineHeldUntilNewlineArrives)
{
    MockChannel mc;
    mc.enqueue("partial");   // no newline
    Consumer c{mc};
    c.open();

    EXPECT_TRUE(c.drain().empty());   // buffered — not yet emitted

    mc.enqueue(" done\n");
    auto lines = c.drain();
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "partial done");
}

// FR-2: a line can be spread across more than two chunks before completing
TEST(Consumer, LineSplitAcrossMultipleChunksIsReassembled)
{
    MockChannel mc;
    mc.enqueue("fo");
    mc.enqueue("o\n");
    mc.enqueue("bar\n");
    Consumer c{mc};
    c.open();

    auto lines = c.drain();
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "foo");
    EXPECT_EQ(lines[1], "bar");
}

// NFR-4: drain() before open() must not crash and must return nothing
TEST(Consumer, DrainBeforeOpenReturnsEmpty)
{
    MockChannel mc;
    mc.enqueue("data\n");
    Consumer c{mc};
    // deliberately skip c.open()
    EXPECT_TRUE(c.drain().empty());
}

// FR-1: blank lines (consecutive newlines) are treated as empty strings,
// not silently discarded
TEST(Consumer, EmptyLinesArePreservedNotDropped)
{
    MockChannel mc;
    mc.enqueue("a\n\nb\n");
    Consumer c{mc};
    c.open();

    auto lines = c.drain();
    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "a");
    EXPECT_EQ(lines[1], "");
    EXPECT_EQ(lines[2], "b");
}

// ===========================================================================
// FifoChannel functional tests  (FR-3, FR-4, FR-5, FR-6, FR-8, NFR-3)
// ===========================================================================

struct FifoChannelTest : ::testing::Test
{
    const std::string path = kBase + "_fc";
    FifoChannel       ch{path};

    void SetUp()    override { signal(SIGPIPE, SIG_IGN); ch.create(); }
    void TearDown() override { ch.destroy(); }   // idempotent — safe even if test already destroyed

    // Opens both ends without blocking: reader first (O_NONBLOCK), then writer.
    // After this returns, both file descriptors are valid.
    void openBothEnds()
    {
        ASSERT_TRUE(ch.openReader());
        std::thread([this] { ch.openWriter(); }).join();
    }
};

// FR-3: create() produces an OS FIFO at the given path
TEST_F(FifoChannelTest, CreateMakesFilesystemEntry)
{
    struct stat st;
    ASSERT_EQ(::stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISFIFO(st.st_mode));
}

// FR-3 / NFR-3: destroy() removes the path; calling it again must not crash
TEST_F(FifoChannelTest, DestroyRemovesFilesystemEntryAndIsIdempotent)
{
    ch.destroy();
    struct stat st;
    EXPECT_NE(::stat(path.c_str(), &st), 0) << "FIFO should be gone after destroy()";

    ch.destroy();  // second call — must not crash (NFR-3)
}

// FR-4: openReader() with O_NONBLOCK must return immediately even if no writer
// has opened the other end yet
TEST_F(FifoChannelTest, OpenReaderDoesNotBlockWithNoWriter)
{
    auto t0 = std::chrono::steady_clock::now();
    ASSERT_TRUE(ch.openReader());
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();
    EXPECT_LT(ms, 50) << "openReader() should return immediately with O_NONBLOCK";
    ch.closeReader();
}

// FR-5: tryRead() on a FIFO with no pending data must return -1 with EAGAIN,
// not block
TEST_F(FifoChannelTest, TryReadOnEmptyChannelReturnsEAGAIN)
{
    openBothEnds();
    char    buf[64];
    ssize_t n = ch.tryRead(buf, sizeof(buf));
    EXPECT_EQ(n, -1);
    EXPECT_TRUE(errno == EAGAIN || errno == EWOULDBLOCK);
    ch.closeWriter();
    ch.closeReader();
}

// FR-1 (channel level): bytes written are received intact
TEST_F(FifoChannelTest, WriteAndReadRoundTrip)
{
    openBothEnds();
    const std::string msg = "sensor_temp=25.0\n";
    ASSERT_EQ(ch.write(msg.data(), msg.size()), static_cast<ssize_t>(msg.size()));

    char    buf[256] = {};
    ssize_t n        = ch.tryRead(buf, sizeof(buf) - 1);
    ASSERT_GT(n, 0);
    EXPECT_EQ(std::string(buf, static_cast<size_t>(n)), msg);
    ch.closeWriter();
    ch.closeReader();
}

// FR-6: the FIFO is a queue — writes arrive in the order they were sent
TEST_F(FifoChannelTest, MultipleWritesArrivedInOrder)
{
    openBothEnds();
    for (int i = 0; i < 5; ++i)
    {
        std::string line = "line" + std::to_string(i) + "\n";
        ASSERT_GT(ch.write(line.data(), line.size()), 0);
    }

    std::string received;
    char        buf[1024] = {};
    ssize_t     n;
    while ((n = ch.tryRead(buf, sizeof(buf) - 1)) > 0)
    {
        buf[n] = '\0';
        received += buf;
    }
    EXPECT_EQ(received, "line0\nline1\nline2\nline3\nline4\n");
    ch.closeWriter();
    ch.closeReader();
}

// FR-3: create() on a path that already has a FIFO must succeed (unlink + mkfifo)
TEST_F(FifoChannelTest, CreateHandlesPreexistingFifo)
{
    ch.destroy();
    ::mkfifo(path.c_str(), 0666);   // leave a stale FIFO
    ch.create();                     // must not fail

    struct stat st;
    ASSERT_EQ(::stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISFIFO(st.st_mode));
}

// FR-8: closing the read end causes write() to fail with EPIPE
// (SIGPIPE is ignored process-wide in SetUp)
TEST_F(FifoChannelTest, CloseReaderCausesEPIPEOnWrite)
{
    openBothEnds();
    ch.closeReader();   // no readers — next write should fail

    const char buf[] = "x";
    ssize_t    result = ch.write(buf, 1);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(errno, EPIPE);
    ch.closeWriter();
}

// ===========================================================================
// Producer tests  (FR-7, FR-8, FR-9, FR-10, NFR-1)
// ===========================================================================

struct ProducerTest : ::testing::Test
{
    FifoChannel               ch{kBase + "_prod"};
    std::unique_ptr<Producer> producer;

    void SetUp() override
    {
        signal(SIGPIPE, SIG_IGN);
        ch.create();
        // Consumer (test) opens read end first so the producer's openWriter()
        // returns immediately when the thread starts.
        ASSERT_TRUE(ch.openReader());
        producer = std::make_unique<Producer>(
            ch, Producer::sensorSource(), std::chrono::milliseconds{50});
    }

    void TearDown() override
    {
        producer.reset();   // stop() is called in destructor
        ch.destroy();
    }
};

// Basic smoke test: start and stop without crashing
TEST_F(ProducerTest, StartAndStopNoCrash)
{
    producer->start();
    producer->stop();
    SUCCEED();
}

// FR-9: stop() must be safe to call multiple times
TEST_F(ProducerTest, StopIsIdempotent)
{
    producer->start();
    producer->stop();
    producer->stop();   // second call must not deadlock or crash
    SUCCEED();
}

// NFR-1: cv.notify_one() in stop() interrupts the inter-message sleep;
// the thread should exit well within one interval period
TEST_F(ProducerTest, StopCompletesWithinBoundedTime)
{
    producer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds{80});   // let it run

    auto t0 = std::chrono::steady_clock::now();
    producer->stop();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    // Interval is 50 ms; with cv notification, stop should complete < 200 ms
    // even on a loaded CI machine.
    EXPECT_LT(ms, 200) << "stop() took too long: " << ms << " ms";
}

// FR-7: at least one message must appear in the FIFO after one interval
TEST_F(ProducerTest, WritesAtLeastOneMessageAfterStart)
{
    producer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds{150});  // > 2 × 50 ms

    char    buf[4096] = {};
    ssize_t n         = ch.tryRead(buf, sizeof(buf) - 1);
    EXPECT_GT(n, 0) << "No data in FIFO after 150 ms";
}

// FR-8: when the read end is closed, the producer must exit its write loop
// cleanly (no crash, no hang) and stop() must join in bounded time
TEST_F(ProducerTest, ExitsCleanlyOnEPIPE)
{
    producer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds{80});
    ch.closeReader();   // delivers EPIPE on the producer's next write

    auto t0 = std::chrono::steady_clock::now();
    producer->stop();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();
    EXPECT_LT(ms, 400) << "Producer did not exit after EPIPE within 400 ms";
}

// FR-10: sensorSource() returns a non-empty string (the '\n' is appended
// by Producer::run(), so we verify the raw fn output has content)
TEST_F(ProducerTest, SensorSourceProducesNonEmptyMessages)
{
    auto fn  = Producer::sensorSource();
    auto msg = fn();
    EXPECT_FALSE(msg.empty());
    EXPECT_GT(msg.size(), 8u) << "Message seems too short: " << msg;
}

// ===========================================================================
// Integration tests — real FIFO, real Producer, real Consumer  (end to end)
// ===========================================================================

struct IntegrationTest : ::testing::Test
{
    FifoChannel               ch{kBase + "_int"};
    std::unique_ptr<Consumer> consumer;
    std::unique_ptr<Producer> producer;

    void SetUp() override
    {
        signal(SIGPIPE, SIG_IGN);
        ch.create();
        // Consumer opens first so the producer's openWriter() is non-blocking.
        consumer = std::make_unique<Consumer>(ch);
        ASSERT_TRUE(consumer->open());
        producer = std::make_unique<Producer>(
            ch, Producer::sensorSource(), std::chrono::milliseconds{50});
    }

    void TearDown() override
    {
        // NFR-2: correct shutdown order — consumer first triggers EPIPE,
        // then producer joins quickly.
        consumer.reset();
        producer.reset();
        ch.destroy();
    }
};

// FR-1 + FR-7: messages written by the producer appear in the consumer's drain()
TEST_F(IntegrationTest, MessagesFlowFromProducerToConsumer)
{
    producer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    producer->stop();

    auto lines = consumer->drain();
    EXPECT_GT(lines.size(), 0u) << "No messages received from producer";
}

// FR-1: every drained line must be non-empty (producer always appends '\n',
// so no empty line should make it through unless intentionally sent)
TEST_F(IntegrationTest, DrainedLinesAreNonEmpty)
{
    producer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    producer->stop();

    for (const auto& line : consumer->drain())
        EXPECT_FALSE(line.empty()) << "Unexpected empty line: '" << line << "'";
}

// FR-7: calling drain() multiple times accumulates all produced messages
TEST_F(IntegrationTest, MultipleSequentialDrainsAccumulateMessages)
{
    producer->start();

    int total = 0;
    for (int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{60});
        total += static_cast<int>(consumer->drain().size());
    }
    producer->stop();
    total += static_cast<int>(consumer->drain().size());  // final drain

    EXPECT_GT(total, 0) << "No messages accumulated across 5 drain() calls";
}

// NFR-2: shutting down consumer before producer (correct order) must complete
// within a bounded time and must not deadlock
TEST_F(IntegrationTest, CleanShutdownOrderDoesNotDeadlock)
{
    producer->start();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    auto t0 = std::chrono::steady_clock::now();
    consumer->close();   // closes read end → EPIPE for producer
    producer->stop();    // joins quickly because of EPIPE
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::steady_clock::now() - t0).count();

    EXPECT_LT(ms, 500) << "Clean shutdown took too long: " << ms << " ms";
}

// FR-10: a custom MessageFn is called instead of the default sensor source;
// this verifies that different data sources can be swapped in without
// modifying Producer or Consumer (the key extensibility point for Day 2+)
TEST_F(IntegrationTest, CustomMessageFunctionIsUsed)
{
    std::atomic<int> callCount{0};
    auto customFn = [&callCount]() -> std::string {
        int n = callCount.fetch_add(1, std::memory_order_relaxed);
        return "custom:" + std::to_string(n);
    };

    Producer custom{ch, customFn, std::chrono::milliseconds{40}};
    custom.start();
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    custom.stop();

    auto lines = consumer->drain();
    ASSERT_GT(lines.size(), 0u);
    for (const auto& line : lines)
        EXPECT_EQ(line.substr(0, 7), "custom:") << "Unexpected line: " << line;
}
