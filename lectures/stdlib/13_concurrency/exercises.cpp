// ============================================================================
// Stdlib 13 — Exercises: Concurrency
// ============================================================================
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <numeric>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ── Exercise 1: Parallel vector sum ──────────────────────────────────────
void ex1_parallel_sum()
{
    std::cout << "Exercise 1: Parallel sum\n";

    std::vector<int> data(1000000);
    std::iota(data.begin(), data.end(), 1);

    // TODO: Split data into N chunks (N = hardware_concurrency)
    // TODO: Launch threads to sum each chunk
    // TODO: Combine results
    // TODO: Verify against sequential sum

    std::cout << "\n";
}

// ── Exercise 2: Producer-consumer queue ──────────────────────────────────
// TODO: Implement a thread-safe queue with:
//   - push(T) — add item, notify consumer
//   - pop() -> T — block until item available
//   - Use mutex + condition_variable

void ex2_producer_consumer()
{
    std::cout << "Exercise 2: Producer-consumer queue\n";

    // TODO: Create a producer thread that pushes 10 items
    // TODO: Create a consumer thread that pops and prints them
    // TODO: Use a special sentinel value to signal "done"

    std::cout << "\n";
}

// ── Exercise 3: Atomic counter comparison ────────────────────────────────
void ex3_atomic_vs_mutex()
{
    std::cout << "Exercise 3: Atomic vs mutex counter\n";

    // TODO: Implement two counters: one with atomic, one with mutex
    // TODO: Have 4 threads each increment 100000 times
    // TODO: Time both approaches using chrono
    // TODO: Print results and timing

    std::cout << "\n";
}

// ── Exercise 4: async parallel map ───────────────────────────────────────
void ex4_parallel_map()
{
    std::cout << "Exercise 4: Parallel map with async\n";

    std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8};

    // TODO: Write a parallel_map function that applies a function to each element
    //        using std::async for each chunk
    // TODO: Test with squaring function
    // TODO: Print results

    std::cout << "\n";
}

// ── Exercise 5: Promise chain ────────────────────────────────────────────
void ex5_promise_chain()
{
    std::cout << "Exercise 5: Promise chain\n";

    // TODO: Create a chain of 3 threads:
    //   Thread 1: produces an int via promise
    //   Thread 2: takes that int, doubles it, produces via another promise
    //   Thread 3: takes the doubled value, prints it
    // TODO: Wire them together with promise/future pairs

    std::cout << "\n";
}

// ── Exercise 6: Reader-writer lock ───────────────────────────────────────
void ex6_reader_writer()
{
    std::cout << "Exercise 6: Reader-writer pattern\n";

    // TODO: Implement a shared data structure with:
    //   - Multiple concurrent readers (use shared_mutex if available, or simulate)
    //   - Exclusive writer access
    // TODO: Launch 4 reader threads and 1 writer thread
    // TODO: Print read/write operations

    std::cout << "\n";
}

// ── Exercise 7: Parallel for_each ────────────────────────────────────────
void ex7_parallel_for_each()
{
    std::cout << "Exercise 7: Parallel for_each\n";

    // TODO: Write a parallel_for_each(begin, end, fn) that:
    //   - Splits the range into chunks
    //   - Launches one thread per chunk
    //   - Each thread applies fn to its elements
    // TODO: Test by transforming a vector of strings to uppercase

    std::cout << "\n";
}

// ── Exercise 8: Mini-project — thread pool ───────────────────────────────
// TODO: Implement a simple ThreadPool with:
//   - Constructor(num_threads) — starts worker threads
//   - submit(callable) -> future<result_type> — enqueue task
//   - Workers pick tasks from a shared queue
//   - Destructor signals workers to stop and joins

void ex8_thread_pool()
{
    std::cout << "Exercise 8: Thread pool\n";

    // TODO: Create a ThreadPool with 4 threads
    // TODO: Submit 20 tasks that each compute fibonacci(n)
    // TODO: Collect and print all results via futures

    std::cout << "\n";
}

// ── Main ─────────────────────────────────────────────────────────────────
int main()
{
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  Stdlib 13 — Exercises: Concurrency              ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    ex1_parallel_sum();
    ex2_producer_consumer();
    ex3_atomic_vs_mutex();
    ex4_parallel_map();
    ex5_promise_chain();
    ex6_reader_writer();
    ex7_parallel_for_each();
    ex8_thread_pool();

    std::cout << "All exercises complete.\n";
    return 0;
}
