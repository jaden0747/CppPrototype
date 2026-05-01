// ============================================================================
// Stdlib 13 — Demo: Concurrency
// ============================================================================
#include <algorithm>
#include <atomic>
#include <barrier>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <latch>
#include <mutex>
#include <numeric>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ──────────────────────────────────────────────────────────────────────────
// 1. std::thread basics
// ──────────────────────────────────────────────────────────────────────────
void demo_thread()
{
    std::cout << "=== 1. std::thread ===\n";

    // Basic thread
    std::thread t([] { std::cout << "  hello from thread " << std::this_thread::get_id() << "\n"; });
    t.join();

    // Thread with arguments
    auto        worker = [](int id, const std::string& msg) { std::cout << "  worker " << id << ": " << msg << "\n"; };
    std::thread t1(worker, 1, "task A");
    std::thread t2(worker, 2, "task B");
    t1.join();
    t2.join();

    // Hardware concurrency
    std::cout << "  hardware_concurrency: " << std::thread::hardware_concurrency() << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 2. std::jthread (C++20) — auto-joining
// ──────────────────────────────────────────────────────────────────────────
void demo_jthread()
{
    std::cout << "=== 2. std::jthread (C++20) ===\n";

    // jthread auto-joins on destruction
    {
        std::jthread jt(
            [](std::stop_token st)
            {
                int i = 0;
                while (!st.stop_requested() && i < 5)
                {
                    std::cout << "  jthread iteration " << i++ << "\n";
                    std::this_thread::sleep_for(10ms);
                }
                std::cout << "  jthread: stop requested, exiting\n";
            });

        std::this_thread::sleep_for(30ms);
        jt.request_stop(); // cooperative cancellation
        // auto-joins here
    }
    std::cout << "  jthread auto-joined\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 3. Mutex & lock guards
// ──────────────────────────────────────────────────────────────────────────
void demo_mutex()
{
    std::cout << "=== 3. Mutex & Lock Guards ===\n";

    std::mutex mtx;
    int        shared_counter = 0;

    // lock_guard — RAII lock
    auto increment = [&](int n)
    {
        for (int i = 0; i < n; ++i)
        {
            std::lock_guard<std::mutex> lock(mtx);
            ++shared_counter;
        }
    };

    std::thread t1(increment, 10000);
    std::thread t2(increment, 10000);
    t1.join();
    t2.join();
    std::cout << "  counter (lock_guard): " << shared_counter << "\n";

    // scoped_lock — deadlock-safe multi-mutex
    std::mutex m1, m2;
    auto       safe_swap = [&]
    {
        std::scoped_lock lock(m1, m2); // acquires both atomically
        // swap operation here
    };
    std::thread t3(safe_swap);
    std::thread t4(safe_swap);
    t3.join();
    t4.join();
    std::cout << "  scoped_lock: no deadlock ✓\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 4. condition_variable
// ──────────────────────────────────────────────────────────────────────────
void demo_condition_variable()
{
    std::cout << "=== 4. condition_variable ===\n";

    std::mutex              mtx;
    std::condition_variable cv;
    bool                    ready = false;
    int                     data  = 0;

    // Producer
    std::thread producer(
        [&]
        {
            std::this_thread::sleep_for(50ms);
            {
                std::lock_guard lock(mtx);
                data  = 42;
                ready = true;
            }
            cv.notify_one();
        });

    // Consumer
    std::thread consumer(
        [&]
        {
            std::unique_lock lock(mtx);
            cv.wait(lock, [&] { return ready; });
            std::cout << "  consumer received: " << data << "\n";
        });

    producer.join();
    consumer.join();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 5. std::atomic
// ──────────────────────────────────────────────────────────────────────────
void demo_atomic()
{
    std::cout << "=== 5. std::atomic ===\n";

    std::atomic<int> counter{0};

    auto increment = [&](int n)
    {
        for (int i = 0; i < n; ++i)
            counter.fetch_add(1, std::memory_order_relaxed);
    };

    std::thread t1(increment, 100000);
    std::thread t2(increment, 100000);
    t1.join();
    t2.join();
    std::cout << "  atomic counter: " << counter.load() << "\n";

    // atomic_flag as spinlock
    std::atomic_flag flag           = ATOMIC_FLAG_INIT;
    int              protected_data = 0;

    auto spin_worker = [&]
    {
        for (int i = 0; i < 1000; ++i)
        {
            while (flag.test_and_set(std::memory_order_acquire))
            {
            } // spin
            ++protected_data;
            flag.clear(std::memory_order_release);
        }
    };

    std::thread t3(spin_worker);
    std::thread t4(spin_worker);
    t3.join();
    t4.join();
    std::cout << "  spinlock counter: " << protected_data << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 6. std::async & std::future
// ──────────────────────────────────────────────────────────────────────────
void demo_async()
{
    std::cout << "=== 6. async & future ===\n";

    // Launch async task
    auto fut = std::async(
        std::launch::async,
        []
        {
            std::this_thread::sleep_for(50ms);
            return 42;
        });

    std::cout << "  doing other work...\n";
    int result = fut.get(); // blocks until ready
    std::cout << "  async result: " << result << "\n";

    // promise/future pair
    std::promise<std::string> prom;
    auto                      f = prom.get_future();

    std::thread t(
        [&prom]
        {
            std::this_thread::sleep_for(30ms);
            prom.set_value("hello from promise");
        });

    std::cout << "  promise result: " << f.get() << "\n";
    t.join();

    // Parallel computation with async
    std::vector<int> data(1000000);
    std::iota(data.begin(), data.end(), 1);

    auto half = data.begin() + static_cast<long>(data.size()) / 2;
    auto f1   = std::async(std::launch::async, [&] { return std::accumulate(data.begin(), half, 0LL); });
    auto f2   = std::async(std::launch::async, [&] { return std::accumulate(half, data.end(), 0LL); });

    long long total = f1.get() + f2.get();
    std::cout << "  parallel sum(1..1M) = " << total << "\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 7. std::latch (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_latch()
{
    std::cout << "=== 7. std::latch (C++20) ===\n";

    constexpr int N = 4;
    std::latch    start_latch(1); // main signals start
    std::latch    done_latch(N);  // workers signal completion

    auto worker = [&](int id)
    {
        start_latch.wait(); // wait for go signal
        std::cout << "  worker " << id << " running\n";
        done_latch.count_down(); // signal done
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < N; ++i)
        threads.emplace_back(worker, i);

    std::cout << "  releasing workers...\n";
    start_latch.count_down(); // start all workers
    done_latch.wait();        // wait for all to finish
    std::cout << "  all workers done\n\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 8. std::barrier (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_barrier()
{
    std::cout << "=== 8. std::barrier (C++20) ===\n";

    constexpr int N     = 3;
    int           phase = 0;

    std::barrier sync_point(
        N,
        [&]() noexcept
        {
            ++phase;
            std::cout << "  --- phase " << phase << " complete ---\n";
        });

    auto worker = [&](int id)
    {
        for (int p = 0; p < 3; ++p)
        {
            std::cout << "  worker " << id << " phase " << p << "\n";
            sync_point.arrive_and_wait();
        }
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < N; ++i)
        threads.emplace_back(worker, i);

    // jthreads auto-join
    threads.clear(); // explicit join by clearing
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
// 9. std::counting_semaphore (C++20)
// ──────────────────────────────────────────────────────────────────────────
void demo_semaphore()
{
    std::cout << "=== 9. std::counting_semaphore (C++20) ===\n";

    // Limit concurrent access to 2
    std::counting_semaphore<2> sem(2);

    auto worker = [&](int id)
    {
        sem.acquire();
        std::cout << "  worker " << id << " acquired (max 2 concurrent)\n";
        std::this_thread::sleep_for(50ms);
        std::cout << "  worker " << id << " releasing\n";
        sem.release();
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 5; ++i)
        threads.emplace_back(worker, i);

    threads.clear();
    std::cout << "\n";
}

// ──────────────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 13 — Concurrency                         ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    demo_thread();
    demo_jthread();
    demo_mutex();
    demo_condition_variable();
    demo_atomic();
    demo_async();
    demo_latch();
    demo_barrier();
    demo_semaphore();

    std::cout << "All demos complete.\n";
    return 0;
}
